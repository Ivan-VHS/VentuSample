#include "runtime_transaction.hpp"

#include <algorithm>
#include <limits>

namespace ventu {

std::uint32_t RuntimeTransaction::checksum(const std::vector<std::uint8_t>& bytes) {
    std::uint32_t crc = 0xFFFFFFFFu;
    for (const auto byte : bytes) {
        crc ^= byte;
        for (unsigned bit = 0; bit < 8u; ++bit) {
            const std::uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1u) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}

RuntimeTransaction::RuntimeTransaction(std::vector<std::uint8_t> initial,
                                       std::uint32_t schemaVersion,
                                       std::size_t maxBytes,
                                       EventSink events)
    : committed_(std::move(initial)),
      staged_(committed_),
      schemaVersion_(schemaVersion),
      committedSchemaVersion_(schemaVersion),
      maxBytes_(maxBytes),
      events_(std::move(events)) {
    if (schemaVersion == 0u) throw std::invalid_argument("schema version must be positive");
    if (maxBytes == 0u || committed_.size() > maxBytes_)
        throw std::invalid_argument("initial payload exceeds the configured transaction limit");
    committedCrc_ = checksum(committed_);
    stagedCrc_ = committedCrc_;
}

bool RuntimeTransaction::stage(const std::vector<std::uint8_t>& value) {
    if (value.size() > maxBytes_) {
        lastError_ = "staged payload exceeds configured byte limit";
        return false;
    }
    staged_ = value;
    stagedCrc_ = checksum(staged_);
    lastError_.clear();
    return true;
}

RuntimeTransaction::~RuntimeTransaction() {
    /* asyncFlash workers capture this; wait until every worker has stopped using it. */
    std::unique_lock lock(asyncMutex_);
    asyncCondition_.wait(lock, [this] { return activeAsync_ == 0u; });
}

RuntimeStatus RuntimeTransaction::patch(std::size_t offset, std::size_t removeSize,
                                        const std::vector<std::uint8_t>& replacement) {
    EventSink sink;
    std::uint64_t eventGeneration{};
    {
        std::lock_guard lock(mutex_);
        if (offset > staged_.size() || removeSize > staged_.size() - offset)
            return RuntimeStatus::Invalid;
        const auto remaining = staged_.size() - removeSize;
        if (replacement.size() > maxBytes_ - remaining) return RuntimeStatus::Limit;
        std::vector<std::uint8_t> next;
        next.reserve(remaining + replacement.size());
        next.insert(next.end(), staged_.begin(),
                    staged_.begin() + static_cast<std::ptrdiff_t>(offset));
        next.insert(next.end(), replacement.begin(), replacement.end());
        next.insert(next.end(),
                    staged_.begin() + static_cast<std::ptrdiff_t>(offset + removeSize), staged_.end());
        if (!stage(next)) return RuntimeStatus::Limit;
        sink = events_;
        eventGeneration = generation_;
    }
    if (sink) sink("PATCH_STAGED", eventGeneration);
    return RuntimeStatus::Ok;
}

RuntimeStatus RuntimeTransaction::rebind(std::string name, std::uint64_t binding) {
    EventSink sink;
    std::uint64_t eventGeneration{};
    {
        std::lock_guard lock(mutex_);
        if (name.empty() || name.size() > 128u) return RuntimeStatus::Invalid;
        bindings_[std::move(name)] = binding;
        sink = events_;
        eventGeneration = generation_;
    }
    if (sink) sink("REBIND_STAGED", eventGeneration);
    return RuntimeStatus::Ok;
}

RuntimeStatus RuntimeTransaction::remap(const std::vector<std::size_t>& targetToSource) {
    EventSink sink;
    std::uint64_t eventGeneration{};
    {
        std::lock_guard lock(mutex_);
        if (targetToSource.size() > maxBytes_) return RuntimeStatus::Limit;
        std::vector<std::uint8_t> next;
        next.reserve(targetToSource.size());
        for (const auto sourceIndex : targetToSource) {
            if (sourceIndex >= staged_.size()) return RuntimeStatus::Invalid;
            next.push_back(staged_[sourceIndex]);
        }
        if (!stage(next)) return RuntimeStatus::Limit;
        sink = events_;
        eventGeneration = generation_;
    }
    if (sink) sink("REMAP_STAGED", eventGeneration);
    return RuntimeStatus::Ok;
}

RuntimeStatus RuntimeTransaction::hotSwap(const std::vector<std::uint8_t>& replacement) {
    EventSink sink;
    std::uint64_t eventGeneration{};
    {
        std::lock_guard lock(mutex_);
        if (!stage(replacement)) return RuntimeStatus::Limit;
        sink = events_;
        eventGeneration = generation_;
    }
    if (sink) sink("HOT_SWAP_STAGED", eventGeneration);
    return RuntimeStatus::Ok;
}

RuntimeStatus RuntimeTransaction::migrate(std::uint32_t targetSchema, const Migration& migration) {
    std::vector<std::uint8_t> source;
    std::uint32_t sourceSchema{};
    std::uint64_t sourceGeneration{};
    {
        std::lock_guard lock(mutex_);
        if (!migration || targetSchema <= schemaVersion_) return RuntimeStatus::Invalid;
        source = staged_;
        sourceSchema = schemaVersion_;
        sourceGeneration = generation_;
    }

    std::vector<std::uint8_t> migrated;
    try {
        /* Migration code is supplied by an extension and must not run under our lock. */
        migrated = migration(source, sourceSchema, targetSchema);
    } catch (const std::exception& error) {
        std::lock_guard lock(mutex_);
        if (generation_ == sourceGeneration && schemaVersion_ == sourceSchema && staged_ == source)
            lastError_ = error.what();
        return RuntimeStatus::Thrown;
    } catch (...) {
        std::lock_guard lock(mutex_);
        if (generation_ == sourceGeneration && schemaVersion_ == sourceSchema && staged_ == source)
            lastError_ = "unknown migration failure";
        return RuntimeStatus::Thrown;
    }

    EventSink sink;
    std::uint64_t eventGeneration{};
    {
        std::lock_guard lock(mutex_);
        /* Reject a stale migration rather than overwriting concurrent staged work. */
        if (generation_ != sourceGeneration || schemaVersion_ != sourceSchema || staged_ != source)
            return RuntimeStatus::Conflict;
        if (!stage(migrated)) return RuntimeStatus::Limit;
        schemaVersion_ = targetSchema;
        sink = events_;
        eventGeneration = generation_;
    }
    if (sink) sink("MIGRATION_STAGED", eventGeneration);
    return RuntimeStatus::Ok;
}

RuntimeStatus RuntimeTransaction::commit(std::optional<std::uint64_t> expected,
                                         std::string_view event) {
    EventSink sink;
    std::uint64_t eventGeneration{};
    {
        std::lock_guard lock(mutex_);
        if (failed_) return RuntimeStatus::Thrown;
        if (expected && *expected != generation_) return RuntimeStatus::Conflict;
        if (generation_ == std::numeric_limits<std::uint64_t>::max()) {
            lastError_ = "transaction generation counter exhausted";
            return RuntimeStatus::Limit;
        }
        if (stagedCrc_ != checksum(staged_)) {
            lastError_ = "staged payload checksum mismatch";
            return RuntimeStatus::Integrity;
        }
        committed_ = staged_;
        committedCrc_ = stagedCrc_;
        committedSchemaVersion_ = schemaVersion_;
        ++generation_;
        lastError_.clear();
        sink = events_;
        eventGeneration = generation_;
    }
    /* User callbacks are never invoked while holding the transaction mutex. */
    if (sink) sink(event, eventGeneration);
    return RuntimeStatus::Ok;
}

RuntimeStatus RuntimeTransaction::flash(std::optional<std::uint64_t> expectedGeneration) {
    return commit(expectedGeneration, "FLASH_COMMITTED");
}

RuntimeStatus RuntimeTransaction::reflash(std::optional<std::uint64_t> expectedGeneration) {
    return commit(expectedGeneration, "REFLASH_COMMITTED");
}

RuntimeStatus RuntimeTransaction::sync() {
    return commit(std::nullopt, "SYNC_COMMITTED");
}

RuntimeStatus RuntimeTransaction::rollback() {
    EventSink sink;
    std::uint64_t eventGeneration{};
    {
        std::lock_guard lock(mutex_);
        staged_ = committed_;
        stagedCrc_ = committedCrc_;
        schemaVersion_ = committedSchemaVersion_;
        lastError_.clear();
        failed_ = false;
        sink = events_;
        eventGeneration = generation_;
    }
    if (sink) sink("ROLLBACK", eventGeneration);
    return RuntimeStatus::Ok;
}

bool RuntimeTransaction::emit(std::string_view name) {
    if (name.empty() || name.size() > 128u) return false;
    EventSink sink;
    std::uint64_t eventGeneration{};
    {
        std::lock_guard lock(mutex_);
        sink = events_;
        eventGeneration = generation_;
    }
    if (sink) sink(name, eventGeneration);
    return true;
}

bool RuntimeTransaction::yieldCheckpoint(std::string_view name) {
    return emit(name);
}

bool RuntimeTransaction::validate() const {
    std::lock_guard lock(mutex_);
    return staged_.size() <= maxBytes_ && stagedCrc_ == checksum(staged_);
}

RuntimeSnapshot RuntimeTransaction::snapshot() const {
    std::lock_guard lock(mutex_);
    return {generation_, committedSchemaVersion_, committedCrc_, committed_};
}

std::uint64_t RuntimeTransaction::generation() const {
    std::lock_guard lock(mutex_);
    return generation_;
}

std::string RuntimeTransaction::lastError() const {
    std::lock_guard lock(mutex_);
    return lastError_;
}

std::future<RuntimeStatus> RuntimeTransaction::asyncFlash(
    std::optional<std::uint64_t> expectedGeneration) {
    {
        std::lock_guard lock(asyncMutex_);
        if (activeAsync_ == std::numeric_limits<std::size_t>::max())
            throw std::overflow_error("too many active asynchronous transaction operations");
        ++activeAsync_;
    }

    const auto finish = [this] {
        std::lock_guard lock(asyncMutex_);
        --activeAsync_;
        /* Keep the lock until notification completes so destruction cannot race it. */
        asyncCondition_.notify_all();
    };

    try {
        return std::async(std::launch::async, [this, expectedGeneration, finish] {
            try {
                const auto result = flash(expectedGeneration);
                finish();
                return result;
            } catch (...) {
                finish();
                throw;
            }
        });
    } catch (...) {
        /* std::async can fail before a worker takes ownership of the operation. */
        finish();
        throw;
    }
}

RuntimeStatus RuntimeTransaction::await(std::future<RuntimeStatus>& task) {
    if (!task.valid()) return RuntimeStatus::State;
    try {
        return task.get();
    } catch (const std::exception&) {
        return RuntimeStatus::Thrown;
    }
}

[[noreturn]] void RuntimeTransaction::throwError(const std::string& message) {
    {
        std::lock_guard lock(mutex_);
        lastError_ = message;
        failed_ = true;
    }
    throw RuntimeTransactionError(message);
}

[[noreturn]] void RuntimeTransaction::panic(const std::string& message) {
    {
        std::lock_guard lock(mutex_);
        lastError_ = "panic: " + message;
        failed_ = true;
    }
    throw RuntimeTransactionError("panic: " + message);
}

std::string RuntimeTransaction::catchError(const std::function<void()>& operation) {
    try {
        operation();
        return {};
    } catch (const std::exception& error) {
        return error.what();
    } catch (...) {
        return "unknown runtime exception";
    }
}

void RuntimeTransaction::assertCondition(bool condition, const std::string& message) {
    if (!condition) throw RuntimeTransactionError(message);
}

} // namespace ventu
