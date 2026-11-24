#pragma once

#include <atomic>
#include <cstdint>

namespace ventu::diagnostics {

enum class QualityMode : std::uint8_t {
    Normal = 0,
    Reduced = 1,
    Emergency = 2
};

struct AudioTelemetrySnapshot {
    std::uint64_t callbacks{};
    std::uint64_t xruns{};
    std::uint64_t consecutiveOverruns{};
    std::uint64_t lastCallbackNs{};
    std::uint64_t peakCallbackNs{};
    std::uint64_t samplePosition{};
    std::uint64_t memoryBytes{};
    std::uint64_t memoryBudgetBytes{};
    std::uint32_t lastLoadPpm{};
    std::uint32_t peakLoadPpm{};
    QualityMode quality{QualityMode::Normal};
};

/* Callback-safe metrics: recordCallback performs no allocation, logging,
 * locking, filesystem access, or Qt calls. The audio engine is the sole
 * callback writer; the UI may read snapshots concurrently. */
class AudioTelemetry final {
public:
    void recordCallback(std::uint64_t elapsedNs, std::uint64_t budgetNs,
                        std::uint32_t frames, bool xrun = false) noexcept;
    void recordMemory(std::uint64_t usedBytes, std::uint64_t budgetBytes) noexcept;
    void recordDeviceHealth(bool healthy) noexcept;
    AudioTelemetrySnapshot snapshot() const noexcept;
    void resetPeaks() noexcept;

private:
    static void updatePeak(std::atomic<std::uint64_t>& peak,
                           std::uint64_t value) noexcept;
    static void updatePeak(std::atomic<std::uint32_t>& peak,
                           std::uint32_t value) noexcept;

    std::atomic<std::uint64_t> callbacks_{0};
    std::atomic<std::uint64_t> xruns_{0};
    std::atomic<std::uint64_t> consecutiveOverruns_{0};
    std::atomic<std::uint64_t> lastCallbackNs_{0};
    std::atomic<std::uint64_t> peakCallbackNs_{0};
    std::atomic<std::uint64_t> samplePosition_{0};
    std::atomic<std::uint64_t> memoryBytes_{0};
    std::atomic<std::uint64_t> memoryBudgetBytes_{0};
    std::atomic<std::uint32_t> lastLoadPpm_{0};
    std::atomic<std::uint32_t> peakLoadPpm_{0};
    std::atomic<bool> deviceHealthy_{true};
};


enum class RenderStage : std::uint8_t {
    Idle = 0,
    Preflight,
    Rendering,
    Processing,
    Exporting,
    Measuring,
    Manifest,
    Complete,
    Failed
};

struct RenderTelemetrySnapshot {
    bool active{};
    bool succeeded{};
    RenderStage stage{RenderStage::Idle};
    std::uint32_t currentVariation{};
    std::uint64_t filesCompleted{};
    std::uint64_t filesTotal{};
    std::uint64_t currentBufferBytes{};
    std::uint64_t lastFileElapsedNs{};
    std::uint64_t peakFileElapsedNs{};
};

class RenderTelemetry final {
public:
    void beginJob(std::uint64_t filesTotal) noexcept;
    void setStage(RenderStage stage, std::uint32_t variation) noexcept;
    void recordBufferSamples(std::uint64_t sampleCount) noexcept;
    void recordFileCompleted(std::uint64_t elapsedNs) noexcept;
    void finish(bool succeeded) noexcept;
    RenderTelemetrySnapshot snapshot() const noexcept;

private:
    static void updatePeak(std::atomic<std::uint64_t>& peak,
                           std::uint64_t value) noexcept;

    std::atomic<bool> active_{false};
    std::atomic<bool> succeeded_{false};
    std::atomic<RenderStage> stage_{RenderStage::Idle};
    std::atomic<std::uint32_t> currentVariation_{0};
    std::atomic<std::uint64_t> filesCompleted_{0};
    std::atomic<std::uint64_t> filesTotal_{0};
    std::atomic<std::uint64_t> currentBufferBytes_{0};
    std::atomic<std::uint64_t> lastFileElapsedNs_{0};
    std::atomic<std::uint64_t> peakFileElapsedNs_{0};
};

} // namespace ventu::diagnostics
