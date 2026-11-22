#pragma once

#include "generator.hpp"

#include <filesystem>
#include <string>

namespace ventu {

// Writes a Standard MIDI File type 1 containing renderer-derived notes.
// The export is a musical arrangement, not an audio transcription.
bool writeMidiFile(const std::filesystem::path& path, const Parameters& parameters,
                   const AudioBuffer& audio, std::string& error);

} // namespace ventu
