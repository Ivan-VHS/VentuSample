# Language and editorial quality

This document defines the writing standard for this repository. It applies to user-facing text, documentation, command-line help, diagnostics, settings, examples, release notes and contribution guidance. It does not prescribe the voice of the product or flatten every project into one style.

## The goal

Good writing lets a reader understand the intended meaning without having to repair the sentence mentally. Grammatical correctness is necessary, but it is not enough: accuracy, terminology, structure and the author's intended voice matter too.

Review text across four connected layers:

1. **Spelling and orthography.** Check spelling, inflection, capitalization, compound words, hyphenation and locale-specific forms. Distinguish a one-off typo from a recurring knowledge gap.
2. **Punctuation and sentence logic.** Place punctuation according to sentence structure, not breathing pauses. Check clause boundaries, parenthetical phrases, lists, quotations and paired punctuation.
3. **Lexical accuracy.** Choose words that express the intended meaning. Check collocations, confusing near-synonyms, technical terms and the difference between a measured result, an estimate, a simulation and a proposal.
4. **Syntax and style.** Keep the actor and action clear, make agreement unambiguous, avoid dangling modifiers, remove needless repetition and split sentences whose structure is difficult to follow.

## Editorial principles

- Preserve the author's meaning, tone and level of formality. Improve clarity; do not replace a distinctive voice with generic corporate prose.
- Prefer concrete verbs and direct wording over inflated noun phrases. Keep specialist terminology where it carries real meaning.
- Use one term for one concept throughout a document set. Record deliberate exceptions in the project's glossary or terminology notes.
- Explain an acronym at first use when the intended audience may not know it. Do not expand established identifiers, API names, commands, file formats or symbols.
- Make the subject of each action explicit. A sentence must not imply that a file, device or process performed an action that belongs to the user or another component.
- Keep claims proportional to evidence. Label implemented, experimental, simulated, estimated, planned and unsupported behavior accurately.
- Remove tautologies, empty intensifiers, redundant qualifiers and repeated explanations unless repetition helps navigation.
- Prefer a readable sentence over a forced brevity rule. Split a sentence when its clauses obscure the main point, not merely because it is long.

## Multilingual writing

- Write each locale as a natural text in that language; do not translate word-for-word when that damages meaning or idiom.
- Follow the spelling, punctuation, quotation marks, capitalization and date/number conventions of the document's declared locale.
- Keep product names, identifiers, code, command examples, API fields and file paths exact.
- Keep translated versions aligned in meaning and feature status. A translation must not add promises, remove limitations or silently change technical behavior.
- When a language-specific construction is uncertain, consult a reputable dictionary or an authoritative language reference rather than guessing from a machine translation.
- Do not impose one locale's punctuation rules on another. In particular, do not treat English punctuation as a universal template for Russian, Ukrainian or Chinese text.

## Practical review workflow

1. **Collect evidence.** Review real README files, guides, UI labels, CLI help, error messages, release notes and recent changes. Record recurring issues instead of repeatedly studying unrelated rules.
2. **Classify each issue.** Mark it as a typo, a rule/grammar issue, a terminology or meaning issue, a structural issue, or a style preference. Do not confuse a rushed typo with a systematic problem.
3. **Fix the highest-impact problems first.** Resolve misleading claims, ambiguous instructions, incorrect terminology and broken sentence structure before cosmetic preferences.
4. **Read in context.** Check the surrounding paragraph, headings, links, code examples and rendered Markdown. Read important passages aloud or review them after a short break.
5. **Verify every edit.** Confirm that links, commands, identifiers, examples, localized versions and technical claims remain correct. Text edits must not change program behavior accidentally.
6. **Keep a small terminology and decisions log.** Record only recurring or consequential decisions, with good and bad examples. Avoid turning the guide into a list of arbitrary exceptions.

## Review checklist

Before merging user-facing text, ask:

- Can a reader identify the subject, action, condition and expected result?
- Is every sentence grammatically complete and logically connected to the next one?
- Are spelling, inflection, agreement, punctuation and paired delimiters correct for this locale?
- Are words used in their exact sense and combined naturally?
- Are technical terms consistent, and are estimates or limitations clearly labelled?
- Can any filler, repetition, bureaucratic phrasing or unnecessary clause be removed without losing meaning?
- Do translations preserve the same facts, warnings and maturity status?
- Does the rendered document remain readable, accessible and navigable?

## Automated checks and their limits

Run:

```sh
python tools/check_prose_quality.py
```

The repository checker is deliberately conservative. It reports a small set of obvious mechanical patterns in Markdown prose, such as duplicated adjacent words and spaces before punctuation. It ignores fenced and inline code as far as practical. Its findings are prompts for review, not automatic rewrite instructions.

A passing check does **not** certify spelling, grammar, factual accuracy, natural phrasing, translation quality or accessibility. Automated lint must never replace contextual editorial review. Do not rewrite source text blindly to silence a warning.
