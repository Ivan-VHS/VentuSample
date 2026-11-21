#include "audio_suite.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <numbers>
#include <string_view>
#include <utility>

namespace ventu {
namespace {
constexpr std::size_t kMaxFrames = 20'000'000u;
constexpr std::uintmax_t kMaxWavBytes = 128u * 1024u * 1024u;
constexpr float kPi = std::numbers::pi_v<float>;

const std::vector<AudioPluginDescriptor> kCatalog{
    {"aetherfold","AETHERFOLD","Space & Acoustics","Algorithmic room reflections with material-like damping and controllable decay.","3D room mesh / reflection rays"},
    {"monolith-diffractor","MONOLITH DIFFRACTOR","Space & Acoustics","All-pass and comb diffusion that creates broad, obstacle-like acoustic scattering.","Brutalist wave frame"},
    {"sub-tectonic","SUB-TECTONIC","Space & Acoustics","Low-frequency transmission model with damping, resonance and harmonic weight.","Geological cross-section"},
    {"echo-carton","ECHO-CARTON","Space & Acoustics","Short multi-tap reflections with softened resonances and box-like coloration.","Isometric boxes and tubes"},
    {"pendulum-hall","PENDULUM HALL","Space & Acoustics","Modulated delay and reverberant motion driven by a pendulum-shaped LFO.","Chrome pendulum / radar"},
    {"void-chamber","VOID-CHAMBER","Space & Acoustics","Creates a separated dry foreground and sparse, distant delay tail.","Dark sphere / diffuse halo"},
    {"horizon-panner","HORIZON PANNER","Space & Acoustics","Places stereo energy along a moving azimuth and elevation-inspired panorama.","Panoramic horizon"},
    {"resonance-loom","RESONANCE LOOM","Space & Acoustics","A bank of tuned resonators excites partials from the incoming signal.","Resonant string matrix"},
    {"cave-breath","CAVE-BREATH","Space & Acoustics","Envelope-controlled feedback and damping make the reverb field expand and contract.","Breathing rock cavity"},
    {"glass-prism","GLASS-PRISM","Space & Acoustics","Splits low, middle and high bands and distributes them across the stereo field.","Spectral glass prism"},
    {"ocean-bathymetry","OCEAN-BATHYMETRY","Space & Acoustics","Depth-inspired low-pass damping, slow modulation and submerged reflections.","Layered sonar display"},

    {"chrono-grain","CHRONO-GRAIN","Granular Time","Reorders short audio grains using deterministic reverse and repeat windows.","Time ribbon / grain cloud"},
    {"fractal-dust","FRACTAL DUST","Granular Time","Builds decaying, multi-scale echoes from transient material.","Fractal branching tree"},
    {"tape-cancer","TAPE-CANCER","Granular Time","Tape-style bandwidth loss, wow, flutter, saturation and occasional dropouts.","Worn reel transport"},
    {"stutter-weaver","STUTTER-WEAVER","Granular Time","Repeats micro-slices in sync-like blocks with adjustable rhythmic density.","Interwoven step grid"},
    {"silence-harvester","SILENCE-HARVESTER","Granular Time","Gates and recycles low-level gaps into a quiet delayed texture.","Inverted waveform"},
    {"freeze-crystal","FREEZE-CRYSTAL","Granular Time","Sample-and-hold grains sustain short moments as frozen spectral-like fragments.","Fracturing crystal"},
    {"drift-slicer","DRIFT-SLICER","Granular Time","Alternates short offsets and grain boundaries for shifting micro-fragments.","Drifting tile field"},
    {"hourglass-delay","HOURGLASS DELAY","Granular Time","A modulated delay whose read position accelerates and decelerates over time.","Animated hourglass"},
    {"micro-glitch-foundry","MICRO-GLITCH FOUNDRY","Granular Time","Deterministic bit reduction, sample holds and controlled buffer-style glitches.","Terminal bit display"},
    {"orbit-cloud","ORBIT-CLOUD","Granular Time","Layers modulated delay taps into a moving, orbit-like granular cloud.","Particle orbit system"},
    {"phantom-loop","PHANTOM LOOP","Granular Time","Reveals a delayed loop as the source envelope falls, creating a gap-triggered response.","Emergent loop outline"},

    {"morph-chimera","MORPH-CHIMERA","Timbre & Synthesis","Crossfades filtered and nonlinear signal paths to create a hybrid timbre.","Dual-form morph cube"},
    {"vocal-sculptor","VOCAL-SCULPTOR","Timbre & Synthesis","Creates formant-like emphasis using tuned band-pass resonances.","Vocal tract / formants"},
    {"brass-forge","BRASS-FORGE","Timbre & Synthesis","Asymmetric saturation and resonant filtering add brass-like upper harmonics.","Heated brass bell"},
    {"sub-gravity","SUB-GRAVITY","Timbre & Synthesis","Reinforces low-frequency energy with controlled rectification and harmonic shaping.","Sub pressure indicator"},
    {"harmonic-weaver","HARMONIC-WEAVER","Timbre & Synthesis","Rebalances harmonic content with tone-dependent nonlinear shaping.","Harmonic ladder"},
    {"noise-alchemist","NOISE-ALCHEMIST","Timbre & Synthesis","Adds deterministic noise through a tone-shaped resonant path.","Noise condenser vessel"},
    {"chord-architect","CHORD-ARCHITECT","Timbre & Synthesis","Adds octave and fifth-oriented harmonic components to monophonic material.","Voicing staircase"},
    {"texture-loom","TEXTURE-LOOM","Timbre & Synthesis","Combines filtered noise, resonance and delayed detail into a texture layer.","Woven texture strands"},
    {"formant-shift-ultra","FORMANT-SHIFT ULTRA","Timbre & Synthesis","Moves resonant emphasis independently from the source's fundamental content.","Pitch / formant controls"},
    {"resonator-chorus","RESONATOR-CHORUS","Timbre & Synthesis","Three detuned resonant paths create a metallic, glass-like chorus.","Tuned resonator tubes"},
    {"synth-skin","SYNTH-SKIN","Timbre & Synthesis","Applies short comb coloration and damping inspired by material resonance.","Material surface panel"},

    {"transient-hammer","TRANSIENT-HAMMER","Dynamics & Punch","Separates fast attacks from the smoothed body and scales the transient component.","Attack impulse hammer"},
    {"swing-engine","SWING-ENGINE","Dynamics & Punch","Applies tempo-like periodic gain displacement for a swung rhythmic feel.","Offset rhythm grid"},
    {"tape-saturator-pro","TAPE-SATURATOR PRO","Dynamics & Punch","Three-band-inspired saturation with independent low, mid and high coloration.","Three saturation bands"},
    {"glue-bus-titan","GLUE-BUS TITAN","Dynamics & Punch","Linked-stereo envelope compression with makeup and controlled soft knee.","Bus compression bridge"},
    {"pump-sidechain-x","PUMP-SIDECHAIN X","Dynamics & Punch","Envelope-following ducking creates an internal sidechain-like pumping effect.","Ducking envelope"},
    {"clip-diamond","CLIP-DIAMOND","Dynamics & Punch","Selectable soft and hard peak clipping with output safety bounds.","Peak-cut diamond"},
    {"dynamics-sculptor","DYNAMICS-SCULPTOR","Dynamics & Punch","Envelope-dependent compression and expansion shapes macro-dynamics.","Dynamic envelope curve"},
    {"low-end-lock","LOW-END LOCK","Dynamics & Punch","Centers low frequencies and narrows sub content for mono compatibility.","Phase compass / mono meter"},
    {"crush-analog","CRUSH-ANALOG","Dynamics & Punch","Reduces bit depth and sample detail with a softened reconstruction stage.","Stepped signal display"},
    {"headroom-guard","HEADROOM-GUARD","Dynamics & Punch","A bounded peak guard prevents internal sample values from exceeding the ceiling.","Headroom barrier"},
    {"velocity-bloom","VELOCITY-BLOOM","Dynamics & Punch","Expands envelope contrast to reveal softer details and note movement.","Expanding bloom"},

    {"spectral-carver","SPECTRAL-CARVER","Frequency & Phase","Uses tone-focused dynamic band attenuation to create room around a competing range.","Overlaid spectral curves"},
    {"phase-aligner","PHASE-ALIGNER","Frequency & Phase","Applies a controllable all-pass phase rotation and stereo phase correction.","Aligned wave traces"},
    {"resonance-surgeon","RESONANCE-SURGEON","Frequency & Phase","Detects narrow-band energy and attenuates resonant peaks with a tuned notch.","Resonance needles"},
    {"air-sculptor","AIR-SCULPTOR","Frequency & Phase","Adds a controlled high-frequency shelf and harmonic lift.","High-frequency air arc"},
    {"sub-harmonic-generator","SUB-HARMONIC GENERATOR","Frequency & Phase","Adds a low-frequency octave-like component derived from the input waveform.","Sub-frequency ladder"},
    {"dynamic-notch-matrix","DYNAMIC-NOTCH MATRIX","Frequency & Phase","A tone-centered notch responds more strongly as the selected band grows.","Frequency-time matrix"},
    {"eq-morpher","EQ-MORPHER","Frequency & Phase","Morphs between low-pass and high-pass tonal curves using a continuous control.","Dual EQ curves"},
    {"de-harsh-pro","DE-HARSH PRO","Frequency & Phase","Reduces upper-mid harshness with a dynamically weighted band cut.","Upper-mid suppression"},
    {"mid-side-architect","MID-SIDE ARCHITECT","Frequency & Phase","Controls mid/side width while retaining centered low-frequency content.","MID stem / SIDE wings"},
    {"filter-cascade","FILTER-CASCADE","Frequency & Phase","Runs a four-stage low/high-pass and resonant filter cascade.","Four filter modules"},
    {"spectrum-freeze","SPECTRUM-FREEZE","Frequency & Phase","Holds smoothed low/high components to create a static spectral impression.","Frozen spectrum trace"},

    {"groove-mutator","GROOVE-MUTATOR","Generative & Rhythm","Creates deterministic rhythmic gating and amplitude mutations within a stable grid.","Mutating step sequencer"},
    {"poly-rhythm-engine","POLY-RHYTHM ENGINE","Generative & Rhythm","Combines independent 3:2, 5:4 and 7:8 modulation cycles.","Concentric rhythm rings"},
    {"chaos-modulator","CHAOS-MODULATOR","Generative & Rhythm","Uses a bounded deterministic nonlinear attractor to move gain and stereo position.","Lorenz-like orbit"},
    {"reverse-architect","REVERSE-ARCHITECT","Generative & Rhythm","Reverses short segments while retaining the surrounding phrase order.","Reverse build waveform"},
    {"drone-weaver","DRONE-WEAVER","Generative & Rhythm","Turns source material into a sustained, filtered feedback texture.","Layered drone field"},
    {"pitch-drifter","PITCH-DRIFTER","Generative & Rhythm","Adds slow, controlled modulation through a variable short delay.","Pitch drift trace"},
    {"beat-deconstructor","BEAT-DECONSTRUCTOR","Generative & Rhythm","Balances transient, body and tail components using envelope separation.","Transient / body / tail lanes"},
    {"echo-matrix","ECHO-MATRIX","Generative & Rhythm","Combines multiple delay taps with stereo cross-feedback.","Delay node matrix"},
    {"lo-fi-time-capsule","LO-FI TIME CAPSULE","Generative & Rhythm","Combines bandwidth limiting, quantization and deterministic noise coloration.","Era selector display"},
    {"harmonic-drift","HARMONIC-DRIFT","Generative & Rhythm","Slowly modulates harmonic emphasis to create rising or falling tonal tension.","Moving harmonic rays"},
    {"sonic-catalyst","SONIC-CATALYST","Generative & Rhythm","A restrained mix enhancer combining soft saturation, presence and stereo detail.","Energy core"}
};

float clampAudio(float value, float limit = 8.0f) {
    if (!std::isfinite(value)) return 0.0f;
    return std::clamp(value, -limit, limit);
}
float onePole(float input, float& state, float coefficient) {
    state += coefficient * (input - state);
    return state;
}
std::uint32_t nextRandom(std::uint32_t& state) {
    state = state * 1664525u + 1013904223u;
    return state;
}
float randomUnit(std::uint32_t& state) {
    return static_cast<float>((nextRandom(state) >> 8u) & 0x00ffffffu) / 16777215.0f;
}
std::size_t delayIndex(std::size_t write, std::size_t delay, std::size_t size) {
    return (write + size - std::min(delay, size - 1u)) % size;
}
std::uint16_t read16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset])
        | static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[offset + 1u]) << 8u);
}
std::uint32_t read32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset])
        | (static_cast<std::uint32_t>(bytes[offset + 1u]) << 8u)
        | (static_cast<std::uint32_t>(bytes[offset + 2u]) << 16u)
        | (static_cast<std::uint32_t>(bytes[offset + 3u]) << 24u);
}
void write16(std::ostream& out, std::uint16_t value) {
    out.put(static_cast<char>(value & 0xffu));
    out.put(static_cast<char>((value >> 8u) & 0xffu));
}
void write32(std::ostream& out, std::uint32_t value) {
    for (unsigned shift = 0; shift < 32u; shift += 8u)
        out.put(static_cast<char>((value >> shift) & 0xffu));
}
}

