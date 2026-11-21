#include "occasion_planner.hpp"

#include <cassert>
#include <numeric>
#include <stdexcept>

int main() {
    using namespace ventu::occasion;
    const auto even = splitEvenly(500, 5);
    assert(even == std::vector<std::uint32_t>({100, 100, 100, 100, 100}));
    assert(splitEvenly(10, 3) == std::vector<std::uint32_t>({4, 3, 3}));
    assert(splitWeighted(500, {2, 1, 1}) == std::vector<std::uint32_t>({250, 125, 125}));
    assert(splitWeighted(10, {1, 1, 1}) == std::vector<std::uint32_t>({4, 3, 3}));
    assert(percentOf(500, 25) == 125);
    assert(percentOf(500, 1) == 5);
    assert(combinationCount(5, 2) == 10);
    assert(combinationCount(5, 3) == 10);
    assert(combinationCount(5, 0) == 1);

    const std::vector<CandyOption> catalog{
        {"Praline", "chocolate", 18, 30, 5, 14},
        {"Truffle", "truffle", 24, 20, 5, 16},
        {"Wafer", "wafer", 12, 25, 3, 18},
    };
    const auto basket = buildCandyBasket(100, catalog);
    assert(basket.second <= 100);
    std::uint64_t basketTotal = 0;
    for (const auto& line : basket.first) {
        assert(line.quantity > 0);
        assert(line.lineTotal == static_cast<std::uint64_t>(line.unitPrice) * line.quantity);
        basketTotal += line.lineTotal;
    }
    assert(basketTotal == basket.second);

    const auto graph = buildGreetingGraph(2, 5, 3);
    assert(graph.size() == 10);
    assert(graph.back().senderIndex == 1 && graph.back().recipientIndex == 4);
    assert(graph.back().templateIndex == 2);

    const auto plan = planGift(500, 5, catalog, 2, 3);
    assert(plan.verify());
    assert(plan.allocations == std::vector<std::uint32_t>({100, 100, 100, 100, 100}));
    assert(plan.baskets.size() == 5 && plan.greetingGraph.size() == 10);
    assert(plan.spent + plan.remaining == 500);
    auto inconsistent = plan;
    ++inconsistent.spent;
    assert(!inconsistent.verify());

    bool rejected = false;
    try { (void)splitWeighted(500, {0, 0}); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    rejected = false;
    try { (void)combinationCount(4, 5); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    return 0;
}
