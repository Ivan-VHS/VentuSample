#!/usr/bin/env python3
"""Wrap tool-specific JSON output in the shared Ivan-VHS report envelope v1.

The wrapper is local and offline. It preserves the tool-specific result payload
and never writes to the input file or records the input path in the report.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import sys
from typing import Any

MAX_RESULT_BYTES = 16 * 1024 * 1024
PROJECT_LIMITATIONS = {
    "pytyliki": (
        "hardware inspection is read-only",
        "performance claims require measurements on the target host",
    ),
    "reaper": (
        "the runtime is not an operating-system sandbox",
    ),
    "ventusample": (
        "processing is offline",
        "the tool does not determine rights to source material",
    ),
    "oplague": (
        "analysis is read-only",
        "no driver is installed or modified",
        "heuristic findings are not malware or compatibility verdicts",
    ),
}


class EnvelopeError(ValueError):
    """Raised when report metadata or its result is invalid."""


def _non_empty_text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise EnvelopeError(f"{label} must be a non-empty string")
    return value.strip()


def _validate_hex(value: str | None, label: str, minimum: int, maximum: int) -> str | None:
    if value is None or value == "":
        return None
    if not isinstance(value, str) or not minimum <= len(value) <= maximum:
        raise EnvelopeError(f"{label} must contain {minimum}–{maximum} hexadecimal characters")
    if any(char not in "0123456789abcdefABCDEF" for char in value):
        raise EnvelopeError(f"{label} must be hexadecimal")
    return value.lower()


def sha256_file(path: str | Path) -> str:
    """Hash a file incrementally without loading it into memory."""
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def build_envelope(
    *,
    tool: str,
    version: str,
    commit: str | None,
    result: Any,
    limitations: list[str] | tuple[str, ...] | None = None,
    input_kind: str = "none",
    input_sha256: str | None = None,
    created: str | None = None,
    os_name: str | None = None,
    architecture: str | None = None,
) -> dict[str, Any]:
    """Build a validated, JSON-compatible report envelope."""
    tool_name = _non_empty_text(tool, "tool name").lower()
    if any(char not in "abcdefghijklmnopqrstuvwxyz0123456789-_" for char in tool_name):
        raise EnvelopeError("tool name may contain only lowercase letters, digits, '-' and '_'")

    version_text = _non_empty_text(version, "tool version")
    commit_hash = _validate_hex(commit, "commit", 7, 64)
    input_hash = _validate_hex(input_sha256, "input SHA-256", 64, 64)
    kind = _non_empty_text(input_kind, "input kind").lower()

    if created is None:
        created = datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")
    else:
        _non_empty_text(created, "created timestamp")
        if not created.endswith("Z"):
            raise EnvelopeError("created timestamp must be an ISO-8601 UTC timestamp ending in 'Z'")

    supplied_limitations = list(limitations or [])
    defaults = list(PROJECT_LIMITATIONS.get(tool_name, ()))
    combined: list[str] = []
    for item in defaults + supplied_limitations:
        entry = _non_empty_text(item, "limitation")
        if entry not in combined:
            combined.append(entry)

    return {
        "envelope": "ivan-vhs.report.v1",
        "tool": {
            "name": tool_name,
            "version": version_text,
            "commit": commit_hash,
        },
        "created": created,
        "platform": {
            "os": os_name or platform.system().lower() or "unknown",
            "arch": architecture or platform.machine().lower() or "unknown",
        },
        "input": {
            "kind": kind,
            "sha256": input_hash,
        },
        "result": result,
        "limitations": combined,
    }


def _read_result(stream: Any) -> Any:
    raw = stream.read(MAX_RESULT_BYTES + 1)
    if len(raw.encode("utf-8")) > MAX_RESULT_BYTES:
        raise EnvelopeError(f"JSON result exceeds the {MAX_RESULT_BYTES}-byte limit")
    try:
        return json.loads(raw)
    except json.JSONDecodeError as error:
        raise EnvelopeError(f"standard input is not valid JSON: {error}") from error


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Wrap a JSON result in the shared ivan-vhs.report.v1 envelope."
    )
    parser.add_argument("--tool", required=True, help="tool identifier: pytyliki, reaper, ventusample or oplague")
    parser.add_argument("--version", default=os.environ.get("IVAN_VHS_TOOL_VERSION", "unknown"))
    parser.add_argument("--commit", default=os.environ.get("GITHUB_SHA") or os.environ.get("IVAN_VHS_COMMIT"))
    parser.add_argument("--input-kind", default="none", help="kind of the source input; defaults to none")
    parser.add_argument("--input-file", help="optional file whose SHA-256 is recorded; the path is not included")
    parser.add_argument("--limitation", action="append", default=[], help="additional limitation; may be repeated")
    args = parser.parse_args(argv)

    try:
        result = _read_result(sys.stdin)
        input_kind = "file" if args.input_file else args.input_kind
        input_hash = sha256_file(args.input_file) if args.input_file else None
        report = build_envelope(
            tool=args.tool,
            version=args.version,
            commit=args.commit,
            result=result,
            limitations=args.limitation,
            input_kind=input_kind,
            input_sha256=input_hash,
        )
    except (EnvelopeError, OSError) as error:
        print(f"report-envelope: {error}", file=sys.stderr)
        return 2

    json.dump(report, sys.stdout, ensure_ascii=False, indent=2, sort_keys=True)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
