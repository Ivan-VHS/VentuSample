#include "generator.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void testValidation() {
    ventu::Parameters p;
    require(ventu::validate(p).empty(), "default parameters should validate");
    p.bpm = 40;
    require(!ventu::validate(p).empty(), "out-of-range BPM should be rejected");
    p.bpm = 93;
    p.lowPassHz = 20;
    require(!ventu::validate(p).empty(), "invalid filter ordering should be rejected");
    p.lowPassHz = 18000;
    p.sampleRate = 22050;
    require(!ventu::validate(p).empty(), "low-pass above the Nyquist limit should be rejected");
    require(ventu::render(p, 0).samples.empty(), "invalid parameters should not allocate an audio render");
    p.sampleRate = 96000; p.lowPassHz = 18000; p.bpm = 60; p.bars = 16; p.variations = 32;
    require(!ventu::validate(p).empty(), "oversized pack estimate should be rejected");
}

void testDeterministicRender() {
    ventu::Parameters p;
    p.sampleRate = 22050;
    p.lowPassHz = 10000;
    p.bpm = 120;
    p.bars = 1;
    p.variations = 2;
    const auto first = ventu::render(p, 0);
    const auto second = ventu::render(p, 0);
    const auto alternate = ventu::render(p, 1);
    require(first.samples.size() == second.samples.size(), "same seed should preserve frame count");
    require(first.samples == second.samples, "same seed should render identical samples");
    require(first.samples != alternate.samples, "different variation should alter rendered samples");
    require(first.frames() == static_cast<std::size_t>(22050 * 2), "one bar at 120 BPM should be two seconds");
    const auto stats = ventu::measure(first);
    require(std::isfinite(stats.peak) && stats.peak <= 0.967, "master peak should remain at or below -0.3 dBFS");
    require(std::isfinite(stats.rms) && stats.rms > 0.0, "rendered audio should have a finite non-zero RMS");
}

void testPhraseGridAndSwing() {
    ventu::Parameters p;
    p.style = ventu::Style::HarlemDipTrap;
    p.target = ventu::Target::FullLoop;
    p.sampleRate = 22050;
    p.lowPassHz = 10000;
    p.bpm = 93;
    p.bars = 4;
    p.variations = 2;
    p.swingPercent = 62;
    p.densityPercent = 85;
    p.microHumanize = false;

    const auto swung = ventu::render(p, 0);
    const auto repeated = ventu::render(p, 0);
    require(!swung.samples.empty(), "four-bar phrase should render audio");
    require(swung.samples == repeated.samples, "phrase cadence and swing must remain seed-deterministic");

    const auto expectedFrames = static_cast<std::size_t>(
        std::ceil((60.0 / p.bpm) * 4.0 * p.bars * p.sampleRate));
    require(swung.frames() == expectedFrames, "phrase grid must preserve exact bar duration");

    p.swingPercent = 50;
    const auto straight = ventu::render(p, 0);
    require(swung.samples != straight.samples, "swing must move offbeat drum and melodic events");

    const auto stats = ventu::measure(swung);
    require(std::isfinite(stats.peak) && stats.peak <= 0.967,
            "phrase rendering must retain the master sample-peak ceiling");
}


void testCadenceGridAudit() {
    ventu::AudioBuffer audio;
    audio.sampleRate = 48000;
    audio.channels = 2;
    audio.samples.assign(48000u * 2u, 0.0F);
    // At 120 BPM the sixteenth-note grid is 6,000 frames; step 2 is swung
    // by 32% of one beat at 58% swing.
    audio.eventFrames = {0u, 13920u, 18000u};
    const auto aligned = ventu::analyzeCadence(audio, 120, 58);
    require(aligned.valid && aligned.analyzed, "valid scheduled events should be analyzed");
    require(aligned.eventCount == 3u && aligned.onGridCount == 3u,
            "straight and swung events should match their shared grid");
    require(aligned.offGridCount == 0u && aligned.gridAccuracyPercent == 100.0,
            "fully aligned events should score 100 percent");

    audio.eventFrames.push_back(19000u);
    const auto displaced = ventu::analyzeCadence(audio, 120, 58);
    require(displaced.valid && displaced.offGridCount == 1u,
            "an event outside tolerance should be reported off-grid");
    require(displaced.gridAccuracyPercent < 100.0 && displaced.maxDeviationMs > 0.0,
            "off-grid events should lower grid accuracy and expose deviation");

    audio.eventFrames = {audio.frames()};
    const auto outOfBounds = ventu::analyzeCadence(audio, 120, 58);
    require(!outOfBounds.valid, "events outside the audio buffer should be rejected");

    audio.eventFrames.clear();
    const auto empty = ventu::analyzeCadence(audio, 120, 58);
    require(empty.valid && !empty.analyzed, "one-shot or event-free audio should report cadence as not applicable");
}

