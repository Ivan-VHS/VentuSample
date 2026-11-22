from __future__ import annotations

import json
import shutil
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE = Path(tempfile.gettempdir()) / "ventusample-tests" / "pack" / "ventusample_manifest.json"
DESTINATION = ROOT / "build" / "reaper-fixtures" / "ventusample.pack.v2.json"


def main() -> None:
    if SOURCE.is_symlink() or not SOURCE.is_file():
        raise SystemExit(f"VentuSample generator fixture is missing or unsafe: {SOURCE}")
    if SOURCE.stat().st_size > 4 * 1024 * 1024:
        raise SystemExit("VentuSample manifest fixture exceeds the 4 MiB validation limit")
    payload = json.loads(SOURCE.read_text(encoding="utf-8"))
    if not isinstance(payload, dict) or payload.get("schema") != "ventusample.pack.v2":
        raise SystemExit("VentuSample generator did not produce the expected v2 pack schema")
    DESTINATION.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(SOURCE, DESTINATION)
    print(f"Prepared generated VentuSample v2 manifest for Reaper: {DESTINATION}")


if __name__ == "__main__":
    main()
