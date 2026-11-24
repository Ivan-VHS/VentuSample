#!/usr/bin/env python3
"""Beelzebul 66-rule repository scanner. Standard library only; read-only."""
from __future__ import annotations

import argparse
import ast
import json
import math
import os
import re
import subprocess
import sys
import unicodedata
from collections import Counter, defaultdict
from dataclasses import asdict, dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
POLICY_PATH = ROOT / "security" / "beelzebul.v1.json"
TEXT_SUFFIXES = {
    ".py", ".pyi", ".c", ".h", ".cc", ".cpp", ".cxx", ".hpp", ".hxx",
    ".js", ".jsx", ".ts", ".tsx", ".mjs", ".cjs", ".java", ".kt", ".rs",
    ".go", ".cs", ".swift", ".sh", ".ps1", ".bat", ".cmd", ".yml", ".yaml",
    ".json", ".jsonc", ".tsv", ".toml", ".ini", ".cfg", ".conf", ".xml", ".html",
    ".css", ".scss", ".md", ".mdx", ".rst", ".txt", ".properties", ".cmake",
    ".gradle", ".sql", ".graphql", ".proto", ".qcs", ".rpr", ".lua", ".make",
}
TEXT_NAMES = {
    ".env", ".env.example", ".gitignore", ".gitattributes", ".editorconfig",
    "Dockerfile", "Makefile", "CMakeLists.txt", "requirements.txt",
    "CODEOWNERS", "LICENSE", "SECURITY.md",
}
SOURCE_SUFFIXES = {
    ".py", ".pyi", ".c", ".h", ".cc", ".cpp", ".cxx", ".hpp", ".hxx",
    ".js", ".jsx", ".ts", ".tsx", ".mjs", ".cjs", ".java", ".kt", ".rs",
    ".go", ".cs", ".swift", ".sh", ".ps1", ".sql", ".lua",
}
SOURCE_DIRS = {"src", "include", "reaper", "oplague", "language", "selfhost"}
NEVER_SUPPRESS = {"A-02", "A-04", "A-06", "A-10", "A-13", "Y-04", "Q-13", "O-02"}
SEVERITY_WEIGHT = {"critical": 25, "high": 12, "medium": 5, "low": 2, "info": 1}


class PolicyError(ValueError):
    """Invalid or internally inconsistent security policy."""


@dataclass(frozen=True)
class Finding:
    rule_id: str
    severity: str
    enforcement: str
    path: str
    line: int | None
    message: str
    remediation: str

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


def _unique_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise PolicyError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def _load_json(path: Path) -> Any:
    try:
        return json.loads(
            path.read_text(encoding="utf-8"),
            object_pairs_hook=_unique_pairs,
            parse_constant=lambda value: (_ for _ in ()).throw(
                PolicyError(f"non-finite JSON constant: {value}")
            ),
        )
    except OSError as exc:
        raise PolicyError(f"cannot read {path}: {exc}") from exc
    except json.JSONDecodeError as exc:
        raise PolicyError(f"invalid JSON policy: {exc}") from exc


def _validate_exceptions(policy: dict[str, Any]) -> None:
    now = datetime.now(timezone.utc)
    known = {rule["id"]: rule for rule in policy["rules"]}
    seen: set[tuple[str, str]] = set()
    for item in policy.get("exceptions", []):
        if not isinstance(item, dict):
            raise PolicyError("each exception must be an object")
        rule_id, rel = item.get("rule_id"), item.get("path")
        if rule_id not in known or not isinstance(rel, str) or not rel:
            raise PolicyError("exception must name a known rule and exact relative path")
        path = Path(rel)
        if path.is_absolute() or ".." in path.parts or "*" in rel:
            raise PolicyError("exception path must be exact, relative and traversal-free")
        if rule_id in NEVER_SUPPRESS or known[rule_id]["severity"] == "critical":
            raise PolicyError(f"critical baseline rule {rule_id} cannot be suppressed")
        key = (rule_id, rel)
        if key in seen:
            raise PolicyError(f"duplicate exception for {rule_id}:{rel}")
        seen.add(key)
        for field in ("reason", "owner", "expires_at"):
            if not isinstance(item.get(field), str) or not item[field].strip():
                raise PolicyError(f"exception {rule_id}:{rel} needs {field}")
        try:
            expiry = datetime.fromisoformat(item["expires_at"].replace("Z", "+00:00"))
        except ValueError as exc:
            raise PolicyError(f"invalid exception expiry for {rule_id}:{rel}") from exc
        if expiry.tzinfo is None:
            raise PolicyError(f"exception expiry must include timezone for {rule_id}:{rel}")
        if expiry.astimezone(timezone.utc) <= now:
            raise PolicyError(f"expired exception for {rule_id}:{rel}")