bool AudioSuiteBuffer::valid() const noexcept {
    if (sampleRate < 8000 || sampleRate > 384000 || left.empty() || left.size() != right.size()
        || left.size() > kMaxFrames) return false;
    return std::all_of(left.begin(), left.end(), [](float v) { return std::isfinite(v); })
        && std::all_of(right.begin(), right.end(), [](float v) { return std::isfinite(v); });
}

const std::vector<AudioPluginDescriptor>& audioPluginCatalog() {
    return kCatalog;
}

const AudioPluginDescriptor* findAudioPlugin(const std::string& idOrName) noexcept {
    const auto found = std::find_if(kCatalog.begin(), kCatalog.end(), [&](const auto& item) {
        return item.id == idOrName || item.name == idOrName;
    });
    return found == kCatalog.end() ? nullptr : &*found;
}

bool processAudioPlugin(AudioSuiteBuffer& buffer, const std::string& idOrName,
                        const AudioPluginParameters& p, std::string& error) {
    const auto* descriptor = findAudioPlugin(idOrName);
    if (!descriptor) { error = "Unknown audio plugin: " + idOrName; return false; }
    if (!buffer.valid()) { error = "Audio buffer is invalid or exceeds the 20-million-frame limit."; return false; }
    const std::array<float, 5> values{p.amount, p.wet, p.rate, p.tone, p.width};
    if (!std::all_of(values.begin(), values.end(), [](float v) {
        return std::isfinite(v) && v >= 0.0f && v <= 1.0f;
    })) { error = "Audio plugin parameters must be finite values in the 0..1 range."; return false; }

    const auto index = static_cast<std::size_t>(std::distance(kCatalog.data(), descriptor));
    const auto dryL = buffer.left;
    const auto dryR = buffer.right;
    const std::size_t frames = buffer.frames();
    const float sr = static_cast<float>(buffer.sampleRate);
    const float amount = p.amount;
    const float wet = p.wet;
    const float rate = p.rate;
    const float tone = p.tone;
    const float width = p.width;
    const float cutoff = 80.0f + tone * 17000.0f;
    const float alpha = 1.0f - std::exp(-2.0f * kPi * std::min(cutoff, sr * 0.45f) / sr);
    const std::size_t maxDelay = std::max<std::size_t>(64u,
        std::min<std::size_t>(static_cast<std::size_t>(sr * 1.25f), 480000u));
    const std::size_t delaySize = maxDelay + 1u;
    const std::size_t baseDelay = std::clamp<std::size_t>(
        static_cast<std::size_t>(sr * (0.018f + rate * 0.58f)), 1u, maxDelay - 2u);
    const std::size_t shortDelay = std::max<std::size_t>(1u, baseDelay / 3u);
    const std::size_t longDelay = std::min<std::size_t>(maxDelay - 1u, baseDelay * 2u);
    const std::size_t grain = std::clamp<std::size_t>(
        static_cast<std::size_t>(sr * (0.008f + rate * 0.09f)), 32u, 8192u);
    const float feedback = 0.12f + amount * 0.62f;
    std::vector<float> delayL(delaySize, 0.0f), delayR(delaySize, 0.0f);
    float lowL = 0.0f, lowR = 0.0f, low2L = 0.0f, low2R = 0.0f;
    float envL = 0.0f, envR = 0.0f, previousL = 0.0f, previousR = 0.0f;
    float resonatorL1 = 0.0f, resonatorL2 = 0.0f, resonatorR1 = 0.0f, resonatorR2 = 0.0f;
    float chaosX = 0.11f, chaosY = 0.07f, chaosZ = 0.13f;
    std::uint32_t randomState = 0x9e3779b9u ^ static_cast<std::uint32_t>(index * 7919u);
    std::size_t write = 0;
    const float resonantHz = 120.0f + tone * 7000.0f;
    const float resonanceA = 2.0f * std::cos(2.0f * kPi * std::min(resonantHz, sr * 0.4f) / sr);
    const float resonanceR = 0.88f + amount * 0.105f;

    for (std::size_t i = 0; i < frames; ++i) {
        const float xL = dryL[i];
        const float xR = dryR[i];
        const float phase = 2.0f * kPi * (0.04f + rate * 7.0f) * static_cast<float>(i) / sr;
        const float lfo = std::sin(phase);
        const float mod = 0.5f + 0.5f * lfo;
        const float lowInL = onePole(xL, lowL, alpha);
        const float lowInR = onePole(xR, lowR, alpha);
        const float highL = xL - lowInL;
        const float highR = xR - lowInR;
        const float lowerL = onePole(xL, low2L, std::min(0.99f, alpha * 0.22f));
        const float lowerR = onePole(xR, low2R, std::min(0.99f, alpha * 0.22f));
        const float envelopeInput = 0.5f * (std::abs(xL) + std::abs(xR));
        const float envCoeff = envelopeInput > 0.5f * (envL + envR) ? 0.08f : 0.003f;
        envL += envCoeff * (std::abs(xL) - envL);
        envR += envCoeff * (std::abs(xR) - envR);
        const float env = 0.5f * (envL + envR);
        const std::size_t movingDelay = std::clamp<std::size_t>(
            static_cast<std::size_t>(static_cast<float>(baseDelay) * (0.72f + 0.56f * mod)),
            1u, maxDelay - 1u);
        const float d1L = delayL[delayIndex(write, baseDelay, delaySize)];
        const float d1R = delayR[delayIndex(write, baseDelay, delaySize)];
        const float d2L = delayL[delayIndex(write, shortDelay, delaySize)];
        const float d2R = delayR[delayIndex(write, shortDelay, delaySize)];
        const float d3L = delayL[delayIndex(write, longDelay, delaySize)];
        const float d3R = delayR[delayIndex(write, longDelay, delaySize)];
        const float dmL = delayL[delayIndex(write, movingDelay, delaySize)];
        const float dmR = delayR[delayIndex(write, movingDelay, delaySize)];
        const float resonL = xL + resonanceA * resonanceR * resonatorL1
            - resonanceR * resonanceR * resonatorL2;
        const float resonR = xR + resonanceA * resonanceR * resonatorR1
            - resonanceR * resonanceR * resonatorR2;
        resonatorL2 = resonatorL1; resonatorL1 = clampAudio(resonL * 0.22f);
        resonatorR2 = resonatorR1; resonatorR1 = clampAudio(resonR * 0.22f);
        float yL = xL, yR = xR;
        const float random = randomUnit(randomState);
        const std::size_t blockStart = (i / grain) * grain;
        const std::size_t inBlock = i - blockStart;
        const std::size_t reverseAt = std::min(frames - 1u,
            blockStart + std::min(grain - 1u - inBlock, frames - 1u - blockStart));

        switch (index) {
        case 0: yL = 0.36f*xL + 0.30f*d1L + 0.20f*d2L + 0.14f*d3L;
                yR = 0.36f*xR + 0.30f*d1R + 0.20f*d2R + 0.14f*d3R; break;
        case 1: yL = d1L - 0.68f*xL + 0.68f*lowInL; yR = d1R - 0.68f*xR + 0.68f*lowInR; break;
        case 2: yL = lowerL*1.2f + lowInL*0.45f + std::tanh(highL*1.8f)*0.18f;
                yR = lowerR*1.2f + lowInR*0.45f + std::tanh(highR*1.8f)*0.18f; break;
        case 3: yL = xL + 0.32f*d2L + 0.22f*d1L + 0.12f*d3L;
                yR = xR + 0.32f*d2R + 0.22f*d1R + 0.12f*d3R; break;
        case 4: yL = xL*0.5f + dmL*0.8f; yR = xR*0.5f + dmR*0.8f; break;
        case 5: yL = 0.78f*xL + 0.52f*d3L; yR = 0.78f*xR + 0.52f*d3R; break;
        case 6: { const float pan = std::sin(phase) * width;
                  yL = xL * std::sqrt(0.5f * (1.0f - pan));
                  yR = xR * std::sqrt(0.5f * (1.0f + pan)); break; }
        case 7: yL = resonL*0.72f; yR = resonR*0.72f; break;
        case 8: yL = xL*0.34f + d1L*(0.30f + 0.35f*mod) + d3L*0.18f;
                yR = xR*0.34f + d1R*(0.30f + 0.35f*mod) + d3R*0.18f; break;
        case 9: { const float midL = lowInL - lowerL, midR = lowInR - lowerR;
                  const float pan = width * 0.65f;
                  yL = lowerL + midL*(1.0f-pan) + highL*(0.5f-pan);
                  yR = lowerR + midR*(1.0f+pan) + highR*(0.5f+pan); break; }
        case 10: yL = lowInL*0.75f + d1L*0.35f + highL*0.08f;
                 yR = lowInR*0.75f + d1R*0.35f + highR*0.08f; break;

        case 11: yL = dryL[reverseAt]; yR = dryR[reverseAt]; break;
        case 12: yL = xL*0.55f + d1L*0.25f + d2L*0.18f + d3L*0.12f;
                 yR = xR*0.55f + d1R*0.25f + d2R*0.18f + d3R*0.12f; break;
        case 13: { const float wow = 1.0f + 0.012f*lfo;
                   const float dropout = random < amount*0.025f ? 0.18f : 1.0f;
                   yL = std::tanh(xL* (1.0f+amount*2.0f))*wow*dropout*0.88f + d1L*0.05f;
                   yR = std::tanh(xR* (1.0f+amount*2.0f))*wow*dropout*0.88f + d1R*0.05f; break; }
        case 14: { const std::size_t repeat = std::max<std::size_t>(16u, grain/4u);
                   const std::size_t at = blockStart + (inBlock % repeat);
                   const auto safe = std::min(at, frames-1u); yL = dryL[safe]; yR = dryR[safe]; break; }
        case 15: { const float gate = env < (0.04f + amount*0.20f) ? 1.0f : 0.0f;
                   yL = gate*(d1L*0.65f + d3L*0.35f); yR = gate*(d1R*0.65f + d3R*0.35f); break; }
        case 16: { const std::size_t hold = std::max<std::size_t>(1u, grain/3u);
                   const auto at = i - (i % hold); yL = dryL[at]; yR = dryR[at]; break; }
        case 17: yL = (i & 1u) ? d1L : dmL; yR = (i & 1u) ? d1R : dmR; break;
        case 18: yL = xL*0.62f + dmL*0.70f; yR = xR*0.62f + dmR*0.70f; break;
        case 19: { const float quant = 2.0f + (1.0f-amount)*62.0f;
                   yL = std::round(xL*quant)/quant; yR = std::round(xR*quant)/quant;
                   if (random < amount*0.008f) { yL = d1L; yR = d1R; } break; }
        case 20: yL = xL*0.40f + d1L*0.22f + dmL*0.25f + d3R*0.12f;
                 yR = xR*0.40f + d1R*0.22f + dmR*0.25f + d3L*0.12f; break;
        case 21: { const float appear = std::clamp(1.0f-env*3.0f, 0.0f, 1.0f);
                   yL = xL*0.18f + d1L*appear; yR = xR*0.18f + d1R*appear; break; }

        case 22: yL = (1.0f-amount)*lowInL + amount*std::tanh(xL*2.8f);
                 yR = (1.0f-amount)*lowInR + amount*std::tanh(xR*2.8f); break;
        case 23: yL = (lowInL-lowerL)*3.2f + (resonL*0.10f);
                 yR = (lowInR-lowerR)*3.2f + (resonR*0.10f); break;
        case 24: yL = std::tanh(xL*(1.0f+amount*4.0f) + 0.12f*amount);
                 yR = std::tanh(xR*(1.0f+amount*4.0f) + 0.12f*amount); break;
        case 25: yL = lowInL*1.15f + std::abs(lowerL)*amount*0.35f;
                 yR = lowInR*1.15f + std::abs(lowerR)*amount*0.35f; break;
        case 26: yL = std::tanh(xL*(1.0f+amount*3.4f)) + highL*0.12f;
                 yR = std::tanh(xR*(1.0f+amount*3.4f)) + highR*0.12f; break;
        case 27: { const float noise = (random*2.0f-1.0f)*amount*0.12f;
                   yL = lowInL + noise; yR = lowInR - noise; break; }
        case 28: yL = xL + std::abs(xL)*amount*0.22f + std::tanh(xL*2.0f)*0.12f;
                 yR = xR + std::abs(xR)*amount*0.22f + std::tanh(xR*2.0f)*0.12f; break;
        case 29: yL = lowInL*0.32f + (lowInL-lowerL)*1.8f + d1L*0.16f + (random-0.5f)*amount*0.06f;
                 yR = lowInR*0.32f + (lowInR-lowerR)*1.8f + d1R*0.16f - (random-0.5f)*amount*0.06f; break;
        case 30: yL = (lowInL-lowerL)* (2.0f+tone*4.0f) + resonL*0.06f;
                 yR = (lowInR-lowerR)* (2.0f+tone*4.0f) + resonR*0.06f; break;
        case 31: yL = (resonL + (lowInL-lowerL)*2.2f + d2L*0.16f) / 1.4f;
                 yR = (resonR + (lowInR-lowerR)*2.2f + d2R*0.16f) / 1.4f; break;
        case 32: yL = xL*0.6f + (xL-d1L)*0.22f + d2L*0.12f;
                 yR = xR*0.6f + (xR-d1R)*0.22f + d2R*0.12f; break;

        case 33: yL = xL + (xL-previousL)*amount*2.0f;
                 yR = xR + (xR-previousR)*amount*2.0f; break;
        case 34: { const float groove = 1.0f - amount*0.28f*(0.5f+0.5f*std::sin(phase*0.5f));
                   yL = xL*groove; yR = xR*groove; break; }
        case 35: yL = std::tanh(lowInL*(1.0f+amount*2.0f))*0.55f
                          + std::tanh((lowInL-lowerL)*(1.0f+amount*3.0f))*0.35f
                          + std::tanh(highL*(1.0f+amount*1.6f))*0.18f;
                 yR = std::tanh(lowInR*(1.0f+amount*2.0f))*0.55f
                          + std::tanh((lowInR-lowerR)*(1.0f+amount*3.0f))*0.35f
                          + std::tanh(highR*(1.0f+amount*1.6f))*0.18f; break;
        case 36: { const float gain = 1.0f / (1.0f + std::max(0.0f, env-0.16f)*amount*7.0f);
                   yL = xL*gain; yR = xR*gain; break; }
        case 37: { const float duck = 1.0f - std::clamp(env*amount*3.0f, 0.0f, 0.85f);
                   yL = xL*duck; yR = xR*duck; break; }
        case 38: { const float drive = 1.0f + amount*8.0f;
                   yL = amount > 0.55f ? std::clamp(xL*drive, -0.85f, 0.85f) : std::tanh(xL*drive);
                   yR = amount > 0.55f ? std::clamp(xR*drive, -0.85f, 0.85f) : std::tanh(xR*drive); break; }
        case 39: { const float gain = env > 0.08f ? 1.0f/(1.0f+amount*env*4.0f) : 1.0f+amount*0.5f;
                   yL = xL*gain; yR = xR*gain; break; }
        case 40: { const float mid = 0.5f*(lowInL+lowInR);
                   const float sideL = xL-lowInL, sideR = xR-lowInR;
                   yL = mid + lowInL*0.25f + sideL*(0.35f+width*0.9f);
                   yR = mid + lowInR*0.25f + sideR*(0.35f+width*0.9f); break; }
        case 41: { const float steps = 8.0f + (1.0f-amount)*120.0f;
                   yL = std::round(xL*steps)/steps; yR = std::round(xR*steps)/steps; break; }
        case 42: yL = std::clamp(xL, -0.92f, 0.92f); yR = std::clamp(xR, -0.92f, 0.92f); break;
        case 43: { const float expansion = 1.0f + amount*std::clamp(0.35f-env, 0.0f, 0.35f)*3.0f;
                   yL = xL*expansion; yR = xR*expansion; break; }

        case 44: { const float bandL = lowInL-lowerL, bandR = lowInR-lowerR;
                   const float cut = std::clamp(env*amount*3.0f, 0.0f, 0.92f);
                   yL = xL-bandL*cut; yR = xR-bandR*cut; break; }
        case 45: yL = d1L*0.18f + xL*0.82f - lowInL*0.12f;
                 yR = d1R*0.18f + xR*0.82f - lowInR*0.12f; break;
        case 46: { const float bandL = lowInL-lowerL, bandR = lowInR-lowerR;
                   yL = xL-bandL*(0.3f+amount*1.4f); yR = xR-bandR*(0.3f+amount*1.4f); break; }
        case 47: yL = lowInL + highL*(1.0f+amount*2.5f);
                 yR = lowInR + highR*(1.0f+amount*2.5f); break;
        case 48: yL = lowInL + std::abs(xL)*amount*0.42f;
                 yR = lowInR + std::abs(xR)*amount*0.42f; break;
        case 49: { const float bandL = lowInL-lowerL, bandR = lowInR-lowerR;
                   const float cut = std::clamp((std::abs(bandL)+std::abs(bandR))*amount*4.0f, 0.0f, 0.9f);
                   yL = xL-bandL*cut; yR = xR-bandR*cut; break; }
        case 50: { const float morph = rate;
                   yL = lowInL*(1.0f-morph) + (xL-lowInL)*morph;
                   yR = lowInR*(1.0f-morph) + (xR-lowInR)*morph; break; }
        case 51: { const float bandL = lowInL-lowerL, bandR = lowInR-lowerR;
                   yL = xL-bandL*(0.35f+amount*1.7f); yR = xR-bandR*(0.35f+amount*1.7f); break; }
        case 52: { const float mid = 0.5f*(xL+xR), side = 0.5f*(xL-xR)*(0.2f+width*1.8f);
                   yL = mid+side; yR = mid-side; break; }
        case 53: yL = onePole(lowInL, low2L, alpha*0.7f) + highL*(0.2f+amount*0.3f);
                 yR = onePole(lowInR, low2R, alpha*0.7f) + highR*(0.2f+amount*0.3f); break;
        case 54: { const std::size_t hold = std::max<std::size_t>(1u, grain);
                   const auto at = i-(i%hold);
                   yL = onePole(dryL[at], low2L, 0.65f); yR = onePole(dryR[at], low2R, 0.65f); break; }

        case 55: { const float step = std::sin(phase*2.0f + std::floor(static_cast<float>(i)/grain)*0.7f);
                   const float gate = (step > -0.35f) ? 1.0f : (1.0f-amount*0.85f);
                   yL = xL*gate; yR = xR*gate; break; }
        case 56: { const float poly = (std::sin(phase*1.5f)+std::sin(phase*2.5f)+std::sin(phase*3.5f))/3.0f;
                   const float gain = 1.0f-amount*0.32f+amount*0.32f*poly;
                   yL = xL*gain; yR = xR*gain; break; }
        case 57: { const float dx = 10.0f*(chaosY-chaosX), dy = chaosX*(28.0f-chaosZ)-chaosY,
                          dz = chaosX*chaosY-(8.0f/3.0f)*chaosZ;
                   chaosX += dx*0.00015f; chaosY += dy*0.00015f; chaosZ += dz*0.00015f;
                   const float chaos = std::clamp(chaosX*0.08f, -1.0f, 1.0f);
                   yL = xL*(1.0f+chaos*amount*0.45f); yR = xR*(1.0f-chaos*amount*0.45f); break; }
        case 58: yL = dryL[reverseAt]*0.75f + d1L*0.12f; yR = dryR[reverseAt]*0.75f + d1R*0.12f; break;
        case 59: yL = lowInL*0.62f + d1L*(0.35f+amount*0.22f);
                 yR = lowInR*0.62f + d1R*(0.35f+amount*0.22f); break;
        case 60: yL = xL*0.72f + dmL*0.28f; yR = xR*0.72f + dmR*0.28f; break;
        case 61: { const float transientL = xL-previousL, transientR = xR-previousR;
                   const float bodyL = lowInL, bodyR = lowInR;
                   const float tailL = d1L*0.25f, tailR = d1R*0.25f;
                   yL = transientL*(0.5f+amount) + bodyL*0.8f + tailL*amount;
                   yR = transientR*(0.5f+amount) + bodyR*0.8f + tailR*amount; break; }
        case 62: yL = xL*0.35f + d1L*0.24f + d2L*0.20f + d3R*0.20f;
                 yR = xR*0.35f + d1R*0.24f + d2R*0.20f + d3L*0.20f; break;
        case 63: { const float steps = 10.0f+(1.0f-amount)*96.0f;
                   yL = std::tanh(std::round(lowInL*steps)/steps*1.4f) + highL*0.12f;
                   yR = std::tanh(std::round(lowInR*steps)/steps*1.4f) + highR*0.12f; break; }
        case 64: { const float harmonic = std::sin(phase*2.0f)*amount*0.12f;
                   yL = xL + harmonic*xL + std::tanh(xL*1.8f)*0.12f;
                   yR = xR + harmonic*xR + std::tanh(xR*1.8f)*0.12f; break; }
        case 65: yL = std::tanh(xL*(1.0f+amount*0.65f))*0.82f + highL*0.12f;
                 yR = std::tanh(xR*(1.0f+amount*0.65f))*0.82f + highR*0.12f; break;
        default: break;
        }

        const float mid = 0.5f*(yL+yR);
        const float side = 0.5f*(yL-yR)*(0.5f+width);
        yL = mid+side; yR = mid-side;
        buffer.left[i] = clampAudio(xL*(1.0f-wet) + yL*wet);
        buffer.right[i] = clampAudio(xR*(1.0f-wet) + yR*wet);
        previousL = xL; previousR = xR;
        const float writeL = clampAudio(xL + d1L*feedback, 4.0f);
        const float writeR = clampAudio(xR + d1R*feedback, 4.0f);
        delayL[write] = writeL; delayR[write] = writeR;
        write = (write+1u) % delaySize;
    }
    error.clear();
    return true;
}

