# Cockos REAPER integration

VentuSample owns this optional ReaScript bundle. The scripts run inside Cockos REAPER and do not load the VentuSample renderer or third-party effects.

## Included actions

- **Track Health Dashboard** reads the project name, tempo, track counts, mute/solo state and tracks without media.
- **Prefix Selected Tracks** renames selected tracks in one explicit REAPER undo block.
- **Marker and Region Report** writes marker and region names and positions to the REAPER ReaScript console.

## Install

In Cockos REAPER, open **Actions → Show action list → ReaScript → Load** and select a Lua file from this folder. The GitHub Actions workflow packages the scripts and this guide into `ventusample-cockos-reaper.zip`.

The actions use the host's documented ReaScript API. The two reporting actions are read-only; the track-prefix action is undoable. They do not install plugins, modify project media, render audio or access the network.