def load_policy(path: Path = POLICY_PATH) -> dict[str, Any]:
    policy = _load_json(path)
    if not isinstance(policy, dict) or policy.get("schema") != "beelzebul.security.v1":
        raise PolicyError("unsupported Beelzebul policy schema")
    rules = policy.get("rules")
    if policy.get("rule_count") != 66 or not isinstance(rules, list) or len(rules) != 66:
        raise PolicyError("Beelzebul policy must contain exactly 66 rules")
    expected = {"atomic": 15, "atom": 15, "nuclear": 15, "quantum": 15, "general": 6}
    if dict(Counter(rule.get("level") for rule in rules)) != expected:
        raise PolicyError("invalid 15+15+15+15+6 rule distribution")
    ids = [rule.get("id") for rule in rules]
    numbers = [rule.get("number") for rule in rules]
    if len(set(ids)) != 66 or numbers != list(range(1, 67)):
        raise PolicyError("rule IDs must be unique and numbers ordered 1..66")
    for rule in rules:
        if rule.get("severity") not in SEVERITY_WEIGHT:
            raise PolicyError(f"invalid severity in {rule.get('id')}")
        if rule.get("enforcement") not in {"block", "review", "governance", "observe"}:
            raise PolicyError(f"invalid enforcement in {rule.get('id')}")
    fail_on = policy.get("policy", {}).get("fail_on", ["critical"])
    if not isinstance(fail_on, list) or any(value not in SEVERITY_WEIGHT for value in fail_on):
        raise PolicyError("policy.fail_on must contain valid severity names")
    _validate_exceptions(policy)
    return policy


def _tracked_paths(root: Path, excluded: set[str], max_files: int) -> list[Path]:
    result = subprocess.run(
        ["git", "ls-files", "-z"], cwd=root, stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL, check=False,
    )
    if result.returncode == 0 and result.stdout:
        relative_paths = [Path(item.decode("utf-8", "surrogateescape"))
                          for item in result.stdout.split(b"\0") if item]
    else:
        relative_paths: list[Path] = []
        for current, dirs, files in os.walk(root, followlinks=False):
            dirs[:] = sorted(d for d in dirs if d not in excluded and not (Path(current) / d).is_symlink())
            for name in files:
                relative_paths.append((Path(current) / name).relative_to(root))
    selected = []
    for rel in relative_paths:
        if any(part in excluded for part in rel.parts) or rel.as_posix().startswith(".git/"):
            continue
        selected.append(rel)
        if len(selected) > max_files:
            raise PolicyError(f"workspace exceeds configured {max_files}-file scan limit")
    return sorted(selected, key=lambda item: item.as_posix())


def _is_text_path(path: Path) -> bool:
    return path.suffix.lower() in TEXT_SUFFIXES or path.name in TEXT_NAMES or not path.suffix


def _in_source_tree(path: Path) -> bool:
    return any(part.lower() in SOURCE_DIRS for part in path.parts[:-1])


def _entropy(value: str) -> float:
    counts = Counter(value)
    size = len(value)
    return -sum((count / size) * math.log2(count / size) for count in counts.values()) if size else 0.0


