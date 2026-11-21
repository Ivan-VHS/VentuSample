#!/usr/bin/env python3
"""Inventory direct dependencies and assess lock/pinning hygiene without network access."""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any

SCHEMA = "repository-dependency-audit.v1"
LOCK_NAMES = {
    "poetry.lock", "uv.lock", "pdm.lock", "pipfile.lock", "package-lock.json",
    "npm-shrinkwrap.json", "pnpm-lock.yaml", "yarn.lock", "vcpkg-lock.json",
    "conan.lock", "Cargo.lock",
}
SKIP_PARTS = {".git", ".venv", "venv", "node_modules", "build", "dist", "vendor"}
REQ_RE = re.compile(r"^\s*([A-Za-z0-9_.-]+)\s*(.*)$")
CM_FIND = re.compile(r"(?im)^\s*find_package\s*\(\s*([A-Za-z0-9_.+-]+)([^)]*)\)")
CM_FETCH = re.compile(r"(?im)^\s*(?:FetchContent_Declare|ExternalProject_Add)\s*\(\s*([A-Za-z0-9_.+-]+)([^)]*)\)")
def _requirement(name: str, spec: str, source: str) -> dict[str, Any]:
    exact = bool(re.search(r"(?:==|===)\s*[A-Za-z0-9_.+-]+", spec))
    return {"name": name, "constraint": spec.strip() or "unspecified", "source": source, "exact_pin": exact}

def audit(root: Path) -> dict[str, Any]:
    items: list[dict[str, Any]] = []
    lockfiles: list[str] = []
    for path in root.rglob("*"):
        if not path.is_file() or any(part in SKIP_PARTS for part in path.relative_to(root).parts):
            continue
        rel = path.relative_to(root).as_posix()
        name = path.name.casefold()
        if name in LOCK_NAMES:
            lockfiles.append(rel)
        if name.startswith("requirements") and path.suffix == ".txt":
            try:
                for line in path.read_text(encoding="utf-8").splitlines():
                    line = line.strip()
                    if not line or line.startswith(("#", "-", "git+", "http:", "https:")):
                        continue
                    match = REQ_RE.match(line)
                    if match:
                        items.append(_requirement(match.group(1), match.group(2), rel))
            except (OSError, UnicodeDecodeError):
                pass
        elif name == "pyproject.toml":
            try:
                import tomllib
                data = tomllib.loads(path.read_text(encoding="utf-8"))
                project = data.get("project", {})
                for requirement in project.get("dependencies", []):
                    match = REQ_RE.match(requirement)
                    if match:
                        items.append(_requirement(match.group(1), match.group(2), rel))
                for group, requirements in project.get("optional-dependencies", {}).items():
                    for requirement in requirements:
                        match = REQ_RE.match(requirement)
                        if match:
                            items.append(_requirement(match.group(1), match.group(2), f"{rel} [{group}]"))
                for requirement in data.get("build-system", {}).get("requires", []):
                    match = REQ_RE.match(requirement)
                    if match:
                        items.append(_requirement(match.group(1), match.group(2), f"{rel} [build-system]"))
            except (OSError, UnicodeDecodeError, ValueError, ImportError):
                pass
        elif name == "package.json":
            try:
                data = json.loads(path.read_text(encoding="utf-8"))
                for section in ("dependencies", "devDependencies", "optionalDependencies"):
                    for dep, version in data.get(section, {}).items():
                        items.append(_requirement(dep, str(version), f"{rel} [{section}]"))
            except (OSError, UnicodeDecodeError, json.JSONDecodeError):
                pass
        elif name == "vcpkg.json":
            try:
                data = json.loads(path.read_text(encoding="utf-8"))
                for dep in data.get("dependencies", []):
                    if isinstance(dep, str):
                        items.append(_requirement(dep, "unspecified", rel))
                    elif isinstance(dep, dict):
                        items.append(_requirement(str(dep.get("name", "unknown")), str(dep.get("version>=", "")), rel))
            except (OSError, UnicodeDecodeError, json.JSONDecodeError):
                pass
        elif name == "conanfile.txt":
            try:
                active = False
                for line in path.read_text(encoding="utf-8").splitlines():
                    line = line.strip()
                    if line.startswith("["):
                        active = line.casefold() == "[requires]"
                    elif active and line and not line.startswith("#"):
                        parts = line.split("/", 1)
                        items.append(_requirement(parts[0], parts[1] if len(parts) == 2 else "unspecified", rel))
            except (OSError, UnicodeDecodeError):
                pass
        elif name == "cmakelists.txt":
            try:
                content = path.read_text(encoding="utf-8")
                for match in CM_FIND.finditer(content):
                    package, args = match.group(1), match.group(2).strip()
                    version = args.split()[0] if args and not args.startswith("REQUIRED") else "unspecified"
                    items.append(_requirement(package, version, rel + " [find_package]"))
                for match in CM_FETCH.finditer(content):
                    package, args = match.group(1), match.group(2)
                    pinned = bool(re.search(r"\bGIT_TAG\s+(?:[0-9a-f]{40}|v?\d+\.\d+(?:\.\d+)?)", args))
                    items.append({"name": package, "constraint": "commit/tag pinned" if pinned else "unversioned source", "source": rel + " [FetchContent]", "exact_pin": pinned})
            except (OSError, UnicodeDecodeError):
                pass
    # Stable de-duplication prevents CMake's repeated includes from inflating the inventory.
    unique: dict[tuple[str, str, str], dict[str, Any]] = {}
    for item in items:
        unique[(item["name"].casefold(), item["constraint"], item["source"])] = item
    items = sorted(unique.values(), key=lambda item: (item["name"].casefold(), item["source"]))
    unpinned = sum(1 for item in items if not item["exact_pin"])
    pin_score = 100 if not items else max(0, round(100 * (len(items) - unpinned) / len(items)))
    return {
        "schema": SCHEMA,
        "dependency_count": len(items),
        "unpinned_count": unpinned,
        "lockfiles": sorted(set(lockfiles)),
        "lockfile_present": bool(lockfiles),
        "pin_hygiene_score": pin_score,
        "dependencies": items,
        "limitations": "Offline inventory and pin-hygiene check only; it does not query a vulnerability database or prove packages are vulnerability-free.",
    }

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd())
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    report = audit(args.root.resolve())
    rendered = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
