#!/usr/bin/env python3
"""DuckStory: careful, read-only quality checks for an application repository."""
from __future__ import annotations

import argparse
import ast
import json
import os
import re
import tempfile
from dataclasses import asdict, dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

VERSION = "1.0.0"
ROOT = Path(__file__).resolve().parent.parent
CONFIG_PATH = ".duckstory/config.json"
MAX_FILE_BYTES = 1_000_000
MAX_SOURCE_LINES = 1_200
SKIP_DIRS = {
    ".git", ".venv", "venv", "__pycache__", "node_modules", "build",
    "dist", "out", "target", "cmake-build-debug", "cmake-build-release",
    ".pytest_cache", ".mypy_cache", ".ruff_cache", ".tox", "vendor",
}
SOURCE_SUFFIXES = {
    ".py", ".c", ".h", ".cc", ".cpp", ".cxx", ".hpp", ".hxx",
    ".js", ".jsx", ".ts", ".tsx", ".rs", ".go", ".java", ".cs",
    ".swift", ".kt", ".rpr",
}
TEST_NAME = re.compile(r"(^test_.*\.py$|.*_test\.(py|c|cc|cpp|h|hpp|js|ts|go|rs)$)", re.I)
ACTION_USE = re.compile(r"^\s*uses:\s*([^\s#]+)\s*$")
FULL_SHA = re.compile(r"^[0-9a-f]{40}$", re.I)
PILLARS = [
    ("security_privacy", "Безопасность и приватность"),
    ("reliability", "Надёжность и тестирование"),
    ("supply_chain", "Зависимости и цепочка поставки"),
    ("accessibility", "Доступность интерфейса"),
    ("performance", "Производительность и ресурсы"),
    ("compatibility", "Совместимость и переносимость"),
    ("maintainability", "Поддерживаемость и чистота кода"),
    ("product_quality", "Качество продукта и документация"),
]


@dataclass(frozen=True)
class Finding:
    code: str
    pillar: str
    severity: str
    title: str
    detail: str
    recommendation: str
    path: str | None = None


def add_finding(
    findings: list[Finding], root: Path, code: str, pillar: str,
    severity: str, title: str, detail: str, recommendation: str,
    path: Path | None = None,
) -> None:
    relative = path.relative_to(root).as_posix() if path else None
    findings.append(Finding(code, pillar, severity, title, detail, recommendation, relative))


def load_profile(root: Path, findings: list[Finding]) -> dict[str, Any]:
    path = root / CONFIG_PATH
    if not path.is_file():
        add_finding(
            findings, root, "DS-CONFIG-001", "product_quality", "warning",
            "Не найден профиль DuckStory", f"Отсутствует {CONFIG_PATH}; используется нейтральный профиль.",
            "Добавьте профиль проекта, чтобы уточнить стек и границы проверок.", path,
        )
        return {}
    try:
        profile = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        add_finding(
            findings, root, "DS-CONFIG-002", "product_quality", "error",
            "Профиль DuckStory не читается", str(error),
            "Исправьте JSON в .duckstory/config.json.", path,
        )
        return {}
    if not isinstance(profile, dict) or profile.get("schema") != "duckstory.config.v1":
        add_finding(
            findings, root, "DS-CONFIG-003", "product_quality", "error",
            "Неизвестная схема профиля", "Ожидается schema=duckstory.config.v1.",
            "Верните профиль к поддерживаемой схеме DuckStory.", path,
        )
        return {}
    return profile


def collect_files(root: Path) -> tuple[list[Path], int]:
    """Walk local files only; never follow symlinks or enter generated build trees."""
    files: list[Path] = []
    skipped_large = 0
    for folder, directories, names in os.walk(root, followlinks=False):
        current = Path(folder)
        directories[:] = sorted(
            name for name in directories
            if name not in SKIP_DIRS and not (current / name).is_symlink()
        )
        for name in sorted(names):
            path = current / name
            if path.is_symlink() or not path.is_file():
                continue
            try:
                if path.stat().st_size > MAX_FILE_BYTES:
                    skipped_large += 1
                    continue
            except OSError:
                continue
            files.append(path)
    return files, skipped_large


