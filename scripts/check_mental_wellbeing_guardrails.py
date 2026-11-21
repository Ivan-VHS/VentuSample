#!/usr/bin/env python3
"""Validate the repository's Mental Wellbeing & Depression Awareness v1 guardrails.

This is a policy-contract integrity check, not a clinical or source-code classifier.
It uses only the Python standard library and fails closed on missing/mismatched policy.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "mental-wellbeing-code-safety.v1.json"
POLICY_DOC = ROOT / "docs" / "MENTAL_WELLBEING_CODE_SAFETY_V1.md"
WORKFLOW = ROOT / ".github" / "workflows" / "mental-wellbeing-guardrails.yml"

COMMON_POLICY = {
    "not_a_clinical_protocol": True,
    "no_diagnosis_or_screening": True,
    "no_medication_or_treatment_advice": True,
    "no_crisis_triage_automation": True,
    "no_mental_health_inference": True,
    "no_health_or_productivity_surveillance": True,
    "no_background_collection_of_wellbeing_data": True,
    "no_unconsented_remote_telemetry": True,
    "no_raw_sensitive_content_logging": True,
    "data_minimization_required": True,
    "bounded_untrusted_input_required": True,
    "respectful_non_stigmatizing_language_required": True,
    "accessible_user_control_required": True,
    "blameless_incident_response_required": True,
    "evidence_and_uncertainty_required": True,
}

PROFILES: dict[str, dict[str, Any]] = {
    "pytyliki": {
        "repository": "Ivan-VHS/Pytyliki",
        "language": "C11",
        "boundaries": {
            "source_review_executes_code": False,
            "cpu_benchmark_wellbeing_inference": False,
            "audio_emotion_analysis": False,
            "hardware_discovery_read_only": True,
            "memory_and_dsp_inputs_bounded": True,
            "c_abi_uses_caller_owned_buffers": True,
        },
    },
    "reaper": {
        "repository": "Ivan-VHS/Reaper",
        "language": "Python 3.10+ / C17",
        "boundaries": {
            "inspection_triggers_execution": False,
            "untrusted_runtime_input_bounded": True,
            "plugin_discovery_executes_code": False,
            "native_ffi_explicit_opt_in": True,
            "interpreter_claimed_as_os_sandbox": False,
            "sensitive_lsp_dap_logging": False,
        },
    },
    "oplague": {
        "repository": "Ivan-VHS/OPlague",
        "language": "Python 3.10+",
        "boundaries": {
            "device_inventory_read_only": True,
            "driver_install_or_load": False,
            "package_payload_execution": False,
            "archive_resource_limits": True,
            "device_identifier_minimization": True,
            "health_inference_from_hardware": False,
        },
    },
    "ventusample": {
        "repository": "Ivan-VHS/VentuSample",
        "language": "C++20",
        "boundaries": {
            "default_network_audio_upload": False,
            "implicit_microphone_capture": False,
            "plugin_manifest_executes_code": False,
            "dsp_input_bounds": True,
            "audio_metadata_minimization": True,
            "audio_features_used_for_health_inference": False,
        },
    },
}


def fail(message: str, errors: list[str]) -> None:
    errors.append(message)


def main() -> int:
    errors: list[str] = []

    if not CONTRACT.is_file():
        print(f"ERROR: missing {CONTRACT.relative_to(ROOT)}", file=sys.stderr)
        return 1
    if not POLICY_DOC.is_file():
        print(f"ERROR: missing {POLICY_DOC.relative_to(ROOT)}", file=sys.stderr)
        return 1
    if not WORKFLOW.is_file():
        print(f"ERROR: missing {WORKFLOW.relative_to(ROOT)}", file=sys.stderr)
        return 1

    try:
        data = json.loads(CONTRACT.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"ERROR: invalid policy JSON: {exc}", file=sys.stderr)
        return 1

    if data.get("schema") != "mental-wellbeing-code-safety.v1":
        fail("unexpected contract schema", errors)
    if data.get("version") != "1.0.0":
        fail("unexpected contract version", errors)

    source = data.get("source", {})
    expected_source = {
        "video_id": "brNbnipJ-i8",
        "channel": "ебаут",
        "published_at": "2025-11-23",
        "duration": "PT1H20M1S",
        "transcript_coverage": "partial_head_and_tail",
        "missing_middle": "not_reconstructed",
        "clinical_authority": False,
    }
    for key, expected in expected_source.items():
        if source.get(key) != expected:
            fail(f"source.{key} must be {expected!r}", errors)

    if data.get("policy") != COMMON_POLICY:
        fail("common policy flags are missing, changed or weaker than the v1 contract", errors)

    project = data.get("project", {})
    project_id = project.get("id")
    profile = PROFILES.get(project_id)
    if profile is None:
        fail(f"unknown or missing project id: {project_id!r}", errors)
    else:
        if project.get("repository") != profile["repository"]:
            fail("project repository does not match the registered profile", errors)
        if project.get("language") != profile["language"]:
            fail("project language does not match the registered profile", errors)
        if data.get("boundaries") != profile["boundaries"]:
            fail("project-specific safety boundaries are missing or weakened", errors)
        if not project.get("surfaces") or not all(
            isinstance(item, str) and item.strip() for item in project["surfaces"]
        ):
            fail("project.surfaces must list the repository's sensitive engineering surfaces", errors)

    enforcement = data.get("enforcement", {})
    expected_enforcement = {
        "contract_path": "mental-wellbeing-code-safety.v1.json",
        "validator_path": "scripts/check_mental_wellbeing_guardrails.py",
        "workflow_path": ".github/workflows/mental-wellbeing-guardrails.yml",
        "fail_closed": True,
        "standard_library_only": True,
    }
    for key, expected in expected_enforcement.items():
        if enforcement.get(key) != expected:
            fail(f"enforcement.{key} must be {expected!r}", errors)

    doc = POLICY_DOC.read_text(encoding="utf-8").casefold()
    required_doc_phrases = (
        "not a clinical protocol",
        "no diagnosis or inference",
        "no medical advice",
        "change acceptance checklist",
        "automated enforcement",
        "scope and limitations",
        "blameless engineering",
    )
    for phrase in required_doc_phrases:
        if phrase not in doc:
            fail(f"policy document is missing required section or phrase: {phrase!r}", errors)

    workflow = WORKFLOW.read_text(encoding="utf-8")
    required_workflow_tokens = (
        "pull_request:",
        "workflow_dispatch:",
        "permissions:",
        "contents: read",
        "scripts/check_mental_wellbeing_guardrails.py",
    )
    for token in required_workflow_tokens:
        if token not in workflow:
            fail(f"guardrail workflow is missing {token!r}", errors)

    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    print(
        "Mental Wellbeing & Depression Awareness v1 guardrails: PASS "
        f"({project_id}; contract, source provenance, policy, project boundaries and CI wiring)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
