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


def validate_contract() -> None:
    data = load_json("project-contract.json")
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
    if "quality_monolith_rhythm_pipeline" not in capability_ids:
        raise ValueError("QMR-20 quality/cohesion/rhythm capability is missing")
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
