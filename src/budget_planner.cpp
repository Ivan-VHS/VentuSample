#include "budget_planner.hpp"

#include <algorithm>

namespace ventu::budget {
namespace {
constexpr std::array<std::string_view, kCategoryCount> kCategoryLabels{
    "Продукты и питание",
    "Транспорт",
    "Быт и здоровье",
    "Связь и обучение",
    "Досуг и кофе",
    "Резерв"
};
constexpr std::array<Scenario, 7> kScenarios{{
    {"weekly", "Недельный баланс", {280, 120, 0, 0, 50, 50}},
    {"groceries", "Продуктовый минимум", {500, 0, 0, 0, 0, 0}},
    {"transport", "Транспортный запас", {0, 500, 0, 0, 0, 0}},
    {"coffee", "Кофе и перезагрузка", {0, 0, 0, 0, 460, 40}},
    {"household", "Бытовой рефилл", {0, 0, 500, 0, 0, 0}},
    {"learning", "Инвестиция в себя", {0, 0, 0, 500, 0, 0}},
    {"reserve", "Неприкосновенный резерв", {0, 0, 0, 0, 0, 500}}
}};
} // namespace

const std::array<std::string_view, kCategoryCount>& categoryNames() {
    return kCategoryLabels;
}

const std::array<Scenario, 7>& scenarioCatalog() {
    return kScenarios;
}

Plan defaultPlan() {
    Plan plan;
    (void)calculate(plan);
    return plan;
}

bool applyScenario(Plan& plan, std::string_view scenarioId) {
    const auto found = std::find_if(kScenarios.begin(), kScenarios.end(),
        [scenarioId](const Scenario& item) { return item.id == scenarioId; });
    if (found == kScenarios.end()) return false;
    plan.allocations = found->allocations;
    return calculate(plan);
}

bool calculate(Plan& plan, std::string* error) {
    if (plan.limitUah < 0 || plan.limitUah > kMaximumUah) {
        if (error) *error = "Лимит должен быть от 0 до 1 000 000 грн.";
        return false;
    }
    int total = 0;
    for (const int amount : plan.allocations) {
        if (amount < 0 || amount > kMaximumUah || total > 6 * kMaximumUah - amount) {
            if (error) *error = "Суммы категорий должны быть целыми числами от 0 до 1 000 000 грн.";
            return false;
        }
        total += amount;
    }
    plan.plannedUah = total;
    plan.remainingUah = plan.limitUah - total;
    plan.withinBudget = plan.remainingUah >= 0;
    if (error) error->clear();
    return true;
}

} // namespace ventu::budget
