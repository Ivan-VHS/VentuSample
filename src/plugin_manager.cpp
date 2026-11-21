#include "plugin_manager.hpp"

#include <algorithm>
#include <cstdint>
#include <cctype>
#include <exception>
#include <fstream>
#include <iterator>
#include <optional>
#include <regex>
#include <system_error>
#include <utility>

namespace ventu {
void registerBuiltinPlugins(PluginManager& manager);

namespace {
namespace fs = std::filesystem;
constexpr std::uintmax_t kMaxManifestBytes = 64u * 1024u;
constexpr std::size_t kMaxManifests = 256u;

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
    if (id.front() == '-' || id.back() == '-') return false;
    bool previousDash = false;
    return std::all_of(id.begin(), id.end(), [&](unsigned char ch) {
        const bool dash = ch == '-';
        const bool valid = (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || dash;
        if (!valid || (dash && previousDash)) return false;
        previousDash = dash;
        return true;
    });
}

bool validVersion(const std::string& version) {
    static const std::regex pattern(
        R"rx(^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)(-[0-9A-Za-z.-]+)?(\+[0-9A-Za-z.-]+)?$)rx");
    return std::regex_match(version, pattern);
}

// The native core accepts the portable manifest's plain-string subset. Escaped JSON
// strings and executable entrypoints are intentionally not interpreted by this loader.
std::optional<std::string> jsonString(const std::string& json, const std::string& key) {
    const std::regex pattern("\"" + key + R"rx("\s*:\s*"([^"\\]*)")rx");
    std::smatch match;
    if (!std::regex_search(json, match, pattern)) return std::nullopt;
    return match[1].str();
}

std::vector<std::string> jsonStringArray(const std::string& json, const std::string& key) {
    const std::regex arrayPattern("\"" + key + R"rx("\s*:\s*\[([^\]]*)\])rx");
    std::smatch arrayMatch;
    if (!std::regex_search(json, arrayMatch, arrayPattern)) return {};
    const std::string body = arrayMatch[1].str();
    const std::regex itemPattern(R"rx("([^"\\]+)")rx");
    std::vector<std::string> result;
    for (std::sregex_iterator it(body.begin(), body.end(), itemPattern), end; it != end; ++it)
        result.push_back((*it)[1].str());
    return result;
}

bool validVentuHost(const std::string& json) {
    const std::regex pattern(R"rx("host"\s*:\s*\{[^}]*"id"\s*:\s*"ventu")rx");
    return std::regex_search(json, pattern);
}

bool readFileBounded(const fs::path& path, std::string& output) {
    std::error_code ec;
    const auto size = fs::file_size(path, ec);
    if (ec || size > kMaxManifestBytes) return false;
    std::ifstream input(path, std::ios::binary);
    if (!input) return false;
    output.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    return input.good() || input.eof();
}
} // namespace

PluginManager::PluginManager() {
    registerBuiltinPlugins(*this);
}

PluginManager::~PluginManager() {
    for (auto& entry : entries_) {
        if (!entry.initialized) continue;
        try {
            if (entry.onShutdown) entry.onShutdown(entry.lifecycleContext);
        } catch (...) {
            // Destruction must remain noexcept; explicit shutdown exposes failures.
        }
        entry.initialized = false;
    }
}

bool PluginManager::registerPlugin(PluginDescriptor descriptor, PluginHandler handler,
                                   bool enabledByDefault, PluginLifecycle onInitialize,
                                   PluginLifecycle onShutdown) {
    if (!validId(descriptor.id) || descriptor.name.empty()
        || !validVersion(descriptor.version) || descriptor.apiVersion != kPluginApiVersion
        || descriptor.operation.empty() || !handler) return false;
    const std::string id = normalized(descriptor.id);
    const bool duplicate = std::any_of(entries_.begin(), entries_.end(), [&](const Entry& entry) {
        return normalized(entry.descriptor.id) == id
            || normalized(entry.descriptor.name) == normalized(descriptor.name);
    });
    if (duplicate) return false;
    descriptor.id = id;
    descriptor.source = descriptor.source.empty() ? "sdk" : descriptor.source;
    Entry entry;
    entry.descriptor = std::move(descriptor);
    entry.handler = std::move(handler);
    entry.onInitialize = std::move(onInitialize);
    entry.onShutdown = std::move(onShutdown);
    entry.enabled = enabledByDefault;
    entries_.push_back(std::move(entry));
    return true;
}

