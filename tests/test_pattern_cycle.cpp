#include "pattern_cycle.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}
} // namespace

int main() {
    const auto repeated = ventu::analyzePatternCycle(
        {0u, 25u, 100u, 125u, 200u, 225u, 300u, 325u}, 400u, 100u);
    require(repeated.valid && repeated.analyzed, "four complete cycles should be analyzed");
    require(repeated.cycleCount == 4u && repeated.eventsPerCycle == 2u,
            "cycle dimensions should describe the reference motif");
    require(repeated.matchingCycles == 4u && repeated.mismatchingCycles == 0u,
            "identical event offsets should repeat in every cycle");
    require(repeated.repeatAccuracyPercent == 100.0,
            "a fully repeated motif should report 100 percent agreement");

    const auto variation = ventu::analyzePatternCycle(
        {0u, 25u, 100u, 125u, 200u, 225u, 300u, 326u}, 400u, 100u);
    require(variation.valid && variation.analyzed, "a changed motif remains valid input");
    require(variation.matchingCycles == 3u && variation.mismatchingCycles == 1u,
            "one changed onset should be localized to one cycle");
    require(variation.repeatAccuracyPercent == 75.0,
            "repeat agreement should reflect the changed cycle");

    const auto missingEvent = ventu::analyzePatternCycle(
        {0u, 25u, 100u, 125u, 200u, 225u, 300u}, 400u, 100u);
    require(missingEvent.valid && missingEvent.mismatchingCycles == 1u,
            "a missing event should distinguish the final cycle");

    const auto empty = ventu::analyzePatternCycle({}, 400u, 100u);
    require(empty.valid && !empty.analyzed,
            "empty event streams should be a valid not-applicable analysis");

    const auto single = ventu::analyzePatternCycle({0u, 25u}, 100u, 100u);
    require(single.valid && !single.analyzed,
            "a single cycle cannot prove repetition");

    const auto unsorted = ventu::analyzePatternCycle({25u, 0u}, 100u, 50u);
    require(!unsorted.valid, "unsorted event frames should be rejected");

    const auto outside = ventu::analyzePatternCycle({0u, 400u}, 400u, 100u);
    require(!outside.valid, "events at the exclusive upper bound should be rejected");

    const auto incomplete = ventu::analyzePatternCycle({0u, 25u}, 101u, 100u);
    require(!incomplete.valid, "partial cycles should be rejected");

    const auto overBudget = ventu::analyzePatternCycle({0u}, 4097u, 1u);
    require(!overBudget.valid, "cycle counts above the bounded budget should be rejected");

    return 0;
}
