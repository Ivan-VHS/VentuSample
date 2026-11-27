#!/usr/bin/env python3
"""Validate VentuSample capability evidence and federation metadata."""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STATUSES = {"implemented", "partial", "planned", "not_applicable"}
PEERS = {"reaper", "pytyliki", "oplague"}


def load_json(relative: str) -> dict:
    with (ROOT / relative).open("r", encoding="utf-8") as handle:
        value = json.load(handle)
    if not isinstance(value, dict):
        raise ValueError(f"{relative} must contain a JSON object")
    return value


def validate_supabase_adapter(project_id: str) -> None:
    schema = load_json("federation/schemas/supabase-adapter.v1.schema.json")
    if (schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema"
            or schema.get("$id") != "https://schemas.ivan-vhs.dev/supabase-adapter.v1.schema.json"
            or schema.get("title") != "Ivan-VHS Supabase Adapter Contract v1"):
        raise ValueError("Supabase adapter schema identity is invalid")

    adapter = load_json("federation/supabase-adapter.v1.json")
    expected_fields = {
        "schema", "project", "repository", "role", "status", "default_mode",
        "supabase", "network_policy", "local_first", "contracts",
        "data_domains", "excluded_data", "security",
    }
    if set(adapter) != expected_fields:
        raise ValueError("Supabase adapter fields do not match v1")
    if adapter["schema"] != "ivan-vhs.supabase-adapter.v1":
        raise ValueError("unknown Supabase adapter contract")
    if project_id != "ventusample" or adapter["project"] != project_id:
        raise ValueError("Supabase adapter project does not match VentuSample")
    if adapter["repository"] != "Ivan-VHS/VentuSample":
        raise ValueError("Supabase adapter repository does not match VentuSample")
    if adapter["role"] not in {"producer", "consumer", "producer-consumer"}:
        raise ValueError("Supabase adapter role is invalid")
    if adapter["status"] != "contract-only" or adapter["default_mode"] != "offline-only":
        raise ValueError("Supabase adapter maturity and default mode are invalid")
    if adapter["supabase"] != {
        "deployment": "not-configured",
        "url_env": "SUPABASE_URL",
        "publishable_key_env": "SUPABASE_PUBLISHABLE_KEY",
        "service_role_key_in_client": False,
    }:
        raise ValueError("Supabase hosted access or client secret policy is not safe by default")
    if adapter["network_policy"] != {
        "enabled_by_default": False,
        "background_sync": False,
        "requires_user_action": True,
    }:
        raise ValueError("Supabase network policy must remain explicitly user-initiated")
    if adapter["local_first"] != {
        "local_data_authoritative": True,
        "sync_model": "append-only-event-log",
        "conflict_resolution": "domain-specific-versioned",
    }:
        raise ValueError("Supabase local-first contract is invalid")
    if adapter["contracts"] != {
        "event_envelope": "ivan-vhs.ecosystem-event.v1",
        "artifact_manifest": "ivan-vhs.ecosystem-artifact.v1",
    }:
        raise ValueError("Supabase shared contract IDs are invalid")
    for field in ("data_domains", "excluded_data"):
        values = adapter[field]
        if (not isinstance(values, list) or not values
                or any(not isinstance(value, str) or not value.strip() for value in values)
                or len(values) != len(set(values))):
            raise ValueError(f"Supabase adapter {field} must be a unique non-empty string array")
    if adapter["security"] != {
        "rls_required": True,
        "service_role_key_never_in_client": True,
        "explicit_upload_consent": True,
        "telemetry_default": "disabled",
    }:
        raise ValueError("Supabase adapter security policy is not safe by default")

