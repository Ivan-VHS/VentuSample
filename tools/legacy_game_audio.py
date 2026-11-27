"""Compatibility planning for Windows XP-era game audio formats.

The planner describes conversions into VentuSample's float-buffer DSP path.
It does not open an audio device or claim to emulate DirectSound hardware,
EAX, or a Windows multimedia driver.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

SCHEMA = "ventusample.legacy-game-audio-plan.v1"
MAX_INPUT_BYTES = 1024 * 1024
LEGACY_APIS = {"directsound", "winmm", "eax", "openal"}
LEGACY_RATES = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000}
LEGACY_BITS = {8, 16, 24, 32}
LEGACY_CHANNELS = {1, 2, 4, 6, 8}


def _integer(value: Any, name: str, allowed: set[int]) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value not in allowed:
        choices = ", ".join(str(item) for item in sorted(allowed))
        raise ValueError(f"{name} must be one of: {choices}")
    return value


def plan_legacy_game_audio(
    request: dict[str, Any],
    host: dict[str, Any] | None = None,
) -> dict[str, Any]:
    if not isinstance(request, dict):
        raise ValueError("audio request must be an object")
    api = request.get("api")
    if not isinstance(api, str) or api.casefold() not in LEGACY_APIS:
        raise ValueError(f"api must be one of: {', '.join(sorted(LEGACY_APIS))}")
    api = api.casefold()
    sample_rate = _integer(request.get("sample_rate", 44100), "sample_rate", LEGACY_RATES)
    bits = _integer(request.get("bits_per_sample", 16), "bits_per_sample", LEGACY_BITS)
    channels = _integer(request.get("channels", 2), "channels", LEGACY_CHANNELS)
    wants_eax = request.get("eax_enabled", api == "eax")
    if not isinstance(wants_eax, bool):
        raise ValueError("eax_enabled must be a boolean")

    host = host or {}
    if not isinstance(host, dict):
        raise ValueError("host capabilities must be an object")
    raw_rates = host.get("sample_rates", [44100, 48000])
    if not isinstance(raw_rates, list) or not raw_rates:
        raise ValueError("host.sample_rates must be a non-empty list")
    rates = sorted({_integer(value, "host.sample_rates[]", LEGACY_RATES) for value in raw_rates})
    max_channels = host.get("max_channels", 2)
    if isinstance(max_channels, bool) or not isinstance(max_channels, int) or max_channels not in {1, 2, 4, 6, 8}:
        raise ValueError("host.max_channels must be 1, 2, 4, 6 or 8")
    eax_available = host.get("eax_available", False)
    if not isinstance(eax_available, bool):
        raise ValueError("host.eax_available must be a boolean")

    target_rate = min(rates, key=lambda value: (abs(value - sample_rate), value))
    warnings: list[str] = []
    if target_rate != sample_rate:
        warnings.append("The requested sample rate needs resampling before it reaches the host audio device.")
    if channels > max_channels:
        warnings.append("The requested channel layout exceeds the declared host channel count; a downmix is required.")
    if wants_eax and not eax_available:
        warnings.append("EAX hardware effects are unavailable in the declared host capabilities.")
    if api == "directsound":
        warnings.append("DirectSound calls need a runtime audio bridge; this plan covers format conversion only.")
    elif api == "winmm":
        warnings.append("WinMM waveform output needs a runtime audio bridge; this plan covers format conversion only.")
    elif api == "openal":
        warnings.append("OpenAL device/context creation is outside this format planner.")

    return {
        "schema": SCHEMA,
        "source": {
            "api": api, "sample_rate": sample_rate,
            "bits_per_sample": bits, "channels": channels,
            "eax_enabled": wants_eax,
        },
        "target": {
            "buffer_format": "float32",
            "sample_rate": target_rate,
            "channels": min(channels, max_channels),
            "maximum_host_channels": max_channels,
            "eax_available": eax_available,
        },
        "conversion": {
            "resampling_required": target_rate != sample_rate,
            "sample_format_conversion_required": bits != 32,
            "downmix_required": channels > max_channels,
            "eax_emulation_available": bool(wants_eax and eax_available),
        },
        "warnings": warnings,
        "status": "attention_required" if warnings else "format_compatible",
        "device_opened": False,
        "audio_played": False,
        "windows_api_emulated": False,
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Plan legacy game-audio conversion into VentuSample float buffers.")
    parser.add_argument("request", type=Path, help="JSON object with API and legacy audio format")
    parser.add_argument("--host", type=Path, help="optional JSON host audio capability object")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args(argv)
    try:
        source = args.request.expanduser()
        if source.is_symlink() or not source.is_file() or source.stat().st_size > MAX_INPUT_BYTES:
            raise ValueError("request must be a regular JSON file no larger than 1 MiB")
        request = json.loads(source.read_text(encoding="utf-8"))
        host = None
        if args.host:
            host_path = args.host.expanduser()
            if host_path.is_symlink() or not host_path.is_file() or host_path.stat().st_size > MAX_INPUT_BYTES:
                raise ValueError("host capabilities must be a regular JSON file no larger than 1 MiB")
            host = json.loads(host_path.read_text(encoding="utf-8"))
        result = plan_legacy_game_audio(request, host)
        encoded = json.dumps(result, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
        if args.output:
            if args.output.resolve() in {source.resolve(), args.host.resolve() if args.host else None}:
                raise ValueError("output must not overwrite an input file")
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(encoded, encoding="utf-8", newline="\n")
        else:
            print(encoded, end="")
        return 0
    except (OSError, ValueError, TypeError, json.JSONDecodeError) as error:
        print(f"ventusample-legacy-audio: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
