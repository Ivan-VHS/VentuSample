#include "reaper_expression.hpp"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void require(bool value, const char* message) {
    if (!value) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}
}

int main() {
    std::string error;
    std::int64_t result = 987;
    std::uint64_t ordinal = 0;

#ifdef VENTU_HAS_REAPER
    require(ventu::reaperRuntimeAvailable(), "linked Reaper ABI v1 should be available");
    require(ventu::evaluateReaperInteger("(6 * 7) - 1", result, error),
            "native expression should evaluate");
    require(result == 41, "native expression result should be 41");
    require(ventu::lookupReaperSelfhood("Сам", ordinal, error),
            "selfhood lookup should succeed");
    require(ordinal == 2u, "selfhood lookup should preserve canonical ordinal");
    ordinal = 99u;
    require(!ventu::lookupReaperSelfhood("unknown", ordinal, error),
            "unknown identity should fail");
    require(ordinal == 99u, "failed lookup must not mutate the output");
#else
    require(!ventu::reaperRuntimeAvailable(), "default build should not discover Reaper");
    require(!ventu::evaluateReaperInteger("6 * 7", result, error),
            "native expression should report unavailable backend");
    require(result == 987, "unavailable backend must not mutate output");
    require(error.find("REAPER_SOURCE_DIR") != std::string::npos,
            "unavailable diagnostic should explain explicit build option");
    require(!ventu::lookupReaperSelfhood("Сам", ordinal, error),
            "selfhood lookup should report unavailable backend");
#endif
    std::cout << "VentuSample Reaper bridge tests: passed\n";
    return 0;
}
