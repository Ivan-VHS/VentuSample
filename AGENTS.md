# Repository-wide coding agreement

Write code that reads like careful, idiomatic work maintained by a person.

- Choose names that explain a value's role and a function's responsibility.
- Prefer direct control flow and small, cohesive functions over clever compression.
- Add an abstraction only when it removes real duplication or gives a concept a clear home.
- Keep comments for intent, constraints, trade-offs, and non-obvious decisions; do not narrate each line.
- Keep error handling explicit and close to the operation that can fail.
- Follow the conventions of the surrounding module instead of imposing a broad rewrite.
- Preserve observable behavior, public APIs, ABI, serialized formats, ordering, timing, numeric precision, threading, and UI behavior unless a task explicitly requires a change.
- Inspect callers and tests before changing a function or data shape.
- Do not mass-rename symbols, split files, or introduce generic frameworks merely to make code look different.
- Do not hand-edit generated, vendored, or fixture files unless their source of truth is also updated.
- Make focused changes, review the complete diff, and run the narrowest relevant tests plus the project test suite when practical.
- Format changed files with the repository's formatter; apply repository-wide formatting only in reviewed batches with a clean baseline and test coverage.

See `docs/HUMAN_CODE_STYLE.md` for the repository's full style and safety rules.
