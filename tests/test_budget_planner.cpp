#include "budget_planner.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace ventu::budget;
    auto plan = defaultPlan();
    assert(plan.limitUah == 500);
    assert(plan.plannedUah == 500);
    assert(plan.remainingUah == 0);
    assert(plan.withinBudget);

    assert(applyScenario(plan, "coffee"));
    assert(plan.allocations[4] == 460);
    assert(plan.allocations[5] == 40);
    assert(plan.plannedUah == 500);

    plan.allocations[0] = 350;
    plan.allocations[1] = 200;
    assert(calculate(plan));
    assert(plan.plannedUah == 670);
    assert(plan.remainingUah == -170);
    assert(!plan.withinBudget);

    assert(!applyScenario(plan, "unknown"));
    plan.limitUah = -1;
    std::string error;
    assert(!calculate(plan, &error));
    assert(!error.empty());

    std::cout << "VentuSample budget planner tests: PASS\n";
    return 0;
}
