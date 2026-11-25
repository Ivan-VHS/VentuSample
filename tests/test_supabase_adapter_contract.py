from copy import deepcopy
import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "project_contract_validator", ROOT / "tools" / "validate_project_contract.py"
)
validator = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(validator)

class SupabaseAdapterContractTests(unittest.TestCase):
    project_id = "ventusample"

    def run_with_adapter(self, mutate=None):
        original_load = validator.load_json

        def load(path):
            value = original_load(path)
            if Path(path).name == "supabase-adapter.v1.json":
                value = deepcopy(value)
                if mutate is not None:
                    mutate(value)
            return value

        with patch.object(validator, "load_json", side_effect=load):
            validator.validate_supabase_adapter(self.project_id)

    def test_current_adapter_contract_is_valid(self):
        self.run_with_adapter()

    def test_network_cannot_be_enabled_by_default(self):
        def mutate(adapter):
            adapter["network_policy"]["enabled_by_default"] = True
        with self.assertRaises(ValueError):
            self.run_with_adapter(mutate)

    def test_service_role_key_cannot_be_used_in_a_client(self):
        def mutate(adapter):
            adapter["supabase"]["service_role_key_in_client"] = True
        with self.assertRaises(ValueError):
            self.run_with_adapter(mutate)

    def test_unknown_adapter_fields_are_rejected(self):
        def mutate(adapter):
            adapter["unexpected"] = "must-not-be-accepted"
        with self.assertRaises(ValueError):
            self.run_with_adapter(mutate)

    def test_adapter_must_remain_contract_only_until_verified(self):
        def mutate(adapter):
            adapter["status"] = "hosted-verified"
        with self.assertRaises(ValueError):
            self.run_with_adapter(mutate)

if __name__ == "__main__":
    unittest.main()
