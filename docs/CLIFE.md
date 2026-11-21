# CLife — Code Lifecycle, Recovery and Second Life

CLife is a local, standard-library recovery utility shared by this repository family. It translates three lifecycle ideas into guarded operations:

1. **Savepoint** — create a timestamped content snapshot with a manifest, Git context and SHA-256 digest for every stored file.
2. **Stitch** — restore selected source/configuration files from a verified savepoint.
3. **Second life / Revive** — recreate missing files and restore the saved source set after a failed change or damaged working tree.

## Commands

Run from the repository root:

~~~sh
python tools/clife/clife.py savepoint --label before-refactor
python tools/clife/clife.py list
python tools/clife/clife.py verify
python tools/clife/clife.py status
python tools/clife/clife.py stitch <snapshot-id> --path src/example.c
python tools/clife/clife.py stitch <snapshot-id> --path src/example.c --apply
python tools/clife/clife.py revive <snapshot-id>
python tools/clife/clife.py revive <snapshot-id> --apply
~~~

Stitch and revive are preview-only by default. The explicit --apply flag is required to write files. Before any applied restoration, CLife creates a new pre-restore savepoint of the current eligible working tree. If the result is not wanted, verify that safety savepoint and use revive <safety-id> --apply to return to the pre-restore text-file state.

## Safety and scope

- Captures tracked and non-ignored untracked UTF-8 text source, test, documentation and configuration files.
- Skips binary data, files above the configured size limit, build/cache directories, .clife itself and common secret-like filenames/directories (.env, private keys, credentials and similar patterns).
- Uses repository-relative paths, rejects traversal and symlink targets, validates the manifest and all payload hashes before writing, and replaces each restored file atomically.
- Revive restores files present in the selected manifest. It **does not delete** files created after that savepoint.
- Snapshots remain local under .clife/snapshots/; the nested .gitignore keeps them out of commits. Copy important savepoints to a separate trusted backup location when protection against disk loss is required.

The secret exclusions are conservative filename heuristics, not a secret scanner. Review what the savepoint captures and never treat CLife as a safe place to store credentials. SHA-256 checks detect accidental corruption; the local checksum file is not a digital signature and does not authenticate a snapshot against a malicious editor. Keep Git history and external backups as independent recovery layers.

CLife is optional developer tooling. It does not run at application startup, alter the runtime, install packages, delete project files or automatically roll back a release. The Pytyliki C11 core, Reaper Python/C17 runtimes, OPlague Windows workbench and VentuSample C++20 DSP engine remain independent of the tool.
