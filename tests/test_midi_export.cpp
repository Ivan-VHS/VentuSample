#include "generator.hpp"
#include "midi_export.hpp"

#include <chrono>
#include <cstdlib>
#include <cstdint>
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

std::vector<std::uint8_t> readBytes(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void testStandardMidiExport() {
    const auto directory = std::filesystem::temp_directory_path()
        / ("ventusample-midi-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(directory);
    const auto cleanup = [&directory] {
        std::error_code ignored;
        std::filesystem::remove_all(directory, ignored);
    };

    ventu::Parameters parameters;
    parameters.bpm = 120;
    parameters.bars = 1;
    parameters.variations = 1;
    parameters.sampleRate = 22050;
    parameters.lowPassHz = 10000;
    parameters.microHumanize = false;
    const auto audio = ventu::render(parameters, 0);
    require(!audio.midiNotes.empty(), "renderer should capture note events for MIDI");

    std::string error;
    const auto path = directory / "loop.mid";
    require(ventu::writeMidiFile(path, parameters, audio, error), "valid render should export to MIDI");
    const auto bytes = readBytes(path);
    require(bytes.size() > 100u, "MIDI file should contain tracks and events");
    require(std::string(bytes.begin(), bytes.begin() + 4) == "MThd", "file should start with MThd");
    require(bytes[8] == 0u && bytes[9] == 1u, "file should use Standard MIDI File type 1");
    require(bytes[10] == 0u && bytes[11] == 4u, "file should contain conductor, drums, bass and melody tracks");
    require(bytes[12] == 0x01u && bytes[13] == 0xE0u, "file should use 480 PPQ");
    const std::string binary(bytes.begin(), bytes.end());
    require(binary.find("MTrk") != std::string::npos, "file should contain MIDI track chunks");
    require(binary.find(std::string("\x99", 1)) != std::string::npos,
            "drum channel note events should be present");
    require(binary.find(std::string("\x90", 1)) != std::string::npos,
            "bass channel note events should be present");
    require(binary.find(std::string("\x91", 1)) != std::string::npos,
            "melody channel note events should be present");

    require(!ventu::writeMidiFile(path, parameters, audio, error),
            "export should refuse to overwrite an existing file");
    cleanup();
}

void testOneShotKitMidiArrangement() {
    const auto directory = std::filesystem::temp_directory_path()
        / ("ventusample-midi-kit-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(directory);
    ventu::Parameters parameters;
    parameters.target = ventu::Target::OneShotKit;
    parameters.bpm = 100;
    parameters.rootNote = 5;
    std::string error;
    const auto path = directory / "kit.mid";
    require(ventu::writeMidiFile(path, parameters, {}, error),
            "one-shot target should export a documented MIDI kit arrangement");
    const auto bytes = readBytes(path);
    require(bytes.size() > 80u, "one-shot arrangement should contain MIDI notes");
    std::error_code ignored;
    std::filesystem::remove_all(directory, ignored);
}
}

int main() {
    testStandardMidiExport();
    testOneShotKitMidiArrangement();
    std::cout << "MIDI export tests passed.\n";
    return 0;
}
