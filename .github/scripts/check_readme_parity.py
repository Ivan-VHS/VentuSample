#!/usr/bin/env python3
"""Check structural and numeric parity across the four localized README files."""

from __future__ import annotations

import re
import sys
from dataclasses import dataclass
from pathlib import Path

README_FILES = ("README.md", "README.uk.md", "README.ru.md", "README.zh-CN.md")
FENCE_RE = re.compile(r"^ {0,3}(\x60{3,}|~{3,})(.*)$")
HEADING_RE = re.compile(r"^ {0,3}(#{1,6})[ \t]+")
PIPE_ROW_RE = re.compile(r"^ *\|.*\| *$")
NUMBER_RE = re.compile(r"[0-9]+(?:[.,][0-9]+)?")
SHELL_LANGUAGES = {"sh", "bash", "shell", "powershell", "pwsh"}
TRANSLATION_NEUTRAL_LANGUAGES = {"text", "plain"}


@dataclass(frozen=True)
class Snapshot:
    heading_levels: tuple[int, ...]
    table_count: int
    table_numbers: tuple[tuple[str, ...], ...]
    fence_languages: tuple[str, ...]
    fence_bodies: tuple[str, ...]
    neutral_line_counts: tuple[int, ...]


def _is_table_separator(line: str) -> bool:
    cells = line.strip().strip("|").split("|")
    return bool(cells) and all(re.fullmatch(r" *:?-{3,}:? *", cell) for cell in cells)


def _shell_body(body: str) -> str:
    useful_lines = []
    for line in body.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        useful_lines.append(line.rstrip())
    return "\n".join(useful_lines)


def analyze_markdown(markdown: str) -> Snapshot:
    headings: list[int] = []
    numeric_rows: list[tuple[str, ...]] = []
    fence_languages: list[str] = []
    fence_bodies: list[str] = []
    neutral_line_counts: list[int] = []
    table_count = 0
    in_table = False
    fence_char = ""
    fence_length = 0
    fence_info = ""
    fence_lines: list[str] = []

    for line in markdown.splitlines():
        fence_match = FENCE_RE.match(line)
        if fence_char:
            if fence_match:
                marker = fence_match.group(1)
                if marker[0] == fence_char and len(marker) >= fence_length and not fence_match.group(2).strip():
                    body = "\n".join(fence_lines).strip("\n")
                    fence_languages.append(fence_info)
                    if fence_info in TRANSLATION_NEUTRAL_LANGUAGES:
                        neutral_line_counts.append(len(body.splitlines()) if body else 0)
                        fence_bodies.append("")
                    elif fence_info in SHELL_LANGUAGES:
                        fence_bodies.append(_shell_body(body))
                    else:
                        fence_bodies.append(body)
                    fence_char = ""
                    fence_length = 0
                    fence_info = ""
                    fence_lines = []
                    continue
            fence_lines.append(line)
            continue

        if fence_match:
            marker = fence_match.group(1)
            info = fence_match.group(2).strip().split(maxsplit=1)
            fence_char = marker[0]
            fence_length = len(marker)
            fence_info = info[0].lower() if info else ""
            fence_lines = []
            in_table = False
            continue

        heading_match = HEADING_RE.match(line)
        if heading_match:
            headings.append(len(heading_match.group(1)))

        if PIPE_ROW_RE.match(line):
            if not in_table:
                table_count += 1
                in_table = True
            if not _is_table_separator(line):
                numbers = tuple(NUMBER_RE.findall(line))
                if numbers:
                    numeric_rows.append(numbers)
        else:
            in_table = False

    if fence_char:
        raise ValueError("unclosed Markdown code fence")

    return Snapshot(
        heading_levels=tuple(headings),
        table_count=table_count,
        table_numbers=tuple(numeric_rows),
        fence_languages=tuple(fence_languages),
        fence_bodies=tuple(fence_bodies),
        neutral_line_counts=tuple(neutral_line_counts),
    )


def compare_documents(documents: dict[str, str]) -> list[str]:
    errors: list[str] = []
    missing = [name for name in README_FILES if name not in documents]
    if missing:
        return ["missing README input: " + ", ".join(missing)]

    try:
        reference = analyze_markdown(documents[README_FILES[0]])
    except ValueError as error:
        return [f"{README_FILES[0]}: {error}"]

    for filename in README_FILES[1:]:
        try:
            candidate = analyze_markdown(documents[filename])
        except ValueError as error:
            errors.append(f"{filename}: {error}")
            continue

        if candidate.heading_levels != reference.heading_levels:
            errors.append(f"{filename}: heading levels or order differ from {README_FILES[0]}")
        if candidate.table_count != reference.table_count:
            errors.append(f"{filename}: Markdown table count differs from {README_FILES[0]}")
        if candidate.table_numbers != reference.table_numbers:
            errors.append(f"{filename}: numeric values in Markdown tables differ from {README_FILES[0]}")
        if candidate.fence_languages != reference.fence_languages:
            errors.append(f"{filename}: code-fence count or language markers differ from {README_FILES[0]}")
        if candidate.neutral_line_counts != reference.neutral_line_counts:
            errors.append(f"{filename}: translated text blocks have a different line structure")
        if candidate.fence_bodies != reference.fence_bodies:
            errors.append(f"{filename}: executable examples or non-translatable code blocks differ from {README_FILES[0]}")

    return errors


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    documents: dict[str, str] = {}
    for filename in README_FILES:
        path = root / filename
        try:
            documents[filename] = path.read_text(encoding="utf-8")
        except OSError as error:
            print(f"README parity: cannot read {filename}: {error}", file=sys.stderr)
            return 2

    errors = compare_documents(documents)
    if errors:
        for error in errors:
            print(f"README parity: {error}", file=sys.stderr)
        return 1

    print("README parity: four locales have matching structure, executable examples, and table numbers.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
