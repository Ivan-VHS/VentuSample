#!/usr/bin/env python3
"""CLife: verified savepoints, targeted restitching and safe source revival."""
from __future__ import annotations

import argparse
import fnmatch
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import uuid
from datetime import datetime, timezone
from pathlib import Path, PurePosixPath
from typing import Any, Iterable

VERSION = "1.0.0"
DEFAULT_EXTENSIONS = {
    ".c", ".h", ".cc", ".cpp", ".cxx", ".hpp", ".hh", ".py", ".pyi",
    ".md", ".rst", ".txt", ".json", ".json5", ".jsonl", ".yaml", ".yml",
    ".toml", ".ini", ".cfg", ".conf", ".cmake", ".sh", ".bash", ".ps1",
    ".bat", ".cmd", ".lua", ".js", ".mjs", ".cjs", ".ts", ".tsx", ".jsx",
    ".html", ".css", ".scss", ".xml", ".svg", ".qss", ".properties", ".gradle",
    ".sql", ".proto", ".def", ".map", ".gitignore", ".gitattributes",
}
EXCLUDED_DIRS = {
    ".git", ".clife", ".venv", "venv", "env", "node_modules", "vendor",
    "build", "dist", "out", "target", "__pycache__", ".pytest_cache",
    ".mypy_cache", ".ruff_cache", ".cache", "cmake-build-debug",
    "cmake-build-release", "coverage", "htmlcov",
}
SENSITIVE_NAMES = {
    ".env", ".npmrc", ".pypirc", "credentials", "credentials.json",
    "secrets.json", "id_rsa", "id_ed25519", "known_hosts", "authorized_keys",
}
SENSITIVE_SUFFIXES = {".pem", ".key", ".p12", ".pfx", ".keystore", ".jks", ".token", ".secret"}
SNAPSHOT_ID_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,100}$")


class CLifeError(Exception):
    """Expected, user-facing CLife error."""


def _utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def _sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _root(path: str | Path) -> Path:
    root = Path(path).expanduser().resolve()
    if not root.is_dir():
        raise CLifeError(f"Repository root is not a directory: {root}")
    return root


def _safe_relative(raw: str) -> str:
    normalized = str(raw).replace("\\", "/")
    candidate = PurePosixPath(normalized)
    if not normalized or candidate.is_absolute() or ":" in normalized:
        raise CLifeError(f"Unsafe repository path: {raw!r}")
    if any(part in {"", ".", ".."} for part in candidate.parts):
        raise CLifeError(f"Unsafe repository path: {raw!r}")
    return candidate.as_posix()


def _inside(root: Path, relative: str) -> Path:
    rel = _safe_relative(relative)
    target = root.joinpath(*PurePosixPath(rel).parts)
    try:
        target.resolve(strict=False).relative_to(root.resolve())
    except ValueError as exc:
        raise CLifeError(f"Path escapes repository root: {relative!r}") from exc
    return target


def _has_symlink_component(root: Path, relative: str) -> bool:
    current = root
    for part in PurePosixPath(relative).parts:
        current = current / part
        if current.is_symlink():
            return True
    return False


def _load_config(root: Path) -> dict[str, Any]:
    path = root / ".clife" / "config.json"
    if not path.is_file():
        return {"schema_version": 1, "repository": root.name, "max_file_bytes": 8 * 1024 * 1024}
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise CLifeError(f"Cannot read CLife config {path}: {exc}") from exc
    if not isinstance(data, dict) or data.get("schema_version") != 1:
        raise CLifeError("Unsupported .clife/config.json schema_version (expected 1).")
    return data


