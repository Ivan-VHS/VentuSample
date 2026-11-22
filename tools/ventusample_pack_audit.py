"""VentuSample pack v1 validation and deterministic metadata comparison.

The adapter consumes metadata only. WAV assets remain in the pack folder and
are never opened, executed, modified or uploaded by this module.
"""
from __future__ import annotations

import hashlib
import json
import math
from collections.abc import Mapping
from pathlib import Path, PurePosixPath
from typing import Any


VENTUSAMPLE_PACK_SCHEMA = "ventusample.pack.v1"
VENTUSAMPLE_COMPARISON_SCHEMA = "ventusample.pack-comparison.v1"
MAX_PACK_FILES = 256

_TOP_LEVEL_FIELDS = {"schema", "generator", "audio_format", "parameters", "source_sample", "files"}
_PARAMETER_FIELDS = {
    "style", "target", "bpm", "key", "scale", "bars", "variations",
    "swing_percent", "density_percent", "grit_percent", "saturation_percent",
    "stereo_percent", "high_pass_hz", "low_pass_hz", "snare_offset_ms",
    "bus_compression_ratio", "triplet_rolls", "micro_humanize", "vinyl_dust",
    "seed", "sample_rate", "sampler", "timbre", "source_mix_percent",
    "source_pitch_semitones",
}
_FILE_FIELDS = {"file", "peak", "rms", "crest_db", "variation"}
_STYLES = {
    "Boom Bap", "East Coast", "Harlem Dip-Trap", "G-Unit", "Modern Trap",
    "Atlanta Trap", "UK Drill", "UK Grime", "Memphis", "Jazz Rap",
    "G-Funk", "Boom Trap", "Cripple Trill", "Billionaire Golden Era",
}
_TARGETS = {"Full Loop", "Melody", "Drums", "Bass", "One-Shot Kit"}
_KEYS = {"C", "Csharp", "D", "Dsharp", "E", "F", "Fsharp", "G", "Gsharp", "A", "Asharp", "B"}
_SCALES = {"Natural Minor", "Harmonic Minor", "Dorian", "Phrygian", "Major", "Minor Pentatonic"}
_SAMPLERS = {"Clean", "MPC 2500", "MPC 3000XL", "MPC 5000"}
_TIMBRES = {"Keys", "Bell", "Pluck", "Organ", "Brass", "Strings"}
_SAMPLE_RATES = {22050, 44100, 48000, 96000}


def _integer(value: Any, name: str, minimum: int, maximum: int) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"{name} must be a whole number")
    if not minimum <= value <= maximum:
        raise ValueError(f"{name} must be between {minimum} and {maximum}")
    return value


def _real(value: Any, name: str, minimum: float, maximum: float) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TypeError(f"{name} must be a real number")
    result = float(value)
    if not math.isfinite(result) or not minimum <= result <= maximum:
        raise ValueError(f"{name} must be finite and between {minimum} and {maximum}")
    return result


def _choice(value: Any, name: str, options: set[str]) -> str:
    if not isinstance(value, str) or value not in options:
        raise ValueError(f"{name} must be one of the supported VentuSample values")
    return value


def _safe_filename(value: Any, name: str) -> str:
    if not isinstance(value, str) or not value or len(value) > 255:
        raise ValueError(f"{name} must be non-empty filename text of at most 255 characters")
    if value in {".", ".."} or "/" in value or "\\" in value or PurePosixPath(value).name != value:
        raise ValueError(f"{name} must be a simple relative filename, not a path")
    if any(ord(character) < 32 for character in value):
        raise ValueError(f"{name} cannot contain control characters")
    return value


