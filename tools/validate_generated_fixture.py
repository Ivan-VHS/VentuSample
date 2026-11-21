#!/usr/bin/env python3
"""Validate a pack emitted by the C++ integration test against the v1 schema shape."""
from __future__ import annotations

import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FIXTURE = Path(__import__("tempfile").gettempdir()) / "ventusample-tests" / "pack"


def main() -> int:
    schema = json.loads((ROOT / "schemas/ventusample.pack.v2.schema.json").read_text(encoding="utf-8"))
    pack = json.loads((FIXTURE / "ventusample_manifest.json").read_text(encoding="utf-8"))
    assert pack["schema"] == schema["properties"]["schema"]["const"]
    assert pack["audio_format"] == schema["properties"]["audio_format"]["const"]
    params = pack["parameters"]
    parameter_schema = schema["properties"]["parameters"]
    assert set(parameter_schema["required"]) == set(params), (set(parameter_schema["required"]) - set(params), set(params) - set(parameter_schema["required"]))
    for key, rule in parameter_schema["properties"].items():
        value = params[key]
        if "minimum" in rule:
            assert value >= rule["minimum"], (key, value)
        if "maximum" in rule:
            assert value <= rule["maximum"], (key, value)
        if "enum" in rule:
            assert value in rule["enum"], (key, value)
        if rule.get("type") == "boolean":
            assert isinstance(value, bool), (key, value)
    assert pack["source_sample"] == "single.wav"
    assert 1 <= len(pack["files"]) <= schema["properties"]["files"]["maxItems"]
    for entry in pack["files"]:
        assert set(entry) == set(schema["properties"]["files"]["items"]["required"])
        assert (FIXTURE / entry["file"]).is_file()
        assert math.isfinite(entry["peak"]) and 0 <= entry["peak"] <= 1
        assert math.isfinite(entry["rms"]) and 0 <= entry["rms"] <= 1
        assert math.isfinite(entry["crest_db"]) and entry["crest_db"] >= 0
        assert entry["musical_key"] is None or isinstance(entry["musical_key"], str)
        assert entry["scale"] is None or isinstance(entry["scale"], str)
        assert entry["mode_family"] in (None, "major", "minor", "modal", "pitched_note")
    print("Generated pack fixture matches ventusample.pack.v2 schema shape.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
