#!/usr/bin/env python3
"""Deterministic, redaction-safe static security audit for tracked source/config files."""
from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path
from typing import Iterable

SCHEMA = "repository-security-audit.v1"
TEXT_SUFFIXES = {
    ".py", ".pyi", ".c", ".h", ".cc", ".cpp", ".cxx", ".hpp", ".hh",
    ".js", ".jsx", ".ts", ".tsx", ".mjs", ".sh", ".ps1", ".rpr",
    ".json", ".jsonc", ".yaml", ".yml", ".toml", ".ini", ".cfg",
    ".conf", ".properties", ".env", ".txt", ".md", ".cmake",
}
SKIP_PARTS = {
    ".git", ".venv", "venv", "__pycache__", "node_modules", "build",
    "build-sanitized", "dist", "out", "target", "vendor", "third_party",
    ".idea", ".pytest_cache", ".mypy_cache",
}
MAX_FILE_BYTES = 3 * 1024 * 1024

# These patterns detect high-confidence credential material. Finding reports
# intentionally contain no matched text, tokens, snippets or secret values.
SECRET_PATTERNS = (
    ("private-key-material", re.compile(r"-----BEGIN (?:RSA |EC |OPENSSH |DSA )?PRIVATE KEY-----")),
    ("github-token", re.compile(r"\bgh[pousr]_[A-Za-z0-9_]{30,}\b")),
    ("aws-access-key", re.compile(r"\bAKIA[0-9A-Z]{16}\b")),
    ("slack-token", re.compile(r"\bxox[baprs]-[A-Za-z0-9-]{20,}\b")),
    ("stripe-live-secret", re.compile(r"\bsk_live_[A-Za-z0-9]{20,}\b")),
    ("generic-hardcoded-credential", re.compile(
        r"""(?i)\b(?:api[_-]?key|client[_-]?secret|access[_-]?token|password|passwd)\b\s*[:=]\s*["']([^"'\\s]{16,})["']"""
    )),
)
PLACEHOLDERS = (
    "example", "placeholder", "changeme", "change-me", "dummy", "redacted",
    "replace-me", "your_", "your-", "test-token", "fake-token", "not-a-secret",
)
PRIVACY_PATTERNS = (
    ("sensitive-value-logging", re.compile(
        r"(?i)\b(?:print|logger\.(?:debug|info|warning|error|exception)|logging\.(?:debug|info|warning|error))"
        r"\s*\([^)\n]{0,240}\b(?:password|passwd|secret|access_token|api_key|serial_number|device_id|email|"
        r"source_text|debug_locals|audio_samples)\b"
    )),
    ("developer-local-path", re.compile(r"(?i)(?:[A-Z]:\\Users\\[A-Za-z0-9._-]+\\|/home/[a-z][a-z0-9_-]*/|/Users/[A-Za-z0-9._-]+/)")),
)
RISK_PATTERNS = (
    ("python-dynamic-eval", re.compile(r"(?<![A-Za-z0-9_.])(?:eval|exec)\s*\("), "medium"),
    ("python-shell-true", re.compile(r"(?i)\bshell\s*=\s*True\b"), "high"),
    ("python-unsafe-pickle", re.compile(r"\bpickle\.(?:load|loads)\s*\("), "high"),
    ("python-unsafe-yaml-load", re.compile(r"\byaml\.load\s*\("), "medium"),
    ("c-unsafe-string-api", re.compile(r"\b(?:gets|strcpy|strcat|sprintf)\s*\("), "medium"),
    ("native-shell-execution", re.compile(r"\b(?:system|_popen|popen)\s*\("), "medium"),
)
def _is_placeholder(value: str) -> bool:
    lowered = value.strip().casefold()
    return any(marker in lowered for marker in PLACEHOLDERS) or lowered in {
        "none", "null", "false", "true", "0", "123456", "password", "token",
    }

def scan_text(text: str, relative_path: str) -> list[dict[str, object]]:
    findings: list[dict[str, object]] = []
    for line_number, line in enumerate(text.splitlines(), 1):
        for rule, pattern in SECRET_PATTERNS:
            match = pattern.search(line)
            if not match:
                continue
            value = match.group(1) if match.lastindex else match.group(0)
            if _is_placeholder(value):
                continue
            findings.append({
                "rule": rule, "severity": "critical", "category": "secret",
                "path": relative_path, "line": line_number,
            })
        for rule, pattern in PRIVACY_PATTERNS:
            if pattern.search(line):
                findings.append({
                    "rule": rule, "severity": "medium", "category": "privacy",
                    "path": relative_path, "line": line_number,
                })
        for rule, pattern, severity in RISK_PATTERNS:
            if pattern.search(line):
                findings.append({
                    "rule": rule, "severity": severity, "category": "source-risk",
                    "path": relative_path, "line": line_number,
                })
    return findings

def tracked_paths(root: Path) -> Iterable[Path]:
    try:
        output = subprocess.run(
            ["git", "ls-files", "-z"], cwd=root, check=True,
            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
        ).stdout
        paths = [root / item.decode("utf-8", "surrogateescape") for item in output.split(b"\0") if item]
    except (OSError, subprocess.CalledProcessError):
        paths = [path for path in root.rglob("*") if path.is_file()]
    for path in paths:
        try:
            relative = path.relative_to(root)
        except ValueError:
            continue
        if any(part in SKIP_PARTS for part in relative.parts):
            continue
        if path.name == "CMakeLists.txt" or path.suffix.casefold() in TEXT_SUFFIXES:
            yield path

def audit(root: Path) -> dict[str, object]:
    findings: list[dict[str, object]] = []
    scanned = 0
    oversized = 0
    for path in tracked_paths(root):
        try:
            if path.stat().st_size > MAX_FILE_BYTES:
                oversized += 1
                continue
            raw = path.read_bytes()
            text = raw.decode("utf-8")
        except (OSError, UnicodeDecodeError):
            continue
        scanned += 1
        findings.extend(scan_text(text, path.relative_to(root).as_posix()))
    findings.sort(key=lambda item: (str(item["severity"]), str(item["path"]), int(item["line"]), str(item["rule"])))
    return {
        "schema": SCHEMA,
        "repository": root.name,
        "scanned_files": scanned,
        "oversized_files_skipped": oversized,
        "finding_count": len(findings),
        "counts": {
            "critical_secrets": sum(1 for item in findings if item["category"] == "secret"),
            "privacy": sum(1 for item in findings if item["category"] == "privacy"),
            "source_risks": sum(1 for item in findings if item["category"] == "source-risk"),
        },
        "findings": findings,
        "redaction": "Finding records contain rule, severity, path and line only; matched values and source snippets are never emitted.",
    }

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd())
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    root = args.root.resolve()
    report = audit(root)
    rendered = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
    return 1 if report["counts"]["critical_secrets"] else 0

if __name__ == "__main__":
    raise SystemExit(main())
