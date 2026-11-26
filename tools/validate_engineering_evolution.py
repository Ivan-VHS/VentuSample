#!/usr/bin/env python3
"""Validate the shared Engineering Evolution Framework without third-party dependencies."""
from __future__ import annotations
import argparse
import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
POLICY_PATH = ROOT / "engineering-evolution.v1.json"
SCHEMA_PATH = ROOT / "schemas" / "engineering-evolution.v1.schema.json"

CAPABILITY_STANDARD_PATH = ROOT / "docs" / "ENGINEERING_EVOLUTION_FRAMEWORK.md"
EXPECTED_CAPABILITY_TRIPLES = [
    [
        "Расширяемость",
        "Полноценность",
        "Полнофункциональность"
    ],
    [
        "Масштабируемость",
        "Завершённость",
        "Всефункциональность"
    ],
    [
        "Модульность",
        "Целостность",
        "Функциональная полнота"
    ],
    [
        "Гибкость",
        "Комплексность",
        "Комплексная функциональность"
    ],
    [
        "Адаптивность",
        "Всеохватность",
        "Абсолютная функциональность"
    ],
    [
        "Универсальность",
        "Исчерпываемость",
        "Исчерпывающая функциональность"
    ],
    [
        "Модифицируемость",
        "Совершенность",
        "Всеобъемлющая функциональность"
    ],
    [
        "Настраиваемость",
        "Самодостаточность",
        "Целостная функциональность"
    ],
    [
        "Дополняемость",
        "Функциональность",
        "Системная функциональность"
    ],
    [
        "Развиваемость",
        "Укомплектованность",
        "Полная оснащённость"
    ],
    [
        "Совместимость",
        "Оснащённость",
        "Полная реализованность"
    ],
    [
        "Интегрируемость",
        "Развитость",
        "Всесторонняя реализация"
    ],
    [
        "Преобразуемость",
        "Зрелость",
        "Комплексная реализация"
    ],
    [
        "Вариативность",
        "Полнота",
        "Полная интеграция"
    ],
    [
        "Многофункциональность",
        "Всесторонность",
        "Всеохватная реализация"
    ],
    [
        "Перестраиваемость",
        "Универсальность",
        "Функциональная завершённость"
    ],
    [
        "Обновляемость",
        "Состоятельность",
        "Максимальная оснащённость"
    ],
    [
        "Реконфигурируемость",
        "Законченность",
        "Полная укомплектованность"
    ],
    [
        "Трансформируемость",
        "Совершенство",
        "Самодостаточность системы"
    ],
    [
        "Перспективность",
        "Всеобъемлемость",
        "Полная готовность системы"
    ]
]

def validate_capability_standard(document: str) -> list[str]:
    """Check that the shared capability-completeness standard stays intact."""
    errors: list[str] = []
    if not isinstance(document, str): return ["capability standard document must be text"]
    required_headings = ("## Capability completeness standard","### Canonical vocabulary","### Capability lifecycle and maturity","### Measurable acceptance gate","### Architecture, extension and integration rules","### Project-specific application","### Release gate")
    for heading in required_headings:
        if heading not in document: errors.append(f"capability standard is missing heading: {heading}")
    for index, terms in enumerate(EXPECTED_CAPABILITY_TRIPLES, start=1):
        row = f"| {index} | " + " | ".join(terms) + " |"
        if row not in document: errors.append(f"capability standard is missing canonical capability triple #{index}")
    for control in ("Requirement → design → source → tests → CI → documentation → versioned contract","invalid settings must fail clearly and safely","treat imported files","rollback or recovery path","partial or planned"):
        if control.casefold() not in document.casefold(): errors.append(f"capability standard is missing control: {control}")
    return errors
EXPECTED_TERMS = [
 ("Архитектура","Квантование","Телеметрия"),("Импульс","Модернизация","Калибровка"),
 ("Кристалл","Векторизация","Дискретизация"),("Преобразование","Автоматизация","Инкапсуляция"),
 ("Орбита","Трансформация","Параметризация"),("Интеграция","Оптимизация","Дедупликация"),
 ("Алгоритм","Компиляция","Контейнеризация"),("Механизм","Моделирование","Репликация"),
 ("Эволюция","Конфигурация","Маршрутизация"),("Резонанс","Декомпозиция","Декомпрессия"),
 ("Горизонт","Масштабирование","Трансляция"),("Молекула","Оркестрация","Сегментация"),
 ("Навигация","Интерпретация","Агрегация"),("Синхронизация","Адаптация","Фрагментация"),
 ("Гравитация","Стабилизация","Деплоймент"),("Протокол","Реконструкция","Рефакторинг"),
 ("Геометрия","Верификация","Абстракция"),("Генерация","Декомпиляция","Индексация"),
 ("Траектория","Синтезирование","Сериализация"),("Перспектива","Прогнозирование","Десериализация"),
]

