"""Regression, privacy and deterministic fuzz tests for Beelzebul."""
from __future__ import annotations
import json, random, unittest
from pathlib import Path
from scanner import correlate, inspect_file, score_findings, validate_policy

class ScannerRegressionTests(unittest.TestCase):
    def test_secret_is_detected_but_never_echoed(self):
        fake="ghp_" + ("A"*40)
        findings=inspect_file("src/config.py",('ACCESS_TOKEN = "'+fake+'"\n').encode(),2097152)
        encoded=json.dumps(findings)
        self.assertTrue(any(x["rule_id"]=="A01" for x in findings))
        self.assertNotIn(fake,encoded)
        self.assertTrue(all("sample" not in x for x in findings))
    def test_invisible_controls_and_conflict_markers(self):
        source="value = 1\n"+"name = 'x'\u202e\n"+"<<<<<<< branch\n"
        ids={x["rule_id"] for x in inspect_file("src/module.py",source.encode("utf-8"),2097152)}
        self.assertIn("A05",ids);self.assertIn("A08",ids)
    def test_unsafe_process_sink_is_classified(self):
        findings=inspect_file("src/runner.py",b"subprocess.run(command, shell=True)\n",2097152)
        self.assertIn("M02",{x["rule_id"] for x in findings})
    def test_oversize_guard_is_bounded(self):
        self.assertEqual(inspect_file("src/blob.py",b"x"*128,64)[0]["rule_id"],"A10")
    def test_seeded_fuzz_inputs_do_not_crash_parser(self):
        rng=random.Random(20261121)
        alphabet=b"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789{}[]()'\\\";:=_+-*/\\\\ \t\n"
        for index in range(300):
            payload=bytes(rng.choice(alphabet) for _ in range(rng.randrange(0,2048)))
            inspect_file(f"fuzz/{index}.py",payload,4096)
    def test_risk_score_is_stable(self):
        findings=[{"severity":"high"},{"severity":"medium"},{"severity":"low"}]
        self.assertEqual(score_findings(findings,{}),score_findings(findings,{}))
        self.assertEqual(score_findings(findings,{})["risk_score"],18)
    def test_correlations_ignore_scanner_and_policy_metadata(self):
        findings = []
        sources = {
            "security/beelzebul/scanner.py": "password token sha1 logger.info(value)",
            ".beelzebul/policy.v1.json": '{"password": "token", "sha1": "auth"}',
        }
        correlate(findings, sources, set(sources))
        self.assertFalse([item for item in findings if item["rule_id"].startswith("Q")])

    def test_policy_contract_has_66_unique_rules(self):
        findings=[];policy=validate_policy(Path(__file__).resolve().parents[2],findings)
        self.assertEqual(policy.get("codex"),"BEELZEBUL-66")
        self.assertFalse([x for x in findings if x["rule_id"]=="G01"])
if __name__=="__main__":unittest.main()
