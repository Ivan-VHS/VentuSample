#include "generator.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

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

void testBillionaireGoldenEraProfile() {
    ventu::Parameters p;
    p.style = ventu::Style::BillionaireGoldenEra;
    p.target = ventu::Target::FullLoop;
    p.sampleRate = 22050;
    p.lowPassHz = 10000;
    p.bpm = 93;
    p.bars = 4;
    p.variations = 1;
    p.microHumanize = false;

    require(ventu::toString(p.style) == "Billionaire Golden Era",
            "profile name must match the shared pack contract");
    require(ventu::validate(p).empty(), "Billionaire Golden Era parameters should validate");
    const auto first = ventu::render(p, 0);
    const auto repeated = ventu::render(p, 0);
    require(!first.samples.empty(), "Billionaire Golden Era should render a loop");
    require(first.samples == repeated.samples,
            "Billionaire Golden Era must remain deterministic for a fixed seed");

    p.style = ventu::Style::EastCoast;
    const auto eastCoast = ventu::render(p, 0);
    require(first.samples != eastCoast.samples,
            "Billionaire Golden Era should have distinct drum voicing");

    // Four distinct source regions make the fixed 0-2-1-3 crate order observable.
    ventu::AudioBuffer source;
    source.sampleRate = 22050;
    source.channels = 2;
    constexpr std::size_t sliceFrames = 11025u;
    source.samples.resize(sliceFrames * 4u * 2u);
    const std::array<double, 4> frequencies{220.0, 330.0, 440.0, 550.0};
    for (std::size_t slice = 0; slice < frequencies.size(); ++slice) {
        for (std::size_t frame = 0; frame < sliceFrames; ++frame) {
            const double phase = 2.0 * 3.14159265358979323846 * frequencies[slice]
                * static_cast<double>(frame) / source.sampleRate;
            const float sample = static_cast<float>(0.35 * std::sin(phase));
            const std::size_t index = (slice * sliceFrames + frame) * 2u;
            source.samples[index] = sample;
            source.samples[index + 1u] = sample;
        }
    }

    p.target = ventu::Target::Melody;
    p.bpm = 120;
    p.bars = 1;
    p.sourceMixPercent = 100;
    p.style = ventu::Style::BillionaireGoldenEra;
    const auto crateOrder = ventu::render(p, 0, {}, &source);
    const auto repeatedCrateOrder = ventu::render(p, 0, {}, &source);
    require(crateOrder.samples == repeatedCrateOrder.samples,
            "crate reassembly must be deterministic");
    p.style = ventu::Style::EastCoast;
    const auto sequentialOrder = ventu::render(p, 0, {}, &source);
    require(crateOrder.samples != sequentialOrder.samples,
            "Billionaire Golden Era must reassemble four source slices in 0-2-1-3 order");

    p.style = ventu::Style::BillionaireGoldenEra;
    const auto packDirectory = std::filesystem::temp_directory_path() / "ventusample-bge-pack";
    std::error_code cleanupError;
    std::filesystem::remove_all(packDirectory, cleanupError);
    const auto pack = ventu::generatePack(p, packDirectory);
    require(static_cast<bool>(pack), "Billionaire Golden Era pack export should succeed");
    std::ifstream manifest(packDirectory / "ventusample_manifest.json", std::ios::binary);
    const std::string manifestText((std::istreambuf_iterator<char>(manifest)),
                                   std::istreambuf_iterator<char>());
    require(manifestText.find("Billionaire Golden Era") != std::string::npos,
            "exported manifest must retain the shared Billionaire Golden Era profile name");
    std::filesystem::remove_all(packDirectory, cleanupError);
}

void testStereoMetering() {
    ventu::AudioBuffer audio;
    audio.sampleRate = 48000;
    audio.channels = 2;
    audio.samples = {0.5F, 0.5F, -0.5F, -0.5F, 0.25F, 0.25F};
    auto stats = ventu::measure(audio);
    require(stats.stereoCorrelationValid, "stereo correlation should be available for stereo buffers");
    require(std::abs(stats.stereoCorrelation - 1.0) < 1e-9,
            "identical left and right channels should correlate at +1");
    require(std::abs(stats.leftRms - stats.rightRms) < 1e-9,
            "identical channels should have matching RMS");

    for (std::size_t index = 1; index < audio.samples.size(); index += 2u)
        audio.samples[index] = -audio.samples[index];
    stats = ventu::measure(audio);
    require(std::abs(stats.stereoCorrelation + 1.0) < 1e-9,
            "polarity-inverted channels should correlate at -1");
}