void testWaveAndPackExport() {
    const auto temp = std::filesystem::temp_directory_path() / "ventusample-tests";
    std::error_code ec;
    std::filesystem::remove_all(temp, ec);
    std::filesystem::create_directories(temp, ec);
    require(!ec, "test output directory should be created");

    ventu::Parameters p;
    p.sampleRate = 22050;
    p.lowPassHz = 10000;
    p.bpm = 120;
    p.bars = 1;
    p.variations = 2;
    const auto audio = ventu::render(p, 0);
    std::string error;
    const auto wav = temp / "single.wav";
    const bool waveWritten = ventu::writeWave(wav, audio, error);
    require(waveWritten, "PCM16 WAV export should succeed");

    std::ifstream file(wav, std::ios::binary);
    std::array<char, 4> signature{};
    file.read(signature.data(), static_cast<std::streamsize>(signature.size()));
    const std::array<char, 4> riff{'R', 'I', 'F', 'F'};
    require(signature == riff, "exported WAV should have a RIFF signature");
    const auto imported = ventu::loadWave(wav, error);
    require(error.empty(), "exported WAV should import without errors");
    require(imported.channels == 2, "WAV importer should preserve stereo channels");
    require(imported.frames() == audio.frames(), "WAV importer should preserve frame count");

    p.sourceSamplePath = wav.string();
    p.sourceMixPercent = 100;
    const auto chopped = ventu::render(p, 0, {}, &imported);
    require(chopped.samples.size() == audio.samples.size(), "sample chops should preserve target loop length");
    require(chopped.samples != audio.samples, "source chop layer should alter the rendered mix");

    const auto pack = temp / "pack";
    const auto batch = ventu::generatePack(p, pack);
    require(static_cast<bool>(batch), "sample pack generation should succeed");
    require(batch.cadence.size() == batch.files.size(), "every exported loop should carry a cadence report");
    require(batch.cadence.front().valid && batch.cadence.front().analyzed &&
            batch.cadence.front().gridAccuracyPercent >= 99.0,
            "generated loops should expose high-accuracy event-grid diagnostics");
    require(batch.files.size() == 2, "two variations should export two loop files");
    require(std::filesystem::exists(pack / "ventusample_manifest.json"), "sample pack should include a manifest");
    std::ifstream manifest(pack / "ventusample_manifest.json", std::ios::binary);
    const std::string json((std::istreambuf_iterator<char>(manifest)), std::istreambuf_iterator<char>());
    require(json.find("ventusample.pack.v1") != std::string::npos, "manifest should use the v1 schema id");
    require(json.find("East Coast") != std::string::npos, "manifest should record the selected style");
    require(json.find("single.wav") != std::string::npos, "manifest should identify the imported source by basename");
    require(json.find("density_percent") != std::string::npos, "manifest should include the full parameter snapshot");
    require(json.find("variations") != std::string::npos, "manifest should include the variation count");
    require(json.find("bus_compression_ratio") != std::string::npos, "manifest should include bus compression settings");
    require(json.find("audio_format") != std::string::npos, "manifest should declare its output format");
    require(json.find("crest_db") != std::string::npos, "manifest should include crest measurements");
    const auto duplicatePack = ventu::generatePack(p, pack);
    require(!duplicatePack && !duplicatePack.error.empty(), "existing pack should never be silently overwritten");

    const auto collision = temp / "collision";
    std::filesystem::create_directories(collision, ec);
    require(!ec, "collision test directory should be created");
    std::filesystem::copy_file(batch.files.front(), collision / batch.files.front().filename(),
                               std::filesystem::copy_options::overwrite_existing, ec);
    require(!ec, "collision fixture should be created");
    const auto collisionResult = ventu::generatePack(p, collision);
    require(!collisionResult, "existing audio file should block the whole pack before rendering");
    require(!std::filesystem::exists(collision / "ventusample_manifest.json"),
            "failed preflight should not create a partial manifest");
    require(std::filesystem::is_regular_file(collision / batch.files.front().filename()),
            "failed preflight must preserve the user's existing audio file");

    p.target = ventu::Target::OneShotKit;
    p.variations = 1;
    const auto kit = ventu::generatePack(p, temp / "kit");
    require(static_cast<bool>(kit), "one-shot kit generation should succeed");
    require(kit.cadence.size() == kit.files.size() && !kit.cadence.front().analyzed,
            "one-shot exports should mark loop cadence as not applicable");
    require(kit.files.size() == 8, "one-shot kit should contain eight separate hits");
    // Keep the deterministic pack fixture for the independent schema-shape test.
}
}

int main() {
    testValidation();
    testDeterministicRender();
    testPhraseGridAndSwing();
    testCadenceGridAudit();
    testWaveAndPackExport();
    std::cout << "VentuSample generator tests passed.\n";
    return 0;
}
