#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ventu {

enum class RuntimeStatus {
    Ok, Invalid, Conflict, Limit, Integrity, Thrown, State
};

struct RuntimeSnapshot {
    std::uint64_t generation{};
    std::uint32_t schemaVersion{};
    std::uint32_t crc32{};
    std::vector<std::uint8_t> payload;
};

class RuntimeTransactionError final : public std::runtime_error {
public:
    explicit RuntimeTransactionError(const std::string& message) : std::runtime_error(message) {}
};

// This transaction owns in-memory bytes only; it does not write firmware, devices,
// plugin binaries, audio hardware or arbitrary files.
class RuntimeTransaction final {
public:
    using EventSink = std::function<void(std::string_view, std::uint64_t)>;
    using Migration = std::function<std::vector<std::uint8_t>(
        const std::vector<std::uint8_t>&, std::uint32_t, std::uint32_t)>;

    static constexpr std::size_t kDefaultMaxBytes = 16u * 1024u * 1024u;

    explicit RuntimeTransaction(std::vector<std::uint8_t> initial,
                                std::uint32_t schemaVersion = 1u,
                                std::size_t maxBytes = kDefaultMaxBytes,
                                EventSink events = {});

    RuntimeStatus patch(std::size_t offset, std::size_t removeSize,
                        const std::vector<std::uint8_t>& replacement);
    RuntimeStatus rebind(std::string name, std::uint64_t binding);
    RuntimeStatus remap(const std::vector<std::size_t>& targetToSource);
    RuntimeStatus hotSwap(const std::vector<std::uint8_t>& replacement);
    RuntimeStatus migrate(std::uint32_t targetSchema, const Migration& migration);

    RuntimeStatus flash(std::optional<std::uint64_t> expectedGeneration = std::nullopt);
    RuntimeStatus reflash(std::optional<std::uint64_t> expectedGeneration = std::nullopt);
    RuntimeStatus sync();
    RuntimeStatus rollback();

    bool emit(std::string_view name);
    bool yieldCheckpoint(std::string_view name);
    bool validate() const;
    RuntimeSnapshot snapshot() const;
    std::uint64_t generation() const;
    std::string lastError() const;

    std::future<RuntimeStatus> asyncFlash(std::optional<std::uint64_t> expectedGeneration = std::nullopt);
    static RuntimeStatus await(std::future<RuntimeStatus>& task);

    [[noreturn]] void throwError(const std::string& message);
    [[noreturn]] void panic(const std::string& message);
    static std::string catchError(const std::function<void()>& operation);
    static void assertCondition(bool condition, const std::string& message = "runtime assertion failed");

    template <typename T>
    static T returnValue(T value) { return value; }

private:
    static std::uint32_t checksum(const std::vector<std::uint8_t>& bytes);
    bool stage(const std::vector<std::uint8_t>& value);
    RuntimeStatus commit(std::optional<std::uint64_t> expected, std::string_view event);

    mutable std::recursive_mutex mutex_;
    std::vector<std::uint8_t> committed_;
    std::vector<std::uint8_t> staged_;
    std::uint64_t generation_{0};
    std::uint32_t schemaVersion_{1};
    std::uint32_t committedSchemaVersion_{1};
    std::uint32_t committedCrc_{0};
    std::uint32_t stagedCrc_{0};
    std::size_t maxBytes_{kDefaultMaxBytes};
    std::map<std::string, std::uint64_t> bindings_;
    EventSink events_;
    std::string lastError_;
};

} // namespace ventu
