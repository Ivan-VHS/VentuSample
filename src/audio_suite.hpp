#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace ventu {

struct AudioBuffer {
    int sampleRate{44100};
    std::vector<float> left;
    std::vector<float> right;

    std::size_t frames() const noexcept { return left.size(); }
    bool valid() const noexcept;
};

struct AudioPluginDescriptor {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    std::string interfaceConcept;
    std::string version{"1.0.0"};
};

struct AudioPluginParameters {
    float amount{0.55f};
    float wet{0.35f};
    float rate{0.25f};
    float tone{0.55f};
    float width{0.50f};
};

struct AudioPluginChainItem {
    std::string pluginId;
    AudioPluginParameters parameters;
};

const std::vector<AudioPluginDescriptor>& audioPluginCatalog();
const AudioPluginDescriptor* findAudioPlugin(const std::string& idOrName) noexcept;

// All effects use deterministic, offline stereo float DSP. The caller owns the buffer.
bool processAudioPlugin(AudioBuffer& buffer, const std::string& idOrName,
                        const AudioPluginParameters& parameters, std::string& error);
bool processAudioChain(AudioBuffer& buffer, const std::vector<AudioPluginChainItem>& chain,
                       std::string& error);

// Bounded PCM16 RIFF/WAVE I/O. Mono files are expanded to dual-mono internally.
bool loadPcm16Wav(const std::filesystem::path& path, AudioBuffer& buffer, std::string& error);
bool savePcm16Wav(const std::filesystem::path& path, const AudioBuffer& buffer,
                  std::string& error);

} // namespace ventu
