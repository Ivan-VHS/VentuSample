#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ventu {

enum class CineForgeEncoding { Pcm24, Float32 };

struct CineForgeSettings {
    std::string scenePrompt{"Dark industrial thriller"};
    int bpm{92};
    int rootNote{5};
    int bars{4};
    int packSize{10};
    std::uint64_t seed{920501};
    int sampleRate{48000};
    CineForgeEncoding encoding{CineForgeEncoding::Pcm24};
    std::filesystem::path outputDirectory;
};

struct CineForgeAssetResult {
    std::string name;
    std::vector<std::filesystem::path> files;
};

struct CineForgeResult {
    bool success{false};
    std::filesystem::path packDirectory;
    std::filesystem::path manifestPath;
    std::filesystem::path previewPath;
    std::vector<CineForgeAssetResult> assets;
    std::string error;
};

// Deterministic offline scene-pack synthesis. Each object exports Dry, Wet,
// Attack, Body and Tail stereo WAV files, plus a preview and JSON manifest.
bool generateCineForgePack(const CineForgeSettings& settings, CineForgeResult& result);

} // namespace ventu
