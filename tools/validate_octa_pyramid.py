"""Validate the shared four-project OCTA-8 federation topology."""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
TOPOLOGY_PATH = ROOT / "federation" / "octa-pyramid.v1.json"
SCHEMA_PATH = ROOT / "schemas" / "octa-pyramid.v1.schema.json"
PROJECT_PATH = ROOT / "project-contract.json"

PROJECTS = {"reaper", "pytyliki", "oplague", "ventusample"}
FACETS = {
    "facet.identity", "facet.contracts", "facet.transport", "facet.provenance",
    "facet.validation", "facet.privacy", "facet.safety", "facet.audit",
}
EXPECTED_EDGES = {
    ("reaper", "oplague", "json-file", "reaper.oplage-comparison.v1", "artifact", "implemented"),
    ("oplague", "reaper", "json-file", "oplage.device-snapshot.v1", "artifact", "implemented"),
    ("reaper", "pytyliki", "native-ffi", "pytyliki.c-abi.v1", "request", "implemented"),
    ("pytyliki", "reaper", "native-ffi", "pytyliki.c-abi.v1", "response", "implemented"),
    ("oplague", "pytyliki", "native-ffi", "pytyliki.c-abi.v1", "request", "implemented"),
    ("pytyliki", "oplague", "native-ffi", "pytyliki.c-abi.v1", "response", "implemented"),
    ("ventusample", "reaper", "json-file", "ventusample.pack.v1", "artifact", "implemented"),
    ("reaper", "ventusample", "json-file", "ventusample.render-request.v1", "artifact", "planned"),
    ("ventusample", "pytyliki", "native-ffi", "pytyliki.c-abi.v1", "request", "planned"),
    ("pytyliki", "ventusample", "native-ffi", "pytyliki.c-abi.v1", "response", "planned"),
    ("ventusample", "oplague", "none", "not-applicable.v1", "none", "not_applicable"),
    ("oplague", "ventusample", "none", "not-applicable.v1", "none", "not_applicable"),
}


