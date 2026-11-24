#!/usr/bin/env python3
"""Create a deterministic SHA-256 inventory for a VentuSample release package."""
from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path

VERSION_RE = re.compile(r"^v[0-9]+\.[0-9]+\.[0-9]+(?:[+-][0-9A-Za-z.-]+)?$")
COMMIT_RE = re.compile(r"^[0-9a-f]{40}$", re.IGNORECASE)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def create_manifest(
    package_dir: Path,
    version: str,
    commit: str,
    platform: str = "windows-x64",
    compiler: str = "MSVC 2022",
    qt_version: str = "6.8.3",
    output_name: str = "release-manifest.json",
) -> dict:
    root = package_dir.resolve(strict=True)
    if not root.is_dir():
        raise ValueError("package_dir must be a directory")
    if not VERSION_RE.fullmatch(version):
        raise ValueError("version must be a v-prefixed semantic version")
    if not COMMIT_RE.fullmatch(commit):
        raise ValueError("commit must be a full 40-character Git SHA")

    output = root / output_name
    files: list[dict] = []
    for path in sorted(root.rglob("*"), key=lambda item: item.as_posix()):
        if path.is_symlink():
            raise ValueError(f"symbolic links are not allowed in release packages: {path}")
        if not path.is_file():
            continue
        resolved = path.resolve(strict=True)
        try:
            relative = resolved.relative_to(root).as_posix()
        except ValueError as exc:
            raise ValueError(f"package file escapes package root: {path}") from exc
        if relative == output_name:
            continue
        if relative.startswith("/") or ".." in Path(relative).parts:
            raise ValueError(f"non-canonical package path: {relative}")
        files.append({
            "path": relative,
            "size": path.stat().st_size,
            "sha256": sha256_file(path),
        })

    if not files:
        raise ValueError("release package contains no files")

    manifest = {
        "schema": "ventusample.release.v1",
        "project": "VentuSample",
        "version": version,
        "source": {
            "repository": "Ivan-VHS/VentuSample",
            "ref": f"refs/tags/{version}",
            "commit": commit.lower(),
        },
        "build": {
            "platform": platform,
            "compiler": compiler,
            "qt_version": qt_version,
            "configuration": "Release",
        },
        "files": files,
    }

    encoded = json.dumps(manifest, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
    temporary = output.with_name(output.name + ".tmp")
    temporary.write_text(encoded, encoding="utf-8", newline="\n")
    temporary.replace(output)
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package-dir", type=Path, required=True)
    parser.add_argument("--version", required=True)
    parser.add_argument("--commit", required=True)
    parser.add_argument("--platform", default="windows-x64")
    parser.add_argument("--compiler", default="MSVC 2022")
    parser.add_argument("--qt-version", default="6.8.3")
    args = parser.parse_args()
    try:
        manifest = create_manifest(
            args.package_dir,
            args.version,
            args.commit,
            args.platform,
            args.compiler,
            args.qt_version,
        )
    except (OSError, ValueError) as exc:
        parser.error(str(exc))
    print(f"Created {args.package_dir / 'release-manifest.json'} with {len(manifest['files'])} file record(s).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
