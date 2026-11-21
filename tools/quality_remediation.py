"""Validate the repository's 20x3 correction/coverage/defect profile.

The evaluator checks structure, exact vocabulary and repository-local evidence paths.
It intentionally does not claim that file presence proves behavior correctness.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path, PurePosixPath, PureWindowsPath
from typing import Any, Mapping

SCHEMA = "anatidae.quality-remediation.v1"
CORRECTIONS = ["Исправление","Корректировка","Устранение","Поправка","Налаживание","Отладка","Доработка","Оптимизация","Реконструкция","Модернизация","Обновление","Ревизия","Восстановление","Регулировка","Совершенствование","Рефакторинг","Стабилизация","Согласование","Унификация","Безошибочность"]
COVERAGE = ["Всеобъемлемость","Всесторонность","Комплексность","Полнота","Исчерпывающесть","Универсальность","Многогранность","Масштабность","Охватность","Многоаспектность","Многопрофильность","Разносторонность","Всеохватность","Целостность","Совокупность","Глобальность","Безграничность","Исчерпывающесть","Фундаментальность","Энциклопедичность"]
DEFECTS = ["Ошибки","Погрешности","Неточности","Недочёты","Просчёты","Отклонения","Дефекты","Изъяны","Недостатки","Оплошности","Промахи","Сбои","Нарушения","Упущения","Несоответствия","Дисфункции","Неисправности","Оговорки","Ляпы","Деформации"]
EXPECTED_TERMS = tuple(zip(CORRECTIONS, COVERAGE, DEFECTS))


class QualityRemediationError(ValueError):
    """Raised when a remediation profile violates its contract."""


def _load(source: str | Path | Mapping[str, Any]) -> dict[str, Any]:
    if isinstance(source, Mapping):
        return dict(source)
    try:
        value = json.loads(Path(source).read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise QualityRemediationError(f"cannot load profile: {error}") from error
    if not isinstance(value, dict):
        raise QualityRemediationError("profile root must be a JSON object")
    return value


def _safe_evidence_path(root: Path, value: Any) -> Path:
    if not isinstance(value, str) or not value.strip():
        raise QualityRemediationError("evidence paths must be non-empty strings")
    posix = PurePosixPath(value)
    windows = PureWindowsPath(value)
    if (
        posix.is_absolute()
        or windows.is_absolute()
        or windows.drive
        or "\\" in value
        or any(part in {"", ".", ".."} for part in posix.parts)
    ):
        raise QualityRemediationError(f"unsafe evidence path: {value}")
    candidate = (root / Path(*posix.parts)).resolve()
    if not candidate.is_relative_to(root):
        raise QualityRemediationError(f"evidence path escapes repository root: {value}")
    return candidate


def assess_profile(
    source: str | Path | Mapping[str, Any], *, root: str | Path
) -> dict[str, Any]:
    profile = _load(source)
    repository_root = Path(root).resolve()
    if not repository_root.is_dir():
        raise QualityRemediationError(f"repository root is not a directory: {repository_root}")
    if profile.get("schema") != SCHEMA:
        raise QualityRemediationError(f"schema must be {SCHEMA!r}")

    project = profile.get("project")
    if not isinstance(project, dict) or not all(
        isinstance(project.get(key), str) and project[key].strip()
        for key in ("id", "name", "scope")
    ):
        raise QualityRemediationError("project must define non-empty id, name and scope")

    entries = profile.get("entries")
    if not isinstance(entries, list) or len(entries) != len(EXPECTED_TERMS):
        raise QualityRemediationError(f"entries must contain exactly {len(EXPECTED_TERMS)} rows")

    seen_ids: set[str] = set()
    references = 0
    found = 0
    missing: list[str] = []
    for index, entry in enumerate(entries):
        row = index + 1
        if not isinstance(entry, dict):
            raise QualityRemediationError(f"entry {row} must be an object")
        entry_id = entry.get("id")
        if not isinstance(entry_id, str) or not entry_id.strip():
            raise QualityRemediationError(f"entry {row} needs a non-empty id")
        if entry_id in seen_ids:
            raise QualityRemediationError(f"duplicate entry id: {entry_id}")
        seen_ids.add(entry_id)

        terms = entry.get("terms")
        expected = EXPECTED_TERMS[index]
        actual = (
            terms.get("correction"),
            terms.get("coverage"),
            terms.get("defect"),
        ) if isinstance(terms, dict) else None
        if actual != expected:
            raise QualityRemediationError(
                f"entry {row} terms must match the supplied triplet exactly"
            )
        for key in ("target", "acceptance"):
            if not isinstance(entry.get(key), str) or not entry[key].strip():
                raise QualityRemediationError(f"entry {row} needs non-empty {key}")

        evidence = entry.get("evidence")
        if not isinstance(evidence, list) or not evidence:
            raise QualityRemediationError(f"entry {row} needs at least one evidence path")
        for raw_path in evidence:
            candidate = _safe_evidence_path(repository_root, raw_path)
            references += 1
            if candidate.is_file():
                found += 1
            else:
                missing.append(str(raw_path))

    limitations = profile.get("limitations")
    if not isinstance(limitations, list) or not limitations or not all(
        isinstance(item, str) and item.strip() for item in limitations
    ):
        raise QualityRemediationError("limitations must be a non-empty list of statements")

    issues = [f"missing evidence file: {path}" for path in missing]
    return {
        "schema": SCHEMA,
        "project": {"id": project["id"], "name": project["name"]},
        "criteria_count": len(entries),
        "term_triplets_preserved": True,
        "evidence": {
            "references": references,
            "files_found": found,
            "files_missing": len(missing),
            "coverage_percent": round(100 * found / references, 1) if references else 100.0,
        },
        "issues": issues,
        "limitations": limitations,
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--profile", type=Path)
    parser.add_argument("--format", choices=("text", "json"), default="text")
    args = parser.parse_args(argv)
    profile_path = args.profile or args.root / "quality-remediation.v1.json"
    try:
        report = assess_profile(profile_path, root=args.root)
    except QualityRemediationError as error:
        print(f"ERROR {error}", file=sys.stderr)
        return 2

    if args.format == "json":
        print(json.dumps(report, ensure_ascii=False, indent=2))
    else:
        project = report["project"]["name"]
        evidence = report["evidence"]
        print(f"{project}: {report['criteria_count']}/20 remediation triplets validated")
        print(
            f"Evidence files: {evidence['files_found']}/{evidence['references']} "
            f"({evidence['coverage_percent']}% present)"
        )
        for issue in report["issues"]:
            print(f"ERROR {issue}", file=sys.stderr)
        if not report["issues"]:
            print("Profile structure, vocabulary and evidence paths are valid.")
        print("Note: file presence is not proof of behavioral correctness.")

    return 1 if report["issues"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
