# VentuSample release distribution

## Supported release path

VentuSample remains an offline Windows/Linux desktop sample workstation. It is not a hosted service and has no production deployment target.

A Windows x64 release candidate is prepared only by manually dispatching **VentuSample Windows Release** from a selected Git tag:

1. Create and review a version tag such as `v1.2.3`.
2. Run the workflow against that tag and enter the exact same tag in `version`.
3. Set `confirm_release` to true to authorize creation of a release candidate.
4. The workflow configures MSVC 2022 and Qt 6.8.3, builds the Release executable, runs CTest, deploys the Qt runtime with `windeployqt`, and packages the application, license and versioned pack schemas.
5. The package receives a deterministic `release-manifest.json` containing the source commit and a SHA-256 inventory for every staged file. The ZIP receives a separate SHA-256 sidecar.
6. The workflow verifies the checksum and creates a **draft** GitHub Release. A maintainer must inspect and manually publish the draft.

The release job uses the GitHub Environment `release-review`. Repository administrators must configure required reviewers for this environment in GitHub Settings before treating it as an enforced approval gate. The workflow does not silently claim that a reviewer rule exists. Even after the workflow completes, the release remains a draft until a maintainer publishes it.

## Manifest contract

The versioned manifest schema is [ventusample.release.v1](../schemas/ventusample.release.v1.schema.json). The generator is [create_release_manifest.py](../tools/create_release_manifest.py).

The manifest records package-relative file paths, byte sizes and SHA-256 hashes; it excludes itself to avoid recursive hashing. File records are sorted so identical package inputs produce identical manifest bytes. Symlinks, invalid versions, non-canonical paths and files outside the package root are rejected. Manifest writing uses a temporary file followed by an atomic replacement.

SHA-256 detects byte changes relative to the manifest and checksum sidecar; it does **not** authenticate the publisher. This increment does not claim Authenticode signing, a hardware-backed signing identity, SBOM certification or a deployed update service.

## Rollback and boundaries

A draft release is not a production deployment. To withdraw a candidate, delete or close the draft release and retain the previous published version. Users can reinstall a previous verified archive. There is no automatic in-app updater or server-side rollout.

Linux desktop packaging, package-manager repositories, code signing, notarization, cloud distribution and automatic updates remain outside this release increment.
