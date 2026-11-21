#!/usr/bin/env python3
"""Beelzebul: read-only, deterministic, standard-library repository security scanner."""
from __future__ import annotations
import argparse, json, math, os, re, subprocess, sys
from pathlib import Path
from typing import Any
VERSION="1.0.0"
SEVERITY_WEIGHT={"critical":25,"high":12,"medium":5,"low":1}
SEVERITY_RANK={"critical":4,"high":3,"medium":2,"low":1}
TEXT_SUFFIXES={".py",".pyi",".c",".h",".cc",".cpp",".cxx",".hpp",".js",".jsx",".ts",".tsx",".go",".rs",".java",".cs",".rb",".php",".sh",".ps1",".yml",".yaml",".json",".toml",".ini",".cfg",".conf",".properties",".xml",".html",".css",".sql",".md",".txt",".cmake",".gradle",".lock"}
CODE_SUFFIXES={".py",".c",".h",".cc",".cpp",".cxx",".hpp",".js",".jsx",".ts",".tsx",".go",".rs",".java",".cs",".rb",".php",".sh",".ps1"}
IGNORED_DIRS={".git","node_modules","build","dist","target",".venv","venv","__pycache__",".pytest_cache",".mypy_cache",".tox","out"}
PLACEHOLDERS={"example","changeme","change-me","placeholder","your-token","your_key","your-key","dummy","test","testing","replace-me","redacted","none","null"}
SECRET_ASSIGNMENT=re.compile(r"""(?i)\b(?:api[_-]?key|access[_-]?token|client[_-]?secret|password|passwd|auth[_-]?token)\b\s*[:=]\s*['"]([^'"\s]{12,})['"]""")
AWS_KEY=re.compile(r"\bAKIA[0-9A-Z]{16}\b")
PRIVATE_KEY=re.compile(r"-----BEGIN (?:RSA |EC |OPENSSH |DSA )?PRIVATE KEY-----")
ENTROPY_ASSIGNMENT=re.compile(r"""(?i)\b(?:secret|token|api[_-]?key|password)\b\s*[:=]\s*['"]([A-Za-z0-9+/=_-]{32,})['"]""")
ZERO_WIDTH={0x200B,0x200C,0x200D,0x2060,0xFEFF}
BIDI={0x061C,0x200E,0x200F,*range(0x202A,0x202F),*range(0x2066,0x206A)}
ODD_SPACES={0x00A0,*range(0x2000,0x200B),0x202F,0x205F,0x3000}
HTTP_SCHEME="ht"+"tp"+"://"
SEMANTIC_PATTERNS=[
("M01",re.compile(r"\b(?:eval|exec)\s*\("),"high","Dynamic expression execution requires a strict trust boundary."),
("M02",re.compile(r"\bshell\s*=\s*True\b"),"high","Shell-enabled process execution can turn data into commands."),
("M03",re.compile(r"\bos\.(?:system|popen)\s*\("),"high","Direct operating-system command execution requires review."),
("M04",re.compile(r"\bpickle\.(?:load|loads)\s*\("),"high","Unsafe object deserialization can execute attacker-controlled payloads."),
("M05",re.compile(r"\byaml\.load\s*\("),"high","Use a safe YAML loader for untrusted documents."),
("M06",re.compile(r"\bhashlib\.(?:md5|sha1)\s*\("),"medium","Weak digest primitive detected; verify compatibility requirements."),
("M07",re.compile(r"\bverify\s*=\s*False\b"),"high","TLS certificate verification is disabled."),
("M08",re.compile(r"\bexecute\s*\(\s*(?:f['\"]|['\"][^'\"]*(?:%s|\.format\s*\())"),"high","SQL execute call appears to use string interpolation."),
("M09",re.compile(r"\btempfile\.mktemp\s*\("),"medium","Race-prone temporary filename helper detected."),
("M10",re.compile(r"\b(?:minidom\.parseString|xml\.etree\.ElementTree\.parse)\s*\("),"medium","Review XML parser hardening, entity handling and input trust."),
("M11",re.compile(r"\bsubprocess\.(?:run|Popen|call|check_output)\s*\(.*(?:request|input|argv|params)"),"high","External input appears on a process invocation line."),
("M12",re.compile(r"\b(?:__import__\s*\(|importlib\.import_module\s*\()"),"medium","Dynamic module loading requires an allowlisted module boundary."),
("M13",re.compile(r"\bopen\s*\(.*(?:request|input|argv|params)"),"high","External input appears to control a filesystem path."),
("M14",re.compile(r"\([^()\n]*[+*][^()\n]*\)[+*{]"),"medium","Nested repetition may cause catastrophic regular-expression backtracking."),
("M15",re.compile(r"\bexcept\s+(?:BaseException|Exception)\s*:\s*pass\b"),"low","Broad exception is silently discarded.")
]
def add(findings,rule_id,severity,path,line,message,confidence="medium",related_paths=None):
    item={"rule_id":rule_id,"tier":rule_id[0] if rule_id else "G","severity":severity,"path":path,"line":int(line or 1),"message":message,"confidence":confidence}
    if related_paths: item["related_paths"]=sorted(set(related_paths))
    findings.append(item)
