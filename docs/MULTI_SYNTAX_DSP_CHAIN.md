# Multi-syntax DSP chain target

**Project author/developer:** Ivan-VHS  
**Contract:** `ventusample.quantum-audio-chain.v1`

VentuSample exposes a syntax-neutral, typed DSP chain interface. A frontend adapter can translate a validated Reaper, Python-subset, pipeline, Lisp, Forth, Brainfuck or Befunge-93 program into a sequence of existing Quantum Audio module IDs and normalized controls. The audio engine does not parse or execute those source languages itself.

## Typed C++ interface

`QuantumAudioChainStep` pairs a stable module ID with the four existing `QuantumAudioParameters` controls. `processQuantumAudioChain` runs the sequence in order and returns a report with completed steps, failed module, source/final statistics and processed IDs.

The chain is transactional: the implementation renders to a private buffer and replaces the caller's audio only after every module succeeds. A missing module, invalid control, malformed audio buffer or oversized chain leaves the original signal untouched. An empty chain is a successful no-op.

## Shared interchange format

The machine-readable `schemas/quantum-audio-chain.v1.schema.json` contract stores a list of plugin IDs and parameter values. An optional `origin` section records the source syntax ID and source SHA-256; it may also carry the hash of the non-executing validation report used by the adapter.

Example:

```json
{
  "schema": "ventusample.quantum-audio-chain.v1",
  "origin": {
    "syntax_id": "pipeline",
    "source_sha256": "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
  },
  "steps": [
    {
      "plugin_id": "strange-quark",
      "parameters": {"amount": 0.62, "tone": 0.78, "rate": 0.24, "mix": 0.71}
    },
    {
      "plugin_id": "coherence-meter",
      "parameters": {"amount": 0.5, "tone": 0.5, "rate": 0.35, "mix": 0.75}
    }
  ]
}
```

## Current integration boundary

The typed chain API and the existing Quantum Audio Lab use the same processing function. Reaper remains the owner of syntax validation and execution; DuckOS is the desktop bridge; Pytyliki provides the cross-repository syntax conformance corpus. The optional Reaper C ABI expression bridge remains a separate build-time feature. No implicit code execution, dynamic library discovery or shell command is introduced by the chain API.

The JSON schema is an interchange contract for frontend adapters. This milestone does not yet ship a JSON file importer or claim that all seven language frontends compile directly into DSP chains.