def detect_stacks(files: list[Path]) -> list[str]:
    names = {path.name.lower() for path in files}
    stacks = []
    if "cmakelists.txt" in names:
        stacks.append("C/C++ · CMake")
    if "pyproject.toml" in names or "setup.py" in names or any(p.suffix == ".py" for p in files):
        stacks.append("Python")
    if "package.json" in names:
        stacks.append("JavaScript / TypeScript")
    if "cargo.toml" in names:
        stacks.append("Rust")
    if "go.mod" in names:
        stacks.append("Go")
    if any(name.endswith((".csproj", ".sln")) for name in names):
        stacks.append(".NET")
    if "pom.xml" in names or "build.gradle" in names:
        stacks.append("Java")
    return stacks or ["Не удалось определить автоматически"]


def is_test_file(path: Path) -> bool:
    return "tests" in {part.lower() for part in path.parts} or bool(TEST_NAME.match(path.name))


def inspect_source(root: Path, files: list[Path], findings: list[Finding]) -> tuple[int, int, int]:
    python_count = json_count = source_count = 0
    for path in files:
        suffix = path.suffix.lower()
        if suffix not in SOURCE_SUFFIXES and suffix != ".json":
            continue
        try:
            content = path.read_text(encoding="utf-8")
        except (OSError, UnicodeError) as error:
            add_finding(
                findings, root, "DS-FILE-001", "maintainability", "warning",
                "Файл не удалось прочитать как UTF-8", str(error),
                "Проверьте кодировку; DuckStory не переписывает этот файл.", path,
            )
            continue
        if suffix == ".py":
            python_count += 1
            try:
                ast.parse(content, filename=str(path))
            except SyntaxError as error:
                add_finding(
                    findings, root, "DS-PY-001", "reliability", "error",
                    "Синтаксическая ошибка Python", f"Строка {error.lineno or '?'}: {error.msg}",
                    "Исправьте синтаксис и подтвердите результат штатными тестами.", path,
                )
        elif suffix == ".json":
            json_count += 1
            try:
                json.loads(content)
            except json.JSONDecodeError as error:
                add_finding(
                    findings, root, "DS-JSON-001", "reliability", "error",
                    "Некорректный JSON", f"Строка {error.lineno}, колонка {error.colno}: {error.msg}",
                    "Исправьте JSON и проверьте совместимость его схемы.", path,
                )
        if suffix in SOURCE_SUFFIXES:
            source_count += 1
            lines = content.count("\n") + bool(content)
            if lines > MAX_SOURCE_LINES:
                add_finding(
                    findings, root, "DS-CODE-001", "maintainability", "warning",
                    "Крупный исходный файл", f"В файле около {lines} строк; это повод для ревью, не ошибка.",
                    "Проверьте, можно ли разделить независимые обязанности без лишних абстракций.", path,
                )
    return python_count, json_count, source_count


