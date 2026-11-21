import json
import tempfile
import unittest
from pathlib import Path

from engine import CLifeError, list_savepoints, plan_rethread, revive, rethread, savepoint, verify_snapshot


class CLifeRecoveryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        (self.root / "src").mkdir()
        (self.root / "src" / "module.py").write_text("stable = True\n", encoding="utf-8")

    def tearDown(self):
        self.temp.cleanup()

    def test_savepoint_is_content_addressed_and_verifiable(self):
        snapshot = savepoint(self.root, ["src/module.py"], label="before-rethread")
        self.assertTrue(verify_snapshot(self.root, snapshot)["verified"])
        self.assertEqual(list_savepoints(self.root)[0]["id"], snapshot)

    def test_plan_is_read_only_and_rethread_requires_apply(self):
        snapshot = savepoint(self.root, ["src/module.py"])
        target = self.root / "src" / "module.py"
        target.write_text("drifted = True\n", encoding="utf-8")
        planned = plan_rethread(self.root, snapshot)
        self.assertEqual(planned["files"][0]["status"], "changed")
        self.assertEqual(target.read_text(encoding="utf-8"), "drifted = True\n")
        preview = rethread(self.root, snapshot)
        self.assertFalse(preview["applied"])
        self.assertEqual(target.read_text(encoding="utf-8"), "drifted = True\n")

    def test_applied_rethread_checkpoints_current_state_and_restores(self):
        snapshot = savepoint(self.root, ["src/module.py"])
        target = self.root / "src" / "module.py"
        target.write_text("drifted = True\n", encoding="utf-8")
        result = rethread(self.root, snapshot, apply=True)
        self.assertTrue(result["applied"])
        self.assertEqual(target.read_text(encoding="utf-8"), "stable = True\n")
        self.assertIsNotNone(result["emergency_savepoint"])
        self.assertTrue(verify_snapshot(self.root, result["emergency_savepoint"])["verified"])

    def test_second_life_only_restores_selected_missing_path(self):
        (self.root / "legacy.py").write_text("legacy = 'still useful'\n", encoding="utf-8")
        snapshot = savepoint(self.root, ["legacy.py", "src/module.py"])
        (self.root / "legacy.py").unlink()
        preview = revive(self.root, snapshot, "legacy.py")
        self.assertEqual(preview["files"][0]["status"], "missing")
        self.assertFalse(preview["applied"])
        self.assertFalse((self.root / "legacy.py").exists())
        applied = revive(self.root, snapshot, "legacy.py", apply=True)
        self.assertTrue(applied["applied"])
        self.assertEqual((self.root / "legacy.py").read_text(encoding="utf-8"), "legacy = 'still useful'\n")

    def test_path_traversal_is_rejected(self):
        with self.assertRaises(CLifeError):
            savepoint(self.root, ["../outside.txt"])

    def test_symlink_components_are_rejected(self):
        outside = self.root.parent / (self.root.name + "-outside.txt")
        outside.write_text("do not capture", encoding="utf-8")
        link = self.root / "linked.txt"
        try:
            link.symlink_to(outside)
        except (OSError, NotImplementedError):
            outside.unlink(missing_ok=True)
            self.skipTest("symlinks are unavailable on this platform")
        try:
            with self.assertRaises(CLifeError):
                savepoint(self.root, ["linked.txt"])
        finally:
            link.unlink(missing_ok=True)
            outside.unlink(missing_ok=True)

    def test_corrupt_snapshot_object_fails_verification(self):
        snapshot = savepoint(self.root, ["src/module.py"])
        manifest = json.loads((self.root / ".clife" / "state" / "savepoints" / snapshot / "manifest.json").read_text(encoding="utf-8"))
        obj = self.root / ".clife" / "state" / "savepoints" / snapshot / manifest["files"][0]["object"]
        obj.write_bytes(b"tampered")
        with self.assertRaises(CLifeError):
            verify_snapshot(self.root, snapshot)


if __name__ == "__main__":
    unittest.main()
