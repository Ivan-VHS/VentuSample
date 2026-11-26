#!/usr/bin/env python3
"""Validate a repository's declared resilience/trust contract using stdlib only.

A passing result validates the policy declaration. It does not prove that runtime
features are implemented; implementation_status intentionally remains contract_only
until code-level enforcement and tests are reviewed.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PROFILE_PATH = ROOT / ".architecture" / "resilience-policy.json"
SCHEMA_PATH = ROOT / ".architecture" / "resilience-policy.schema.json"


def load_policy(path: Path = PROFILE_PATH) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        value = json.load(stream)
    if not isinstance(value, dict):
        raise ValueError("policy root must be a JSON object")
    return value


def validate_policy(policy: dict[str, Any]) -> list[str]:
    errors: list[str] = []

    def section(name: str) -> dict[str, Any]:
        value = policy.get(name)
        if not isinstance(value, dict):
            errors.append(f"{name} must be an object")
            return {}
        return value

    def require_value(obj: dict[str, Any], key: str, expected: Any, path: str) -> None:
        if obj.get(key) != expected:
            errors.append(f"{path}.{key} must be {expected!r}")

    def require_positive_int(obj: dict[str, Any], key: str, path: str, maximum: int | None = None) -> None:
        value = obj.get(key)
        if isinstance(value, bool) or not isinstance(value, int) or value <= 0:
            errors.append(f"{path}.{key} must be a positive integer")
        elif maximum is not None and value > maximum:
            errors.append(f"{path}.{key} must be <= {maximum}")

    if policy.get("policy_version") != "1.0":
        errors.append("policy_version must be '1.0'")
    project = section("project")
    for key in ("name", "scope"):
        if not isinstance(project.get(key), str) or not project[key].strip():
            errors.append(f"project.{key} must be a non-empty string")
    require_value(policy, "implementation_status", "contract_only", "policy")

    architecture = section("architecture")
    if architecture.get("deployment_model") not in {"single_machine", "federated", "centralized", "hybrid"}:
        errors.append("architecture.deployment_model is invalid")
    if architecture.get("network_access") not in {"disabled", "allowlisted_signed_artifacts_only", "explicitly_scoped"}:
        errors.append("architecture.network_access is invalid")
    if architecture.get("distributed_execution") not in {"not_exposed", "planned", "enabled"}:
        errors.append("architecture.distributed_execution is invalid")
    require_value(architecture, "offline_capable", True, "architecture")
    require_value(architecture, "external_dependency_policy", "verified_only", "architecture")
    require_value(architecture, "cache_fallback", True, "architecture")

    trust = section("trust")
    for key, expected in (
        ("default_decision", "deny"),
        ("tls_verification_required", True),
        ("artifact_signature_required", True),
        ("schema_validation_required", True),
        ("allow_unverified_upstream", False),
        ("policy_bypass_allowed", False),
        ("privileged_actions_require_human_approval", True),
    ):
        require_value(trust, key, expected, "trust")

    resilience = section("resilience")
    require_positive_int(resilience, "timeout_ms", "resilience", 300000)
    if resilience.get("degraded_mode") not in {"fail_closed", "safe_read_only", "bounded_local_fallback"}:
        errors.append("resilience.degraded_mode is invalid")
    require_value(resilience, "resource_limits_required", True, "resilience")
    retry = resilience.get("retry")
    if not isinstance(retry, dict):
        errors.append("resilience.retry must be an object")
    else:
        attempts = retry.get("max_attempts")
        if isinstance(attempts, bool) or not isinstance(attempts, int) or not 0 <= attempts <= 10:
            errors.append("resilience.retry.max_attempts must be an integer from 0 to 10")
        require_value(retry, "exponential_backoff", True, "resilience.retry")
        require_value(retry, "jitter", True, "resilience.retry")
    breaker = resilience.get("circuit_breaker")
    if not isinstance(breaker, dict):
        errors.append("resilience.circuit_breaker must be an object")
    else:
        require_value(breaker, "enabled", True, "resilience.circuit_breaker")
        for key in ("failure_threshold", "window_seconds", "cooldown_seconds"):
            require_positive_int(breaker, key, "resilience.circuit_breaker")

    escalation = section("escalation")
    require_value(escalation, "levels", ["observe", "throttle", "isolate", "operator_review"], "escalation")
    require_value(escalation, "hard_stop_on_integrity_failure", True, "escalation")
    require_value(escalation, "automatic_credential_revocation", False, "escalation")
    require_value(escalation, "automatic_network_block", False, "escalation")
    require_value(escalation, "destructive_action_human_approval", True, "escalation")
    thresholds = escalation.get("thresholds")
    if not isinstance(thresholds, dict):
        errors.append("escalation.thresholds must be an object")
    else:
        for key in ("window_seconds", "events_before_operator_review"):
            require_positive_int(thresholds, key, "escalation.thresholds")

    migration = section("migration")
    for key in ("backup_before_migration", "rollback_plan_required", "staged_rollout_required", "api_compatibility_check_required"):
        require_value(migration, key, True, "migration")

    supply_chain = section("supply_chain")
    for key in ("dependency_lock_required", "sbom_required_for_release", "provenance_required",
                "release_signing_required", "reproducible_builds_targeted", "unverified_sources_rejected"):
        require_value(supply_chain, key, True, "supply_chain")

    observability = section("observability")
    for key, expected in (
        ("structured_events", True),
        ("audit_trail", True),
        ("secret_redaction", True),
        ("telemetry_default", "local_only"),
        ("evidence_labels", ["measured", "derived", "estimated", "unknown"]),
    ):
        require_value(observability, key, expected, "observability")

    manual = section("manual_override")
    for key in ("enabled", "reason_required", "audit_required", "configuration_drift_detection"):
        require_value(manual, key, True, "manual_override")
    require_positive_int(manual, "expiry_seconds", "manual_override", 86400)

    compatibility = section("compatibility")
    for key, expected in (
        ("api_contract_versioning", True),
        ("schema_validation", True),
        ("allow_unreviewed_exceptions", False),
        ("exception_expiry_required", True),
    ):
        require_value(compatibility, key, expected, "compatibility")

    guardrails = policy.get("domain_guardrails")
    if not isinstance(guardrails, list) or not guardrails or any(
        not isinstance(item, str) or len(item.strip()) < 8 for item in guardrails
    ):
        errors.append("domain_guardrails must contain non-empty, meaningful strings")

    return errors


def main() -> int:
    try:
        policy = load_policy()
        with SCHEMA_PATH.open("r", encoding="utf-8") as stream:
            schema = json.load(stream)
        if schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
            raise ValueError("schema must declare JSON Schema Draft 2020-12")
        required = set(schema.get("required", []))
        if not required.issubset(policy.keys()):
            raise ValueError("policy is missing one or more top-level schema-required fields")
        errors = validate_policy(policy)
        if errors:
            for error in errors:
                print(f"resilience policy: FAIL: {error}", file=sys.stderr)
            return 1
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        print(f"resilience policy: FAIL: {exc}", file=sys.stderr)
        return 1

    print(
        f"resilience policy: PASS ({policy['project']['name']}; "
        "contract declaration only; runtime enforcement is not implied)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
