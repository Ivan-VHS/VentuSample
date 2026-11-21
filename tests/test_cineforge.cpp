#include "cineforge.hpp"

#include <cstdint>
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

std::vector<unsigned char> readBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

std::uint16_t read16(const std::vector<unsigned char>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset])
        | static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[offset + 1u]) << 8u);
}

void testFormatsAndDeterminism() {
    const auto root = std::filesystem::temp_directory_path() / "ventu-cineforge-tests";
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    std::filesystem::create_directories(root / "a", ec);
    std::filesystem::create_directories(root / "b", ec);

    ventu::CineForgeSettings settings;
    settings.scenePrompt = "Industrial thriller / sublevel chase";
    settings.bpm = 92;
    settings.rootNote = 5;
    settings.bars = 2;
    settings.packSize = 8;
    settings.seed = 123456u;
    settings.sampleRate = 48000;
    settings.outputDirectory = root / "a";

    ventu::CineForgeResult first;
    require(ventu::generateCineForgePack(settings, first), "PCM24 pack must generate");
    require(first.success && first.assets.size() == 8u, "pack must contain eight assets");
    require(first.assets.front().files.size() == 5u, "each asset must export five layers");
    require(std::filesystem::exists(first.manifestPath), "manifest must exist");
    require(std::filesystem::exists(first.previewPath), "preview must exist");

    const auto dryBytes = readBytes(first.assets.front().files.front());
    require(dryBytes.size() > 44u, "WAV must contain audio payload");
    require(std::string(dryBytes.begin(), dryBytes.begin() + 4) == "RIFF", "WAV must use RIFF container");
    require(read16(dryBytes, 34u) == 24u, "PCM output must be 24-bit");

    settings.outputDirectory = root / "b";
    ventu::CineForgeResult second;
    require(ventu::generateCineForgePack(settings, second), "same seed must generate another pack");
    require(readBytes(second.assets.front().files.front()) == dryBytes,
            "same seed and settings must produce byte-identical WAV output");

    settings.encoding = ventu::CineForgeEncoding::Float32;
    settings.seed += 1u;
    ventu::CineForgeResult floating;
    require(ventu::generateCineForgePack(settings, floating), "float32 pack must generate");
    const auto floatBytes = readBytes(floating.assets.front().files.front());
    require(read16(floatBytes, 34u) == 32u, "float output must be 32-bit");
    require(read16(floatBytes, 20u) == 3u, "float output must declare IEEE float format");
    std::filesystem::remove_all(root, ec);
}

void testValidationAndNoOverwrite() {
    const auto root = std::filesystem::temp_directory_path() / "ventu-cineforge-invalid";
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    std::filesystem::create_directories(root, ec);
    ventu::CineForgeSettings settings;
    settings.outputDirectory = root;
    settings.packSize = 7;
    ventu::CineForgeResult result;
    require(!ventu::generateCineForgePack(settings, result), "invalid pack size must fail");
    settings.packSize = 8;
    require(ventu::generateCineForgePack(settings, result), "valid pack must generate");
    ventu::CineForgeResult duplicate;
    require(!ventu::generateCineForgePack(settings, duplicate),
            "existing pack folder must never be overwritten");
    std::filesystem::remove_all(root, ec);
}
}

int main() {
    testFormatsAndDeterminism();
    testValidationAndNoOverwrite();
    std::cout << "CINEFORGE procedural pack tests passed.\n";
}
