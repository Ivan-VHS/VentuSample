#!/usr/bin/env python3
"""Small CTest smoke test for the shared CLife v1 savepoint tool."""
from __future__ import annotations
import importlib.util
import tempfile
from pathlib import Path

SPEC = importlib.util.spec_from_file_location("clife_under_test", Path(__file__).with_name("clife.py"))
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("cannot load tools/clife.py")
CLIFE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CLIFE)

def main() -> int:
    with tempfile.TemporaryDirectory() as folder:
        base = Path(folder)
        source = base / "source"
        source.mkdir()
        (source / "state.json").write_text('{"ok":true}', encoding="utf-8")
        archive = base / "state.clife"
        CLIFE.snapshot(source, archive)
        assert CLIFE.verify(archive)["verified"] is True
        target = base / "recovered"
        CLIFE.restore(archive, target)
        assert (target / "state.json").read_text(encoding="utf-8") == '{"ok":true}'
        try:
            CLIFE.restore(archive, target)
        except CLIFE.ClifeError:
            pass
        else:
            raise AssertionError("CLife must refuse to overwrite an existing recovery directory")
    print("CLife savepoint contract passed")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
