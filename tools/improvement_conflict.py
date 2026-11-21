"""Validate the repository's 20-control improvement/conflict governance profile."""
from __future__ import annotations
import argparse
import json
import sys
from pathlib import Path, PurePosixPath
from typing import Any, Mapping

SCHEMA = "anatidae.improvement-conflict.v1"
ALLOWED_STATES = ("not_assessed", "detected", "root_cause_confirmed", "remediated", "verified", "deferred")
EXPECTED_TERMS = [["Совершенствование","Дисгармония","Конфликты"],["Преображение","Разлад","Противоречия"],["Оптимизация","Несозвучие","Столкновения"],["Модернизация","Дисбаланс","Несовместимости"],["Улучшение","Диспропорция","Разногласия"],["Развитие","Несогласованность","Расхождения"],["Преимущественное развитие","Разноречие","Коллизии"],["Усовершенствование","Противоречивость","Споры"],["Реконструкция","Какофония","Несогласованности"],["Рефакторинг","Дискретность","Противоборства"],["Переосмысление","Разобщённость","Соперничества"],["Обновление","Неоднородность","Раздвоения"],["Преобразование","Несоответствие","Пересечения"],["Обогащение","Расхождение","Накладки"],["Усиление","Разноголосица","Неувязки"],["Доработка","Дисгармоничность","Диссонансы"],["Возвышение","Конфликтность","Антагонизмы"],["Прогрессирование","Неслаженность","Несоответствия"],["Эволюционирование","Раздвоенность","Разобщённости"],["Комплексное совершенствование","Поляризация","Раздоры"]]

class ImprovementConflictProfileError(ValueError):
    """Raised when an improvement/conflict profile violates its contract."""


def _load(source: str | Path | Mapping[str, Any]) -> dict[str, Any]:
    if isinstance(source, Mapping):
        return dict(source)
    try:
        value = json.loads(Path(source).read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ImprovementConflictProfileError(f"cannot load profile: {error}") from error
    if not isinstance(value, dict):
        raise ImprovementConflictProfileError("profile root must be an object")
    return value


def _safe_evidence_path(raw: Any) -> PurePosixPath:
    if not isinstance(raw, str) or not raw or "\\" in raw or "\x00" in raw:
        raise ImprovementConflictProfileError("evidence paths must be non-empty repository-relative POSIX paths")
    path = PurePosixPath(raw)
    if path.is_absolute() or any(part in {"", ".", ".."} for part in path.parts):
        raise ImprovementConflictProfileError(f"unsafe evidence path: {raw!r}")
    return path


def validate_profile(source: str | Path | Mapping[str, Any], *, root: str | Path | None = None) -> dict[str, Any]:
    """Validate the complete 20-control profile and any evidence it declares."""
    profile = _load(source)
    if profile.get("schema") != SCHEMA:
        raise ImprovementConflictProfileError(f"schema must be {SCHEMA}")
    if profile.get("version") != "1.0.0":
        raise ImprovementConflictProfileError("version must be 1.0.0")
    project = profile.get("project")
    required_project = ("id", "name", "scope", "safety_boundary")
    if not isinstance(project, dict) or any(not isinstance(project.get(k), str) or not project[k].strip() for k in required_project):
        raise ImprovementConflictProfileError("project must declare non-empty id, name, scope, and safety_boundary")
    status_model = profile.get("status_model")
    if not isinstance(status_model, dict) or status_model.get("initial") != "not_assessed" or status_model.get("allowed") != list(ALLOWED_STATES):
        raise ImprovementConflictProfileError("status_model differs from the governed lifecycle")
    controls = profile.get("controls")
    if not isinstance(controls, list) or len(controls) != 20:
        raise ImprovementConflictProfileError("exactly 20 controls are required")
    root_path = Path(root).resolve() if root is not None else None
    counts = {state: 0 for state in ALLOWED_STATES}
    for index, control in enumerate(controls):
        if not isinstance(control, dict):
            raise ImprovementConflictProfileError(f"control {index + 1} must be an object")
        expected_id = f"IC-{index + 1:02d}"
        if control.get("id") != expected_id:
            raise ImprovementConflictProfileError(f"control {index + 1} must have identifier {expected_id}")
        actual = (control.get("improvement_term"), control.get("dissonance_term"), control.get("conflict_term"))
        if actual != tuple(EXPECTED_TERMS[index]):
            raise ImprovementConflictProfileError(f"{expected_id} vocabulary does not match the canonical 20-triplet register")
        for field in ("intent", "remediation", "verification"):
            if not isinstance(control.get(field), str) or not control[field].strip():
                raise ImprovementConflictProfileError(f"{expected_id}.{field} must be non-empty")
        state = control.get("state")
        if state not in ALLOWED_STATES:
            raise ImprovementConflictProfileError(f"{expected_id} has unsupported state {state!r}")
        counts[state] += 1
        evidence = control.get("evidence")
        if not isinstance(evidence, list) or any(not isinstance(item, str) for item in evidence):
            raise ImprovementConflictProfileError(f"{expected_id}.evidence must be an array of paths")
        if state in {"detected", "root_cause_confirmed", "remediated", "verified"} and not evidence:
            raise ImprovementConflictProfileError(f"{expected_id} state {state!r} requires evidence")
        if state == "deferred" and not (isinstance(control.get("note"), str) and control["note"].strip()):
            raise ImprovementConflictProfileError(f"{expected_id} deferred state requires a note")
        for raw_path in evidence:
            relative = _safe_evidence_path(raw_path)
            if root_path is not None:
                resolved = (root_path / Path(*relative.parts)).resolve()
                try:
                    resolved.relative_to(root_path)
                except ValueError as error:
                    raise ImprovementConflictProfileError(f"{expected_id} evidence escapes repository root: {raw_path!r}") from error
                if not resolved.is_file():
                    raise ImprovementConflictProfileError(f"{expected_id} evidence file does not exist: {raw_path!r}")
    return {"schema": SCHEMA, "project": project["name"], "controls": len(controls), "states": counts, "assessed": len(controls) - counts["not_assessed"], "verified": counts["verified"]}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    repository_root = Path(__file__).resolve().parents[1]
    parser.add_argument("--profile", type=Path, default=repository_root / "improvement-conflict.v1.json")
    parser.add_argument("--root", type=Path, default=repository_root)
    parser.add_argument("--format", choices=("text", "json"), default="text")
    args = parser.parse_args(argv)
    try:
        result = validate_profile(args.profile, root=args.root)
    except ImprovementConflictProfileError as error:
        print(f"improvement/conflict validation failed: {error}", file=sys.stderr)
        return 2
    if args.format == "json":
        print(json.dumps(result, ensure_ascii=False, indent=2))
    else:
        states = ", ".join(f"{name}={count}" for name, count in result["states"].items() if count)
        print(f"{result['project']}: {result['controls']} controls valid; {states}.")
        print("Registry validation only; this command does not scan source code or certify fixes.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
