#!/usr/bin/env python3
"""Validate roadmap governance and bounded CI-scale contracts (stdlib only)."""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
GOVERNANCE = ROOT / ".architecture" / "roadmap-governance.v1.json"
GOVERNANCE_SCHEMA = ROOT / ".architecture" / "roadmap-governance.v1.schema.json"
SCALE = ROOT / ".architecture" / "ci-scale-policy.v1.json"
SCALE_SCHEMA = ROOT / ".architecture" / "ci-scale-policy.v1.schema.json"


def load_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def validate_governance(policy: Any, repository: str) -> list[str]:
    errors: list[str] = []
    if not isinstance(policy, dict):
        return ["governance policy must be an object"]
    if policy.get("schema") != "anatidae.roadmap-governance.v1" or policy.get("version") != 1:
        errors.append("unsupported roadmap governance schema/version")
    if policy.get("repository") != repository:
        errors.append("governance repository does not match project contract")
    if not isinstance(policy.get("owner"), str) or not policy["owner"].strip():
        errors.append("governance owner is required")
    for section in ("change_control", "status_rules", "release_controls"):
        value = policy.get(section)
        if not isinstance(value, dict) or not value:
            errors.append(f"{section} must be a non-empty object")
        elif any(item is not True for item in value.values()):
            errors.append(f"all {section} controls must remain enabled")
    priorities = policy.get("priorities")
    if not isinstance(priorities, dict) or set(priorities) != {"P0", "P1", "P2"}:
        errors.append("priorities must define P0, P1 and P2")
    else:
        for level, detail in priorities.items():
            if not isinstance(detail, dict) or not all(
                isinstance(detail.get(key), str) and detail[key].strip()
                for key in ("meaning", "review")
            ):
                errors.append(f"{level} must define a meaning and review rule")
    return errors


def _extract_workflow_jobs(text: str) -> list[tuple[str, str]]:
    lines = text.splitlines()
    try:
        jobs_index = next(i for i, line in enumerate(lines) if line.strip() == "jobs:")
    except StopIteration:
        return []
    jobs: list[tuple[str, str]] = []
    current_name: str | None = None
    current_lines: list[str] = []
    for line in lines[jobs_index + 1:]:
        if line and not line.startswith(" "):
            break
        match = re.match(r"^  ([A-Za-z0-9_-]+):\s*$", line)
        if match:
            if current_name is not None:
                jobs.append((current_name, "\n".join(current_lines)))
            current_name = match.group(1)
            current_lines = [line]
        elif current_name is not None:
            current_lines.append(line)
    if current_name is not None:
        jobs.append((current_name, "\n".join(current_lines)))
    return jobs


def _workflow_concurrency_errors(text: str, label: str) -> list[str]:
    lines = text.splitlines()
    jobs_index = next((i for i, line in enumerate(lines) if line.strip() == "jobs:"), len(lines))
    before_jobs = lines[:jobs_index]
    index = next((i for i, line in enumerate(before_jobs) if line.strip() == "concurrency:"), None)
    if index is None:
        return [f"{label}: workflow-level concurrency is required"]
    block: list[str] = []
    for line in before_jobs[index + 1:]:
        if line and not line.startswith(" "):
            break
        block.append(line)
    content = "\n".join(block)
    errors: list[str] = []
    if not re.search(r"(?m)^\s+group:\s*\S", content):
        errors.append(f"{label}: concurrency group is missing")
    if not re.search(r"(?m)^\s+cancel-in-progress:\s*(true|false)\s*$", content):
        errors.append(f"{label}: cancel-in-progress must be explicit")
    return errors


def validate_workflow_scale(
    text: str,
    label: str,
    limits: dict[str, int],
) -> list[str]:
    errors = _workflow_concurrency_errors(text, label)
    jobs = _extract_workflow_jobs(text)
    if not jobs:
        return errors + [f"{label}: jobs section is missing or empty"]
    if len(jobs) > limits["max_jobs"]:
        errors.append(f"{label}: {len(jobs)} jobs exceeds limit {limits['max_jobs']}")

    for name, block in jobs:
        if not re.search(r"(?m)^\s+runs-on:\s*\S", block):
            continue
        timeout_match = re.search(r"(?m)^\s+timeout-minutes:\s*(\d+)\s*$", block)
        if timeout_match is None:
            errors.append(f"{label}:{name}: runner job has no timeout-minutes")
        elif int(timeout_match.group(1)) > limits["max_job_timeout_minutes"]:
            errors.append(
                f"{label}:{name}: timeout {timeout_match.group(1)} exceeds "
                f"{limits['max_job_timeout_minutes']} minutes"
            )

        if re.search(r"(?m)^\s+matrix:\s*$", block):
            parallel_match = re.search(r"(?m)^\s+max-parallel:\s*(\d+)\s*$", block)
            if parallel_match is None:
                errors.append(f"{label}:{name}: matrix has no explicit max-parallel")
            elif int(parallel_match.group(1)) > limits["max_parallel_matrix"]:
                errors.append(
                    f"{label}:{name}: matrix parallelism {parallel_match.group(1)} exceeds "
                    f"{limits['max_parallel_matrix']}"
                )
    return errors


