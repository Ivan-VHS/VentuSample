# README and visual documentation standard

This file is the publishing contract for the VentuSample README family, documentation index, technical Markdown, diagrams and visual assets. It applies the repository's 66-point README brief as an implementation and review standard. It does not override source code, tests, schemas, CI results or project-specific UI contracts.

## 1. Purpose and first-screen contract

The root README must answer, in its opening sentence, what the project is and who it serves. Keep the hero compact: title, one-line purpose, no more than 3–5 functional badges, language links, short anchor navigation and the main visual.

Put requirements and a copy-ready build/test path near the top. A quick example should produce a meaningful local result; do not invent command names or imply an example was executed unless it is covered by tests or has been manually verified. Avoid a large table of contents when the README is short.

Use one H1 per document, then a consistent H2/H3 hierarchy. Prefer short paragraphs, descriptive link text, language-tagged fences and tables for build options, environment variables, limits and compatibility. Use GitHub Alerts only for actionable warnings or notes. Fold long logs/configuration with details blocks; keep essential instructions visible.

## 2. Product truth and maturity

Use stable, project-specific maturity wording. Distinguish implemented, partial, optional, reference-only and planned capabilities. A roadmap checkbox records repository state; it is not proof that every proposed feature exists.

Explain what the product does, why its separation matters and where it stops. For VentuSample, preserve these boundaries:

- It is a local procedural sample generator and offline WAV processor, not a DAW, real-time audio host, browser sandbox or hosted service.
- The core product is native C++20, with a C11 DSP support library and optional Qt 6 desktop application.
- The Qt desktop target is optional; the portable core can be built with VENTU_BUILD_DESKTOP=OFF.
- Plugin discovery is metadata-only; do not claim arbitrary DLL/SO loading.
- Reaper integration is optional and requires an explicit local source tree.
- Atomic-Quantum is classical simulation, not quantum hardware.
- Generated or imported audio does not automatically acquire project copyright or ownership claims.

Update the README when source, build options, tests, schemas, UI, supported platforms or safety boundaries change. Remove stale claims instead of decorating them.

## 3. Quick start and reproducibility

Before publishing commands, check them against CMakeLists.txt, the actual executable targets, dependency discovery and CI workflows.

Current verified baseline:

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
~~~

For a Qt 6 Linux desktop build, document the Qt development dependency and the Ninja configure path used in CI. For Windows, state that Visual Studio 2022 and Qt 6 must be installed/configured; distinguish commands from expected output. Keep Python 3.10–3.12 in the engineering-tooling matrix rather than describing Python as an audio-rendering runtime dependency.

Treat fixed seeds, PCM format, source constraints, output paths, overwrite behavior and JSON schema versions as part of reproducibility. Use small, deterministic fixtures rather than large generated audio packs in tests.

## 4. Screenshots and visual evidence

Use actual captures from the built application. A generated illustration, wireframe or architecture drawing must never be presented as a live product screenshot.

- Keep the canonical capture in assets/screenshots/ with a stable descriptive filename.
- Capture a useful real workflow with readable text and meaningful controls, preferably at 16:9 desktop resolution.
- Add meaningful alt text and a short caption explaining the visible state and capture context.
- Refresh the screenshot after material UI changes. Use the existing screenshot workflow; do not silently substitute an older build or mockup.
- Keep additional views out of the opening flow when they do not help explain the first use case.

The current Windows desktop capture is assets/screenshots/ventusample-studio.png. The workflow diagram is a separate SVG illustration.

## 5. Theme-aware local assets

Prefer local SVG for logos, diagrams and interface illustrations. Raster screenshots should be optimized and reasonably sized; aim for less than 200 KB for small supporting images where visual quality permits, but do not destroy readable application evidence just to hit a file-size target.

Use paired light/dark assets with the same viewBox/canvas dimensions:

~~~html
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="assets/readme/diagram-dark.svg">
  <img src="assets/readme/diagram-light.svg" alt="A concise description of the diagram">
</picture>
~~~

