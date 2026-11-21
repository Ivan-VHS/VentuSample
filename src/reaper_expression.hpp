#pragma once

#include <cstdint>
#include <string>

namespace ventu {

/* Optional bridge to the Reaper C17 expression runtime.
 * No DLL/SO discovery is performed; availability is fixed at build time. */
bool reaperRuntimeAvailable() noexcept;
bool evaluateReaperInteger(const std::string& expression,
                           std::int64_t& result,
                           std::string& error);
bool lookupReaperSelfhood(const std::string& identity,
                          std::uint64_t& ordinal,
                          std::string& error);

} // namespace ventu