def validate_scale(policy: Any, repository: str, root: Path = ROOT) -> list[str]:
    errors: list[str] = []
    if not isinstance(policy, dict):
        return ["CI scale policy must be an object"]
    if policy.get("schema") != "anatidae.ci-scale-policy.v1" or policy.get("version") != 1:
        errors.append("unsupported CI scale policy schema/version")
    if policy.get("repository") != repository:
        errors.append("CI scale policy repository does not match project contract")

    global_limits = policy.get("global_limits")
    if not isinstance(global_limits, dict):
        return errors + ["global_limits must be an object"]
    keys = ("max_jobs_per_workflow", "max_job_timeout_minutes", "max_parallel_matrix")
    global_limits_valid = True
    for key in keys:
        value = global_limits.get(key)
        if isinstance(value, bool) or not isinstance(value, int) or value <= 0:
            errors.append(f"global_limits.{key} must be a positive integer")
            global_limits_valid = False
    if not global_limits_valid:
        return errors
    workflows = policy.get("workflows")
    if not isinstance(workflows, dict) or not workflows:
        return errors + ["workflows must be a non-empty path-to-budget object"]

    for relative, limits in workflows.items():
        if not isinstance(relative, str) or relative.startswith("/") or ".." in Path(relative).parts:
            errors.append(f"unsafe workflow path: {relative!r}")
            continue
        if not isinstance(limits, dict):
            errors.append(f"{relative}: limits must be an object")
            continue
        for key in ("max_jobs", "max_job_timeout_minutes", "max_parallel_matrix"):
            value = limits.get(key)
            if isinstance(value, bool) or not isinstance(value, int) or value <= 0:
                errors.append(f"{relative}: {key} must be a positive integer")
        if any(
            isinstance(limits.get(key), bool) or not isinstance(limits.get(key), int)
            for key in ("max_jobs", "max_job_timeout_minutes", "max_parallel_matrix")
        ):
            continue
        if limits["max_jobs"] > global_limits["max_jobs_per_workflow"]:
            errors.append(f"{relative}: workflow budget exceeds global job limit")
        if limits["max_job_timeout_minutes"] > global_limits["max_job_timeout_minutes"]:
            errors.append(f"{relative}: workflow timeout budget exceeds global limit")
        if limits["max_parallel_matrix"] > global_limits["max_parallel_matrix"]:
            errors.append(f"{relative}: workflow matrix budget exceeds global limit")
        path = root / relative
        if not path.is_file():
            errors.append(f"workflow evidence is missing: {relative}")
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except OSError as exc:
            errors.append(f"cannot read {relative}: {exc}")
            continue
        errors.extend(validate_workflow_scale(text, relative, limits))
    rules = policy.get("rules")
    if not isinstance(rules, dict) or not rules or any(value is not True for value in rules.values()):
        errors.append("all CI scale safety rules must remain enabled")
    return errors


def validate_documentation(root: Path = ROOT) -> list[str]:
    path = root / "docs" / "ROADMAP_GOVERNANCE_CI_SCALE.md"
    if not path.is_file():
        return ["roadmap governance and CI scale documentation is missing"]
    content = path.read_text(encoding="utf-8")
    required = (
        "Capability maturity",
        "Priority and change control",
        "CI execution budget",
        "Safe recovery and release boundaries",
        "Toolchain optimization",
        "`active`",
        "`extend`",
        "`planned`",
        "timeout-minutes",
        "max-parallel",
    )
    return [f"roadmap governance documentation is missing required section or term: {term}"
            for term in required if term not in content]


def validate_contracts(repository: str, root: Path = ROOT) -> list[str]:
    errors: list[str] = []
    pairs = (
        (GOVERNANCE, GOVERNANCE_SCHEMA, validate_governance),
        (SCALE, SCALE_SCHEMA, None),
    )
    for policy_path, schema_path, validator in pairs:
        try:
            policy = load_json(root / policy_path.relative_to(ROOT))
            schema = load_json(root / schema_path.relative_to(ROOT))
        except (OSError, json.JSONDecodeError) as exc:
            errors.append(f"cannot load {policy_path.name} or schema: {exc}")
            continue
        if schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
            errors.append(f"{schema_path.name} must use JSON Schema Draft 2020-12")
        required = set(schema.get("required", []))
        if not required.issubset(policy):
            errors.append(f"{policy_path.name} is missing schema-required fields")
        if validator is not None:
            errors.extend(validator(policy, repository))
        else:
            errors.extend(validate_scale(policy, repository, root))
    errors.extend(validate_documentation(root))
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--self-test", action="store_true", help="run validator negative-case checks")
    args = parser.parse_args()
    try:
        project_contract = load_json(ROOT / "project-contract.json")
    except (OSError, json.JSONDecodeError) as exc:
        print(f"delivery contracts: cannot load project contract: {exc}", file=sys.stderr)
        return 2
    project = project_contract.get("project")
    repository = project.get("repository") if isinstance(project, dict) else None
    if not isinstance(repository, str) or not repository.strip():
        print("delivery contracts: project-contract.json has no project.repository", file=sys.stderr)
        return 2
    errors = validate_contracts(repository)
    if args.self_test:
        governance = load_json(GOVERNANCE)
        scale = load_json(SCALE)
        broken_governance = dict(governance)
        broken_governance["status_rules"] = {"active_requires_existing_evidence": False}
        if not validate_governance(broken_governance, repository):
            errors.append("self-test failed: disabled governance rule was accepted")
        broken_scale = dict(scale)
        broken_scale["repository"] = "other/project"
        if not validate_scale(broken_scale, repository):
            errors.append("self-test failed: wrong repository scale policy was accepted")
        sample = "jobs:\n  build:\n    runs-on: ubuntu-latest\n    steps: []\n"
        if not validate_workflow_scale(sample, "self-test", {
            "max_jobs": 1, "max_job_timeout_minutes": 10, "max_parallel_matrix": 1
        }):
            errors.append("self-test failed: workflow without concurrency/timeout was accepted")
    if errors:
        for error in errors:
            print(f"delivery contracts: FAIL: {error}", file=sys.stderr)
        return 1
    print("delivery contracts: PASS (roadmap governance, workflow timeouts and bounded CI scale)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
