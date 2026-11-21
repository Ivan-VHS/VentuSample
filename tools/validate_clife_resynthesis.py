#!/usr/bin/env python3
"""CLife catalytic resynthesis: validate policy, audit Git state, and create reversible recovery points."""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
POLICY = ROOT / ".clife" / "resynthesis-policy.v1.json"
SCHEMA = ROOT / "schemas" / "clife-resynthesis.v1.schema.json"
SCHEMA_ID = "clife.catalytic-resynthesis.v1"
STAGES = [
    "inventory", "baseline", "checkpoint", "isolate", "transform",
    "validate", "compare", "accept_or_recover",
]
CHECKPOINT_RE = re.compile(r"^clife/checkpoint-[a-z0-9][a-z0-9-]{0,39}-[0-9]{8}T[0-9]{6}Z$")
LABEL_RE = re.compile(r"^[a-z0-9][a-z0-9-]{0,39}$")


def _git(root: Path, *args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", "-C", str(root), *args],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )


def validate_policy(data: dict[str, Any], root: Path = ROOT) -> list[str]:
    """Return policy/schema/evidence violations; this function never changes files."""
    errors: list[str] = []
    if data.get("schema") != SCHEMA_ID:
        errors.append("unsupported CLife schema identifier")
    if data.get("version") != 1:
        errors.append("CLife policy version must be 1")

    project = data.get("project")
    if not isinstance(project, dict) or not all(
        isinstance(project.get(k), str) and project[k].strip()
        for k in ("name", "repository", "stack")
    ):
        errors.append("project name, repository and stack are required")

    model = data.get("catalytic_model")
    if not isinstance(model, dict):
        errors.append("catalytic_model is required")
        model = {}
    for key in ("substrate", "washcoat", "active_sites", "oxygen_buffer"):
        if not isinstance(model.get(key), dict):
            errors.append(f"catalytic_model.{key} must be an object")

    substrate = model.get("substrate", {})
    washcoat = model.get("washcoat", {})
    active = model.get("active_sites", {})
    oxygen = model.get("oxygen_buffer", {})
    for key, obj, field in (
        ("substrate", substrate, "protected_paths"),
        ("washcoat", washcoat, "extension_points"),
        ("active_sites", active, "quality_gates"),
    ):
        values = obj.get(field, []) if isinstance(obj, dict) else []
        if not isinstance(values, list) or not values:
            errors.append(f"{key}.{field} must be a non-empty list")
        else:
            for relative in values:
                if not isinstance(relative, str) or not relative:
                    errors.append(f"{key}.{field} contains an invalid path")
                elif not (root / relative).exists():
                    errors.append(f"{key}.{field} references missing path: {relative}")

    if isinstance(oxygen, dict):
        expected = {
            "checkpoint_mode": "annotated_git_tag",
            "recovery_mode": "new_branch_only",
            "checkpoint_requires_clean_worktree": True,
            "automatic_remote_push": False,
        }
        for key, value in expected.items():
            if oxygen.get(key) != value:
                errors.append(f"oxygen_buffer.{key} must be {value!r}")

    window = data.get("lambda_window")
    if not isinstance(window, dict):
        errors.append("lambda_window is required")
    else:
        for key in ("baseline_required", "tests_required", "compatibility_required", "security_review_required"):
            if window.get(key) is not True:
                errors.append(f"lambda_window.{key} must be true")

    diagnostics = data.get("diagnostics")
    if not isinstance(diagnostics, dict) or not isinstance(diagnostics.get("signals"), list) or not diagnostics["signals"]:
        errors.append("diagnostics.signals must be a non-empty list")

    projection = data.get("mega_projection")
    if not isinstance(projection, dict):
        errors.append("mega_projection is required")
    else:
        if projection.get("horizons_days") != [30, 90, 180, 365]:
            errors.append("mega_projection.horizons_days must be [30, 90, 180, 365]")
        if projection.get("forecast_policy") != "measured_evidence_only":
            errors.append("mega_projection.forecast_policy must be measured_evidence_only")
        if projection.get("baseline_status") != "pending_initial_measurement":
            errors.append("baseline must remain pending until real measurements are collected")
        if not isinstance(projection.get("indicators"), list) or not projection["indicators"]:
            errors.append("mega_projection.indicators must be a non-empty list")

    safety = data.get("safety")
    required_safety = {
        "automatic_source_rewrite": False,
        "force_push": False,
        "automatic_merge": False,
        "destructive_rollback": False,
        "manual_review_required_for_acceptance": True,
        "telemetry": "local_only",
    }
    if not isinstance(safety, dict):
        errors.append("safety is required")
    else:
        for key, expected in required_safety.items():
            if safety.get(key) != expected:
                errors.append(f"safety.{key} must be {expected!r}")

    adoption = data.get("adoption")
    if not isinstance(adoption, dict) or adoption.get("stages") != STAGES:
        errors.append("adoption.stages do not match the canonical reversible lifecycle")

    evidence = data.get("evidence_paths")
    if not isinstance(evidence, list) or not evidence:
        errors.append("evidence_paths must be a non-empty list")
    else:
        for relative in evidence:
            if not isinstance(relative, str) or not (root / relative).exists():
                errors.append(f"missing evidence path: {relative}")

    try:
        schema_data = json.loads(SCHEMA.read_text(encoding="utf-8"))
        if schema_data.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
            errors.append("CLife schema must use JSON Schema draft 2020-12")
        if schema_data.get("properties", {}).get("schema", {}).get("const") != SCHEMA_ID:
            errors.append("CLife policy and JSON Schema identifiers differ")
    except (OSError, json.JSONDecodeError) as exc:
        errors.append(f"cannot read CLife JSON Schema: {exc}")

    return errors


def inspect_git(root: Path = ROOT) -> dict[str, Any]:
    """Collect metadata only; no file contents, secrets, or external telemetry are read."""
    inside = _git(root, "rev-parse", "--is-inside-work-tree")
    if inside.returncode != 0:
        raise RuntimeError("current directory is not a Git working tree")
    branch = _git(root, "branch", "--show-current").stdout.strip() or "detached"
    commit = _git(root, "rev-parse", "HEAD").stdout.strip()
    status = _git(root, "status", "--porcelain", "--untracked-files=normal")
    if status.returncode != 0:
        raise RuntimeError(status.stderr.strip() or "git status failed")
    tracked = _git(root, "ls-files", "-z")
    tracked_count = len([item for item in tracked.stdout.split("\0") if item])
    changed = [line for line in status.stdout.splitlines() if line]
    return {
        "branch": branch,
        "commit": commit,
        "worktree_clean": not changed,
        "changed_entries": len(changed),
        "tracked_files": tracked_count,
    }


def create_checkpoint(root: Path, label: str) -> str:
    """Create an annotated local tag only when the complete worktree is clean."""
    if not LABEL_RE.fullmatch(label):
        raise ValueError("label must contain 1-40 lowercase letters, digits or hyphens")
    state = inspect_git(root)
    if not state["worktree_clean"]:
        raise RuntimeError("checkpoint refused: clean worktree required; commit or safely set aside all tracked and untracked changes first")
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    tag = f"clife/checkpoint-{label}-{stamp}"
    if _git(root, "rev-parse", "--verify", f"refs/tags/{tag}").returncode == 0:
        raise RuntimeError(f"checkpoint tag already exists: {tag}")
    result = _git(root, "tag", "-a", tag, "-m", f"CLife recovery checkpoint: {label}", "HEAD")
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip() or "could not create checkpoint tag")
    return tag