def inspect_workflows(root: Path, files: list[Path], findings: list[Finding]) -> dict[str, int]:
    workflows = [
        path for path in files
        if path.parent.name == "workflows" and path.parent.parent.name == ".github"
        and path.suffix.lower() in {".yml", ".yaml"}
    ]
    unpinned = broad_permissions = missing_permissions = 0
    for path in workflows:
        try:
            lines = path.read_text(encoding="utf-8").splitlines()
        except (OSError, UnicodeError):
            continue
        if not any(re.match(r"^permissions\s*:", line) for line in lines):
            missing_permissions += 1
        for number, line in enumerate(lines, start=1):
            match = ACTION_USE.match(line)
            if match:
                action = match.group(1)
                if not action.startswith("./") and "@" in action:
                    revision = action.rsplit("@", 1)[1]
                    if not FULL_SHA.fullmatch(revision):
                        unpinned += 1
                        add_finding(
                            findings, root, "DS-SUPPLY-001", "supply_chain", "warning",
                            "GitHub Action не закреплён полным SHA",
                            f"{path.relative_to(root).as_posix()}:{number} использует {action}.",
                            "Закрепите проверенный релиз GitHub Action полным 40-символьным commit SHA.", path,
                        )
            compact = line.strip().lower()
            if re.match(r"^permissions\s*:\s*write-all\b", compact):
                broad_permissions += 1
            elif re.match(r"^contents\s*:\s*write\b", compact):
                broad_permissions += 1
    if broad_permissions:
        add_finding(
            findings, root, "DS-SEC-001", "security_privacy", "warning",
            "Workflow запрашивает права записи", f"Найдено потенциально расширенных разрешений: {broad_permissions}.",
            "Ограничьте token permissions минимальным набором для конкретной job.",
        )
    if missing_permissions and workflows:
        add_finding(
            findings, root, "DS-SEC-002", "security_privacy", "warning",
            "Не во всех workflow заданы явные permissions",
            f"Без явного блока permissions: {missing_permissions} из {len(workflows)}.",
            "Задайте минимальные permissions на уровне workflow или job.",
        )
    return {
        "workflow_count": len(workflows), "unpinned_actions": unpinned,
        "broad_permission_entries": broad_permissions,
        "workflows_without_permissions": missing_permissions,
    }


def audit(root: Path) -> dict[str, Any]:
    findings: list[Finding] = []
    profile = load_profile(root, findings)
    files, skipped_large = collect_files(root)
    python_count, json_count, source_count = inspect_source(root, files, findings)
    workflow = inspect_workflows(root, files, findings)
    relative_names = {path.relative_to(root).as_posix().lower() for path in files}
    basenames = {Path(name).name for name in relative_names}
    tests = [path for path in files if is_test_file(path)]

    checks = [
        (any(name.startswith("readme") for name in basenames), "DS-DOC-001",
         "product_quality", "Не найден README", "Добавьте описание продукта, установку, запуск и ограничения."),
        ("security.md" in basenames, "DS-SEC-003", "security_privacy",
         "Не найден SECURITY.md", "Добавьте порядок ответственного сообщения об уязвимостях."),
        (any(name.startswith("license") for name in basenames), "DS-DOC-002",
         "product_quality", "Не найден файл лицензии", "Уточните условия проекта и добавьте соответствующий файл."),
        (bool(tests), "DS-TEST-001", "reliability", "Не обнаружены тестовые файлы",
         "Добавьте проверки критических пользовательских сценариев и регрессии."),
        (workflow["workflow_count"] > 0, "DS-CI-001", "reliability",
         "Не обнаружен GitHub Actions workflow", "Подключите CI для сборки, статических проверок и тестов."),
    ]
    for passed, code, pillar, title, recommendation in checks:
        if not passed:
            add_finding(findings, root, code, pillar, "warning", title,
                        "Автоматический поиск не обнаружил ожидаемый проектный ресурс.", recommendation)

    for path in files:
        name = path.name.lower()
        sensitive_suffix = path.suffix.lower() in {".p12", ".pfx", ".key"}
        is_example = name.endswith((".example", ".example.key", ".example.p12", ".example.pfx"))
        if (name == ".env" or sensitive_suffix) and not is_example:
            add_finding(
                findings, root, "DS-SEC-004", "security_privacy", "warning",
                "Потенциально чувствительный файл в дереве проекта",
                "Проверено только имя файла; DuckStory не читает значения секретов.",
                "Проверьте, что в файле нет закрытого ключа или рабочего секрета и что он должен находиться в Git.", path,
            )

    if profile.get("ui", False):
        add_finding(
            findings, root, "DS-A11Y-001", "accessibility", "review",
            "Интерфейсу нужна ручная проверка доступности",
            "Статический анализ не подтверждает клавиатурную навигацию, фокус, контраст и работу со screen reader.",
            "Проведите проверку по применимым критериям WCAG 2.2 и нативным правилам платформ.",
        )
    add_finding(
        findings, root, "DS-PERF-001", "performance", "review",
        "Производительность требует измерений",
        "DuckStory не выводит скорость и потребление памяти из исходного кода.",
        "Добавьте воспроизводимые измерения времени, памяти и нагрузки для реальных сценариев.",
    )
    add_finding(
        findings, root, "DS-COMPAT-001", "compatibility", "review",
        "Совместимость требует проверки на целевых окружениях",
        "Указанная в документации платформа сама по себе не доказывает успешную сборку и запуск.",
        "Проверяйте сборку и тесты на каждой поддерживаемой ОС, архитектуре и версии runtime.",
    )

    order = {"error": 0, "warning": 1, "review": 2, "info": 3}
    findings.sort(key=lambda item: (order.get(item.severity, 9), item.pillar, item.code, item.path or ""))
    pillars = []
    for pillar_id, title in PILLARS:
        related = [item for item in findings if item.pillar == pillar_id]
        status = (
            "blocked" if any(item.severity == "error" for item in related)
            else "improve" if any(item.severity == "warning" for item in related)
            else "human_review" if any(item.severity == "review" for item in related)
            else "baseline"
        )
        pillars.append({"id": pillar_id, "title": title, "status": status})

    errors = sum(item.severity == "error" for item in findings)
    warnings = sum(item.severity == "warning" for item in findings)
    reviews = sum(item.severity == "review" for item in findings)
    overall = "blocked" if errors else "improvements_available" if warnings else "human_review_required" if reviews else "baseline_ready"
    return {
        "schema": "duckstory.report.v1",
        "tool": {"name": "DuckStory", "version": VERSION},
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "project": profile.get("project", root.name),
        "profile": profile.get("profile", "auto"),
        "detected_stacks": detect_stacks(files),
        "scope": "read_only_static_review",
        "limitations": [
            "Отчёт не является сертификатом соответствия стандартам.",
            "Сборка, runtime-поведение, безопасность зависимостей и доступность требуют профильных тестов.",
            "DuckStory не запускает команды проекта, не обращается к сети и не переписывает исходники.",
        ],
        "counts": {
            "files_scanned": len(files), "large_files_skipped": skipped_large,
            "python_files_parsed": python_count, "json_files_parsed": json_count,
            "source_files_seen": source_count, "test_files_found": len(tests),
            "findings_error": errors, "findings_warning": warnings, "manual_reviews": reviews,
        },
        "workflow": workflow, "pillars": pillars, "overall": overall,
        "findings": [asdict(item) for item in findings],
    }


