#include "plugin_manager.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <cctype>
#include <exception>
#include <fstream>
#include <iterator>
#include <optional>
#include <regex>
#include <string_view>
#include <system_error>
#include <utility>
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

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

struct JsonSpan {
    std::size_t begin{};
    std::size_t end{};
};

void skipJsonWhitespace(const std::string& json, std::size_t& position) {
    while (position < json.size()
           && (json[position] == ' ' || json[position] == '\t'
               || json[position] == '\r' || json[position] == '\n'))
        ++position;
}

/* The portable plugin-manifest contract accepts plain UTF-8 strings only. */
bool readPlainJsonString(const std::string& json, std::size_t& position,
                         std::string* value = nullptr) {
    if (position >= json.size() || json[position] != '"') return false;
    const std::size_t begin = ++position;
    while (position < json.size()) {
        const unsigned char ch = static_cast<unsigned char>(json[position]);
        if (ch == '\\') return false;
        if (ch < 0x20u) return false;
        if (ch == '"') {
            if (value) *value = json.substr(begin, position - begin);
            ++position;
            return true;
        }
        ++position;
    }
    return false;
}

bool jsonHexDigit(char value) {
    return (value >= '0' && value <= '9')
        || (value >= 'a' && value <= 'f')
        || (value >= 'A' && value <= 'F');
}

bool skipJsonString(const std::string& json, std::size_t& position) {
    if (position >= json.size() || json[position] != '"') return false;
    ++position;
    while (position < json.size()) {
        const unsigned char ch = static_cast<unsigned char>(json[position]);
        if (ch == '\\') {
            ++position;
            if (position >= json.size()) return false;
            const char escape = json[position++];
            switch (escape) {
                case '"': case '\\': case '/':
                case 'b': case 'f': case 'n': case 'r': case 't':
                    break;
                case 'u': {
                    if (position + 4u > json.size()) return false;
                    unsigned int codeUnit = 0u;
                    for (unsigned int i = 0u; i < 4u; ++i) {
                        const char digit = json[position++];
                        if (!jsonHexDigit(digit)) return false;
                        codeUnit <<= 4u;
                        if (digit >= '0' && digit <= '9') codeUnit |= static_cast<unsigned int>(digit - '0');
                        else if (digit >= 'a' && digit <= 'f') codeUnit |= static_cast<unsigned int>(digit - 'a' + 10);
                        else codeUnit |= static_cast<unsigned int>(digit - 'A' + 10);
                    }
                    if (codeUnit >= 0xD800u && codeUnit <= 0xDBFFu) {
                        if (position + 6u > json.size()
                            || json[position] != '\\' || json[position + 1u] != 'u')
                            return false;
                        position += 2u;
                        unsigned int low = 0u;
                        for (unsigned int i = 0u; i < 4u; ++i) {
                            const char digit = json[position++];
                            if (!jsonHexDigit(digit)) return false;
                            low <<= 4u;
                            if (digit >= '0' && digit <= '9') low |= static_cast<unsigned int>(digit - '0');
                            else if (digit >= 'a' && digit <= 'f') low |= static_cast<unsigned int>(digit - 'a' + 10);
                            else low |= static_cast<unsigned int>(digit - 'A' + 10);
                        }
                        if (low < 0xDC00u || low > 0xDFFFu) return false;
                    } else if (codeUnit >= 0xDC00u && codeUnit <= 0xDFFFu) {
                        return false;
                    }
                    break;
                }
                default:
                    return false;
            }
            continue;
        }
        if (ch == '"') {
            ++position;
            return true;
        }
        if (ch < 0x20u) return false;
        ++position;
    }
    return false;
}

bool skipJsonNumber(const std::string& json, std::size_t& position) {
    const std::size_t begin = position;
    if (position < json.size() && json[position] == '-') ++position;
    if (position >= json.size()) return false;

    if (json[position] == '0') {
        ++position;
        if (position < json.size() && json[position] >= '0' && json[position] <= '9')
            return false;
    } else if (json[position] >= '1' && json[position] <= '9') {
        while (position < json.size()
               && json[position] >= '0' && json[position] <= '9')
            ++position;
    } else {
        return false;
    }

    if (position < json.size() && json[position] == '.') {
        ++position;
        const std::size_t fractionBegin = position;
        while (position < json.size()
               && json[position] >= '0' && json[position] <= '9')
            ++position;
        if (position == fractionBegin) return false;
    }

    if (position < json.size() && (json[position] == 'e' || json[position] == 'E')) {
        ++position;
        if (position < json.size() && (json[position] == '+' || json[position] == '-'))
            ++position;
        const std::size_t exponentBegin = position;
        while (position < json.size()
               && json[position] >= '0' && json[position] <= '9')
            ++position;
        if (position == exponentBegin) return false;
    }
    return position > begin;
}

