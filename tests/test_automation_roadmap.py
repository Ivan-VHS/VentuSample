"""Regression tests for explicit not-applicable automation roadmap workstreams."""
from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from scripts.validate_automation_roadmap import validate_task_status  # noqa: E402


class RoadmapStatusTests(unittest.TestCase):
    def test_active_extend_and_planned_remain_valid(self):
        for status in ("active", "extend", "planned"):
            with self.subTest(status=status):
                self.assertEqual(validate_task_status({"id": "sample", "status": status}), [])

    def test_not_applicable_requires_a_reason(self):
        errors = validate_task_status({"id": "14-coordinate-deployment", "status": "not_applicable"})
        self.assertEqual(len(errors), 1)
        self.assertIn("explain why", errors[0])

    def test_not_applicable_accepts_a_specific_reason(self):
        task = {
            "id": "14-coordinate-deployment",
            "status": "not_applicable",
            "not_applicable_reason": (
                "This product is a local desktop tool with no hosted service or production runtime. "
                "Desktop archive distribution is not service deployment."
            ),
        }
        self.assertEqual(validate_task_status(task), [])

    def test_short_not_applicable_reason_is_rejected(self):
        task = {
            "id": "14-coordinate-deployment",
            "status": "not_applicable",
            "not_applicable_reason": "Out of scope",
        }
        self.assertEqual(len(validate_task_status(task)), 1)

    def test_unknown_status_is_rejected(self):
        self.assertEqual(
            len(validate_task_status({"id": "sample", "status": "done"})),
            1,
        )

    def test_committed_schema_requires_reason_for_not_applicable(self):
        schema = json.loads((ROOT / "automation-roadmap.v1.schema.json").read_text(encoding="utf-8"))
        task_schema = schema["properties"]["tasks"]["items"]
        self.assertIn("not_applicable", task_schema["properties"]["status"]["enum"])
        self.assertIn("not_applicable_reason", task_schema["properties"])
        condition = task_schema["allOf"][0]
        self.assertEqual(condition["if"]["properties"]["status"]["const"], "not_applicable")
        self.assertIn("not_applicable_reason", condition["then"]["required"])

    def test_current_local_deployment_workstream_has_explicit_rationale(self):
        roadmap = json.loads((ROOT / "automation-roadmap.v1.json").read_text(encoding="utf-8"))
        task = next(item for item in roadmap["tasks"] if item["id"] == "14-coordinate-deployment")
        self.assertEqual(task["status"], "not_applicable")
        self.assertEqual(validate_task_status(task), [])


if __name__ == "__main__":
    unittest.main()
