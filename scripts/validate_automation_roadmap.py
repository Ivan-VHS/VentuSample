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


def validate_recovery_contract(repository: str) -> None:
    policy_path = ROOT / ".architecture" / "automation-rerun-policy.v1.json"
    schema_path = ROOT / ".architecture" / "automation-rerun-policy.v1.schema.json"
    workflow_path = ROOT / ".github" / "workflows" / "safe-ci-recovery.yml"
    validator_path = ROOT / "scripts" / "validate_safe_rerun.py"
    tests_path = ROOT / "tests" / "test_safe_rerun.py"
    issue_path = ROOT / ".github" / "ISSUE_TEMPLATE" / "engineering_improvement.yml"

    for path in (policy_path, schema_path, workflow_path, validator_path, tests_path, issue_path):
        if not path.is_file():
            fail(f"safe recovery / prioritization control is missing: {path.relative_to(ROOT)}")

    policy = load_json(policy_path)
    schema = load_json(schema_path)
    if policy.get("schema") != "anatidae.safe-ci-rerun.v1":
        fail("unsupported safe CI rerun policy schema")
    if policy.get("repository") != repository:
        fail("safe CI rerun policy repository does not match project metadata")
    if policy.get("max_attempts") != 2 or policy.get("same_repository_only") is not True:
        fail("safe CI rerun must enforce one retry and same-repository runs")
    if set(policy.get("allowed_conclusions", [])) != {"failure", "timed_out"}:
        fail("safe CI rerun may only allow failure and timed_out conclusions")
    if not policy.get("allowed_workflow_names"):
        fail("safe CI rerun allowlist is empty")
    if schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
        fail("safe CI rerun schema must use JSON Schema Draft 2020-12")
    if schema.get("properties", {}).get("max_attempts", {}).get("const") != 2:
        fail("safe CI rerun schema must enforce the two-attempt ceiling")
    if schema.get("properties", {}).get("same_repository_only", {}).get("const") is not True:
        fail("safe CI rerun schema must require same-repository runs")

    workflow = workflow_path.read_text(encoding="utf-8")
    if "workflow_dispatch:" not in workflow:
        fail("safe CI recovery must require manual workflow_dispatch")
    if "  push:" in workflow or "  pull_request:" in workflow or "  schedule:" in workflow:
        fail("safe CI recovery must not run automatically")
    for token in (
        "if: github.ref == 'refs/heads/main'",
        "actions: write",
        "contents: read",
        "validate_safe_rerun.py",
        "rerun-failed-jobs",
    ):
        if token not in workflow:
            fail(f"safe CI recovery workflow is missing required guard: {token}")

    validator = validator_path.read_text(encoding="utf-8")
    for token in ("head_repository", "run_attempt", "allowed_workflow_names", "max_attempts"):
        if token not in validator:
            fail(f"safe CI rerun validator does not enforce: {token}")

    tests = tests_path.read_text(encoding="utf-8")
    for token in ("test_fork_pull_request_is_rejected", "test_only_one_retry_is_permitted",
                  "test_committed_policy_and_schema_are_valid"):
        if token not in tests:
            fail(f"safe CI rerun regression coverage is missing: {token}")

    issue_form = issue_path.read_text(encoding="utf-8")
    for token in ("id: priority", "id: impact", "id: acceptance", "id: risks", "id: rollback"):
        if token not in issue_form:
            fail(f"engineering improvement intake is missing required field: {token}")


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

    validate_recovery_contract(project["repository"])

    print(
        f"Validated {len(tasks)} workstreams for {project['repository']}: "
        f"{states['active']} active, {states['extend']} to extend, "
        f"{states['planned']} planned. Evidence and safety gates are present."
    )
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
