#pragma once

#include "generator.hpp"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace ventu {

inline constexpr unsigned int kPluginApiVersion = 1u;
inline constexpr const char* kPluginManifestSchema = "ivan-vhs.plugin.v1";

struct PluginDescriptor {
    std::string id;
    std::string name;
    std::string domain;
    std::string version;
    std::string description;
    std::string operation;
    std::vector<std::string> capabilities;
    unsigned int apiVersion{kPluginApiVersion};
    std::string source{"builtin"};
    std::string manifestPath;
    std::string entrypoint;
};

struct PluginContext {
    Parameters parameters;
    std::filesystem::path workspaceRoot;
    std::filesystem::path outputDirectory;
};

struct PluginResult {
    bool success{false};
    std::string summary;
    std::vector<std::string> details;
};

struct PluginStatus {
    PluginDescriptor descriptor;
    bool enabled{false};
    bool initialized{false};
    bool handlerAttached{false};
    std::string lastError;
};

using PluginHandler = std::function<PluginResult(const PluginContext&)>;
using PluginLifecycle = std::function<void(const PluginContext&)>;

class PluginManager final {
public:
    PluginManager();
    ~PluginManager();

    bool registerPlugin(PluginDescriptor descriptor, PluginHandler handler,
                        bool enabledByDefault = true,
                        PluginLifecycle onInitialize = {},
                        PluginLifecycle onShutdown = {});
    // Manifest discovery only registers metadata. It never loads or executes native code.
    std::size_t discoverManifests(const std::filesystem::path& directory,
                                  std::vector<std::string>* diagnostics = nullptr);
    bool attachHandler(const std::string& idOrName, PluginHandler handler,
                       PluginLifecycle onInitialize = {},
                       PluginLifecycle onShutdown = {});
    // Replaces an attached in-memory handler only while the plugin is disabled and stopped.
    // This never loads a manifest entrypoint, DLL, SO or other executable payload.
    bool hotSwapHandler(const std::string& idOrName, PluginHandler handler,
                        PluginLifecycle onInitialize = {},
                        PluginLifecycle onShutdown = {});
    std::vector<PluginDescriptor> plugins() const;
    std::vector<PluginStatus> statuses() const;
    const PluginDescriptor* find(const std::string& idOrName) const noexcept;
    bool isEnabled(const std::string& idOrName) const noexcept;
    bool setEnabled(const std::string& idOrName, bool enabled) noexcept;
    bool initializePlugin(const std::string& idOrName, const PluginContext& context) noexcept;
    bool shutdownPlugin(const std::string& idOrName) noexcept;
    PluginResult execute(const std::string& idOrName, const PluginContext& context);

private:
    struct Entry {
        PluginDescriptor descriptor;
        PluginHandler handler;
        PluginLifecycle onInitialize;
        PluginLifecycle onShutdown;
        bool enabled{true};
        bool initialized{false};
        std::string lastError;
        PluginContext lifecycleContext;
    };
    std::vector<Entry> entries_;
};

} // namespace ventu
