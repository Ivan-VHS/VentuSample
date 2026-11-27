# Supabase Adapter Contract v1

Project: ventusample

Status: contract-only

Default: offline-only

This file declares the data this repository may exchange with the optional Ivan-VHS Supabase data plane. It is an allow-list, not a claim that a network client or hosted integration already exists.

## Allowed data domains

- sample_pack_manifests
- preset_metadata
- render_summaries
- user_selected_artifact_hashes

## Explicit exclusions

- raw_audio_payloads_by_default
- unselected_wav_assets
- private_recordings
- source_project_paths
- credentials
- service_role_keys

## Runtime rules

- Local project data remains authoritative.
- Cloud access is disabled by default and requires an explicit user action.
- No background synchronization, telemetry or silent uploads.
- A client may receive only SUPABASE_URL and a publishable key at runtime.
- A service-role key, database password or signing secret must never enter this repository or a client binary.
- The shared event and artifact envelope IDs are versioned; domain-specific payload schemas remain owned by this repository.
- Do not change the adapter status to local-ci-verified or hosted-verified without corresponding test evidence.
