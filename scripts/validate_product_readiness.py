#!/usr/bin/env python3
"""Validate the repository-local product-readiness passport (stdlib only)."""
from __future__ import annotations

import argparse
import json
import re
import sys
import unittest
from pathlib import Path, PurePosixPath
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_MANIFEST = ROOT / "product-readiness.json"
EXPECTED_SCHEMA = "ivan-vhs.product-readiness.v1"
AXES = ("quality", "cohesion", "commercial")
ALLOWED = {"implemented", "partial", "planned", "not_applicable"}
WEIGHTS = {"implemented": 1.0, "partial": 0.5, "planned": 0.0}


def axis_coverage(controls: list[dict[str, Any]]) -> dict[str, Any]:
    applicable = [item for item in controls if item.get("status") != "not_applicable"]
    if not applicable:
        return {"score": 0.0, "implemented": 0, "partial": 0, "planned": 0, "applicable": 0}
    points = sum(WEIGHTS.get(item.get("status"), 0.0) for item in applicable)
    counts = {status: sum(item.get("status") == status for item in controls)
              for status in ("implemented", "partial", "planned")}
    return {
        "score": round(points / len(applicable) * 100, 1),
        **counts,
        "applicable": len(applicable),
    }


def _text(value: Any) -> bool:
    return isinstance(value, str) and bool(value.strip())


def validate_manifest(manifest_path: Path) -> tuple[list[str], dict[str, Any] | None]:
    errors: list[str] = []
    try:
        raw = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        return [f"cannot read valid UTF-8 JSON manifest: {exc}"], None

    if not isinstance(raw, dict):
        return ["manifest root must be a JSON object"], None
    if raw.get("schema") != EXPECTED_SCHEMA:
        errors.append(f"schema must equal {EXPECTED_SCHEMA!r}")

    project = raw.get("project")
    if not isinstance(project, dict):
        errors.append("project must be an object")
        project = {}
    for key in ("id", "name", "repository", "product_type", "value_proposition"):
        if not _text(project.get(key)):
            errors.append(f"project.{key} must be a non-empty string")
    if _text(project.get("id")) and not re.fullmatch(r"[a-z][a-z0-9-]{1,40}", project["id"]):
        errors.append("project.id must be a lowercase stable identifier")
    if _text(project.get("repository")) and not re.fullmatch(r"[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+", project["repository"]):
        errors.append("project.repository must use owner/name form")
    for key in ("primary_users", "jobs_to_be_done"):
        value = project.get(key)
        if not isinstance(value, list) or len(value) < 2 or not all(_text(item) for item in value):
            errors.append(f"project.{key} must contain at least two non-empty strings")

    commercial_position = raw.get("commercial_position")
    if not isinstance(commercial_position, dict):
        errors.append("commercial_position must be an object")
        commercial_position = {}
    for key in ("model", "license", "pricing", "paid_support", "support_route", "privacy_posture"):
        if not _text(commercial_position.get(key)):
            errors.append(f"commercial_position.{key} must be a non-empty string")
    if commercial_position.get("pricing") not in {"not-published", "not-established"}:
        errors.append("pricing must remain explicitly not-published or not-established")
    distribution = commercial_position.get("distribution")
    if not isinstance(distribution, list) or not distribution or not all(_text(item) for item in distribution):
        errors.append("commercial_position.distribution must list at least one real delivery channel")

    axis_reports: dict[str, Any] = {}
    seen_ids: set[str] = set()
    for axis in AXES:
        controls = raw.get(axis)
        if not isinstance(controls, list) or len(controls) != 5:
            errors.append(f"{axis} must contain exactly five controls")
            controls = controls if isinstance(controls, list) else []
        for index, item in enumerate(controls, start=1):
            prefix = f"{axis}[{index}]"
            if not isinstance(item, dict):
                errors.append(f"{prefix} must be an object")
                continue
            control_id = item.get("id")
            if not _text(control_id):
                errors.append(f"{prefix}.id must be a non-empty string")
            elif control_id in seen_ids:
                errors.append(f"duplicate control id: {control_id}")
            else:
                seen_ids.add(control_id)
            if item.get("status") not in ALLOWED:
                errors.append(f"{prefix}.status must be one of {sorted(ALLOWED)}")
            if not _text(item.get("requirement")):
                errors.append(f"{prefix}.requirement must state an acceptance condition")
            evidence = item.get("evidence", [])
            if not isinstance(evidence, list) or not all(_text(path) for path in evidence):
                errors.append(f"{prefix}.evidence must be an array of repository-relative paths")
                evidence = []
            status = item.get("status")
            if status in {"implemented", "partial"} and not evidence:
                errors.append(f"{prefix} needs evidence while status is {status}")
            if status == "not_applicable" and not _text(item.get("reason")):
                errors.append(f"{prefix}.reason is required for not_applicable")
            for evidence_path in evidence:
                relative = PurePosixPath(evidence_path)
                if relative.is_absolute() or ".." in relative.parts or "\\" in evidence_path:
                    errors.append(f"{prefix} has unsafe evidence path: {evidence_path}")
                    continue
                candidate = ROOT.joinpath(*relative.parts)
                try:
                    resolved = candidate.resolve(strict=True)
                except OSError:
                    errors.append(f"{prefix} evidence file does not exist: {evidence_path}")
                    continue
                if ROOT.resolve() not in resolved.parents or not resolved.is_file() or candidate.is_symlink():
                    errors.append(f"{prefix} evidence is not a regular repository file: {evidence_path}")
        axis_reports[axis] = axis_coverage([item for item in controls if isinstance(item, dict)])

    gaps = raw.get("known_gaps")
    if not isinstance(gaps, list) or len(gaps) < 2 or not all(_text(item) for item in gaps):
        errors.append("known_gaps must contain at least two explicit limitations")

    scores = [axis_reports[axis]["score"] for axis in AXES]
    report = {
        "schema": EXPECTED_SCHEMA,
        "project": project.get("name"),
        "repository": project.get("repository"),
        "evidence_coverage": {
            "quality": axis_reports.get("quality"),
            "cohesion": axis_reports.get("cohesion"),
            "commercial": axis_reports.get("commercial"),
            "overall": round(sum(scores) / len(scores), 1),
            "interpretation": "Evidence coverage only; not certification, market validation, financial performance or a product-quality rating.",
        },
    }
    return errors, report


