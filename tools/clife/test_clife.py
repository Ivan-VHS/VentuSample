import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

SPEC = importlib.util.spec_from_file_location("clife", Path(__file__).with_name("clife.py"))
clife = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(clife)


class CLifeTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        (self.root / "src").mkdir()
        (self.root / "tests").mkdir()
        (self.root / ".clife").mkdir()
        (self.root / ".clife" / "config.json").write_text(json.dumps({
            "schema_version": 1,
            "repository": "fixture",
            "max_file_bytes": 1024 * 1024,
        }), encoding="utf-8")
        (self.root / "src" / "engine.py").write_text("VALUE = 1\n", encoding="utf-8")
        (self.root / "README.md").write_text("initial\n", encoding="utf-8")
        (self.root / ".env").write_text("TOKEN=do-not-copy\n", encoding="utf-8")
        (self.root / "src" / "binary.dat").write_bytes(b"\x00\x01")

    def tearDown(self):
        self.temp.cleanup()

    def test_savepoint_verifies_and_excludes_secrets_and_binary(self):
        snapshot = clife.make_snapshot(self.root, "test")
        count, errors = clife.verify_snapshot(self.root, snapshot)
        self.assertEqual(errors, [])
        self.assertEqual(count, 2)
        _, manifest = clife._load_manifest(self.root, snapshot)
        paths = {item["path"] for item in manifest["files"]}
        self.assertEqual(paths, {"README.md", "src/engine.py"})

    def test_preview_does_not_change_worktree(self):
        snapshot = clife.make_snapshot(self.root)
        (self.root / "src" / "engine.py").write_text("VALUE = 2\n", encoding="utf-8")
        actions, safety = clife.restore_snapshot(self.root, snapshot, ["src/engine.py"], apply=False)
        self.assertIsNone(safety)
        self.assertTrue(actions[0].startswith("restore"))
        self.assertEqual((self.root / "src" / "engine.py").read_text(encoding="utf-8"), "VALUE = 2\n")

    def test_stitch_applies_and_creates_pre_restore_savepoint(self):
        snapshot = clife.make_snapshot(self.root)
        (self.root / "src" / "engine.py").write_text("BROKEN\n", encoding="utf-8")
        actions, safety = clife.restore_snapshot(self.root, snapshot, ["src/engine.py"], apply=True)
        self.assertIsNotNone(safety)
        self.assertTrue(actions[0].startswith("restore"))
        self.assertEqual((self.root / "src" / "engine.py").read_text(encoding="utf-8"), "VALUE = 1\n")
        self.assertEqual(clife.verify_snapshot(self.root, safety)[1], [])

    def test_revive_recreates_missing_files_and_keeps_extra_files(self):
        snapshot = clife.make_snapshot(self.root)
        (self.root / "README.md").unlink()
        (self.root / "extra.txt").write_text("keep me\n", encoding="utf-8")
        actions, safety = clife.restore_snapshot(self.root, snapshot, apply=True)
        self.assertIsNotNone(safety)
        self.assertTrue(any(action.startswith("recreate  README.md") for action in actions))
        self.assertEqual((self.root / "README.md").read_text(encoding="utf-8"), "initial\n")
        self.assertTrue((self.root / "extra.txt").exists())

    def test_manifest_tampering_is_detected(self):
        snapshot = clife.make_snapshot(self.root)
        manifest = self.root / ".clife" / "snapshots" / snapshot / "manifest.json"
        manifest.write_text(manifest.read_text(encoding="utf-8") + " ", encoding="utf-8")
        with self.assertRaises(clife.CLifeError):
            clife.verify_snapshot(self.root, snapshot)

    def test_path_traversal_is_rejected(self):
        with self.assertRaises(clife.CLifeError):
            clife.restore_snapshot(self.root, "x", ["../../outside"], apply=False)

    def test_symlink_restore_is_refused(self):
        snapshot = clife.make_snapshot(self.root)
        external = self.root.parent / (self.root.name + "-outside.txt")
        external.write_text("outside", encoding="utf-8")
        link = self.root / "outside-link.txt"
        try:
            link.symlink_to(external)
        except (OSError, NotImplementedError):
            external.unlink(missing_ok=True)
            self.skipTest("symlinks are unavailable on this platform")
        try:
            with self.assertRaises(clife.CLifeError):
                clife.restore_snapshot(self.root, snapshot, ["outside-link.txt"], apply=True)
        finally:
            link.unlink(missing_ok=True)
            external.unlink(missing_ok=True)


if __name__ == "__main__":
    unittest.main(verbosity=2)