def validate_contract() -> None:
    data = load_json("project-contract.json")
    validate_supabase_adapter(data["project"]["id"])
    required = {"schema", "project", "implementation", "capabilities", "interfaces", "integrations", "safety_boundaries", "extension_points"}
    if set(data) != required:
        raise ValueError(f"project-contract.json root keys differ: {sorted(set(data) ^ required)}")
    if data["schema"] != "ivan-vhs.project-contract.v1":
        raise ValueError("unknown project contract schema")
    capability_ids: set[str] = set()
    for item in data["capabilities"]:
        identifier = item.get("id")
        if not identifier or identifier in capability_ids:
            raise ValueError(f"missing or duplicate capability id: {identifier!r}")
        capability_ids.add(identifier)
        if item.get("status") not in STATUSES:
            raise ValueError(f"invalid status on capability {identifier}")
        for evidence in item.get("evidence", []):
            candidate = (ROOT / evidence).resolve()
            if ROOT not in candidate.parents or not candidate.is_file():
                raise ValueError(f"missing or unsafe evidence path: {evidence}")
    for interface in data["interfaces"]:
        if not (ROOT / interface["schema"]).is_file():
            raise ValueError(f"missing interface schema: {interface['schema']}")
    for integration in data["integrations"]:
        if integration.get("peer") not in PEERS or integration.get("status") not in STATUSES:
            raise ValueError(f"invalid integration entry: {integration}")
    load_json("schemas/ventusample.pack.v1.schema.json")
    load_json("schemas/ventusample.pack.v2.schema.json")
    plugin_schema = load_json("schemas/ivan-vhs.plugin.v1.schema.json")
    if plugin_schema.get("properties", {}).get("schema", {}).get("const") != "ivan-vhs.plugin.v1":
        raise ValueError("shared plugin manifest schema is missing or invalid")
    plugin_manifest = load_json("examples/plugins/metadata-example/plugin.json")
    if (plugin_manifest.get("schema") != "ivan-vhs.plugin.v1"
            or plugin_manifest.get("host", {}).get("id") != "ventu"
            or plugin_manifest.get("api_version") != 1):
        raise ValueError("Ventu metadata-only plugin fixture violates the shared manifest contract")
    if "quality_monolith_rhythm_pipeline" not in capability_ids:
        raise ValueError("QMR-20 quality/cohesion/rhythm capability is missing")
    if "tonal_export_metadata" not in capability_ids:
        raise ValueError("per-file tonal export metadata capability is missing")
    if not (ROOT / "docs/QUALITY_MONOLITH_RHYTHM.md").is_file():
        raise ValueError("QMR-20 framework documentation is missing")


def validate_federation() -> None:
    data = load_json("federation/octa-pyramid.v1.json")
    projects = data["base_mesh"]["project_ids"]
    expected = {(source, target) for source in projects for target in projects if source != target}
    links = data["peer_links"]
    actual = {(link["from"], link["to"]) for link in links}
    if len(projects) != 4 or len(actual) != len(links) or actual != expected:
        raise ValueError("federation must contain the complete directed four-project peer mesh")
    if data["base_mesh"]["directed_peer_links"] != 12 or data["base_mesh"]["self_loops"] != 4:
        raise ValueError("incorrect four-project mesh counts")
    if len(data["self_loops"]) != len(projects):
        raise ValueError("every project must retain one independent self-loop")
    if data["apex"]["executable"] or data["apex"]["single_point_of_failure"]:
        raise ValueError("octa-hub must remain non-executable and non-critical")
    if len(data["facets"]) != 8 or {facet["side"] for facet in data["facets"]} != set(range(1, 9)):
        raise ValueError("OCTA-8 must declare eight independently numbered facets")
    if data.get("semantics", {}).get("reaper_pack_consumer_implemented") is not True:
        raise ValueError("Reaper pack manifest consumer must be declared implemented")


def main() -> int:
    try:
        validate_contract()
        validate_federation()
    except (OSError, ValueError, KeyError, TypeError, json.JSONDecodeError) as error:
        print(f"VentuSample contract validation failed: {error}", file=sys.stderr)
        return 1
    print("VentuSample project contract and four-peer OCTA mesh are valid.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
