#!/usr/bin/env python3
"""Check translated README structure and all repository-local links."""
from __future__ import annotations

import re
import sys
from pathlib import Path
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[2]
READMES = ("README.md", "README.ru.md", "README.uk.md", "README.zh-CN.md")
MARKDOWN_LINK = re.compile(r"!?\[[^\]]*\]\(\s*(?:<([^>]+)>|([^\s)]+))")
HTML_LINK = re.compile(r"""\b(?:href|src)\s*=\s*["']([^"']+)["']""", re.IGNORECASE)
FENCE = re.compile(r"^\s*(?P<marker>\x60{3,}|~{3,})(?P<info>.*)$")
TABLE_ROW = re.compile(r"^\s*\|.*\|\s*$")


def inspect_structure(markdown: str):
    headings = []
    code_languages = []
    table_shapes = []
    visible_lines = []
    in_fence = False
    fence_char = ""
    table_rows = 0

    for line in markdown.splitlines():
        fence = FENCE.match(line)
        if fence:
            if not in_fence:
                if table_rows:
                    table_shapes.append(table_rows)
                    table_rows = 0
                in_fence = True
                fence_char = fence.group("marker")[0]
                info = fence.group("info").strip().split()
                code_languages.append(info[0].lower() if info else "")
            elif fence.group("marker")[0] == fence_char:
                in_fence = False
                fence_char = ""
            continue

        if in_fence:
            continue

        heading = re.match(r"^(#{1,6})\s+\S", line)
        if heading:
            headings.append(len(heading.group(1)))

        if TABLE_ROW.match(line):
            table_rows += 1
        elif table_rows:
            table_shapes.append(table_rows)
            table_rows = 0

        visible_lines.append(line)

    if table_rows:
        table_shapes.append(table_rows)

    return (
        "\n".join(visible_lines),
        tuple(headings),
        tuple(code_languages),
        tuple(table_shapes),
    )


def targets(markdown: str):
    visible, _, _, _ = inspect_structure(markdown)
    for match in MARKDOWN_LINK.finditer(visible):
        yield match.group(1) or match.group(2)
    yield from HTML_LINK.findall(visible)


def validate() -> list[str]:
    errors = []
    documents = {}
    paths = [ROOT / name for name in READMES]

    if not (ROOT / "CONTRIBUTING.md").is_file():
        errors.append("CONTRIBUTING.md: contribution guide is missing")

    for path in paths:
        if not path.is_file():
            errors.append(f"{path.name}: required language edition is missing")
            continue

        content = path.read_text(encoding="utf-8")
        documents[path.name] = content
        if not content.strip():
            errors.append(f"{path.name}: file is empty")
            continue
        if not re.search(r"(?m)^#\s+\S", content):
            errors.append(f"{path.name}: missing top-level Markdown heading")
        if not re.search(r"\[[^\]]+\]\(CONTRIBUTING\.md(?:#[^)]+)?\)", content):
            errors.append(f"{path.name}: must link to CONTRIBUTING.md")

        for raw in targets(content):
            target = raw.strip().strip(chr(34) + chr(39))
            if not target:
                continue
            parsed = urlsplit(target)
            if parsed.scheme or parsed.netloc or not parsed.path:
                continue
            relative = unquote(parsed.path)
            candidate = ROOT / relative.lstrip("/") if relative.startswith("/") else path.parent / relative
            resolved = candidate.resolve()
            try:
                resolved.relative_to(ROOT)
            except ValueError:
                errors.append(f"{path.name}: link escapes repository: {target}")
                continue
            if not resolved.exists():
                errors.append(f"{path.name}: broken local link: {target}")

    baseline = documents.get("README.md")
    if baseline:
        expected = inspect_structure(baseline)[1:]
        reference_links = len(list(targets(baseline)))
        labels = ("heading levels", "code fence languages", "table row groups")
        for name, content in documents.items():
            if name == "README.md":
                continue
            actual = inspect_structure(content)[1:]
            for label, wanted, found in zip(labels, expected, actual):
                if wanted != found:
                    errors.append(
                        f"{name}: translated Markdown {label} differ from README.md "
                        f"(expected {wanted}, got {found})"
                    )
            actual_links = len(list(targets(content)))
            if actual_links != reference_links:
                errors.append(
                    f"{name}: Markdown link count differs from README.md "
                    f"(expected {reference_links}, got {actual_links})"
                )

    return errors


if __name__ == "__main__":
    problems = validate()
    if problems:
        for problem in problems:
            print(f"ERROR: {problem}", file=sys.stderr)
        raise SystemExit(1)
    print("README check passed: four language editions, matching structure, contribution links and local targets.")
