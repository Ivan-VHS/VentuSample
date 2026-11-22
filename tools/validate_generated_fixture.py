#!/usr/bin/env python3
"""Validate a pack emitted by the C++ integration test against the v2 schema shape."""
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
    assert pack["measurement_basis"] == "sample_domain"
    pack_properties = schema["properties"]
    assert set(pack) <= set(pack_properties), set(pack) - set(pack_properties)
    midi_files = pack.get("midi_files", [])
    midi_rule = pack_properties["midi_files"]
    assert isinstance(midi_files, list) and len(midi_files) <= midi_rule["maxItems"]
    assert len(midi_files) == len(set(midi_files))
    for midi_name in midi_files:
        assert isinstance(midi_name, str) and 1 <= len(midi_name) <= 255
        assert Path(midi_name).suffix.casefold() == ".mid"
        assert (FIXTURE / midi_name).is_file(), midi_name
    params = pack["parameters"]
    parameter_schema = schema["properties"]["parameters"]
    parameter_properties = parameter_schema["properties"]
    assert set(parameter_schema["required"]) <= set(params), set(parameter_schema["required"]) - set(params)
    assert set(params) <= set(parameter_properties), set(params) - set(parameter_properties)
    for key, value in params.items():
        rule = parameter_properties[key]
        if "minimum" in rule:
            assert value >= rule["minimum"], (key, value)
        if "maximum" in rule:
            assert value <= rule["maximum"], (key, value)
        if "enum" in rule:
            assert value in rule["enum"], (key, value)
        if rule.get("type") == "boolean":
            assert isinstance(value, bool), (key, value)
    assert params["export_midi"] is False
    assert params["tpdf_dither"] is False
    assert params["normalize_peak"] is False
    assert params["target_peak_dbfs"] == -1.0
    assert pack["source_sample"] == "single.wav"
    assert 1 <= len(pack["files"]) <= schema["properties"]["files"]["maxItems"]
    file_schema = schema["properties"]["files"]["items"]
    for entry in pack["files"]:
        file_properties = file_schema["properties"]
        assert set(file_schema["required"]) <= set(entry), set(file_schema["required"]) - set(entry)
        assert set(entry) <= set(file_properties), set(entry) - set(file_properties)
        if "stochastic_routes" in entry:
            routes = entry["stochastic_routes"]
            assert isinstance(routes, list) and len(routes) <= file_properties["stochastic_routes"]["maxItems"]
            assert all(isinstance(route, str) and route for route in routes)
        assert (FIXTURE / entry["file"]).is_file()
        assert math.isfinite(entry["peak"]) and 0 <= entry["peak"] <= 1
        assert math.isfinite(entry["rms"]) and 0 <= entry["rms"] <= 1
        assert math.isfinite(entry["crest_db"]) and entry["crest_db"] >= 0
        assert 0 <= entry["left_rms"] <= 1 and 0 <= entry["right_rms"] <= 1
        assert -1 <= entry["stereo_correlation"] <= 1
        assert entry["dither"] == "off" and entry["dither_seed"] == 0
        assert entry["normalized"] is False and entry["normalization_skipped_silence"] is False
        assert entry["musical_key"] is None or isinstance(entry["musical_key"], str)
        assert entry["scale"] is None or isinstance(entry["scale"], str)
        assert entry["mode_family"] in (None, "major", "minor", "modal", "pitched_note")
    print("Generated pack fixture matches ventusample.pack.v2 schema shape.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