def is_placeholder(value):
    low=value.strip().lower()
    return low in PLACEHOLDERS or any(p in low for p in ("your_","your-","example","placeholder","redacted")) or low.startswith(("$"+"{","{{","<","****"))
def shannon_entropy(value):
    if not value:return 0.0
    counts={ch:value.count(ch) for ch in set(value)}
    return -sum((n/len(value))*math.log2(n/len(value)) for n in counts.values())
def semantic_path_allowed(path):
    value=path.replace("\\","/").lower()
    if value.startswith("security/beelzebul/") or "/tests/" in "/"+value or value.startswith("tests/"):return False
    if value.endswith("_test.py") or value.endswith(".test.js") or value.endswith(".spec.js"):return False
    return Path(value).suffix in CODE_SUFFIXES
def inspect_file(path,data,max_bytes):
    findings=[]; suffix=Path(path).suffix.lower()
    if len(data)>max_bytes:
        add(findings,"A10","medium",path,1,"Text artifact exceeds the configured scan-size bound.","high");return findings
    filename=Path(path).name.lower()
    if filename==".env" or filename in {"id_rsa","id_ed25519","id_ecdsa"} or suffix in {".p12",".pfx"} or (suffix==".pem" and not filename.endswith(".pub")):
        if not any(x in path.lower() for x in (".example","example.","tests/fixtures/")):
            add(findings,"A15","critical",path,1,"Credential or private-key filename is present in the repository.","high")
    is_text=suffix in TEXT_SUFFIXES or not suffix
    if b"\x00" in data and suffix in CODE_SUFFIXES|{".json",".yml",".yaml",".toml",".txt"}:
        add(findings,"A07","high",path,1,"NUL byte detected in a text/source file.","high")
    try:text=data.decode("utf-8")
    except UnicodeDecodeError:
        if is_text:add(findings,"A11","medium",path,1,"File expected to be text is not valid UTF-8.","high")
        return findings
    if not is_text:return findings
    if b"\r\n" in data and data.replace(b"\r\n",b"").count(b"\n")>0:
        add(findings,"A09","low",path,1,"CRLF and LF line endings are mixed in the same file.","high")
    semantic=semantic_path_allowed(path)
    for number,line in enumerate(text.splitlines() or [text],1):
        if line.startswith(("<<<<<<< ","=======",">>>>>>> ")):add(findings,"A08","high",path,number,"Unresolved Git merge marker detected.","high")
        if any(ord(ch) in BIDI for ch in line):add(findings,"A05","high",path,number,"Unicode bidirectional control detected.","high")
        if any(ord(ch) in ZERO_WIDTH for ch in line):add(findings,"A06","medium",path,number,"Zero-width or byte-order character detected.","high")
        if any(ord(ch) in ODD_SPACES for ch in line):add(findings,"A12","medium",path,number,"Ambiguous Unicode whitespace detected.","high")
        if suffix in CODE_SUFFIXES|{".json",".yml",".yaml",".toml",".sh",".ps1"} and HTTP_SCHEME in line.lower() and not re.search(r"localhost|127\.0\.0\.1|0\.0\.0\.0",line,re.I):
            add(findings,"A13","medium",path,number,"Plain HTTP endpoint found in executable/configuration content.","medium")
        match=SECRET_ASSIGNMENT.search(line)
        if match and not is_placeholder(match.group(1)):add(findings,"A01","high",path,number,"Credential-like literal assignment detected; value redacted.","high")
        if AWS_KEY.search(line):add(findings,"A02","critical",path,number,"Cloud access-key identifier detected; value redacted.","high")
        if PRIVATE_KEY.search(line):add(findings,"A03","critical",path,number,"Private-key material detected; content redacted.","high")
        match=ENTROPY_ASSIGNMENT.search(line)
        if match and not is_placeholder(match.group(1)) and shannon_entropy(match.group(1))>=4.0:
            add(findings,"A04","high",path,number,"High-entropy secret-like literal detected; value redacted.","medium")
        if re.search(r"\b(?:print|logger\.\w+|logging\.\w+)\s*\(",line,re.I) and re.search(r"\b(?:token|secret|password|api[_-]?key)\b",line,re.I):
            add(findings,"A14","high",path,number,"Credential-like value may be sent to console or logs.","medium")
        if semantic:
            for rid,pattern,severity,message in SEMANTIC_PATTERNS:
                if pattern.search(line):
                    if rid=="M05" and re.search(r"SafeLoader|safe_load",line):continue
                    add(findings,rid,severity,path,number,message,"medium")
    if suffix in CODE_SUFFIXES and any(byte<32 and byte not in (9,10,13) for byte in data):
        add(findings,"A07","high",path,1,"Unexpected control byte detected in source file.","high")
    return findings
