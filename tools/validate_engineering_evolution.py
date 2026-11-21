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
    required = {"schema","version","title","purpose","adoption","pillars","guardrails"}
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
    result = {
        "ok": not errors,
        "policy": "engineering-evolution.v1",
        "pillars": len(policy.get("pillars", [])) if isinstance(policy, dict) else 0,
        "practices": sum(len(x.get("practices", [])) for x in policy.get("pillars", []) if isinstance(x, dict)) if isinstance(policy, dict) else 0,
        "errors": errors,
    }
    if args.json:
        print(json.dumps(result, ensure_ascii=False, indent=2))
    elif errors:
        print("Engineering Evolution contract: FAIL", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
    else:
        print("Engineering Evolution contract: PASS (20 domains, 40 practices)")
    return 1 if errors else 0

if __name__ == "__main__":
    raise SystemExit(main())
