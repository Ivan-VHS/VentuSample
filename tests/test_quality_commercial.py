from __future__ import annotations
import json, tempfile, unittest
from pathlib import Path
from tools.quality_commercial import QualityProfileError, assess_profile

ROOT = Path(__file__).resolve().parents[1]
PROFILE = ROOT / "quality-commercial.v1.json"


class QualityCommercialTests(unittest.TestCase):
    def test_all_sixty_controls_have_existing_evidence(self):
        report = assess_profile(PROFILE, root=ROOT)
        self.assertEqual(report["criteria_count"], 20)
        self.assertEqual(report["evidence"]["files_missing"], 0)
        self.assertEqual(report["evidence"]["coverage_percent"], 100.0)
        self.assertEqual(set(report["axes"]), {"quality", "execution", "commercial"})

    def test_all_three_original_terms_are_preserved(self):
        profile = json.loads(PROFILE.read_text(encoding="utf-8"))
        for row in profile["criteria"]:
            self.assertEqual(set(row["terms"]), {"quality", "execution", "commercial"})
            self.assertTrue(all(row["terms"].values()))

    def test_missing_evidence_is_reported(self):
        profile = json.loads(PROFILE.read_text(encoding="utf-8"))
        profile["criteria"][0]["controls"]["quality"]["evidence"] = ["missing/evidence.txt"]
        report = assess_profile(profile, root=ROOT)
        self.assertTrue(report["issues"])
        self.assertEqual(report["evidence"]["files_missing"], 1)
        self.assertTrue(any("missing/evidence.txt" in issue for issue in report["issues"]))

    def test_path_traversal_is_rejected(self):
        profile = json.loads(PROFILE.read_text(encoding="utf-8"))
        profile["criteria"][0]["controls"]["quality"]["evidence"] = ["../outside.txt"]
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(QualityProfileError):
                assess_profile(profile, root=directory)

    def test_unverified_business_facts_remain_open(self):
        report = assess_profile(PROFILE, root=ROOT)
        fields = {item["field"] for item in report["commercial_open_items"]}
        self.assertTrue({"market_validation", "pricing", "unit_economics"}.issubset(fields))
        self.assertLess(report["axes"]["commercial"]["maturity_percent"], 100.0)


if __name__ == "__main__":
    unittest.main()
