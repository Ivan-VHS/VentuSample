# Anatidae Naming Canon — Shared Product Layer

**Status:** normative vocabulary and presentation guidance for the four-project federation.  
**Applies to:** Pytyliki, Reaper, OPlague and VentuSample.  
**Compatibility:** additive design guidance only; it does not rename products, APIs, protocols, schemas, commands, files or shipped capabilities.

## 1. Purpose

The Anatidae vocabulary gives the product family a recognizable, friendly identity without turning engineering software into a novelty interface. The bird and waterfowl language is a restrained presentation layer: it may help name onboarding, optional assistance, overview surfaces and small decorative details. It must never obscure what a control does or weaken technical precision.

The formal umbrella is **Anatidae**. Existing application names remain canonical and unchanged: **Pytyliki**, **Reaper**, **OPlague** and **VentuSample**.

## 2. Normalized vocabulary

Repeated entries in the source list are intentionally normalized here; spelling variants are preserved as distinct forms where supplied.

### DAK and dakling family
Утёнок · Утка · Утёныш · Утеночек · Уточка · Уточёнок · Утёночек · Утеныш · Утёнушка · Утюша · Утя · Утюня · Утюняш · Утюняшка · Утяшка

### Quack and waterfowl family
Крякуша · Кряква · Кряквашка · Кряквёнок · Крякушонок · Крякалыш · Крякозябра · Крякальщица

### Bird, wing and water family
Водоплавающая · Птица · Пташка · Пернатая · Крылатая · Пернатик · Крылыш · Прудовая · Озерная · Водоплавчик · Водяная птица

### Chick, feather and soft-detail family
Птенчик · Птенец · Птеныш · Пёрышко · Пуховичок · Пухляш · Пушистик · Лапчатая

## 3. Semantic roles

| Term family | Permitted design role | Preferred treatment |
|---|---|---|
| Утёнок, Утёныш, Уточка | Friendly guide, first-run helper, optional empty-state companion | Friendly, brief and optional |
| Утка, Водоплавающая, Птица, Пернатая | High-level family identity in governance and design documentation | Formal, never a replacement product name |
| Кряква, Крякуша, Кряквёнок | Internal concept names for optional guided workflows or grouped examples | Clearly secondary to the real feature name |
| Крылыш, Крылатая | Visual grouping for a compact command rail or navigation group | Preserve each command's explicit label |
| Прудовая, Озерная, Водяная птица | Metaphor for a home/overview/workspace surface | Use only where the actual surface is already an overview |
| Птенчик, Птенец, Птеныш | Beginner-facing setup and learning stages | Never imply that a setup action is safe by default |
| Пёрышко, Пернатик | Small, non-critical metadata or decorative marker | Never replace a value, measurement, evidence label or status word |
| Пуховичок, Пухляш, Пушистик, Лапчатая | Optional visual texture or friendly empty-state voice | No effect on density, contrast or hit target size |
| Кряквашка, Крякушонок, Крякалыш, Крякозябра, Крякальщица | Creative vocabulary for internal examples and optional onboarding copy | Do not use for errors, alerts, security or diagnostic findings |

These are presentation roles, not a request to add new product features. A term must not be attached to a control unless its real action is already clear in the same visible label.

## 4. Writing rules

1. Keep the actual function name first. A friendly term may follow as a secondary subtitle, helper label or illustration caption; it must not replace a functional label.
2. Use no more than one playful term in a heading or short tooltip. Avoid chains of synonyms in production UI.
3. Preserve standard technical nouns verbatim: CPU, device, driver, hardware ID, INF, hash, path, schema, manifest, seed, tempo, sample rate, key, scale, benchmark, source, output and report.
4. Do not rename public symbols, command-line flags, filenames, JSON keys, environment variables, protocol names, language keywords or serialized values.
5. Avoid playful language in errors, warnings, security prompts, consent dialogs, data-loss confirmations, compatibility findings, benchmark results and exported reports.
6. A friendly empty state must still state what is empty, why it is empty when known, and the next valid action. It must never fabricate a result.
7. Never use the absence of a mascot or themed label as a sign of failure. The interface must remain complete and understandable with all decorative identity elements disabled.

## 5. Visual and interaction rules

- Retain the existing white-blue-red semantic palette and each project's documented light/dark behavior. The Anatidae vocabulary does not authorize new status colors.
- A feather or wing motif may appear as a small outline, divider detail or quiet illustration. It must not compete with the product title, selected state, warning or primary action.
- Use existing icon/text conventions. An emoji or animal silhouette must never be the only representation of a control, state or action.
- Keep contrast, keyboard focus, tab order, hit areas, responsive behavior and reduced-motion support at least as strong as the current design contract.
- No perpetual flapping, bouncing, floating or autoplay animation. Motion is optional, short, state-driven and disabled under reduced-motion preferences.
- Keep the bird motif out of hashes, evidence payloads, file metadata, telemetry keys and machine-readable exports unless a separately versioned schema explicitly introduces such a field.

## 6. Product-specific application

### Pytyliki
Use **Утёнок** only for optional first-run guidance; **Прудовая** may describe an existing overview/workspace surface in design copy; **Пёрышко** may be a compact, secondary provenance cue when the full measurement record remains one click away. CPU inventory, benchmark names, TRACE provenance, safety boundaries and report data remain literal and technical.

### Reaper
Use **Крылыш** as an internal visual grouping metaphor for the existing command-bar groups, not as a replacement for Check, Run, Debug, Save or Open. **Птенчик** may appear in an optional first-run tutorial. Language grammar, keywords, diagnostics, runtime output, audit records and native/reference status remain exact and unmodified.

### OPlague
Use **Утёнок** or **Кряквёнок** only for optional onboarding guidance. **Пёрышко** may mark a secondary metadata summary only when the full hardware ID, hash, source and evidence remain explicit. Device inventory, INF/ZIP evidence, DRIVE-LENS, candidate-only compatibility semantics and all read-only safety language remain literal.

### VentuSample
Use **Утёнок** for optional first-pack guidance and **Крякуша** only for non-critical help around the existing variation workflow. **Пёрышко** may label a compact file-summary decoration while filename, format, sample rate, key, scale, seed and measurements remain visible. Do not add a genre, sound profile, sample format or generator behavior merely to use a vocabulary word.

## 7. Adoption and acceptance

The naming layer is accepted only when:
- application and API names remain unchanged;
- all real actions keep literal, unambiguous labels;
- errors, warnings, confirmations and exported data remain technical;
- no unsupported capability is implied by onboarding copy or illustration;
- both themes, keyboard navigation, screen-reader labels and reduced-motion behavior remain valid;
- the ordinary workflows remain understandable when the optional Anatidae layer is absent.

This canon is a controlled design vocabulary, not evidence that every proposed presentation element has already shipped. The repository source, tests and existing project contracts remain authoritative for implemented behavior.