def _validate_pack(pack: Any) -> tuple[dict[str, float], dict[str, Any]]:
    label = "VentuSample pack"
    if not isinstance(pack, Mapping):
        raise TypeError(f"{label} must be a JSON object")
    keys = set(pack)
    missing = _TOP_LEVEL_FIELDS - keys
    extra = keys - _TOP_LEVEL_FIELDS
    if missing or extra:
        raise ValueError(f"{label} fields are invalid; missing={sorted(missing)}, extra={sorted(extra)}")
    if pack.get("schema") != VENTUSAMPLE_PACK_SCHEMA:
        raise ValueError(f"{label}.schema must be {VENTUSAMPLE_PACK_SCHEMA}")
    generator = pack.get("generator")
    if not isinstance(generator, str) or not generator.strip() or len(generator) > 128:
        raise ValueError(f"{label}.generator must be non-empty text of at most 128 characters")
    if pack.get("audio_format") != "wav_pcm16_stereo":
        raise ValueError(f"{label}.audio_format must be wav_pcm16_stereo")
    source_sample = pack.get("source_sample")
    if not isinstance(source_sample, str) or len(source_sample) > 255:
        raise ValueError(f"{label}.source_sample must be text of at most 255 characters")
    if source_sample and ("/" in source_sample or "\\" in source_sample or source_sample in {".", ".."}):
        raise ValueError(f"{label}.source_sample must contain a filename, not a local path")

    parameters = pack.get("parameters")
    if not isinstance(parameters, Mapping):
        raise TypeError(f"{label}.parameters must be an object")
    parameter_keys = set(parameters)
    missing_parameters = _PARAMETER_FIELDS - parameter_keys
    extra_parameters = parameter_keys - _PARAMETER_FIELDS
    if missing_parameters or extra_parameters:
        raise ValueError(
            f"{label}.parameters fields are invalid; missing={sorted(missing_parameters)}, "
            f"extra={sorted(extra_parameters)}"
        )

    style = _choice(parameters["style"], "parameters.style", _STYLES)
    target = _choice(parameters["target"], "parameters.target", _TARGETS)
    key = _choice(parameters["key"], "parameters.key", _KEYS)
    scale = _choice(parameters["scale"], "parameters.scale", _SCALES)
    sampler = _choice(parameters["sampler"], "parameters.sampler", _SAMPLERS)
    timbre = _choice(parameters["timbre"], "parameters.timbre", _TIMBRES)

    bpm = _integer(parameters["bpm"], "parameters.bpm", 60, 190)
    bars = _integer(parameters["bars"], "parameters.bars", 1, 16)
    variations = _integer(parameters["variations"], "parameters.variations", 1, 32)
    swing = _integer(parameters["swing_percent"], "parameters.swing_percent", 50, 68)
    density = _integer(parameters["density_percent"], "parameters.density_percent", 0, 100)
    grit = _integer(parameters["grit_percent"], "parameters.grit_percent", 0, 100)
    saturation = _integer(parameters["saturation_percent"], "parameters.saturation_percent", 0, 100)
    stereo = _integer(parameters["stereo_percent"], "parameters.stereo_percent", 0, 100)
    high_pass = _integer(parameters["high_pass_hz"], "parameters.high_pass_hz", 20, 300)
    low_pass = _integer(parameters["low_pass_hz"], "parameters.low_pass_hz", 2000, 20000)
    snare_offset = _integer(parameters["snare_offset_ms"], "parameters.snare_offset_ms", -30, 30)
    compression = _real(parameters["bus_compression_ratio"], "parameters.bus_compression_ratio", 1.0, 12.0)
    seed = _integer(parameters["seed"], "parameters.seed", 0, 2147483647)
    sample_rate = _integer(parameters["sample_rate"], "parameters.sample_rate", 22050, 96000)
    if sample_rate not in _SAMPLE_RATES:
        raise ValueError("parameters.sample_rate must be one of 22050, 44100, 48000 or 96000")
    if high_pass >= low_pass or low_pass >= 0.49 * sample_rate:
        raise ValueError("filter bounds must satisfy high_pass_hz < low_pass_hz < 0.49 * sample_rate")
    source_mix = _integer(parameters["source_mix_percent"], "parameters.source_mix_percent", 0, 100)
    source_pitch = _integer(parameters["source_pitch_semitones"], "parameters.source_pitch_semitones", -12, 12)
    for name in ("triplet_rolls", "micro_humanize", "vinyl_dust"):
        if not isinstance(parameters[name], bool):
            raise TypeError(f"parameters.{name} must be boolean")

    files = pack.get("files")
    if not isinstance(files, list) or not 1 <= len(files) <= MAX_PACK_FILES:
        raise ValueError(f"{label}.files must contain 1..{MAX_PACK_FILES} audio records")
    filenames: set[str] = set()
    peaks: list[float] = []
    rms_values: list[float] = []
    crest_values: list[float] = []
    seen_variations: set[int] = set()
    variation_counts = {number: 0 for number in range(1, variations + 1)}
    files_per_variation = 8 if target == "One-Shot Kit" else 1
    expected_count = variations * files_per_variation
    if len(files) != expected_count:
        raise ValueError(
            f"{label}.files must contain exactly {expected_count} records for "
            f"{variations} variation(s) of {target}"
        )
    for index, item in enumerate(files):
        if not isinstance(item, Mapping):
            raise TypeError(f"{label}.files[{index}] must be an object")
        if set(item) != _FILE_FIELDS:
            raise ValueError(f"{label}.files[{index}] must contain exactly {sorted(_FILE_FIELDS)}")
        filename = _safe_filename(item["file"], f"{label}.files[{index}].file")
        if not filename.lower().endswith(".wav"):
            raise ValueError(f"{label}.files[{index}].file must use the .wav extension")
        marker = filename.casefold()
        if marker in filenames:
            raise ValueError(f"{label}.files contains duplicate filename: {filename}")
        filenames.add(marker)
        peaks.append(_real(item["peak"], f"{label}.files[{index}].peak", 0.0, 1.0))
        rms_values.append(_real(item["rms"], f"{label}.files[{index}].rms", 0.0, 1.0))
        crest_values.append(_real(item["crest_db"], f"{label}.files[{index}].crest_db", 0.0, 100.0))
        variation = _integer(item["variation"], f"{label}.files[{index}].variation", 1, variations)
        seen_variations.add(variation)
        variation_counts[variation] += 1
    if seen_variations != set(range(1, variations + 1)):
        raise ValueError(f"{label}.files must contain every declared variation")
    if any(count != files_per_variation for count in variation_counts.values()):
        raise ValueError(
            f"{label}.files must contain exactly {files_per_variation} file(s) per variation"
        )

    mean = lambda values: math.fsum(values) / len(values)
    metrics = {
        "bpm_normalized": bpm / 190.0,
        "bars_normalized": bars / 16.0,
        "variations_normalized": variations / 32.0,
        "swing_normalized": (swing - 50.0) / 18.0,
        "density_normalized": density / 100.0,
        "grit_normalized": grit / 100.0,
        "saturation_normalized": saturation / 100.0,
        "stereo_normalized": stereo / 100.0,
        "high_pass_normalized": (high_pass - 20.0) / 280.0,
        "low_pass_normalized": (low_pass - 2000.0) / 18000.0,
        "snare_offset_normalized": (snare_offset + 30.0) / 60.0,
        "compression_normalized": (compression - 1.0) / 11.0,
        "triplet_rolls_enabled": float(parameters["triplet_rolls"]),
        "micro_humanize_enabled": float(parameters["micro_humanize"]),
        "vinyl_dust_enabled": float(parameters["vinyl_dust"]),
        "sample_rate_normalized": sample_rate / 96000.0,
        "source_mix_normalized": source_mix / 100.0,
        "source_pitch_normalized": (source_pitch + 12.0) / 24.0,
        "mean_peak": mean(peaks),
        "max_peak": max(peaks),
        "mean_rms": mean(rms_values),
        "mean_crest_normalized": mean(crest_values) / 100.0,
    }
    metadata = {
        "generator": generator,
        "style": style,
        "target": target,
        "bpm": bpm,
        "key": key,
        "scale": scale,
        "bars": bars,
        "variations": variations,
        "seed": seed,
        "sample_rate": sample_rate,
        "sampler": sampler,
        "timbre": timbre,
        "source_sample": source_sample,
        "file_count": len(files),
        "source_sample_present": bool(source_sample),
    }
    return metrics, metadata


