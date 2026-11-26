"""Regression tests for the declared resilience/trust policy."""
from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from validate_resilience_policy import PROFILE_PATH, SCHEMA_PATH, load_policy, validate_policy  # noqa: E402


class ResiliencePolicyTests(unittest.TestCase):
    def setUp(self) -> None:
        self.policy = load_policy()

    def test_repository_profile_is_valid(self) -> None:
        self.assertEqual(validate_policy(self.policy), [])

    def test_policy_and_schema_are_valid_json(self) -> None:
        schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
        self.assertEqual(schema["$schema"], "https://json-schema.org/draft/2020-12/schema")
        self.assertTrue(set(schema["required"]).issubset(self.policy))
        self.assertTrue(PROFILE_PATH.is_file())

    def test_contract_status_does_not_claim_runtime_implementation(self) -> None:
        self.assertEqual(self.policy["implementation_status"], "contract_only")

    def test_policy_bypass_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["trust"]["policy_bypass_allowed"] = True
        self.assertTrue(any("trust.policy_bypass_allowed" in error for error in validate_policy(candidate)))

    def test_unverified_upstream_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["trust"]["allow_unverified_upstream"] = True
        self.assertTrue(any("trust.allow_unverified_upstream" in error for error in validate_policy(candidate)))

    def test_destructive_actions_require_human_approval(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["escalation"]["destructive_action_human_approval"] = False
        self.assertTrue(any("destructive_action_human_approval" in error for error in validate_policy(candidate)))

    def test_retry_budget_is_bounded(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["resilience"]["retry"]["max_attempts"] = 99
        self.assertTrue(any("max_attempts" in error for error in validate_policy(candidate)))

    def test_manual_override_must_expire_and_be_audited(self) -> None:
        candidate = copy.deepcopy(self.policy)
        candidate["manual_override"]["audit_required"] = False
        self.assertTrue(any("manual_override.audit_required" in error for error in validate_policy(candidate)))
        candidate["manual_override"]["audit_required"] = True
        candidate["manual_override"]["expiry_seconds"] = 0
        self.assertTrue(any("expiry_seconds" in error for error in validate_policy(candidate)))


if __name__ == "__main__":
    unittest.main()
