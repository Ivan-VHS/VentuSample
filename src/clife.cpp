#include "clife.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace ventu::clife {
namespace {
constexpr std::uint64_t fnv_offset = UINT64_C(14695981039346656037);
constexpr std::uint64_t fnv_prime = UINT64_C(1099511628211);

bool unit(double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

std::uint64_t checksum(const std::vector<std::uint8_t>& bytes) noexcept {
    std::uint64_t hash = fnv_offset;
    for (const auto byte : bytes) {
        hash ^= static_cast<std::uint64_t>(byte);
        hash *= fnv_prime;
    }
    return hash;
}
} // namespace

bool capture_snapshot(const std::string& project_id, std::uint64_t generation,
                      const std::vector<std::uint8_t>& payload, Snapshot& output) noexcept {
    if (project_id.empty() || project_id.size() > 128u || payload.size() > max_payload_bytes)
        return false;
    try {
        Snapshot candidate;
        candidate.project_id = project_id;
        candidate.generation = generation;
        candidate.payload = payload;
        candidate.checksum_fnv1a64 = checksum(candidate.payload);
        candidate.schema = schema_version;
        output = std::move(candidate);
        return true;
    } catch (...) {
        return false;
    }
}

bool verify_snapshot(const Snapshot& snapshot) noexcept {
    return snapshot.schema == schema_version &&
           !snapshot.project_id.empty() && snapshot.project_id.size() <= 128u &&
           snapshot.payload.size() <= max_payload_bytes &&
           checksum(snapshot.payload) == snapshot.checksum_fnv1a64;
}

bool restore_snapshot(const Snapshot& snapshot, const std::string& expected_project_id,
                      std::vector<std::uint8_t>& output) noexcept {
    if (!verify_snapshot(snapshot) || expected_project_id.empty() ||
        snapshot.project_id != expected_project_id) return false;
    try {
        output = snapshot.payload;
        return true;
    } catch (...) {
        return false;
    }
}

RecoveryBranch branch_awakening(RuntimeState& state, const Snapshot* snapshot,
                                double external_threat) noexcept {
    if (!unit(state.artifact_integrity) || !unit(state.resource_health) ||
        !unit(state.processing_health) || !unit(external_threat) ||
        state.project_id.empty() || state.project_id.size() > 128u)
        return RecoveryBranch::invalid_state;
    const double viability = 0.50 * state.artifact_integrity +
                             0.25 * state.resource_health +
                             0.25 * state.processing_health;
    const bool snapshot_ok = snapshot != nullptr && verify_snapshot(*snapshot) &&
                             snapshot->project_id == state.project_id &&
                             snapshot->generation <= state.generation;
    if (external_threat > 0.85 && viability < 0.70) {
        state.recovery_locked = true;
        return RecoveryBranch::emergency_safe_stop;
    }
    if (viability >= 0.95 && state.processing_health >= 0.95) {
        state.recovery_locked = false;
        return RecoveryBranch::normal_resume;
    }
    if (snapshot_ok && viability >= 0.55) {
        state.recovery_locked = false;
        return RecoveryBranch::verified_restore;
    }
    state.recovery_locked = true;
    return RecoveryBranch::partial_hold;
}

RecoveryBranch handle_fault(RuntimeState& state, FaultEvent& fault,
                            const Snapshot* snapshot, double external_threat) noexcept {
    if (!unit(fault.severity)) return RecoveryBranch::invalid_state;
    double* metric = nullptr;
    double scale = 0.0;
    switch (fault.type) {
        case FaultType::storage_io: metric = &state.resource_health; scale = 0.30; break;
        case FaultType::artifact_integrity: metric = &state.artifact_integrity; scale = 0.45; break;
        case FaultType::resource_pressure: metric = &state.resource_health; scale = 0.45; break;
        case FaultType::processing_interruption: metric = &state.processing_health; scale = 0.50; break;
        default: return RecoveryBranch::invalid_state;
    }
    if (!unit(*metric)) return RecoveryBranch::invalid_state;
    *metric = std::max(0.0, *metric - fault.severity * scale);
    fault.mitigated = false;
    const auto branch = branch_awakening(state, snapshot, external_threat);
    fault.mitigated = branch == RecoveryBranch::normal_resume ||
                      branch == RecoveryBranch::verified_restore;
    return branch;
}

} // namespace ventu::clife