def render_markdown(report: dict[str, Any]) -> str:
    counts = report["counts"]
    lines = [
        f"# DuckStory · {report['project']}", "",
        f"**Результат:** `{report['overall']}`  ",
        f"**Профиль:** `{report['profile']}`  ",
        f"**Стек:** {', '.join(report['detected_stacks'])}", "",
        "## Восемь направлений качества", "", "| Направление | Статус |", "|---|---|",
    ]
    lines.extend(f"| {pillar['title']} | `{pillar['status']}` |" for pillar in report["pillars"])
    lines.extend([
        "", "## Проверка", "",
        f"Файлов: {counts['files_scanned']} · Python: {counts['python_files_parsed']} · JSON: {counts['json_files_parsed']} · тестовых файлов: {counts['test_files_found']}",
        f"Ошибки: {counts['findings_error']} · рекомендации: {counts['findings_warning']} · ручные проверки: {counts['manual_reviews']}",
        "", "## Следующие улучшения", "",
    ])
    if not report["findings"]:
        lines.append("На этом проходе замечаний не обнаружено.")
    for item in report["findings"]:
        where = f" (`{item['path']}`)" if item.get("path") else ""
        lines.append(f"- **{item['severity'].upper()} · {item['code']} — {item['title']}**{where}")
        lines.append(f"  {item['detail']}")
        lines.append(f"  Следующий шаг: {item['recommendation']}")
    lines.extend(["", "> Статический отчёт помогает выбрать улучшения, но не подтверждает полное соответствие стандартам."])
    return "\n".join(lines) + "\n"


