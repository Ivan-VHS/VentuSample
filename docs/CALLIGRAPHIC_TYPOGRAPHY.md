# Calligraphic Typography

This document defines the approved calligraphic font set and its use in VentuSample. The goal is to add human, handwritten accents without reducing the readability of technical screens, system controls or diagnostic data.

## Approved font families

### Cyrillic-capable set

| Font | Intended use |
|---|---|
| Lobster | Compact brand marks and short display headings |
| Marck Script | Elegant headings and short notes |
| Bad Script | Natural handwritten accents |
| Comforter | Expressive poster-like accents |
| Comforter Brush | Large, brush-style decorative titles |
| Caveat | Short handwritten notes and secondary headings |
| Pacifico | Rounded retro display accents |
| Miama Nueva | Light calligraphic headings and ornamental text |

### Latin display set

These families are reserved for Latin-script content unless the exact font file being shipped is independently verified to contain the required glyphs.

| Font | Intended use |
|---|---|
| Great Vibes | Formal, high-contrast display accents |
| Alex Brush | Restrained signatures and labels |
| Allura | Minimal script accents |
| Pinyon Script | Formal titles and monograms |
| Italianno | Light ornamental headings |
| Parisienne | Informal retro-style accents |
| Herr Von Muellerhoff | Oversized ornamental marks |

## Usage rules

- Keep interface body text, menus, forms, tables, controls and technical data in the existing high-legibility UI family.
- Use **Lobster** or **Pacifico** for short brand/display text; use **Caveat**, **Marck Script** or **Bad Script** for brief handwritten notes.
- Reserve **Comforter** and **Comforter Brush** for large decorative surfaces. Do not use them for dense navigation or small labels.
- Never use a Latin-only font for Cyrillic text. Keep a script-capable fallback in every stack.
- Keep code, logs, identifiers, measurements and tabular numeric data in the existing monospace or UI family.
- Do not apply decorative fonts to every component. Typography should clarify hierarchy, not compete with content.

## Licensing and distribution

Google Fonts keeps a license file in each family directory. Check the exact family license before redistribution, and include its original license and copyright notices beside any bundled font files. The Google Fonts collection includes more than one open license, so do not infer a font's license from another family's license.

Miama Nueva is listed separately from the Google Fonts families. Verify the exact download's commercial-use license and retain its supplied notice before bundling or redistributing it.

Font binaries are not bundled by this change. Native applications use a preferred family only when it is installed on the host and otherwise keep the platform fallback. Browser interfaces must not load remote font CSS at runtime; local/offline behavior takes priority.

Sources: [Google Fonts](https://fonts.google.com/), [Google Fonts license-file guidance](https://googlefonts.github.io/gf-guide/license-file.html), [Google Fonts collection license notes](https://github.com/google/fonts/blob/main/README.md).

## VentuSample integration

Use locally installed display/script families for the brand title, hero heading and group-box headings. Keep parameter controls, plugin metadata, waveform/measurement text and numeric values in the existing Qt UI family.
