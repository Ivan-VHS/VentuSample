from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from tools.quality_remediation import (
    CORRECTIONS,
    COVERAGE,
    DEFECTS,
    QualityRemediationError,
    assess_profile,
)

ROOT = Path(__file__).resolve().parents[1]
PROFILE = ROOT / "quality-remediation.v1.json"
SCHEMA = ROOT / "schemas" / "quality-remediation.v1.schema.json"


class QualityRemediationTests(unittest.TestCase):
    def test_all_twenty_triplets_and_evidence_paths_are_complete(self):
        report = assess_profile(PROFILE, root=ROOT)
        self.assertEqual(report["criteria_count"], 20)
        self.assertTrue(report["term_triplets_preserved"])
        self.assertEqual(report["evidence"]["files_missing"], 0)
        self.assertEqual(report["evidence"]["coverage_percent"], 100.0)
        self.assertEqual(report["issues"], [])

    def test_all_supplied_terms_are_preserved_in_order(self):
        profile = json.loads(PROFILE.read_text(encoding="utf-8"))
        self.assertEqual(len(profile["entries"]), 20)
        self.assertEqual([row["terms"]["correction"] for row in profile["entries"]], CORRECTIONS)
        self.assertEqual([row["terms"]["coverage"] for row in profile["entries"]], COVERAGE)
        self.assertEqual([row["terms"]["defect"] for row in profile["entries"]], DEFECTS)
        self.assertEqual(COVERAGE[4], COVERAGE[17])

    def test_missing_evidence_is_reported_without_masking_other_paths(self):
        profile = json.loads(PROFILE.read_text(encoding="utf-8"))
        profile["entries"][0]["evidence"] = ["missing/defect-evidence.txt"]
        report = assess_profile(profile, root=ROOT)
        self.assertEqual(report["evidence"]["files_missing"], 1)
        self.assertTrue(any("missing/defect-evidence.txt" in issue for issue in report["issues"]))

    def test_path_traversal_is_rejected(self):
        profile = json.loads(PROFILE.read_text(encoding="utf-8"))
        profile["entries"][0]["evidence"] = ["../outside.txt"]
        with self.assertRaises(QualityRemediationError):
            assess_profile(profile, root=ROOT)

    def test_windows_absolute_paths_are_rejected(self):
        profile = json.loads(PROFILE.read_text(encoding="utf-8"))
        profile["entries"][0]["evidence"] = ["C:\\outside.txt"]
        with self.assertRaises(QualityRemediationError):
            assess_profile(profile, root=ROOT)

    def test_duplicate_control_ids_are_rejected(self):
        profile = json.loads(PROFILE.read_text(encoding="utf-8"))
        profile["entries"][1]["id"] = profile["entries"][0]["id"]
        with self.assertRaises(QualityRemediationError):
            assess_profile(profile, root=ROOT)

    def test_schema_exposes_exact_twenty_row_vocabulary(self):
        schema = json.loads(SCHEMA.read_text(encoding="utf-8"))
        entries = schema["properties"]["entries"]
        self.assertEqual(entries["minItems"], 20)
        self.assertEqual(entries["maxItems"], 20)
        terms = entries["items"]["properties"]["terms"]["properties"]
        self.assertEqual(terms["correction"]["enum"], CORRECTIONS)
        self.assertEqual(terms["coverage"]["enum"], COVERAGE)
        self.assertEqual(terms["defect"]["enum"], DEFECTS)


if __name__ == "__main__":
    unittest.main()
