#include "plugin_manager.hpp"

#include <algorithm>
#include <cctype>
#include <exception>
#include <utility>

namespace ventu {
void registerBuiltinPlugins(PluginManager& manager);

namespace {
std::string normalized(const std::string& value) {
    std::string result;
    result.reserve(value.size());
    for (const unsigned char ch : value) {
        if (ch >= 'A' && ch <= 'Z') result.push_back(static_cast<char>(ch - 'A' + 'a'));
        else result.push_back(static_cast<char>(ch));
    }
    return result;
}

bool validId(const std::string& id) {
    if (id.empty() || id.size() > 80u) return false;
    return std::all_of(id.begin(), id.end(), [](unsigned char ch) {
        return (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '-';
    });
}
}

PluginManager::PluginManager() {
    registerBuiltinPlugins(*this);
}

bool PluginManager::registerPlugin(PluginDescriptor descriptor, PluginHandler handler,
                                   bool enabledByDefault) {
    if (!validId(descriptor.id) || descriptor.name.empty() || descriptor.version.empty()
        || descriptor.operation.empty() || !handler) return false;
    const std::string id = normalized(descriptor.id);
    const bool duplicate = std::any_of(entries_.begin(), entries_.end(), [&](const Entry& entry) {
        return normalized(entry.descriptor.id) == id
            || normalized(entry.descriptor.name) == normalized(descriptor.name);
    });
    if (duplicate) return false;
    descriptor.id = id;
    entries_.push_back({std::move(descriptor), std::move(handler), enabledByDefault});
    return true;
}

std::vector<PluginDescriptor> PluginManager::plugins() const {
    std::vector<PluginDescriptor> result;
    result.reserve(entries_.size());
    for (const auto& entry : entries_) result.push_back(entry.descriptor);
    return result;
}

const PluginDescriptor* PluginManager::find(const std::string& idOrName) const noexcept {
    const auto key = normalized(idOrName);
    const auto found = std::find_if(entries_.begin(), entries_.end(), [&](const Entry& entry) {
        return normalized(entry.descriptor.id) == key
            || normalized(entry.descriptor.name) == key;
    });
    return found == entries_.end() ? nullptr : &found->descriptor;
}

bool PluginManager::isEnabled(const std::string& idOrName) const noexcept {
    const auto key = normalized(idOrName);
    const auto found = std::find_if(entries_.begin(), entries_.end(), [&](const Entry& entry) {
        return normalized(entry.descriptor.id) == key
            || normalized(entry.descriptor.name) == key;
    });
    return found != entries_.end() && found->enabled;
}

bool PluginManager::setEnabled(const std::string& idOrName, bool enabled) noexcept {
    const auto key = normalized(idOrName);
    const auto found = std::find_if(entries_.begin(), entries_.end(), [&](const Entry& entry) {
        return normalized(entry.descriptor.id) == key
            || normalized(entry.descriptor.name) == key;
    });
    if (found == entries_.end()) return false;
    found->enabled = enabled;
    return true;
}

PluginResult PluginManager::execute(const std::string& idOrName,
                                    const PluginContext& context) const {
    const auto key = normalized(idOrName);
    const auto found = std::find_if(entries_.begin(), entries_.end(), [&](const Entry& entry) {
        return normalized(entry.descriptor.id) == key
            || normalized(entry.descriptor.name) == key;
    });
    if (found == entries_.end()) return {false, "Unknown plugin: " + idOrName, {}};
    if (!found->enabled) return {false, "Plugin is disabled: " + found->descriptor.name, {}};
    try {
        return found->handler(context);
    } catch (const std::exception& error) {
        return {false, "Plugin failed: " + found->descriptor.name + ": " + error.what(), {}};
    } catch (...) {
        return {false, "Plugin failed with an unknown exception: " + found->descriptor.name, {}};
    }
}

} // namespace ventu
