from __future__ import annotations

import importlib.util
import unittest
from pathlib import Path

SCRIPT = Path(__file__).resolve().parents[1] / ".github" / "scripts" / "check_readme_parity.py"
SPEC = importlib.util.spec_from_file_location("readme_parity", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
readme_parity = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(readme_parity)


def documents(uk: str | None = None, ru: str | None = None, zh: str | None = None) -> dict[str, str]:
    fence = chr(96) * 3
    base = (
        "# Project\n\n"
        "## Limits\n\n"
        "| Setting | Value |\n"
        "| --- | ---: |\n"
        "| Maximum input | 128 MiB |\n\n"
        f"{fence}sh\n"
        "# English comment\n"
        "python app.py --limit 128\n"
        f"{fence}\n\n"
        f"{fence}text\n"
        "Input -> Core -> Report\n"
        f"{fence}\n"
    )
    return {
        "README.md": base,
        "README.uk.md": base if uk is None else uk,
        "README.ru.md": base if ru is None else ru,
        "README.zh-CN.md": base if zh is None else zh,
    }


class ReadmeParityTests(unittest.TestCase):
    def test_localized_prose_and_shell_comments_are_allowed(self) -> None:
        fence = chr(96) * 3
        translated = (
            "# Проєкт\n\n"
            "## Обмеження\n\n"
            "| Параметр | Значення |\n"
            "| --- | ---: |\n"
            "| Максимальний розмір | 128 MiB |\n\n"
            f"{fence}sh\n"
            "# Коментар українською\n"
            "python app.py --limit 128\n"
            f"{fence}\n\n"
            f"{fence}text\n"
            "Вхід -> Ядро -> Звіт\n"
            f"{fence}\n"
        )
        self.assertEqual(readme_parity.compare_documents(documents(uk=translated)), [])

    def test_numeric_table_change_is_rejected(self) -> None:
        translated = documents()["README.ru.md"].replace("128 MiB", "256 MiB")
        self.assertTrue(any("numeric values" in item for item in readme_parity.compare_documents(documents(ru=translated))))

    def test_heading_structure_change_is_rejected(self) -> None:
        translated = documents()["README.uk.md"].replace("## Limits", "### Limits")
        self.assertTrue(any("heading levels" in item for item in readme_parity.compare_documents(documents(uk=translated))))

    def test_executable_command_change_is_rejected(self) -> None:
        translated = documents()["README.zh-CN.md"].replace("--limit 128", "--limit 64")
        self.assertTrue(any("executable examples" in item for item in readme_parity.compare_documents(documents(zh=translated))))

    def test_missing_code_fence_is_rejected(self) -> None:
        fence = chr(96) * 3
        translated = documents()["README.uk.md"].replace(f"{fence}text\nInput -> Core -> Report\n{fence}\n", "")
        self.assertTrue(any("code-fence count" in item for item in readme_parity.compare_documents(documents(uk=translated))))

    def test_unclosed_code_fence_is_rejected(self) -> None:
        sample = chr(96) * 3 + "python\nprint('hello')\n"
        with self.assertRaisesRegex(ValueError, "unclosed Markdown code fence"):
            readme_parity.analyze_markdown(sample)


if __name__ == "__main__":
    unittest.main()