bool skipJsonValue(const std::string& json, std::size_t& position,
                   unsigned int depth = 0u) {
    skipJsonWhitespace(json, position);
    if (position >= json.size() || depth > 32u) return false;

    if (json[position] == '"') return skipJsonString(json, position);

    if (json[position] == '{') {
        ++position;
        skipJsonWhitespace(json, position);
        if (position < json.size() && json[position] == '}') {
            ++position;
            return true;
        }
        std::vector<std::string> keys;
        for (;;) {
            skipJsonWhitespace(json, position);
            const std::size_t keyBegin = position;
            if (!skipJsonString(json, position)) return false;
            const std::string key = json.substr(keyBegin, position - keyBegin);
            if (std::find(keys.begin(), keys.end(), key) != keys.end()) return false;
            keys.push_back(key);
            skipJsonWhitespace(json, position);
            if (position >= json.size() || json[position++] != ':') return false;
            if (!skipJsonValue(json, position, depth + 1u)) return false;
            skipJsonWhitespace(json, position);
            if (position >= json.size()) return false;
            if (json[position] == '}') {
                ++position;
                return true;
            }
            if (json[position++] != ',') return false;
        }
    }

    if (json[position] == '[') {
        ++position;
        skipJsonWhitespace(json, position);
        if (position < json.size() && json[position] == ']') {
            ++position;
            return true;
        }
        for (;;) {
            if (!skipJsonValue(json, position, depth + 1u)) return false;
            skipJsonWhitespace(json, position);
            if (position >= json.size()) return false;
            if (json[position] == ']') {
                ++position;
                return true;
            }
            if (json[position++] != ',') return false;
        }
    }

    for (const std::string_view literal : {std::string_view("true"),
                                           std::string_view("false"),
                                           std::string_view("null")}) {
        if (json.compare(position, literal.size(), literal) == 0) {
            position += literal.size();
            return true;
        }
    }
    return skipJsonNumber(json, position);
}

/* Locate a field in the current object only; nested keys never shadow it. */
std::optional<JsonSpan> topLevelJsonValue(const std::string& json,
                                          const std::string& wantedKey) {
    std::size_t position = 0u;
    std::vector<std::string> seenKeys;
    std::optional<JsonSpan> found;
    skipJsonWhitespace(json, position);
    if (position >= json.size() || json[position++] != '{') return std::nullopt;
    for (;;) {
        skipJsonWhitespace(json, position);
        if (position >= json.size()) return std::nullopt;
        if (json[position] == '}') {
            ++position;
            skipJsonWhitespace(json, position);
            return position == json.size() ? found : std::nullopt;
        }
        std::string key;
        if (!readPlainJsonString(json, position, &key)) return std::nullopt;
        if (std::find(seenKeys.begin(), seenKeys.end(), key) != seenKeys.end())
            return std::nullopt;
        seenKeys.push_back(key);
        skipJsonWhitespace(json, position);
        if (position >= json.size() || json[position++] != ':') return std::nullopt;
        skipJsonWhitespace(json, position);
        const std::size_t valueBegin = position;
        if (!skipJsonValue(json, position)) return std::nullopt;
        if (key == wantedKey) found = JsonSpan{valueBegin, position};
        skipJsonWhitespace(json, position);
        if (position >= json.size()) return std::nullopt;
        if (json[position] == ',') {
            ++position;
            continue;
        }
        if (json[position] == '}') {
            ++position;
            skipJsonWhitespace(json, position);
            return position == json.size() ? found : std::nullopt;
        }
        return std::nullopt;
    }
}

std::optional<unsigned int> jsonUnsignedInteger(const std::string& json,
                                                const std::string& key) {
    const auto span = topLevelJsonValue(json, key);
    if (!span || span->begin >= span->end) return std::nullopt;
    unsigned int value = 0u;
    const char* begin = json.data() + span->begin;
    const char* end = json.data() + span->end;
    const auto parsed = std::from_chars(begin, end, value);
    if (parsed.ec != std::errc{} || parsed.ptr != end) return std::nullopt;
    return value;
}

std::optional<std::string> jsonString(const std::string& json, const std::string& key) {
    const auto span = topLevelJsonValue(json, key);
    if (!span || span->begin >= span->end || json[span->begin] != '"')
        return std::nullopt;
    std::size_t position = span->begin;
    std::string value;
    if (!readPlainJsonString(json, position, &value) || position != span->end)
        return std::nullopt;
    return value;
}

std::vector<std::string> jsonStringArray(const std::string& json, const std::string& key) {
    const auto span = topLevelJsonValue(json, key);
    if (!span || span->begin >= span->end || json[span->begin] != '[') return {};
    std::size_t position = span->begin + 1u;
    std::vector<std::string> result;
    for (;;) {
        skipJsonWhitespace(json, position);
        if (position >= span->end) return {};
        if (json[position] == ']') {
            ++position;
            return position == span->end ? result : std::vector<std::string>{};
        }
        std::string item;
        if (!readPlainJsonString(json, position, &item)) return {};
        result.push_back(std::move(item));
        skipJsonWhitespace(json, position);
        if (position >= span->end) return {};
        if (json[position] == ',') {
            ++position;
            continue;
        }
        if (json[position] == ']') {
            ++position;
            return position == span->end ? result : std::vector<std::string>{};
        }
        return {};
    }
}

