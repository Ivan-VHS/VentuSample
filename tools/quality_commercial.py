"""Evidence-backed quality, operations and commercial readiness evaluator."""
from __future__ import annotations
import argparse, json, math, sys
from pathlib import Path, PurePosixPath
from typing import Any, Mapping

SCHEMA = "anatidae.quality-commercial.v1"
AXES = ("quality", "execution", "commercial")
POINTS = {"implemented": 1.0, "planned": 0.25, "not_assessed": 0.0, "not_applicable": None}


class QualityProfileError(ValueError):
    """Invalid quality-commercial profile."""


def _load(source: str | Path | Mapping[str, Any]) -> dict[str, Any]:
    if isinstance(source, Mapping):
        return dict(source)
    try:
        value = json.loads(Path(source).read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise QualityProfileError(f"cannot load profile: {error}") from error
    if not isinstance(value, dict):
        raise QualityProfileError("profile root must be an object")
    return value


def _evidence_path(raw: Any) -> PurePosixPath:
    if not isinstance(raw, str) or not raw or "\\" in raw or "\x00" in raw:
        raise QualityProfileError("evidence paths must be repository-relative POSIX paths")
    path = PurePosixPath(raw)
    if path.is_absolute() or any(part in {"", ".", ".."} for part in path.parts):
        raise QualityProfileError(f"unsafe evidence path: {raw!r}")
    return path


def assess_profile(source: str | Path | Mapping[str, Any], *, root: str | Path | None = None) -> dict[str, Any]:
    """Validate all 60 controls, verify cited files and calculate declared maturity."""
    profile = _load(source)
    if profile.get("schema") != SCHEMA:
        raise QualityProfileError(f"schema must be {SCHEMA}")
    project = profile.get("project")
    if not isinstance(project, dict) or not all(isinstance(project.get(k), str) and project[k].strip() for k in ("id", "name", "scope")):
        raise QualityProfileError("project must declare id, name and scope")
    criteria = profile.get("criteria")
    if not isinstance(criteria, list) or len(criteria) != 20:
        raise QualityProfileError("exactly 20 criteria are required")
    if profile.get("scoring") != POINTS:
        raise QualityProfileError("scoring weights differ from the shared standard")
    operations = profile.get("commercial_operations")
    required = {"market_validation", "pricing", "unit_economics", "license_review", "support_model", "distribution", "boundary"}
    if not isinstance(operations, dict) or not required.issubset(operations):
        raise QualityProfileError("commercial_operations is incomplete")
    base = Path(root).resolve() if root is not None else (Path(source).resolve().parent if not isinstance(source, Mapping) else Path.cwd().resolve())
    sums = {a: [0.0, 0.0, 0] for a in AXES}
    issues, rows = [], []
    refs = found = 0
    seen = set()
    for index, row in enumerate(criteria):
        if not isinstance(row, dict):
            raise QualityProfileError(f"criteria[{index}] must be an object")
        ident = row.get("id")
        if not isinstance(ident, str) or not ident or ident in seen:
            raise QualityProfileError(f"criteria[{index}] has an empty or duplicate id")
        seen.add(ident)
        terms, controls = row.get("terms"), row.get("controls")
        if not isinstance(row.get("intent"), str) or not row["intent"].strip():
            raise QualityProfileError(f"{ident}: missing intent")
        if not isinstance(terms, dict) or set(terms) != set(AXES) or any(not isinstance(terms[a], str) or not terms[a].strip() for a in AXES):
            raise QualityProfileError(f"{ident}: all three terms are required")
        if not isinstance(controls, dict) or set(controls) != set(AXES):
            raise QualityProfileError(f"{ident}: all three controls are required")
        weight = row.get("weight", 1)
        if isinstance(weight, bool) or not isinstance(weight, (int, float)) or not math.isfinite(weight) or weight <= 0:
            raise QualityProfileError(f"{ident}: weight must be positive and finite")
        rendered = {}
        for axis in AXES:
            control = controls[axis]
            if not isinstance(control, dict) or control.get("status") not in POINTS:
                raise QualityProfileError(f"{ident}.{axis}: invalid control status")
            paths = control.get("evidence")
            if not isinstance(paths, list) or not paths:
                raise QualityProfileError(f"{ident}.{axis}: at least one evidence file is required")
            present, missing = [], []
            for raw in paths:
                rel = _evidence_path(raw)
                refs += 1
                target = (base / Path(*rel.parts)).resolve()
                try:
                    target.relative_to(base)
                except ValueError as error:
                    raise QualityProfileError(f"{ident}.{axis}: evidence escapes repository root") from error
                if target.is_file():
                    present.append(rel.as_posix())
                    found += 1
                else:
                    missing.append(rel.as_posix())
                    issues.append(f"{ident}.{axis}: missing evidence file {rel.as_posix()}")
            status = control["status"]
            points = POINTS[status]
            if points is not None:
                sums[axis][0] += points * weight
                sums[axis][1] += weight
                sums[axis][2] += 1
            rendered[axis] = {"term": terms[axis], "status": status, "evidence_found": present, "evidence_missing": missing}
        rows.append({"id": ident, "intent": row["intent"], "controls": rendered})
    axes = {}
    for axis, (earned, maximum, count) in sums.items():
        axes[axis] = {"maturity_percent": round(100 * earned / maximum, 1) if maximum else None,
                      "weighted_points": round(earned, 3), "maximum_points": round(maximum, 3),
                      "applicable_criteria": count}
    open_items = [{"field": k, "status": v} for k, v in operations.items()
                  if k != "boundary" and v in {"not_assessed", "not_set", "manual_required", "planned"}]
    scores = [item["maturity_percent"] for item in axes.values() if item["maturity_percent"] is not None]
    return {"schema": "anatidae.quality-commercial-report.v1", "project": project, "criteria_count": len(rows),
            "evidence": {"references": refs, "files_found": found, "files_missing": refs-found,
                         "coverage_percent": round(100*found/refs, 1) if refs else 100.0},
            "axes": axes, "overall_declared_maturity_percent": round(sum(scores)/len(scores), 1) if scores else None,
            "commercial_operations": operations, "commercial_open_items": open_items, "issues": issues, "criteria": rows,
            "limitations": ["Declared maturity is not a certification or warranty.",
             "File presence is checked; referenced code and tests are not executed by this evaluator.",
             "Market demand, sales, revenue, profitability, valuation and investment readiness require independent evidence."]}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Assess the 20 x 3 quality, execution and commercial evidence matrix.")
    parser.add_argument("--profile", default="quality-commercial.v1.json")
    parser.add_argument("--format", choices=("text", "json"), default="text")
    args = parser.parse_args(argv)
    try:
        report = assess_profile(args.profile)
    except QualityProfileError as error:
        print(f"quality-commercial: {error}", file=sys.stderr)
        return 2
    if args.format == "json":
        print(json.dumps(report, ensure_ascii=False, indent=2))
    else:
        print(f"{report['project']['name']} — 20 x 3 readiness matrix")
        print(f"Evidence files: {report['evidence']['files_found']}/{report['evidence']['references']} ({report['evidence']['coverage_percent']:.1f}%)")
        for axis, value in report["axes"].items():
            print(f"{axis}: {value['maturity_percent']}% declared maturity")
        print(f"Overall declared maturity: {report['overall_declared_maturity_percent']}%")
        for item in report["commercial_open_items"]:
            print(f"OPEN {item['field']}: {item['status']}")
        for issue in report["issues"]:
            print(f"ERROR {issue}", file=sys.stderr)
    return 1 if report["issues"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