EXPECTED_DIRECTIVES = [["feature-development","Разработать Функционал Единовременно",["architecture","protocol","geometry"]],["parallel-capability-design","Спроектировать Возможности Одновременно",["architecture","impulse","mechanism"]],["comprehensive-tooling","Реализовать Инструментарий Комплексно",["transformation","algorithm","generation"]],["complete-enablement","Создать Оснащение Целиком",["molecule","orbit","gravity"]],["full-integration","Внедрить Оснащённость Полностью",["orbit","integration","geometry"]],["competency-integration","Интегрировать Компетенции Разом",["integration","protocol","synchronization"]],["integration-modernization","Модернизировать Интеграции Сразу",["integration","algorithm","protocol"]],["subsystem-optimization","Оптимизировать Подсистемы Моментально",["crystal","resonance","geometry"]],["parallel-automation","Автоматизировать Механизмы Параллельно",["transformation","evolution","gravity"]],["collective-tool-improvement","Усовершенствовать Инструменты Совокупно",["algorithm","navigation","generation"]],["systematic-component-rework","Переработать Компоненты Системно",["resonance","geometry","protocol"]],["module-scaling","Масштабировать Модули Массово",["horizon","molecule","algorithm"]],["batch-adaptation","Адаптировать Реализация Пакетно",["orbit","synchronization","protocol"]],["centralized-automation","Унифицировать Автоматизация Централизованно",["evolution","transformation","molecule"]],["synchronized-optimization","Синхронизировать Оптимизация Комплексно",["synchronization","gravity","crystal"]],["automatic-compatibility","Имплементировать Совместимость Автоматически",["protocol","integration","generation"]],["synchronized-performance","Расширить Производительность Синхронно",["geometry","crystal","horizon"]],["sequential-scalability","Ускорить Масштабируемость Последовательно",["trajectory","horizon","gravity"]],["continuous-universality","Комплексно усовершенствовать Универсальность Безостановочно",["perspective","trajectory","architecture"]],["atomic-extensibility-refactor","Рефакторить Расширяемость Одномоментно",["architecture","resonance","protocol"]]]

