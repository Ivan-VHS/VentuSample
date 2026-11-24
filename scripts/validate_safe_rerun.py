#!/usr/bin/env python3
"""Authorize one manual rerun of an allow-listed, read-only CI workflow."""
from __future__ import annotations
import argparse, json, os, re, sys
from pathlib import Path
from typing import Any
SHA_RE = re.compile(r"^[0-9a-f]{40}$")
REPOSITORY_RE = re.compile(r"^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$")

def load_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as stream: return json.load(stream)

def validate_policy(policy: Any) -> list[str]:
    if not isinstance(policy, dict): return ["policy must be a JSON object"]
    errors: list[str] = []
    if policy.get("schema") != "anatidae.safe-ci-rerun.v1": errors.append("unsupported policy schema")
    repo = policy.get("repository")
    if not isinstance(repo, str) or not REPOSITORY_RE.fullmatch(repo): errors.append("repository must be owner/name")
    if policy.get("max_attempts") != 2: errors.append("max_attempts must be exactly 2 (one retry maximum)")
    if policy.get("same_repository_only") is not True: errors.append("same_repository_only must remain true")
    for key, allowed in (("allowed_events", {"push","pull_request","schedule","workflow_dispatch"}),
                         ("allowed_conclusions", {"failure","timed_out"})):
        value = policy.get(key)
        if not isinstance(value, list) or not value or len(set(value)) != len(value): errors.append(f"{key} must be a non-empty unique array")
        elif any(item not in allowed for item in value): errors.append(f"{key} contains an unsupported value")
    names = policy.get("allowed_workflow_names")
    if not isinstance(names, list) or not names or any(not isinstance(x,str) or not x.strip() for x in names) or len(set(names)) != len(names):
        errors.append("allowed_workflow_names must contain unique non-empty workflow names")
    return errors

def validate_run(run: Any, policy: dict[str, Any]) -> list[str]:
    if not isinstance(run, dict): return ["workflow run must be a JSON object"]
    errors: list[str] = []
    expected = policy["repository"]
    repo = run.get("repository")
    head_repo = run.get("head_repository")
    if (repo.get("full_name") if isinstance(repo,dict) else None) != expected:
        errors.append("run does not belong to the configured repository")
    if (head_repo.get("full_name") if isinstance(head_repo,dict) else None) != expected:
        errors.append("fork and cross-repository pull-request runs cannot be retried")
    if run.get("name") not in policy["allowed_workflow_names"]: errors.append("workflow is not on the safe-rerun allowlist")
    if run.get("event") not in policy["allowed_events"]: errors.append("workflow event is not permitted")
    if run.get("status") != "completed": errors.append("only completed workflow runs may be retried")
    if run.get("conclusion") not in policy["allowed_conclusions"]: errors.append("only failure or timed_out conclusions may be retried")
    attempt = run.get("run_attempt")
    if isinstance(attempt,bool) or not isinstance(attempt,int) or attempt < 1: errors.append("run_attempt must be a positive integer")
    elif attempt >= policy["max_attempts"]: errors.append("retry ceiling reached; only one retry is permitted")
    run_id = run.get("id")
    if isinstance(run_id,bool) or not isinstance(run_id,int) or run_id <= 0: errors.append("run id must be a positive integer")
    sha = run.get("head_sha")
    if not isinstance(sha,str) or not SHA_RE.fullmatch(sha): errors.append("head_sha must be a 40-character hexadecimal commit id")
    if not isinstance(run.get("head_branch"),str) or not run["head_branch"].strip(): errors.append("head_branch is required for an auditable rerun")
    return errors

def write_summary(run: dict[str, Any], summary_path: str | None) -> None:
    target = summary_path or os.environ.get("GITHUB_STEP_SUMMARY")
    if not target: return
    lines = ["## Safe CI recovery","",f"- Repository: `{run['repository']['full_name']}`",
             f"- Workflow: **{run['name']}**",f"- Run: [{run['id']}]({run.get('html_url','')})",
             f"- Branch: `{run['head_branch']}`",f"- Commit: `{run['head_sha']}`",
             f"- Attempt: {run['run_attempt']} of 2","- Decision: **one failed-job rerun authorized by manual dispatch**","",
             "Only failed jobs are rerun. Fork runs, unlisted workflows, non-failure conclusions,","and any run that already consumed its retry are rejected.",""]
    with Path(target).open("a",encoding="utf-8") as stream: stream.write("\n".join(lines))

def main() -> int:
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run_json",type=Path); parser.add_argument("policy_json",type=Path)
    parser.add_argument("--summary")
    args=parser.parse_args()
    try: policy=load_json(args.policy_json); run=load_json(args.run_json)
    except (OSError,json.JSONDecodeError) as error:
        print(f"safe rerun: cannot read input: {error}",file=sys.stderr); return 2
    policy_errors=validate_policy(policy)
    if policy_errors:
        for error in policy_errors: print(f"safe rerun policy: {error}",file=sys.stderr)
        return 2
    errors=validate_run(run,policy)
    if errors:
        for error in errors: print(f"safe rerun rejected: {error}",file=sys.stderr)
        return 1
    write_summary(run,args.summary)
    print(f"safe rerun approved for {policy['repository']} run {run['id']} (attempt {run['run_attempt']}/2)")
    return 0

if __name__=="__main__": raise SystemExit(main())
