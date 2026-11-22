from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SCRIPTS = ROOT / "integrations" / "cockos-reaper"


class CockosReaScriptTests(unittest.TestCase):
    def test_three_host_actions_are_packaged(self):
        scripts = sorted(SCRIPTS.glob("*.lua"))
        self.assertEqual(len(scripts), 3)
        for path in scripts:
            source = path.read_text(encoding="utf-8")
            self.assertIn("-- @description", source)
            self.assertIn("reaper", source)
            self.assertNotIn("os.execute", source)
            self.assertNotIn("io.popen", source)

    def test_track_rename_is_one_undoable_operation(self):
        source = (SCRIPTS / "Kora_Prefix_Selected_Tracks.lua").read_text(encoding="utf-8")
        self.assertIn("Undo_BeginBlock", source)
        self.assertIn("Undo_EndBlock", source)

    def test_read_only_actions_do_not_open_undo_blocks(self):
        for name in ("Kora_Track_Health.lua", "Kora_Marker_Region_Report.lua"):
            source = (SCRIPTS / name).read_text(encoding="utf-8")
            self.assertNotIn("Undo_BeginBlock", source)


if __name__ == "__main__":
    unittest.main()