def walk_files(root):
    found=[]
    for current,dirs,files in os.walk(root,followlinks=False):
        dirs[:]=sorted(d for d in dirs if d not in IGNORED_DIRS and not (Path(current)/d).is_symlink())
        for filename in sorted(files):
            full=Path(current)/filename
            if full.is_symlink() or not full.is_file():continue
            if full.relative_to(root).as_posix().startswith(".git/"):continue
            found.append(full)
    return sorted(found,key=lambda p:p.relative_to(root).as_posix())
def validate_policy(root,findings):
    path=root/".beelzebul"/"policy.v1.json"; errors=[]
    try:policy=json.loads(path.read_text(encoding="utf-8"))
    except (OSError,json.JSONDecodeError):
        add(findings,"G01","critical",".beelzebul/policy.v1.json",1,"Canonical policy is missing or invalid JSON.","high");return {}
    tiers=policy.get("tiers")
    if not isinstance(tiers,list) or len(tiers)!=4:errors.append("exactly four tiers are required");tiers=[]
    seen=set()
    for tier in tiers:
        if not isinstance(tier,dict) or not isinstance(tier.get("rules"),list):errors.append("tier rule list is invalid");continue
        if len(tier["rules"])!=15 or tier.get("rule_count")!=15:errors.append("each specialized tier must contain exactly 15 rules")
        for rule in tier["rules"]:
            rid=rule.get("rule_id") if isinstance(rule,dict) else None
            if not isinstance(rid,str) or rid in seen:errors.append("rule IDs must be unique non-empty strings")
            else:seen.add(rid)
    global_rules=policy.get("global_rules")
    if not isinstance(global_rules,list) or len(global_rules)!=6:errors.append("exactly six global rules are required")
    else:
        for rule in global_rules:
            rid=rule.get("rule_id") if isinstance(rule,dict) else None
            if not isinstance(rid,str) or rid in seen:errors.append("global rule IDs must be unique non-empty strings")
            else:seen.add(rid)
    if len(seen)!=66:errors.append("the complete codex must contain 66 unique rule IDs")
    if policy.get("codex")!="BEELZEBUL-66" or policy.get("policy_version")!="1.0.0":errors.append("unsupported codex or policy version")
    if policy.get("blocking",{}).get("changed_files")!=["critical","high"]:
        add(findings,"G02","high",".beelzebul/policy.v1.json",1,"Changed-file blocking threshold must include critical and high.","high")
    if errors:add(findings,"G01","critical",".beelzebul/policy.v1.json",1,"Policy validation failed: "+"; ".join(sorted(set(errors))),"high")
    if any(policy.get("scoring",{}).get(k)!=v for k,v in SEVERITY_WEIGHT.items()):
        add(findings,"G06","high",".beelzebul/policy.v1.json",1,"Severity scoring weights diverge from the canonical scale.","high")
    return policy
