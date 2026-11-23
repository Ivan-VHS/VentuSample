#!/usr/bin/env python3
"""Validate an evidence-based backend engineering profile using only the standard library."""

from __future__ import annotations

import argparse
import copy
import json
import sys
from collections import Counter
from pathlib import Path, PurePosixPath
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PROFILE_PATH = ROOT / ".architecture" / "backend-engineering.v1.json"
SCHEMA_PATH = ROOT / ".architecture" / "backend-engineering.v1.schema.json"
CONTROL_IDS = (
    "boundaries-contracts",
    "input-validation-idempotency",
    "io-transport",
    "concurrency-backpressure",
    "memory-resource-lifecycle",
    "data-integrity-transactions",
    "failure-recovery-lifecycle",
    "security-privilege-secrets",
    "observability-diagnostics",
    "performance-capacity",
    "configuration-extensibility",
    "verification-delivery",
)
STATUSES = {"implemented", "partial", "planned", "not_applicable", "not_assessed"}


def _non_empty_text(value: Any) -> bool:
    return isinstance(value, str) and bool(value.strip())


def _check_repo_paths(
    values: Any,
    field_name: str,
    control_index: int,
    root: Path,
    errors: list[str],
) -> list[str]:
    prefix = f"controls[{control_index}].{field_name}"
    if not isinstance(values, list) or any(not _non_empty_text(item) for item in values):
        errors.append(f"{prefix} must be an array of non-empty repository-relative paths")
        return []

    valid: list[str] = []
    seen: set[str] = set()
    root_resolved = root.resolve()

    for raw_path in values:
        if raw_path in seen:
            errors.append(f"{prefix} contains a duplicate path: {raw_path}")
            continue
        seen.add(raw_path)

        relative = PurePosixPath(raw_path)
        if relative.is_absolute() or ".." in relative.parts or not relative.parts:
            errors.append(f"{prefix} contains an unsafe path: {raw_path}")
            continue

        target = root.joinpath(*relative.parts).resolve()
        try:
            target.relative_to(root_resolved)
        except ValueError:
            errors.append(f"{prefix} escapes the repository: {raw_path}")
            continue

        if not target.exists():
            errors.append(f"{prefix} path does not exist: {raw_path}")
            continue
        valid.append(raw_path)

    return valid


def validate_profile(profile: Any, root: Path = ROOT) -> list[str]:
    errors: list[str] = []
    if not isinstance(profile, dict):
        return ["profile must be a JSON object"]

    if profile.get("schema") != "ivan-vhs.backend-engineering.v1":
        errors.append("unsupported or missing profile schema")
    if profile.get("version") != 1:
        errors.append("profile version must be 1")
    for field in ("project", "scope"):
        if not _non_empty_text(profile.get(field)):
            errors.append(f"{field} must be a non-empty string")

    if profile.get("standard") != "docs/BACKEND_ENGINEERING_STANDARD.md":
        errors.append("standard must point to docs/BACKEND_ENGINEERING_STANDARD.md")
    if profile.get("schema_file") != ".architecture/backend-engineering.v1.schema.json":
        errors.append("schema_file must point to the committed JSON Schema")

    policy = profile.get("status_policy")
    if not isinstance(policy, dict):
        errors.append("status_policy must be an object")
    else:
        allowed = policy.get("allowed_statuses")
        if not isinstance(allowed, list) or set(allowed) != STATUSES:
            errors.append("status_policy must enumerate every supported maturity status")
        if not _non_empty_text(policy.get("rule")):
            errors.append("status_policy.rule must explain evidence-based status assignment")

    controls = profile.get("controls")
    if not isinstance(controls, list) or len(controls) != len(CONTROL_IDS):
        errors.append("controls must contain exactly twelve engineering domains")
        return errors

    found_ids: list[str] = []
    for index, control in enumerate(controls):
        prefix = f"controls[{index}]"
        if not isinstance(control, dict):
            errors.append(f"{prefix} must be an object")
            continue

        control_id = control.get("id")
        found_ids.append(control_id if isinstance(control_id, str) else "")
        if not _non_empty_text(control_id):
            errors.append(f"{prefix}.id must be a non-empty string")
        if not _non_empty_text(control.get("title")):
            errors.append(f"{prefix}.title must be a non-empty string")

        status = control.get("status")
        if status not in STATUSES:
            errors.append(f"{prefix}.status is not supported")
        if not _non_empty_text(control.get("assessment")):
            errors.append(f"{prefix}.assessment must describe the repository-specific state")

        limitations = control.get("limitations")
        if not isinstance(limitations, list) or not limitations or any(not _non_empty_text(item) for item in limitations):
            errors.append(f"{prefix}.limitations must contain at least one explicit boundary")

        evidence = _check_repo_paths(control.get("evidence"), "evidence", index, root, errors)
        verification = _check_repo_paths(control.get("verification"), "verification", index, root, errors)

        if status in {"implemented", "partial"}:
            if not evidence:
                errors.append(f"{prefix} claims implementation without valid evidence")
            if not verification:
                errors.append(f"{prefix} claims implementation without a verification path")

    if tuple(found_ids) != CONTROL_IDS:
        errors.append("controls must list all twelve canonical domains exactly once and in order")

    standard_path = (root / "docs" / "BACKEND_ENGINEERING_STANDARD.md").resolve()
    schema_path = (root / ".architecture" / "backend-engineering.v1.schema.json").resolve()
    for label, path in (("standard", standard_path), ("schema", schema_path)):
        try:
            path.relative_to(root.resolve())
        except ValueError:
            errors.append(f"{label} path escapes the repository")
            continue
        if not path.is_file():
            errors.append(f"{label} file is missing")

    return errors


