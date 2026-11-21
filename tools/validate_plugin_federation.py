"""Validate the vendored cross-repository plugin catalog and host contract."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXPECTED = [
    ("operandu", "Operandu"), ("dropaul", "Dropaul"), ("zdoro", "Zdoro"),
    ("pyemont", "Pyemont"), ("dilly", "Dilly"), ("baplap", "Baplap"),
    ("sretuo", "Sretuo"), ("kali", "Kali"), ("zalam", "Zalam"),
    ("liur", "Liur"), ("keel-koh", "Keel Koh"), ("vorof", "Vorof"),
    ("vprof", "Vprof"), ("profe", "Profe"), ("prossee", "Prossee"),
    ("vprossee", "Vprossee"), ("professiya", "Professiya"), ("profevo", "Profevo"),
]


def load(relative: str) -> dict:
    value = json.loads((ROOT / relative).read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"{relative} must contain a JSON object")
    return value


def validate() -> None:
    catalog = load("federation/plugins/catalog.v1.json")
    host = load("federation/plugins/host.v1.json")
    schema = load("schemas/ivan-vhs.plugin-catalog.v1.schema.json")

    if catalog.get("schema") != "ivan-vhs.plugin-catalog.v1" or catalog.get("api_version") != 1:
        raise ValueError("unsupported plugin catalog schema or API version")
    pairs = [(row.get("id"), row.get("name")) for row in catalog.get("plugins", [])]
    if pairs != EXPECTED:
        raise ValueError("catalog IDs/names do not exactly match the shared 18-plugin contract")
    if len({plugin_id for plugin_id, _ in pairs}) != len(EXPECTED):
        raise ValueError("plugin IDs must be unique")
    policy = catalog.get("policy", {})
    if policy.get("discovery") != "metadata-only" or policy.get("external_code_loading") is not False:
        raise ValueError("catalog execution policy is unsafe or missing")
    if schema.get("properties", {}).get("plugins", {}).get("maxItems") != 18:
        raise ValueError("catalog JSON Schema does not constrain the canonical 18 entries")

    if host.get("schema") != "ivan-vhs.plugin-host.v1" or host.get("api_version") != 1:
        raise ValueError("unsupported host manifest schema or API version")
    if host.get("catalog") != "federation/plugins/catalog.v1.json":
        raise ValueError("host manifest does not point to the vendored catalog")
    host_pairs = [(row.get("id"), row.get("name")) for row in host.get("plugins", [])]
    if host_pairs != EXPECTED:
        raise ValueError("host plugin list differs from the canonical catalog")
    if host.get("host_id") == "pytyliki":
        if host.get("execution") != "metadata-catalogue-only" or any(row.get("status") != "catalogued" for row in host["plugins"]):
            raise ValueError("Pytyliki must declare metadata-only plugin support")
    else:
        if any(row.get("status") != "implemented" for row in host["plugins"]):
            raise ValueError("runtime host entries must declare implemented status")
    safety = host.get("safety", {})
    if safety.get("metadata_is_not_execution_authority") is not True:
        raise ValueError("host must explicitly separate metadata from execution authority")


if __name__ == "__main__":
    validate()
    print(f"Plugin federation contract: PASS ({len(EXPECTED)} entries, {load('federation/plugins/host.v1.json')['host_id']})")