def validate_policy(policy: Any, schema: Any) -> list[str]:
    errors: list[str] = []
    if not isinstance(schema, dict):
        return ["schema file must contain a JSON object"]
    if schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
        errors.append("schema must declare JSON Schema Draft 2020-12")
    if schema.get("type") != "object" or schema.get("additionalProperties") is not False:
        errors.append("schema must define a closed top-level object")
    if not isinstance(policy, dict):
        return errors + ["policy must contain a JSON object"]
    required = {"schema","version","title","purpose","adoption","pillars","execution_directives","guardrails"}
    missing = sorted(required - policy.keys())
    if missing:
        errors.append("missing top-level fields: " + ", ".join(missing))
    if policy.get("schema") != "engineering-evolution.v1":
        errors.append("unsupported policy schema identifier")
    if policy.get("version") != 1:
        errors.append("version must be 1")
    if policy.get("title") != "Engineering Evolution Framework":
        errors.append("unexpected framework title")
    adoption = policy.get("adoption")
    if not isinstance(adoption, dict):
        errors.append("adoption must be an object")
    else:
        if adoption.get("sequence") != ["Inventory","Model","Pilot","Verify","Adopt","Measure"]:
            errors.append("adoption sequence must preserve the six controlled stages")
        if adoption.get("statuses") != ["not_assessed","planned","implemented","not_applicable"]:
            errors.append("adoption statuses are incomplete or out of order")
        if not isinstance(adoption.get("rule"), str) or len(adoption["rule"]) < 80:
            errors.append("adoption rule must explain evidence-based status claims")
    pillars = policy.get("pillars")
    if not isinstance(pillars, list) or len(pillars) != 20:
        errors.append("policy must contain exactly 20 pillars")
        pillars = pillars if isinstance(pillars, list) else []
    seen_ids: set[str] = set()
    for i, expected in enumerate(EXPECTED_TERMS):
        if i >= len(pillars):
            break
        item = pillars[i]
        where = f"pillars[{i}]"
        if not isinstance(item, dict):
            errors.append(f"{where} must be an object")
            continue
        item_id = item.get("id")
        if not isinstance(item_id, str) or not item_id:
            errors.append(f"{where}.id must be a non-empty string")
        elif item_id in seen_ids:
            errors.append(f"duplicate pillar id: {item_id}")
        else:
            seen_ids.add(item_id)
        if item.get("terms") != list(expected):
            errors.append(f"{where}.terms must exactly match the canonical ordered triple")
        practices = item.get("practices")
        if not isinstance(practices, list) or len(practices) != 2:
            errors.append(f"{where}.practices must contain exactly two practice records")
        else:
            for j, method in enumerate(expected[1:]):
                practice = practices[j]
                if not isinstance(practice, dict) or practice.get("term") != method:
                    errors.append(f"{where}.practices[{j}] must implement {method}")
                elif not isinstance(practice.get("guidance"), str) or len(practice["guidance"]) < 20:
                    errors.append(f"{where}.practices[{j}].guidance is too short")
        if not isinstance(item.get("intent"), str) or len(item["intent"]) < 40:
            errors.append(f"{where}.intent must explain the domain")
        acceptance = item.get("acceptance")
        if not isinstance(acceptance, list) or len(acceptance) < 2 or any(not isinstance(x, str) or len(x) < 20 for x in acceptance):
            errors.append(f"{where}.acceptance needs at least two testable checks")
    directives = policy.get("execution_directives")
    if not isinstance(directives, list) or len(directives) != 20:
        errors.append("policy must contain exactly 20 execution directives")
        directives = directives if isinstance(directives, list) else []
    seen_directive_ids: set[str] = set()
    for i, (expected_id, expected_text, expected_domains) in enumerate(EXPECTED_DIRECTIVES):
        if i >= len(directives):
            break
        item = directives[i]
        where = f"execution_directives[{i}]"
        if not isinstance(item, dict):
            errors.append(f"{where} must be an object")
            continue
        item_id = item.get("id")
        if item_id != expected_id:
            errors.append(f"{where}.id must be {expected_id}")
        if not isinstance(item_id, str) or not item_id:
            errors.append(f"{where}.id must be a non-empty string")
        elif item_id in seen_directive_ids:
            errors.append(f"duplicate execution directive id: {item_id}")
        else:
            seen_directive_ids.add(item_id)
        if item.get("source_text") != expected_text:
            errors.append(f"{where}.source_text must preserve the canonical wording and order")
        if item.get("domains") != expected_domains:
            errors.append(f"{where}.domains must preserve the canonical domain mapping")
        if not isinstance(item.get("intent"), str) or len(item["intent"]) < 40:
            errors.append(f"{where}.intent must explain the operational meaning")
        acceptance = item.get("acceptance")
        if not isinstance(acceptance, list) or len(acceptance) < 2 or len(acceptance) > 3 or any(not isinstance(x, str) or len(x) < 20 for x in acceptance):
            errors.append(f"{where}.acceptance needs two or three testable checks")
    guardrails = policy.get("guardrails")
    if not isinstance(guardrails, list) or len(guardrails) < 8 or any(not isinstance(x, str) or len(x) < 30 for x in guardrails):
        errors.append("guardrails must contain at least eight substantive rules")
    return errors

def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", action="store_true", help="emit machine-readable validation output")
    args = parser.parse_args(argv)
    try:
        policy = json.loads(POLICY_PATH.read_text(encoding="utf-8"))
        schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        errors = [f"cannot load policy or schema: {exc}"]
        policy = {}
    else:
        errors = validate_policy(policy, schema)
    try:
        standard_document = CAPABILITY_STANDARD_PATH.read_text(encoding="utf-8")
    except OSError as exc:
        errors.append(f"cannot load capability completeness standard: {exc}")
    else:
        errors.extend(validate_capability_standard(standard_document))
    result = {
        "ok": not errors,
        "capability_triples": len(EXPECTED_CAPABILITY_TRIPLES),
        "policy": "engineering-evolution.v1",
        "pillars": len(policy.get("pillars", [])) if isinstance(policy, dict) else 0,
        "practices": sum(len(x.get("practices", [])) for x in policy.get("pillars", []) if isinstance(x, dict)) if isinstance(policy, dict) else 0,
        "directives": len(policy.get("execution_directives", [])) if isinstance(policy, dict) else 0,
        "errors": errors,
    }
    if args.json:
        print(json.dumps(result, ensure_ascii=False, indent=2))
    elif errors:
        print("Engineering Evolution contract: FAIL", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
    else:
        print("Engineering Evolution contract: PASS (20 domains, 40 practices, 20 execution directives, 20 capability triples)")
    return 1 if errors else 0

if __name__ == "__main__":
    raise SystemExit(main())