def _line_for(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def _attribute_chain(node: ast.AST) -> str:
    parts = []
    current = node
    while isinstance(current, ast.Attribute):
        parts.append(current.attr)
        current = current.value
    if isinstance(current, ast.Name):
        parts.append(current.id)
    return ".".join(reversed(parts))


def _inspect_python(tree: ast.AST, path: str, add: Any) -> None:
    managed_open_lines: set[int] = set()
    for node in ast.walk(tree):
        if isinstance(node, (ast.With, ast.AsyncWith)):
            for child in ast.walk(node):
                if isinstance(child, ast.Call) and (
                    (isinstance(child.func, ast.Name) and child.func.id == "open")
                    or (isinstance(child.func, ast.Attribute) and child.func.attr == "open")
                ):
                    managed_open_lines.add(child.lineno)

    for node in ast.walk(tree):
        if isinstance(node, ast.Call):
            func = node.func
            name = func.id if isinstance(func, ast.Name) else (func.attr if isinstance(func, ast.Attribute) else "")
            chain = _attribute_chain(func)
            if name in {"eval", "exec"}:
                add("M-02", path, node.lineno, f"dynamic execution call {name} requires review")
            if chain in {"pickle.loads", "pickle.load", "marshal.loads", "shelve.open"}:
                add("M-02", path, node.lineno, f"general-purpose deserialization API {chain} requires review")
            if chain in {"yaml.load", "yaml.unsafe_load"}:
                safe = any(k.arg == "Loader" and isinstance(k.value, ast.Name) and k.value.id == "SafeLoader" for k in node.keywords)
                if not safe:
                    add("M-02", path, node.lineno, "yaml.load without an explicit safe loader")
            if name in {"run", "Popen", "call", "check_call", "check_output"} and any(
                k.arg == "shell" and isinstance(k.value, ast.Constant) and k.value.value is True for k in node.keywords
            ):
                add("M-10", path, node.lineno, "subprocess call enables shell=True")
            if chain in {"os.system", "os.popen"}:
                add("M-10", path, node.lineno, f"shell API {chain} requires parameterized replacement")
            if name == "extractall":
                add("M-09", path, node.lineno, "archive extraction requires validated destination containment")
            if chain in {"hashlib.md5", "hashlib.sha1"}:
                add("M-13", path, node.lineno, f"weak hash primitive {chain} requires security-purpose review")
            if name == "open" and node.lineno not in managed_open_lines:
                add("M-15", path, node.lineno, "open() is not visibly managed by a context manager")
            if name == "execute" and node.args and isinstance(node.args[0], (ast.JoinedStr, ast.BinOp)):
                add("M-10", path, node.lineno, "execute() uses dynamically assembled query text")
        elif isinstance(node, ast.ExceptHandler):
            broad = node.type is None or (isinstance(node.type, ast.Name) and node.type.id in {"Exception", "BaseException"})
            if broad and any(isinstance(child, ast.Pass) for child in node.body):
                add("M-08", path, node.lineno, "broad exception handler silently passes")
        elif isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)):
            branches = sum(isinstance(child, (ast.If, ast.For, ast.AsyncFor, ast.While, ast.Try, ast.Match, ast.ExceptHandler)) for child in ast.walk(node))
            lines = (getattr(node, "end_lineno", node.lineno) or node.lineno) - node.lineno + 1
            if branches > 20 or lines > 120:
                add("M-11", path, node.lineno, f"complexity indicator: {branches} branches across {lines} lines")


def _report_import_cycles(graph: dict[str, set[str]], modules: set[str], add: Any) -> None:
    adjacency: dict[str, set[str]] = {}
    for module, imports in graph.items():
        adjacency[module] = set()
        for imported in imports:
            candidate = imported
            while candidate:
                if candidate in modules:
                    adjacency[module].add(candidate)
                    break
                candidate = candidate.rsplit(".", 1)[0] if "." in candidate else ""
    state: dict[str, int] = {}
    stack: list[str] = []
    emitted: set[tuple[str, ...]] = set()

    def visit(node: str) -> None:
        state[node] = 1
        stack.append(node)
        for target in sorted(adjacency.get(node, set())):
            if state.get(target, 0) == 0:
                visit(target)
            elif state.get(target) == 1:
                cycle = tuple(sorted(stack[stack.index(target):]))
                if cycle not in emitted:
                    emitted.add(cycle)
                    add("M-04", "<module-graph>", None, "possible Python import cycle: " + " -> ".join(cycle))
        stack.pop()
        state[node] = 2

    for module in sorted(adjacency):
        if state.get(module, 0) == 0:
            visit(module)


