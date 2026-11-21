#pragma once

#include <cstdint>
#include <string_view>

namespace ventu::careguard {

enum class LoadStatus { normal, watch, pause, invalid };
enum class SupportPath { invalid, trusted_person, peer_support, professional, urgent_human };

struct WorkloadSnapshot {
    std::uint32_t queued_items{0};
    std::uint32_t failed_checks{0};
    std::uint32_t alerts_per_hour{0};
    std::uint32_t active_workers{0};
    std::uint32_t worker_capacity{1};
};

struct Assessment {
    LoadStatus status{LoadStatus::invalid};
    std::uint32_t pressure_score{0};
    std::uint32_t reserve_percent{0};
};

Assessment careguard_assess_project_load(const WorkloadSnapshot& snapshot) noexcept;
std::uint32_t careguard_review_language(std::string_view text) noexcept;
bool careguard_accept_help_request(std::string_view message) noexcept;
SupportPath careguard_select_support_path(std::string_view path) noexcept;
bool careguard_review_contextual_claim(std::string_view claim, std::string_view evidence_ref) noexcept;
bool careguard_check_clinical_boundary(std::string_view action) noexcept;
bool careguard_allocate_resilience_buffer(std::uint32_t capacity_units, std::uint32_t reserve_percent,
                                          std::uint32_t& reserved_units, std::uint32_t& usable_units) noexcept;

} // namespace ventu::careguard
