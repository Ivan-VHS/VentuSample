#!/usr/bin/env python3
"""Validate the repository's evidence-based system capability profile."""

from __future__ import annotations

import argparse
import copy
import json
import sys
from pathlib import Path, PurePosixPath
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PROFILE_PATH = ROOT / ".architecture" / "system-capabilities.v1.json"
STATUSES = {"implemented", "partial", "planned", "not_applicable", "not_assessed"}
EXPECTED_GROUPS = [
    ("security-crypto-policy", 1, 14, 14),
    ("network-protocols", 15, 28, 14),
    ("installation-servicing", 29, 40, 12),
    ("user-interface-storage", 41, 54, 14),
    ("lifecycle-compatibility", 55, 66, 12),
]


def _is_text(value: Any) -> bool:
    return isinstance(value, str) and bool(value.strip())


def validate_profile(profile: Any, root: Path = ROOT) -> list[str]:
    errors: list[str] = []
    if not isinstance(profile, dict):
        return ["profile must be a JSON object"]

    if profile.get("schema") != "ivan-vhs.system-capabilities.v1":
        errors.append("unsupported or missing schema identifier")
    if profile.get("version") != 1:
        errors.append("profile version must be 1")
    for key in ("project", "scope"):
        if not _is_text(profile.get(key)):
            errors.append(f"{key} must be a non-empty string")

    reference = profile.get("reference")
    if not isinstance(reference, dict):
        errors.append("reference must be an object")
    else:
        if reference.get("total_points") != 66:
            errors.append("reference must account for exactly 66 source points")
        if reference.get("quality") != "mixed_historical_claims":
            errors.append("reference quality must remain explicitly classified")
        if not _is_text(reference.get("warning")):
            errors.append("reference warning is required")

    groups = profile.get("source_groups")
    group_ids: set[str] = set()
    if not isinstance(groups, list) or len(groups) != len(EXPECTED_GROUPS):
        errors.append("source_groups must contain exactly five ordered groups")
        groups = []
    else:
        covered: list[int] = []
        for index, (expected_id, first, last, count) in enumerate(EXPECTED_GROUPS):
            group = groups[index]
            if not isinstance(group, dict):
                errors.append(f"source_groups[{index}] must be an object")
                continue
            group_id = group.get("id")
            if group_id != expected_id:
                errors.append(f"source_groups[{index}] must be {expected_id}")
            if group_id in group_ids:
                errors.append(f"duplicate source group: {group_id}")
            group_ids.add(group_id)
            if (group.get("first"), group.get("last"), group.get("count")) != (first, last, count):
                errors.append(f"{expected_id} must cover points {first}-{last} ({count} points)")
            covered.extend(range(first, last + 1))
        if covered != list(range(1, 67)):
            errors.append("source groups must cover points 1 through 66 exactly once")

    controls = profile.get("controls")
    if not isinstance(controls, list) or len(controls) != 5:
        errors.append("controls must contain exactly five project assessments")
        controls = []

    control_ids: set[str] = set()
    control_groups: set[str] = set()
    expected_group_by_id = {item[0]: (item[1], item[2]) for item in EXPECTED_GROUPS}

    for index, control in enumerate(controls):
        prefix = f"controls[{index}]"
        if not isinstance(control, dict):
            errors.append(f"{prefix} must be an object")
            continue

        control_id = control.get("id")
        source_group = control.get("source_group")
        status = control.get("status")

        if not _is_text(control_id):
            errors.append(f"{prefix}.id must be a non-empty string")
        elif control_id in control_ids:
            errors.append(f"duplicate control id: {control_id}")
        else:
            control_ids.add(control_id)

        if source_group not in expected_group_by_id:
            errors.append(f"{prefix}.source_group is not a recognized source group")
        else:
            if source_group in control_groups:
                errors.append(f"source group assessed more than once: {source_group}")
            control_groups.add(source_group)
            expected_range = list(expected_group_by_id[source_group])
            if control.get("source_points") != expected_range:
                errors.append(f"{prefix}.source_points must be {expected_range}")

        if status not in STATUSES:
            errors.append(f"{prefix}.status is not supported")
        if not _is_text(control.get("assessment")):
            errors.append(f"{prefix}.assessment must explain the project-specific decision")

        limitations = control.get("limitations")
        if not isinstance(limitations, list) or any(not _is_text(item) for item in limitations):
            errors.append(f"{prefix}.limitations must be an array of non-empty strings")

        def check_paths(field: str) -> list[str]:
            values = control.get(field)
            if not isinstance(values, list) or any(not _is_text(item) for item in values):
                errors.append(f"{prefix}.{field} must be an array of repository-relative paths")
                return []
            valid: list[str] = []
            for raw in values:
                relative = PurePosixPath(raw)
                if relative.is_absolute() or ".." in relative.parts or not relative.parts:
                    errors.append(f"{prefix}.{field} contains an unsafe path: {raw}")
                    continue
                target = (root / Path(*relative.parts)).resolve()
                if root.resolve() not in target.parents and target != root.resolve():
                    errors.append(f"{prefix}.{field} escapes the repository: {raw}")
                    continue
                if not target.exists():
                    errors.append(f"{prefix}.{field} path does not exist: {raw}")
                    continue
                valid.append(raw)
            return valid

        evidence = check_paths("evidence")
        verification = check_paths("verification")

        if status in {"implemented", "partial"}:
            if not evidence:
                errors.append(f"{prefix} claims implementation but has no valid evidence")
            if not verification:
                errors.append(f"{prefix} claims implementation but has no valid verification path")
        if status == "not_applicable" and not _is_text(control.get("assessment")):
            errors.append(f"{prefix} must explain why the capability is not applicable")
        if status in {"planned", "not_assessed"} and not _is_text(control.get("assessment")):
            errors.append(f"{prefix} must explain the planned or unassessed state")

    if control_groups != group_ids:
        errors.append("every source group must have exactly one project assessment")
    return errors


