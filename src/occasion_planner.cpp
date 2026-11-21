#include "occasion_planner.hpp"

#include <algorithm>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace ventu::occasion {
namespace {

void validateBudget(std::uint32_t budget) {
    if (budget > kMaxBudget) throw std::invalid_argument("budget exceeds the configured limit");
}

void validateCount(std::size_t value, std::size_t maximum, const char* label) {
    if (value == 0 || value > maximum)
        throw std::invalid_argument(std::string(label) + " is outside the supported range");
}

bool betterRatio(const CandyOption& left, std::size_t leftIndex,
                 const CandyOption& right, std::size_t rightIndex) {
    const auto lhs = static_cast<std::uint64_t>(left.preference) * right.unitPrice;
    const auto rhs = static_cast<std::uint64_t>(right.preference) * left.unitPrice;
    if (lhs != rhs) return lhs > rhs;
    return leftIndex < rightIndex;
}

} // namespace

bool GiftPlan::verify() const noexcept {
    if (remaining > budget || spent + remaining != budget ||
        allocations.size() != baskets.size()) return false;
    std::uint64_t allocationTotal = 0;
    std::uint64_t basketSpent = 0;
    for (std::size_t i = 0; i < allocations.size(); ++i) {
        allocationTotal += allocations[i];
        std::uint64_t basketTotal = 0;
        for (const auto& line : baskets[i]) basketTotal += line.lineTotal;
        if (basketTotal > allocations[i]) return false;
        basketSpent += basketTotal;
    }
    return allocationTotal == budget && basketSpent == spent && spent <= budget;
}

std::vector<std::uint32_t> splitEvenly(std::uint32_t total, std::size_t parts) {
    validateCount(parts, kMaxRecipients, "parts");
    const auto base = total / static_cast<std::uint32_t>(parts);
    const auto remainder = total % static_cast<std::uint32_t>(parts);
    std::vector<std::uint32_t> shares(parts, base);
    for (std::size_t i = 0; i < remainder; ++i) ++shares[i];
    return shares;
}

std::vector<std::uint32_t> splitWeighted(
    std::uint32_t total, const std::vector<std::uint32_t>& weights) {
    validateCount(weights.size(), kMaxRecipients, "weights");
    std::uint64_t weightSum = 0;
    for (const auto weight : weights) weightSum += weight;
    if (weightSum == 0) throw std::invalid_argument("at least one weight must be positive");

    std::vector<std::uint32_t> shares(weights.size(), 0);
    std::vector<std::uint64_t> remainders(weights.size(), 0);
    std::uint64_t assigned = 0;
    for (std::size_t i = 0; i < weights.size(); ++i) {
        const auto product = static_cast<std::uint64_t>(total) * weights[i];
        shares[i] = static_cast<std::uint32_t>(product / weightSum);
        remainders[i] = product % weightSum;
        assigned += shares[i];
    }
    auto left = static_cast<std::size_t>(static_cast<std::uint64_t>(total) - assigned);
    std::vector<bool> awarded(weights.size(), false);
    while (left > 0) {
        std::size_t best = weights.size();
        for (std::size_t i = 0; i < weights.size(); ++i) {
            if (awarded[i]) continue;
            if (best == weights.size() || remainders[i] > remainders[best]) best = i;
        }
        if (best == weights.size()) throw std::logic_error("largest-remainder allocation failed");
        ++shares[best];
        awarded[best] = true;
        --left;
    }
    return shares;
}

std::uint32_t percentOf(std::uint32_t total, std::uint32_t percent) {
    if (percent > 100) throw std::invalid_argument("percent must be from 0 to 100");
    return static_cast<std::uint32_t>((static_cast<std::uint64_t>(total) * percent) / 100u);
}

