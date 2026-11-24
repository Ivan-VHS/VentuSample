#pragma once

#include "generator.hpp"

#include <cstddef>
#include <cstdint>
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

struct PluginExecutionPolicy {
    // The generator remains offline and deterministic; these are host-side
    // admission/observation limits, not an OS sandbox or forced cancellation.
    std::uint64_t maxEstimatedOutputBytes{512u * 1024u * 1024u};
    std::uint64_t softDeadlineMs{120'000u};
    bool requireDeterministicSeed{true};
};

struct PluginContext {
    Parameters parameters;
    std::filesystem::path workspaceRoot;
    std::filesystem::path outputDirectory;
    std::string correlationId;
    PluginExecutionPolicy executionPolicy;
};

struct PluginResult {
    bool success{false};
    std::string summary;
    std::vector<std::string> details;
    std::string executionId;
    std::uint64_t durationMs{0};
    std::uint64_t estimatedOutputBytes{0};
    bool deadlineExceeded{false};
};

struct PluginStatus {
    PluginDescriptor descriptor;
    bool enabled{false};
    bool initialized{false};
    bool handlerAttached{false};
    std::string lastError;
    std::uint64_t invocations{0};
    std::uint64_t successes{0};
    std::uint64_t failures{0};
    std::uint64_t deadlineExceeded{0};
    std::uint64_t lastDurationMs{0};
    std::uint64_t maxDurationMs{0};
    std::string lastExecutionId;
    std::string lastStatus{"never_run"};
};

struct PluginHealth {
    std::size_t registered{0};
    std::size_t enabled{0};
    std::size_t initialized{0};
    std::size_t handlerAttached{0};
    bool ready{false};
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
    PluginHealth health() const noexcept;
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
        std::uint64_t invocations{0};
        std::uint64_t successes{0};
        std::uint64_t failures{0};
        std::uint64_t deadlineExceeded{0};
        std::uint64_t lastDurationMs{0};
        std::uint64_t maxDurationMs{0};
        std::string lastExecutionId;
        std::string lastStatus{"never_run"};
    };
    std::vector<Entry> entries_;
};

} // namespace ventu
