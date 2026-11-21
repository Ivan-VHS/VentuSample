#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace ventu::occasion {

inline constexpr std::size_t kMaxRecipients = 256;
inline constexpr std::size_t kMaxCandyTypes = 256;
inline constexpr std::size_t kMaxGreetingEdges = 65'536;
inline constexpr std::uint32_t kMaxBudget = 10'000'000;

struct CandyOption {
    std::string name;
    std::string category;
    std::uint32_t unitPrice{0};
    std::uint32_t stock{0};
    std::uint32_t preference{1};
    std::uint32_t weightGrams{0};
};

struct CandyLine {
    std::size_t catalogIndex{0};
    std::string name;
    std::string category;
    std::uint32_t unitPrice{0};
    std::uint32_t quantity{0};
    std::uint64_t lineTotal{0};
    std::uint64_t weightGrams{0};
};

struct GreetingEdge {
    std::size_t senderIndex{0};
    std::size_t recipientIndex{0};
    std::size_t templateIndex{0};
};

struct GiftPlan {
    std::uint32_t budget{0};
    std::vector<std::uint32_t> allocations;
    std::vector<std::vector<CandyLine>> baskets;
    std::uint64_t spent{0};
    std::uint32_t remaining{0};
    std::vector<GreetingEdge> greetingGraph;

    bool verify() const noexcept;
};

/* Exact integer shares; ties for indivisible remainder are stable by index. */
std::vector<std::uint32_t> splitEvenly(std::uint32_t total, std::size_t parts);
std::vector<std::uint32_t> splitWeighted(
    std::uint32_t total, const std::vector<std::uint32_t>& weights);
std::uint32_t percentOf(std::uint32_t total, std::uint32_t percent);
std::uint64_t combinationCount(std::uint32_t n, std::uint32_t k);

/* Explainable preference/price greedy selection, bounded by stock and budget.
 * This is a heuristic, not a claim of optimal knapsack selection. */
std::pair<std::vector<CandyLine>, std::uint32_t> buildCandyBasket(
    std::uint32_t budget, const std::vector<CandyOption>& catalog);

std::vector<GreetingEdge> buildGreetingGraph(
    std::size_t senderCount, std::size_t recipientCount, std::size_t templateCount);

GiftPlan planGift(
    std::uint32_t budget,
    std::size_t recipientCount,
    const std::vector<CandyOption>& catalog,
    std::size_t senderCount = 1,
    std::size_t templateCount = 1,
    const std::vector<std::uint32_t>& weights = {});

} // namespace ventu::occasion
