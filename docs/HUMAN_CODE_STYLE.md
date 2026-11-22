# Human-readable code standard

The goal is code that is natural to read, easy to maintain, and faithful to the project's existing design. "Human-readable" does not mean adding decorative comments, unusual syntax, arbitrary renames, or unnecessary abstraction.

## Everyday code

- Use domain language in names. A reader should understand what a value represents without decoding abbreviations.
- Keep functions focused on one meaningful operation. Extract a helper when it names a real step or removes meaningful duplication, not just to reduce line count.
- Prefer clear branches, explicit early returns, and ordinary loops when they make the intent easier to follow.
- Keep data flow visible. Avoid hidden mutation, surprising global state, and clever one-liners in non-trivial logic.
- Handle failures deliberately. Preserve useful context in errors and avoid silently swallowing exceptions.
- Keep comments concise and explain why a decision exists, especially around compatibility, platform behavior, concurrency, numerical work, and security boundaries.
- Match the established style in a module when it is more specific than the repository-wide defaults.

## No-regression rule

A readability change must not silently alter behavior. Preserve public function signatures, input/output formats, defaults, serialization, error behavior, command-line output, arithmetic precision, DSP/audio timing, deterministic output, concurrency, resource lifetime, filesystem behavior, security checks, and UI interaction.

When a behavior change is required, isolate it from formatting or naming work and cover it with a test.

## Safe modernization

1. Read the surrounding module, its callers, tests, and relevant project documentation.
2. Capture a baseline with the narrowest relevant tests before editing.
3. Make a focused change and keep its diff reviewable.
4. Run the same tests afterward; add a regression test where the old behavior was not covered.
5. Review the diff for accidental API, data, generated-file, and whitespace-sensitive changes.
6. Do not claim a test passed unless it was actually executed and its result was observed.

## Formatting

- Python: Ruff formatter settings in `pyproject.toml`.
- C and C++: the repository's `.clang-format` profile.
- All text files: the repository's `.editorconfig` rules.
- Formatter configuration is a convention, not permission to reformat the entire history in one sweep. Apply formatting in small, reviewed groups after confirming that the tool version and project tests are available.

## Scope of this baseline change

This baseline adds guidance and formatter configuration only. It deliberately does not rewrite existing source files or alter build, runtime, package, or test behavior. Existing code should be modernized incrementally, with each batch checked against the no-regression rule above.