def _git_changed_files(root: Path, base_ref: str | None) -> list[str]:
    if not base_ref:
        return []
    result = subprocess.run(
        ["git", "diff", "--name-only", "--diff-filter=ACMR", f"{base_ref}...HEAD"],
        cwd=root, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
        text=True, check=False,
    )
    return sorted(line.strip() for line in result.stdout.splitlines() if line.strip()) if result.returncode == 0 else []


def _path_risk(path: str) -> int:
    lower = path.lower()
    if lower.startswith((".github/workflows/", "security/")):
        return 6
    if any(word in lower for word in ("dependency", "lock", "requirements", "pyproject", "package.json")):
        return 4
    if lower.startswith(("src/", "include/")):
        return 3
    return 1


def scan_repository(root: Path, policy_path: Path, base_ref: str | None = None) -> dict[str, Any]:
    root = root.resolve()
    policy = load_policy(policy_path)
    rules = {rule["id"]: rule for rule in policy["rules"]}
    limits = policy["limits"]
    paths = _tracked_paths(root, set(policy["excluded_directories"]), int(limits["max_files"]))
    findings: list[Finding] = []
    seen: set[tuple[str, str, int | None, str]] = set()
    changed_files = _git_changed_files(root, base_ref)
    changed_set = set(changed_files)
    file_count = text_count = total_bytes = 0
    py_import_graph: dict[str, set[str]] = defaultdict(set)
    py_modules: set[str] = set()
    workflows: dict[str, str] = {}

    def add(rule_id: str, path: str, line: int | None, message: str) -> None:
        for exception in policy.get("exceptions", []):
            if exception["rule_id"] == rule_id and exception["path"] == path:
                return
        item_key = (rule_id, path, line, message)
        if item_key in seen:
            return
        seen.add(item_key)
        rule = rules[rule_id]
        findings.append(Finding(rule_id, rule["severity"], rule["enforcement"], path, line, message, rule["remediation"]))

    # NFC/case-insensitive path collisions and reserved Windows device names.
    normalized: dict[str, str] = {}
    reserved = {"CON", "PRN", "AUX", "NUL"} | {f"COM{i}" for i in range(1, 10)} | {f"LPT{i}" for i in range(1, 10)}
    for rel in paths:
        name = rel.as_posix()
        key = unicodedata.normalize("NFC", name).casefold()
        previous = normalized.get(key)
        if previous is not None and previous != name:
            add("A-13", name, None, f"path collides with {previous} after casefold/NFC normalization")
        else:
            normalized[key] = name
        if any(part.split(".", 1)[0].upper() in reserved for part in rel.parts):
            add("A-13", name, None, "reserved Windows device-name component in path")

    # Build signature fragments without embedding a complete credential example in this scanner.
    credential_prefix = re.compile(
        r"(?:AK" + r"IA[0-9A-Z]{16}|gh[pousr]_[A-Za-z0-9]{20,}|"
        r"xox[baprs]-[A-Za-z0-9-]{16,}|-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----)"
    )
    assignment_secret = re.compile(
        r"""(?i)\b(?:api[_-]?key|access[_-]?token|client[_-]?secret|password|secret)\s*[:=]\s*["'][^"']{12,}["']"""
    )
    entropy_token = re.compile(r"[A-Za-z0-9_+/=-]{40,}")
    hidden_codes = {0x061C, 0x200B, 0x200C, 0x200D, 0x200E, 0x200F, 0x202A, 0x202B, 0x202C, 0x202D, 0x202E, 0x2066, 0x2067, 0x2068, 0x2069}
    conflict_marker = re.compile(r"^\s*(?:<{7}|>{7})(?:\s|$)|^\s*={7}\s*$")
    suspicious_regex = re.compile(r"\([^)\n]{0,200}[+*][^)\n]{0,200}\)[+*{]")

    dependency_change = False
    for rel in paths:
        name = rel.as_posix()
        file_path = root / rel
        try:
            if file_path.is_symlink():
                add("M-09", name, None, "tracked symbolic link requires path-boundary review")
                continue
            resolved = file_path.resolve(strict=True)
            if not resolved.is_relative_to(root):
                add("M-09", name, None, "path resolves outside repository root")
                continue
            size = file_path.stat().st_size
            file_count += 1
            total_bytes += size
            if total_bytes > int(limits["max_total_bytes"]):
                raise PolicyError("workspace exceeds configured total byte scan limit")
            if size > int(limits["max_file_bytes"]):
                continue
            if not _is_text_path(rel):
                if _in_source_tree(rel):
                    sample = file_path.read_bytes()[:8192]
                    if b"\0" in sample:
                        add("A-11", name, None, "binary content detected inside a source directory")
                continue
            raw = file_path.read_bytes()
        except (OSError, RuntimeError) as exc:
            add("A-04", name, None, f"file cannot be inspected safely: {type(exc).__name__}")
            continue

        if b"\0" in raw:
            add("A-11" if _in_source_tree(rel) else "A-04", name, None, "NUL byte detected in a text/source file")
            continue
        try:
            text = raw.decode("utf-8", errors="strict")
        except UnicodeDecodeError as exc:
            add("A-04", name, exc.start + 1, "text file is not valid UTF-8")
            continue
        text_count += 1

        crlf = raw.count(b"\r\n")
        lone_lf = raw.count(b"\n") - crlf
        lone_cr = raw.count(b"\r") - crlf
        if (crlf and lone_lf) or (crlf and lone_cr) or (lone_lf and lone_cr):
            add("A-09", name, None, "mixed CRLF/LF/CR line endings")

        for pos, char in enumerate(text):
            codepoint = ord(char)
            if codepoint in hidden_codes or (codepoint == 0xFEFF and pos > 0):
                add("A-06", name, text.count("\n", 0, pos) + 1, f"hidden direction/zero-width character U+{codepoint:04X}")
                break
            if (codepoint < 32 and char not in "\n\r\t") or codepoint == 127:
                add("A-04", name, text.count("\n", 0, pos) + 1, f"unexpected control byte U+{codepoint:04X}")
                break

        for line_no, line in enumerate(text.splitlines(), 1):
            if conflict_marker.match(line):
                add("A-10", name, line_no, "unresolved merge-conflict marker")
            if len(line) > int(limits["max_line_length"]) and not rel.name.endswith((".min.js", ".min.css")):
                add("A-12", name, line_no, "line exceeds configured length limit")
            if credential_prefix.search(line):
                add("A-02", name, line_no, "known credential prefix or private-key header; value redacted")
            if assignment_secret.search(line):
                add("A-01", name, line_no, "credential-like assignment; value redacted")
            for match in entropy_token.finditer(line):
                token = match.group(0)
                if token.isdigit() or re.fullmatch(r"[0-9a-fA-F]{40,}", token):
                    continue
                context = line[max(0, match.start() - 50):match.start()].lower()
                if _entropy(token) >= float(limits["entropy_threshold"]) and any(
                    word in context for word in ("token", "secret", "key", "password", "credential")
                ):
                    add("A-03", name, line_no, "high-entropy credential-like string; value redacted")
            if suspicious_regex.search(line) and rel.suffix.lower() not in {".pyc"}:
                add("A-07", name, line_no, "nested regex quantifier may cause excessive backtracking")

        if name.startswith(".github/workflows/") and rel.suffix.lower() in {".yml", ".yaml"}:
            workflows[name] = text

        if rel.name.lower() in {"requirements.txt", "pyproject.toml", "package.json", "cargo.toml", "go.mod"} or "lock" in rel.name.lower():
            if name in changed_set:
                dependency_change = True

        if rel.suffix.lower() in SOURCE_SUFFIXES:
            if rel.suffix.lower() in {".c", ".h", ".cc", ".cpp", ".hpp", ".js", ".ts", ".rs", ".go"}:
                for line_no, line in enumerate(text.splitlines(), 1):
                    if re.search(r"\b(?:system|popen|strcpy|strcat|gets)\s*\(", line):
                        add("M-02", name, line_no, "potentially dangerous native/system API requires review")
            if rel.suffix.lower() in {".py", ".pyi"}:
                try:
                    tree = ast.parse(text, filename=name)
                except SyntaxError as exc:
                    add("M-03", name, exc.lineno, "Python source syntax error at module boundary")
                    continue
                module = ".".join(rel.with_suffix("").parts)
                if module.endswith(".__init__"):
                    module = module[:-9]
                py_modules.add(module)
                _inspect_python(tree, name, add)
                for node in ast.walk(tree):
                    if isinstance(node, ast.Import):
                        py_import_graph[module].update(alias.name for alias in node.names)
                    elif isinstance(node, ast.ImportFrom) and node.module:
                        if node.level:
                            package = module.split(".")[:-1]
                            if rel.name == "__init__.py":
                                package = module.split(".")
                            package = package[:max(0, len(package) - node.level + 1)]
                            target = ".".join(package + node.module.split("."))
                        else:
                            target = node.module
                        py_import_graph[module].add(target)

    for rel, text in workflows.items():
        lower = text.lower()
        if re.search(r"(?m)^\s*permissions\s*:\s*write-all\s*$", text, re.I):
            add("Y-04", rel, None, "workflow grants write-all permissions")
        if "pull_request_target:" in lower:
            # Metadata-only use of PR fields is not the same as executing PR code.
            # Flag only a checkout explicitly pinned to the contributor's head, or a
            # run expression that directly interpolates a PR-controlled head value.
            checkout_of_head = bool(re.search(
                r"(?is)uses\s*:\s*actions/checkout@[^\n]+\n(?:(?:[ \t]+[^\n]*\n){0,12})[ \t]+ref\s*:\s*[^\n#]*(?:pull_request\.head|head\.sha)",
                text,
            ))
            run_interpolation = bool(re.search(
                r"(?im)^\s*run\s*:\s*.*\$\{\{\s*github\.event\.pull_request\.head\.",
                text,
            ))
            if checkout_of_head or run_interpolation:
                add("Y-04", rel, None, "pull_request_target executes or checks out contributor-controlled head content")
            elif re.search(r"(?im)^\s*permissions\s*:", text) and re.search(r"(?im)^\s*contents\s*:\s*write\s*$", text):
                add("Y-07", rel, None, "pull_request_target uses write permissions; review metadata-only handling and token scope")
        if re.search(r"(?im)^\s*run\s*:\s*.*\$\{\{\s*secrets\.", text):
            add("Y-05", rel, None, "secret context interpolated directly into shell run expression")
        if re.search(r"(?im)^\s*continue-on-error\s*:\s*true\s*$", text):
            add("Y-14", rel, None, "workflow step can fail without failing its job")
        if re.search(r"(?im)^\s*uses\s*:\s*[^#\n]+@(main|master|latest)\s*$", text):
            add("Y-07", rel, None, "workflow action follows a mutable branch/tag reference")
        if "timeout-minutes:" not in lower:
            add("Y-13", rel, None, "workflow has no explicit timeout-minutes")
        if "actions/cache" in lower and "restore-keys:" in lower and "github.event.pull_request.head.sha" not in lower:
            add("Y-10", rel, None, "shared cache restore scope needs fork/branch isolation review")

    _report_import_cycles(py_import_graph, py_modules, add)
    if len(changed_files) > int(limits["large_change_files"]):
        add("Y-12", "<pull-request>", None, f"pull request changes {len(changed_files)} files; additional review required")
    if dependency_change:
        add("Q-02", "<pull-request>", None, "dependency manifest/lock file changed; review transitive and mirrored components")

    # Risk is not a count of grep matches. Cap each rule at its strongest
    # observed severity, then add a small path-exposure and change-volume modifier.
    strongest_by_rule: dict[str, int] = {}
    for finding in findings:
        strongest_by_rule[finding.rule_id] = max(
            strongest_by_rule.get(finding.rule_id, 0),
            SEVERITY_WEIGHT[finding.severity],
        )
    sensitive_path_bonus = min(12, len({
        finding.path for finding in findings
        if finding.path.startswith((".github/workflows/", "security/"))
    }) * 2)
    risk = sum(strongest_by_rule.values()) + sensitive_path_bonus
    if changed_files:
        risk += min(10, len(changed_files) // 10)
    risk = min(int(limits["max_risk_score"]), risk)
    fail_on = set(policy.get("policy", {}).get("fail_on", ["critical"]))
    blocked = [f for f in findings if f.enforcement == "block" or f.severity in fail_on]
    review = [f for f in findings if f.enforcement in {"review", "governance"}]
    verdict = "blocked" if blocked else ("review" if review else "clear")
    return {
        "schema": "beelzebul.report.v1",
        "scanner": {"name": "Веельзевул", "version": policy["version"], "policy": "security/beelzebul.v1.json"},
        "repository": root.name, "base_ref": base_ref,
        "scan": {"files_seen": file_count, "text_files_scanned": text_count, "tracked_paths": len(paths), "changed_files": len(changed_files), "read_only": True},
        "policy": {"rules_total": 66, "levels": dict(Counter(rule["level"] for rule in policy["rules"]))},
        "verdict": verdict, "risk_score": risk, "findings_count": len(findings),
        "severity_counts": dict(Counter(item.severity for item in findings)),
        "findings": [item.to_dict() for item in sorted(findings, key=lambda f: (f.severity, f.path, f.line or 0, f.rule_id))],
        "limits": {key: limits[key] for key in ("max_files", "max_file_bytes", "max_total_bytes")},
        "notes": [
            "Static heuristics indicate review targets; a clean scan is not a security certification.",
            "Branch protection, authorship, roles, release signatures and audit logs require GitHub governance checks.",
            "Quantum-level correlation is approximated with PR/dependency indicators; no quantum computation or author-intent inference is performed.",
            "Secret values are redacted from findings.",
        ],
    }


def format_text(report: dict[str, Any]) -> str:
    lines = [
        f"Веельзевул {report['scanner']['version']} — {report['repository']}",
        f"Verdict: {report['verdict'].upper()} | risk score: {report['risk_score']}/100",
        f"Scanned: {report['scan']['text_files_scanned']} text files / {report['scan']['tracked_paths']} tracked paths",
        f"Rules: {report['policy']['rules_total']} | findings: {report['findings_count']}",
    ]
    if not report["findings"]:
        lines.append("No automated findings.")
    for item in report["findings"]:
        loc = item["path"] + (f":{item['line']}" if item["line"] else "")
        lines.append(f"[{item['severity'].upper()} / {item['enforcement'].upper()}] {item['rule_id']} {loc}: {item['message']}")
        lines.append(f"  Fix: {item['remediation']}")
    lines.append("A clean scan is not a security certification; see SECURITY.md for governance controls.")
    return "\n".join(lines)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Read-only 66-rule Beelzebul repository security scanner.")
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--policy", type=Path)
    parser.add_argument("--base-ref", help="base ref for PR change-volume/dependency correlation checks")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--output", type=Path, help="write JSON report to a separate path")
    parser.add_argument("--strict-review", action="store_true", help="return 1 when review findings exist")
    args = parser.parse_args(argv)
    policy_path = args.policy or (args.root.resolve() / "security" / "beelzebul.v1.json")
    try:
        report = scan_repository(args.root, policy_path, args.base_ref)
    except (PolicyError, OSError) as exc:
        print(f"BEELZEBUL POLICY ERROR: {exc}", file=sys.stderr)
        return 2
    encoded = json.dumps(report, ensure_ascii=False, indent=2)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded + "\n", encoding="utf-8")
    print(encoded if args.json else format_text(report))
    if report["verdict"] == "blocked":
        return 2
    if args.strict_review and report["verdict"] == "review":
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
