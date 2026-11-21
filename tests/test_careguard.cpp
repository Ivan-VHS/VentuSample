#include "careguard.hpp"

#include <cassert>

int main() {
    using namespace ventu::careguard;
    const auto empty = careguard_assess_project_load({});
    const auto overloaded = careguard_assess_project_load({20, 4, 30, 12, 10});
    assert(empty.status == LoadStatus::normal);
    assert(overloaded.status == LoadStatus::pause);
    assert(careguard_review_language("That is weakness") == 1);
    assert(careguard_accept_help_request("I need help"));
    assert(!careguard_accept_help_request("  "));
    assert(careguard_select_support_path("urgent_human") == SupportPath::urgent_human);
    assert(careguard_select_support_path("auto_call") == SupportPath::invalid);
    assert(careguard_review_contextual_claim("A claim", "ci://run/42"));
    assert(!careguard_review_contextual_claim("A claim", ""));
    assert(!careguard_check_clinical_boundary("prescribe antidepressant"));
    assert(careguard_check_clinical_boundary("pause render queue"));
    std::uint32_t reserved = 0, usable = 0;
    assert(careguard_allocate_resilience_buffer(10, 20, reserved, usable));
    assert(reserved == 2 && usable == 8);
    assert(!careguard_allocate_resilience_buffer(10, 90, reserved, usable));
    return 0;
}
