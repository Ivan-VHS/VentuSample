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

RuntimeStatus RuntimeTransaction::patch(std::size_t offset, std::size_t removeSize,
                                        const std::vector<std::uint8_t>& replacement) {
    std::lock_guard lock(mutex_);
    if (offset > staged_.size() || removeSize > staged_.size() - offset) return RuntimeStatus::Invalid;
    const auto remaining = staged_.size() - removeSize;
    if (replacement.size() > maxBytes_ - remaining) return RuntimeStatus::Limit;
    std::vector<std::uint8_t> next;
    next.reserve(remaining + replacement.size());
    next.insert(next.end(), staged_.begin(), staged_.begin() + static_cast<std::ptrdiff_t>(offset));
    next.insert(next.end(), replacement.begin(), replacement.end());
    next.insert(next.end(),
                staged_.begin() + static_cast<std::ptrdiff_t>(offset + removeSize), staged_.end());
    if (!stage(next)) return RuntimeStatus::Limit;
    if (events_) events_("PATCH_STAGED", generation_);
    return RuntimeStatus::Ok;
}

RuntimeStatus RuntimeTransaction::rebind(std::string name, std::uint64_t binding) {
    std::lock_guard lock(mutex_);
    if (name.empty() || name.size() > 128u) return RuntimeStatus::Invalid;
    bindings_[std::move(name)] = binding;
    if (events_) events_("REBIND_STAGED", generation_);
    return RuntimeStatus::Ok;
}

RuntimeStatus RuntimeTransaction::remap(const std::vector<std::size_t>& targetToSource) {
    std::lock_guard lock(mutex_);
    if (targetToSource.size() > maxBytes_) return RuntimeStatus::Limit;
    std::vector<std::uint8_t> next;
    next.reserve(targetToSource.size());
    for (const auto sourceIndex : targetToSource) {
        if (sourceIndex >= staged_.size()) return RuntimeStatus::Invalid;
        next.push_back(staged_[sourceIndex]);
    }
    if (!stage(next)) return RuntimeStatus::Limit;
    if (events_) events_("REMAP_STAGED", generation_);
    return RuntimeStatus::Ok;
}

RuntimeStatus RuntimeTransaction::hotSwap(const std::vector<std::uint8_t>& replacement) {
    std::lock_guard lock(mutex_);
    if (!stage(replacement)) return RuntimeStatus::Limit;
    if (events_) events_("HOT_SWAP_STAGED", generation_);
    return RuntimeStatus::Ok;
}

RuntimeStatus RuntimeTransaction::migrate(std::uint32_t targetSchema, const Migration& migration) {
    std::lock_guard lock(mutex_);
    if (!migration || targetSchema <= schemaVersion_) return RuntimeStatus::Invalid;
    try {
        const auto migrated = migration(staged_, schemaVersion_, targetSchema);
        if (!stage(migrated)) return RuntimeStatus::Limit;
        schemaVersion_ = targetSchema;
        if (events_) events_("MIGRATION_STAGED", generation_);
        return RuntimeStatus::Ok;
    } catch (const std::exception& error) {
        lastError_ = error.what();
        return RuntimeStatus::Thrown;
    } catch (...) {
        lastError_ = "unknown migration failure";
        return RuntimeStatus::Thrown;
    }
}

RuntimeStatus RuntimeTransaction::commit(std::optional<std::uint64_t> expected,
                                         std::string_view event) {
    std::lock_guard lock(mutex_);
    if (failed_) return RuntimeStatus::Thrown;
    if (expected && *expected != generation_) return RuntimeStatus::Conflict;
    if (stagedCrc_ != checksum(staged_)) {
        lastError_ = "staged payload checksum mismatch";
        return RuntimeStatus::Integrity;
    }
    committed_ = staged_;
    committedCrc_ = stagedCrc_;
    committedSchemaVersion_ = schemaVersion_;
    ++generation_;
    lastError_.clear();
    if (events_) events_(event, generation_);
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
    std::lock_guard lock(mutex_);
    staged_ = committed_;
    stagedCrc_ = committedCrc_;
    schemaVersion_ = committedSchemaVersion_;
    lastError_.clear();
    failed_ = false;
    if (events_) events_("ROLLBACK", generation_);
    return RuntimeStatus::Ok;
}

bool RuntimeTransaction::emit(std::string_view name) {
    std::lock_guard lock(mutex_);
    if (name.empty() || name.size() > 128u) return false;
    if (events_) events_(name, generation_);
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
    // The caller must keep this transaction alive until the returned future is awaited.
    return std::async(std::launch::async, [this, expectedGeneration] {
        return flash(expectedGeneration);
    });
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