bool validVentuHost(const std::string& json) {
    const auto span = topLevelJsonValue(json, "host");
    if (!span || span->begin >= span->end || json[span->begin] != '{')
        return false;
    const std::string hostObject = json.substr(span->begin, span->end - span->begin);
    const auto hostId = jsonString(hostObject, "id");
    return hostId && *hostId == "ventu";
}

bool readFileBounded(const fs::path& path, std::string& output) {
    std::string candidate;
    candidate.reserve(static_cast<std::size_t>(kMaxManifestBytes));
#if defined(_WIN32)
    HANDLE handle = CreateFileW(
        path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    if (handle == INVALID_HANDLE_VALUE) return false;
    BY_HANDLE_FILE_INFORMATION metadata{};
    LARGE_INTEGER fileSize{};
    bool valid = GetFileInformationByHandle(handle, &metadata) != 0
        && GetFileSizeEx(handle, &fileSize) != 0
        && GetFileType(handle) == FILE_TYPE_DISK
        && (metadata.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == 0u
        && fileSize.QuadPart >= 0
        && static_cast<unsigned long long>(fileSize.QuadPart) <= kMaxManifestBytes;
    if (valid) {
        char buffer[4096];
        for (;;) {
            DWORD count = 0u;
            if (!ReadFile(handle, buffer, static_cast<DWORD>(sizeof buffer), &count, nullptr)) {
                valid = false;
                break;
            }
            if (count == 0u) break;
            if (candidate.size() > kMaxManifestBytes - static_cast<std::uintmax_t>(count)) {
                valid = false;
                break;
            }
            candidate.append(buffer, static_cast<std::size_t>(count));
        }
    }
    CloseHandle(handle);
    if (!valid) return false;
#else
    struct stat metadata{};
    const int flags = O_RDONLY
#ifdef O_CLOEXEC
        | O_CLOEXEC
#endif
#ifdef O_NOFOLLOW
        | O_NOFOLLOW
#endif
#ifdef O_NONBLOCK
        | O_NONBLOCK
#endif
        ;
    const int descriptor = ::open(path.c_str(), flags);
    if (descriptor < 0) return false;
    bool valid = ::fstat(descriptor, &metadata) == 0
        && S_ISREG(metadata.st_mode)
        && metadata.st_size >= 0
        && static_cast<std::uintmax_t>(metadata.st_size) <= kMaxManifestBytes;
    if (valid) {
        char buffer[4096];
        for (;;) {
            const auto count = ::read(descriptor, buffer, sizeof buffer);
            if (count < 0) {
                if (errno == EINTR) continue;
                valid = false;
                break;
            }
            if (count == 0) break;
            const auto bytes = static_cast<std::size_t>(count);
            if (candidate.size() > kMaxManifestBytes - bytes) {
                valid = false;
                break;
            }
            candidate.append(buffer, bytes);
        }
    }
    ::close(descriptor);
    if (!valid) return false;
#endif
    output.swap(candidate);
    return true;
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
        const auto apiVersion = jsonUnsignedInteger(json, "api_version");
        const bool hostValid = validVentuHost(json);
        if (!schema || *schema != kPluginManifestSchema || !id || !name || !version
            || !domain || !description || !entrypoint || !hostValid || !apiVersion) {
            if (diagnostics) {
                diagnostics->push_back(
                    "manifest fields invalid schema=" + std::to_string(static_cast<bool>(schema))
                    + " id=" + std::to_string(static_cast<bool>(id))
                    + " name=" + std::to_string(static_cast<bool>(name))
                    + " version=" + std::to_string(static_cast<bool>(version))
                    + " domain=" + std::to_string(static_cast<bool>(domain))
                    + " description=" + std::to_string(static_cast<bool>(description))
                    + " entrypoint=" + std::to_string(static_cast<bool>(entrypoint))
                    + " host=" + std::to_string(hostValid)
                    + " api_version=" + std::to_string(static_cast<bool>(apiVersion))
                    + ": " + path.string());
            }
            continue;
        }
        const auto capabilities = jsonStringArray(json, "capabilities");
        if (!validId(*id) || name->empty() || domain->empty() || description->empty()
            || !validVersion(*version) || *apiVersion != kPluginApiVersion
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
        descriptor.apiVersion = *apiVersion;
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

bool PluginManager::hotSwapHandler(const std::string& idOrName, PluginHandler handler,
                                   PluginLifecycle onInitialize, PluginLifecycle onShutdown) {
    if (!handler) return false;
    const auto key = normalized(idOrName);
    const auto found = std::find_if(entries_.begin(), entries_.end(), [&](const Entry& entry) {
        return normalized(entry.descriptor.id) == key
            || normalized(entry.descriptor.name) == key;
    });
    // A running/initialised handler is never replaced underneath an active dispatch.
    if (found == entries_.end() || !found->handler || found->enabled || found->initialized)
        return false;
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
