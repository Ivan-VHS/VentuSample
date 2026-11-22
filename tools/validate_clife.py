#!/usr/bin/env python3
"""Small CTest smoke test for the shared CLife v1 savepoint tool."""
from __future__ import annotations
import tempfile
from pathlib import Path
from tools.clife import ClifeError, snapshot, verify, restore

def main() -> int:
    with tempfile.TemporaryDirectory() as folder:
        base = Path(folder)
        source = base / "source"
        source.mkdir()
        (source / "state.json").write_text('{"ok":true}', encoding="utf-8")
        archive = base / "state.clife"
        snapshot(source, archive)
        assert verify(archive)["verified"] is True
        target = base / "recovered"
        restore(archive, target)
        assert (target / "state.json").read_text(encoding="utf-8") == '{"ok":true}'
        try:
            restore(archive, target)
        except ClifeError:
            pass
        else:
            raise AssertionError("CLife must refuse to overwrite an existing recovery directory")
    print("CLife savepoint contract passed")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