Keep asset URLs repository-relative. Do not depend on external image hosts. Use 2–3 consistent brand colors, sufficient text contrast (WCAG AA target for text), safe interior padding and consistent icon/stroke language. Avoid giant animated GIF headers, decorative badge clutter and mixed visual styles without a documented reason.

For the public GitHub README, use theme selection for the documentation image. Do not claim that a paired README graphic means the Qt application itself has a light/dark mode; document the product's actual theme controls separately.

## 6. Diagrams and architecture

Use one small Mermaid diagram for one flow: parameters/seed, generator, buffer, offline DSP and exported artifact. A diagram should explain one concept at a glance, not become a repository-wide map of every subsystem.

Keep text explanation beside the diagram so the architecture remains understandable in plain-text views. Diagram labels must match code and contract names. Use SVG when a hand-composed vector communicates better, and keep the editable source in the repository.

## 7. Badges, links and change signals

Limit the hero to 3–5 useful badges, all with a consistent Shields style. Suitable signals include an existing CI workflow, the latest commit and the declared license. Do not add a release badge when there is no published release, a coverage badge when coverage is not measured, or arbitrary decorative labels.

Use direct links to actual workflows, commits, schema files, docs and release artifacts. Never invent a binary download, hosted demo or changelog path. Link to the commit history if no release/changelog stream exists.

Keep a compact Revival Highlights section for material documentation changes and link to a changelog only when one exists. Show a short, honest Legacy-to-Current comparison; don't claim unmeasured speedups. Roadmap checkboxes should be evidence-based.

## 8. Navigation, localization and repository structure

Keep the canonical English README and the Russian, Ukrainian and Simplified Chinese versions aligned in scope, status, build commands, limitations and safety notes. Do not mix languages within a sentence or allow translated files to retain stale features.

Use the documentation index as a route into focused topic hubs; avoid duplicating entire architecture or feature specifications in multiple READMEs. Include a compact ecosystem map where repositories integrate, while keeping ownership and optional dependency boundaries explicit.

Show only the key repository paths a contributor needs. Include a link to CONTRIBUTING and a small local verification sequence. Expand acronyms on first use when they are project-specific.

## 9. Automation and quality gates

The repository CI currently validates translated README structure and local links through .github/scripts/check_readme_links.py, validates Python engineering contracts, compiles tooling, builds/tests the native core, and builds/tests Qt desktop targets on Linux and Windows. The prose-quality workflow runs tools/check_prose_quality.py.

These checks do not replace a human review of layout, facts, image rendering or mobile readability. Markdownlint and external link-checking tools are not currently declared as required CI gates; add them only through a separately tested workflow change. Snippets should be copied into the relevant test/CI path where they can be executed deterministically.

For each README change, inspect:
- heading structure and language parity;
- all local link targets and image paths;
- code fences and copy-ready commands;
- badge/workflow/license targets;
- SVG pair viewBox dimensions and contrast;
- screenshot freshness and truthful captions;
- table wrapping on mobile width;
- status, supported platform and roadmap claims.

## 10. Contributing, license and publication checklist

Keep contribution guidance close to the root README. Explain native build/test expectations and ask contributors to preserve deterministic output, bounded parsing, safe file handling and offline rendering. Credit key dependencies and standards in their dedicated documents rather than adding decorative acknowledgements.

The root README must link to the repository's Apache-2.0 LICENSE file. Do not add new license or ownership claims for generated or imported audio.

Before publishing a README update:

- [ ] A reader can explain the project and intended audience from the first sentence.
- [ ] The first screen shows a useful visual, status, language links and real build/test entry.
- [ ] Commands and compatibility claims match CMake and CI.
- [ ] Every local link, image and badge target exists.
- [ ] Light/dark visual variants have matching dimensions and readable contrast.
- [ ] The screenshot is a genuine current application capture with alt text and caption.
- [ ] Diagrams reflect implemented data flow and have a text explanation.
- [ ] Translations agree on capability, maturity and limitations.
- [ ] Roadmap and release claims are factual; no placeholder/TODO is left on the public front page.
- [ ] License and contribution links are present.

The governing principle is respect for reader time: present a working product and honest evidence before visual polish or aspirational scope.
