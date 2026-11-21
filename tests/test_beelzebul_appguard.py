"""Regression tests for the Beelzebul AppGuard catalog validator."""
from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from validate_beelzebul_appguard import validate_manifest  # noqa: E402


class AppGuardCatalogTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with (ROOT / ".beelzebul" / "appguard.v1.json").open("r", encoding="utf-8") as stream:
            cls.catalog = json.load(stream)

    def test_canonical_catalog_passes(self):
        self.assertEqual(validate_manifest(copy.deepcopy(self.catalog)), [])

    def test_missing_level_fails(self):
        data = copy.deepcopy(self.catalog)
        data["levels"].pop(120)
        self.assertTrue(any("exactly 666" in error for error in validate_manifest(data)))

    def test_duplicate_level_fails(self):
        data = copy.deepcopy(self.catalog)
        data["levels"][1]["level_id"] = data["levels"][0]["level_id"]
        self.assertTrue(any("duplicate level id" in error for error in validate_manifest(data)))

    def test_echelon_boundary_fails(self):
        data = copy.deepcopy(self.catalog)
        data["levels"][111]["echelon_id"] = "I"
        self.assertTrue(any("L112: incorrect echelon_id" in error for error in validate_manifest(data)))

    def test_unverified_implementation_claim_fails(self):
        data = copy.deepcopy(self.catalog)
        data["implementation_claims"] = 1
        data["levels"][0]["implementation_state"] = "verified"
        data["levels"][0]["enforcement"] = "enforced"
        self.assertTrue(any("verified state requires" in error for error in validate_manifest(data)))

    def test_rule_crosswalk_gap_fails(self):
        data = copy.deepcopy(self.catalog)
        data["repository_rule_groups"][0]["rule_ids"].pop()
        self.assertTrue(any("A must contain exactly 15 rule ids" in error for error in validate_manifest(data)))


if __name__ == "__main__":
    unittest.main()
