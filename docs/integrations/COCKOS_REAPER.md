# Cockos REAPER helper scripts

VentuSample owns the optional Lua utilities in `integrations/cockos-reaper/kora/`. They run inside Cockos REAPER's own ReaScript host; they are not part of VentuSample's generator or processing pipeline.

## Included actions

- **Track Health Dashboard** reads project name, tempo, track counts, mute/solo state and tracks without media.
- **Prefix Selected Tracks** renames selected tracks in one REAPER undo block.
- **Marker and Region Report** prints marker and region names and positions to the ReaScript console.

## Install

In Cockos REAPER, open **Actions → Show action list → ReaScript → Load**, select a Lua file from `integrations/cockos-reaper/kora/`, then run it from the Action List.

The `Cockos REAPER helpers` GitHub Actions workflow checks the scripts and publishes a ZIP artifact. The scripts do not launch shells or external processes; the dashboard and arrangement report are read-only, while track-prefix changes are grouped into a single undoable action.

These scripts are optional integrations. VentuSample itself remains a standalone desktop application and does not require Cockos REAPER.
