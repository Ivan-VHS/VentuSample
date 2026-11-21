#!/usr/bin/env python3
"""Run VentuSample's bounded malformed-audio/DSP C++ fuzz-smoke test."""
from __future__ import annotations

import argparse
import subprocess
from pathlib import Path

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, default=Path("build"))
    args = parser.parse_args()
    build = args.build.resolve()
    if not (build / "CTestTestfile.cmake").is_file():
        raise SystemExit("Build directory is missing; configure and compile VentuSample before fuzz-smoke.")
    result = subprocess.run(
        ["ctest", "--test-dir", str(build), "-R", "ventusample-quantum-audio", "--output-on-failure"],
        check=False,
    )
    return result.returncode

if __name__ == "__main__":
    raise SystemExit(main())
