#include "clife.hpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main() {
    using namespace ventu::clife;
    const std::vector<std::uint8_t> payload{1u, 2u, 3u, 4u, 5u};
    Snapshot snapshot;
    assert(capture_snapshot("ventu-demo", 3u, payload, snapshot));
    assert(verify_snapshot(snapshot));

    std::vector<std::uint8_t> restored;
    assert(restore_snapshot(snapshot, "ventu-demo", restored));
    assert(restored == payload);
    assert(!restore_snapshot(snapshot, "other-project", restored));

    auto damaged = snapshot;
    damaged.payload[0] ^= 0xffu;
    assert(!verify_snapshot(damaged));

    RuntimeState state{"ventu-demo", 4u, 0.80, 0.90, 0.90, false};
    FaultEvent fault{FaultType::artifact_integrity, 0.10, false};
    assert(handle_fault(state, fault, &snapshot, 0.0) == RecoveryBranch::verified_restore);
    assert(fault.mitigated);

    state.artifact_integrity = 0.20;
    state.resource_health = 0.30;
    state.processing_health = 0.20;
    assert(branch_awakening(state, nullptr, 0.95) == RecoveryBranch::emergency_safe_stop);
    assert(state.recovery_locked);

    RuntimeState invalid{"ventu-demo", 0u, 2.0, 1.0, 1.0, false};
    assert(branch_awakening(invalid, nullptr, 0.0) == RecoveryBranch::invalid_state);
    return 0;
}
