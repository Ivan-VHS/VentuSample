#pragma once

#include <array>
#include <string>
#include <string_view>

namespace ventu::budget {

inline constexpr int kDefaultLimitUah = 500;
inline constexpr int kMaximumUah = 1'000'000;
inline constexpr std::size_t kCategoryCount = 6;

struct Scenario {
    std::string_view id;
    std::string_view label;
    std::array<int, kCategoryCount> allocations;
};

struct Plan {
    int limitUah{kDefaultLimitUah};
    std::array<int, kCategoryCount> allocations{280, 120, 0, 0, 50, 50};
    int plannedUah{500};
    int remainingUah{0};
    bool withinBudget{true};
};

const std::array<std::string_view, kCategoryCount>& categoryNames();
const std::array<Scenario, 7>& scenarioCatalog();
Plan defaultPlan();
bool applyScenario(Plan& plan, std::string_view scenarioId);
bool calculate(Plan& plan, std::string* error = nullptr);

} // namespace ventu::budget