std::size_t PluginManager::discoverManifests(const fs::path& directory,
                                             std::vector<std::string>* diagnostics) {
    std::error_code ec;
    if (directory.empty() || !fs::is_directory(directory, ec) || ec) {
        if (diagnostics) diagnostics->push_back("plugin directory is unavailable: " + directory.string());
        return 0u;
    }

    std::vector<fs::path> manifests;
    fs::recursive_directory_iterator it(
        directory, fs::directory_options::skip_permission_denied, ec), end;
    while (!ec && it != end && manifests.size() < kMaxManifests) {
        const auto path = it->path();
        std::error_code itemError;
        if (it->is_symlink(itemError)) {
            if (it->is_directory(itemError)) it.disable_recursion_pending();
        } else if (!itemError && it->is_regular_file(itemError)
                   && path.filename() == "plugin.json") {
            manifests.push_back(path);
        }
        it.increment(ec);
        if (ec) ec.clear();
    }
    std::sort(manifests.begin(), manifests.end());

    std::size_t discovered = 0u;
    for (const auto& path : manifests) {
        std::string json;
        if (!readFileBounded(path, json)) {
            if (diagnostics) diagnostics->push_back("manifest unreadable or larger than 64 KiB: " + path.string());
            continue;
        }
        const auto schema = jsonString(json, "schema");
        const auto id = jsonString(json, "id");
        const auto name = jsonString(json, "name");
        const auto version = jsonString(json, "version");
        const auto domain = jsonString(json, "domain");
        const auto description = jsonString(json, "description");
        const auto entrypoint = jsonString(json, "entrypoint");
        const std::regex apiPattern(R"rx("api_version"\s*:\s*([0-9]+))rx");
        std::smatch apiMatch;
        if (!schema || *schema != kPluginManifestSchema || !id || !name || !version
            || !domain || !description || !entrypoint || !validVentuHost(json)
            || !std::regex_search(json, apiMatch, apiPattern)) {
            if (diagnostics) diagnostics->push_back("manifest does not satisfy the Ventu v1 contract: " + path.string());
            continue;
        }

        unsigned int apiVersion = 0u;
        try {
            apiVersion = static_cast<unsigned int>(std::stoul(apiMatch[1].str()));
        } catch (const std::exception&) {
            if (diagnostics) diagnostics->push_back("invalid api_version: " + path.string());
            continue;
        }
        const auto capabilities = jsonStringArray(json, "capabilities");
        if (!validId(*id) || name->empty() || domain->empty() || description->empty()
            || !validVersion(*version) || apiVersion != kPluginApiVersion
            || capabilities.size() > 32u) {
            if (diagnostics) diagnostics->push_back("invalid ID, version, API or capabilities: " + path.string());
            continue;
        }
        const bool duplicate = std::any_of(entries_.begin(), entries_.end(), [&](const Entry& entry) {
            return normalized(entry.descriptor.id) == normalized(*id)
                || normalized(entry.descriptor.name) == normalized(*name);
        });
        if (duplicate) {
            if (diagnostics) diagnostics->push_back("duplicate plugin identity: " + *id);
            continue;
        }

        PluginDescriptor descriptor;
        descriptor.id = normalized(*id);
        descriptor.name = *name;
        descriptor.domain = *domain;
        descriptor.version = *version;
        descriptor.description = *description;
        descriptor.operation = "manifest-only";
        descriptor.capabilities = capabilities;
        descriptor.apiVersion = apiVersion;
        descriptor.source = "manifest";
        descriptor.manifestPath = path.string();
        descriptor.entrypoint = *entrypoint;

        Entry entry;
        entry.descriptor = std::move(descriptor);
        entry.enabled = false;
        entries_.push_back(std::move(entry));
        ++discovered;
    }
    return discovered;
}

