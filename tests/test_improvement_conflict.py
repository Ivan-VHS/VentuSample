"""Regression tests for the improvement/conflict governance profile."""
from __future__ import annotations
import copy
import json
import unittest
from pathlib import Path
from tools.improvement_conflict import ImprovementConflictProfileError, validate_profile

ROOT = Path(__file__).resolve().parents[1]
PROFILE = json.loads((ROOT / "improvement-conflict.v1.json").read_text(encoding="utf-8"))

class ImprovementConflictProfileTests(unittest.TestCase):
    def clone(self):
        return copy.deepcopy(PROFILE)

    def test_repository_profile_has_twenty_unassessed_controls(self):
        result = validate_profile(PROFILE, root=ROOT)
        self.assertEqual(result["controls"], 20)
        self.assertEqual(result["assessed"], 0)
        self.assertEqual(result["verified"], 0)
        self.assertEqual(result["states"]["not_assessed"], 20)

    def test_rejects_missing_control(self):
        profile = self.clone()
        profile["controls"].pop()
        with self.assertRaises(ImprovementConflictProfileError):
            validate_profile(profile)

    def test_rejects_changed_vocabulary(self):
        profile = self.clone()
        profile["controls"][0]["dissonance_term"] = "другое"
        with self.assertRaises(ImprovementConflictProfileError):
            validate_profile(profile)

    def test_rejects_duplicate_or_reordered_identifier(self):
        profile = self.clone()
        profile["controls"][1]["id"] = profile["controls"][0]["id"]
        with self.assertRaises(ImprovementConflictProfileError):
            validate_profile(profile)

    def test_requires_remediation_and_verification(self):
        for field in ("remediation", "verification"):
            profile = self.clone()
            profile["controls"][0][field] = " "
            with self.subTest(field=field), self.assertRaises(ImprovementConflictProfileError):
                validate_profile(profile)

    def test_rejects_unknown_state(self):
        profile = self.clone()
        profile["controls"][0]["state"] = "fixed"
        with self.assertRaises(ImprovementConflictProfileError):
            validate_profile(profile)

    def test_rejects_unsafe_evidence_path(self):
        profile = self.clone()
        profile["controls"][0]["evidence"] = ["../outside.txt"]
        with self.assertRaises(ImprovementConflictProfileError):
            validate_profile(profile)

    def test_verified_state_requires_evidence(self):
        profile = self.clone()
        profile["controls"][0]["state"] = "verified"
        with self.assertRaises(ImprovementConflictProfileError):
            validate_profile(profile)

    def test_verified_state_requires_existing_repository_evidence(self):
        profile = self.clone()
        profile["controls"][0]["state"] = "verified"
        profile["controls"][0]["evidence"] = ["missing-evidence-for-test.txt"]
        with self.assertRaises(ImprovementConflictProfileError):
            validate_profile(profile, root=ROOT)

    def test_deferred_state_requires_reason(self):
        profile = self.clone()
        profile["controls"][0]["state"] = "deferred"
        with self.assertRaises(ImprovementConflictProfileError):
            validate_profile(profile)

if __name__ == "__main__":
    unittest.main()
