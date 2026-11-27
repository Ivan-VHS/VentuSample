# Pytyliki Iron Syntax audio-origin contract v1

**Canonical syntax owner:** Pytyliki  
**AST schema:** `pytyliki.iron-syntax-ast.v1`  
**Version:** `1.0.0`

The Reaper audio contract accepts Iron only as source-origin metadata after upstream syntax validation. The DSP engine does not parse Iron source, interpret its markers, or execute the AST.

The contract added to `integrations/reaper/audio_contract.rpr` validates schema, version, canonical owner, syntax-only status, record count bounds, and processing context. Only `offline_worker` is accepted. Audio-callback execution and external upload are rejected.

The twenty Iron phrases do not map to DSP graph nodes, plugin operations, realtime commands, or audio callback actions. A valid AST is provenance metadata, not a DSP instruction. The existing Reaper integration workflow executes `audio_contract.rpr`, so the assertions are covered by that workflow.

No DSP runtime or realtime path is introduced by this contract.
