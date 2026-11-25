# Beelzebul repository security policy

**Policy:** 1.0.0 · **Rule set:** 66 rules · **Scanner:** static, local and CI.

## Sources of truth

- [Complete rule catalogue](security/beelzebul.v1.json)
- [Catalogue schema](schemas/beelzebul.security.v1.schema.json)
- [Defensive security doctrine](docs/DEFENSIVE_SECURITY_DOCTRINE.md)
- [Scanner](tools/beelzebul.py) · [CI workflow](.github/workflows/beelzebul.yml)

The JSON catalogue is authoritative for all 66 rule definitions. This page summarizes operation and does not duplicate the rule table.

## Verdicts

| Verdict | Meaning | CI |
|---|---|---|
| blocked | Critical blocking finding | Fails |
| review | Ambiguous or advisory finding | Human review |
| clear | No blocking findings | Passes this scan |

The scanner does not execute inspected code, quarantine files or delete content. Reports include rule ID, severity, path and remediation; secret values are not copied.

## Run locally

```sh
python tools/beelzebul.py
python tools/beelzebul.py --json --output beelzebul-report.json
python tools/beelzebul.py --base-ref origin/main
python -m unittest discover -s tests -p 'test_beelzebul.py' -v
```

## Exceptions and context

Exceptions must name one rule and one path, include a reason and owner, and expire at a UTC timestamp. Wildcards and expired exceptions are not accepted; critical baseline rules cannot be exempted.

security/defense-context.v1.tsv is descriptive metadata only. A context match is not a malware signature, risk score or standalone reason to block a change.
