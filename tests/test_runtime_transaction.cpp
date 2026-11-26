#include "runtime_transaction.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}
}

int main() {
    std::vector<std::string> events;
    ventu::RuntimeTransaction* observed = nullptr;
    std::thread callbackReader;
    bool callbackRanUnlocked = false;
    bool callbackCheckPerformed = false;
    std::uint64_t callbackObservedGeneration = 0u;
    ventu::RuntimeTransaction tx(
        {'a', 'b', 'c'}, 1u, 1024u,
        [&](std::string_view name, std::uint64_t) {
            events.emplace_back(name);
            if (name == "PATCH_STAGED" && !callbackCheckPerformed) {
                callbackCheckPerformed = true;
                std::promise<std::uint64_t> promise;
                auto generation = promise.get_future();
                callbackReader = std::thread([observed, promise = std::move(promise)]() mutable {
                    promise.set_value(observed->generation());
                });
                callbackRanUnlocked =
                    generation.wait_for(std::chrono::milliseconds(250)) == std::future_status::ready;
                if (callbackRanUnlocked) callbackObservedGeneration = generation.get();
            }
        });
    observed = &tx;

    require(tx.patch(1u, 1u, {'Z'}) == ventu::RuntimeStatus::Ok, "patch should stage a bounded byte edit");
    if (callbackReader.joinable()) callbackReader.join();
    require(callbackRanUnlocked && callbackObservedGeneration == 0u,
            "event callback must run after releasing the transaction lock");
    require(tx.rebind("sensor.bank1", 7u) == ventu::RuntimeStatus::Ok, "rebind should update symbolic binding");
    require(tx.remap({2u, 1u, 0u}) == ventu::RuntimeStatus::Ok, "remap should reorder staged bytes");
    require(tx.hotSwap({'a', 'b', 'c'}) == ventu::RuntimeStatus::Ok, "hotSwap should stage a replacement payload");
    require(tx.migrate(2u, [&tx](const auto& input, std::uint32_t from, std::uint32_t to) {
        require(tx.generation() == 0u, "migration callbacks must run outside the transaction lock");
        require(from == 1u && to == 2u, "migration should receive source and target schemas");
        auto output = input;
        output.push_back('!');
        return output;
    }) == ventu::RuntimeStatus::Ok, "monotonic schema migration should stage");
    require(tx.validate(), "staged CRC must validate");
    auto pending = tx.asyncFlash(0u);
    require(ventu::RuntimeTransaction::await(pending) == ventu::RuntimeStatus::Ok, "async flash should resolve when awaited");
    require(tx.generation() == 1u, "await should commit one generation");
    require(tx.flash(0u) == ventu::RuntimeStatus::Conflict, "stale expected generations must conflict");

    require(tx.patch(0u, 1u, {'x'}) == ventu::RuntimeStatus::Ok, "second patch should stage");
    require(tx.rollback() == ventu::RuntimeStatus::Ok, "rollback should restore committed payload");
    const auto snapshot = tx.snapshot();
    require(snapshot.schemaVersion == 2u, "rollback must preserve committed schema version");
    require(snapshot.payload == std::vector<std::uint8_t>({'a', 'b', 'c', '!'}), "rollback should restore the committed bytes");
    require(tx.reflash(1u) == ventu::RuntimeStatus::Ok, "reflash should commit with an explicit generation");
    require(tx.sync() == ventu::RuntimeStatus::Ok, "sync should commit current staged state");
    require(tx.emit("CATALISK_STABLE") && tx.yieldCheckpoint("checkpoint-1"), "emit and yield checkpoints should be observable");

    require(ventu::RuntimeTransaction::returnValue(9) == 9, "return helper should preserve values");
    require(ventu::RuntimeTransaction::catchError([&] { tx.throwError("recoverable"); }) == "recoverable",
            "catch should turn a thrown runtime error into a diagnostic");
    require(ventu::RuntimeTransaction::catchError([&] { tx.panic("guarded"); }) == "panic: guarded",
            "panic should be catchable and must not terminate the host");
    require(ventu::RuntimeTransaction::catchError([&] {
        ventu::RuntimeTransaction::assertCondition(false, "assertion rejected");
    }) == "assertion rejected", "assert should fail through the runtime error channel");
    require(!events.empty(), "transaction events should be emitted");

    /* The owner may release the object before its future is consumed. */
    std::promise<void> callbackEntered;
    std::promise<void> releaseCallback;
    const auto callbackGate = releaseCallback.get_future().share();
    auto guarded = std::make_unique<ventu::RuntimeTransaction>(
        std::vector<std::uint8_t>{'x'}, 1u, 1024u,
        [&](std::string_view name, std::uint64_t) {
            if (name == "FLASH_COMMITTED") {
                callbackEntered.set_value();
                callbackGate.wait();
            }
        });
    auto outstanding = guarded->asyncFlash(0u);
    callbackEntered.get_future().wait();
    std::atomic<bool> destroyed{false};
    std::thread destroyer([&] {
        guarded.reset();
        destroyed.store(true, std::memory_order_release);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    const bool destructorWaited = !destroyed.load(std::memory_order_acquire);
    releaseCallback.set_value();
    destroyer.join();
    require(destructorWaited, "transaction destruction must wait for active asyncFlash workers");
    require(destroyed.load(std::memory_order_acquire), "transaction should be destroyed after its worker exits");
    require(outstanding.get() == ventu::RuntimeStatus::Ok,
            "future may be consumed safely after transaction destruction");

    std::cout << "VentuSample runtime transaction tests passed.\n";
    return 0;
}
