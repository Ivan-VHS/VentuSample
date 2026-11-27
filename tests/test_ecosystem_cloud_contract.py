import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ADAPTER = json.loads(
    (ROOT / "federation" / "ecosystem-cloud-adapter.v1.json").read_text(encoding="utf-8")
)


class EcosystemCloudAdapterContractTests(unittest.TestCase):
    def test_contract_identity_and_native_boundaries(self):
        self.assertEqual(ADAPTER["schema"], "ivan-vhs.ecosystem-cloud-adapter.v1")
        self.assertEqual(ADAPTER["version"], 1)
        self.assertEqual(ADAPTER["project"], "ventusample")
        self.assertEqual(ADAPTER["project_key"], "ventusample")
        self.assertEqual(ADAPTER["shared_contract"], "ivan-vhs.ecosystem-cloud.v1")
        self.assertEqual(ADAPTER["status"], "contract-only")
        self.assertFalse(ADAPTER["compatibility"]["sdk_dependency"])

    def test_audio_engine_remains_offline(self):
        self.assertEqual(ADAPTER["runtime"]["default_network_mode"], "disabled")
        self.assertTrue(ADAPTER["runtime"]["user_opt_in_required"])
        self.assertFalse(ADAPTER["runtime"]["core_requires_network"])
        self.assertEqual(ADAPTER["media"]["local_rendering"], "always-available")
        self.assertEqual(ADAPTER["media"]["cloud_rendering"], "not-implemented")

    def test_audio_and_plugin_sources_are_not_synced_by_default(self):
        denied = set(ADAPTER["denylist"])
        for value in (
            "audio-recordings-by-default",
            "source-sessions",
            "raw-waveform-captures",
            "plugin-binaries",
            "private-plugin-state",
            "access-tokens",
            "service-role-keys",
        ):
            with self.subTest(value=value):
                self.assertIn(value, denied)
        self.assertEqual(ADAPTER["media"]["upload_audio"], "explicit-publish-only")
        self.assertTrue(ADAPTER["synchronization"]["upload_requires_user_action"])


if __name__ == "__main__":
    unittest.main()
