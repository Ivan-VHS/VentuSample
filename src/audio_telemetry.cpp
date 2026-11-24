#include "audio_telemetry.hpp"

#include <algorithm>
#include <limits>

namespace ventu::diagnostics {

void AudioTelemetry::updatePeak(std::atomic<std::uint64_t>& peak,
                                std::uint64_t value) noexcept {
    auto current = peak.load(std::memory_order_relaxed);
    while (current < value &&
           !peak.compare_exchange_weak(current, value,
                                       std::memory_order_relaxed,
                                       std::memory_order_relaxed)) {
    }
}

void AudioTelemetry::updatePeak(std::atomic<std::uint32_t>& peak,
                                std::uint32_t value) noexcept {
    auto current = peak.load(std::memory_order_relaxed);
    while (current < value &&
           !peak.compare_exchange_weak(current, value,
                                       std::memory_order_relaxed,
                                       std::memory_order_relaxed)) {
    }
}

void AudioTelemetry::recordCallback(std::uint64_t elapsedNs,
                                    std::uint64_t budgetNs,
                                    std::uint32_t frames,
                                    bool xrun) noexcept {
    callbacks_.fetch_add(1u, std::memory_order_relaxed);
    samplePosition_.fetch_add(frames, std::memory_order_relaxed);
    lastCallbackNs_.store(elapsedNs, std::memory_order_relaxed);
    updatePeak(peakCallbackNs_, elapsedNs);

    std::uint32_t loadPpm = 0u;
    const bool overBudget = xrun || budgetNs == 0u || elapsedNs > budgetNs;
    if (budgetNs != 0u) {
        const auto scaled = static_cast<long double>(elapsedNs) * 1000000.0L
                          / static_cast<long double>(budgetNs);
        loadPpm = static_cast<std::uint32_t>(std::min<long double>(
            scaled, static_cast<long double>(std::numeric_limits<std::uint32_t>::max())));
    }
    lastLoadPpm_.store(loadPpm, std::memory_order_relaxed);
    updatePeak(peakLoadPpm_, loadPpm);

    if (overBudget) {
        xruns_.fetch_add(1u, std::memory_order_relaxed);
        consecutiveOverruns_.fetch_add(1u, std::memory_order_relaxed);
    } else {
        consecutiveOverruns_.store(0u, std::memory_order_relaxed);
    }
}

void AudioTelemetry::recordMemory(std::uint64_t usedBytes,
                                  std::uint64_t budgetBytes) noexcept {
    memoryBytes_.store(usedBytes, std::memory_order_relaxed);
    memoryBudgetBytes_.store(budgetBytes, std::memory_order_relaxed);
}

void AudioTelemetry::recordDeviceHealth(bool healthy) noexcept {
    deviceHealthy_.store(healthy, std::memory_order_release);
}

AudioTelemetrySnapshot AudioTelemetry::snapshot() const noexcept {
    AudioTelemetrySnapshot result;
    result.callbacks = callbacks_.load(std::memory_order_relaxed);
    result.xruns = xruns_.load(std::memory_order_relaxed);
    result.consecutiveOverruns = consecutiveOverruns_.load(std::memory_order_relaxed);
    result.lastCallbackNs = lastCallbackNs_.load(std::memory_order_relaxed);
    result.peakCallbackNs = peakCallbackNs_.load(std::memory_order_relaxed);
    result.samplePosition = samplePosition_.load(std::memory_order_relaxed);
    result.memoryBytes = memoryBytes_.load(std::memory_order_relaxed);
    result.memoryBudgetBytes = memoryBudgetBytes_.load(std::memory_order_relaxed);
    result.lastLoadPpm = lastLoadPpm_.load(std::memory_order_relaxed);
    result.peakLoadPpm = peakLoadPpm_.load(std::memory_order_relaxed);

    const bool healthy = deviceHealthy_.load(std::memory_order_acquire);
    if (!healthy || (result.memoryBudgetBytes != 0u &&
                     result.memoryBytes >= result.memoryBudgetBytes)) {
        result.quality = QualityMode::Emergency;
    } else if ((result.memoryBudgetBytes != 0u &&
                result.memoryBytes >= result.memoryBudgetBytes -
                    result.memoryBudgetBytes / 10u) ||
               result.lastLoadPpm >= 900000u ||
               result.consecutiveOverruns >= 3u) {
        result.quality = QualityMode::Reduced;
    } else {
        result.quality = QualityMode::Normal;
    }
    return result;
}

void AudioTelemetry::resetPeaks() noexcept {
    peakCallbackNs_.store(lastCallbackNs_.load(std::memory_order_relaxed),
                          std::memory_order_relaxed);
    peakLoadPpm_.store(lastLoadPpm_.load(std::memory_order_relaxed),
                       std::memory_order_relaxed);
}

} // namespace ventu::diagnostics