bool processAudioChain(AudioSuiteBuffer& buffer, const std::vector<AudioPluginChainItem>& chain,
                       std::string& error) {
    if (chain.empty()) { error = "The effect rack is empty."; return false; }
    AudioSuiteBuffer working = buffer;
    for (const auto& item : chain) {
        if (!processAudioPlugin(working, item.pluginId, item.parameters, error)) {
            error = item.pluginId + ": " + error;
            return false;
        }
    }
    buffer = std::move(working);
    error.clear();
    return true;
}

bool loadPcm16Wav(const std::filesystem::path& path, AudioSuiteBuffer& buffer, std::string& error) {
    std::error_code ec;
    const auto fileBytes = std::filesystem::file_size(path, ec);
    if (ec || fileBytes < 44u || fileBytes > kMaxWavBytes) {
        error = "WAV file is missing, too small, or exceeds the 128 MiB import limit.";
        return false;
    }
    std::ifstream input(path, std::ios::binary);
    if (!input) { error = "Cannot open the WAV file."; return false; }
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(fileBytes));
    input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!input || bytes.size() < 44u
        || std::string(reinterpret_cast<const char*>(bytes.data()), 4u) != "RIFF"
        || std::string(reinterpret_cast<const char*>(bytes.data()+8u), 4u) != "WAVE") {
        error = "Invalid RIFF/WAVE container.";
        return false;
    }
    std::uint16_t format = 0, channels = 0, bits = 0, blockAlign = 0;
    std::uint32_t sampleRate = 0, dataOffset = 0, dataSize = 0;
    std::size_t offset = 12u;
    while (offset + 8u <= bytes.size()) {
        const std::string chunk(reinterpret_cast<const char*>(bytes.data()+offset), 4u);
        const std::uint32_t chunkSize = read32(bytes, offset+4u);
        const std::size_t begin = offset+8u;
        if (static_cast<std::uint64_t>(begin)+chunkSize > bytes.size()) break;
        if (chunk == "fmt " && chunkSize >= 16u) {
            format = read16(bytes, begin);
            channels = read16(bytes, begin+2u);
            sampleRate = read32(bytes, begin+4u);
            blockAlign = read16(bytes, begin+12u);
            bits = read16(bytes, begin+14u);
        } else if (chunk == "data") {
            dataOffset = static_cast<std::uint32_t>(begin);
            dataSize = chunkSize;
        }
        offset = begin + static_cast<std::size_t>(chunkSize) + (chunkSize & 1u);
    }
    if (format != 1u || (channels != 1u && channels != 2u) || bits != 16u
        || sampleRate < 8000u || sampleRate > 384000u
        || blockAlign != channels*2u || dataSize == 0u || dataOffset == 0u) {
        error = "Only PCM16 mono/stereo WAV from 8 to 384 kHz is supported.";
        return false;
    }
    const std::size_t frames = static_cast<std::size_t>(dataSize / blockAlign);
    if (frames == 0u || frames > kMaxFrames) {
        error = "WAV frame count exceeds the 20-million-frame safety limit.";
        return false;
    }
    AudioSuiteBuffer loaded;
    loaded.sampleRate = static_cast<int>(sampleRate);
    loaded.left.resize(frames);
    loaded.right.resize(frames);
    for (std::size_t i=0; i<frames; ++i) {
        const std::size_t at = static_cast<std::size_t>(dataOffset) + i*blockAlign;
        const auto l = static_cast<std::int16_t>(read16(bytes, at));
        const auto r = channels == 2u ? static_cast<std::int16_t>(read16(bytes, at+2u)) : l;
        loaded.left[i] = static_cast<float>(l)/32768.0f;
        loaded.right[i] = static_cast<float>(r)/32768.0f;
    }
    buffer = std::move(loaded);
    error.clear();
    return true;
}

