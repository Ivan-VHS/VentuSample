#!/usr/bin/env python3
# Copyright (c) 2026 Ivan-VHS
# SPDX-License-Identifier: Apache-2.0
"""VentuSample Meter Lab: bounded streaming PCM analysis and A/B comparison."""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
import sys
import wave
from pathlib import Path
from typing import Any

MAX_FILE_BYTES = 2 * 1024 * 1024 * 1024
BLOCK_FRAMES = 16_384
NEAR_FULL_SCALE = 0.999969


def _decode_pcm(block: bytes, width: int, sample_format: int = 1) -> list[float]:
    if sample_format != 1:
        raise ValueError(f"unsupported WAV format tag {sample_format}")
    if width == 1:
        return [(sample - 128) / 128.0 for sample in block]
    if width == 2:
        if len(block) % 2:
            raise ValueError("PCM16 data chunk ends inside a sample")
        return [sample[0] / 32768.0 for sample in struct.iter_unpack("<h", block)]
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
        return [sample[0] / 2_147_483_648.0 for sample in struct.iter_unpack("<i", block)]
    raise ValueError(f"unsupported PCM sample width: {width} bytes")


def _dbfs(value: float) -> float | None:
    return round(20.0 * math.log10(value), 2) if value > 0 else None


def _file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while chunk := stream.read(1024 * 1024):
            digest.update(chunk)
    return digest.hexdigest()


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
        if width not in {1, 2, 3, 4}:
            raise ValueError("only 8-, 16-, 24- and 32-bit integer PCM are supported")
        if wav.getcomptype() != "NONE":
            raise ValueError(f"compressed WAV is not supported: {wav.getcomptype()}")

        channel_sum = [0.0] * channels
        channel_squares = [0.0] * channels
        channel_peak = [0.0] * channels
        channel_peak_frame: list[int | None] = [None] * channels
        channel_near_full_scale = [0] * channels
        total_samples = 0
        frames_read = 0

        stereo_cross_sum = 0.0
        mid_squares = 0.0
        side_squares = 0.0
        mono_peak = 0.0
        mono_near_full_scale = 0

        while block := wav.readframes(BLOCK_FRAMES):
            values = _decode_pcm(block, width)
            if len(values) % channels:
                raise ValueError("audio block does not contain complete sample frames")
            frame_count = len(values) // channels
            for frame in range(frame_count):
                frame_base = frame * channels
                absolute_frame = frames_read + frame
                for channel in range(channels):
                    value = values[frame_base + channel]
                    magnitude = abs(value)
                    channel_sum[channel] += value
                    channel_squares[channel] += value * value
                    if magnitude > channel_peak[channel]:
                        channel_peak[channel] = magnitude
                        channel_peak_frame[channel] = absolute_frame
                    if magnitude >= NEAR_FULL_SCALE:
                        channel_near_full_scale[channel] += 1

                if channels >= 2:
                    left, right = values[frame_base], values[frame_base + 1]
                    mid = (left + right) * 0.5
                    side = (left - right) * 0.5
                    stereo_cross_sum += left * right
                    mid_squares += mid * mid
                    side_squares += side * side
                    mono_peak = max(mono_peak, abs(mid))
                    if abs(mid) >= NEAR_FULL_SCALE:
                        mono_near_full_scale += 1

            total_samples += len(values)
            frames_read += frame_count

    if frames_read != frames:
        raise ValueError(f"WAV declared {frames} frames but supplied {frames_read}")

    channel_reports = []
    channel_rms_values: list[float] = []
    channel_means: list[float] = []
    sample_count = frames
    for channel in range(channels):
        mean = channel_sum[channel] / sample_count if sample_count else 0.0
        rms = math.sqrt(channel_squares[channel] / sample_count) if sample_count else 0.0
        peak = channel_peak[channel]
        channel_rms_values.append(rms)
        channel_means.append(mean)
        channel_reports.append({
            "channel": channel + 1,
            "peak_dbfs": _dbfs(peak),
            "peak_linear": round(peak, 8),
            "peak_frame": channel_peak_frame[channel],
            "rms_dbfs": _dbfs(rms),
            "dc_offset": round(mean, 8),
            "crest_factor_db": round(20.0 * math.log10(peak / rms), 2) if peak > 0 and rms > 0 else None,
            "near_full_scale_samples": channel_near_full_scale[channel],
        })

    stereo_report = None
    if channels >= 2 and frames:
        left_mean, right_mean = channel_means[0], channel_means[1]
        left_var = max(0.0, channel_squares[0] / frames - left_mean * left_mean)
        right_var = max(0.0, channel_squares[1] / frames - right_mean * right_mean)
        covariance = stereo_cross_sum / frames - left_mean * right_mean
        denominator = math.sqrt(left_var * right_var)
        correlation = max(-1.0, min(1.0, covariance / denominator)) if denominator > 0 else None
        mid_rms = math.sqrt(mid_squares / frames)
        side_rms = math.sqrt(side_squares / frames)
        stereo_report = {
            "correlation": round(correlation, 5) if correlation is not None else None,
            "mid_rms_dbfs": _dbfs(mid_rms),
            "side_rms_dbfs": _dbfs(side_rms),
            "side_to_mid_db": round(20.0 * math.log10(side_rms / mid_rms), 2) if side_rms > 0 and mid_rms > 0 else None,
            "mono_fold_down_peak_dbfs": _dbfs(mono_peak),
            "mono_fold_down_near_full_scale_samples": mono_near_full_scale,
        }

    peak_db_values = [item["peak_dbfs"] for item in channel_reports if item["peak_dbfs"] is not None]
    rms_db_values = [item["rms_dbfs"] for item in channel_reports if item["rms_dbfs"] is not None]
    return {
        "schema": "ventusample.meter-report.v2",
        "file": file_path.name,
        "sha256": _file_sha256(file_path),
        "size_bytes": size,
        "encoding": "integer_pcm",
        "channels": channels,
        "sample_rate_hz": sample_rate,
        "sample_width_bits": width * 8,
        "frames": frames,
        "duration_seconds": round(frames / sample_rate, 6),
        "inspected_samples": total_samples,
        "peak_spread_db": round(max(peak_db_values) - min(peak_db_values), 2) if len(peak_db_values) > 1 else 0.0,
        "rms_spread_db": round(max(rms_db_values) - min(rms_db_values), 2) if len(rms_db_values) > 1 else 0.0,
        "near_full_scale_sample_count": sum(channel_near_full_scale),
        "channels_report": channel_reports,
        "stereo": stereo_report,
        "limitations": [
            "Peak is sample-peak, not oversampled true peak.",
            "RMS is a linear sample-energy measure, not EBU R128 integrated loudness or LUFS.",
            "No audio is altered, normalized, uploaded or played.",
        ],
    }