def validate_exceptions(root,findings):
    path=root/".beelzebul"/"exceptions.v1.json"
    try:value=json.loads(path.read_text(encoding="utf-8"))
    except (OSError,json.JSONDecodeError):
        add(findings,"G04","high",".beelzebul/exceptions.v1.json",1,"Exception register is missing or invalid JSON.","high");return []
    entries=value.get("exceptions",[]) if isinstance(value,dict) else None
    if not isinstance(entries,list):
        add(findings,"G04","high",".beelzebul/exceptions.v1.json",1,"Exception register must contain an exceptions array.","high");return []
    valid=[]
    from datetime import date
    today=date.today()
    for index,item in enumerate(entries):
        if not isinstance(item,dict):
            add(findings,"G04","high",".beelzebul/exceptions.v1.json",index+1,"Exception entry must be an object.","high");continue
        reason,rid,target,expires=(item.get("reason"),item.get("rule_id"),item.get("path"),item.get("expires_at"))
        if not all(isinstance(x,str) and x.strip() for x in (reason,rid,target,expires)):
            add(findings,"G04","high",".beelzebul/exceptions.v1.json",index+1,"Exception requires rule_id, exact path, reason and expires_at.","high");continue
        if any(ch in target for ch in "*?[]") or target.startswith("/") or ".." in Path(target).parts:
            add(findings,"G05","high",".beelzebul/exceptions.v1.json",index+1,"Exception path must be one exact repository-relative path.","high");continue
        try:expiry=date.fromisoformat(expires)
        except ValueError:
            add(findings,"G04","high",".beelzebul/exceptions.v1.json",index+1,"Exception expiry must use YYYY-MM-DD.","high");continue
        if expiry<today:
            add(findings,"G04","high",".beelzebul/exceptions.v1.json",index+1,"Expired exception detected.","high");continue
        valid.append(item)
    return valid
