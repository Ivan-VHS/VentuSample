#!/usr/bin/env bash
set -euo pipefail

repo="${GITHUB_REPOSITORY:-}"
repo="${repo##*/}"
repo="${repo:-$(basename "$PWD")}"
failures=0

require_file() {
  if [[ ! -f "$1" ]]; then
    printf '::error file=%s::required file is missing\n' "$1"
    failures=1
  fi
}

require_dir() {
  if [[ ! -d "$1" ]]; then
    printf '::error file=%s::required directory is missing\n' "$1"
    failures=1
  fi
}

for path in README.md LICENSE CONTRIBUTING.md SECURITY.md; do require_file "$path"; done
for path in .github/workflows docs tests schemas scripts assets; do require_dir "$path"; done
require_file docs/REPOSITORY_STRUCTURE_PROFILE.md
require_file scripts/verify_repository_structure.sh

shopt -s nullglob
workflows=(.github/workflows/*.yml .github/workflows/*.yaml)
if (("${#workflows[@]}" == 0)); then
  printf '::error::.github/workflows must contain at least one workflow\n'
  failures=1
fi

case "$repo" in
  Pytyliki)
    for path in src include cmake fixtures; do require_dir "$path"; done
    for path in CMakeLists.txt project-contract.json; do require_file "$path"; done
    ;;
  Reaper)
    for path in reaper language src include stdlib frameworks tests; do require_dir "$path"; done
    for path in CMakeLists.txt project-contract.json; do require_file "$path"; done
    ;;
  OPlague)
    for path in oplague crates tools; do require_dir "$path"; done
    for path in pyproject.toml project-contract.json; do require_file "$path"; done
    ;;
  VentuSample)
    for path in src tests examples; do require_dir "$path"; done
    for path in CMakeLists.txt project-contract.json; do require_file "$path"; done
    ;;
  DAK)
    for path in duckos kernel desktop foundation boot; do require_dir "$path"; done
    for path in CMakeLists.txt package.json engineering-evolution.v1.json; do require_file "$path"; done
    ;;
  Kaskad)
    for path in src/kaskad_k3 tests/vectors docs/foundations docs/theory docs/proofs docs/extensions docs/classical-links docs/open-problems docs/applied-models formal; do require_dir "$path"; done
    for path in project-contract.json pyproject.toml CITATION.cff CITATION.bib docs/VYUNENKO_THEORY.md; do require_file "$path"; done
    ;;
  *)
    printf '::error::No repository-specific structure profile for %s\n' "$repo"
    failures=1
    ;;
esac

if ((failures)); then
  printf 'Repository structure validation failed for %s.\n' "$repo"
  exit 1
fi
printf 'Repository structure validation passed for %s.\n' "$repo"