def create_recovery_branch(root: Path, tag: str, branch: str) -> str:
    """Create a new branch from a CLife tag; never reset or overwrite an existing branch."""
    if not CHECKPOINT_RE.fullmatch(tag):
        raise ValueError("recovery requires a correctly named CLife checkpoint tag")
    if _git(root, "check-ref-format", "--branch", branch).returncode != 0:
        raise ValueError("invalid Git branch name")
    if _git(root, "rev-parse", "--verify", f"refs/tags/{tag}").returncode != 0:
        raise ValueError(f"checkpoint tag not found: {tag}")
    if _git(root, "show-ref", "--verify", "--quiet", f"refs/heads/{branch}").returncode == 0:
        raise RuntimeError(f"recovery branch already exists: {branch}")
    result = _git(root, "branch", branch, tag)
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip() or "could not create recovery branch")
    return branch


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--strict", action="store_true", help="fail when the CLife contract or evidence is incomplete")
    parser.add_argument("--audit", action="store_true", help="report repository metadata without changing files")
    parser.add_argument("--checkpoint", metavar="LABEL", help="create a local annotated tag from a clean worktree")
    parser.add_argument("--resurrect", nargs=2, metavar=("CHECKPOINT_TAG", "NEW_BRANCH"),
                        help="create a new recovery branch from an existing CLife checkpoint")
    args = parser.parse_args()
    if not (args.audit or args.checkpoint or args.resurrect):
        parser.error("choose --audit, --checkpoint LABEL, or --resurrect TAG NEW_BRANCH")

    if args.strict:
        try:
            data = json.loads(POLICY.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            print(f"ERROR: cannot read {POLICY.relative_to(ROOT)}: {exc}", file=sys.stderr)
            return 1
        errors = validate_policy(data, ROOT)
        if errors:
            for error in errors:
                print(f"ERROR: {error}", file=sys.stderr)
            return 1

    try:
        if args.audit:
            data = json.loads(POLICY.read_text(encoding="utf-8"))
            errors = validate_policy(data, ROOT)
            report = {"schema": SCHEMA_ID, "policy_valid": not errors, "git": inspect_git(ROOT), "errors": errors}
            print(json.dumps(report, indent=2, ensure_ascii=False))
            if args.strict and errors:
                return 1
        if args.checkpoint:
            print(create_checkpoint(ROOT, args.checkpoint))
        if args.resurrect:
            print(create_recovery_branch(ROOT, args.resurrect[0], args.resurrect[1]))
    except (OSError, RuntimeError, ValueError, json.JSONDecodeError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