def inspect_infrastructure(file_texts,findings):
    paths=set(file_texts)
    for path,text in sorted(file_texts.items()):
        low=text.lower()
        if path.startswith(".github/workflows/"):
            if re.search(r"permissions\s*:\s*[\s\S]{0,160}\bwrite-all\b",text,re.I):add(findings,"N01","critical",path,1,"Workflow declares unbounded write-all permissions.","high")
            if "pull_request_target" in low and re.search(r"head\.(?:sha|ref)|pull_request\.head",low):add(findings,"N02","critical",path,1,"Privileged pull_request_target workflow checks out untrusted head content.","high")
            for number,line in enumerate(text.splitlines(),1):
                if "run:" in line and re.search(r"\$\{\{\s*github\.event\.",line):add(findings,"N03","high",path,number,"Untrusted event data is interpolated directly into a shell run command.","high")
                action_ref=re.search(r"^\s*uses:\s*[^#\s]+@([^\s#]+)",line)
                if action_ref and not re.fullmatch(r"[0-9a-fA-F]{40}",action_ref.group(1)):
                    add(findings,"N04","medium",path,number,"Action reference is tag-based rather than pinned to a full commit SHA.","high")
                if "persist-credentials: true" in line.lower():add(findings,"N07","medium",path,number,"Checkout persists credentials beyond the checkout step.","high")
            if re.search(r"\b(?:curl|wget)\b[\s\S]{0,180}\|\s*(?:ba)?sh\b",text,re.I):add(findings,"N05","critical",path,1,"Remote content is piped directly to a shell.","high")
            if "timeout-minutes:" not in text:add(findings,"N06","low",path,1,"Workflow has no explicit job timeout.","high")
            if "pull_request:" in low and "secrets." in low:add(findings,"N08","high",path,1,"Secret references exist in a workflow triggered by pull_request.","medium")
            if re.search(r"contents\s*:\s*write|permissions\s*:\s*write-all",text,re.I):add(findings,"N14","high",path,1,"Workflow requests repository content write permission.","high")
            if "pull_request_target" in low and ("secrets." in low or "write-all" in low):add(findings,"N15","critical",path,1,"Privileged trigger is combined with secrets or broad write permissions.","high")
        if Path(path).name.lower()=="dockerfile" or Path(path).name.lower().startswith("dockerfile."):
            if re.search(r"^\s*from\s+\S+:latest\b",text,re.I|re.M):add(findings,"N09","medium",path,1,"Container base image uses a floating latest tag.","high")
        if Path(path).name.lower().startswith("requirements") and Path(path).suffix==".txt":
            for number,line in enumerate(text.splitlines(),1):
                entry=line.strip()
                if not entry or entry.startswith(("#","-","http")):continue
                if not re.search(r"(?:==|===|@\s*https?://)",entry):add(findings,"N11","medium",path,number,"Python requirement is not pinned to an exact version or immutable source.","medium")
        if Path(path).name=="package.json":
            try:package=json.loads(text)
            except json.JSONDecodeError:package={}
            for section in ("dependencies","devDependencies","optionalDependencies"):
                for name,version in package.get(section,{}).items():
                    if isinstance(version,str) and version.strip().lower() in {"*","latest","x"}:add(findings,"N12","medium",path,1,"JavaScript dependency uses a floating wildcard/latest version.","high")
    if any(p.endswith("package.json") for p in paths) and not any(p.endswith(("package-lock.json","pnpm-lock.yaml","yarn.lock")) for p in paths):
        add(findings,"N10","medium","package.json",1,"JavaScript package manifest has no matching lockfile.","high")
    if any(Path(p).name in {"pyproject.toml","Pipfile"} for p in paths) and not any(Path(p).name in {"poetry.lock","uv.lock","Pipfile.lock"} for p in paths):
        add(findings,"N10","medium","pyproject.toml",1,"Python project manifest has no recognized lockfile.","medium")
    for path,text in sorted(file_texts.items()):
        if re.search(r"\b(?:curl|wget)\b",text,re.I) and not re.search(r"sha256sum|sha256\s*[:=]|checksum",text,re.I) and path.endswith((".sh",".ps1",".yml",".yaml")):
            add(findings,"N13","high",path,1,"Remote download exists without an adjacent checksum-verification marker.","low")
def changed_paths(root,base_ref):
    result=subprocess.run(["git","diff","--name-only",base_ref+"...HEAD"],cwd=root,capture_output=True,text=True,timeout=30,check=True)
    return {x.strip().replace("\\","/") for x in result.stdout.splitlines() if x.strip()}
def scan_history(root,base_ref,findings):
    try:result=subprocess.run(["git","log","--format=","--no-ext-diff","--unified=0",base_ref+"..HEAD","--"],cwd=root,capture_output=True,text=True,timeout=45,check=True)
    except (OSError,subprocess.SubprocessError):
        add(findings,"G03","medium","__history__",1,"Git history evidence could not be collected; current-tree scanning still completed.","high");return
    current_path=""
    for number,line in enumerate(result.stdout.splitlines(),1):
        if line.startswith("+++ b/"):current_path=line[6:];continue
        if not line.startswith("+") or line.startswith("+++"):continue
        added=line[1:]
        if SECRET_ASSIGNMENT.search(added) and not any(p in added.lower() for p in PLACEHOLDERS):add(findings,"A01","high",current_path or "__history__",number,"Credential-like value was added in commit history; value redacted.","high")
        if AWS_KEY.search(added):add(findings,"A02","critical",current_path or "__history__",number,"Cloud access-key identifier was added in commit history; value redacted.","high")
        if PRIVATE_KEY.search(added):add(findings,"A03","critical",current_path or "__history__",number,"Private-key material was added in commit history; content redacted.","high")
