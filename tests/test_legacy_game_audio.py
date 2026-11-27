from __future__ import annotations

import unittest

from tools.legacy_game_audio import plan_legacy_game_audio


class LegacyGameAudioTests(unittest.TestCase):
    def test_directsound_44100_stereo_can_use_stereo_float_buffer(self):
        report = plan_legacy_game_audio({
            "api": "directsound", "sample_rate": 44100,
            "bits_per_sample": 16, "channels": 2,
        }, {"sample_rates": [44100, 48000], "max_channels": 2})
        self.assertEqual(report["schema"], "ventusample.legacy-game-audio-plan.v1")
        self.assertEqual(report["target"]["buffer_format"], "float32")
        self.assertFalse(report["conversion"]["resampling_required"])
        self.assertFalse(report["conversion"]["downmix_required"])
        self.assertFalse(report["device_opened"])
        self.assertFalse(report["windows_api_emulated"])

    def test_resampling_and_downmix_are_reported_not_performed(self):
        report = plan_legacy_game_audio({
            "api": "winmm", "sample_rate": 22050,
            "bits_per_sample": 8, "channels": 6,
        }, {"sample_rates": [44100, 48000], "max_channels": 2})
        self.assertTrue(report["conversion"]["resampling_required"])
        self.assertTrue(report["conversion"]["sample_format_conversion_required"])
        self.assertTrue(report["conversion"]["downmix_required"])
        self.assertEqual(report["target"]["sample_rate"], 44100)
        self.assertEqual(report["target"]["channels"], 2)

    def test_eax_is_only_reported_when_host_declares_it(self):
        missing = plan_legacy_game_audio({"api": "eax", "eax_enabled": True})
        available = plan_legacy_game_audio(
            {"api": "eax", "eax_enabled": True},
            {"sample_rates": [44100], "max_channels": 2, "eax_available": True},
        )
        self.assertFalse(missing["conversion"]["eax_emulation_available"])
        self.assertTrue(available["conversion"]["eax_emulation_available"])

    def test_rejects_invalid_legacy_format(self):
        with self.assertRaises(ValueError):
            plan_legacy_game_audio({"api": "directsound", "sample_rate": 12345})
        with self.assertRaises(ValueError):
            plan_legacy_game_audio({"api": "unknown"})
        with self.assertRaises(ValueError):
            plan_legacy_game_audio({"api": "winmm"}, {"sample_rates": []})


if __name__ == "__main__":
    unittest.main()
