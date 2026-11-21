# Visual documentation and graphics contract

**Project:** VentuSample · **Scope:** README, Markdown/MDX, audio workflow diagrams, screenshots, graphics, typography, badges and native Qt motion references.

This contract defines how product behavior is visualized and documented. A diagram, shader reference or concept graphic must not be presented as an implemented audio capability.

## Documentation hierarchy

| Surface | Responsibility | Authority |
|---|---|---|
| `README.md` | Product purpose, real desktop screenshot, generation matrix, formats, supported controls and limits | Main branch and CI |
| `README.ru.md`, `README.uk.md`, `README.zh-CN.md` | Localized public entry points | Same facts and boundaries as English |
| `docs/README.md` | Navigation index for technical guides and schemas | Files present in repository |
| `docs/VISUAL_DOCUMENTATION.md` | This graphic, Markdown and rendering contract | Audio pipeline, schemas and UI contracts |
| `assets/` | Static diagrams and actual application captures | Source SVG or CI screenshot |

Use root `README.md` as the GitHub entry point. Do not add a duplicate `README.mdx` to compete with it. MDX is reserved for an actual docs website when interactive content components are necessary.

## Markdown typography and navigation

- Use one H1 and a concise factual lead, then H2/H3 sections in logical order.
- Use descriptive link labels, relative repository links, sentence-case titles and short paragraphs.
- Put command lines in fenced `sh` / `powershell` / `cpp` blocks; use inline code for paths, parameter names, schema versions and file formats.
- Tables should compactly express generator outputs, parameter ranges, audio formats and feature status. Distinguish **implemented**, **partial**, **planned**, **catalogued-only** and **not supported**.
- Give every meaningful image alt text. Label artwork as a diagram/concept and application captures as real screenshots with build/OS context.
- Do not use colour, animated waveforms, gradients or badges as the sole status indicator.

## Graphics, rasterization and rendering

### SVG and vector geometry

Use SVG for generator pipeline maps, plugin lifecycle explainers, parameter diagrams and static infographics. Preserve the SVG source in version control. Include a `viewBox`, descriptive `<title>` and `<desc>`, readable text, clear flow direction and adequate contrast. GitHub-facing SVG stays static and self-contained: no scripts, animation, event handlers, remote fonts, trackers or `foreignObject`. Use consistent geometry and alignment; avoid decorative effects that obscure audio signal flow.

### Screenshots, PNG, WebP and AVIF

The Windows/Qt desktop screenshot must be captured from the actual build/run workflow. Never call a vector mock-up or browser showcase a live application capture. Keep PNG as the dependable GitHub README image. WebP/AVIF can be optional website derivatives when the source PNG is retained as a fallback. Keep source SVG for diagrams and rasterize only when needed at target pixel density. Avoid lossy compression for fine text, small controls or waveform details.

### Colour, transparency, gradients and texture

Follow the [Anatidae desktop design system](ANATIDAE_DESKTOP_DESIGN_SYSTEM.md), [UI motion contract](UI_MOTION.md) and [UI/UX 2026 convergence](UI_UX_2026_CONVERGENCE.md). Use semantic action/focus and critical-state colors consistently. Gradients, glass-like surfaces, transparency, grain, glow and post-processing are optional presentation treatments, not signal-processing features. Keep parameter labels, meter units, waveform axes and file-status indicators legible in both light and dark themes.

## Animation, kinematics and audio simulation

README illustrations remain static. In the Qt UI, animations are state feedback only and run on the UI thread, never inside DSP callbacks, audio buffers, generator workers or stochastic-routing code. Reduce Motion and Eco suspend the beat indicator and cap retained transitions as documented in [UI_MOTION.md](UI_MOTION.md). The displayed BPM is the internal project tempo, not external DAW synchronization. Visual waveform/spectrum displays must be labelled as illustrative or measured according to their actual data source. Do not claim low latency, real-time playback, zero jitter or 60/120 FPS without reproducible measurement.

## Diagram and publishing toolchain

| Tool / format | Appropriate use | Repository policy |
|---|---|---|
| GitHub Markdown + Mermaid | Short generation, processing and export flows | Preferred source-first diagram in GitHub |
| PlantUML (`.puml`) | Plugin lifecycle, worker/UI interactions and component boundaries | Store source with static SVG fallback |
| D2 (`.d2`) | Complex audio pipeline and module relationships | Use if it materially clarifies the graph |
| SVG infographic | Stable visual fallback for README and release notes | Editable, static and accessible |
| Markdown (`.md`) | Portable canonical product documentation | Default README/docs format |
| MDX (`.mdx`) | Interactive components in a configured docs site | Not a replacement for GitHub README |
| Docusaurus | Versioned docs and sidebar structure | Optional adapter; not a current dependency |
| VitePress | Markdown content with Vue site components | Optional adapter; not a current dependency |
| Astro Starlight | Structured docs navigation with Markdown/MDX | Optional adapter; not a current dependency |
| MkDocs Material | Documentation site and navigation | Optional adapter; not a current dependency |
| Shields.io | CI, license and published release badges | Real endpoints only |
| GitHub Pages | Hosting a generated docs site | Only with an actual site build and deploy workflow |

Do not install multiple site generators just to support alternative formats. GitHub-flavoured Markdown is the portable baseline. Use Mermaid for concise source diagrams; retain a static output for hosts and offline readers that do not render a given syntax.

## VentuSample-specific visual boundaries

VentuSample is a native C++20 / Qt 6 Widgets workstation for deterministic procedural sample-pack generation. The scalar renderer exports offline PCM16 stereo WAV plus versioned JSON manifests. The optional Stochastic Router is bounded offline processing; plugin manifest discovery is metadata-only; the Quantum Audio Engine is a creative offline DSP catalogue with tested kernels, not physical quantum computation, a real-time DAW engine or a VST/AU host. The internal project BPM indicator is not an external DAW transport connection.

The generator flow should show deterministic parameters/seed, procedural rendering, bounded offline processing and the exported WAV/manifest pair. Keep the plugin platform distinct from the generator: discovered manifests are metadata-only until an explicitly attached reviewed handler exists. Describe Quantum Audio Engine modules as a DSP catalogue with reusable kernels; do not imply quantum hardware, physical simulation or arbitrary native plugin loading. The stochastic router remains an offline pre-export stage, not an external live insert.

## Review checklist

- [ ] All four README languages agree on formats, parameter ranges and processing limits.
- [ ] Desktop screenshot comes from the real Qt app build/run and has a factual caption.
- [ ] SVG includes title, description, viewBox and static vector source.
- [ ] Audio diagrams distinguish generator, offline DSP, plugin discovery and export.
- [ ] BPM is clearly described as internal project tempo, not external DAW sync.
- [ ] DSP visuals stay outside audio callbacks and generator workers.
- [ ] Badges link to live CI/license/release endpoints.
- [ ] `docs/README.md` indexes the visual contract.

## References

- [GitHub image and SVG rendering](https://docs.github.com/en/repositories/working-with-files/using-files/working-with-non-code-files)
- [GitHub Mermaid diagrams](https://docs.github.com/en/get-started/writing-on-github/working-with-advanced-formatting/creating-diagrams)
- [Docusaurus Markdown and MDX](https://docusaurus.io/docs/markdown-features)
- [VitePress Markdown](https://vitepress.dev/guide/markdown)
- [Astro Starlight authoring content](https://starlight.astro.build/guides/authoring-content/)