def validate_schema_document(schema: Any) -> list[str]:
    if not isinstance(schema, dict):
        return ["JSON Schema must be an object"]
    errors: list[str] = []
    if schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
        errors.append("JSON Schema draft must be 2020-12")
    if schema.get("title") != "Backend Engineering Profile v1":
        errors.append("JSON Schema title is missing or unsupported")
    properties = schema.get("properties")
    if not isinstance(properties, dict) or properties.get("controls", {}).get("type") != "array":
        errors.append("JSON Schema must define the controls array")
    return errors


def run_self_tests(profile: Any, root: Path) -> None:
    if validate_profile(profile, root):
        raise AssertionError("the committed profile must pass before negative tests")

    broken = copy.deepcopy(profile)
    broken["controls"].pop()
    if not validate_profile(broken, root):
        raise AssertionError("missing control was not rejected")

    broken = copy.deepcopy(profile)
    broken["controls"][0]["status"] = "implemented_without_evidence"
    if not validate_profile(broken, root):
        raise AssertionError("unknown maturity status was not rejected")

    broken = copy.deepcopy(profile)
    broken["controls"][0]["evidence"] = []
    if not validate_profile(broken, root):
        raise AssertionError("an implementation claim without evidence was not rejected")

    broken = copy.deepcopy(profile)
    broken["controls"][0]["evidence"] = ["../outside-repository"]
    if not validate_profile(broken, root):
        raise AssertionError("repository path traversal was not rejected")

    broken = copy.deepcopy(profile)
    broken["controls"][0]["verification"] = ["tests/does-not-exist.test"]
    if not validate_profile(broken, root):
        raise AssertionError("a missing verification path was not rejected")

    broken = copy.deepcopy(profile)
    broken["controls"][1]["id"] = broken["controls"][0]["id"]
    if not validate_profile(broken, root):
        raise AssertionError("duplicate or missing domains were not rejected")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--self-test", action="store_true", help="run validator negative-case checks")
    args = parser.parse_args()

    try:
        with PROFILE_PATH.open("r", encoding="utf-8") as stream:
            profile = json.load(stream)
        with SCHEMA_PATH.open("r", encoding="utf-8") as stream:
            schema = json.load(stream)
    except (OSError, json.JSONDecodeError) as error:
        print(f"Backend engineering profile: cannot load profile or schema: {error}", file=sys.stderr)
        return 2

    errors = validate_schema_document(schema) + validate_profile(profile)
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    counts = Counter(control["status"] for control in profile["controls"])
    summary = ", ".join(f"{status}={counts[status]}" for status in sorted(counts))
    print(f"Backend engineering profile: {profile['project']} — 12 domains ({summary})")

    if args.self_test:
        try:
            run_self_tests(profile, ROOT)
        except AssertionError as error:
            print(f"SELF-TEST FAILED: {error}", file=sys.stderr)
            return 1
        print("Backend engineering profile validator self-tests: passed")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
