# Veelsevul AppGuard v1

This directory contains the shared repository/runtime security contract for **VentuSample**.

## Coverage model

- **66 repository rules**: A-01..A-15, M-01..M-15, Я-01..Я-15, K-01..K-15 and O-01..O-06.
- **666 application controls**: L001..L666, split into six echelons of 111 controls each.
- Every AppGuard level links to repository rules; the reverse mapping is validated.
- Verdicts: `allow`, `quarantine`, `deny` and `degrade`. The default is `deny`.

## Echelons

| Range | Boundary |
|---|---|
| L001–L111 | Hardware, kernel, process, memory, concurrency and runtime foundation |
| L112–L222 | Ingress, parsers, encodings, paths and deserialization |
| L223–L333 | Identity, authentication, authorization, sessions and cryptography |
| L334–L444 | Persistence, transactions, caches, queues, privacy and recovery |
| L445–L555 | Network, external services, plugins, IPC and software supply chain |
| L556–L666 | Audit, telemetry, quotas, abuse handling, degradation and correlation |

## Runtime API

The dependency-free C11 ABI in `runtime/appguard.h` and `runtime/appguard.c` provides payload and collection ceilings, identifier allowlists, relative-path traversal rejection, explicit authorization decisions and structured audit callbacks.

Build and execute the smoke tests:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Werror \
  .veelsevul/runtime/appguard.c \
  .veelsevul/runtime/test_appguard.c \
  -o /tmp/veelsevul-appguard-test
/tmp/veelsevul-appguard-test
```

## CI contract

`.github/workflows/veelsevul-appguard.yml` validates the registry, bidirectional mappings and exception expiry; compiles/runs the C runtime tests; and scans changed text files for high-confidence credential signatures and hidden Unicode format controls.

## Enforcement boundary

**A registry entry is not automatically an active security mechanism.** A level is active only after its check is connected to a real application call site or configuration, emits evidence, and has a passing test. This release establishes the common policy, CI gate and portable runtime primitives; it does not claim every application subsystem is already rewired.

The runtime library does not independently configure OS sandboxes, TLS, identity providers, cryptographic key storage, databases, plugin processes or network policy. Those integrations remain owned by their respective application layers and must use the mapped AppGuard controls.

## Exceptions

No exceptions are pre-authorized. Each exception requires an owner, reason, approver, exact rule scope, creation date and expiry within 30 days. Expired or malformed exceptions fail validation.
