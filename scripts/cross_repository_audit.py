#!/usr/bin/env python3
"""Validate the shared security baseline across the four project checkouts."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

EXPECTED = {"Pytyliki": "pytyliki", "Reaper": "reaper", "OPlague": "oplague", "VentuSample": "ventusample"}
SHARED_FILES = (
    "scripts/security_audit.py",
    "scripts/dependency_audit.py",
    "scripts/security_score.py",
    "scripts/test_privacy_regressions.py",
    "scripts/cross_repository_audit.py",
    ".github/workflows/security-assurance.yml",
)
def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()

def audit(roots: dict[str, Path]) -> dict[str, Any]:
    problems: list[str] = []
    if set(roots) != set(EXPECTED):
        problems.append("exactly the four registered repositories must be supplied")
    contracts: dict[str, dict[str, Any]] = {}
    for name, project_id in EXPECTED.items():
        root = roots.get(name)
        if root is None or not root.is_dir():
            problems.append(f"{name}: repository checkout is missing")
            continue
        contract_path = root / "mental-wellbeing-code-safety.v1.json"
        if not contract_path.is_file():
            problems.append(f"{name}: machine-readable policy contract is missing")
            continue
        try:
            contract = json.loads(contract_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            problems.append(f"{name}: policy contract is unreadable or invalid JSON")
            continue
        contracts[name] = contract
        if contract.get("schema") != "mental-wellbeing-code-safety.v1":
            problems.append(f"{name}: unsupported policy schema")
        if contract.get("project", {}).get("id") != project_id:
            problems.append(f"{name}: project identity mismatch")
        for rel in SHARED_FILES:
            if not (root / rel).is_file():
                problems.append(f"{name}: missing shared assurance file {rel}")
        for rel in (
            "docs/MENTAL_WELLBEING_CODE_SAFETY_V1.md",
            "scripts/check_mental_wellbeing_guardrails.py",
            "scripts/fuzz_smoke.py",
            ".github/workflows/mental-wellbeing-guardrails.yml",
        ):
            if not (root / rel).is_file():
                problems.append(f"{name}: missing required local guardrail {rel}")
    if len(contracts) == 4:
        first = contracts["Pytyliki"]
        for name, contract in contracts.items():
            if contract.get("policy") != first.get("policy"):
                problems.append(f"{name}: shared policy flags differ from Pytyliki baseline")
            if contract.get("source") != first.get("source"):
                problems.append(f"{name}: shared transcript provenance differs")
        for rel in SHARED_FILES:
            hashes = {name: digest(roots[name] / rel) for name in EXPECTED if (roots[name] / rel).is_file()}
            if len(set(hashes.values())) != 1:
                problems.append(f"shared assurance file differs between repositories: {rel}")
    return {
        "schema": "cross-repository-policy-audit.v1",
        "repositories_checked": sorted(contracts),
        "expected_repositories": sorted(EXPECTED),
        "shared_files_checked": list(SHARED_FILES),
        "status": "pass" if not problems else "fail",
        "problems": problems,
        "score": 100 if not problems else max(0, 100 - 15 * len(problems)),
        "limitations": "Checks policy/tooling parity and file presence; it does not prove the application code is free of vulnerabilities.",
    }

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", action="append", required=True, metavar="NAME=PATH")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    roots: dict[str, Path] = {}
    for item in args.repo:
        if "=" not in item:
            parser.error("--repo must use NAME=PATH")
        name, path = item.split("=", 1)
        roots[name] = Path(path).resolve()
    report = audit(roots)
    rendered = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
    return 0 if report["status"] == "pass" else 1

if __name__ == "__main__":
    raise SystemExit(main())