void testTpdfDitherIsDeterministic() {
    const auto temp = std::filesystem::temp_directory_path() / "ventusample-dither-tests";
    std::error_code ec;
    std::filesystem::remove_all(temp, ec);
    std::filesystem::create_directories(temp, ec);
    require(!ec, "dither test folder should be created");

    ventu::AudioBuffer audio;
    audio.sampleRate = 48000;
    audio.channels = 2;
    audio.samples.reserve(4096);
    for (std::size_t index = 0; index < 2048u; ++index) {
        const float sample = static_cast<float>(0.00002 * std::sin(
            2.0 * 3.14159265358979323846 * static_cast<double>(index) / 67.0));
        audio.samples.push_back(sample);
        audio.samples.push_back(sample * 0.73F);
    }

    std::string error;
    const auto plainA = temp / "plain-a.wav";
    const auto plainB = temp / "plain-b.wav";
    const auto ditherA = temp / "dither-a.wav";
    const auto ditherB = temp / "dither-b.wav";
    const auto ditherOther = temp / "dither-other.wav";
    require(ventu::writeWave(plainA, audio, error), "default PCM16 export should succeed");
    require(ventu::writeWave(plainB, audio, error, {false, 123u}),
            "explicitly disabled dither export should succeed");
    require(ventu::writeWave(ditherA, audio, error, {true, 12345u}),
            "TPDF export should succeed");
    require(ventu::writeWave(ditherB, audio, error, {true, 12345u}),
            "repeated TPDF export should succeed");
    require(ventu::writeWave(ditherOther, audio, error, {true, 54321u}),
            "TPDF export with a different seed should succeed");

    const auto readBytes = [](const std::filesystem::path& path) {
        std::ifstream stream(path, std::ios::binary);
        return std::vector<char>(std::istreambuf_iterator<char>(stream),
                                 std::istreambuf_iterator<char>());
    };
    require(readBytes(plainA) == readBytes(plainB),
            "disabled dither must preserve the default PCM16 output bytes");
    require(readBytes(ditherA) == readBytes(ditherB),
            "the same dither seed must reproduce identical WAV bytes");
    require(readBytes(ditherA) != readBytes(ditherOther),
            "different dither seeds should produce distinct quantization noise");

    std::filesystem::remove_all(temp, ec);
}

