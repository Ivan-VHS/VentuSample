# Copyright (c) 2026 Ivan-VHS
# SPDX-License-Identifier: Apache-2.0
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import json
import tempfile
import unittest
import wave
from ventu_wav_inspect import _decode_pcm, compare_reports, inspect_wav

def write_wav(path: Path, samples: list[tuple[int, ...]], channels: int = 2, width: int = 2, rate: int = 48_000) -> None:
    with wave.open(str(path), "wb") as wav:
        wav.setnchannels(channels)
        wav.setsampwidth(width)
        wav.setframerate(rate)
        payload = bytearray()
        for frame in samples:
            for value in frame:
                payload.extend(int(value).to_bytes(width, "little", signed=(width != 1)))
        wav.writeframes(bytes(payload))

class VentuWavInspectTests(unittest.TestCase):
    def test_pcm16_decoding_handles_negative_full_scale(self):
        values = _decode_pcm(b"\x00\x80\xff\x7f", 2)
        self.assertEqual(values[0], -1.0)
        self.assertGreater(values[1], 0.99)

    def test_pcm24_sign_extension(self):
        self.assertEqual(_decode_pcm(b"\x00\x00\x80", 3), [-1.0])

    def test_stereo_meter_reports_correlation_mid_side_and_mono_peak(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "stereo.wav"
            write_wav(path, [(8192, 8192), (-8192, -8192), (4096, 4096), (0, 0)])
            before = path.read_bytes()
            report = inspect_wav(path)
            self.assertEqual(report["schema"], "ventusample.meter-report.v2")
            self.assertEqual(report["frames"], 4)
            self.assertEqual(report["stereo"]["correlation"], 1.0)
            self.assertIsNone(report["stereo"]["side_to_mid_db"])
            self.assertEqual(report["stereo"]["mono_fold_down_peak_dbfs"], report["channels_report"][0]["peak_dbfs"])
            self.assertEqual(path.read_bytes(), before)
            json.dumps(report)

    def test_antiphase_stereo_reports_negative_correlation(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "phase.wav"
            write_wav(path, [(8192, -8192), (-8192, 8192), (4096, -4096), (0, 0)])
            report = inspect_wav(path)
            self.assertEqual(report["stereo"]["correlation"], -1.0)
            self.assertIsNone(report["stereo"]["mono_fold_down_peak_dbfs"])

    def test_compare_reports_channel_deltas_and_format_warnings(self):
        with tempfile.TemporaryDirectory() as directory:
            reference_path = Path(directory) / "reference.wav"
            candidate_path = Path(directory) / "candidate.wav"
            write_wav(reference_path, [(4096, 4096), (-4096, -4096)])
            write_wav(candidate_path, [(8192, 8192), (-8192, -8192)])
            comparison = compare_reports(inspect_wav(reference_path), inspect_wav(candidate_path))
            self.assertTrue(comparison["format_compatible"])
            self.assertEqual(comparison["channel_deltas"][0]["rms_delta_db"], 6.02)
            self.assertEqual(comparison["frame_delta"], 0)

    def test_rejects_non_wav_content(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "bad.wav"
            path.write_bytes(b"not a wav file" * 10)
            with self.assertRaises(wave.Error):
                inspect_wav(path)

if __name__ == "__main__":
    unittest.main()
