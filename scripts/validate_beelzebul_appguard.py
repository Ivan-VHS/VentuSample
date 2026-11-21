#!/usr/bin/env python3
"""Validate Beelzebul AppGuard catalog structure; does not enforce runtime security."""
from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / ".beelzebul" / "appguard.v1.json"
SCHEMA = ROOT / "schemas" / "beelzebul-appguard.v1.schema.json"

EXPECTED_ECHELONS = [
    ("I", 1, 111), ("II", 112, 222), ("III", 223, 333),
    ("IV", 334, 444), ("V", 445, 555), ("VI", 556, 666),
]
EXPECTED_PREFIX_COUNTS = {"A": 15, "M": 15, "Y": 15, "K": 15, "O": 6}
ACTIVE_STATES = {"implemented", "verified"}


def _read_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        value = json.load(stream)
    if not isinstance(value, dict):
        raise ValueError(f"{path.name}: root must be a JSON object")
    return value


def validate_manifest(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    if data.get("schema_version") != "beelzebul.appguard/v1":
        errors.append("schema_version must be beelzebul.appguard/v1")
    if data.get("catalog_status") != "policy_catalog_only":
        errors.append("catalog_status must remain policy_catalog_only")
    if data.get("runtime_enforcement") != "not_implemented_by_catalog":
        errors.append("runtime_enforcement must not imply runtime activation")

    echelons = data.get("echelons")
    if not isinstance(echelons, list) or len(echelons) != 6:
        errors.append("exactly six echelons are required")
        echelons = []
    for index, expected in enumerate(EXPECTED_ECHELONS):
        if index >= len(echelons) or not isinstance(echelons[index], dict):
            errors.append(f"missing echelon at index {index}")
            continue
        item = echelons[index]
        actual = (item.get("id"), item.get("start"), item.get("end"))
        if actual != expected:
            errors.append(f"echelon {expected[0]} range must be {expected[1]}..{expected[2]}")
        domains = item.get("domains")
        if not isinstance(domains, list) or len(domains) != 11 or any(not isinstance(v, str) or not v.strip() for v in domains):
            errors.append(f"echelon {expected[0]} must define exactly eleven non-empty domains")

    levels = data.get("levels")
    if not isinstance(levels, list) or len(levels) != 666:
        errors.append("levels must contain exactly 666 entries")
        levels = []

    seen: set[str] = set()
    counts_by_echelon = {item[0]: 0 for item in EXPECTED_ECHELONS}
    active_claims = 0
    for index, item in enumerate(levels):
        if not isinstance(item, dict):
            errors.append(f"levels[{index}] must be an object")
            continue
        number = index + 1
        expected_id = f"L{number:03d}"
        level_id = item.get("level_id")
        if level_id != expected_id:
            errors.append(f"levels[{index}] must have sequential id {expected_id}")
        if level_id in seen:
            errors.append(f"duplicate level id: {level_id}")
        seen.add(str(level_id))
        if item.get("number") != number:
            errors.append(f"{expected_id}: number must equal {number}")

        expected_echelon = next((eid for eid, start, end in EXPECTED_ECHELONS if start <= number <= end), None)
        if item.get("echelon_id") != expected_echelon:
            errors.append(f"{expected_id}: incorrect echelon_id")
        if expected_echelon:
            counts_by_echelon[expected_echelon] += 1
        for field in ("subsystem", "control_id", "control_title", "objective"):
            if not isinstance(item.get(field), str) or not item[field].strip():
                errors.append(f"{expected_id}: {field} must be a non-empty string")
        state = item.get("implementation_state")
        enforcement = item.get("enforcement")
        source_refs = item.get("source_refs")
        test_refs = item.get("test_refs")
        verification_refs = item.get("verification_refs")
        for field, value in (("source_refs", source_refs), ("test_refs", test_refs), ("verification_refs", verification_refs)):
            if not isinstance(value, list) or any(not isinstance(ref, str) or not ref.strip() for ref in value):
                errors.append(f"{expected_id}: {field} must be an array of non-empty strings")
        if state in ACTIVE_STATES:
            active_claims += 1
            if enforcement != "enforced":
                errors.append(f"{expected_id}: active state requires enforcement=enforced")
            if not isinstance(source_refs, list) or not source_refs:
                errors.append(f"{expected_id}: active state requires source_refs")
            if not isinstance(test_refs, list) or not test_refs:
                errors.append(f"{expected_id}: active state requires test_refs")
            if state == "verified" and (not isinstance(verification_refs, list) or not verification_refs):
                errors.append(f"{expected_id}: verified state requires verification_refs")
        elif state == "partial":
            if enforcement != "partially_enforced":
                errors.append(f"{expected_id}: partial state requires enforcement=partially_enforced")
            if not isinstance(source_refs, list) or not source_refs or not isinstance(test_refs, list) or not test_refs:
                errors.append(f"{expected_id}: partial state requires source_refs and test_refs")
        elif state in {"planned", "specified", "deferred", "not_applicable"}:
            if enforcement != "not_enforced":
                errors.append(f"{expected_id}: non-active state requires enforcement=not_enforced")
        else:
            errors.append(f"{expected_id}: unknown implementation_state {state!r}")

    for echelon_id, start, end in EXPECTED_ECHELONS:
        if counts_by_echelon[echelon_id] != 111:
            errors.append(f"echelon {echelon_id} must contain exactly 111 levels")

    if data.get("implementation_claims") != active_claims:
        errors.append(f"implementation_claims must equal implemented + verified count ({active_claims})")

    groups = data.get("repository_rule_groups")
    if not isinstance(groups, list) or len(groups) != 5:
        errors.append("repository_rule_groups must contain A, M, Y, K and O")
        groups = []
    gathered: list[str] = []
    seen_prefixes: set[str] = set()
    for group in groups:
        if not isinstance(group, dict):
            errors.append("each repository rule group must be an object")
            continue
        prefix = group.get("prefix")
        if prefix not in EXPECTED_PREFIX_COUNTS or prefix in seen_prefixes:
            errors.append(f"unknown or duplicate repository rule prefix: {prefix!r}")
            continue
        seen_prefixes.add(prefix)
        expected_count = EXPECTED_PREFIX_COUNTS[prefix]
        ids = group.get("rule_ids")
        if not isinstance(ids, list) or len(ids) != expected_count:
            errors.append(f"{prefix} must contain exactly {expected_count} rule ids")
            continue
        expected_ids = [f"{prefix}-{i:02d}" for i in range(1, expected_count + 1)]
        if ids != expected_ids:
            errors.append(f"{prefix} rule ids must be sequential from {expected_ids[0]} to {expected_ids[-1]}")
        gathered.extend(ids)
        if group.get("count") != expected_count:
            errors.append(f"{prefix}.count must be {expected_count}")
        if group.get("per_rule_mapping_status") != "pending_full_rule_text":
            errors.append(f"{prefix}: per-rule mapping must remain pending until full rule text is linked")
        candidates = group.get("candidate_echelons")
        if not isinstance(candidates, list) or not candidates or any(e not in {x[0] for x in EXPECTED_ECHELONS} for e in candidates):
            errors.append(f"{prefix}: candidate_echelons must reference known echelons")
    if seen_prefixes != set(EXPECTED_PREFIX_COUNTS):
        errors.append("all A, M, Y, K and O rule groups are required")
    if len(gathered) != 66 or len(set(gathered)) != 66:
        errors.append("repository rule crosswalk must contain exactly 66 unique rule ids")

    verdict = data.get("verdict_contract")
    if not isinstance(verdict, dict):
        errors.append("verdict_contract must be an object")
    else:
        if verdict.get("allowed_verdicts") != ["allow", "quarantine", "deny", "degrade"]:
            errors.append("verdict_contract.allowed_verdicts must be allow/quarantine/deny/degrade")
        required_fields = {"level_id", "rule_id", "object_ref", "reason", "verdict", "timestamp"}
        actual_fields = verdict.get("required_event_fields")
        if not isinstance(actual_fields, list) or not required_fields.issubset(set(actual_fields)):
            errors.append("verdict contract is missing required event fields")
        overrides = verdict.get("override_requirements")
        if not isinstance(overrides, list) or not {"owner", "reason", "expires_at"}.issubset(set(overrides)):
            errors.append("overrides must require owner, reason and expires_at")

    profile = data.get("repository_profile")
    if not isinstance(profile, dict):
        errors.append("repository_profile must be an object")
    else:
        for field in ("project", "runtime"):
            if not isinstance(profile.get(field), str) or not profile[field].strip():
                errors.append(f"repository_profile.{field} must be non-empty")
        for field in ("surfaces", "guardrails"):
            if not isinstance(profile.get(field), list) or not profile[field]:
                errors.append(f"repository_profile.{field} must be a non-empty array")

    return errors


def main() -> int:
    try:
        data = _read_json(CATALOG)
        schema = _read_json(SCHEMA)
        if schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
            raise ValueError("schema must declare JSON Schema Draft 2020-12")
        required = set(schema.get("required", []))
        if not required.issubset(data):
            raise ValueError("catalog is missing one or more schema-required top-level fields")
        errors = validate_manifest(data)
        if errors:
            for error in errors:
                print(f"AppGuard catalog: FAIL: {error}", file=sys.stderr)
            return 1
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        print(f"AppGuard catalog: FAIL: {exc}", file=sys.stderr)
        return 1

    print(
        f"AppGuard catalog: PASS — 666 sequential control positions, "
        f"6×111 echelons, 66 repository-rule IDs; "
        f"{data['implementation_claims']} runtime implementation claims."
    )
    print("This validates the catalog only; it does not prove or activate runtime enforcement.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
