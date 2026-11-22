from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / "tools" / "report_envelope.py"
SPEC = importlib.util.spec_from_file_location("report_envelope", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
report_envelope = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(report_envelope)


class ReportEnvelopeTests(unittest.TestCase):
    def test_builds_shared_envelope_and_preserves_tool_result(self):
        result = {"frames": 128, "peak_dbfs": -1.0}
        report = report_envelope.build_envelope(
            tool="ventusample",
            version="2.1.0",
            commit="ABCDEF012345678",
            result=result,
            created="2026-11-22T06:00:00Z",
            os_name="linux",
            architecture="x86_64",
        )
        self.assertEqual(report["envelope"], "ivan-vhs.report.v1")
        self.assertEqual(report["tool"]["commit"], "abcdef012345678")
        self.assertEqual(report["result"], result)
        self.assertIn("processing is offline", report["limitations"])
        self.assertEqual(report["input"], {"kind": "none", "sha256": None})

    def test_hashes_input_without_disclosing_its_path(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "private-source.bin"
            path.write_bytes(b"local source bytes")
            digest = report_envelope.sha256_file(path)
            report = report_envelope.build_envelope(
                tool="oplague",
                version="1.0",
                commit=None,
                result={"observations": []},
                input_kind="file",
                input_sha256=digest,
                created="2026-11-22T06:00:00Z",
            )
        self.assertEqual(report["input"]["sha256"], digest)
        self.assertNotIn("private-source.bin", json.dumps(report))

    def test_rejects_invalid_commit_and_hash(self):
        with self.assertRaises(report_envelope.EnvelopeError):
            report_envelope.build_envelope(
                tool="reaper", version="1", commit="not-a-hash", result={}
            )
        with self.assertRaises(report_envelope.EnvelopeError):
            report_envelope.build_envelope(
                tool="pytyliki", version="1", commit=None, result={}, input_sha256="123"
            )

    def test_rejects_non_utc_timestamp_and_empty_limitations(self):
        with self.assertRaises(report_envelope.EnvelopeError):
            report_envelope.build_envelope(
                tool="pytyliki", version="1", commit=None, result={},
                created="2026-11-22T06:00:00+02:00",
            )
        with self.assertRaises(report_envelope.EnvelopeError):
            report_envelope.build_envelope(
                tool="pytyliki", version="1", commit=None, result={}, limitations=["  "]
            )

    def test_project_limitations_are_added_once(self):
        report = report_envelope.build_envelope(
            tool="oplague", version="1", commit=None, result={},
            limitations=["analysis is read-only", "local review only"],
        )
        self.assertEqual(report["limitations"].count("analysis is read-only"), 1)
        self.assertIn("local review only", report["limitations"])


if __name__ == "__main__":
    unittest.main()
