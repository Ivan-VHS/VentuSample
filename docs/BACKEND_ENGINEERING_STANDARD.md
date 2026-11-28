# Backend Engineering Standard

**Version:** 1.0  
**Owner:** Ivan-VHS  
**Applies to:** backend services, operating-system services, runtimes, compilers, native cores, device adapters, local processing pipelines and their storage boundaries.

## 1. Purpose and interpretation

This standard turns extensibility, completeness, full functionality, scalability, modularity, integrity, flexibility, adaptability, universality, compatibility, configurability, evolvability, interoperability, resilience, maturity and self-sufficiency into reviewable engineering properties.

These are **quality goals, not blanket implementation claims**. A repository is not "fully functional" merely because the profile, documentation or CI validator exists. Every capability must be assessed against the actual product boundary and supported by source code, focused tests and operational evidence.

The word *backend* includes the internal service/core behind an application. It does not require every project to become a network server. Do not introduce a database, message broker, cluster, always-on daemon, remote control plane or privileged host service without a product requirement and a separate threat/performance review.

## 2. System model and boundaries

Use an explicit flow appropriate to the product:

`transport / host adapter → boundary validation → application use case → domain rules → storage or external adapter`

- Transport code handles protocol framing, authentication, request decoding, deadlines and response mapping. It does not own business rules.
- Domain logic consumes typed values and explicit capabilities. It does not depend on HTTP, SQL, UI widgets, OS handles or hidden global state.
- Storage and device access stay behind narrow interfaces with explicit ownership and failure contracts.
- Modules expose stable interfaces. Internal implementation details remain private.
- Cross-module changes are versioned and reviewed as contract changes, not silently propagated through shared mutable state.

A local CLI, stdio LSP/DAP, offline audio engine or read-only hardware inspector can use file, process, IPC or device adapters without pretending to be a hosted service.

## 3. Contracts, validation and idempotency

Every public boundary defines input and output types, required fields, units, valid ranges, maximum lengths, error codes and compatibility rules. Version serialized formats and protocol envelopes. Reject unknown or malformed data deliberately; do not silently coerce security-sensitive values.

Validate at the trust boundary and repeat critical invariants at the owning domain or storage boundary. Use parameterized database operations and safe parsers. File and archive processing must constrain size, nesting, entry count, decompressed size and path resolution.

Operations that may be retried or submitted twice must define idempotency. Use a caller-supplied operation key or an optimistic generation/version check where appropriate. Repeating a successful request must not create duplicate records, apply a migration twice or corrupt state. Do not add idempotency machinery to pure operations that cannot mutate state.

Configuration, ports, limits and feature flags belong in typed configuration. Secrets belong in an approved secret store or protected host configuration, never in source control, example fixtures, command output or logs.

## 4. Low-level I/O and transport

Choose the I/O model from the target platforms and measured workload:

- Use a bounded event-driven non-blocking loop for many concurrent sockets or descriptors when the target supports it. Relevant platform mechanisms include `epoll`, `kqueue`, `IOCP` and `io_uring`; they are alternatives, not a requirement to implement all of them.
- Use bounded worker pools for blocking filesystem, database, device or legacy API calls.
- Isolate CPU-heavy jobs from latency-sensitive event loops and real-time audio or UI paths.
- Define framing, maximum payload, partial-read/write handling, cancellation, deadlines and ownership for every stream.
- Reuse connections or handles only when the lifetime and thread-safety rules are explicit. Close descriptors and release handles on success, cancellation and every error path.
- Stream large results in bounded chunks or pages. Do not load unbounded files, exports or result sets into memory.

A process-local adapter or offline file pipeline should use the same bounded-I/O discipline without exposing an unnecessary network listener.

## 5. Concurrency, scheduling and backpressure

All work queues, worker counts, outstanding requests and per-client workloads need explicit limits. Define scheduling priority, cancellation and shutdown behavior. Do not spawn an unbounded thread or task per request.

Never block an event loop, UI thread or real-time DSP callback on slow I/O, an unbounded lock or a long computation. Use ownership transfer, immutable messages or narrowly scoped synchronization. Shared state must have a documented concurrency model.

When capacity is reached, apply backpressure or reject work with a clear, recoverable result. A bounded queue is preferable to silently growing memory use. Load tests should cover saturation, cancellation, contention and recovery where concurrency is part of the product.

## 6. Memory and resource lifetime

Every allocation, buffer, handle, file descriptor, lock, timer, connection and worker has an identifiable owner and end of lifetime.

- In C/C++, prefer RAII and explicit ownership; use smart pointers, spans or bounded views where suitable. Raw pointers must not outlive their owner.
- In systems code, document allocator/arena ownership, alignment, maximum capacity and reset rules.
- In managed runtimes, bound parsed structures, recursion, caches, queues and retained snapshots.
- Validate sizes before allocation and guard integer arithmetic against overflow.
- Release resources on all return, exception, timeout, cancellation and shutdown paths.
- Clear secrets from memory when the threat model requires it; do not promise perfect zeroization without a verified implementation.

## 7. Data integrity and transactions

Treat the data store as a final integrity boundary. Use schema constraints, unique keys, foreign keys and transactions for related changes. Make the transaction's commit point explicit.

For concurrent updates, use locks or optimistic versions/generations and return a recognizable conflict instead of overwriting newer state. Migrations are versioned, monotonic where possible, backed up and paired with compatibility and rollback tests.

