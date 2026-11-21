#!/usr/bin/env python3
"""Validate the quality, repeatability and native-integration policy manifest."""
from __future__ import annotations
import argparse, json, sys, tempfile
from pathlib import Path
from typing import Any
SCHEMA = "anatidae.quality-repeatability-native.v1"
PILLARS = ("quality", "multiplicity", "native")
STATES = {"implemented", "planned", "not_applicable"}
def evidence_file(root: Path, relative: Any) -> bool:
    if not isinstance(relative, str) or not relative.strip(): return False
    path = Path(relative)
    if path.is_absolute() or ".." in path.parts: return False
    resolved = (root / path).resolve()
    try: resolved.relative_to(root.resolve())
    except ValueError: return False
    return resolved.is_file()
def validate_contract(data: Any, root: Path) -> list[str]:
    errors: list[str] = []
    if not isinstance(data, dict): return ["contract root must be a JSON object"]
    if data.get("schema") != SCHEMA: errors.append(f"schema must equal {SCHEMA!r}")
    project = data.get("project")
    if not isinstance(project, dict): errors.append("project must be an object"); project = {}
    for key in ("id", "name", "runtime"):
        if not isinstance(project.get(key), str) or not project[key].strip(): errors.append(f"project.{key} must be a non-empty string")
    surfaces = project.get("native_surfaces")
    if not isinstance(surfaces, list) or not surfaces or any(not isinstance(v, str) or not v.strip() for v in surfaces):
        errors.append("project.native_surfaces must be a non-empty list of strings")
    pillars = data.get("pillars")
    if not isinstance(pillars, dict): return errors + ["pillars must be an object"]
    for name in pillars:
        if name not in PILLARS: errors.append(f"unknown pillar: {name}")
    for name in PILLARS:
        if name not in pillars: errors.append(f"missing pillar: {name}")
    seen: set[str] = set()
    for name in PILLARS:
        pillar = pillars.get(name)
        if not isinstance(pillar, dict): continue
        if not isinstance(pillar.get("intent"), str) or not pillar["intent"].strip(): errors.append(f"{name}.intent must be a non-empty string")
        criteria = pillar.get("criteria")
        if not isinstance(criteria, list) or len(criteria) < 3: errors.append(f"{name}.criteria must contain at least 3 entries"); continue
        for i, item in enumerate(criteria):
            label = f"{name}.criteria[{i}]"
            if not isinstance(item, dict): errors.append(f"{label} must be an object"); continue
            ident = item.get("id")
            if not isinstance(ident, str) or not ident.strip(): errors.append(f"{label}.id must be a non-empty string")
            elif ident in seen: errors.append(f"duplicate criterion id: {ident}")
            else: seen.add(ident)
            for key in ("requirement", "verification"):
                if not isinstance(item.get(key), str) or not item[key].strip(): errors.append(f"{label}.{key} must be a non-empty string")
            if item.get("status") not in STATES: errors.append(f"{label}.status must be one of {sorted(STATES)}")
            evidence = item.get("evidence")
            if not isinstance(evidence, dict): errors.append(f"{label}.evidence must contain implementation and verification"); continue
            for kind in ("implementation", "verification"):
                paths = evidence.get(kind)
                if not isinstance(paths, list) or not paths: errors.append(f"{label}.evidence.{kind} must be a non-empty list")
                else:
                    for path in paths:
                        if not evidence_file(root, path): errors.append(f"{label}: missing or unsafe evidence path {path!r}")
    return errors
def self_test() -> None:
    with tempfile.TemporaryDirectory() as temp:
        root = Path(temp)
        for name in ("source.py", "test.py"): (root / name).write_text("fixture\n", encoding="utf-8")
        def criterion(ident: str) -> dict[str, Any]:
            return {"id": ident, "status": "implemented", "requirement": "A concrete requirement.", "verification": "A test covers the behavior.", "evidence": {"implementation": ["source.py"], "verification": ["test.py"]}}
        valid = {"schema": SCHEMA, "project": {"id": "fixture", "name": "Fixture", "runtime": "Python", "native_surfaces": ["Python API"]}, "pillars": {name: {"intent": "Explicit intent.", "criteria": [criterion(f"{name}-{i}") for i in range(3)]} for name in PILLARS}}
        assert not validate_contract(valid, root), "valid fixture rejected"
        bad = json.loads(json.dumps(valid)); bad["pillars"]["native"]["criteria"][0]["evidence"]["verification"] = ["../outside"]; assert validate_contract(bad, root), "path traversal accepted"
        bad = json.loads(json.dumps(valid)); bad["pillars"]["quality"]["criteria"].pop(); assert validate_contract(bad, root), "incomplete pillar accepted"
        bad = json.loads(json.dumps(valid)); bad["pillars"]["multiplicity"]["criteria"][0]["id"] = "quality-0"; assert validate_contract(bad, root), "duplicate criterion accepted"
def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument("--self-test", action="store_true"); parser.add_argument("--json", action="store_true"); args = parser.parse_args()
    if args.self_test: self_test()
    root = Path(__file__).resolve().parents[1]; manifest = root / "quality-repeatability-native.v1.json"
    try: data = json.loads(manifest.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc: print(f"manifest load failed: {exc}", file=sys.stderr); return 2
    errors = validate_contract(data, root); pillars = data.get("pillars", {}) if isinstance(data, dict) else {}
    counts = {name: len(pillars.get(name, {}).get("criteria", [])) for name in PILLARS}
    report = {"schema": SCHEMA, "project": data.get("project", {}).get("id") if isinstance(data, dict) else None, "criteria": counts, "total_criteria": sum(counts.values()), "errors": errors, "self_test": "passed" if args.self_test else "not_run"}
    if args.json: print(json.dumps(report, ensure_ascii=False, indent=2))
    else:
        print(f"{report['project']}: {report['total_criteria']} criteria across {len(PILLARS)} pillars")
        print("self-test: passed" if args.self_test else "self-test: not requested")
        for error in errors: print(f"ERROR: {error}", file=sys.stderr)
    return 1 if errors else 0
if __name__ == "__main__": raise SystemExit(main())