def ventusample_metrics(pack: Any) -> dict[str, float]:
    """Validate a complete VentuSample pack manifest and return normalized metrics."""
    metrics, _ = _validate_pack(pack)
    return dict(sorted(metrics.items()))


def compare_ventusample_packs(reference: Any, candidate: Any) -> dict[str, Any]:
    """Compare two validated pack manifests without opening or altering assets."""
    left, left_meta = _validate_pack(reference)
    right, right_meta = _validate_pack(candidate)
    if left.keys() != right.keys():
        raise ValueError("VentuSample metric dimensions differ; refusing implicit alignment")

    dimensions = list(sorted(left))
    reference_values = [left[name] for name in dimensions]
    candidate_values = [right[name] for name in dimensions]
    distance = math.sqrt(
        math.fsum((left_value - right_value) ** 2
                  for left_value, right_value in zip(reference_values, candidate_values, strict=True))
    )

    components = {
        name: {
            "reference": left[name],
            "candidate": right[name],
            "difference": right[name] - left[name],
        }
        for name in dimensions
    }
    result: dict[str, Any] = {
        "schema": VENTUSAMPLE_COMPARISON_SCHEMA,
        "mode": "read_only",
        "reference": left_meta,
        "candidate": right_meta,
        "same_style": left_meta["style"] == right_meta["style"],
        "same_target": left_meta["target"] == right_meta["target"],
        "same_key_and_scale": (
            left_meta["key"] == right_meta["key"] and left_meta["scale"] == right_meta["scale"]
        ),
        "same_sampler": left_meta["sampler"] == right_meta["sampler"],
        "same_timbre": left_meta["timbre"] == right_meta["timbre"],
        "same_seed": left_meta["seed"] == right_meta["seed"],
        "dimensions": dimensions,
        "dimension_count": len(dimensions),
        "euclidean_distance": distance,
        "distance_backend": "python-standard-library",
        "components": components,
    }
    canonical = json.dumps(
        result, ensure_ascii=False, sort_keys=True, separators=(",", ":"), allow_nan=False
    ).encode("utf-8")
    result["report_sha256"] = hashlib.sha256(canonical).hexdigest()
    return result
