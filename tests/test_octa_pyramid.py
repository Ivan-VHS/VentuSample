from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from tools.validate_octa_pyramid import TOPOLOGY_PATH, validate


class OctaPyramidTests(unittest.TestCase):
    def test_complete_four_project_product(self) -> None:
        result = validate()
        self.assertTrue(result["valid"])
        self.assertEqual(
            (result["projects"], result["facets"], result["directed_peer_links"], result["self_loops"]),
            (4, 8, 12, 4),
        )

    def _assert_invalid(self, mutate) -> None:
        topology = json.loads(TOPOLOGY_PATH.read_text(encoding="utf-8"))
        mutate(topology)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "topology.json"
            path.write_text(json.dumps(topology), encoding="utf-8")
            with self.assertRaises(ValueError):
                validate(topology_path=path)

    def test_missing_directed_link(self) -> None:
        self._assert_invalid(lambda topology: topology["peer_links"].pop())

    def test_duplicate_facet(self) -> None:
        self._assert_invalid(
            lambda topology: topology["facets"][0].update(id=topology["facets"][1]["id"])
        )

    def test_executable_apex(self) -> None:
        self._assert_invalid(lambda topology: topology["apex"].update(executable=True))

    def test_missing_self_loop(self) -> None:
        self._assert_invalid(lambda topology: topology["self_loops"].pop())

    def test_wrong_transport(self) -> None:
        self._assert_invalid(lambda topology: topology["peer_links"][0].update(transport="none"))

    def test_planned_and_not_applicable_edges_are_explicit(self) -> None:
        topology = json.loads(TOPOLOGY_PATH.read_text(encoding="utf-8"))
        status_by_id = {item["id"]: item["status"] for item in topology["peer_links"]}
        self.assertEqual(status_by_id["reaper-to-ventusample"], "planned")
        self.assertEqual(status_by_id["ventusample-to-pytyliki"], "planned")
        self.assertEqual(status_by_id["ventusample-to-oplage"], "not_applicable")


if __name__ == "__main__":
    unittest.main()
