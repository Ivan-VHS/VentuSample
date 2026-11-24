#include "audio_telemetry.hpp"

#include <cassert>
#include <cstdint>

int main() {
    ventu::diagnostics::AudioTelemetry telemetry;

    telemetry.recordCallback(2'000'000u, 10'000'000u, 256u);
    auto state = telemetry.snapshot();
    assert(state.callbacks == 1u);
    assert(state.samplePosition == 256u);
    assert(state.lastLoadPpm == 200'000u);
    assert(state.xruns == 0u);
    assert(state.quality == ventu::diagnostics::QualityMode::Normal);

    telemetry.recordCallback(9'500'000u, 10'000'000u, 256u);
    state = telemetry.snapshot();
    assert(state.quality == ventu::diagnostics::QualityMode::Reduced);

    telemetry.recordCallback(12'000'000u, 10'000'000u, 256u);
    telemetry.recordCallback(12'000'000u, 10'000'000u, 256u);
    telemetry.recordCallback(12'000'000u, 10'000'000u, 256u);
    state = telemetry.snapshot();
    assert(state.xruns == 3u);
    assert(state.consecutiveOverruns == 3u);
    assert(state.peakCallbackNs == 12'000'000u);

    telemetry.recordCallback(1'000'000u, 10'000'000u, 256u);
    state = telemetry.snapshot();
    assert(state.consecutiveOverruns == 0u);

    telemetry.recordMemory(900u, 1000u);
    assert(telemetry.snapshot().quality == ventu::diagnostics::QualityMode::Reduced);
    telemetry.recordMemory(1000u, 1000u);
    assert(telemetry.snapshot().quality == ventu::diagnostics::QualityMode::Emergency);
    telemetry.recordMemory(1u, 1000u);
    telemetry.recordDeviceHealth(false);
    assert(telemetry.snapshot().quality == ventu::diagnostics::QualityMode::Emergency);

    telemetry.recordDeviceHealth(true);
    telemetry.resetPeaks();
    state = telemetry.snapshot();
    assert(state.peakCallbackNs == state.lastCallbackNs);
    assert(state.peakLoadPpm == state.lastLoadPpm);

    ventu::diagnostics::RenderTelemetry render;
    render.beginJob(3u);
    auto renderState = render.snapshot();
    assert(renderState.active);
    assert(renderState.filesTotal == 3u);
    assert(renderState.filesCompleted == 0u);
    assert(renderState.stage == ventu::diagnostics::RenderStage::Preflight);

    render.setStage(ventu::diagnostics::RenderStage::Rendering, 2u);
    render.recordBufferSamples(48000u * 2u);
    render.recordFileCompleted(12'000'000u);
    render.recordFileCompleted(8'000'000u);
    renderState = render.snapshot();
    assert(renderState.currentVariation == 2u);
    assert(renderState.filesCompleted == 2u);
    assert(renderState.currentBufferBytes == 48000u * 2u * sizeof(float));
    assert(renderState.lastFileElapsedNs == 8'000'000u);
    assert(renderState.peakFileElapsedNs == 12'000'000u);

    render.finish(true);
    renderState = render.snapshot();
    assert(!renderState.active);
    assert(renderState.succeeded);
    assert(renderState.stage == ventu::diagnostics::RenderStage::Complete);

    render.beginJob(1u);
    render.finish(false);
    renderState = render.snapshot();
    assert(!renderState.active);
    assert(!renderState.succeeded);
    assert(renderState.stage == ventu::diagnostics::RenderStage::Failed);

    return 0;
}