def _unique(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _load(path: Path) -> Any:
    try:
        return json.loads(
            path.read_text(encoding="utf-8"),
            object_pairs_hook=_unique,
            parse_constant=lambda token: (_ for _ in ()).throw(
                ValueError(f"non-finite JSON number: {token}")
            ),
        )
    except OSError as error:
        raise ValueError(f"cannot read {path}: {error}") from error


def _need(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def validate(
    topology_path: Path = TOPOLOGY_PATH,
    schema_path: Path = SCHEMA_PATH,
    project_path: Path = PROJECT_PATH,
) -> dict[str, Any]:
    schema = _load(schema_path)
    _need(
        isinstance(schema, dict)
        and schema.get("$schema") == "https://json-schema.org/draft/2020-12/schema"
        and schema.get("title") == "OCTA-8 Federation Pyramid v1",
        "invalid OCTA JSON Schema declaration",
    )
    topology = _load(topology_path)
    _need(
        isinstance(topology, dict)
        and topology.get("schema") == "octa.federation-pyramid.v1"
        and topology.get("topology_id") == "octa-8",
        "invalid OCTA topology identifier",
    )
    _need(
        set(topology) == {
            "schema", "topology_id", "shape", "apex", "base_mesh",
            "facets", "peer_links", "self_loops", "semantics",
        },
        "unexpected or missing topology fields",
    )

    shape = topology["shape"]
    _need(
        isinstance(shape, dict)
        and shape.get("kind") == "octagonal-pyramid"
        and shape.get("base_sides") == 8
        and shape.get("triangular_facets") == 8
        and shape.get("apex_vertex") == "octa-hub",
        "invalid octagonal pyramid geometry",
    )
    apex = topology["apex"]
    _need(
        isinstance(apex, dict)
        and apex.get("id") == "octa-hub"
        and apex.get("role") == "protocol-coordination-only"
        and apex.get("executable") is False
        and apex.get("single_point_of_failure") is False,
        "apex must remain non-executable coordination metadata",
    )

    facets = topology["facets"]
    _need(isinstance(facets, list) and len(facets) == 8, "exactly eight facets are required")
    facet_ids = [item.get("id") for item in facets if isinstance(item, dict)]
    _need(
        len(facet_ids) == 8 and len(set(facet_ids)) == 8 and set(facet_ids) == FACETS,
        "eight unique canonical facet IDs are required",
    )
    _need(
        apex.get("linked_facets") == facet_ids
        and sorted(item.get("side") for item in facets) == list(range(1, 9)),
        "apex/facet geometry is inconsistent",
    )

    base = topology["base_mesh"]
    _need(
        isinstance(base, dict)
        and base.get("mode") == "horizontal-complete-directed-peer-mesh"
        and base.get("project_ids") == ["reaper", "pytyliki", "oplague", "ventusample"]
        and base.get("directed_peer_links") == 12
        and base.get("self_loops") == 4,
        "four-project horizontal peer base is incomplete",
    )

    links = topology["peer_links"]
    _need(isinstance(links, list) and len(links) == 12, "exactly twelve directed links are required")
    actual_edges: set[tuple[str, str, str, str, str, str]] = set()
    link_ids: list[str] = []
    for index, item in enumerate(links):
        _need(isinstance(item, dict), f"link {index} must be an object")
        _need(
            set(item) == {"id", "from", "to", "transport", "interface", "flow", "status"},
            f"link {index} fields are invalid",
        )
        link_id = item.get("id")
        _need(isinstance(link_id, str) and bool(link_id), f"link {index} needs a stable ID")
        link_ids.append(link_id)
        source, target = item.get("from"), item.get("to")
        _need(
            source in PROJECTS and target in PROJECTS and source != target,
            f"link {index} has an invalid project direction",
        )
        actual_edges.add((
            source, target, item.get("transport"), item.get("interface"),
            item.get("flow"), item.get("status"),
        ))
    _need(len(set(link_ids)) == 12, "all twelve directed link IDs must be unique")
    _need(actual_edges == EXPECTED_EDGES, "all twelve directed pair contracts must match the expected topology")
    _need(
        {(edge[0], edge[1]) for edge in actual_edges}
        == {(source, target) for source in PROJECTS for target in PROJECTS if source != target},
        "every ordered pair of distinct projects must have exactly one link",
    )

    loops = topology["self_loops"]
    _need(
        isinstance(loops, list)
        and len(loops) == 4
        and {item.get("project") for item in loops if isinstance(item, dict)} == PROJECTS,
        "each project must have exactly one local self-loop",
    )

    semantics = topology["semantics"]
    _need(
        isinstance(semantics, dict)
        and semantics.get("complete_pairwise_product") is True
        and semantics.get("directed_link_count_formula") == "n*(n-1)"
        and semantics.get("hub_is_not_a_runtime_gateway") is True
        and semantics.get("request_response_edges_are_synchronous") is True
        and semantics.get("no_implicit_native_loading") is True
        and semantics.get("no_driver_mutation_through_mesh") is True
        and semantics.get("reaper_pack_consumer_implemented") is True,
        "OCTA topology semantics are invalid or stale",
    )

    manifest = _load(project_path)
    _need(isinstance(manifest, dict) and isinstance(manifest.get("project"), dict), "project manifest is malformed")
    local_project = manifest["project"].get("id")
    _need(local_project in PROJECTS, "local project is missing from the four-project mesh")

    return {
        "valid": True,
        "topology": "octa-8",
        "project": local_project,
        "projects": 4,
        "base_sides": 8,
        "facets": 8,
        "directed_peer_links": 12,
        "self_loops": 4,
        "complete_pairwise_product": True,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", action="store_true")
    arguments = parser.parse_args()
    try:
        result = validate()
    except (ValueError, TypeError, KeyError) as error:
        print(f"OCTA-8 INVALID: {error}", file=sys.stderr)
        return 1
    if arguments.json:
        print(json.dumps(result, ensure_ascii=False, sort_keys=True, indent=2))
    else:
        print(
            f"OCTA-8 valid | {result['project']} | 4 projects | 8 facets | "
            "12 directed links | 4 self-loops"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
