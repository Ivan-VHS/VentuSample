#!/usr/bin/env python3
"""Validate the repository's machine-readable automation roadmap (stdlib only)."""
from __future__ import annotations

import argparse
import json
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ROADMAP = ROOT / "automation-roadmap.v1.json"
SCHEMA = ROOT / "automation-roadmap.v1.schema.json"
DOC = ROOT / "docs" / "AUTOMATION_ROADMAP.md"

EXPECTED_IDS = [
    "01-plan-governance", "02-organize-triggers", "03-systematize-sync",
    "04-prioritize-change-flow", "05-refresh-project-config", "06-optimize-toolchain",
    "07-synchronize-branches", "08-integrate-projects", "09-modernize-quality-flow",
    "10-automate-diagnostics", "11-structure-test-updates", "12-scale-validation",
    "13-implement-verification", "14-coordinate-deployment", "15-consolidate-test-matrix",
    "16-improve-recovery", "17-test-controls", "18-verify-observability",
    "19-document-automation", "20-release-publication",
]
REQUIRED_SAFETY = {
    "default_token_permissions": "read",
    "auto_merge_requires_successful_checks": True,
    "automatic_production_deploy": False,
    "automatic_driver_or_firmware_install": False,
    "automatic_secret_rotation": False,
    "automatic_destructive_cleanup": False,
    "automatic_publication_without_release_gate": False,
}
REQUIRED_FIELDS = (
    "id", "intent_ru", "automation", "priority", "status", "objective",
    "target", "trigger", "current_controls", "guardrails", "acceptance",
)

def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)

def load_json(path: Path) -> dict:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError:
        fail(f"required file is missing: {path.relative_to(ROOT)}")
    except json.JSONDecodeError as exc:
        fail(f"invalid JSON in {path.relative_to(ROOT)}: {exc}")

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--strict", action="store_true", help="fail on missing evidence or documentation drift")
    parser.parse_args()

    data = load_json(ROADMAP)
    schema = load_json(SCHEMA)
    if data.get("schema") != "anatidae.automation-roadmap.v1":
        fail("unsupported roadmap schema identifier")
    if schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
        fail("schema must declare JSON Schema 2020-12")
    if schema.get("properties", {}).get("schema", {}).get("const") != data["schema"]:
        fail("roadmap and schema identifiers do not match")

    project = data.get("project", {})
    if not all(project.get(key) for key in ("id", "name", "repository", "stack", "focus")):
        fail("project metadata is incomplete")
    if project.get("default_branch") != "main":
        fail("default branch must be main")

    safety = data.get("safety", {})
    for key, expected in REQUIRED_SAFETY.items():
        if safety.get(key) != expected:
            fail(f"unsafe or missing safety setting: {key} must be {expected!r}")

    phases = data.get("phases")
    if not isinstance(phases, list) or len(phases) != 3:
        fail("exactly three execution phases are required")

    tasks = data.get("tasks")
    if not isinstance(tasks, list) or len(tasks) != 20:
        fail("the roadmap must contain exactly 20 workstreams")
    ids = [item.get("id") for item in tasks if isinstance(item, dict)]
    if ids != EXPECTED_IDS:
        fail("workstream identifiers are missing, duplicated, or out of canonical order")

    states = Counter()
    for index, task in enumerate(tasks, start=1):
        missing = [field for field in REQUIRED_FIELDS if field not in task]
        if missing:
            fail(f"task {index:02d} is missing fields: {', '.join(missing)}")
        if task["status"] not in {"active", "extend", "planned"}:
            fail(f"task {task['id']} has an invalid status")
        if task["priority"] not in {"P0", "P1", "P2"}:
            fail(f"task {task['id']} has an invalid priority")
        for field in ("automation", "trigger", "current_controls", "guardrails"):
            if not isinstance(task[field], list) or not task[field]:
                fail(f"task {task['id']} must have a non-empty {field} list")
        if task["status"] in {"active", "extend"} and not task["current_controls"]:
            fail(f"task {task['id']} claims active coverage without evidence")
        for evidence in task["current_controls"]:
            path = ROOT / evidence
            if not path.is_file():
                fail(f"task {task['id']} references missing evidence: {evidence}")
        states[task["status"]] += 1

    if not DOC.is_file():
        fail("docs/AUTOMATION_ROADMAP.md is missing")
    doc = DOC.read_text(encoding="utf-8")
    missing_docs = [task_id for task_id in EXPECTED_IDS if task_id not in doc]
    if missing_docs:
        fail("documentation is missing workstreams: " + ", ".join(missing_docs))

    workflow = ROOT / ".github" / "workflows" / "automation-roadmap.yml"
    if not workflow.is_file():
        fail("automation-roadmap.yml is missing")
    workflow_text = workflow.read_text(encoding="utf-8")
    for token in ("pull_request:", "workflow_dispatch:", "schedule:", "contents: read"):
        if token not in workflow_text:
            fail(f"automation roadmap workflow is missing required control: {token}")

    gate_relative = (
        ".github/workflows/ci.yml"
        if project["id"] == "ventusample"
        else ".github/workflows/verified-auto-merge.yml"
    )
    gate = ROOT / gate_relative
    if not gate.is_file():
        fail(f"existing merge/CI gate is missing: {gate_relative}")
    if "python scripts/validate_automation_roadmap.py --strict" not in gate.read_text(encoding="utf-8"):
        fail(f"roadmap validation is not wired into the existing merge/CI gate: {gate_relative}")

    print(
        f"Validated {len(tasks)} workstreams for {project['repository']}: "
        f"{states['active']} active, {states['extend']} to extend, "
        f"{states['planned']} planned. Evidence and safety gates are present."
    )
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