bool savePcm16Wav(const std::filesystem::path& path, const AudioSuiteBuffer& buffer,
                  std::string& error) {
    if (!buffer.valid()) { error = "Cannot export an invalid audio buffer."; return false; }
    const std::uint64_t dataBytes = static_cast<std::uint64_t>(buffer.frames()) * 4u;
    if (dataBytes > std::numeric_limits<std::uint32_t>::max()-36u) {
        error = "Output WAV exceeds the RIFF size limit.";
        return false;
    }
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) { error = "Cannot create the output WAV file."; return false; }
    const auto rate = static_cast<std::uint32_t>(buffer.sampleRate);
    out.write("RIFF", 4); write32(out, static_cast<std::uint32_t>(36u+dataBytes));
    out.write("WAVEfmt ", 8); write32(out, 16u); write16(out, 1u); write16(out, 2u);
    write32(out, rate); write32(out, rate*4u); write16(out, 4u); write16(out, 16u);
    out.write("data", 4); write32(out, static_cast<std::uint32_t>(dataBytes));
    for (std::size_t i=0; i<buffer.frames(); ++i) {
        const auto toPcm = [](float sample) {
            const float bounded = std::clamp(sample, -1.0f, 0.9999695f);
            return static_cast<std::int16_t>(std::lrint(bounded*32768.0f));
        };
        write16(out, static_cast<std::uint16_t>(toPcm(buffer.left[i])));
        write16(out, static_cast<std::uint16_t>(toPcm(buffer.right[i])));
    }
    if (!out) { error = "Failed while writing PCM16 WAV data."; return false; }
    error.clear();
    return true;
}
} // namespace ventu
