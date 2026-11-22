from __future__ import annotations

import argparse
import json
from pathlib import Path

from tools.ventusample_pack_audit import compare_ventusample_packs


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare two VentuSample pack manifests.")
    parser.add_argument("reference", type=Path, help="reference pack JSON manifest")
    parser.add_argument("candidate", type=Path, help="candidate pack JSON manifest")
    parser.add_argument("--output", type=Path, default=Path("ventusample-comparison.json"))
    args = parser.parse_args()

    reference = json.loads(args.reference.read_text(encoding="utf-8"))
    candidate = json.loads(args.candidate.read_text(encoding="utf-8"))
    report = compare_ventusample_packs(reference, candidate)
    args.output.write_text(
        json.dumps(report, ensure_ascii=False, indent=2, allow_nan=False) + "\n",
        encoding="utf-8",
    )
    print(f"Comparison report written to {args.output}")
    print(f"Euclidean distance: {report['euclidean_distance']:.8f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
