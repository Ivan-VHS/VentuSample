#pragma once

#include "generator.hpp"

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace ventu {

struct PluginDescriptor {
    std::string id;
    std::string name;
    std::string domain;
    std::string version;
    std::string description;
    std::string operation;
    std::vector<std::string> capabilities;
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

using PluginHandler = std::function<PluginResult(const PluginContext&)>;

class PluginManager final {
public:
    PluginManager();

    bool registerPlugin(PluginDescriptor descriptor, PluginHandler handler,
                        bool enabledByDefault = true);
    std::vector<PluginDescriptor> plugins() const;
    const PluginDescriptor* find(const std::string& idOrName) const noexcept;
    bool isEnabled(const std::string& idOrName) const noexcept;
    bool setEnabled(const std::string& idOrName, bool enabled) noexcept;
    PluginResult execute(const std::string& idOrName, const PluginContext& context) const;

private:
    struct Entry {
        PluginDescriptor descriptor;
        PluginHandler handler;
        bool enabled{true};
    };
    std::vector<Entry> entries_;
};

} // namespace ventu
