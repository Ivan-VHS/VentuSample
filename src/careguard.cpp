#include "careguard.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>

namespace ventu::careguard {
namespace {
bool contains_ci(std::string_view text, std::string_view needle) noexcept {
    if (needle.empty()) return false;
    for (std::size_t i = 0; i + needle.size() <= text.size(); ++i) {
        bool same = true;
        for (std::size_t j = 0; j < needle.size(); ++j) {
            const auto a = static_cast<unsigned char>(text[i + j]);
            const auto b = static_cast<unsigned char>(needle[j]);
            if (std::tolower(a) != std::tolower(b)) { same = false; break; }
        }
        if (same) return true;
    }
    return false;
}
bool nonempty(std::string_view text) noexcept {
    return std::any_of(text.begin(), text.end(), [](unsigned char c) { return !std::isspace(c); });
}
} // namespace

Assessment careguard_assess_project_load(const WorkloadSnapshot& s) noexcept {
    if (s.worker_capacity == 0) return {};
    std::uint32_t score = 0;
    if (s.queued_items >= 16) score += 25;
    else if (s.queued_items >= 8) score += 12;
    if (s.failed_checks >= 3) score += 30;
    else if (s.failed_checks > 0) score += 12;
    if (s.alerts_per_hour >= 20) score += 30;
    else if (s.alerts_per_hour >= 8) score += 12;
    const double utilization = static_cast<double>(s.active_workers) /
                              static_cast<double>(s.worker_capacity);
    if (utilization >= 1.0) score += 25;
    else if (utilization >= 0.85) score += 12;
    Assessment out;
    out.pressure_score = std::min(score, 100u);
    if (score >= 60) { out.status = LoadStatus::pause; out.reserve_percent = 30; }
    else if (score >= 25) { out.status = LoadStatus::watch; out.reserve_percent = 20; }
    else { out.status = LoadStatus::normal; out.reserve_percent = 10; }
    return out;
}

std::uint32_t careguard_review_language(std::string_view text) noexcept {
    constexpr std::array<std::string_view, 8> phrases{
        "weakness", "lazy", "attention seeking", "just snap out of it",
        "слабость", "слабкість", "ленивый", "лінивий"
    };
    return static_cast<std::uint32_t>(std::count_if(phrases.begin(), phrases.end(),
        [text](std::string_view phrase) { return contains_ci(text, phrase); }));
}

bool careguard_accept_help_request(std::string_view message) noexcept {
    return nonempty(message);
}

SupportPath careguard_select_support_path(std::string_view path) noexcept {
    if (path == "trusted_person") return SupportPath::trusted_person;
    if (path == "peer_support") return SupportPath::peer_support;
    if (path == "professional") return SupportPath::professional;
    if (path == "urgent_human") return SupportPath::urgent_human;
    return SupportPath::invalid;
}

bool careguard_review_contextual_claim(std::string_view claim, std::string_view evidence_ref) noexcept {
    return nonempty(claim) && nonempty(evidence_ref);
}

bool careguard_check_clinical_boundary(std::string_view action) noexcept {
    constexpr std::array<std::string_view, 10> blocked{
        "diagnose", "diagnosis", "prescribe", "prescription", "medication",
        "antidepressant", "dosage", "поставить диагноз", "поставити діагноз",
        "назначить препарат"
    };
    if (!nonempty(action)) return false;
    return std::none_of(blocked.begin(), blocked.end(),
        [action](std::string_view term) { return contains_ci(action, term); });
}

bool careguard_allocate_resilience_buffer(std::uint32_t capacity, std::uint32_t percent,
                                          std::uint32_t& reserved, std::uint32_t& usable) noexcept {
    if (percent > 50) return false;
    const std::uint64_t amount =
        (static_cast<std::uint64_t>(capacity) * percent + 99u) / 100u;
    if (amount > capacity) return false;
    reserved = static_cast<std::uint32_t>(amount);
    usable = capacity - reserved;
    return true;
}

} // namespace ventu::careguard