def correlate(findings,file_texts,focus_paths=None):
    # Never correlate the policy catalogue, scanner implementation, or its tests as target application code.
    # When evaluating a PR, restrict correlations to files changed in that PR.
    file_texts={path:text for path,text in file_texts.items()
                if not path.startswith('.beelzebul/')
                and not path.startswith('security/beelzebul/')
                and not path.startswith('tests/')
                and (focus_paths is None or path in focus_paths)}
    by_path={}
    for finding in findings:by_path.setdefault(finding["path"],[]).append(finding)
    for path,text in sorted(file_texts.items()):
        low=text.lower()
        has_input=bool(re.search(r"\b(?:request\.|request\[|input\s*\(|sys\.argv|process\.env)",low))
        has_exec=bool(re.search(r"\b(?:eval|exec)\s*\(|\bos\.(?:system|popen)\s*\(|\bshell\s*=\s*true\b",low))
        if has_input and has_exec:
            add(findings,"Q01","high",path,1,"External-input and dynamic-execution indicators coexist in one file.","medium")
            add(findings,"Q03","critical",path,1,"External input and command/expression execution share a trust boundary.","medium")
        if re.search(r"\b(?:requests\.|urllib\.request|httpx\.|fetch\s*\()",low) and "verify=false" in low:add(findings,"Q02","high",path,1,"Network client and disabled TLS verification are correlated.","medium")
        if re.search(r"\b(?:requests\.|urllib\.request|httpx\.|fetch\s*\()",low) and re.search(r"\bpickle\.(?:load|loads)\s*\(",low):add(findings,"Q04","critical",path,1,"Network input and unsafe deserialization are correlated.","medium")
        if re.search(r"\b(?:password|token|secret|api[_-]?key)\b",low) and re.search(r"\b(?:print|logger\.|logging\.)",low):add(findings,"Q05","high",path,1,"Secret-bearing identifiers and output/logging calls coexist.","medium")
        if re.search(r"\b(?:password|auth|credential)\b",low) and re.search(r"\b(?:md5|sha1)\b",low):add(findings,"Q06","high",path,1,"Weak digest and authentication-related identifiers coexist.","medium")
    workflow_text="\n".join(text for path,text in file_texts.items() if path.startswith(".github/workflows/"))
    if workflow_text:
        privileged=bool(re.search(r"write-all|contents\s*:\s*write",workflow_text,re.I))
        untrusted=bool(re.search(r"pull_request_target|github\.event\.(?:issue|pull_request|comment)",workflow_text,re.I))
        if privileged and untrusted:add(findings,"Q07","high",".github/workflows",1,"Privileged workflow permissions correlate with untrusted event data.","medium")
        if "pull_request_target" in workflow_text and (untrusted or privileged):add(findings,"Q08","critical",".github/workflows",1,"pull_request_target is combined with an untrusted or privileged execution path.","medium")
        if re.search(r"(?:skip|continue-on-error).{0,100}(?:security|test)|(?:security|test).{0,100}(?:skip|continue-on-error)",workflow_text,re.I):add(findings,"Q11","medium",".github/workflows",1,"Security/test execution appears to be bypassed or made non-blocking.","low")
    for path,text in sorted(file_texts.items()):
        if ".github/rulesets/" in path and re.search(r"bypass.{0,100}(?:always|unrestricted)|(?:always|unrestricted).{0,100}bypass",text,re.I):add(findings,"Q12","high",path,1,"Repository governance contains an unrestricted bypass marker.","medium")
    for path,items in sorted(by_path.items()):
        if sum(1 for x in items if x["severity"] in {"high","critical"})>=3:add(findings,"Q10","high",path,1,"Multiple independent high-risk signals are concentrated in one file.","medium")
    high_paths=sorted({f["path"] for f in findings if f["severity"] in {"high","critical"} and f["path"]!="__history__"})
    if len(high_paths)>=4:add(findings,"Q14","high","__repository__",1,"High-risk indicators are distributed across several repository files.","low",high_paths)
    if any(f["rule_id"]=="N10" for f in findings):add(findings,"Q09","medium","__repository__",1,"Dependency manifests and repository lockfile inventory are inconsistent.","medium",[f["path"] for f in findings if f["rule_id"]=="N10"])
