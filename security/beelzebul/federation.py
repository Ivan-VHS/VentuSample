#!/usr/bin/env python3
"""Read-only parity audit across the four public Ivan-VHS repositories."""
from __future__ import annotations
import argparse, hashlib, sys, urllib.error, urllib.request
from pathlib import Path
OWNER="Ivan-VHS"
PEERS=("Pytyliki","Reaper","OPlague","VentuSample")
FILES=(".beelzebul/policy.v1.json",".beelzebul/exceptions.v1.json","security/beelzebul/scanner.py","security/beelzebul/test_scanner.py","security/beelzebul/federation.py","docs/BEELZEBUL.md",".github/workflows/beelzebul.yml")
def digest(data):return hashlib.sha256(data).hexdigest()
def fetch(repo,ref,path):
    url=f"https://raw.githubusercontent.com/{OWNER}/{repo}/{ref}/{path}"
    request=urllib.request.Request(url,headers={"User-Agent":"Beelzebul-Federation-Audit/1.0"})
    with urllib.request.urlopen(request,timeout=12) as response:data=response.read(4*1024*1024+1)
    if len(data)>4*1024*1024:raise ValueError("peer file exceeds 4 MiB safety limit")
    return data
def main():
    parser=argparse.ArgumentParser(description="Compare Beelzebul policy and scanner parity")
    parser.add_argument("--root",type=Path,default=Path.cwd());parser.add_argument("--ref",default="main");parser.add_argument("--strict",action="store_true")
    args=parser.parse_args();root=args.root.resolve();mismatches=[];checked=0
    for peer in PEERS:
        for path in FILES:
            local=root/path
            if not local.is_file():mismatches.append(f"{peer}: local file missing: {path}");continue
            try:
                remote=fetch(peer,args.ref,path)
                if digest(local.read_bytes())!=digest(remote):mismatches.append(f"{peer}: drift in {path}")
                checked+=1
            except (OSError,urllib.error.URLError,ValueError) as exc:mismatches.append(f"{peer}: could not verify {path}: {type(exc).__name__}")
    print(f"Beelzebul federation: {checked} peer-file checks, {len(mismatches)} mismatches, ref={args.ref}")
    for item in mismatches:print("WARN "+item)
    return 1 if mismatches and args.strict else 0
if __name__=="__main__":raise SystemExit(main())