def write_report(path_value: str | None, content: str) -> None:
    if not path_value:
        return
    path = Path(path_value).expanduser()
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")


def safe_fix(root: Path) -> None:
    """Create a missing editor policy, but never overwrite an existing one."""
    path = root / ".editorconfig"
    if path.exists():
        print("DuckStory: .editorconfig уже существует; изменений нет.")
        return
    content = (
        "root = true\n\n[*]\ncharset = utf-8\nend_of_line = lf\ninsert_final_newline = true\n"
        "\n[*.py]\nindent_style = space\nindent_size = 4\n"
        "\n[*.{c,h,cc,cpp,cxx,hpp,hxx}]\nindent_style = space\nindent_size = 4\n"
        "\n[*.{json,yml,yaml,toml}]\nindent_style = space\nindent_size = 2\n"
        "\n[Makefile]\nindent_style = tab\n\n[*.md]\ntrim_trailing_whitespace = false\n"
    )
    path.write_text(content, encoding="utf-8", newline="\n")
    print("DuckStory: добавлен .editorconfig; исходный код не менялся.")


def self_test() -> None:
    """Check the pipeline in a disposable project without touching the repository."""
    with tempfile.TemporaryDirectory(prefix="duckstory-") as folder:
        root = Path(folder)
        (root / ".duckstory").mkdir()
        (root / ".duckstory/config.json").write_text(json.dumps({
            "schema": "duckstory.config.v1", "project": "DuckStory test",
            "profile": "test", "ui": False,
        }), encoding="utf-8")
        workflow_dir = root / ".github/workflows"
        workflow_dir.mkdir(parents=True)
        (workflow_dir / "ci.yml").write_text(
            "name: CI\npermissions:\n  contents: read\njobs: {}\n", encoding="utf-8"
        )
        (root / "tests").mkdir()
        (root / "tests/test_sample.py").write_text(
            "def test_smoke():\n    assert 2 + 2 == 4\n", encoding="utf-8"
        )
        for name in ("README.md", "SECURITY.md", "LICENSE"):
            (root / name).write_text("DuckStory test\n", encoding="utf-8")
        report = audit(root)
        assert report["schema"] == "duckstory.report.v1"
        assert len(report["pillars"]) == 8
        assert report["counts"]["python_files_parsed"] == 1
        assert report["counts"]["findings_error"] == 0
    print(f"DuckStory {VERSION}: self-test passed.")


def main() -> int:
    parser = argparse.ArgumentParser(description="DuckStory — бережная система улучшения качества проекта.")
    commands = parser.add_subparsers(dest="command", required=True)
    audit_command = commands.add_parser("audit", help="аудит без изменения прикладного кода")
    audit_command.add_argument("--root", type=Path, default=ROOT)
    audit_command.add_argument("--json", dest="json_path")
    audit_command.add_argument("--markdown", dest="markdown_path")
    audit_command.add_argument("--summary", dest="summary_path")
    audit_command.add_argument("--fail-on", choices=("none", "warning", "error"), default="none")
    fix_command = commands.add_parser("safe-fix", help="добавить .editorconfig, если его ещё нет")
    fix_command.add_argument("--root", type=Path, default=ROOT)
    commands.add_parser("self-test", help="проверить DuckStory во временной папке")
    args = parser.parse_args()

    if args.command == "self-test":
        self_test()
        return 0
    if args.command == "safe-fix":
        safe_fix(args.root.resolve())
        return 0

    root = args.root.resolve()
    if not root.is_dir():
        parser.error(f"Папка проекта не найдена: {root}")
    report = audit(root)
    json_text = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    markdown = render_markdown(report)
    write_report(args.json_path, json_text)
    write_report(args.markdown_path, markdown)
    write_report(args.summary_path, markdown)
    print(markdown)
    if args.fail_on == "error" and report["counts"]["findings_error"]:
        return 2
    if args.fail_on == "warning" and (report["counts"]["findings_error"] or report["counts"]["findings_warning"]):
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
