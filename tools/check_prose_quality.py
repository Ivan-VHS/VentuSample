#!/usr/bin/env python3
"""Conservative, dependency-free checks for obvious Markdown prose defects.

This is a review aid, not a grammar checker. It never rewrites repository files.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

SKIP_DIRS = {".git", ".venv", "venv", "build", "dist", "node_modules", "vendor"}
MARKDOWN_SUFFIXES = {".md", ".markdown"}

FENCE_RE = re.compile(r"^\s*(`{3,}|~{3,})")
INLINE_CODE_RE = re.compile(r"(`+)(.*?)\1")
LINK_RE = re.compile(r"!?(\[[^\]]*\])\([^)]*\)")
URL_RE = re.compile(r"https?://\S+")
HTML_TAG_RE = re.compile(r"<[^>]+>")
DUPLICATE_WORD_RE = re.compile(r"\b([\w’'-]{2,})\s+\1\b", re.IGNORECASE)
SPACE_BEFORE_PUNCTUATION_RE = re.compile(r"[ \t]+([,.;:!?])")
REPEATED_PUNCTUATION_RE = re.compile(r"([,;!?])\1+")


def visible_markdown_lines(source: str) -> list[str]:
    """Return prose lines while blanking fenced/inline code and link targets."""
    result: list[str] = []
    fence_char: str | None = None
    fence_size = 0

    for line in source.splitlines():
        fence = FENCE_RE.match(line)
        if fence:
            marker = fence.group(1)
            if fence_char is None:
                fence_char, fence_size = marker[0], len(marker)
            elif marker[0] == fence_char and len(marker) >= fence_size:
                fence_char, fence_size = None, 0
            result.append("")
            continue

        if fence_char is not None:
            result.append("")
            continue

        text = INLINE_CODE_RE.sub(" ", line)
        text = LINK_RE.sub(r"\1", text)
        text = URL_RE.sub(" ", text)
        text = HTML_TAG_RE.sub(" ", text)
        result.append(text)

    return result


def inspect_file(path: Path) -> list[tuple[int, str, str]]:
    try:
        source = path.read_text(encoding="utf-8")
    except (OSError, UnicodeError) as error:
        return [(1, "error", f"cannot read UTF-8 Markdown: {error}")]

    findings: list[tuple[int, str, str]] = []
    for line_number, line in enumerate(visible_markdown_lines(source), start=1):
        if not line.strip():
            continue

        for match in SPACE_BEFORE_PUNCTUATION_RE.finditer(line):
            findings.append((
                line_number,
                "error",
                f"remove whitespace before '{match.group(1)}'",
            ))

        for match in DUPLICATE_WORD_RE.finditer(line):
            word = match.group(1)
            findings.append((
                line_number,
                "review",
                f"check whether '{word} {word}' is an accidental repetition",
            ))

        for match in REPEATED_PUNCTUATION_RE.finditer(line):
            findings.append((
                line_number,
                "review",
                f"check repeated punctuation '{match.group(0)}'",
            ))

    return findings


def markdown_files(root: Path) -> list[Path]:
    files: list[Path] = []
    for path in root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in MARKDOWN_SUFFIXES:
            continue
        if any(part in SKIP_DIRS for part in path.relative_to(root).parts):
            continue
        files.append(path)
    return sorted(files)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("paths", nargs="*", type=Path, help="Markdown files or directories; default: repository root")
    parser.add_argument("--strict", action="store_true", help="return failure for review findings as well as errors")
    args = parser.parse_args()

    root = Path.cwd()
    targets = args.paths or [root]
    files: list[Path] = []
    for target in targets:
        target = target if target.is_absolute() else root / target
        if target.is_dir():
            files.extend(markdown_files(target))
        elif target.is_file() and target.suffix.lower() in MARKDOWN_SUFFIXES:
            files.append(target)
        else:
            print(f"warning: skipped missing or non-Markdown path: {target}", file=sys.stderr)

    files = sorted(set(path.resolve() for path in files))
    errors = 0
    reviews = 0

    for path in files:
        try:
            display_path = path.relative_to(root.resolve())
        except ValueError:
            display_path = path
        for line_number, severity, message in inspect_file(path):
            print(f"{display_path}:{line_number}: {severity}: {message}")
            if severity == "error":
                errors += 1
            else:
                reviews += 1

    print(
        f"Checked {len(files)} Markdown file(s): "
        f"{errors} error(s), {reviews} item(s) for editorial review."
    )
    return 1 if errors or (args.strict and reviews) else 0


if __name__ == "__main__":
    raise SystemExit(main())
