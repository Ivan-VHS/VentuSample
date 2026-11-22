#!/usr/bin/env python3
# Copyright (c) 2026 Ivan-VHS
# SPDX-License-Identifier: Apache-2.0
"""Bounded, streaming WAV inspection for the VentuSample audio workflow."""
from __future__ import annotations

import argparse
import json
import math
import sys
import wave
from pathlib import Path
from typing import Any

MAX_FILE_BYTES = 2 * 1024 * 1024 * 1024
BLOCK_FRAMES = 16_384


def _decode_pcm(block: bytes, width: int, sample_format: int) -> list[float]:
    if sample_format != 1:
        raise ValueError(f"unsupported WAV format tag {sample_format}")
    if width == 1:
        return [(sample - 128) / 128.0 for sample in block]
    if width == 2:
        if len(block) % 2:
            raise ValueError("PCM16 data chunk ends inside a sample")
        return [sample[0] / 32768.0 for sample in __import__("struct").iter_unpack("<h", block)]
    if width == 3:
        if len(block) % 3:
            raise ValueError("PCM24 data chunk ends inside a sample")
        values: list[float] = []
        for offset in range(0, len(block), 3):
            raw = int.from_bytes(block[offset:offset + 3], "little", signed=False)
            if raw & 0x800000:
                raw -= 1 << 24
            values.append(raw / 8_388_608.0)
        return values
    if width == 4:
        if len(block) % 4:
            raise ValueError("PCM32 data chunk ends inside a sample")
        return [sample[0] / 2_147_483_648.0 for sample in __import__("struct").iter_unpack("<i", block)]
    raise ValueError(f"unsupported PCM sample width: {width} bytes")


def inspect_wav(path: str | Path) -> dict[str, Any]:
    file_path = Path(path)
    size = file_path.stat().st_size
    if size < 44 or size > MAX_FILE_BYTES:
        raise ValueError(f"WAV size must be between 44 bytes and {MAX_FILE_BYTES} bytes")

    with wave.open(str(file_path), "rb") as wav:
        channels, sample_rate, width, frames = (
            wav.getnchannels(), wav.getframerate(), wav.getsampwidth(), wav.getnframes()
        )
        if channels < 1 or channels > 8:
            raise ValueError("channel count must be between 1 and 8")
        if sample_rate < 1 or sample_rate > 768_000:
            raise ValueError("sample rate is outside the supported inspection range")
        if wav.getcomptype() != "NONE":
            raise ValueError(f"compressed WAV is not supported: {wav.getcomptype()}")

        channel_sum = [0.0] * channels
        channel_squares = [0.0] * channels
        channel_peak = [0.0] * channels
        channel_clipped = [0] * channels
        total_samples = 0
        while block := wav.readframes(BLOCK_FRAMES):
            values = _decode_pcm(block, width, 1)
            if len(values) % channels:
                raise ValueError("audio block does not contain complete sample frames")
            for index, value in enumerate(values):
                channel = index % channels
                channel_sum[channel] += value
                channel_squares[channel] += value * value
                magnitude = abs(value)
                channel_peak[channel] = max(channel_peak[channel], magnitude)
                if magnitude >= 0.999969:
                    channel_clipped[channel] += 1
            total_samples += len(values)

    duration = frames / sample_rate if sample_rate else 0.0
    channel_reports = []
    samples_per_channel = total_samples // channels
    for channel in range(channels):
        count = samples_per_channel
        mean = channel_sum[channel] / count if count else 0.0
        rms = math.sqrt(channel_squares[channel] / count) if count else 0.0
        peak = channel_peak[channel]
        channel_reports.append({
            "channel": channel + 1,
            "peak_dbfs": round(20 * math.log10(peak), 2) if peak > 0 else None,
            "rms_dbfs": round(20 * math.log10(rms), 2) if rms > 0 else None,
            "dc_offset": round(mean, 8),
            "crest_factor_db": round(20 * math.log10(peak / rms), 2) if peak > 0 and rms > 0 else None,
            "near_full_scale_samples": channel_clipped[channel],
        })

    return {
        "schema": "ventusample.wav-inspection.v1",
        "file": file_path.name,
        "size_bytes": size,
        "format": "PCM",
        "channels": channels,
        "sample_rate_hz": sample_rate,
        "sample_width_bits": width * 8,
        "frames": frames,
        "duration_seconds": round(duration, 6),
        "inspected_samples": total_samples,
        "channels_report": channel_reports,
        "limitations": [
            "This report measures peak, RMS, DC offset and near-full-scale samples; it does not claim BS.1770 LUFS or true-peak compliance.",
            "No audio is altered, normalized, uploaded or played.",
        ],
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Read-only PCM WAV measurement report")
    parser.add_argument("file", help="WAV file to inspect")
    parser.add_argument("--json", action="store_true", help="emit JSON")
    args = parser.parse_args(argv)
    try:
        report = inspect_wav(args.file)
    except (OSError, ValueError, wave.Error) as exc:
        print(f"ventu-wav-inspect: {exc}", file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(report, ensure_ascii=False, indent=2))
    else:
        print(f"{report['file']}: {report['channels']} ch · {report['sample_rate_hz']} Hz · {report['sample_width_bits']} bit · {report['duration_seconds']:.3f} s")
        for channel in report["channels_report"]:
            print(f"CH {channel['channel']}: peak {channel['peak_dbfs']} dBFS, RMS {channel['rms_dbfs']} dBFS, DC {channel['dc_offset']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
