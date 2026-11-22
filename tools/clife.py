#!/usr/bin/env python3
"""CLife v1: bounded, integrity-checked project snapshots.

Snapshots are ZIP containers with a SHA-256 manifest. Restore always creates a
new directory; it never overwrites the source tree or an existing destination.
This is a software-data lifecycle tool, not a physical/biological simulator.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import stat
import sys
import tempfile
from datetime import datetime, timezone
from typing import Any
import zipfile

SCHEMA = "clife.snapshot.v1"
CHUNK_SIZE = 1024 * 1024
MAX_FILES = 20_000
MAX_FILE_BYTES = 128 * 1024 * 1024
MAX_TOTAL_BYTES = 1024 * 1024 * 1024
MAX_MANIFEST_BYTES = 16 * 1024 * 1024
EXCLUDED_DIRS = {
    ".git", ".hg", ".svn", ".venv", "venv", "node_modules",
    "__pycache__", ".pytest_cache", ".mypy_cache", ".tox",
    "build", "dist", ".clife",
}


class ClifeError(Exception):
    """A validation, integrity, or filesystem error in the CLife workflow."""


def _relative_name(value: str) -> str:
    if not isinstance(value, str) or not value or "\\" in value:
        raise ClifeError("snapshot contains an invalid path")
    path = PurePosixPath(value)
    if path.is_absolute() or any(part in ("", ".", "..") for part in path.parts):
        raise ClifeError(f"unsafe snapshot path: {value!r}")
    if path.parts and ":" in path.parts[0]:
        raise ClifeError(f"drive-qualified snapshot path is not allowed: {value!r}")
    return path.as_posix()


def _walk(root: Path, excluded: Path | None = None) -> list[tuple[Path, str, os.stat_result]]:
    result: list[tuple[Path, str, os.stat_result]] = []
    total = 0
    for base, dirs, names in os.walk(root, topdown=True, followlinks=False):
        base_path = Path(base)
        kept: list[str] = []
        for name in sorted(dirs):
            if name in EXCLUDED_DIRS:
                continue
            candidate = base_path / name
            try:
                if candidate.is_symlink() or (excluded and candidate.resolve() == excluded):
                    continue
            except OSError:
                continue
            kept.append(name)
        dirs[:] = kept
        for name in sorted(names):
            path = base_path / name
            try:
                if path.is_symlink() or (excluded and path.resolve() == excluded):
                    continue
                info = path.stat(follow_symlinks=False)
            except OSError as exc:
                raise ClifeError(f"cannot inspect {path}: {exc}") from exc
            if not stat.S_ISREG(info.st_mode):
                continue
            rel = path.relative_to(root).as_posix()
            _relative_name(rel)
            if info.st_size > MAX_FILE_BYTES:
                raise ClifeError(f"file exceeds the {MAX_FILE_BYTES}-byte per-file limit: {rel}")
            total += info.st_size
            if total > MAX_TOTAL_BYTES:
                raise ClifeError(f"snapshot exceeds the {MAX_TOTAL_BYTES}-byte total limit")
            result.append((path, rel, info))
            if len(result) > MAX_FILES:
                raise ClifeError(f"snapshot exceeds the {MAX_FILES}-file limit")
    result.sort(key=lambda item: item[1])
    return result


def snapshot(root: str | Path, output: str | Path | None = None) -> dict[str, Any]:
    source = Path(root).expanduser().resolve(strict=True)
    if not source.is_dir():
        raise ClifeError("snapshot root must be a directory")
    if output is None:
        stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        destination = source / ".clife" / "snapshots" / f"clife-{stamp}.clife"
    else:
        destination = Path(output).expanduser().absolute()
    destination.parent.mkdir(parents=True, exist_ok=True)
    if os.path.lexists(destination):
        raise ClifeError(f"snapshot destination already exists: {destination}")
    excluded = destination.resolve(strict=False)
    files = _walk(source, excluded)
    temp_path: str | None = None
    entries: list[dict[str, Any]] = []
    total = 0
    try:
        with tempfile.NamedTemporaryFile(
            prefix=".clife-", suffix=".tmp", dir=destination.parent, delete=False
        ) as temp:
            temp_path = temp.name
        with zipfile.ZipFile(temp_path, "w", compression=zipfile.ZIP_DEFLATED,
                             compresslevel=6, allowZip64=True) as archive:
            for path, relative, before in files:
                digest = hashlib.sha256()
                bytes_written = 0
                info = zipfile.ZipInfo("files/" + relative)
                info.compress_type = zipfile.ZIP_DEFLATED
                info.external_attr = (stat.S_IMODE(before.st_mode) & 0o777) << 16
                with path.open("rb") as source_file, archive.open(info, "w", force_zip64=True) as member:
                    while True:
                        chunk = source_file.read(CHUNK_SIZE)
                        if not chunk:
                            break
                        bytes_written += len(chunk)
                        if bytes_written > MAX_FILE_BYTES:
                            raise ClifeError(f"file grew beyond the configured limit while snapshotting: {relative}")
                        digest.update(chunk)
                        member.write(chunk)
                after = path.stat(follow_symlinks=False)
                if (before.st_size, before.st_mtime_ns, before.st_ino) != (
                    after.st_size, after.st_mtime_ns, after.st_ino
                ) or bytes_written != before.st_size:
                    raise ClifeError(f"file changed while snapshotting: {relative}")
                total += bytes_written
                if total > MAX_TOTAL_BYTES:
                    raise ClifeError("snapshot exceeded the total-size limit while reading")
                entries.append({
                    "path": relative,
                    "size": bytes_written,
                    "sha256": digest.hexdigest(),
                    "mode": stat.S_IMODE(before.st_mode) & 0o777,
                })
            manifest = {
                "schema": SCHEMA,
                "created_at": datetime.now(timezone.utc).isoformat(),
                "root_name": source.name,
                "file_count": len(entries),
                "total_bytes": total,
                "entries": entries,
            }
            archive.writestr(
                "manifest.json",
                json.dumps(manifest, ensure_ascii=False, sort_keys=True, separators=(",", ":")),
            )
        os.replace(temp_path, destination)
        temp_path = None
        return {
            "archive": str(destination),
            "schema": SCHEMA,
            "file_count": len(entries),
            "total_bytes": total,
            "manifest_sha256": hashlib.sha256(
                json.dumps(manifest, sort_keys=True, separators=(",", ":")).encode("utf-8")
            ).hexdigest(),
        }
    except (OSError, zipfile.BadZipFile) as exc:
        raise ClifeError(f"snapshot failed: {exc}") from exc
    finally:
        if temp_path and os.path.exists(temp_path):
            try:
                os.unlink(temp_path)
            except OSError:
                pass


def _load_manifest(archive: zipfile.ZipFile) -> dict[str, Any]:
    try:
        names = archive.namelist()
        if len(names) > MAX_FILES + 1 or len(names) != len(set(names)):
            raise ClifeError("snapshot has too many or duplicate archive members")
        if "manifest.json" not in names:
            raise ClifeError("snapshot manifest is missing")
        info = archive.getinfo("manifest.json")
        if info.file_size > MAX_MANIFEST_BYTES:
            raise ClifeError("snapshot manifest is too large")
        manifest = json.loads(archive.read("manifest.json").decode("utf-8"))
    except (KeyError, UnicodeDecodeError, json.JSONDecodeError, zipfile.BadZipFile) as exc:
        raise ClifeError(f"snapshot manifest is invalid: {exc}") from exc
    if not isinstance(manifest, dict) or manifest.get("schema") != SCHEMA:
        raise ClifeError("unsupported CLife snapshot schema")
    entries = manifest.get("entries")
    if not isinstance(entries, list) or len(entries) > MAX_FILES:
        raise ClifeError("snapshot entry list is invalid")
    if manifest.get("file_count") != len(entries):
        raise ClifeError("snapshot file count does not match its manifest")
    seen: set[str] = set()
    total = 0
    expected = {"manifest.json"}
    for entry in entries:
        if not isinstance(entry, dict):
            raise ClifeError("snapshot entry is not an object")
        rel = _relative_name(entry.get("path"))
        if rel in seen:
            raise ClifeError(f"duplicate snapshot path: {rel}")
        seen.add(rel)
        size = entry.get("size")
        mode = entry.get("mode", 0o644)
        digest = entry.get("sha256")
        if isinstance(size, bool) or not isinstance(size, int) or not 0 <= size <= MAX_FILE_BYTES:
            raise ClifeError(f"invalid size for snapshot entry: {rel}")
        if isinstance(mode, bool) or not isinstance(mode, int) or not 0 <= mode <= 0o777:
            raise ClifeError(f"invalid mode for snapshot entry: {rel}")
        if not isinstance(digest, str) or len(digest) != 64 or any(c not in "0123456789abcdef" for c in digest):
            raise ClifeError(f"invalid SHA-256 for snapshot entry: {rel}")
        total += size
        if total > MAX_TOTAL_BYTES:
            raise ClifeError("snapshot manifest exceeds the total-size limit")
        expected.add("files/" + rel)
    if manifest.get("total_bytes") != total:
        raise ClifeError("snapshot total size does not match its manifest")
    if set(names) != expected:
        raise ClifeError("archive members do not exactly match the manifest")
    return manifest


def verify(archive_path: str | Path) -> dict[str, Any]:
    path = Path(archive_path).expanduser().resolve(strict=True)
    if not path.is_file():
        raise ClifeError("snapshot archive must be a regular file")
    try:
        with zipfile.ZipFile(path, "r") as archive:
            manifest = _load_manifest(archive)
            for entry in manifest["entries"]:
                name = "files/" + entry["path"]
                digest = hashlib.sha256()
                count = 0
                with archive.open(name, "r") as member:
                    while True:
                        chunk = member.read(CHUNK_SIZE)
                        if not chunk:
                            break
                        count += len(chunk)
                        if count > entry["size"]:
                            raise ClifeError(f"snapshot member is larger than declared: {entry['path']}")
                        digest.update(chunk)
                if count != entry["size"] or digest.hexdigest() != entry["sha256"]:
                    raise ClifeError(f"integrity check failed: {entry['path']}")
    except (OSError, zipfile.BadZipFile, RuntimeError) as exc:
        if isinstance(exc, ClifeError):
            raise
        raise ClifeError(f"cannot verify snapshot: {exc}") from exc
    return {
        "verified": True,
        "schema": SCHEMA,
        "file_count": manifest["file_count"],
        "total_bytes": manifest["total_bytes"],
        "created_at": manifest.get("created_at"),
        "root_name": manifest.get("root_name"),
    }


def restore(archive_path: str | Path, destination: str | Path) -> dict[str, Any]:
    checked = verify(archive_path)
    archive_file = Path(archive_path).expanduser().resolve(strict=True)
    target = Path(destination).expanduser().absolute()
    if os.path.lexists(target):
        raise ClifeError(f"restore destination already exists; refusing to overwrite: {target}")
    target.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix=".clife-restore-", dir=target.parent))
    try:
        with zipfile.ZipFile(archive_file, "r") as archive:
            manifest = _load_manifest(archive)
            for entry in manifest["entries"]:
                relative = _relative_name(entry["path"])
                output = staging.joinpath(*PurePosixPath(relative).parts)
                output.parent.mkdir(parents=True, exist_ok=True)
                digest = hashlib.sha256()
                count = 0
                with archive.open("files/" + relative, "r") as member, output.open("xb") as target_file:
                    while True:
                        chunk = member.read(CHUNK_SIZE)
                        if not chunk:
                            break
                        count += len(chunk)
                        if count > entry["size"]:
                            raise ClifeError(f"restore member is larger than declared: {relative}")
                        digest.update(chunk)
                        target_file.write(chunk)
                if count != entry["size"] or digest.hexdigest() != entry["sha256"]:
                    raise ClifeError(f"restore integrity check failed: {relative}")
                os.chmod(output, entry.get("mode", 0o644) & 0o777)
        if os.path.lexists(target):
            raise ClifeError(f"restore destination appeared during operation: {target}")
        os.replace(staging, target)
        return {**checked, "restored_to": str(target), "overwrote_existing_files": False}
    except Exception:
        if staging.exists():
            import shutil
            shutil.rmtree(staging, ignore_errors=True)
        raise


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="CLife v1 project snapshot and recovery utility")
    sub = parser.add_subparsers(dest="command", required=True)
    create = sub.add_parser("snapshot", help="create a bounded project snapshot")
    create.add_argument("root", nargs="?", default=".")
    create.add_argument("--output")
    check = sub.add_parser("verify", help="verify every file against its SHA-256 manifest")
    check.add_argument("archive")
    info = sub.add_parser("inspect", help="print the verified snapshot metadata")
    info.add_argument("archive")
    recovery = sub.add_parser("restore", help="restore into a new directory without overwriting")
    recovery.add_argument("archive")
    recovery.add_argument("--to", required=True, dest="destination")
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        if args.command == "snapshot":
            result = snapshot(args.root, args.output)
        elif args.command == "verify":
            result = verify(args.archive)
        elif args.command == "inspect":
            result = verify(args.archive)
        else:
            result = restore(args.archive, args.destination)
        print(json.dumps(result, ensure_ascii=False, indent=2))
        return 0
    except (ClifeError, OSError, ValueError) as exc:
        print(f"CLife error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