bool PluginManager::attachHandler(const std::string& idOrName, PluginHandler handler,
                                  PluginLifecycle onInitialize, PluginLifecycle onShutdown) {
    if (!handler) return false;
    const auto key = normalized(idOrName);
    const auto found = std::find_if(entries_.begin(), entries_.end(), [&](const Entry& entry) {
        return normalized(entry.descriptor.id) == key
            || normalized(entry.descriptor.name) == key;
    });
    if (found == entries_.end() || found->handler) return false;
    found->handler = std::move(handler);
    found->onInitialize = std::move(onInitialize);
    found->onShutdown = std::move(onShutdown);
    found->lastError.clear();
    return true;
}

std::vector<PluginDescriptor> PluginManager::plugins() const {
    std::vector<PluginDescriptor> result;
    result.reserve(entries_.size());
    for (const auto& entry : entries_) result.push_back(entry.descriptor);
    return result;
}

std::vector<PluginStatus> PluginManager::statuses() const {
    std::vector<PluginStatus> result;
    result.reserve(entries_.size());
    for (const auto& entry : entries_) {
        result.push_back({entry.descriptor, entry.enabled, entry.initialized,
                          static_cast<bool>(entry.handler), entry.lastError});
    }
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
    if (enabled && !found->handler) return false;
    if (!enabled && found->initialized && !shutdownPlugin(idOrName)) return false;
    found->enabled = enabled;
    return true;
}

bool PluginManager::initializePlugin(const std::string& idOrName,
                                     const PluginContext& context) noexcept {
    const auto key = normalized(idOrName);
    const auto found = std::find_if(entries_.begin(), entries_.end(), [&](const Entry& entry) {
        return normalized(entry.descriptor.id) == key
            || normalized(entry.descriptor.name) == key;
    });
    if (found == entries_.end() || !found->enabled || !found->handler) return false;
    if (found->initialized) return true;
    try {
        found->lifecycleContext = context;
        if (found->onInitialize) found->onInitialize(context);
        found->initialized = true;
        found->lastError.clear();
        return true;
    } catch (const std::exception& error) {
        found->lastError = error.what();
    } catch (...) {
        found->lastError = "unknown initialization exception";
    }
    return false;
}

bool PluginManager::shutdownPlugin(const std::string& idOrName) noexcept {
    const auto key = normalized(idOrName);
    const auto found = std::find_if(entries_.begin(), entries_.end(), [&](const Entry& entry) {
        return normalized(entry.descriptor.id) == key
            || normalized(entry.descriptor.name) == key;
    });
    if (found == entries_.end()) return false;
    if (!found->initialized) return true;
    try {
        if (found->onShutdown) found->onShutdown(found->lifecycleContext);
        found->initialized = false;
        found->lastError.clear();
        return true;
    } catch (const std::exception& error) {
        found->lastError = error.what();
    } catch (...) {
        found->lastError = "unknown shutdown exception";
    }
    return false;
}

PluginResult PluginManager::execute(const std::string& idOrName,
                                    const PluginContext& context) {
    const auto key = normalized(idOrName);
    auto found = std::find_if(entries_.begin(), entries_.end(), [&](const Entry& entry) {
        return normalized(entry.descriptor.id) == key
            || normalized(entry.descriptor.name) == key;
    });
    if (found == entries_.end()) return {false, "Unknown plugin: " + idOrName, {}};
    if (!found->enabled) return {false, "Plugin is disabled: " + found->descriptor.name, {}};
    if (!found->handler) return {false, "Plugin is discovered but has no reviewed SDK handler: " + found->descriptor.name, {}};
    if (!found->initialized && !initializePlugin(idOrName, context)) {
        return {false, "Plugin initialization failed: " + found->descriptor.name,
                {found->lastError}};
    }
    try {
        return found->handler(context);
    } catch (const std::exception& error) {
        found->lastError = error.what();
        return {false, "Plugin failed: " + found->descriptor.name + ": " + error.what(), {}};
    } catch (...) {
        found->lastError = "unknown handler exception";
        return {false, "Plugin failed with an unknown exception: " + found->descriptor.name, {}};
    }
}

} // namespace ventu
