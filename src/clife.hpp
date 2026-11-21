#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ventu::clife {

inline constexpr std::size_t max_payload_bytes = 64u * 1024u * 1024u;
inline constexpr std::uint32_t schema_version = 1u;

enum class FaultType {
    storage_io,
    artifact_integrity,
    resource_pressure,
    processing_interruption
};

enum class RecoveryBranch {
    normal_resume,
    verified_restore,
    partial_hold,
    emergency_safe_stop,
    invalid_state
};

struct Snapshot {
    std::string project_id;
    std::uint64_t generation{0};
    std::vector<std::uint8_t> payload;
    std::uint64_t checksum_fnv1a64{0};
    std::uint32_t schema{schema_version};
};

struct RuntimeState {
    std::string project_id;
    std::uint64_t generation{0};
    double artifact_integrity{1.0};
    double resource_health{1.0};
    double processing_health{1.0};
    bool recovery_locked{false};
};

struct FaultEvent {
    FaultType type{FaultType::storage_io};
    double severity{0.0};
    bool mitigated{false};
};

bool capture_snapshot(const std::string& project_id, std::uint64_t generation,
                      const std::vector<std::uint8_t>& payload, Snapshot& output) noexcept;
bool verify_snapshot(const Snapshot& snapshot) noexcept;
/* Restore copies bytes to a caller-owned output; no render callback or real-time path is used. */
bool restore_snapshot(const Snapshot& snapshot, const std::string& expected_project_id,
                      std::vector<std::uint8_t>& output) noexcept;
RecoveryBranch branch_awakening(RuntimeState& state, const Snapshot* snapshot,
                                double external_threat) noexcept;
RecoveryBranch handle_fault(RuntimeState& state, FaultEvent& fault,
                            const Snapshot* snapshot, double external_threat) noexcept;

} // namespace ventu::clife
