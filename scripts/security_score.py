#!/usr/bin/env python3
"""Compute the common 0-100 repository assurance score from machine reports."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

SCHEMA = "cross-repository-assurance-score.v1"

def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"{path} must contain a JSON object")
    return value

def compute(scan: dict[str, Any], deps: dict[str, Any], fuzz: str, privacy: str, federation: str) -> dict[str, Any]:
    counts = scan.get("counts", {})
    critical = int(counts.get("critical_secrets", 0))
    privacy_findings = int(counts.get("privacy", 0))
    source_risks = int(counts.get("source_risks", 0))
    pin_score = max(0, min(100, int(deps.get("pin_hygiene_score", 0))))
    categories = {
        "secret_hygiene": {
            "weight": 25,
            "score": 25 if critical == 0 else 0,
            "basis": "25 points only when no high-confidence credential or private-key finding is present",
        },
        "privacy_leak_prevention": {
            "weight": 15,
            "score": max(0, 15 - min(15, privacy_findings * 5)),
            "basis": "15 points minus 5 per sensitive-log or developer-local-path finding",
        },
        "source_risk_review": {
            "weight": 15,
            "score": max(0, 15 - min(15, source_risks * 2)),
            "basis": "15 points minus 2 per risky-API review finding; findings require human review",
        },
        "fuzz_robustness": {
            "weight": 15,
            "score": 15 if fuzz == "passed" else 0,
            "basis": "Deterministic project-specific malformed-input fuzz corpus completed",
        },
        "dependency_hygiene": {
            "weight": 10,
            "score": round(pin_score / 10),
            "basis": "Offline inventory and exact-version pin hygiene; not a vulnerability-database result",
        },
        "privacy_regressions": {
            "weight": 10,
            "score": 10 if privacy == "passed" else 0,
            "basis": "Synthetic secret detection, redaction, privacy controls and contract regression tests passed",
        },
        "federation_consistency": {
            "weight": 10,
            "score": 10 if federation == "passed" else 0,
            "basis": "All four repositories share the same policy and assurance tooling contract",
        },
    }
    total = sum(int(item["score"]) for item in categories.values())
    grade = "excellent" if total >= 90 else "good" if total >= 80 else "review" if total >= 70 else "needs-remediation"
    return {
        "schema": SCHEMA,
        "repository": scan.get("repository"),
        "score": total,
        "maximum": 100,
        "grade": grade,
        "gate": "pass" if total >= 70 and critical == 0 and fuzz == "passed" and privacy == "passed" and federation == "passed" else "fail",
        "categories": categories,
        "inputs": {
            "critical_secret_findings": critical,
            "privacy_findings": privacy_findings,
            "source_risk_findings": source_risks,
            "fuzz_status": fuzz,
            "privacy_regression_status": privacy,
            "dependency_pin_hygiene_score": pin_score,
            "federation_status": federation,
        },
        "limitations": "A policy score is an engineering trend indicator, not a safety certification, clinical assessment, proof of exploit absence or substitute for review.",
    }

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scan", type=Path, required=True)
    parser.add_argument("--dependencies", type=Path, required=True)
    parser.add_argument("--fuzz-status", choices=("passed", "failed"), required=True)
    parser.add_argument("--privacy-status", choices=("passed", "failed"), required=True)
    parser.add_argument("--federation-status", choices=("passed", "failed"), required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    result = compute(read_json(args.scan), read_json(args.dependencies), args.fuzz_status, args.privacy_status, args.federation_status)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0 if result["gate"] == "pass" else 1

if __name__ == "__main__":
    raise SystemExit(main())
