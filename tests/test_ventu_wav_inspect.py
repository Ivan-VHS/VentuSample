# Copyright (c) 2026 Ivan-VHS
# SPDX-License-Identifier: Apache-2.0
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import json
import tempfile
import unittest
import wave
from ventu_wav_inspect import _decode_pcm, inspect_wav

class VentuWavInspectTests(unittest.TestCase):
    def test_pcm16_decoding_handles_negative_full_scale(self):
        values = _decode_pcm(b"\x00\x80\xff\x7f", 2, 1)
        self.assertEqual(values[0], -1.0)
        self.assertGreater(values[1], 0.99)

    def test_pcm24_sign_extension(self):
        self.assertEqual(_decode_pcm(b"\x00\x00\x80", 3, 1), [-1.0])

    def test_reports_channel_metrics_without_modifying_input(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "tone.wav"
            samples = b"".join(int(value).to_bytes(2, "little", signed=True) for value in (0, 8192, -8192, 0))
            with wave.open(str(path), "wb") as wav:
                wav.setnchannels(1)
                wav.setsampwidth(2)
                wav.setframerate(48_000)
                wav.writeframes(samples)
            before = path.read_bytes()
            report = inspect_wav(path)
            self.assertEqual(report["schema"], "ventusample.wav-inspection.v1")
            self.assertEqual(report["frames"], 4)
            self.assertEqual(report["channels_report"][0]["near_full_scale_samples"], 0)
            self.assertEqual(path.read_bytes(), before)
            json.dumps(report)

    def test_rejects_non_wav_content(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "bad.wav"
            path.write_bytes(b"not a wav file" * 10)
            with self.assertRaises(wave.Error):
                inspect_wav(path)

if __name__ == "__main__":
    unittest.main()
