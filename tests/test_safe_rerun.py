"""Regression tests for the bounded, manual CI rerun gate."""
from __future__ import annotations
import copy, unittest
from validate_safe_rerun import validate_policy, validate_run
POLICY={"schema":"anatidae.safe-ci-rerun.v1","repository":"Ivan-VHS/OPlague","max_attempts":2,
"allowed_events":["push","pull_request","schedule","workflow_dispatch"],"allowed_conclusions":["failure","timed_out"],
"allowed_workflow_names":["OPlague Desktop Build","Automation Roadmap Contract"],"same_repository_only":True}
RUN={"id":123456,"name":"OPlague Desktop Build","event":"pull_request","status":"completed","conclusion":"failure","run_attempt":1,
"head_sha":"a"*40,"head_branch":"feature/example","html_url":"https://github.com/Ivan-VHS/OPlague/actions/runs/123456",
"repository":{"full_name":"Ivan-VHS/OPlague"},"head_repository":{"full_name":"Ivan-VHS/OPlague"}}
class SafeRerunPolicyTests(unittest.TestCase):
 def test_repository_policy_is_valid(self): self.assertEqual(validate_policy(POLICY),[])
 def test_valid_same_repository_failed_run_is_eligible(self): self.assertEqual(validate_run(RUN,POLICY),[])
 def test_fork_pull_request_is_rejected(self):
  x=copy.deepcopy(RUN); x["head_repository"]["full_name"]="contributor/OPlague"
  self.assertTrue(any("fork" in e for e in validate_run(x,POLICY)))
 def test_wrong_repository_is_rejected(self):
  x=copy.deepcopy(RUN); x["repository"]["full_name"]="other/project"; self.assertTrue(validate_run(x,POLICY))
 def test_unlisted_workflow_is_rejected(self):
  x=copy.deepcopy(RUN); x["name"]="Release publication"; self.assertTrue(any("allowlist" in e for e in validate_run(x,POLICY)))
 def test_non_failure_conclusions_are_rejected(self):
  for outcome in ("success","cancelled","action_required"):
   with self.subTest(outcome=outcome):
    x=copy.deepcopy(RUN); x["conclusion"]=outcome; self.assertTrue(validate_run(x,POLICY))
 def test_in_progress_run_is_rejected(self):
  x=copy.deepcopy(RUN); x["status"]="in_progress"; self.assertTrue(validate_run(x,POLICY))
 def test_retry_ceiling_is_enforced(self):
  x=copy.deepcopy(RUN); x["run_attempt"]=2; self.assertTrue(any("retry ceiling" in e for e in validate_run(x,POLICY)))
 def test_invalid_sha_and_id_are_rejected(self):
  x=copy.deepcopy(RUN); x["head_sha"]="bad"; x["id"]=-1; self.assertTrue(validate_run(x,POLICY))
 def test_unsupported_event_is_rejected(self):
  x=copy.deepcopy(RUN); x["event"]="workflow_run"; self.assertTrue(validate_run(x,POLICY))
 def test_policy_cannot_expand_retry_ceiling(self):
  x=copy.deepcopy(POLICY); x["max_attempts"]=5; self.assertTrue(validate_policy(x))
 def test_policy_cannot_allow_forks(self):
  x=copy.deepcopy(POLICY); x["same_repository_only"]=False; self.assertTrue(validate_policy(x))
if __name__=="__main__": unittest.main()