def _git_file_list(root: Path) -> list[str] | None:
    try:
        result = subprocess.run(
            ["git", "-C", str(root), "ls-files", "-z", "--cached", "--others", "--exclude-standard"],
            check=False, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    if result.returncode != 0:
        return None
    return [os.fsdecode(item) for item in result.stdout.split(b"\0") if item]


def _walk_file_list(root: Path) -> list[str]:
    found: list[str] = []
    for current, dirs, files in os.walk(root, followlinks=False):
        current_path = Path(current)
        dirs[:] = [name for name in dirs if name not in EXCLUDED_DIRS and not (current_path / name).is_symlink()]
        for name in files:
            try:
                found.append((current_path / name).relative_to(root).as_posix())
            except ValueError:
                continue
    return found


def _eligible_path(relative: str, config: dict[str, Any]) -> bool:
    try:
        rel = _safe_relative(relative)
    except CLifeError:
        return False
    parts = PurePosixPath(rel).parts
    if any(part in EXCLUDED_DIRS for part in parts):
        return False
    name = parts[-1].lower()
    if name in SENSITIVE_NAMES or name.startswith(".env."):
        return False
    if any(name.endswith(suffix) for suffix in SENSITIVE_SUFFIXES):
        return False
    lowered = tuple(part.lower() for part in parts)
    if any(part in {"secrets", "credentials", "private", ".ssh"} for part in lowered):
        return False
    configured = config.get("excluded_paths", [])
    if isinstance(configured, list) and any(fnmatch.fnmatchcase(rel, pattern) for pattern in configured if isinstance(pattern, str)):
        return False
    extensions = config.get("allowed_extensions")
    allowed = {str(x).lower() for x in extensions} if isinstance(extensions, list) else DEFAULT_EXTENSIONS
    return Path(name).suffix.lower() in allowed or name in {"cmakelists.txt", "makefile", "dockerfile", ".gitignore", ".gitattributes"}


def _candidate_files(root: Path, config: dict[str, Any]) -> tuple[list[tuple[str, bytes, int]], int]:
    paths = _git_file_list(root)
    if paths is None:
        paths = _walk_file_list(root)
    max_bytes = config.get("max_file_bytes", 8 * 1024 * 1024)
    if not isinstance(max_bytes, int) or max_bytes < 1 or max_bytes > 64 * 1024 * 1024:
        raise CLifeError("max_file_bytes must be an integer between 1 and 67108864.")
    captured: list[tuple[str, bytes, int]] = []
    skipped = 0
    for relative in sorted(set(paths)):
        if not _eligible_path(relative, config):
            skipped += 1
            continue
        source = _inside(root, relative)
        if _has_symlink_component(root, relative) or not source.is_file():
            skipped += 1
            continue
        try:
            stat = source.stat()
            if stat.st_size > max_bytes:
                skipped += 1
                continue
            data = source.read_bytes()
            if b"\0" in data:
                skipped += 1
                continue
            data.decode("utf-8")
            captured.append((relative, data, stat.st_mode & 0o777))
        except (OSError, UnicodeError):
            skipped += 1
    return captured, skipped


def _git_metadata(root: Path) -> dict[str, Any]:
    def command(args: list[str]) -> str | None:
        try:
            result = subprocess.run(["git", "-C", str(root), *args], check=False,
                                    stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
            return result.stdout.strip() if result.returncode == 0 else None
        except (OSError, subprocess.SubprocessError):
            return None
    return {
        "commit": command(["rev-parse", "HEAD"]),
        "branch": command(["branch", "--show-current"]),
        "working_tree_dirty": bool(command(["status", "--porcelain"])),
    }


def make_snapshot(root: str | Path, label: str = "savepoint") -> str:
    base = _root(root)
    config = _load_config(base)
    files, skipped = _candidate_files(base, config)
    snapshot_id = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ") + "-" + uuid.uuid4().hex[:8]
    snapshot_dir = base / ".clife" / "snapshots" / snapshot_id
    payload_dir = snapshot_dir / "files"
    try:
        payload_dir.mkdir(parents=True, exist_ok=False)
        records: list[dict[str, Any]] = []
        for relative, data, mode in files:
            destination = _inside(payload_dir, relative)
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(data)
            records.append({"path": relative, "sha256": _sha256(data), "size": len(data), "mode": mode})
        manifest = {
            "format": "clife.snapshot.v1",
            "version": VERSION,
            "snapshot_id": snapshot_id,
            "label": str(label)[:120],
            "created_at": _utc_now(),
            "repository": str(config.get("repository") or base.name),
            "git": _git_metadata(base),
            "file_count": len(records),
            "skipped_count": skipped,
            "files": records,
        }
        encoded = (json.dumps(manifest, ensure_ascii=False, sort_keys=True, indent=2) + "\n").encode("utf-8")
        (snapshot_dir / "manifest.json").write_bytes(encoded)
        (snapshot_dir / "manifest.sha256").write_text(_sha256(encoded) + "  manifest.json\n", encoding="ascii")
    except Exception:
        shutil.rmtree(snapshot_dir, ignore_errors=True)
        raise
    return snapshot_id


def _snapshot_dir(root: Path, snapshot_id: str) -> Path:
    if not SNAPSHOT_ID_RE.fullmatch(snapshot_id):
        raise CLifeError(f"Invalid snapshot id: {snapshot_id!r}")
    directory = root / ".clife" / "snapshots" / snapshot_id
    if not directory.is_dir() or directory.is_symlink():
        raise CLifeError(f"Snapshot not found: {snapshot_id}")
    return directory


def _load_manifest(root: Path, snapshot_id: str) -> tuple[Path, dict[str, Any]]:
    directory = _snapshot_dir(root, snapshot_id)
    manifest_path = directory / "manifest.json"
    checksum_path = directory / "manifest.sha256"
    try:
        encoded = manifest_path.read_bytes()
        expected = checksum_path.read_text(encoding="ascii").split()[0]
    except (OSError, UnicodeError, IndexError) as exc:
        raise CLifeError(f"Snapshot manifest/checksum is unavailable: {exc}") from exc
    actual = _sha256(encoded)
    if not re.fullmatch(r"[0-9a-f]{64}", expected) or actual != expected:
        raise CLifeError("Snapshot manifest checksum mismatch; refusing to use a damaged savepoint.")
    try:
        manifest = json.loads(encoded.decode("utf-8"))
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise CLifeError(f"Snapshot manifest is invalid: {exc}") from exc
    if not isinstance(manifest, dict) or manifest.get("format") != "clife.snapshot.v1" or manifest.get("snapshot_id") != snapshot_id:
        raise CLifeError("Snapshot manifest format or identity mismatch.")
    if not isinstance(manifest.get("files"), list):
        raise CLifeError("Snapshot manifest has no valid files list.")
    seen: set[str] = set()
    for record in manifest["files"]:
        if not isinstance(record, dict) or not isinstance(record.get("path"), str):
            raise CLifeError("Snapshot contains a malformed file record.")
        rel = _safe_relative(record["path"])
        if rel in seen:
            raise CLifeError(f"Snapshot contains a duplicate path: {rel}")
        seen.add(rel)
        if not re.fullmatch(r"[0-9a-f]{64}", str(record.get("sha256", ""))):
            raise CLifeError(f"Snapshot contains an invalid digest for {rel}.")
    return directory, manifest


def verify_snapshot(root: str | Path, snapshot_id: str) -> tuple[int, list[str]]:
    base = _root(root)
    directory, manifest = _load_manifest(base, snapshot_id)
    errors: list[str] = []
    verified = 0
    for record in manifest["files"]:
        relative = _safe_relative(record["path"])
        payload = _inside(directory / "files", relative)
        if _has_symlink_component(directory / "files", relative) or not payload.is_file():
            errors.append(f"missing: {relative}")
            continue
        try:
            data = payload.read_bytes()
        except OSError as exc:
            errors.append(f"unreadable: {relative}: {exc}")
            continue
        if len(data) != record.get("size") or _sha256(data) != record["sha256"]:
            errors.append(f"hash mismatch: {relative}")
        else:
            verified += 1
    return verified, errors


def latest_snapshot(root: str | Path) -> str:
    base = _root(root)
    snapshots = base / ".clife" / "snapshots"
    if not snapshots.is_dir():
        raise CLifeError("No savepoints found. Create one with: clife.py savepoint")
    candidates = sorted(p.name for p in snapshots.iterdir() if p.is_dir() and SNAPSHOT_ID_RE.fullmatch(p.name))
    if not candidates:
        raise CLifeError("No savepoints found. Create one with: clife.py savepoint")
    return candidates[-1]


def compare_snapshot(root: str | Path, snapshot_id: str) -> dict[str, list[str]]:
    base = _root(root)
    _, manifest = _load_manifest(base, snapshot_id)
    current_files, _ = _candidate_files(base, _load_config(base))
    current = {path: _sha256(data) for path, data, _ in current_files}
    saved = {record["path"]: record["sha256"] for record in manifest["files"]}
    same = sorted(path for path, digest in saved.items() if current.get(path) == digest)
    changed = sorted(path for path, digest in saved.items() if path in current and current[path] != digest)
    missing = sorted(path for path in saved if path not in current)
    added = sorted(path for path in current if path not in saved)
    return {"same": same, "changed": changed, "missing": missing, "added": added}


def restore_snapshot(root: str | Path, snapshot_id: str, paths: Iterable[str] | None = None,
                     apply: bool = False) -> tuple[list[str], str | None]:
    base = _root(root)
    directory, manifest = _load_manifest(base, snapshot_id)
    _, errors = verify_snapshot(base, snapshot_id)
    if errors:
        raise CLifeError("Savepoint verification failed: " + "; ".join(errors[:8]))
    records = {record["path"]: record for record in manifest["files"]}
    selected = sorted(records) if paths is None else sorted({_safe_relative(path) for path in paths})
    if not selected:
        raise CLifeError("No files selected for restoration.")
    unknown = [path for path in selected if path not in records]
    if unknown:
        raise CLifeError("Path not present in this savepoint: " + ", ".join(unknown))
    actions: list[str] = []
    to_write: list[tuple[str, bytes, int]] = []
    for relative in selected:
        destination = _inside(base, relative)
        if _has_symlink_component(base, relative):
            raise CLifeError(f"Refusing to restore through a symlink: {relative}")
        if destination.exists() and not destination.is_file():
            raise CLifeError(f"Refusing to replace a non-file target: {relative}")
        payload = _inside(directory / "files", relative)
        data = payload.read_bytes()
        if _sha256(data) != records[relative]["sha256"]:
            raise CLifeError(f"Snapshot payload changed during restore: {relative}")
        if destination.is_file() and destination.read_bytes() == data:
            actions.append(f"unchanged  {relative}")
            continue
        actions.append(f"{'restore' if destination.exists() else 'recreate'}  {relative}")
        to_write.append((relative, data, int(records[relative].get("mode", 0o644)) & 0o777))
    if not apply or not to_write:
        return actions, None
    safety_id = make_snapshot(base, label=f"pre-restore:{snapshot_id}")
    for relative, data, mode in to_write:
        destination = _inside(base, relative)
        if _has_symlink_component(base, relative):
            raise CLifeError(f"Path changed to a symlink during restore; stopped: {relative}")
        destination.parent.mkdir(parents=True, exist_ok=True)
        if _has_symlink_component(base, relative):
            raise CLifeError(f"Refusing to write through a symlink: {relative}")
        fd, temp_name = tempfile.mkstemp(prefix=f".{destination.name}.clife-", dir=str(destination.parent))
        try:
            with os.fdopen(fd, "wb") as handle:
                handle.write(data)
                handle.flush()
                os.fsync(handle.fileno())
            os.chmod(temp_name, mode)
            os.replace(temp_name, destination)
        except Exception:
            try:
                os.unlink(temp_name)
            except OSError:
                pass
            raise
    return actions, safety_id


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="CLife — savepoint, verify, restitch and safely revive repository source.")
    parser.add_argument("--root", default=str(Path(__file__).resolve().parents[2]), help="repository root (defaults to this checkout)")
    parser.add_argument("--version", action="version", version=f"CLife {VERSION}")
    sub = parser.add_subparsers(dest="command", required=True)
    save = sub.add_parser("savepoint", aliases=["snapshot"], help="create a content-addressed recovery savepoint")
    save.add_argument("--label", default="savepoint")
    sub.add_parser("verify", help="verify manifest and every saved file hash").add_argument("snapshot", nargs="?", help="snapshot id; defaults to latest")
    sub.add_parser("status", help="compare the working tree with a savepoint").add_argument("snapshot", nargs="?", help="snapshot id; defaults to latest")
    sub.add_parser("list", help="list available savepoints")
    stitch = sub.add_parser("stitch", help="restitch selected files from a verified savepoint")
    stitch.add_argument("snapshot", help="snapshot id")
    stitch.add_argument("--path", action="append", required=True, help="repository-relative path; may be repeated")
    stitch.add_argument("--apply", action="store_true", help="write changes; without this flag, only preview")
    revive = sub.add_parser("revive", help="revive all files from a verified savepoint (never deletes extra files)")
    revive.add_argument("snapshot", nargs="?", help="snapshot id; defaults to latest")
    revive.add_argument("--apply", action="store_true", help="write changes; without this flag, only preview")
    return parser


def main(argv: list[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        root = _root(args.root)
        if args.command in {"savepoint", "snapshot"}:
            snapshot_id = make_snapshot(root, args.label)
            directory, manifest = _load_manifest(root, snapshot_id)
            print(f"CLife savepoint: {snapshot_id}")
            print(f"Captured: {manifest['file_count']} text files; skipped: {manifest['skipped_count']}")
            print(f"Stored: {directory}")
        elif args.command == "list":
            snapshots = root / ".clife" / "snapshots"
            if not snapshots.is_dir():
                print("No savepoints yet.")
                return 0
            for item in sorted(p for p in snapshots.iterdir() if p.is_dir()):
                try:
                    _, manifest = _load_manifest(root, item.name)
                    verified, errors = verify_snapshot(root, item.name)
                    state = "VERIFIED" if not errors else "DAMAGED"
                    print(f"{item.name}  {state}  {verified}/{manifest['file_count']}  {manifest.get('label','')}")
                except CLifeError as exc:
                    print(f"{item.name}  UNREADABLE  {exc}")
        elif args.command == "verify":
            snapshot_id = args.snapshot or latest_snapshot(root)
            verified, errors = verify_snapshot(root, snapshot_id)
            if errors:
                print(f"CLife verification FAILED: {snapshot_id}")
                for error in errors:
                    print(f"  {error}")
                return 2
            print(f"CLife verification OK: {snapshot_id} ({verified} files)")
        elif args.command == "status":
            snapshot_id = args.snapshot or latest_snapshot(root)
            changes = compare_snapshot(root, snapshot_id)
            print(f"CLife status against {snapshot_id}")
            for label in ("changed", "missing", "added", "same"):
                paths = changes[label]
                print(f"{label}: {len(paths)}")
                for path in paths[:30]:
                    print(f"  {path}")
                if len(paths) > 30:
                    print(f"  … and {len(paths) - 30} more")
        elif args.command in {"stitch", "revive"}:
            snapshot_id = args.snapshot or latest_snapshot(root)
            paths = args.path if args.command == "stitch" else None
            actions, safety_id = restore_snapshot(root, snapshot_id, paths, apply=args.apply)
            print(f"CLife {args.command}: {snapshot_id}")
            for action in actions:
                print(f"  {action}")
            if args.apply:
                print(f"Safety savepoint: {safety_id or 'not needed'}")
                print("Restoration applied. Extra files were left untouched.")
            else:
                print("Preview only. Add --apply to write; a pre-restore savepoint will be created first.")
        return 0
    except CLifeError as exc:
        print(f"CLife error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
