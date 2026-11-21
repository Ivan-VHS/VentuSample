#!/usr/bin/env python3
"""Audit source, dependency hygiene and policy parity across all four repositories."""
from __future__ import annotations

import argparse
import hashlib
import importlib.util
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

def load_module(path: Path, name: str) -> Any:
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load assurance module: {path.name}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module

def audit(roots: dict[str, Path]) -> dict[str, Any]:
    problems: list[str] = []
    if set(roots) != set(EXPECTED):
        problems.append("exactly the four registered repositories must be supplied")
    contracts: dict[str, dict[str, Any]] = {}
    source_reports: dict[str, dict[str, Any]] = {}
    dependency_reports: dict[str, dict[str, Any]] = {}
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
        if not (root / ".github" / "workflows" / "codeql.yml").is_file():
            problems.append(f"{name}: CodeQL workflow is missing")
        try:
            scanner = load_module(root / "scripts" / "security_audit.py", f"security_audit_{project_id}")
            dependency = load_module(root / "scripts" / "dependency_audit.py", f"dependency_audit_{project_id}")
            source_reports[name] = scanner.audit(root)
            dependency_reports[name] = dependency.audit(root)
        except Exception as error:  # A broken assurance tool is itself a failed audit.
            problems.append(f"{name}: source/dependency audit failed ({type(error).__name__})")

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

    combined = {
        "critical_secrets": sum(int(report.get("counts", {}).get("critical_secrets", 0)) for report in source_reports.values()),
        "privacy": sum(int(report.get("counts", {}).get("privacy", 0)) for report in source_reports.values()),
        "source_risks": sum(int(report.get("counts", {}).get("source_risks", 0)) for report in source_reports.values()),
    }
    if combined["critical_secrets"]:
        problems.append("one or more repositories contain high-confidence secret findings")
    parity_score = max(0, 100 - min(100, 15 * len(problems)))
    source_penalty = min(100, combined["critical_secrets"] * 30 + combined["privacy"] * 3 + combined["source_risks"])
    source_score = max(0, 100 - source_penalty)
    dep_scores = [int(report.get("pin_hygiene_score", 0)) for report in dependency_reports.values()]
    dependency_score = round(sum(dep_scores) / len(dep_scores)) if dep_scores else 0
    overall_score = round(parity_score * 0.50 + source_score * 0.35 + dependency_score * 0.15)
    return {
        "schema": "cross-repository-security-assessment.v2",
        "repositories_checked": sorted(contracts),
        "expected_repositories": sorted(EXPECTED),
        "shared_files_checked": list(SHARED_FILES),
        "status": "pass" if not problems else "fail",
        "problems": problems,
        "score": overall_score,
        "score_components": {
            "policy_and_tooling_parity": {"weight": 50, "score": parity_score},
            "source_and_privacy_scan": {"weight": 35, "score": source_score},
            "dependency_pin_hygiene": {"weight": 15, "score": dependency_score},
        },
        "aggregate_findings": combined,
        "source_audits": source_reports,
        "dependency_audits": dependency_reports,
        "limitations": "Static rule matching and offline dependency metadata inventory; no secret values are emitted. Does not prove absence of vulnerabilities or replace CodeQL, fuzzing, tests or expert review.",
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
