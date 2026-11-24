"""Regression tests for roadmap governance and CI scale validators."""
from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from tools.validate_delivery_contracts import (  # noqa: E402
    GOVERNANCE,
    GOVERNANCE_SCHEMA,
    SCALE,
    SCALE_SCHEMA,
    load_json,
    validate_governance,
    validate_scale,
    validate_workflow_scale,
)

REPOSITORY = "Ivan-VHS/Pytyliki"


class DeliveryContractTests(unittest.TestCase):
    def test_committed_governance_and_scale_policies_are_valid(self):
        governance = load_json(GOVERNANCE)
        scale = load_json(SCALE)
        self.assertEqual(validate_governance(governance, REPOSITORY), [])
        self.assertEqual(validate_scale(scale, REPOSITORY), [])

    def test_committed_schemas_use_draft_2020_12_and_require_core_fields(self):
        for path, policy in (
            (GOVERNANCE_SCHEMA, load_json(GOVERNANCE)),
            (SCALE_SCHEMA, load_json(SCALE)),
        ):
            with self.subTest(schema=path.name):
                schema = load_json(path)
                self.assertEqual(schema["$schema"], "https://json-schema.org/draft/2020-12/schema")
                self.assertTrue(set(schema["required"]).issubset(policy))

    def test_disabled_status_guard_is_rejected(self):
        candidate = load_json(GOVERNANCE)
        candidate["status_rules"]["active_requires_existing_evidence"] = False
        self.assertTrue(validate_governance(candidate, REPOSITORY))

    def test_missing_priority_is_rejected(self):
        candidate = load_json(GOVERNANCE)
        candidate["priorities"].pop("P2")
        self.assertTrue(validate_governance(candidate, REPOSITORY))

    def test_wrong_repository_is_rejected(self):
        self.assertTrue(validate_governance(load_json(GOVERNANCE), "other/project"))

    def test_matrix_requires_bounded_parallelism(self):
        workflow = """concurrency:
  group: test-${{ github.ref }}
  cancel-in-progress: true
jobs:
  test:
    runs-on: ubuntu-latest
    timeout-minutes: 10
    strategy:
      matrix:
        python: [3.10, 3.11]
    steps: []
"""
        errors = validate_workflow_scale(workflow, "test.yml", {
            "max_jobs": 1, "max_job_timeout_minutes": 20, "max_parallel_matrix": 2
        })
        self.assertTrue(any("max-parallel" in error for error in errors))

    def test_workflow_without_concurrency_or_timeout_is_rejected(self):
        workflow = """jobs:
  build:
    runs-on: ubuntu-latest
    steps: []
"""
        errors = validate_workflow_scale(workflow, "test.yml", {
            "max_jobs": 1, "max_job_timeout_minutes": 20, "max_parallel_matrix": 1
        })
        self.assertTrue(any("concurrency" in error for error in errors))
        self.assertTrue(any("timeout-minutes" in error for error in errors))

    def test_job_count_over_budget_is_rejected(self):
        workflow = """concurrency:
  group: test-${{ github.ref }}
  cancel-in-progress: true
jobs:
  first:
    runs-on: ubuntu-latest
    timeout-minutes: 5
  second:
    runs-on: ubuntu-latest
    timeout-minutes: 5
"""
        errors = validate_workflow_scale(workflow, "test.yml", {
            "max_jobs": 1, "max_job_timeout_minutes": 10, "max_parallel_matrix": 1
        })
        self.assertTrue(any("jobs exceeds limit" in error for error in errors))

    def test_timeout_over_budget_is_rejected(self):
        workflow = """concurrency:
  group: test-${{ github.ref }}
  cancel-in-progress: true
jobs:
  build:
    runs-on: ubuntu-latest
    timeout-minutes: 40
    steps: []
"""
        errors = validate_workflow_scale(workflow, "test.yml", {
            "max_jobs": 1, "max_job_timeout_minutes": 30, "max_parallel_matrix": 1
        })
        self.assertTrue(any("exceeds" in error for error in errors))

    def test_global_limits_with_wrong_types_fail_without_crashing(self):
        candidate = load_json(SCALE)
        candidate["global_limits"]["max_jobs_per_workflow"] = "unbounded"
        self.assertTrue(validate_scale(candidate, REPOSITORY))


if __name__ == "__main__":
    unittest.main()
