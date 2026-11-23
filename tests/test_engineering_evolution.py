"""Contract tests for the Engineering Evolution Framework."""
from __future__ import annotations
import copy
import json
import sys
import unittest
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from validate_engineering_evolution import POLICY_PATH, SCHEMA_PATH, validate_policy  # noqa: E402

class EngineeringEvolutionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.policy = json.loads(POLICY_PATH.read_text(encoding="utf-8"))
        cls.schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))

    def test_reference_manifest_passes(self) -> None:
        self.assertEqual(validate_policy(self.policy, self.schema), [])

    def test_exactly_twenty_domains_are_required(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["pillars"].pop()
        self.assertTrue(any("exactly 20" in x for x in validate_policy(candidate, self.schema)))

    def test_canonical_term_order_is_enforced(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["pillars"][0]["terms"][1] = "Калибровка"
        self.assertTrue(any("canonical ordered triple" in x for x in validate_policy(candidate, self.schema)))

    def test_duplicate_domain_ids_are_rejected(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["pillars"][1]["id"] = candidate["pillars"][0]["id"]
        self.assertTrue(any("duplicate pillar id" in x for x in validate_policy(candidate, self.schema)))

    def test_each_domain_needs_two_methods(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["pillars"][0]["practices"].pop()
        self.assertTrue(any("exactly two practice records" in x for x in validate_policy(candidate, self.schema)))

    def test_acceptance_checks_must_be_testable(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["pillars"][0]["acceptance"] = ["ok"]
        self.assertTrue(any("at least two testable checks" in x for x in validate_policy(candidate, self.schema)))

    def test_safety_guardrails_are_present(self) -> None:
        joined = " ".join(self.policy["guardrails"]).lower()
        self.assertIn("telemetry", joined)
        self.assertIn("untrusted", joined)
        self.assertIn("rollback", joined)

    def test_exactly_twenty_execution_directives_are_required(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["execution_directives"].pop()
        self.assertTrue(any("exactly 20 execution directives" in x for x in validate_policy(candidate, self.schema)))

    def test_canonical_directive_wording_and_order_are_enforced(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["execution_directives"][0]["source_text"] = "Разработать всё сразу"
        self.assertTrue(any("canonical wording and order" in x for x in validate_policy(candidate, self.schema)))

    def test_directive_domain_mapping_is_enforced(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["execution_directives"][0]["domains"] = ["perspective"]
        self.assertTrue(any("canonical domain mapping" in x for x in validate_policy(candidate, self.schema)))

    def test_directives_need_testable_acceptance_checks(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["execution_directives"][0]["acceptance"] = ["ok"]
        self.assertTrue(any("two or three testable checks" in x for x in validate_policy(candidate, self.schema)))

    def test_duplicate_directive_ids_are_rejected(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["execution_directives"][1]["id"] = candidate["execution_directives"][0]["id"]
        self.assertTrue(any("duplicate execution directive id" in x for x in validate_policy(candidate, self.schema)))

if __name__ == "__main__":
    unittest.main()
