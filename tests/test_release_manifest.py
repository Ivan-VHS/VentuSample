"""Tests for deterministic VentuSample release-manifest generation."""
from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from tools.create_release_manifest import create_manifest


class ReleaseManifestTests(unittest.TestCase):
    VALID_COMMIT = "a" * 40

    def test_manifest_is_deterministic_and_excludes_itself(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "VentuSample.exe").write_bytes(b"app-binary")
            (root / "README.md").write_text("VentuSample\n", encoding="utf-8")

            first = create_manifest(root, "v1.2.3", self.VALID_COMMIT)
            first_bytes = (root / "release-manifest.json").read_bytes()
            second = create_manifest(root, "v1.2.3", self.VALID_COMMIT)
            second_bytes = (root / "release-manifest.json").read_bytes()

            self.assertEqual(first, second)
            self.assertEqual(first_bytes, second_bytes)
            self.assertEqual([f["path"] for f in first["files"]], ["README.md", "VentuSample.exe"])
            self.assertNotIn("release-manifest.json", [f["path"] for f in first["files"]])
            self.assertEqual(first["source"]["commit"], self.VALID_COMMIT)

    def test_file_modification_changes_digest_and_size(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            target = root / "sample.dat"
            target.write_bytes(b"abc")
            first = create_manifest(root, "v1.2.3", self.VALID_COMMIT)
            target.write_bytes(b"abcd")
            second = create_manifest(root, "v1.2.3", self.VALID_COMMIT)

            self.assertNotEqual(first["files"][0]["sha256"], second["files"][0]["sha256"])
            self.assertEqual(second["files"][0]["size"], 4)

    def test_rejects_invalid_version(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "app.exe").write_bytes(b"app")
            with self.assertRaisesRegex(ValueError, "v-prefixed semantic version"):
                create_manifest(root, "1.2.3", self.VALID_COMMIT)

    def test_rejects_invalid_commit(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "app.exe").write_bytes(b"app")
            with self.assertRaisesRegex(ValueError, "40-character Git SHA"):
                create_manifest(root, "v1.2.3", "bad")

    def test_rejects_symlink_when_platform_supports_it(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            outside = Path(temp).parent / (Path(temp).name + "-outside")
            outside.write_bytes(b"outside")
            try:
                (root / "outside-link").symlink_to(outside)
            except (OSError, NotImplementedError):
                self.skipTest("This platform does not permit symlink creation in the test environment")
            try:
                with self.assertRaisesRegex(ValueError, "symbolic links"):
                    create_manifest(root, "v1.2.3", self.VALID_COMMIT)
            finally:
                outside.unlink(missing_ok=True)


if __name__ == "__main__":
    unittest.main()