def score_findings(findings,policy):
    weights=policy.get("scoring",SEVERITY_WEIGHT)
    score=min(int(weights.get("cap",100)),sum(int(weights.get(x["severity"],0)) for x in findings))
    level="critical" if score>=75 else "high" if score>=40 else "elevated" if score>=20 else "guarded" if score>=1 else "clear"
    counts={s:sum(1 for f in findings if f["severity"]==s) for s in ("critical","high","medium","low")}
    return {"risk_score":score,"risk_level":level,"by_severity":counts,"total":len(findings)}
def apply_exceptions(findings,exceptions):
    return [f for f in findings if not any(e.get("rule_id")==f["rule_id"] and e.get("path")==f["path"] for e in exceptions)]
def build_report(root,base_ref=None,only_changed=False):
    findings=[];policy=validate_policy(root,findings);exceptions=validate_exceptions(root,findings)
    file_texts={};file_count=0;max_bytes=int(policy.get("scan",{}).get("max_text_file_bytes",2097152)) if policy else 2097152
    for full in walk_files(root):
        rel=full.relative_to(root).as_posix()
        try:data=full.read_bytes()
        except OSError:add(findings,"G03","medium",rel,1,"File could not be read during audit.","high");continue
        file_count+=1;findings.extend(inspect_file(rel,data,max_bytes))
        if Path(rel).suffix.lower() in CODE_SUFFIXES|{".yml",".yaml",".json",".toml"}:
            try:file_texts[rel]=data.decode("utf-8")
            except UnicodeDecodeError:pass
    inspect_infrastructure(file_texts,findings);correlate(findings,file_texts)
    if base_ref:scan_history(root,base_ref,findings)
    findings=apply_exceptions(findings,exceptions);changed=None
    if only_changed:
        if not base_ref:raise ValueError("--changed-only requires --base-ref")
        changed=changed_paths(root,base_ref)
        findings=[f for f in findings if f["path"] in changed or (f["path"]=="__repository__" and set(f.get("related_paths",[])).intersection(changed))]
    unique={}
    for f in findings:unique[(f["rule_id"],f["path"],f["line"],f["message"])]=f
    findings=sorted(unique.values(),key=lambda x:(x["path"],x["line"],x["rule_id"]))
    return {"tool":"Beelzebul","scanner_version":VERSION,"policy_version":policy.get("policy_version","unknown") if policy else "invalid","repository":root.name,"files_scanned":file_count,"scope":"changed_files" if only_changed else "full_tree","changed_paths":sorted(changed) if changed is not None else [],"summary":score_findings(findings,policy or {}),"findings":findings,"evidence_policy":{"matched_content_included":False,"network_calls":0,"scanned_code_executed":False}}
def main():
    parser=argparse.ArgumentParser(description="Read-only repository security scanner")
    parser.add_argument("--root",type=Path,default=Path.cwd());parser.add_argument("--format",choices=("json","text"),default="json")
    parser.add_argument("--output",type=Path);parser.add_argument("--base-ref");parser.add_argument("--changed-only",action="store_true");parser.add_argument("--fail-on",choices=("critical","high","medium","low"))
    args=parser.parse_args()
    try:report=build_report(args.root.resolve(),args.base_ref,args.changed_only)
    except (OSError,ValueError,subprocess.SubprocessError) as exc:print("Beelzebul audit failed safely: "+str(exc),file=sys.stderr);return 2
    if args.format=="json":rendered=json.dumps(report,ensure_ascii=False,indent=2,sort_keys=True)+"\n"
    else:
        s=report["summary"];rendered=f"Beelzebul {VERSION}: {s['risk_level']} ({s['risk_score']}/100), {s['total']} findings\n"
        for f in report["findings"]:rendered+=f"{f['severity'].upper():8} {f['rule_id']} {f['path']}:{f['line']} {f['message']}\n"
    if args.output:
        args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(rendered,encoding="utf-8")
    sys.stdout.write(rendered)
    if args.fail_on and any(SEVERITY_RANK[f["severity"]]>=SEVERITY_RANK[args.fail_on] for f in report["findings"]):return 1
    return 0
if __name__=="__main__":raise SystemExit(main())
