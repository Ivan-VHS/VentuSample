from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest
import zipfile

from tools.clife import ClifeError, restore, snapshot, verify


class ClifeSnapshotTests(unittest.TestCase):
    def test_snapshot_verify_and_restore_to_new_directory(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "project"
            root.mkdir()
            (root / "src").mkdir()
            (root / "src" / "main.txt").write_text("stable source\n", encoding="utf-8")
            (root / "notes.txt").write_text("project notes\n", encoding="utf-8")
            (root / ".git").mkdir()
            (root / ".git" / "secret").write_text("excluded", encoding="utf-8")

            archive = Path(temporary) / "checkpoint.clife"
            created = snapshot(root, archive)
            self.assertEqual(created["file_count"], 2)
            checked = verify(archive)
            self.assertTrue(checked["verified"])
            self.assertEqual(checked["total_bytes"], len(b"stable source\n") + len(b"project notes\n"))

            target = Path(temporary) / "recovered"
            result = restore(archive, target)
            self.assertFalse(result["overwrote_existing_files"])
            self.assertEqual((target / "src" / "main.txt").read_text(encoding="utf-8"), "stable source\n")
            self.assertEqual((target / "notes.txt").read_text(encoding="utf-8"), "project notes\n")
            self.assertFalse((target / ".git").exists())

    def test_refuses_to_overwrite_existing_restore_directory(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "project"
            root.mkdir()
            (root / "file.txt").write_text("safe", encoding="utf-8")
            archive = Path(temporary) / "checkpoint.clife"
            snapshot(root, archive)
            target = Path(temporary) / "existing"
            target.mkdir()
            (target / "keep.txt").write_text("keep", encoding="utf-8")
            with self.assertRaises(ClifeError):
                restore(archive, target)
            self.assertEqual((target / "keep.txt").read_text(encoding="utf-8"), "keep")

    def test_detects_modified_archive_member(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "project"
            root.mkdir()
            (root / "file.txt").write_text("original", encoding="utf-8")
            archive = Path(temporary) / "checkpoint.clife"
            snapshot(root, archive)
            changed = Path(temporary) / "changed.clife"
            with zipfile.ZipFile(archive, "r") as source, zipfile.ZipFile(changed, "w") as output:
                for name in source.namelist():
                    data = source.read(name)
                    if name == "files/file.txt":
                        data = b"tampered"
                    output.writestr(name, data)
            with self.assertRaises(ClifeError):
                verify(changed)

    def test_rejects_path_traversal_manifest(self):
        with tempfile.TemporaryDirectory() as temporary:
            archive = Path(temporary) / "malicious.clife"
            manifest = {
                "schema": "clife.snapshot.v1",
                "created_at": "2026-01-01T00:00:00+00:00",
                "root_name": "bad",
                "file_count": 1,
                "total_bytes": 1,
                "entries": [{
                    "path": "../outside.txt", "size": 1,
                    "sha256": "0" * 64, "mode": 420
                }],
            }
            with zipfile.ZipFile(archive, "w") as output:
                output.writestr("manifest.json", json.dumps(manifest))
                output.writestr("files/../outside.txt", b"x")
            with self.assertRaises(ClifeError):
                verify(archive)

    def test_excludes_clife_output_and_generated_directories(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "project"
            (root / ".clife").mkdir(parents=True)
            (root / "build").mkdir()
            (root / "src").mkdir()
            (root / ".clife" / "old.clife").write_bytes(b"old")
            (root / "build" / "generated.bin").write_bytes(b"generated")
            (root / "src" / "code.txt").write_text("code", encoding="utf-8")
            archive = Path(temporary) / "checkpoint.clife"
            result = snapshot(root, archive)
            self.assertEqual(result["file_count"], 1)
            self.assertTrue(verify(archive)["verified"])


if __name__ == "__main__":
    unittest.main()
