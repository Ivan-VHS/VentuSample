# Occasion Allocation Core

VentuSample's C++20 core includes an independent integer-only occasion planner. It can be used by future desktop panels or host integrations without tying monetary logic to audio rendering.

## Capabilities

- Exact equal and largest-remainder weighted budget splits.
- Integer percentage calculations and exact bounded combination counts.
- Stock-aware candy assortment planning, ordered by preference-to-price ratio with stable input-order ties.
- A deterministic directed sender/recipient greeting graph with rotating template IDs.
- A composite `GiftPlan` whose `verify()` method checks budget conservation and per-recipient caps.

Money values are integer units. Use whole hryvnias or kopiykas consistently. No floating-point calculation is used for budget allocation.

The candy selector is intentionally a transparent greedy heuristic; it is not advertised as a globally optimal knapsack solver. It never exceeds the recipient's allocation or available stock.

## Example

```cpp
#include "occasion_planner.hpp"

using namespace ventu::occasion;

const auto plan = planGift(500, 5, {
    {"Praline", "chocolate", 18, 30, 5, 14},
    {"Fruit caramel", "caramel", 8, 40, 2, 10},
}, 1, 3);

if (!plan.verify()) {
    // Reject an inconsistent plan before rendering/exporting it.
}
```

## Validation

The `ventusample-occasion-planner` CTest executable covers integer splits, proportional remainder handling, percentages, combinations, basket caps, greeting edges and the conservation invariant.
