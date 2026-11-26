"""Regression tests for the bounded, manual CI rerun gate."""
from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from validate_safe_rerun import load_json, validate_policy, validate_run  # noqa: E402

REPOSITORY = "Ivan-VHS/VentuSample"
POLICY = {
    "schema": "anatidae.safe-ci-rerun.v1",
    "repository": REPOSITORY,
    "max_attempts": 2,
    "allowed_events": ["push", "pull_request", "schedule", "workflow_dispatch"],
    "allowed_conclusions": ["failure", "timed_out"],
    "allowed_workflow_names": ["CI validation", "Automation Roadmap Contract"],
    "same_repository_only": True,
}
RUN = {
    "id": 123456,
    "name": "CI validation",
    "event": "pull_request",
    "status": "completed",
    "conclusion": "failure",
    "run_attempt": 1,
    "head_sha": "a" * 40,
    "head_branch": "feature/example",
    "html_url": f"https://github.com/{REPOSITORY}/actions/runs/123456",
    "repository": {"full_name": REPOSITORY},
    "head_repository": {"full_name": REPOSITORY},
}


class SafeRerunPolicyTests(unittest.TestCase):
    def test_committed_policy_and_schema_are_valid(self) -> None:
        committed = load_json(ROOT / ".architecture" / "automation-rerun-policy.v1.json")
        schema = load_json(ROOT / ".architecture" / "automation-rerun-policy.v1.schema.json")
        self.assertEqual(schema["$schema"], "https://json-schema.org/draft/2020-12/schema")
        self.assertEqual(validate_policy(committed), [])
        self.assertEqual(committed["repository"], REPOSITORY)

    def test_fixture_policy_is_valid(self) -> None:
        self.assertEqual(validate_policy(POLICY), [])

    def test_valid_same_repository_failed_run_is_eligible(self) -> None:
        self.assertEqual(validate_run(RUN, POLICY), [])

    def test_fork_pull_request_is_rejected(self) -> None:
        candidate = copy.deepcopy(RUN)
        candidate["head_repository"]["full_name"] = "contributor/project"
        self.assertTrue(any("fork" in error for error in validate_run(candidate, POLICY)))

    def test_wrong_repository_is_rejected(self) -> None:
        candidate = copy.deepcopy(RUN)
        candidate["repository"]["full_name"] = "other/project"
        self.assertTrue(validate_run(candidate, POLICY))

    def test_unlisted_workflow_is_rejected(self) -> None:
        candidate = copy.deepcopy(RUN)
        candidate["name"] = "Release publication"
        self.assertTrue(any("allowlist" in error for error in validate_run(candidate, POLICY)))

    def test_success_cancelled_and_action_required_are_rejected(self) -> None:
        for conclusion in ("success", "cancelled", "action_required"):
            with self.subTest(conclusion=conclusion):
                candidate = copy.deepcopy(RUN)
                candidate["conclusion"] = conclusion
                self.assertTrue(validate_run(candidate, POLICY))

    def test_in_progress_run_is_rejected(self) -> None:
        candidate = copy.deepcopy(RUN)
        candidate["status"] = "in_progress"
        self.assertTrue(validate_run(candidate, POLICY))

    def test_only_one_retry_is_permitted(self) -> None:
        candidate = copy.deepcopy(RUN)
        candidate["run_attempt"] = 2
        self.assertTrue(any("retry ceiling" in error for error in validate_run(candidate, POLICY)))

    def test_bad_sha_and_run_id_are_rejected(self) -> None:
        candidate = copy.deepcopy(RUN)
        candidate["head_sha"] = "not-a-commit"
        candidate["id"] = -1
        self.assertTrue(validate_run(candidate, POLICY))

    def test_unsupported_event_is_rejected(self) -> None:
        candidate = copy.deepcopy(RUN)
        candidate["event"] = "workflow_run"
        self.assertTrue(validate_run(candidate, POLICY))

    def test_policy_cannot_expand_retry_ceiling(self) -> None:
        candidate = copy.deepcopy(POLICY)
        candidate["max_attempts"] = 5
        self.assertTrue(validate_policy(candidate))

    def test_policy_cannot_allow_cross_repository_runs(self) -> None:
        candidate = copy.deepcopy(POLICY)
        candidate["same_repository_only"] = False
        self.assertTrue(validate_policy(candidate))


if __name__ == "__main__":
    unittest.main()
