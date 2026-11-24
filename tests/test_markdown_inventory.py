"""Regression tests for repository-wide Markdown integrity validation."""
from __future__ import annotations
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from validate_markdown_inventory import markdown_anchors, slugify_heading, validate_repository, validate_target


class MarkdownInventoryTests(unittest.TestCase):
    def test_heading_slugs_support_unicode_and_duplicates(self):
        anchors = markdown_anchors("# Українська документація\n\n## Start Here\n\n## Start Here\n")
        self.assertIn("українська-документація", anchors)
        self.assertIn("start-here", anchors)
        self.assertIn("start-here-1", anchors)

    def test_code_fences_are_not_treated_as_links(self):
        fence = chr(96) * 3
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "README.md").write_text(
                "# Home\n\n" + fence + "md\n[example](missing.md)\n" + fence + "\n\n[real](target.md)\n",
                encoding="utf-8",
            )
            (root / "target.md").write_text("# Target\n", encoding="utf-8")
            files, links, errors = validate_repository(root)
            self.assertEqual((files, links, errors), (2, 1, []))

    def test_detects_missing_targets_and_fragments(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "README.md").write_text("# Home\n[missing](absent.md)\n[bad](target.md#not-here)\n", encoding="utf-8")
            (root / "target.md").write_text("# Present\n", encoding="utf-8")
            _, _, errors = validate_repository(root)
            self.assertEqual(len(errors), 2)
            self.assertTrue(any("broken local target" in e for e in errors))
            self.assertTrue(any("unknown Markdown fragment" in e for e in errors))

    def test_rejects_links_that_escape_repository(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp) / "repo"
            root.mkdir()
            (Path(temp) / "secret.md").write_text("# Outside\n", encoding="utf-8")
            source = root / "README.md"
            source.write_text("# Home\n[escape](../secret.md)\n", encoding="utf-8")
            self.assertIn("escapes repository root", validate_target(root, source, "../secret.md"))

    def test_directory_links_require_an_index_document(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "README.md").write_text("# Home\n[docs](docs/)\n", encoding="utf-8")
            (root / "docs").mkdir()
            self.assertTrue(any("directory link has no" in e for e in validate_repository(root)[2]))
            (root / "docs" / "README.md").write_text("# Docs\n", encoding="utf-8")
            self.assertEqual(validate_repository(root)[2], [])

    def test_heading_slug_removes_markup(self):
        self.assertEqual(slugify_heading("**API**: [overview](api.md)"), "api-overview")


if __name__ == "__main__":
    unittest.main()