void testPeakNormalizationIsOptInAndRecorded() {
    ventu::Parameters p;
    p.sampleRate = 22050;
    p.lowPassHz = 10000;
    p.bpm = 120;
    p.bars = 1;
    p.variations = 1;
    p.normalizePeak = true;
    p.targetPeakDbfs = -6.0;
    require(ventu::validate(p).empty(), "valid peak normalization options should pass validation");

    const auto output = std::filesystem::temp_directory_path() / "ventusample-normalize-test";
    std::error_code ec;
    std::filesystem::remove_all(output, ec);
    const auto result = ventu::generatePack(p, output);
    require(static_cast<bool>(result), "peak-normalized pack should export");
    require(result.files.size() == 1u, "normalization test should produce one WAV");

    std::string error;
    const auto audio = ventu::loadWave(result.files.front(), error);
    require(error.empty() && !audio.samples.empty(), "normalized WAV should reopen");
    const auto stats = ventu::measure(audio);
    const double targetPeak = std::pow(10.0, p.targetPeakDbfs / 20.0);
    require(std::abs(stats.peak - targetPeak) < 0.001,
            "exported sample peak should match the requested dBFS target");

    std::ifstream manifestFile(output / "ventusample_manifest.json", std::ios::binary);
    const std::string manifest((std::istreambuf_iterator<char>(manifestFile)),
                               std::istreambuf_iterator<char>());
    require(manifest.find("\"normalize_peak\": true") != std::string::npos,
            "manifest should record the normalization option");
    require(manifest.find("\"normalized\": true") != std::string::npos,
            "file metadata should confirm normalization was applied");
    require(manifest.find("\"target_peak_dbfs\": -6") != std::string::npos,
            "manifest should retain the exact target peak");

    p.targetPeakDbfs = -25.0;
    require(!ventu::validate(p).empty(), "normalization targets below -24 dBFS should be rejected");
    std::filesystem::remove_all(output, ec);
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
    require(batch.files.front().filename().string().find("_C_Minor_") != std::string::npos,
            "tonal loop filenames should include the selected root and minor mode");
    require(std::filesystem::exists(pack / "ventusample_manifest.json"), "sample pack should include a manifest");
    std::ifstream manifest(pack / "ventusample_manifest.json", std::ios::binary);
    const std::string json((std::istreambuf_iterator<char>(manifest)), std::istreambuf_iterator<char>());
    require(json.find("ventusample.pack.v2") != std::string::npos, "manifest should use the v2 schema id");
    require(json.find("\"musical_key\": \"C Minor\"") != std::string::npos,
            "manifest should export a readable musical key label");
    require(json.find("\"mode_family\": \"minor\"") != std::string::npos,
            "manifest should classify the selected scale as minor");
    require(json.find("East Coast") != std::string::npos, "manifest should record the selected style");
    require(json.find("single.wav") != std::string::npos, "manifest should identify the imported source by basename");
    require(json.find("density_percent") != std::string::npos, "manifest should include the full parameter snapshot");
    require(json.find("variations") != std::string::npos, "manifest should include the variation count");
    require(json.find("bus_compression_ratio") != std::string::npos, "manifest should include bus compression settings");
    require(json.find("audio_format") != std::string::npos, "manifest should declare its output format");
    require(json.find("crest_db") != std::string::npos, "manifest should include crest measurements");
    auto stochasticParameters = p;
    stochasticParameters.variations = 1;
    stochasticParameters.stochasticEnabled = true;
    stochasticParameters.stochasticIntensityPercent = 55;
    stochasticParameters.stochasticTriggerMode = 0;
    const auto stochasticPack = ventu::generatePack(stochasticParameters, temp / "stochastic-pack");
    require(static_cast<bool>(stochasticPack), "stochastic-enabled export should succeed");
    std::ifstream stochasticManifestFile(
        temp / "stochastic-pack" / "ventusample_manifest.json", std::ios::binary);
    const std::string stochasticManifest(
        (std::istreambuf_iterator<char>(stochasticManifestFile)),
        std::istreambuf_iterator<char>());
    require(stochasticManifest.find("\"stochastic_enabled\": true") != std::string::npos,
            "manifest must record that stochastic routing was enabled");
    require(stochasticManifest.find("\"stochastic_intensity_percent\": 55") != std::string::npos,
            "manifest must record stochastic intensity");
    require(stochasticManifest.find("\"stochastic_trigger_mode\": \"file\"") != std::string::npos,
            "manifest must record the trigger mode");
    require(stochasticManifest.find("\"stochastic_routes\": [") != std::string::npos,
            "each exported file must include its selected route list");
    std::string stochasticReadError;
    const auto baselineAudio = ventu::loadWave(batch.files.front(), stochasticReadError);
    require(stochasticReadError.empty(), "baseline WAV should remain readable");
    const auto stochasticAudio = ventu::loadWave(stochasticPack.files.front(), stochasticReadError);
    require(stochasticReadError.empty(), "stochastic WAV should remain readable");
    require(stochasticAudio.samples != baselineAudio.samples,
            "stochastic routing must alter the actual exported audio samples");

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
    bool hasUnpitchedKick = false;
    bool hasKeyedBass = false;
    for (const auto& filePath : kit.files) {
        const auto filename = filePath.filename().string();
        if (filename.find("_v1_kick.wav") != std::string::npos &&
            filename.find("_C_") == std::string::npos) hasUnpitchedKick = true;
        if (filename.find("_C_v1_bass.wav") != std::string::npos) hasKeyedBass = true;
    }
    require(hasUnpitchedKick, "unpitched drum one-shots must not receive a false key tag");
    require(hasKeyedBass, "pitched one-shots should carry their note name in the filename");

    auto majorParameters = p;
    majorParameters.target = ventu::Target::Melody;
    majorParameters.variations = 1;
    majorParameters.rootNote = 9;
    majorParameters.scale = ventu::Scale::Major;
    const auto majorPack = ventu::generatePack(majorParameters, temp / "major");
    require(static_cast<bool>(majorPack), "major-key sample export should succeed");
    require(majorPack.files.front().filename().string().find("_A_Major_") != std::string::npos,
            "major-key export filenames should include the major label");
    // Keep the deterministic pack fixture for the independent schema-shape test.
}
}

int main() {
    testValidation();
    testDeterministicRender();
    testPhraseGridAndSwing();
    testCadenceGridAudit();
    testStereoMetering();
    testTpdfDitherIsDeterministic();
    testPeakNormalizationIsOptInAndRecorded();
    testBillionaireGoldenEraProfile();
    testWaveAndPackExport();
    std::cout << "VentuSample generator tests passed.\n";
    return 0;
}
