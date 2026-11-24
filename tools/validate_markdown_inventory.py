#!/usr/bin/env python3
"""Validate every tracked Markdown document and its local navigation targets."""
from __future__ import annotations
import argparse
import re
import subprocess
import sys
from pathlib import Path
from urllib.parse import unquote, urlsplit

FENCE_OPEN = re.compile(r"^[ \t]{0,3}(\x60{3,}|~{3,})(.*)$")
MARKDOWN_LINK = re.compile(r"!? \[[^\]]*\]\(\s*(?:<([^>]+)>|([^\s)]+))".replace("!? ", "!?"))
REFERENCE_LINK = re.compile(r"(?m)^[ \t]{0,3}\[[^\]]+\]:[ \t]*(?:<([^>]+)>|(\S+))")
HTML_LINK = re.compile(r"""\b(?:href|src)\s*=\s*["']([^"']+)["']""", re.IGNORECASE)
HTML_ID = re.compile(r"""\b(?:id|name)\s*=\s*["']([^"']+)["']""", re.IGNORECASE)
HEADING = re.compile(r"^[ \t]{0,3}(#{1,6})[ \t]+(.+?)\s*#*[ \t]*$")
INLINE_CODE = re.compile(r"(\x60+)(.*?)\1")
HTML_TAG = re.compile(r"<[^>]*>")
MARKDOWN_LINK_LABEL = re.compile(r"!\[([^\]]*)\]\([^)]*\)")
LINE_ANCHOR = re.compile(r"^L\d+(?:-L\d+)?$", re.IGNORECASE)


def strip_code_blocks(source: str) -> str:
    output: list[str] = []
    marker: str | None = None
    marker_length = 0
    for line in source.splitlines(keepends=True):
        match = FENCE_OPEN.match(line.rstrip("\r\n"))
        if marker is None and match:
            token = match.group(1)
            marker = token[0]
            marker_length = len(token)
            output.append("\n" if line.endswith("\n") else "")
            continue
        if marker is not None:
            if match and match.group(1)[0] == marker and len(match.group(1)) >= marker_length:
                marker = None
                marker_length = 0
            output.append("\n" if line.endswith("\n") else "")
            continue
        output.append(line)
    return INLINE_CODE.sub("", "".join(output))


def slugify_heading(value: str) -> str:
    value = HTML_TAG.sub("", value)
    value = re.sub(r"\[([^\]]*)\]\([^)]*\)", r"\1", value)
    value = re.sub(r"[*_~]", "", value).strip().casefold()
    value = re.sub(r"[^\w\- ]", "", value, flags=re.UNICODE)
    value = re.sub(r"\s+", "-", value)
    value = re.sub(r"-{2,}", "-", value)
    return value.strip("-")


def markdown_anchors(source: str) -> set[str]:
    anchors: set[str] = set()
    seen: dict[str, int] = {}
    for line in strip_code_blocks(source).splitlines():
        match = HEADING.match(line)
        if match:
            base = slugify_heading(match.group(2))
            count = seen.get(base, 0)
            seen[base] = count + 1
            anchors.add(base if count == 0 else f"{base}-{count}")
        anchors.update(HTML_ID.findall(line))
        anchors.update(re.findall(r"\{#([A-Za-z0-9_.:-]+)\}", line))
    return anchors


def tracked_markdown_files(root: Path) -> list[Path]:
    try:
        result = subprocess.run(
            ["git", "-C", str(root), "ls-files", "-z", "--", "*.md", "*.mdx"],
            check=True, capture_output=True,
        )
        paths = [Path(item.decode("utf-8", errors="strict"))
                 for item in result.stdout.split(b"\0") if item]
        return sorted(path for path in paths if (root / path).is_file())
    except (OSError, subprocess.CalledProcessError, UnicodeDecodeError):
        excluded = {".git", ".venv", "venv", "node_modules", "build", "dist"}
        return sorted(
            path.relative_to(root) for path in root.rglob("*")
            if path.is_file() and path.suffix.lower() in {".md", ".mdx"}
            and not any(part in excluded for part in path.relative_to(root).parts)
        )


def validate_target(root: Path, source_path: Path, raw_target: str) -> str | None:
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
    try:
        resolved.relative_to(root.resolve(strict=False))
    except ValueError:
        return f"link escapes repository root: {raw_target}"
    if not resolved.exists():
        return f"broken local target: {raw_target}"

    if resolved.is_dir():
        options = (resolved / "README.md", resolved / "index.md", resolved / "index.html")
        resolved = next((item for item in options if item.is_file()), resolved)
        if resolved.is_dir():
            return f"directory link has no README.md, index.md, or index.html: {raw_target}"

    if fragment and resolved.suffix.lower() in {".md", ".mdx"}:
        if LINE_ANCHOR.fullmatch(fragment):
            return None
        try:
            linked_text = resolved.read_text(encoding="utf-8-sig")
        except (OSError, UnicodeDecodeError):
            return f"cannot read linked Markdown document: {raw_target}"
        if fragment not in markdown_anchors(linked_text):
            return f"unknown Markdown fragment '#{fragment}' in {raw_target}"
    return None


def link_targets(source: str):
    visible = strip_code_blocks(source)
    for regex in (MARKDOWN_LINK, REFERENCE_LINK):
        for match in regex.finditer(visible):
            target = match.group(1) or match.group(2)
            if target:
                yield target, visible.count("\n", 0, match.start()) + 1
    for match in HTML_LINK.finditer(visible):
        yield match.group(1), visible.count("\n", 0, match.start()) + 1


def validate_repository(root: Path) -> tuple[int, int, list[str]]:
    root = root.resolve()
    paths = tracked_markdown_files(root)
    if not paths:
        return 0, 0, ["no tracked Markdown documents found"]
    errors: list[str] = []
    checked_links = 0
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
        for raw_target, line in link_targets(content):
            parsed = urlsplit(raw_target.strip().strip("<>"))
            if parsed.scheme or parsed.netloc:
                continue
            checked_links += 1
            issue = validate_target(root, path, raw_target)
            if issue:
                errors.append(f"{relative}:{line}: {issue}")
    return len(paths), checked_links, errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    args = parser.parse_args(argv)
    files, links, errors = validate_repository(args.root)
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        print(f"Markdown integrity failed: {files} documents, {links} local links, {len(errors)} issue(s).", file=sys.stderr)
        return 1
    print(f"Markdown integrity passed: {files} tracked documents; {links} local navigation targets checked.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
