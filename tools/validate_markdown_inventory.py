#!/usr/bin/env python3
"""Validate every tracked Markdown document and its local navigation targets."""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path
from urllib.parse import unquote, urlsplit

FENCE_OPEN = re.compile("^[ \\t]{0,3}(\\x60{3,}|~{3,})(.*)$")
MARKDOWN_LINK = re.compile("!?\\[[^\\]]*\\]\\(\\s*(?:<([^>]+)>|([^\\s)]+))")
REFERENCE_LINK = re.compile("(?m)^[ \\t]{0,3}\\[[^\\]]+\\]:[ \\t]*(?:<([^>]+)>|(\\S+))")
HTML_LINK = re.compile("\\\\b(?:href|src)\\\\s*=\\\\s*[\\"']([^\\"']+)[\\"']", re.IGNORECASE)
HTML_ID = re.compile("\\\\b(?:id|name)\\\\s*=\\\\s*[\\"']([^\\"']+)[\\"']", re.IGNORECASE)
HEADING = re.compile("^[ \\t]{0,3}(#{1,6})[ \\t]+(.+?)\\\\s*#*[ \\t]*$")
INLINE_CODE = re.compile("(\\\\x60+)(.*?)\\\\1")
HTML_TAG = re.compile("<[^>]*>")
MARKDOWN_LINK_LABEL = re.compile("!?\\[([^\\]]*)\\]\\([^)]*\\)")
LINE_ANCHOR = re.compile("^L\\d+(?:-L\\d+)?$", re.IGNORECASE)


def strip_code_blocks(source: str) -> str:
    """Remove fenced code blocks while preserving line numbers."""
    output: list[str] = []
    marker: str | None = None
    marker_length = 0
    for line in source.splitlines(keepends=True):
        match = FENCE_OPEN.match(line.rstrip("\\r\\n"))
        if marker is None and match:
            token = match.group(1)
            marker = token[0]
            marker_length = len(token)
            output.append("\\n" if line.endswith("\\n") else "")
            continue
        if marker is not None:
            if match and match.group(1)[0] == marker and len(match.group(1)) >= marker_length:
                marker = None
                marker_length = 0
            output.append("\\n" if line.endswith("\\n") else "")
            continue
        output.append(line)
    return INLINE_CODE.sub("", "".join(output))


def slugify_heading(value: str) -> str:
    """Approximate GitHub heading slug rules for local Markdown fragments."""
    value = HTML_TAG.sub("", value)
    value = MARKDOWN_LINK_LABEL.sub("\\\\1", value)
    value = re.sub("[*_~]", "", value)
    value = value.strip().casefold()
    value = re.sub("[^\\w\\- ]", "", value, flags=re.UNICODE)
    value = re.sub("\\\\s+", "-", value)
    value = re.sub("-{2,}", "-", value)
    return value.strip("-")


def markdown_anchors(source: str) -> set[str]:
    anchors: set[str] = set()
    seen: dict[str, int] = {}
    visible = strip_code_blocks(source)
    for line in visible.splitlines():
        match = HEADING.match(line)
        if match:
            base = slugify_heading(match.group(2))
            count = seen.get(base, 0)
            seen[base] = count + 1
            anchors.add(base if count == 0 else f"{base}-{count}")
        anchors.update(HTML_ID.findall(line))
        anchors.update(re.findall("\\\\{#([A-Za-z0-9_.:-]+)\\\\}", line))
    return anchors


def tracked_markdown_files(root: Path) -> list[Path]:
    """Use Git's tracked-file list in CI; fall back to a safe recursive scan."""
    try:
        result = subprocess.run(
            ["git", "-C", str(root), "ls-files", "-z", "--", "*.md", "*.mdx"],
            check=True,
            capture_output=True,
        )
        relative = [
            Path(item.decode("utf-8", errors="strict"))
            for item in result.stdout.split(b"\\0")
            if item
        ]
        return sorted(path for path in relative if (root / path).is_file())
    except (OSError, subprocess.CalledProcessError, UnicodeDecodeError):
        excluded = {".git", ".venv", "venv", "node_modules", "build", "dist"}
        return sorted(
            path.relative_to(root)
            for path in root.rglob("*")
            if path.is_file()
            and path.suffix.lower() in {".md", ".mdx"}
            and not any(part in excluded for part in path.relative_to(root).parts)
        )


