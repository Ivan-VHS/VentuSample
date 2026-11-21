#!/usr/bin/env python3
"""Dependency-free validator and changed-file scanner for Veelsevul AppGuard."""
from __future__ import annotations
import argparse
import json
import re
import subprocess
import sys
import unicodedata
from datetime import date
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
POLICY = ROOT / ".veelsevul" / "policy.v1.json"
EXCEPTIONS = ROOT / ".veelsevul" / "exceptions.json"
EXPECTED_RULES = {"A": 15, "M": 15, "Я": 15, "K": 15, "O": 6}
EXPECTED_RANGES = [(1, 111), (112, 222), (223, 333), (334, 444), (445, 555), (556, 666)]
VERDICTS = {"allow", "quarantine", "deny", "degrade"}
SECRET_PATTERNS = (
    ("private-key", re.compile(r"-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----")),
    ("github-token", re.compile(r"\b(?:ghp|gho|ghu|ghs|ghr)_[A-Za-z0-9]{30,}\b")),
    ("github-fine-grained-token", re.compile(r"\bgithub_pat_[A-Za-z0-9_]{50,}\b")),
    ("aws-access-key-id", re.compile(r"\bAKIA[0-9A-Z]{16}\b")),
    ("slack-token", re.compile(r"\bxox[baprs]-[A-Za-z0-9-]{20,}\b")),
)
SKIP_SCAN = {".veelsevul/guard.py"}
MAX_SCAN_BYTES = 2 * 1024 * 1024

def fail(message: str) -> None:
    print("VEEELZEVUL FAIL: " + message, file=sys.stderr)
    raise SystemExit(1)

def load_json(path: Path):
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        fail(f"cannot read valid UTF-8 JSON at {path.relative_to(ROOT)}: {exc}")

def validate() -> None:
    policy = load_json(POLICY)
    exceptions = load_json(EXCEPTIONS)
    if policy.get("policy_id") != "veelsevul.appguard" or policy.get("schema_version") != "veelsevul.policy.v1":
        fail("unexpected policy identity or schema version")
    architecture = policy.get("architecture", {})
    if architecture.get("repository_rule_count") != 66 or architecture.get("appguard_level_count") != 666:
        fail("policy must declare 66 repository rules and 666 AppGuard levels")
    if architecture.get("default_decision") != "deny" or set(architecture.get("allowed_verdicts", [])) != VERDICTS:
        fail("default-deny or verdict taxonomy is invalid")

    rules = policy.get("repository_rules", [])
    expected_ids = {f"{group}-{index:02d}" for group, count in EXPECTED_RULES.items() for index in range(1, count + 1)}
    rule_ids = [item.get("id") for item in rules]
    if len(rules) != 66 or set(rule_ids) != expected_ids or len(set(rule_ids)) != len(rule_ids):
        fail("repository rule registry must contain exactly the 66 canonical IDs")
    rule_map = {item["id"]: item for item in rules}
    for item in rules:
        if not item.get("title") or not item.get("verification") or not item.get("appguard_ids"):
            fail(f"incomplete or unmapped repository rule: {item.get('id')}")

    levels = policy.get("appguard_levels", [])
    if len(levels) != 666:
        fail(f"expected 666 AppGuard levels; found {len(levels)}")
    expected_level_ids = [f"L{index:03d}" for index in range(1, 667)]
    level_ids = [item.get("id") for item in levels]
    if level_ids != expected_level_ids:
        fail("AppGuard IDs must be contiguous and ordered L001..L666")
    by_id = {item["id"]: item for item in levels}
    for item in levels:
        if item.get("on_success") != "allow" or item.get("on_failure") not in VERDICTS - {"allow"}:
            fail(f"invalid verdict policy for {item['id']}")
        links = item.get("linked_rules", [])
        if not links or any(link not in rule_map for link in links):
            fail(f"invalid repository-rule mapping for {item['id']}")
        if not item.get("control") or not item.get("description"):
            fail(f"missing control description for {item['id']}")
        for link in links:
            if item["id"] not in rule_map[link].get("appguard_ids", []):
                fail(f"mapping is not bidirectional: {item['id']} <-> {link}")

    for echelon, (first, last) in enumerate(EXPECTED_RANGES, 1):
        subset = [item for item in levels if item.get("echelon") == echelon]
        if len(subset) != 111:
            fail(f"echelon {echelon} must contain 111 levels")
        actual = [int(item["id"][1:]) for item in subset]
        if min(actual) != first or max(actual) != last:
            fail(f"echelon {echelon} has an incorrect ID range")
    for rule in rules:
        for level_id in rule.get("appguard_ids", []):
            if level_id not in by_id or rule["id"] not in by_id[level_id].get("linked_rules", []):
                fail(f"invalid reverse mapping for {rule['id']} -> {level_id}")

    if exceptions.get("policy") != "veelsevul.exceptions.v1" or not isinstance(exceptions.get("exceptions"), list):
        fail("invalid exception registry")
    required = {"id", "owner", "reason", "approver", "scope", "rules", "created_on", "expires_on"}
    today = date.today()
    for item in exceptions["exceptions"]:
        if not isinstance(item, dict) or not required.issubset(item):
            fail("every exception needs id, owner, reason, approver, scope, rules, created_on and expires_on")
        try:
            created, expires = date.fromisoformat(item["created_on"]), date.fromisoformat(item["expires_on"])
        except (TypeError, ValueError):
            fail(f"exception has invalid ISO date: {item.get('id')}")
        if not all(str(item[key]).strip() for key in ("id", "owner", "reason", "approver", "scope")):
            fail(f"exception metadata is incomplete: {item.get('id')}")
        if expires <= today or expires <= created or (expires - created).days > 30:
            fail(f"exception is expired or exceeds its 30-day lifetime: {item.get('id')}")
        if not isinstance(item["rules"], list) or not item["rules"] or any(rule not in rule_map for rule in item["rules"]):
            fail(f"exception has an invalid rule scope: {item.get('id')}")

    for relative in (".veelsevul/runtime/appguard.h", ".veelsevul/runtime/appguard.c", ".veelsevul/runtime/test_appguard.c"):
        if not (ROOT / relative).is_file():
            fail(f"missing runtime implementation or test: {relative}")
    print("VEEELZEVUL PASS: 66 repository rules, 666 AppGuard levels, six 111-level echelons, bidirectional mappings and exception expiry")

