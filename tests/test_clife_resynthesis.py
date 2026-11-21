from __future__ import annotations

import json
import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.validate_clife_resynthesis import (
    LABEL_RE,
    ROOT,
    create_checkpoint,
    create_recovery_branch,
    inspect_git,
    validate_policy,
)


class CLifePolicyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.policy = json.loads((ROOT / ".clife/resynthesis-policy.v1.json").read_text(encoding="utf-8"))

    def test_repository_policy_is_valid(self) -> None:
        self.assertEqual(validate_policy(self.policy, ROOT), [])

    def test_rejects_automatic_source_rewrite(self) -> None:
        policy = json.loads(json.dumps(self.policy))
        policy["safety"]["automatic_source_rewrite"] = True
        self.assertTrue(any("automatic_source_rewrite" in issue for issue in validate_policy(policy, ROOT)))

    def test_rejects_wrong_policy_schema_dialect(self) -> None:
        policy = json.loads(json.dumps(self.policy))
        policy["$schema"] = "not-a-schema"
        self.assertTrue(any("schema dialect" in issue for issue in validate_policy(policy, ROOT)))

    def test_rejects_force_push(self) -> None:
        policy = json.loads(json.dumps(self.policy))
        policy["safety"]["force_push"] = True
        self.assertTrue(any("force_push" in issue for issue in validate_policy(policy, ROOT)))

    def test_rejects_incomplete_lifecycle(self) -> None:
        policy = json.loads(json.dumps(self.policy))
        policy["adoption"]["stages"] = ["inventory", "transform"]
        self.assertTrue(any("adoption.stages" in issue for issue in validate_policy(policy, ROOT)))

    def test_checkpoint_label_is_bounded(self) -> None:
        self.assertTrue(LABEL_RE.fullmatch("pre-refactor"))
        self.assertFalse(LABEL_RE.fullmatch("../overwrite-main"))


class CLifeGitRecoveryTests(unittest.TestCase):
    def make_repository(self, path: Path) -> None:
        subprocess.run(["git", "init", "-q", str(path)], check=True)
        subprocess.run(["git", "-C", str(path), "config", "user.name", "CLife Test"], check=True)
        subprocess.run(["git", "-C", str(path), "config", "user.email", "clife-test@example.invalid"], check=True)
        (path / "README.md").write_text("checkpoint fixture\n", encoding="utf-8")
        subprocess.run(["git", "-C", str(path), "add", "README.md"], check=True)
        subprocess.run(["git", "-C", str(path), "commit", "-q", "-m", "fixture"], check=True)

    def test_audit_reports_clean_repository_without_mutation(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            self.make_repository(repo)
            before = subprocess.check_output(["git", "-C", str(repo), "rev-parse", "HEAD"], text=True).strip()
            status = inspect_git(repo)
            after = subprocess.check_output(["git", "-C", str(repo), "rev-parse", "HEAD"], text=True).strip()
            self.assertTrue(status["worktree_clean"])
            self.assertEqual(status["commit"], before)
            self.assertEqual(after, before)

    def test_checkpoint_and_recovery_branch_are_non_destructive(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            self.make_repository(repo)
            initial_branch = subprocess.check_output(["git", "-C", str(repo), "branch", "--show-current"], text=True).strip()
            tag = create_checkpoint(repo, "known-good")
            self.assertTrue(tag.startswith("clife/checkpoint-known-good-"))
            branch = create_recovery_branch(repo, tag, "clife/recovery-known-good")
            self.assertEqual(branch, "clife/recovery-known-good")
            current_branch = subprocess.check_output(["git", "-C", str(repo), "branch", "--show-current"], text=True).strip()
            self.assertEqual(current_branch, initial_branch)
            self.assertEqual(
                subprocess.check_output(["git", "-C", str(repo), "rev-parse", f"{branch}^{{commit}}"], text=True).strip(),
                subprocess.check_output(["git", "-C", str(repo), "rev-parse", f"{tag}^{{commit}}"], text=True).strip(),
            )

    def test_checkpoint_refuses_dirty_worktree(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            self.make_repository(repo)
            (repo / "uncommitted.txt").write_text("do not include implicitly\n", encoding="utf-8")
            with self.assertRaisesRegex(RuntimeError, "clean worktree"):
                create_checkpoint(repo, "must-not-snapshot")

    def test_recovery_refuses_existing_branch(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            self.make_repository(repo)
            tag = create_checkpoint(repo, "recovery")
            subprocess.run(["git", "-C", str(repo), "branch", "clife/already-exists", tag], check=True)
            with self.assertRaisesRegex(RuntimeError, "already exists"):
                create_recovery_branch(repo, tag, "clife/already-exists")


if __name__ == "__main__":
    unittest.main()