def compare_reports(reference: dict[str, Any], candidate: dict[str, Any]) -> dict[str, Any]:
    compatible = (
        reference["channels"] == candidate["channels"]
        and reference["sample_rate_hz"] == candidate["sample_rate_hz"]
        and reference["sample_width_bits"] == candidate["sample_width_bits"]
    )
    channel_deltas = []
    for index, (left, right) in enumerate(zip(reference["channels_report"], candidate["channels_report"]), start=1):
        reference_rms, candidate_rms = left["rms_dbfs"], right["rms_dbfs"]
        reference_peak, candidate_peak = left["peak_dbfs"], right["peak_dbfs"]
        channel_deltas.append({
            "channel": index,
            "rms_delta_db": round(candidate_rms - reference_rms, 2)
            if candidate_rms is not None and reference_rms is not None else None,
            "peak_delta_db": round(candidate_peak - reference_peak, 2)
            if candidate_peak is not None and reference_peak is not None else None,
            "dc_offset_delta": round(right["dc_offset"] - left["dc_offset"], 8),
        })
    warnings = []
    if not compatible:
        warnings.append("format_mismatch_metrics_are_not_level_aligned")
    if reference["frames"] != candidate["frames"]:
        warnings.append("duration_or_frame_count_differs")
    return {
        "schema": "ventusample.meter-comparison.v1",
        "reference": {"file": reference["file"], "sha256": reference["sha256"]},
        "candidate": {"file": candidate["file"], "sha256": candidate["sha256"]},
        "format_compatible": compatible,
        "frame_delta": candidate["frames"] - reference["frames"],
        "duration_delta_seconds": round(candidate["duration_seconds"] - reference["duration_seconds"], 6),
        "channel_deltas": channel_deltas,
        "warnings": warnings,
        "limitations": ["No time alignment, loudness matching or perceptual quality score is performed."],
    }


def render_table(report: dict[str, Any]) -> str:
    lines = [
        f"{report['file']}: {report['channels']} ch · {report['sample_rate_hz']} Hz · "
        f"{report['sample_width_bits']} bit · {report['duration_seconds']:.3f} s",
        "CH   PEAK dBFS   RMS dBFS   DC OFFSET       CREST dB   PEAK FRAME",
    ]
    for channel in report["channels_report"]:
        lines.append(
            f"{channel['channel']:<4} {str(channel['peak_dbfs']):<11} {str(channel['rms_dbfs']):<10} "
            f"{channel['dc_offset']:<15} {str(channel['crest_factor_db']):<10} {channel['peak_frame']}"
        )
    if report["stereo"]:
        stereo = report["stereo"]
        lines.extend([
            "",
            f"Stereo correlation: {stereo['correlation']}",
            f"Mid/Side ratio: {stereo['side_to_mid_db']} dB",
            f"Mono fold-down peak: {stereo['mono_fold_down_peak_dbfs']} dBFS",
        ])
    return "\n".join(lines)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Streaming PCM WAV meter and A/B comparison")
    parser.add_argument("file", help="WAV file to inspect")
    parser.add_argument("--compare", help="second WAV file to compare against the first")
    parser.add_argument("--json", action="store_true", help="emit machine-readable JSON")
    parser.add_argument("--output", help="write the report to this file")
    args = parser.parse_args(argv)
    try:
        reference = inspect_wav(args.file)
        if args.compare:
            report = compare_reports(reference, inspect_wav(args.compare))
        else:
            report = reference
    except (OSError, ValueError, wave.Error) as exc:
        print(f"ventu-wav-inspect: {exc}", file=sys.stderr)
        return 2

    if args.json or args.output:
        rendered = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    elif "channels_report" in report:
        rendered = render_table(report) + "\n"
    else:
        rendered = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    if args.output:
        target = Path(args.output)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(rendered, encoding="utf-8")
    else:
        sys.stdout.write(rendered)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
