#pragma once

#include "generator.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace ventu::cineforge {

struct Settings {
    std::string scene{"Dark industrial thriller / monumental metal and sub impacts"};
    int bpm{92};
    int rootNote{5}; // F
    int sampleRate{48000};
    int packSize{8};
    int seed{24011};
    int mood{72};
    int space{58};
    int punch{78};
    int variations{3};
};

struct Asset {
    std::string name;
    std::filesystem::path dryPath;
    std::filesystem::path wetPath;
    AudioStats stats;
};

struct Result {
    std::vector<Asset> assets;
    std::filesystem::path previewPath;
    std::filesystem::path manifestPath;
    std::string error;
    explicit operator bool() const noexcept { return error.empty() && !assets.empty(); }
};

std::string validate(const Settings& settings);
Result generate(const Settings& settings, const std::filesystem::path& outputDirectory);

} // namespace ventu::cineforge