def validate_target(root: Path, source_path: Path, raw_target: str) -> str | None:
    """Return a navigation error, or None for a valid/irrelevant target."""
    target = raw_target.strip().strip("<>").strip()
    if not target:
        return None

    parsed = urlsplit(target)
    if parsed.scheme or parsed.netloc:
        return None

    path_part = unquote(parsed.path)
    fragment = unquote(parsed.fragment)
    candidate = (root / path_part.lstrip("/")) if path_part.startswith("/") else (
        source_path.parent / path_part if path_part else source_path
    )
    resolved = candidate.resolve(strict=False)
    resolved_root = root.resolve(strict=False)

    try:
        resolved.relative_to(resolved_root)
    except ValueError:
        return f"link escapes repository root: {raw_target}"

    if not resolved.exists():
        return f"broken local target: {raw_target}"

    if resolved.is_dir():
        directory_candidates = (resolved / "README.md", resolved / "index.md", resolved / "index.html")
        resolved = next((item for item in directory_candidates if item.is_file()), resolved)
        if resolved.is_dir():
            return f"directory link has no README.md, index.md, or index.html: {raw_target}"

    if fragment and resolved.suffix.lower() in {".md", ".mdx"}:
        if LINE_ANCHOR.fullmatch(fragment):
            return None
        try:
            text = resolved.read_text(encoding="utf-8-sig")
        except (OSError, UnicodeDecodeError):
            return f"cannot read linked Markdown document: {raw_target}"
        if fragment not in markdown_anchors(text):
            return f"unknown Markdown fragment '#{fragment}' in {raw_target}"

    return None


def _link_targets(source: str):
    visible = strip_code_blocks(source)
    for regex in (MARKDOWN_LINK, REFERENCE_LINK):
        for match in regex.finditer(visible):
            target = match.group(1) or match.group(2)
            if target:
                yield target, visible.count("\\n", 0, match.start()) + 1
    for match in HTML_LINK.finditer(visible):
        yield match.group(1), visible.count("\\n", 0, match.start()) + 1


def validate_repository(root: Path) -> tuple[int, int, list[str]]:
    root = root.resolve()
    paths = tracked_markdown_files(root)
    if not paths:
        return 0, 0, ["no tracked Markdown documents found"]

    errors: list[str] = []
    link_count = 0
    for relative in paths:
        path = root / relative
        try:
            content = path.read_text(encoding="utf-8-sig")
        except UnicodeDecodeError as exc:
            errors.append(f"{relative}: invalid UTF-8 at byte {exc.start}")
            continue
        except OSError as exc:
            errors.append(f"{relative}: cannot read file: {exc}")
            continue

        if not content.strip():
            errors.append(f"{relative}: empty Markdown document")
            continue

        for raw_target, line in _link_targets(content):
            parsed = urlsplit(raw_target.strip().strip("<>"))
            if parsed.scheme or parsed.netloc:
                continue
            link_count += 1
            issue = validate_target(root, path, raw_target)
            if issue:
                errors.append(f"{relative}:{line}: {issue}")

    return len(paths), link_count, errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    args = parser.parse_args(argv)

    files, links, errors = validate_repository(args.root)
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        print(
            f"Markdown integrity failed: {files} documents scanned, "
            f"{links} local links checked, {len(errors)} issue(s).",
            file=sys.stderr,
        )
        return 1

    print(
        f"Markdown integrity passed: {files} tracked documents scanned; "
        f"{links} local navigation targets checked."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
