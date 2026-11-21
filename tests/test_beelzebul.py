from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from tools.beelzebul import PolicyError, load_policy, scan_repository

ROOT = Path(__file__).resolve().parents[1]
POLICY = ROOT / "security" / "beelzebul.v1.json"


class BeelzebulTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)

    def tearDown(self):
        self.temp.cleanup()

    def write(self, name: str, data: str | bytes) -> None:
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        if isinstance(data, bytes):
            path.write_bytes(data)
        else:
            path.write_text(data, encoding="utf-8", newline="")

    def report(self):
        return scan_repository(self.root, POLICY)

    def test_policy_contains_exactly_66_rules_in_five_groups(self):
        policy = load_policy(POLICY)
        self.assertEqual(policy["rule_count"], 66)
        self.assertEqual(len(policy["rules"]), 66)
        self.assertEqual([level["rules"] for level in policy["levels"]], [15, 15, 15, 15, 6])

    def test_credential_prefix_is_blocked_and_redacted(self):
        secret = "AK" + "IA" + ("A" * 16)
        self.write("src/config.py", f'ACCESS = "{secret}"\n')
        report = self.report()
        self.assertEqual(report["verdict"], "blocked")
        finding = next(item for item in report["findings"] if item["rule_id"] == "A-02")
        self.assertNotIn(secret, json.dumps(finding))

    def test_hidden_unicode_is_blocked(self):
        self.write("src/example.py", "name = 'safe'\u202E # hidden override\n")
        report = self.report()
        self.assertIn("A-06", {item["rule_id"] for item in report["findings"]})
        self.assertEqual(report["verdict"], "blocked")

    def test_mixed_line_endings_are_detected(self):
        self.write("README.md", b"one\r\ntwo\n")
        self.assertIn("A-09", {item["rule_id"] for item in self.report()["findings"]})

    def test_invalid_utf8_is_detected(self):
        self.write("src/bad.py", b"name = '\xff'\n")
        self.assertIn("A-04", {item["rule_id"] for item in self.report()["findings"]})

    def test_merge_markers_are_blocked(self):
        marker_a = "<" * 7 + " HEAD"
        marker_b = "=" * 7
        marker_c = ">" * 7 + " branch"
        self.write("docs/conflict.md", "before\n" + marker_a + "\nleft\n" + marker_b + "\nright\n" + marker_c + "\n")
        self.assertIn("A-10", {item["rule_id"] for item in self.report()["findings"]})

    def test_path_case_collision_is_blocked(self):
        self.write("Config/Readme.txt", "a\n")
        self.write("config/readme.txt", "b\n")
        self.assertIn("A-13", {item["rule_id"] for item in self.report()["findings"]})

    def test_dynamic_execution_is_review_not_automatic_block(self):
        self.write("src/module.py", "def run(value):\n    return eval(value)\n")
        report = self.report()
        finding = next(item for item in report["findings"] if item["rule_id"] == "M-02")
        self.assertEqual(finding["enforcement"], "review")
        self.assertEqual(report["verdict"], "review")

    def test_shell_true_is_reported(self):
        self.write("src/module.py", "import subprocess\nsubprocess.run('echo hi', shell=True)\n")
        self.assertIn("M-10", {item["rule_id"] for item in self.report()["findings"]})

    def test_critical_baseline_cannot_be_suppressed(self):
        policy = load_policy(POLICY)
        policy["exceptions"] = [{
            "rule_id": "A-02", "path": "src/config.py",
            "reason": "test exception that must be rejected", "owner": "security",
            "expires_at": "2099-01-01T00:00:00Z"
        }]
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "policy.json"
            path.write_text(json.dumps(policy), encoding="utf-8")
            with self.assertRaises(PolicyError):
                load_policy(path)

    def test_scanner_does_not_modify_repository_files(self):
        self.write("src/plain.py", "def add(a: int, b: int) -> int:\n    return a + b\n")
        before = (self.root / "src/plain.py").read_bytes()
        self.report()
        self.assertEqual((self.root / "src/plain.py").read_bytes(), before)

    def test_risk_score_is_bounded(self):
        value = "AK" + "IA" + ("A" * 16)
        self.write("src/credential.py", f'TOKEN = "{value}"\n')
        report = self.report()
        self.assertGreaterEqual(report["risk_score"], 0)
        self.assertLessEqual(report["risk_score"], 100)


if __name__ == "__main__":
    unittest.main()