def load_profile(path: Path = PROFILE_PATH) -> Any:
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def run_self_tests(profile: Any) -> None:
    if validate_profile(profile, ROOT):
        raise AssertionError("the committed profile must pass before negative tests")

    broken = copy.deepcopy(profile)
    broken["controls"].pop()
    if not validate_profile(broken, ROOT):
        raise AssertionError("missing control was not rejected")

    broken = copy.deepcopy(profile)
    broken["controls"][0]["status"] = "implemented_without_evidence"
    if not validate_profile(broken, ROOT):
        raise AssertionError("unknown status was not rejected")

    broken = copy.deepcopy(profile)
    broken["controls"][0]["evidence"] = []
    if not validate_profile(broken, ROOT):
        raise AssertionError("implementation claim without evidence was not rejected")

    broken = copy.deepcopy(profile)
    broken["controls"][0]["source_points"] = [2, 14]
    if not validate_profile(broken, ROOT):
        raise AssertionError("incorrect source-point range was not rejected")

    broken = copy.deepcopy(profile)
    broken["controls"][0]["evidence"] = ["../outside-the-repository"]
    if not validate_profile(broken, ROOT):
        raise AssertionError("repository path traversal was not rejected")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--self-test", action="store_true", help="run validator negative-case checks")
    args = parser.parse_args()

    try:
        profile = load_profile()
    except (OSError, json.JSONDecodeError) as error:
        print(f"System capability profile: cannot load {PROFILE_PATH}: {error}", file=sys.stderr)
        return 2

    errors = validate_profile(profile)
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    print(f"System capability profile: {profile['project']} — 66 source points, 5 assessed domains")
    if args.self_test:
        try:
            run_self_tests(profile)
        except AssertionError as error:
            print(f"SELF-TEST FAILED: {error}", file=sys.stderr)
            return 1
        print("System capability profile validator self-tests: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
