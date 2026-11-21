#!/usr/bin/env python3
"""CLife: explicit, content-addressed savepoints and safe source revival.

This tool never executes restored code and never deletes project files.
All source paths must be supplied explicitly by the operator.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import stat
import sys
import tempfile
import uuid
from datetime import datetime, timezone
from pathlib import Path, PurePosixPath
from typing import Any, Iterable

FORMAT_VERSION = 1
MAX_FILE_BYTES = 64 * 1024 * 1024
MAX_TOTAL_BYTES = 512 * 1024 * 1024
SNAPSHOT_ID_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$")
SHA256_RE = re.compile(r"^[a-f0-9]{64}$")


class CLifeError(Exception):
    """Expected, user-correctable CLife failure."""


def _root_path(root: str | Path) -> Path:
    result = Path(root).expanduser().resolve()
    if not result.exists() or not result.is_dir():
        raise CLifeError(f"Repository root is not a directory: {root}")
    return result


def _relative_path(raw: str) -> PurePosixPath:
    if not isinstance(raw, str) or not raw or "\x00" in raw:
        raise CLifeError("Path must be a non-empty relative path")
    normalized = raw.replace("\\", "/")
    path = PurePosixPath(normalized)
    parts = path.parts
    if path.is_absolute() or not parts or any(part in ("", ".", "..") for part in parts):
        raise CLifeError(f"Unsafe relative path: {raw}")
    if ":" in parts[0] or parts[0].lower() == ".git":
        raise CLifeError(f"Unsafe repository path: {raw}")
    if len(parts) >= 2 and parts[0] == ".clife" and parts[1] == "state":
        raise CLifeError("CLife state cannot be included in its own savepoints")
    return path


def _reject_symlink_components(root: Path, relative: PurePosixPath) -> Path:
    current = root
    for part in relative.parts:
        current = current / part
        if current.is_symlink():
            raise CLifeError(f"Symlinks are not allowed in CLife paths: {relative}")
        if not current.exists():
            break
    return root.joinpath(*relative.parts)


def _state_root(root: Path) -> Path:
    current = root
    for part in (".clife", "state", "savepoints"):
        current = current / part
        if current.is_symlink():
            raise CLifeError(f"CLife state path contains a symlink: {current}")
        current.mkdir(exist_ok=True)
        if not current.is_dir():
            raise CLifeError(f"CLife state component is not a directory: {current}")
    return current


def _digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _atomic_write(destination: Path, data: bytes, mode: int | None = None) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.is_symlink():
        raise CLifeError(f"Refusing to replace symlink: {destination}")
    fd, temp_name = tempfile.mkstemp(prefix=".clife-tmp-", dir=str(destination.parent))
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        if mode is not None:
            os.chmod(temp_name, stat.S_IMODE(mode))
        os.replace(temp_name, destination)
    finally:
        try:
            os.unlink(temp_name)
        except FileNotFoundError:
            pass


def _make_snapshot_id() -> str:
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return f"sp-{stamp}-{uuid.uuid4().hex[:8]}"


def savepoint(root: str | Path, paths: Iterable[str], label: str = "") -> str:
    """Capture explicit regular files into a verified, content-addressed savepoint."""
    base = _root_path(root)
    requested = list(paths)
    if not requested:
        raise CLifeError("At least one --path must be supplied; CLife never snapshots implicitly")
    state = _state_root(base)
    snapshot_id = _make_snapshot_id()
    snapshot_dir = state / snapshot_id
    snapshot_dir.mkdir()
    entries: list[dict[str, Any]] = []
    seen: set[str] = set()
    total = 0
    try:
        for raw in requested:
            relative = _relative_path(raw)
            key = relative.as_posix()
            if key in seen:
                raise CLifeError(f"Duplicate path: {key}")
            seen.add(key)
            source = _reject_symlink_components(base, relative)
            if not source.exists() or not source.is_file():
                raise CLifeError(f"Path is not an existing regular file: {key}")
            info = source.stat()
            if info.st_size > MAX_FILE_BYTES:
                raise CLifeError(f"File exceeds {MAX_FILE_BYTES} byte limit: {key}")
            data = source.read_bytes()
            total += len(data)
            if total > MAX_TOTAL_BYTES:
                raise CLifeError(f"Savepoint exceeds {MAX_TOTAL_BYTES} byte total limit")
            digest = _digest(data)
            object_rel = f"objects/{digest}.blob"
            object_path = snapshot_dir / object_rel
            object_path.parent.mkdir(exist_ok=True)
            if not object_path.exists():
                _atomic_write(object_path, data)
            entries.append({
                "path": key,
                "sha256": digest,
                "size": len(data),
                "mode": stat.S_IMODE(info.st_mode),
                "object": object_rel,
            })
        manifest = {
            "format_version": FORMAT_VERSION,
            "id": snapshot_id,
            "created_utc": datetime.now(timezone.utc).isoformat(),
            "label": label[:120],
            "files": entries,
        }
        _atomic_write(snapshot_dir / "manifest.json", (json.dumps(manifest, indent=2, sort_keys=True) + "\n").encode("utf-8"))
        return snapshot_id
    except Exception:
        for child in sorted(snapshot_dir.rglob("*"), reverse=True):
            if child.is_file() and not child.is_symlink():
                child.unlink(missing_ok=True)
            elif child.is_dir() and not child.is_symlink():
                try:
                    child.rmdir()
                except OSError:
                    pass
        try:
            snapshot_dir.rmdir()
        except OSError:
            pass
        raise


def _load_snapshot(root: Path, snapshot_id: str) -> tuple[Path, dict[str, Any]]:
    if not SNAPSHOT_ID_RE.fullmatch(snapshot_id):
        raise CLifeError("Invalid savepoint identifier")
    state = _state_root(root)
    snapshot_dir = state / snapshot_id
    if snapshot_dir.is_symlink() or not snapshot_dir.is_dir():
        raise CLifeError(f"Savepoint not found: {snapshot_id}")
    manifest_path = snapshot_dir / "manifest.json"
    if manifest_path.is_symlink() or not manifest_path.is_file():
        raise CLifeError("Savepoint manifest is missing or unsafe")
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise CLifeError(f"Cannot read savepoint manifest: {exc}") from exc
    if manifest.get("format_version") != FORMAT_VERSION or manifest.get("id") != snapshot_id:
        raise CLifeError("Unsupported or mismatched savepoint manifest")
    files = manifest.get("files")
    if not isinstance(files, list) or not files:
        raise CLifeError("Savepoint manifest contains no files")
    seen: set[str] = set()
    for entry in files:
        if not isinstance(entry, dict):
            raise CLifeError("Malformed savepoint entry")
        relative = _relative_path(entry.get("path"))
        key = relative.as_posix()
        if key in seen:
            raise CLifeError("Duplicate path in savepoint manifest")
        seen.add(key)
        digest = entry.get("sha256")
        if not isinstance(digest, str) or not SHA256_RE.fullmatch(digest):
            raise CLifeError(f"Invalid checksum for {key}")
        if entry.get("object") != f"objects/{digest}.blob":
            raise CLifeError(f"Invalid object reference for {key}")
        obj = snapshot_dir / "objects" / f"{digest}.blob"
        if obj.is_symlink() or not obj.is_file():
            raise CLifeError(f"Savepoint object is missing or unsafe: {key}")
        data = obj.read_bytes()
        if len(data) != entry.get("size") or _digest(data) != digest:
            raise CLifeError(f"Savepoint object failed integrity check: {key}")
    return snapshot_dir, manifest


def verify_snapshot(root: str | Path, snapshot_id: str) -> dict[str, Any]:
    base = _root_path(root)
    _, manifest = _load_snapshot(base, snapshot_id)
    return {"snapshot": snapshot_id, "verified": True, "files": len(manifest["files"])}


def _selected_entries(manifest: dict[str, Any], paths: Iterable[str] | None) -> list[dict[str, Any]]:
    entries = manifest["files"]
    if paths is None:
        return entries
    wanted = {_relative_path(path).as_posix() for path in paths}
    selected = [entry for entry in entries if entry["path"] in wanted]
    missing = wanted - {entry["path"] for entry in selected}
    if missing:
        raise CLifeError("Path is not part of the savepoint: " + ", ".join(sorted(missing)))
    return selected


def plan_rethread(root: str | Path, snapshot_id: str, paths: Iterable[str] | None = None) -> dict[str, Any]:
    base = _root_path(root)
    _, manifest = _load_snapshot(base, snapshot_id)
    entries = _selected_entries(manifest, paths)
    statuses: list[dict[str, str]] = []
    for entry in entries:
        relative = _relative_path(entry["path"])
        target = _reject_symlink_components(base, relative)
        if not target.exists():
            status = "missing"
        elif not target.is_file():
            raise CLifeError(f"Target is not a regular file: {entry['path']}")
        else:
            status = "same" if _digest(target.read_bytes()) == entry["sha256"] else "changed"
        statuses.append({"path": entry["path"], "status": status})
    return {"snapshot": snapshot_id, "files": statuses, "needs_rethread": sum(x["status"] != "same" for x in statuses)}


def _rollback_from_snapshot(root: Path, snapshot_id: str, existed_before: set[str], applied: list[str], expected_hashes: dict[str, str]) -> None:
    snapshot_dir, manifest = _load_snapshot(root, snapshot_id)
    originals = {entry["path"]: entry for entry in manifest["files"]}
    for key in reversed(applied):
        relative = _relative_path(key)
        target = _reject_symlink_components(root, relative)
        if key in existed_before:
            entry = originals[key]
            data = (snapshot_dir / entry["object"]).read_bytes()
            _atomic_write(target, data, entry.get("mode"))
        elif target.is_file() and _digest(target.read_bytes()) == expected_hashes[key]:
            target.unlink()


def rethread(root: str | Path, snapshot_id: str, paths: Iterable[str] | None = None, apply: bool = False) -> dict[str, Any]:
    """Plan or apply content repair; applying always checkpoints existing targets first."""
    base = _root_path(root)
    snapshot_dir, manifest = _load_snapshot(base, snapshot_id)
    entries = _selected_entries(manifest, paths)
    plan = plan_rethread(base, snapshot_id, [entry["path"] for entry in entries])
    changes = [entry for entry in entries if next(x["status"] for x in plan["files"] if x["path"] == entry["path"]) != "same"]
    if not apply or not changes:
        return {**plan, "applied": False, "emergency_savepoint": None}

    existing = [entry["path"] for entry in changes if (base / entry["path"]).is_file()]
    existed_before = set(existing)
    emergency_id = savepoint(base, existing, label=f"CLife pre-rethread {snapshot_id}") if existing else None
    staged: list[tuple[str, Path, Path]] = []
    expected_hashes = {entry["path"]: entry["sha256"] for entry in changes}
    applied: list[str] = []
    try:
        for entry in changes:
            relative = _relative_path(entry["path"])
            target = _reject_symlink_components(base, relative)
            target.parent.mkdir(parents=True, exist_ok=True)
            if target.is_symlink():
                raise CLifeError(f"Refusing to replace symlink: {entry['path']}")
            data = (snapshot_dir / entry["object"]).read_bytes()
            fd, temp_name = tempfile.mkstemp(prefix=".clife-stage-", dir=str(target.parent))
            with os.fdopen(fd, "wb") as stream:
                stream.write(data)
                stream.flush()
                os.fsync(stream.fileno())
            os.chmod(temp_name, stat.S_IMODE(entry.get("mode", 0o644)))
            staged.append((entry["path"], target, Path(temp_name)))
        for key, target, staged_path in staged:
            os.replace(staged_path, target)
            applied.append(key)
        for key, target, _ in staged:
            if _digest(target.read_bytes()) != expected_hashes[key]:
                raise CLifeError(f"Post-write checksum mismatch: {key}")
    except Exception as exc:
        if emergency_id:
            _rollback_from_snapshot(base, emergency_id, existed_before, applied, expected_hashes)
        else:
            for key in reversed(applied):
                target = base / key
                if target.is_file() and _digest(target.read_bytes()) == expected_hashes[key]:
                    target.unlink()
        raise CLifeError(f"Rethread failed; rollback attempted: {exc}") from exc
    finally:
        for _, _, staged_path in staged:
            staged_path.unlink(missing_ok=True)
    return {**plan, "applied": True, "restored": [entry["path"] for entry in changes], "emergency_savepoint": emergency_id}


def revive(root: str | Path, snapshot_id: str, path: str, apply: bool = False) -> dict[str, Any]:
    """Give one archived file a second life; never imports or executes it."""
    relative = _relative_path(path).as_posix()
    result = rethread(root, snapshot_id, paths=[relative], apply=apply)
    return {**result, "operation": "second_life", "path": relative}


def list_savepoints(root: str | Path) -> list[dict[str, Any]]:
    base = _root_path(root)
    state = _state_root(base)
    output: list[dict[str, Any]] = []
    for item in sorted(state.iterdir()):
        if item.is_symlink() or not item.is_dir():
            continue
        manifest = item / "manifest.json"
        if not manifest.is_file() or manifest.is_symlink():
            continue
        try:
            data = json.loads(manifest.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            continue
        output.append({"id": data.get("id", item.name), "created_utc": data.get("created_utc"), "files": len(data.get("files", [])), "label": data.get("label", "")})
    return output


def _print_json(value: Any) -> None:
    print(json.dumps(value, indent=2, sort_keys=True))


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="CLife savepoints, controlled rethreading and safe second-life recovery")
    parser.add_argument("--root", default=".", help="repository root (default: current directory)")
    sub = parser.add_subparsers(dest="command", required=True)
    capture = sub.add_parser("savepoint", help="capture explicitly listed files")
    capture.add_argument("--path", action="append", required=True, help="relative file path; repeat for multiple files")
    capture.add_argument("--label", default="", help="short human-readable checkpoint label")
    verify = sub.add_parser("verify", help="verify manifest and all stored SHA-256 objects")
    verify.add_argument("snapshot")
    plan = sub.add_parser("plan", help="show drift against a savepoint without writing")
    plan.add_argument("snapshot")
    repair = sub.add_parser("rethread", help="repair drift; requires --apply to write")
    repair.add_argument("snapshot")
    repair.add_argument("--path", action="append", help="limit to a path present in the savepoint")
    repair.add_argument("--apply", action="store_true", help="explicitly apply after automatic emergency checkpoint")
    second = sub.add_parser("revive", help="restore one archived path; requires --apply to write")
    second.add_argument("snapshot")
    second.add_argument("--path", required=True)
    second.add_argument("--apply", action="store_true", help="explicitly restore the selected path")
    sub.add_parser("list", help="list available savepoints")
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        if args.command == "savepoint":
            _print_json({"snapshot": savepoint(args.root, args.path, args.label), "captured": args.path})
        elif args.command == "verify":
            _print_json(verify_snapshot(args.root, args.snapshot))
        elif args.command == "plan":
            _print_json(plan_rethread(args.root, args.snapshot))
        elif args.command == "rethread":
            _print_json(rethread(args.root, args.snapshot, args.path, args.apply))
        elif args.command == "revive":
            _print_json(revive(args.root, args.snapshot, args.path, args.apply))
        elif args.command == "list":
            _print_json({"savepoints": list_savepoints(args.root)})
        return 0
    except CLifeError as exc:
        print(f"CLife: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
