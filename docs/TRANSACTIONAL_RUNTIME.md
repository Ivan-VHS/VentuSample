# Runtime Transaction Contract v1

VentuSample exposes a bounded C++20 `ventu::RuntimeTransaction` in `src/runtime_transaction.hpp`.

## Operation map

| Operation | API | Meaning |
|---|---|---|
| `patch` | `tx.patch(offset, removeSize, bytes)` | Edit a bounded staged byte range |
| `rebind` | `tx.rebind(name, id)` | Update a symbolic binding |
| `remap` | `tx.remap(indexes)` | Reorder/copy bytes using validated source indexes |
| `hotSwap` | `tx.hotSwap(bytes)` | Stage a complete in-memory payload replacement |
| `migrate` | `tx.migrate(schema, transform)` | Apply a monotonic schema transformation |
| `flash` / `reflash` | `tx.flash()`, `tx.reflash()` | Commit using an optional expected generation |
| `rollback` | `tx.rollback()` | Restore last committed bytes and schema |
| `sync` | `tx.sync()` | Commit current staged state |
| `async` / `await` | `tx.asyncFlash()`, `RuntimeTransaction::await(future)` | Commit on a worker and resolve the future |
| `emit` / `yield` | `tx.emit(name)`, `tx.yieldCheckpoint(name)` | Publish an event/checkpoint |
| `throw` / `catch` | `tx.throwError(message)`, `RuntimeTransaction::catchError(fn)` | Throw/capture a typed runtime exception |
| `panic` / `assert` | `tx.panic(message)`, `RuntimeTransaction::assertCondition(...)` | Raise a guarded runtime error, never abort the process |
| `return` | `RuntimeTransaction::returnValue(value)` | Pass a value through |

Default payload limit is 16 MiB; CRC32 and optimistic generation checks protect commits. A panic/throw poisons the transaction until rollback. The caller must keep the transaction alive until its asynchronous future has been awaited.

## Plugin handler hot swap

`PluginManager::hotSwapHandler` replaces only an already attached in-memory native handler. The plugin must be disabled and not initialized. This is not a DLL/SO loader: manifest discovery remains metadata-only, and manifest entrypoints are never executed automatically. It also does not hot-swap the audio engine or run inside the real-time DSP path.

## Tests

CTest target: `ventusample-runtime-transaction`. The existing plugin-host test also verifies the disabled-only hot-swap guard.