std::uint64_t combinationCount(std::uint32_t n, std::uint32_t k) {
    if (k > n || n > 1'000'000u) throw std::invalid_argument("require 0 <= k <= n <= 1000000");
    k = std::min(k, n - k);
    std::uint64_t result = 1;
    for (std::uint32_t i = 1; i <= k; ++i) {
        auto numerator = static_cast<std::uint64_t>(n - k + i);
        auto denominator = static_cast<std::uint64_t>(i);
        auto common = std::gcd(numerator, denominator);
        numerator /= common;
        denominator /= common;
        common = std::gcd(result, denominator);
        result /= common;
        denominator /= common;
        if (denominator != 1 || (numerator != 0 && result > std::numeric_limits<std::uint64_t>::max() / numerator))
            throw std::overflow_error("combination count exceeds uint64 range");
        result *= numerator;
    }
    return result;
}

std::pair<std::vector<CandyLine>, std::uint32_t> buildCandyBasket(
    std::uint32_t budget, const std::vector<CandyOption>& catalog) {
    validateBudget(budget);
    if (catalog.empty() || catalog.size() > kMaxCandyTypes)
        throw std::invalid_argument("catalog must contain 1 to 256 candy types");
    for (const auto& candy : catalog) {
        if (candy.name.empty() || candy.category.empty() || candy.unitPrice == 0 || candy.preference == 0)
            throw std::invalid_argument("candy names, categories, price and preference must be valid");
    }

    std::vector<std::size_t> order(catalog.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        return betterRatio(catalog[a], a, catalog[b], b);
    });

    std::uint32_t remaining = budget;
    std::vector<CandyLine> lines;
    for (const auto index : order) {
        const auto& candy = catalog[index];
        const auto quantity = std::min(candy.stock, remaining / candy.unitPrice);
        if (quantity == 0) continue;
        const auto lineTotal = static_cast<std::uint64_t>(quantity) * candy.unitPrice;
        lines.push_back(CandyLine{
            index, candy.name, candy.category, candy.unitPrice, quantity,
            lineTotal, static_cast<std::uint64_t>(quantity) * candy.weightGrams
        });
        remaining -= static_cast<std::uint32_t>(lineTotal);
    }
    return {std::move(lines), budget - remaining};
}

std::vector<GreetingEdge> buildGreetingGraph(
    std::size_t senderCount, std::size_t recipientCount, std::size_t templateCount) {
    validateCount(senderCount, kMaxRecipients, "sender count");
    validateCount(recipientCount, kMaxRecipients, "recipient count");
    validateCount(templateCount, kMaxRecipients, "template count");
    if (senderCount > kMaxGreetingEdges / recipientCount)
        throw std::invalid_argument("greeting graph exceeds the edge limit");
    std::vector<GreetingEdge> edges;
    edges.reserve(senderCount * recipientCount);
    for (std::size_t sender = 0; sender < senderCount; ++sender) {
        for (std::size_t recipient = 0; recipient < recipientCount; ++recipient) {
            edges.push_back({sender, recipient, (sender + recipient) % templateCount});
        }
    }
    return edges;
}

GiftPlan planGift(
    std::uint32_t budget, std::size_t recipientCount,
    const std::vector<CandyOption>& catalog, std::size_t senderCount,
    std::size_t templateCount, const std::vector<std::uint32_t>& weights) {
    validateBudget(budget);
    validateCount(recipientCount, kMaxRecipients, "recipient count");
    GiftPlan plan;
    plan.budget = budget;
    plan.allocations = weights.empty() ? splitEvenly(budget, recipientCount)
                                       : splitWeighted(budget, weights);
    if (plan.allocations.size() != recipientCount)
        throw std::invalid_argument("weights must contain one entry per recipient");
    auto available = catalog;
    plan.baskets.reserve(recipientCount);
    for (const auto share : plan.allocations) {
        auto basket = buildCandyBasket(share, available);
        plan.spent += basket.second;
        plan.baskets.push_back(std::move(basket.first));
        for (const auto& line : plan.baskets.back()) {
            auto& stock = available[line.catalogIndex].stock;
            stock -= line.quantity;
        }
    }
    plan.remaining = budget - static_cast<std::uint32_t>(plan.spent);
    plan.greetingGraph = buildGreetingGraph(senderCount, recipientCount, templateCount);
    if (!plan.verify()) throw std::logic_error("gift plan violated its conservation invariant");
    return plan;
}

} // namespace ventu::occasion
