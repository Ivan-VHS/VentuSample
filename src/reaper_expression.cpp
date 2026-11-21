#include "reaper_expression.hpp"

#include <cstdint>

#ifdef VENTU_HAS_REAPER
#include "reaper/ffi.h"
#endif

namespace ventu {

bool reaperRuntimeAvailable() noexcept {
#ifdef VENTU_HAS_REAPER
    return reaper_abi_version() == REAPER_ABI_VERSION;
#else
    return false;
#endif
}

bool evaluateReaperInteger(const std::string& expression,
                           std::int64_t& result,
                           std::string& error) {
#ifdef VENTU_HAS_REAPER
    if (expression.size() > static_cast<std::size_t>(REAPER_MAX_SOURCE_BYTES)) {
        error = "Reaper expression exceeds the 1 MiB native limit";
        return false;
    }
    std::int64_t candidate = result;
    char diagnostic[256] = {};
    const auto status = reaper_v1_eval_i64(
        expression.data(), static_cast<std::uint64_t>(expression.size()),
        &candidate, diagnostic, static_cast<std::uint64_t>(sizeof diagnostic));
    if (status != REAPER_ABI_OK) {
        error = diagnostic[0] != '\0'
            ? diagnostic
            : "Reaper native evaluation failed with status " + std::to_string(status);
        return false;
    }
    result = candidate;
    error.clear();
    return true;
#else
    (void)expression;
    (void)result;
    error = "Reaper runtime is not linked; configure REAPER_SOURCE_DIR";
    return false;
#endif
}

bool lookupReaperSelfhood(const std::string& identity,
                          std::uint64_t& ordinal,
                          std::string& error) {
#ifdef VENTU_HAS_REAPER
    if (identity.empty() || identity.find('\0') != std::string::npos) {
        error = "identity must be non-empty and must not contain NUL";
        return false;
    }
    std::uint64_t candidate = ordinal;
    const auto status = reaper_v1_selfhood_lookup(identity.c_str(), &candidate);
    if (status != REAPER_ABI_OK) {
        error = status == REAPER_ABI_NOT_FOUND
            ? "Reaper selfhood identity was not found"
            : "Reaper selfhood lookup failed with status " + std::to_string(status);
        return false;
    }
    ordinal = candidate;
    error.clear();
    return true;
#else
    (void)identity;
    (void)ordinal;
    error = "Reaper runtime is not linked; configure REAPER_SOURCE_DIR";
    return false;
#endif
}

} // namespace ventu