class ValidatorSelfTests(unittest.TestCase):
    def test_fully_implemented_axis(self) -> None:
        self.assertEqual(axis_coverage([{"status": "implemented"}] * 5)["score"], 100.0)

    def test_partial_and_planned_are_not_overstated(self) -> None:
        controls = [{"status": "implemented"}, {"status": "partial"},
                    {"status": "planned"}, {"status": "implemented"}]
        self.assertEqual(axis_coverage(controls)["score"], 62.5)

    def test_not_applicable_is_excluded(self) -> None:
        controls = [{"status": "implemented"}, {"status": "not_applicable"}]
        self.assertEqual(axis_coverage(controls)["score"], 100.0)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST,
                        help="manifest path (default: product-readiness.json)")
    parser.add_argument("--json", action="store_true", help="emit a machine-readable report")
    parser.add_argument("--self-test", action="store_true", help="run validator scoring self-tests")
    args = parser.parse_args()

    if args.self_test:
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(ValidatorSelfTests)
        result = unittest.TextTestRunner(verbosity=2).run(suite)
        return 0 if result.wasSuccessful() else 1

    errors, report = validate_manifest(args.manifest.resolve())
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1
    assert report is not None
    if args.json:
        print(json.dumps(report, ensure_ascii=False, indent=2))
    else:
        coverage = report["evidence_coverage"]
        print(f"{report['project']} — product-readiness contract valid")
        for axis in AXES:
            item = coverage[axis]
            print(f"  {axis:10} {item['score']:5.1f}%  "
                  f"(implemented={item['implemented']}, partial={item['partial']}, "
                  f"planned={item['planned']})")
        print(f"  overall    {coverage['overall']:5.1f}% evidence coverage")
        print("  Note: evidence coverage is not certification or market validation.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
