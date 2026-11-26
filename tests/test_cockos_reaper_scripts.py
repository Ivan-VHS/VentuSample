from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1];DIR=ROOT/"integrations"/"cockos-reaper"/"kora"
class CockosReaScriptTests(unittest.TestCase):
 def test_three_reaper_actions(self):
  files=sorted(DIR.glob("*.lua"));self.assertEqual(len(files),3)
  for p in files:
   s=p.read_text(encoding="utf-8");self.assertIn("-- @description",s);self.assertIn("reaper",s)
   self.assertNotIn("os.execute",s);self.assertNotIn("io.popen",s)
 def test_mutation_is_undoable(self):
  s=(DIR/"Kora_Prefix_Selected_Tracks.lua").read_text()
  self.assertIn("Undo_BeginBlock",s);self.assertIn("Undo_EndBlock",s)
 def test_read_only_apps(self):
  for n in ("Kora_Track_Health.lua","Kora_Marker_Region_Report.lua"):
   self.assertNotIn("Undo_BeginBlock",(DIR/n).read_text())
if __name__=="__main__":unittest.main()
