#!/usr/bin/env python3
"""Privacy and secret-scanner regression suite; Python standard library only."""
from __future__ import annotations

import importlib.util
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("repository_security_audit", ROOT / "scripts" / "security_audit.py")
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("cannot load scripts/security_audit.py")
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)

class PrivacyRegressionTests(unittest.TestCase):
    def test_contract_prohibits_diagnosis_and_surveillance(self) -> None:
        contract = json.loads((ROOT / "mental-wellbeing-code-safety.v1.json").read_text(encoding="utf-8"))
        policy = contract["policy"]
        for key in (
            "not_a_clinical_protocol",
            "no_diagnosis_or_screening",
            "no_medication_or_treatment_advice",
            "no_mental_health_inference",
            "no_health_or_productivity_surveillance",
            "no_background_collection_of_wellbeing_data",
            "no_unconsented_remote_telemetry",
            "no_raw_sensitive_content_logging",
            "data_minimization_required",
        ):
            self.assertIs(policy.get(key), True, key)

    def test_detects_high_confidence_access_token(self) -> None:
        token = "ghp_" + ("Ab3" * 14)
        findings = AUDIT.scan_text(f'ACCESS_TOKEN = "{token}"', "fixture.py")
        self.assertTrue(any(item["category"] == "secret" for item in findings))

    def test_detects_hardcoded_password_assignment(self) -> None:
        findings = AUDIT.scan_text('password = "correct-horse-battery-staple-77"', "fixture.py")
        self.assertTrue(any(item["rule"] == "generic-hardcoded-credential" for item in findings))

    def test_does_not_flag_obvious_placeholders(self) -> None:
        findings = AUDIT.scan_text('api_key = "EXAMPLE_PLACEHOLDER_TOKEN"', "fixture.py")
        self.assertFalse(any(item["category"] == "secret" for item in findings))

    def test_detects_sensitive_logging_without_copying_values(self) -> None:
        private_value = "DEVICE-SERIAL-DO-NOT-LOG-123456"
        findings = AUDIT.scan_text('logger.info("serial_number=%s", serial_number)', "fixture.py")
        self.assertTrue(any(item["rule"] == "sensitive-value-logging" for item in findings))
        self.assertNotIn(private_value, json.dumps(findings))

    def test_detects_machine_specific_home_path(self) -> None:
        findings = AUDIT.scan_text(r'cache = "C:\\Users\\developer\\Desktop\\private"', "fixture.py")
        self.assertTrue(any(item["rule"] == "developer-local-path" for item in findings))

    def test_findings_never_include_source_snippets(self) -> None:
        findings = AUDIT.scan_text('password = "unique-sensitive-value-123456"', "fixture.py")
        encoded = json.dumps(findings)
        self.assertNotIn("unique-sensitive-value-123456", encoded)
        self.assertTrue(all(set(item) == {"rule", "severity", "category", "path", "line"} for item in findings))

    def test_audit_scans_current_repository(self) -> None:
        report = AUDIT.audit(ROOT)
        self.assertEqual(report["schema"], "repository-security-audit.v1")
        self.assertGreater(report["scanned_files"], 0)

if __name__ == "__main__":
    unittest.main(verbosity=2)
