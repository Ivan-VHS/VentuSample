# Documentation Control

**Status:** normative  
**Scope:** all public documentation, examples, schemas, release notes and repository metadata.

Published documentation describes supported product behavior and stable data contracts. It is not an archive of planning sessions, private inputs or implementation history.

## Publication requirements

1. **Evidence:** claims marked as implemented MUST be supported by checked-in code and relevant tests. A proposal or schema alone does not establish implementation.
2. **Private material:** public documentation MUST NOT identify, quote or fingerprint private attachments, local working files, Library objects, chat threads, prompts, personal notes, internal share handles or unpublished reference material. Filenames, hashes, sizes, line counts and storage identifiers MUST NOT be published.
3. **Public surface:** document observable behavior, prerequisites, supported formats, versioned interfaces, operational limits, security boundaries and known exclusions. Internal file maps, local paths and design history MUST be omitted unless required by a supported integration.
4. **Normative language:** use MUST, MUST NOT, SHOULD, SHOULD NOT and MAY consistently. Each requirement MUST identify its scope and a verifiable acceptance condition.
5. **Capability status:** distinguish implemented, partial, planned and unsupported behavior. Roadmap items MUST NOT be presented as shipped features.
6. **Data protection:** credentials, access tokens, private endpoints, personal identifiers and unredacted local paths MUST NOT appear in public documentation. Digests establish byte identity, not authenticity or safety.
7. **Integration:** expose only versioned contracts and minimum consumer obligations. Private implementation details MUST NOT become cross-project dependencies.
8. **Change control:** changes to public behavior MUST update affected specifications, schemas, examples and compatibility notes in the same change. Breaking changes require a new interface version or a documented migration rule.

## Review gate

Before publication, verify links, examples, quantitative claims and capability status. Remove private provenance and unnecessary internal detail. Tests and schemas take precedence over descriptive prose when they conflict.