def changed_paths(base: str | None, scan_head: bool) -> list[Path]:
    if scan_head:
        command = ["git", "diff-tree", "--no-commit-id", "--name-only", "--diff-filter=ACMR", "-r", "-z", "HEAD"]
    elif base:
        command = ["git", "diff", "--name-only", "--diff-filter=ACMR", "-z", f"{base}...HEAD"]
    else:
        fail("secret scan requires --scan-base <sha> or --scan-head")
    try:
        result = subprocess.run(command, cwd=ROOT, check=True, capture_output=True)
    except (OSError, subprocess.CalledProcessError) as exc:
        fail(f"cannot enumerate changed files: {exc}")
    paths = []
    for raw in result.stdout.split(b"\0"):
        if not raw:
            continue
        relative = Path(raw.decode("utf-8", errors="surrogateescape"))
        if relative.as_posix() in SKIP_SCAN:
            continue
        candidate = (ROOT / relative).resolve()
        if not candidate.is_relative_to(ROOT) or not candidate.is_file():
            continue
        paths.append(candidate)
    return paths

def scan(base: str | None, scan_head: bool) -> None:
    findings = []
    for path in changed_paths(base, scan_head):
        try:
            if path.stat().st_size > MAX_SCAN_BYTES:
                continue
            raw = path.read_bytes()
            if b"\0" in raw:
                continue
            content = raw.decode("utf-8")
        except UnicodeDecodeError:
            findings.append((path.relative_to(ROOT).as_posix(), 0, "invalid-utf8"))
            continue
        except OSError:
            continue
        for line_no, line in enumerate(content.splitlines(), 1):
            if any(unicodedata.category(char) == "Cf" for char in line):
                findings.append((path.relative_to(ROOT).as_posix(), line_no, "hidden-format-control"))
            for label, pattern in SECRET_PATTERNS:
                if pattern.search(line):
                    findings.append((path.relative_to(ROOT).as_posix(), line_no, label))
    if findings:
        for relative, line_no, label in findings:
            print(f"VEEELZEVUL FINDING: {relative}:{line_no}: {label}", file=sys.stderr)
        fail(f"{len(findings)} high-confidence finding(s); secret values are not printed")
    print("VEEELZEVUL PASS: changed-file credential and hidden-control scan")

def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scan-base", help="base commit SHA; scan changes from base...HEAD")
    parser.add_argument("--scan-head", action="store_true", help="scan files changed by HEAD")
    args = parser.parse_args()
    validate()
    if args.scan_base or args.scan_head:
        scan(args.scan_base, args.scan_head)

if __name__ == "__main__":
    main()
