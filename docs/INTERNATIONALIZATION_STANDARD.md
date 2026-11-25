# Internationalization and language support

**Status:** i18n contract v1 and starter catalogs. This is the extensible locale foundation, not a claim that every screen or every human language is already translated.

## Universal policy

All well-formed BCP 47 tags are valid input. There is no finite language allowlist. A locale without a translated catalog remains usable for user text and platform formatting; untranslated product messages fall back through exact tag → script parent → language parent → English.

Baseline: UTF-8; Unicode 18.0; CLDR 49; BCP 47; grapheme-aware editing; font fallback; IME composition; bidi-safe mixed text; script-aware direction. Use host ICU/CLDR services rather than hand-written date, number, currency, calendar, plural or collation rules.

Machine contracts never change with UI language: file formats, preset keys, audio samples, API names, JSON keys, diagnostic codes and serialized numeric values remain invariant.

## VentuSample adapter

Use Qt 6 `QLocale` for number/date/measurement presentation and `QTranslator` for widget catalogs. UI text must be marked for translation and extracted into Qt Linguist `.ts` catalogs; compiled `.qm` files are installed with the application. The selected locale is persisted as a BCP 47 tag, not an index or display name.

The real-time audio callback, DSP graph, sample buffers and preset serialization remain locale-independent. Do not allocate, block, load translation catalogs or access widgets from the audio thread. Perform language switching on the UI thread and rebuild accessible names/tooltips together with visible labels.

## Starter catalogs and CI

The v1 catalog set includes source English and reviewed Russian, Ukrainian, Simplified Chinese and Arabic common actions. `complete` means complete only against the starter message-id set, not against every UI string. New messages must use stable ids and pass human review before promotion.

Run `node tools/check_i18n.mjs`. GitHub Actions runs the same check on changes to i18n resources. Add Qt UI tests for RTL layouts, CJK/Arabic/Devanagari font fallback, IME composition, grapheme cursor/deletion, accessibility labels and live locale switching.

## Adding a language

Add its canonical BCP 47 tag and local catalog, mark it `partial` until reviewed, verify placeholder parity and script direction, then mark `complete`. No cloud translation or remote locale dependency is permitted.