Normalize wall-clock timestamps to UTC for interchange. Use a monotonic clock for elapsed time, deadlines and timeout accounting. Keep analytical work out of latency-sensitive transactions. Backups count as a safety feature only after restoration has been tested.

## 8. Failure handling and lifecycle

Assume external processes, storage, devices and networks can fail or disappear. Every blocking call has a deadline or cancellation path. Retries are finite and are permitted only when the operation is safe to repeat or has an idempotency key; use exponential backoff and jitter to avoid synchronized retry storms.

Use a circuit breaker only for a real dependency failure mode and define its threshold, observation window, cooldown and recovery behavior. Document safe degraded behavior. Do not hide integrity errors or widen permissions after a failure.

Long-running services should stop accepting new work, drain or cancel active work according to policy, flush durable state and release resources during graceful shutdown. Provide liveness and readiness separately when a service is actually supervised or load-balanced. Local tools should expose clear exit statuses and leave user data recoverable.

## 9. Security and privacy

Apply least privilege to processes, tokens, adapters and data access. Authenticate callers where a trust boundary exists and authorize each operation against the specific object and action; do not rely on an object's identifier alone. Protect against IDOR, injection, path traversal, unsafe deserialization and confused-deputy behavior.

Use TLS for network links that cross an untrusted boundary. Store passwords with a purpose-built, salted password-hashing scheme; verify signatures and trust chains rather than treating a hash as authorship proof. Rate-limit exposed interfaces. Redact credentials, tokens, personal data and device identifiers from logs and reports unless explicitly required and protected.

A local-only product should document that boundary and avoid adding outbound telemetry or remote control by default.

## 10. Observability and diagnostics

Operational paths should emit structured events with a correlation or operation ID where applicable. Record failure category, stage and safe diagnostic context; never log secrets or entire untrusted payloads by default.

For services and measurable processing pipelines, collect relevant latency distributions (`p50`, `p95`, `p99`), error rates, queue depth, CPU, memory and disk/network I/O. Metrics need units, sampling rules and cardinality limits. Use tracing when work crosses meaningful subsystem boundaries. Alerts should indicate user impact and a concrete action.

A local application may use bounded logs, diagnostic reports and reproducible benchmark outputs rather than distributed tracing.

## 11. Capacity, scaling and degradation

Measure before optimizing. Record workload shape, input size, hardware, environment and repeatability conditions. Define resource budgets and performance baselines for hot paths. Use batching, indexes, streaming and caching only with measurable benefit and explicit invalidation/expiry rules.

Keep horizontally scalable service state recoverable and outside a single process when the product actually requires multiple instances. Do not claim horizontal scalability from modular code alone. Under overload, shed optional work and return honest statuses such as `429` or `503` on applicable service interfaces; local applications should report a bounded, actionable resource error instead.

## 12. Configuration, extensibility and integration

Prefer narrow versioned interfaces, explicit dependency injection and capability-based adapters. Extensions must declare their contract, permissions, lifecycle and compatibility range. Discovery must not imply execution; loading untrusted plugins or drivers requires a separate sandbox and supply-chain review.

Configuration is validated at startup and on update. Invalid critical configuration fails clearly before the component accepts work. Changes are atomic or staged when partial application would leave inconsistent state. Preserve backward compatibility or provide a migration path for persisted formats and public interfaces.

## 13. Verification and delivery

A change is complete only when it includes the implementation, boundary tests, failure-path tests, documentation and any required migration. Tests should cover malformed input, size limits, timeouts, cancellation, duplicate operations, conflicts, rollback, resource cleanup and permission failures where applicable.

Use reproducible builds, pinned dependencies, provenance and software-bill-of-materials evidence for releases. CI must validate the machine-readable profile and its negative cases. The profile validator checks declarations and referenced paths; it does **not** prove runtime behavior or production readiness.

Every repository change is delivered as a separate pull request. Review the diff, run relevant checks, inspect CI results and merge sequentially into `main`. Never bypass a failing required check or represent an unavailable check as passed.

## 14. Evidence and maturity states

Each repository maintains `.architecture/backend-engineering.v1.json`:

| Status | Meaning |
|---|---|
| `implemented` | The exact control exists and has focused verification evidence. |
| `partial` | Some behavior exists; evidence and remaining limitations are recorded. |
| `planned` | Accepted work with no implementation claim. |
| `not_applicable` | Outside the product boundary, with a reason. |
| `not_assessed` | Evidence is insufficient to make a reliable claim. |

Use the narrowest truthful status. A design note, catalog entry, proposed API or passing profile validator is not evidence that an underlying runtime feature exists. Update the profile in the same pull request as the owning implementation. The included validator checks the profile shape, safe repository-relative paths, evidence references and status rules; product tests remain authoritative for behavior.

## 15. Review acceptance checklist

- [ ] Boundaries, ownership and public contracts are explicit.
- [ ] Inputs, sizes, arithmetic, deadlines and resource budgets are bounded.
- [ ] Blocking work cannot stall latency-sensitive paths.
- [ ] State changes are transactional or have a documented recovery path.
- [ ] Retry, concurrency and backpressure policies are finite and testable.
- [ ] Privileges, secrets and sensitive diagnostics are controlled.
- [ ] Failure, cancellation, shutdown and cleanup paths are tested.
- [ ] Measurements and maturity claims match the implementation.
- [ ] The repository profile and CI gate are updated and pass.
- [ ] The PR is reviewed and merged only after its checks are verified.

Project author/developer: **Ivan-VHS**. Preserve the authorship, contributions and license notices of existing contributors and dependencies.
