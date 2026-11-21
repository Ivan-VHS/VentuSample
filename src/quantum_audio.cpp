#include "quantum_audio.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace ventu {
namespace {
constexpr double kPi = 3.1415926535897932384626433832795;
constexpr std::size_t kMaxFrames = 20u * 1000u * 1000u;

#define QP(ID, NAME, CAT, DESC, VISUAL, MODEL, VARIANT, AMOUNT, TONE, RATE, MIX) \
    QuantumAudioPlugin{ID, NAME, QuantumAudioCategory::CAT, DESC, VISUAL, \
        QuantumDspModel::MODEL, VARIANT, {AMOUNT, TONE, RATE, MIX}}

const std::vector<QuantumAudioPlugin> kPlugins{
    // I. Quarks, hadrons and strong interaction (1–23)
    QP("quark-confiner", "QUARK-CONFINER", Quarks, "Locks the low-frequency image to mono while retaining stereo detail above the crossover.", "Three luminous nodes held inside a taut force triangle.", StereoField, 0, .72, .28, .18, .82),
    QP("gluon-glue", "GLUON-GLUE", Quarks, "Linked-bus compressor that makes separate layers move as one controlled cluster.", "Coloured field tubes converge on one pulsing core.", Dynamics, 0, .68, .52, .22, .78),
    QP("color-charge-eq", "COLOR-CHARGE EQ", Quarks, "Three-band low, mid and high balance with a neutral summed reference.", "A tri-colour spectral prism with a central balance point.", Equalizer, 0, .48, .55, .2, .86),
    QP("up-down-harmonizer", "UP-DOWN HARMONIZER", Quarks, "Micro-pitch movement using a short, modulated dual-head delay approximation.", "Opposing pitch vectors with fractional interval markings.", Modulator, 0, .36, .72, .41, .52),
    QP("strange-quark", "STRANGE-QUARK", Quarks, "Nonlinear phase-coloured saturation that emphasizes upper partials.", "A violet unstable sphere shedding harmonic sparks.", Saturation, 0, .62, .78, .24, .71),
    QP("charm-exciter", "CHARM-EXCITER", Quarks, "Adds controlled upper-harmonic definition with a restrained high-pass residual.", "A gold and turquoise crystal trail over the spectrum.", Exciter, 0, .37, .82, .25, .68),
    QP("top-quark-mass", "TOP-QUARK MASS", Quarks, "Adds impact weight by increasing attack contrast and low-body perception.", "A dense dark sphere bending the coordinate grid.", Transient, 0, .74, .3, .18, .82),
    QP("bottom-quark-decay", "BOTTOM-QUARK DECAY", Quarks, "Tempo-independent multi-tap decay with controlled feedback.", "A heavy line branches into a fan of fine reflections.", Delay, 0, .47, .52, .3, .38),
    QP("hadron-collider", "HADRON-COLLIDER", Quarks, "Combines nonlinear collision, phase contrast and transient emphasis.", "Two opposing beams flash at a circular detector.", Morph, 0, .72, .62, .4, .66),
    QP("baryon-trio", "BARYON-TRIO", Quarks, "Three-band saturation with linked gain and crossover protection.", "Three orbital rings around one barycentre.", Spectrum, 0, .54, .58, .18, .76),
    QP("meson-bridge", "MESON-BRIDGE", Quarks, "Short resonant echo that couples rhythmic attacks and low-frequency body.", "A pulsing bridge between two orbiting nodes.", Resonator, 0, .43, .34, .46, .39),
    QP("asymptotic-free", "ASYMPTOTIC-FREE", Quarks, "Keeps low-level material open and progressively restrains large peaks.", "A free green region meets a red containment wall.", Dynamics, 1, .58, .48, .2, .9),
    QP("pentaquark-chord", "PENTAQUARK-CHORD", Quarks, "Adds a five-part harmonic colour from the signal envelope and selected formant bands.", "A five-point star whose nodes respond to voice energy.", Vocoder, 0, .44, .61, .32, .56),
    QP("jet-fragmentation", "JET-FRAGMENTATION", Quarks, "Turns sharp attacks into deterministic short grain fragments.", "A particle jet fans out from one transient point.", Granular, 0, .72, .24, .62, .64),
    QP("sea-quark-noise", "SEA-QUARK NOISE", Quarks, "Adds low-level, envelope-following stochastic texture in pauses and tails.", "A dark field of rapidly appearing and fading sparks.", Noise, 0, .24, .68, .36, .28),
    QP("valence-core", "VALENCE-CORE", Quarks, "Highlights three dominant spectral regions and suppresses surrounding clutter.", "Three bright spectral columns over a subdued floor.", Spectrum, 1, .55, .42, .2, .8),
    QP("string-tension", "STRING-TENSION", Quarks, "Links stereo width to harmonic drive so wider material becomes denser, not louder.", "An elastic line between left and right channels.", StereoField, 1, .56, .67, .24, .78),
    QP("bag-model-limiter", "BAG-MODEL LIMITER", Quarks, "Soft containment stage that rounds peaks without hard digital clipping.", "A flexible membrane deforms around incoming peaks.", Dynamics, 2, .74, .55, .15, .93),
    QP("flavor-oscillator", "FLAVOR-OSCILLATOR", Quarks, "Slowly changes filter and saturation colour using a cyclic modulation path.", "A six-sector colour wheel with gradual transitions.", Morph, 1, .48, .62, .3, .66),
    QP("hyperon-sub", "HYPERON-SUB", Quarks, "Adds a short, phase-stable sub-octave foundation from the incoming low-frequency envelope.", "A deep blue funnel with a half-life decay marker.", SubGenerator, 0, .72, .22, .16, .72),
    QP("pion-punch", "PION-PUNCH", Quarks, "Fast attack emphasis with a bounded transient envelope.", "A white impulse needle above a rounded waveform body.", Transient, 1, .78, .38, .15, .82),
    QP("glueball-reverb", "GLUEBALL-REVERB", Quarks, "Dense reflective reverb with short, metallic feedback paths.", "A faceted knot of interwoven luminous loops.", Reverb, 0, .51, .77, .32, .44),
    QP("lattice-quantizer", "LATTICE-QUANTIZER", Quarks, "Quantizes amplitude and local sample timing to a controllable lattice.", "A cubic grid whose nodes flash on sample events.", Quantizer, 0, .45, .32, .5, .8),

    // II. Quantum mechanics, superposition and entanglement (24–46)
    QP("superposition-synth", "SUPERPOSITION-SYNTH", Quantum, "Blends several harmonic states into one evolving oscillator texture.", "Ghost waveforms collapse into one clear trace.", Synth, 0, .52, .7, .25, .58),
    QP("schrodinger-gate", "SCHRÖDINGER-GATE", Quantum, "Probabilistic envelope gate with a deterministic threshold trigger.", "A translucent state box with a 50/50 probability dial.", Gate, 0, .58, .42, .2, .76),
    QP("entanglement-stereo", "ENTANGLEMENT-STEREO", Quantum, "Couples left and right movement through a linked mid-side modulation law.", "Two linked spirals mirror each other's movement.", StereoField, 2, .62, .48, .38, .78),
    QP("wavefunction-collapse", "WAVEFUNCTION-COLLAPSE", Quantum, "Compresses a diffuse envelope into a sharp, transient-focused shape.", "A wide probability cloud contracts into one vertical line.", Transient, 2, .72, .26, .18, .86),
    QP("heisenberg-blur", "HEISENBERG-BLUR", Quantum, "Trades fine timing definition for modulated pitch and phase movement.", "An uncertainty ellipse spans time and frequency axes.", Modulator, 1, .51, .56, .62, .65),
    QP("tunneling-filter", "TUNNELING-FILTER", Quantum, "Lets a controlled amount of energy pass a steep spectral boundary.", "Wave packets leak through a closed filter wall.", Equalizer, 1, .48, .2, .2, .82),
    QP("qubit-sequencer", "QUBIT-SEQUENCER", Quantum, "Creates a stepped, state-driven sample pattern with repeatable probability.", "A row of state spheres replaces conventional step buttons.", Granular, 1, .57, .45, .7, .72),
    QP("decoherence-dissolve", "DECOHERENCE-DISSOLVE", Quantum, "Gradually de-correlates transients into a diffuse reflective tail.", "A clear sine wave dissolves into pixel-like particles.", Reverb, 1, .54, .73, .36, .58),
    QP("eigenstate-resonator", "EIGENSTATE-RESONATOR", Quantum, "Emphasizes stable resonant bands with a tuned feedback comb.", "Narrow laser lines stand at resonant peaks.", Resonator, 1, .58, .68, .22, .7),
    QP("planck-crusher", "PLANCK-CRUSHER", Quantum, "Reduces amplitude resolution and sample update rate for microscopic texture.", "A fine staircase grid at extreme magnification.", Quantizer, 1, .68, .25, .68, .78),
    QP("probability-cloud-delay", "PROBABILITY-CLOUD DELAY", Quantum, "Adds repeat timing offsets inside a bounded deterministic timing cloud.", "Soft Gaussian repeat markers hover around a time grid.", Delay, 1, .42, .52, .44, .5),
    QP("bell-state-panner", "BELL-STATE PANNER", Quantum, "Creates a four-state stereo trajectory while protecting the central image.", "A four-lobed phase diagram in polar coordinates.", StereoField, 3, .66, .57, .44, .78),
    QP("quantum-zeno-freeze", "QUANTUM-ZENO FREEZE", Quantum, "Captures short slices and sustains them with a controllable observation rate.", "A strobe beam holds a waveform in one position.", Granular, 2, .65, .35, .6, .72),
    QP("spin-up-spin-down", "SPIN-UP / SPIN-DOWN", Quantum, "Rotates stereo phase clockwise or counter-clockwise at a tempo-like rate.", "A gyroscopic sphere carries an up/down spin vector.", StereoField, 4, .54, .48, .48, .76),
    QP("pauli-exclusion-eq", "PAULI-EXCLUSION EQ", Quantum, "Makes complementary spectral slots for a less congested two-layer balance.", "A checkerboard spectral matrix rejects occupied cells.", Spectrum, 2, .6, .65, .25, .83),
    QP("feynman-path-reverb", "FEYNMAN-PATH REVERB", Quantum, "Accumulates multiple short reflection paths into a dense but bounded tail.", "A web of reflected paths links source and listener.", Reverb, 2, .58, .68, .34, .48),
    QP("zero-point-saturator", "ZERO-POINT SATURATOR", Quantum, "Raises low-level harmonic detail through a restrained nonlinear floor.", "A quiet baseline vibrates with fine golden micro-waves.", Saturation, 1, .34, .74, .22, .62),
    QP("wave-particle-morpher", "WAVE-PARTICLE MORPHER", Quantum, "Morphs between smooth modulation and stepped granular pulses.", "A sine trace transforms into a stream of bright spheres.", Morph, 2, .56, .55, .4, .68),
    QP("teleport-send", "TELEPORT-SEND", Quantum, "A phase-aware short routing delay for transferring a signal between offline stages.", "A portal ring shows paired timing and phase states.", Delay, 2, .28, .6, .32, .58),
    QP("hamiltonian-mod", "HAMILTONIAN-MOD", Quantum, "Uses signal energy to drive a smooth amplitude and filter movement.", "A two-colour energy pendulum moves between states.", Dynamics, 3, .52, .48, .35, .74),
    QP("bloch-sphere-chorus", "BLOCH-SPHERE CHORUS", Quantum, "A multi-voice chorus with bounded phase paths and shared stereo motion.", "A transparent globe carries points along its meridians.", Modulator, 2, .45, .62, .5, .62),
    QP("observer-effect", "OBSERVER-EFFECT", Quantum, "Adds repeatable motion and nonlinear colour; it never depends on the desktop cursor or window focus.", "A central iris reacts to the evolving spectral trace.", Morph, 3, .48, .72, .34, .64),
    QP("multiverse-bounce", "MULTIVERSE-BOUNCE", Quantum, "Creates four parallel gain and tone branches for a controllable parallel blend.", "A branching tree of four channels with a branch selector.", Dynamics, 4, .55, .56, .3, .68),

    // III. Atoms, orbitals and nuclear structure (47–69)
    QP("orbital-harmonics", "ORBITAL-HARMONICS (s, p, d, f)", Atomic, "Shapes harmonic density and stereo motion using four orbital-style modes.", "Rotating volumetric s, p, d and f orbital clouds.", Exciter, 5, .55, .66, .28, .68),
    QP("isotope-detune", "ISOTOPE-DETUNE", Atomic, "Adds small, bounded detuning offsets with stable modulation depth.", "A nuclide map highlights stable and drifting lanes.", Modulator, 3, .35, .62, .3, .55),
    QP("half-life-decay", "HALF-LIFE DECAY", Atomic, "Applies a measured exponential decay to the signal envelope and high-frequency detail.", "An exponential decay curve is marked with T-half.", Dynamics, 5, .52, .44, .26, .72),
    QP("fission-splitter", "FISSION-SPLITTER", Atomic, "Splits the spectrum into two complementary bands and emphasizes the split transient.", "A heavy nucleus separates into two energetic fragments.", Spectrum, 3, .62, .4, .22, .74),
    QP("fusion-compressor", "FUSION-COMPRESSOR", Atomic, "Links envelope compression and soft saturation for a dense combined signal.", "Two spheres merge into one bright compressed core.", Dynamics, 6, .62, .5, .18, .82),
    QP("valence-ring", "VALENCE-RING", Atomic, "Pairs saturation amount with stereo width in one macro-style control.", "Concentric orbitals show one to eight valence points.", Saturation, 2, .48, .58, .23, .74),
    QP("spectral-lines-balmer", "SPECTRAL-LINES (BALMER)", Atomic, "Emphasizes selected narrow spectral regions using a multi-band tone model.", "A dark spectral strip carries bright emission lines.", Spectrum, 4, .5, .78, .2, .82),
    QP("ionization-drive", "IONIZATION-DRIVE", Atomic, "A staged nonlinear drive that increases upper harmonic density with intensity.", "Ionization stages discharge as the level rises.", Saturation, 3, .72, .76, .24, .78),
    QP("covalent-sidechain", "COVALENT-SIDECHAIN", Atomic, "An envelope-linked sidechain-style processor for complementary layer control.", "Two overlapping orbitals share a glowing centre.", Dynamics, 7, .54, .52, .25, .78),
    QP("nuclear-force-sub", "NUCLEAR-FORCE SUB", Atomic, "Reinforces a narrow 30–60 Hz foundation without widening the low end.", "A dense pulsing core has a sharply bounded force field.", SubGenerator, 1, .7, .18, .12, .74),
    QP("photon-emission", "PHOTON-EMISSION", Atomic, "Adds short, high-frequency harmonic flashes around detected transients.", "Wave packets emit as short light-like bursts.", Exciter, 1, .44, .86, .4, .68),
    QP("brownian-drift", "BROWNIAN-DRIFT", Atomic, "Adds deterministic pseudo-random micro-motion to pitch, filter and stereo position.", "A wandering particle draws an irregular trajectory.", Modulator, 4, .38, .55, .52, .62),
    QP("crystal-lattice-reverb", "CRYSTAL-LATTICE REVERB", Atomic, "Uses short, slightly inharmonic reflection lines for a crystalline spatial tail.", "A molecular lattice carries reflected light pulses.", Reverb, 3, .52, .78, .38, .52),
    QP("laser-coherence", "LASER-COHERENCE", Atomic, "Improves low-band phase consistency and narrows unstable stereo differences.", "A scattered beam focuses into one coherent line.", StereoField, 5, .62, .25, .2, .84),
    QP("plasma-state", "PLASMA-STATE", Atomic, "Combines bright asymmetric drive with a slowly moving spectral edge.", "A luminous torus of plasma sits inside magnetic coils.", Saturation, 4, .68, .82, .44, .7),
    QP("neutron-star-density", "NEUTRON-STAR DENSITY", Atomic, "A high-density compressor and soft peak shaper for heavy program material.", "A compact star emits two narrow polar jets.", Dynamics, 8, .78, .55, .14, .9),
    QP("alpha-particle-punch", "ALPHA-PARTICLE PUNCH", Atomic, "Adds a hard, short attack while keeping the body under envelope control.", "Four linked spheres punch through a waveform barrier.", Transient, 3, .74, .34, .16, .82),
    QP("beta-decay-shifter", "BETA-DECAY SHIFTER", Atomic, "Morphs a muted tone toward a brighter harmonic profile with a short upper transient.", "A curve changes state and releases a bright particle trail.", Morph, 4, .5, .72, .32, .68),
    QP("gamma-burst-transient", "GAMMA-BURST TRANSIENT", Atomic, "Creates a focused, high-frequency attack flash with strict output bounds.", "A narrow high-energy flash crosses the upper spectrum.", Transient, 4, .66, .88, .16, .78),
    QP("boson-condensate", "BOSON-CONDENSATE", Atomic, "Reduces detune and phase spread for a stable unison-like image.", "Independent waves merge into one smooth blue contour.", Modulator, 5, .42, .28, .12, .8),
    QP("fermion-stack", "FERMION-STACK", Atomic, "Stacks several harmonic layers with deterministic frequency and phase separation.", "A stepped energy ladder fills one state at a time.", Synth, 1, .52, .68, .3, .62),
    QP("magnetic-moment-pan", "MAGNETIC-MOMENT PAN", Atomic, "Moves the stereo image along a smooth precessing pan orbit.", "A spinning top carries a magnetic dipole vector.", StereoField, 6, .58, .56, .34, .8),
    QP("auger-cascade", "AUGER-CASCADE", Atomic, "Generates a controlled cascade of harmonics from the incoming envelope.", "A chain of arrows jumps between orbital shells.", Exciter, 2, .58, .7, .36, .7),

    // IV. Fields, waves and relativistic acoustics (70–92)
    QP("interference-slit", "INTERFERENCE-SLIT", Fields, "Creates moving comb-filter interference from a pair of short phase paths.", "Two slits produce a live interference pattern.", Resonator, 2, .58, .7, .42, .66),
    QP("doppler-horizon", "DOPPLER-HORIZON", Fields, "Simulates a moving source using controlled pitch and stereo modulation.", "A passing source compresses wavefronts ahead and stretches them behind.", Modulator, 6, .54, .62, .55, .68),
    QP("time-dilation-stretch", "TIME-DILATION STRETCH", Fields, "Uses bounded variable-rate granular reads to create time and pitch movement.", "A space-time grid stretches along the time axis.", Time, 0, .48, .55, .34, .66),
    QP("event-horizon-limiter", "EVENT-HORIZON LIMITER", Fields, "A fast soft-knee ceiling with an optional restrained harmonic halo.", "A black disc with a bright accretion ring contains the peaks.", Dynamics, 9, .8, .54, .16, .94),
    QP("hawking-radiation", "HAWKING-RADIATION", Fields, "Adds a tiny filtered noise tail around low-level decays.", "A fine glow rises from the event horizon edge.", Noise, 1, .19, .82, .44, .3),
    QP("gravitational-wave", "GRAVITATIONAL-WAVE", Fields, "Modulates mid-side width in a slow quadrature movement.", "A quadrupole grid pulses on two opposing axes.", StereoField, 7, .52, .56, .12, .8),
    QP("cherenkov-glow", "CHERENKOV-GLOW", Fields, "Adds a bright upper-harmonic glow when attacks cross a level threshold.", "A blue shock cone surrounds a fast transient.", Exciter, 3, .5, .88, .25, .65),
    QP("faraday-rotator", "FARADAY-ROTATOR", Fields, "Rotates the stereo image with low-frequency energy as its control signal.", "A polarization vector turns inside a magnetic coil.", StereoField, 8, .56, .62, .35, .78),
    QP("standing-wave-room", "STANDING-WAVE ROOM", Fields, "Adds tuned comb resonances that emulate a small virtual resonator.", "A stationary wave shows bright nodes and dark antinodes.", Resonator, 3, .52, .42, .18, .6),
    QP("soliton-pulse", "SOLITON-PULSE", Fields, "Strengthens the transient contour while preserving the central pulse envelope.", "A single bell-shaped pulse travels without visible decay.", Transient, 5, .46, .34, .18, .78),
    QP("phase-velocity-eq", "PHASE-VELOCITY EQ", Fields, "Shapes low, middle and high bands with separate phase-colour emphasis.", "A dispersion curve contrasts early and late frequency groups.", Spectrum, 5, .44, .62, .18, .8),
    QP("casimir-gap", "CASIMIR-GAP", Fields, "Uses a narrow tuned resonator with a short low-frequency boundary.", "Two mirrored plates frame a microscopic resonant gap.", Resonator, 4, .4, .24, .2, .66),
    QP("magneto-strictive", "MAGNETO-STRICTIVE", Fields, "Adds transformer-like hysteresis using asymmetric saturation and recovery.", "A metal core deforms inside magnetic flux lines.", Saturation, 5, .56, .45, .22, .72),
    QP("waveguide-tube", "WAVEGUIDE-TUBE", Fields, "Uses variable comb reflection geometry to emulate a resonant waveguide.", "A curved metal tube carries moving internal modes.", Resonator, 5, .5, .58, .32, .68),
    QP("diffraction-grating", "DIFFRACTION-GRATING", Fields, "Redistributes spectral emphasis into a set of narrow, bright bands.", "A grating separates one beam into a frequency fan.", Spectrum, 6, .5, .75, .25, .78),
    QP("wormhole-delay", "WORMHOLE-DELAY", Fields, "Uses a pre-echo and short reverse-grain cue to suggest a time-fold effect.", "A curved tunnel joins two points on a time line.", Delay, 3, .48, .44, .54, .48),
    QP("tachyon-transient", "TACHYON-TRANSIENT", Fields, "Creates a short pre-attack cue from a look-ahead envelope copy in offline processing.", "A light pulse leads the main waveform.", Transient, 6, .58, .62, .18, .72),
    QP("vacuum-polarization", "VACUUM-POLARIZATION", Fields, "Adds subtle nonlinear interaction to quiet passages and signal gaps.", "Virtual pairs align along a field boundary.", Noise, 2, .18, .5, .38, .28),
    QP("holographic-principle", "HOLOGRAPHIC-PRINCIPLE", Fields, "Combines mid-side projection and three-band balancing into a compact spatial image.", "An interference plate reconstructs a volumetric audio image.", Morph, 5, .52, .62, .3, .7),
    QP("dark-matter-sub", "DARK-MATTER SUB", Fields, "Adds a restrained low-frequency anchor from the incoming bass envelope.", "An invisible halo bends the lower spectral grid.", SubGenerator, 2, .58, .18, .12, .72),
    QP("dark-energy-expander", "DARK-ENERGY EXPANDER", Fields, "Expands stereo width smoothly as the signal decays.", "Points move outward across a widening spatial field.", StereoField, 9, .48, .56, .22, .72),
    QP("plasmon-surface", "PLASMON-SURFACE", Fields, "Adds near-field upper-mid harmonics for a closer, more articulated sound.", "A surface wave travels along a reflective boundary.", Exciter, 4, .4, .7, .2, .72),
    QP("lorentz-contraction", "LORENTZ-CONTRACTION", Fields, "Applies short, bounded variable-rate contraction to transient-rich material.", "A waveform packet compresses along its horizontal axis.", Time, 1, .46, .52, .4, .7),

    // V. Instruments, generators and experimental processors (93–133)
    QP("atom-drum", "ATOM-DRUM", Experimental, "Adds a compact percussive oscillator and transient layer to the incoming signal.", "A planet-like atom assigns drum roles to nucleus and shells.", Synth, 2, .58, .32, .25, .7),
    QP("quark-bass", "QUARK-BASS", Experimental, "A three-part low-frequency oscillator with bounded phase separation.", "Three bass oscillators are linked by a force triangle.", SubGenerator, 3, .72, .2, .18, .74),
    QP("quantum-pad", "QUANTUM-PAD", Experimental, "Adds a slowly evolving multi-part harmonic bed from the input envelope.", "A multidimensional probability cloud changes colour over time.", Synth, 3, .42, .76, .16, .46),
    QP("schrodinger-arp", "SCHRÖDINGER-ARP", Experimental, "Creates probabilistic, repeatable note-like grain steps from the source signal.", "A branched note grid highlights its active path.", Granular, 3, .5, .54, .7, .68),
    QP("antimatter-inverter", "ANTIMATTER-INVERTER", Experimental, "Uses phase inversion and mid-side cancellation to reduce correlated conflicts.", "Opposing waves meet and cancel at a central point.", StereoField, 10, .62, .48, .2, .86),
    QP("annihilation-distortion", "ANNIHILATION-DISTORTION", Experimental, "A collision-driven asymmetric distortion for energetic harmonic bursts.", "A photon flash appears at the collision threshold.", Saturation, 6, .72, .78, .25, .76),
    QP("chronon-slicer", "CHRONON-SLICER", Experimental, "Cuts signal into repeatable short time cells with optional reverse ordering.", "A timeline of draggable time quanta.", Granular, 4, .66, .36, .72, .78),
    QP("spin-network-reverb", "SPIN-NETWORK REVERB", Experimental, "A network of short feedback reflections with an irregular spatial pattern.", "A 3D graph carries pulses across linked nodes.", Reverb, 4, .54, .64, .42, .5),
    QP("coherence-meter", "COHERENCE-METER", Experimental, "Measures peak, RMS, crest and stereo correlation without changing the audio.", "A correlation matrix sits beside a live interferometer trace.", Analyzer, 0, .5, .5, .5, .0),
    QP("wave-packet-generator", "WAVE-PACKET GENERATOR", Experimental, "Adds a Gaussian-windowed oscillator packet shaped by the incoming envelope.", "A Gaussian envelope contains a high-frequency carrier.", Synth, 4, .5, .68, .42, .62),
    QP("superfluid-delay", "SUPERFLUID-DELAY", Experimental, "Creates clear feedback repeats with minimal high-frequency damping.", "A closed loop carries a continuously travelling pulse.", Delay, 4, .44, .72, .46, .44),
    QP("superconductor-bus", "SUPERCONDUCTOR-BUS", Experimental, "A fast linked-channel compressor that preserves attack contrast.", "A cryogenic blue channel carries an unrestricted signal flow.", Dynamics, 10, .46, .4, .16, .82),
    QP("josephson-junction", "JOSEPHSON-JUNCTION", Experimental, "Adds a nonlinear sample-and-hold distortion with high-rate modulation.", "Two superconductors face a thin oscillating barrier.", Quantizer, 2, .54, .72, .58, .76),
    QP("copenhagen-mixer", "COPENHAGEN-MIXER", Experimental, "Balances the signal with a tone-dependent energy reference; no cursor tracking is used.", "A focus reticle sits above a multichannel spectrum.", Dynamics, 11, .38, .5, .24, .78),
    QP("many-worlds-comp", "MANY-WORLDS COMP", Experimental, "Blends multiple parallel compression strengths into one controlled result.", "Eight translucent compression curves merge together.", Dynamics, 12, .6, .54, .2, .78),
    QP("planck-length-gate", "PLANCK-LENGTH GATE", Experimental, "Uses a fast, smoothed envelope gate with sample-accurate offline control.", "A microscopic shutter follows a smooth S-curve.", Gate, 1, .54, .26, .18, .8),
    QP("neutrino-pass", "NEUTRINO-PASS", Experimental, "Emphasizes a selected pass band with a gentle minimum-phase tone model.", "A ghost particle passes through a dense spectral wall.", Equalizer, 2, .42, .76, .18, .84),
    QP("chamber-tracker", "CHAMBER-TRACKER", Experimental, "Analyzes envelope peaks and converts them into a repeatable event-density report.", "A bubble chamber draws spiral particle tracks.", Analyzer, 1, .5, .5, .5, .0),
    QP("crystal-quark", "CRYSTAL-QUARK", Experimental, "Applies deterministic grain symmetry and harmonic scattering.", "A faceted gemstone contains internal luminous nodes.", Granular, 5, .58, .72, .62, .68),
    QP("quantum-dot-vocoder", "QUANTUM-DOT VOCODER", Experimental, "A three-band envelope vocoder with discrete formant-like carrier components.", "A grid of nanocrystals changes colour with frequency.", Vocoder, 1, .58, .66, .32, .66),
    QP("teleport-preset", "TELEPORT-PRESET", Experimental, "Morphs between dry, saturated and spatial states using one continuous macro value.", "Two portal rings exchange their parameter states.", Morph, 6, .5, .62, .34, .7),
    QP("eigen-vocal", "EIGEN-VOCAL", Experimental, "Separates low body, mid formants and high articulation with an envelope-guided model.", "Four orthogonal vectors represent vocal components.", Vocoder, 2, .52, .62, .26, .7),
    QP("magnetar-limiter", "MAGNETAR-LIMITER", Experimental, "A dense peak limiter with magnetic-style asymmetric soft saturation.", "A pulsing star is wrapped in strong magnetic field lines.", Dynamics, 13, .78, .7, .12, .92),
    QP("quark-gluon-plasma", "QUARK-GLUON PLASMA", Experimental, "High-density nonlinear drive with controlled cross-band harmonic mixing.", "A boiling field of coloured quarks and connecting strings.", Saturation, 7, .76, .82, .38, .8),
    QP("phase-entangler", "PHASE-ENTANGLER", Experimental, "Aligns stereo movement and reduces unstable inter-channel phase differences.", "Interwoven wavefronts align into a coherent pattern.", StereoField, 11, .7, .44, .2, .84),
    QP("sub-atomic-transient", "SUB-ATOMIC TRANSIENT", Experimental, "Shapes attack into click, impulse and body zones with independent envelope emphasis.", "The first millisecond is split into three microscopic regions.", Transient, 7, .62, .36, .18, .82),
    QP("zero-latency-oracle", "ZERO-LATENCY ORACLE", Experimental, "An offline look-ahead soft peak shaper; it does not claim zero live-processing latency.", "A ghost waveform forecasts the next few milliseconds.", Dynamics, 14, .68, .62, .12, .92),
    QP("hybrid-boson", "HYBRID-BOSON", Experimental, "Combines moderate compression, saturation and spatial control in one field.", "One central field exposes dynamics, tone and width axes.", Morph, 7, .54, .62, .24, .78),
    QP("quantum-walk-lfo", "QUANTUM-WALK LFO", Experimental, "Uses deterministic random-walk modulation with bounded drift and repeatability.", "A branching modulation path travels across a step grid.", Modulator, 7, .4, .58, .56, .62),
    QP("spectral-tunnel", "SPECTRAL-TUNNEL", Experimental, "Reduces low-mid masking while restoring upper-band definition.", "An energy tunnel crosses a gap in the middle spectrum.", Spectrum, 7, .52, .68, .2, .82),
    QP("atom-clock-sync", "ATOM-CLOCK SYNC", Experimental, "Applies deterministic micro-timing and phase correction to short events.", "A precision atomic clock marks a fine time grid.", Time, 2, .32, .4, .6, .74),
    QP("fractal-orbital", "FRACTAL-ORBITAL", Experimental, "A nested chorus/flanger motion with bounded multi-rate modulation.", "Fractal epicycles orbit around one another.", Modulator, 8, .52, .58, .68, .66),
    QP("relativity-pan", "RELATIVITY-PAN", Experimental, "Pan movement includes a short interaural timing offset for spatial depth.", "A curved coordinate grid surrounds the listening position.", StereoField, 12, .46, .62, .32, .8),
    QP("string-landscape", "STRING-LANDSCAPE", Experimental, "A macro-controlled texture morph across harmonics, modulation and stereo.", "A multidimensional manifold folds into one control surface.", Morph, 8, .5, .74, .4, .66),
    QP("quantum-noise-gate", "QUANTUM-NOISE GATE", Experimental, "Envelope and noise-floor gate with a soft release and hysteresis-like hold.", "An entropy filter separates irregular noise from structured signal.", Gate, 2, .6, .48, .3, .82),
    QP("nucleon-punch", "NUCLEON-PUNCH", Experimental, "Reinforces bass and lower-mid body with a linked mono low-frequency envelope.", "A proton and neutron pulse as a coupled pair.", SubGenerator, 4, .64, .32, .16, .78),
    QP("photon-delay", "PHOTON-DELAY", Experimental, "A clear short delay with a filtered feedback loop and stereo reflection offset.", "Laser beams bounce across a bank of ideal mirrors.", Delay, 5, .42, .86, .42, .5),
    QP("wave-collapse-comp", "WAVE-COLLAPSE COMP", Experimental, "Fast envelope compression with a soft-knee peak clamp.", "Amplitude collapses at the moment it crosses a threshold.", Dynamics, 15, .72, .52, .12, .92),
    QP("entropy-reverser", "ENTROPY-REVERSER", Experimental, "Uses deterministic reverse-grain windows to pull scattered tails toward the transient.", "Particles gather from a cloud into a defined crystal.", Granular, 6, .56, .48, .62, .68),
    QP("singularity-bass", "SINGULARITY-BASS", Experimental, "Builds a focused mono sub layer with strict low-frequency stability.", "A single bright point anchors the centre of the low spectrum.", SubGenerator, 5, .82, .16, .12, .82),
    QP("omega-field", "OMEGA-FIELD", Experimental, "A final macro processor combining tone, dynamics, phase and spatial balance.", "One unified sphere joins the spectral and spatial fields.", Morph, 9, .5, .58, .22, .78)
};

#undef QP

bool equalFold(std::string_view left, std::string_view right) noexcept {
    if (left.size() != right.size()) return false;
    for (std::size_t i = 0; i < left.size(); ++i) {
        const auto fold = [](unsigned char ch) noexcept {
            return static_cast<unsigned char>(ch >= 'A' && ch <= 'Z' ? ch - 'A' + 'a' : ch);
        };
        if (fold(static_cast<unsigned char>(left[i])) != fold(static_cast<unsigned char>(right[i])))
            return false;
    }
    return true;
}

double onePoleCoefficient(double frequency, int sampleRate) {
    const double bounded = std::clamp(frequency, 5.0, static_cast<double>(sampleRate) * 0.45);
    return 1.0 - std::exp(-2.0 * kPi * bounded / static_cast<double>(sampleRate));
}

float interpolate(const std::vector<float>& source, std::size_t frames, int channels,
                  double frame, int channel) {
    if (frames == 0u || frame < 0.0) return 0.0f;
    const double bounded = std::clamp(frame, 0.0, static_cast<double>(frames - 1u));
    const auto first = static_cast<std::size_t>(bounded);
    const auto second = std::min(first + 1u, frames - 1u);
    const double fraction = bounded - static_cast<double>(first);
    const auto a = source[first * static_cast<std::size_t>(channels) + static_cast<std::size_t>(channel)];
    const auto b = source[second * static_cast<std::size_t>(channels) + static_cast<std::size_t>(channel)];
    return static_cast<float>(static_cast<double>(a) * (1.0 - fraction) + static_cast<double>(b) * fraction);
}

void processStereoField(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                        const QuantumAudioParameters& p, const std::vector<float>& dry,
                        std::vector<float>& wet) {
    if (audio.channels == 1) return;
    const double amount = p.amount;
    const double width = plugin.variant % 3 == 1 ? 1.0 + amount * 1.6
        : (plugin.variant % 3 == 2 ? 1.0 - amount * 0.9 : 1.0 - amount * 0.82);
    const double rotation = (static_cast<double>(plugin.variant % 13) - 6.0) * amount * 0.012;
    const double lowAlpha = onePoleCoefficient(65.0 + p.tone * 280.0, audio.sampleRate);
    double lowL = 0.0, lowR = 0.0;
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        const auto base = frame * 2u;
        const double left = dry[base];
        const double right = dry[base + 1u];
        lowL += lowAlpha * (left - lowL);
        lowR += lowAlpha * (right - lowR);
        const double lowMid = (lowL + lowR) * 0.5;
        const double highL = left - lowL;
        const double highR = right - lowR;
        const double mid = (highL + highR) * 0.5 + lowMid;
        const double side = (highL - highR) * 0.5 * width;
        const double angle = rotation * std::sin(2.0 * kPi * (0.08 + p.rate * 1.8)
                                                 * static_cast<double>(frame) / audio.sampleRate);
        const double rotatedMid = mid * std::cos(angle) - side * std::sin(angle);
        const double rotatedSide = mid * std::sin(angle) + side * std::cos(angle);
        wet[base] = static_cast<float>(rotatedMid + rotatedSide);
        wet[base + 1u] = static_cast<float>(rotatedMid - rotatedSide);
    }
}

void processDynamics(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                     const QuantumAudioParameters& p, const std::vector<float>& dry,
                     std::vector<float>& wet) {
    double envelope = 0.0;
    const double attack = onePoleCoefficient(8.0 + p.rate * 70.0, audio.sampleRate);
    const double release = onePoleCoefficient(4.0 + (1.0 - p.rate) * 18.0, audio.sampleRate);
    const int mode = plugin.variant % 4;
    const double threshold = mode == 2 ? 0.68 : 0.82 - p.amount * 0.48;
    const double ratio = 1.0 + p.amount * (mode == 0 ? 11.0 : 17.0);
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        double magnitude = 0.0;
        for (int ch = 0; ch < audio.channels; ++ch)
            magnitude += std::abs(static_cast<double>(dry[frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch)]));
        magnitude /= static_cast<double>(audio.channels);
        const double coefficient = magnitude > envelope ? attack : release;
        envelope += coefficient * (magnitude - envelope);
        double gain = 1.0;
        if (mode == 3) {
            if (envelope < threshold) gain = std::pow(std::max(0.01, envelope / threshold), 1.8 + p.amount * 2.0);
        } else if (envelope > threshold) {
            gain = std::pow(threshold / std::max(envelope, 1.0e-9), 1.0 - 1.0 / ratio);
        }
        if (mode == 2 || mode == 0) gain = std::min(gain, 1.0);
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            double value = static_cast<double>(dry[index]) * gain;
            if (mode == 1 || mode == 2 || plugin.variant >= 8)
                value = std::tanh(value * (1.0 + p.amount * 2.4)) / std::tanh(1.0 + p.amount * 2.4);
            wet[index] = static_cast<float>(value);
        }
    }
}

void processEqualizer(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                      const QuantumAudioParameters& p, const std::vector<float>& dry,
                      std::vector<float>& wet) {
    const double lowCut = 110.0 + p.tone * 300.0;
    const double highCut = 1800.0 + p.tone * 10500.0;
    const double aLow = onePoleCoefficient(lowCut, audio.sampleRate);
    const double aHigh = onePoleCoefficient(highCut, audio.sampleRate);
    const int variant = plugin.variant % 4;
    const double lowGain = variant == 1 ? 1.0 - p.amount * 0.55 : 1.0 + p.amount * 0.9;
    const double midGain = variant == 2 ? 1.0 - p.amount * 0.45 : 1.0 + (variant == 3 ? -0.25 : 0.18) * p.amount;
    const double highGain = variant == 1 || variant == 3 ? 1.0 + p.amount * 0.85 : 1.0 + p.amount * 0.38;
    std::vector<double> low(static_cast<std::size_t>(audio.channels), 0.0);
    std::vector<double> upper(static_cast<std::size_t>(audio.channels), 0.0);
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto channel = static_cast<std::size_t>(ch);
            const auto index = frame * static_cast<std::size_t>(audio.channels) + channel;
            const double x = dry[index];
            low[channel] += aLow * (x - low[channel]);
            upper[channel] += aHigh * (x - upper[channel]);
            const double mid = upper[channel] - low[channel];
            const double high = x - upper[channel];
            wet[index] = static_cast<float>(low[channel] * lowGain + mid * midGain + high * highGain);
        }
    }
}

void processSaturation(const QuantumAudioPlugin& plugin,
                       const QuantumAudioParameters& p, const std::vector<float>& dry,
                       std::vector<float>& wet) {
    const double drive = 1.0 + p.amount * (3.0 + static_cast<double>(plugin.variant % 7) * 1.8);
    const double normalizer = std::tanh(drive);
    const double bias = (static_cast<double>(plugin.variant % 3) - 1.0) * p.amount * 0.12;
    for (std::size_t i = 0; i < dry.size(); ++i) {
        const double x = static_cast<double>(dry[i]) * drive + bias;
        double shaped = std::tanh(x) / std::max(normalizer, 1.0e-9);
        if (plugin.variant % 4 == 1) shaped = std::atan(x) / std::atan(drive);
        if (plugin.variant % 4 == 2) {
            const double folded = std::remainder(x, 4.0);
            shaped = std::sin(folded * 1.12) / std::max(0.5, std::sin(drive * 1.12));
        }
        wet[i] = static_cast<float>(shaped);
    }
}

void processExciter(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                    const QuantumAudioParameters& p, const std::vector<float>& dry,
                    std::vector<float>& wet) {
    const double alpha = onePoleCoefficient(1500.0 + p.tone * 8500.0, audio.sampleRate);
    std::vector<double> low(static_cast<std::size_t>(audio.channels), 0.0);
    double envelope = 0.0;
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        double magnitude = 0.0;
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            low[static_cast<std::size_t>(ch)] += alpha * (static_cast<double>(dry[index]) - low[static_cast<std::size_t>(ch)]);
            magnitude += std::abs(static_cast<double>(dry[index]));
        }
        envelope += onePoleCoefficient(15.0, audio.sampleRate) * (magnitude / audio.channels - envelope);
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const double high = static_cast<double>(dry[index]) - low[static_cast<std::size_t>(ch)];
            const double threshold = plugin.variant == 3 ? 0.28 : 0.0;
            const double gate = envelope > threshold ? 1.0 : 0.35;
            wet[index] = static_cast<float>(static_cast<double>(dry[index]) + high * p.amount * gate * 0.9);
        }
    }
}

void processModulator(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                      const QuantumAudioParameters& p, const std::vector<float>& dry,
                      std::vector<float>& wet) {
    const double rateHz = 0.08 + p.rate * 7.2;
    const double maximumDelay = static_cast<double>(audio.sampleRate) * (0.001 + p.amount * 0.014);
    const double depth = plugin.variant == 5 ? p.amount * 0.18 : p.amount * 0.72;
    const double phase = static_cast<double>(plugin.variant) * 0.73;
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        const double seconds = static_cast<double>(frame) / audio.sampleRate;
        double modulation = 0.5 + 0.5 * std::sin(2.0 * kPi * rateHz * seconds + phase);
        if (plugin.variant == 4 || plugin.variant == 7)
            modulation = std::clamp(modulation + (std::sin(seconds * 17.0 + phase) * 0.12), 0.0, 1.0);
        const double delay = 1.0 + maximumDelay * modulation;
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const float shifted = interpolate(dry, audio.frames(), audio.channels,
                static_cast<double>(frame) - delay, ch);
            wet[index] = static_cast<float>(static_cast<double>(dry[index]) * (1.0 - depth)
                                           + static_cast<double>(shifted) * depth);
        }
    }
}

void processDelay(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                  const QuantumAudioParameters& p, const std::vector<float>& dry,
                  std::vector<float>& wet) {
    const auto delayFrames = std::max<std::size_t>(1u, static_cast<std::size_t>(
        std::llround(audio.sampleRate * (0.035 + p.rate * 0.52))));
    const double feedback = 0.06 + p.amount * 0.68;
    const double secondTap = (plugin.variant % 2 == 0) ? 0.24 : 0.0;
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const double previous = frame >= delayFrames
                ? static_cast<double>(wet[(frame - delayFrames) * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch)])
                : 0.0;
            const double earlier = frame >= delayFrames * 2u
                ? static_cast<double>(wet[(frame - delayFrames * 2u) * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch)])
                : 0.0;
            wet[index] = static_cast<float>(static_cast<double>(dry[index]) + previous * feedback
                                           + earlier * feedback * secondTap);
        }
    }
}

void processReverb(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                   const QuantumAudioParameters& p, const std::vector<float>& dry,
                   std::vector<float>& wet) {
    const std::array<double, 4> milliseconds{19.1, 27.7, 34.9, 43.3};
    std::array<std::vector<float>, 4> lines;
    std::array<std::size_t, 4> lengths{};
    std::array<std::size_t, 4> cursors{};
    for (std::size_t i = 0; i < lines.size(); ++i) {
        lengths[i] = std::max<std::size_t>(2u, static_cast<std::size_t>(
            audio.sampleRate * milliseconds[i] / 1000.0));
        lines[i].assign(lengths[i] * static_cast<std::size_t>(audio.channels), 0.0f);
    }
    const double feedback = 0.18 + p.amount * (plugin.variant == 1 ? 0.66 : 0.56);
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            double reflections = 0.0;
            for (std::size_t line = 0; line < lines.size(); ++line) {
                const auto lineIndex = cursors[line] * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
                const double delayed = lines[line][lineIndex];
                const double polarity = ((plugin.variant + static_cast<int>(line)) % 3 == 0) ? -1.0 : 1.0;
                lines[line][lineIndex] = static_cast<float>(static_cast<double>(dry[index]) * 0.22
                    + delayed * feedback * polarity);
                reflections += delayed * (line % 2u == 0u ? 0.28 : 0.22);
            }
            wet[index] = static_cast<float>(static_cast<double>(dry[index]) * 0.68 + reflections * 0.72);
        }
        for (std::size_t line = 0; line < cursors.size(); ++line)
            cursors[line] = (cursors[line] + 1u) % lengths[line];
    }
}

void processGate(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                 const QuantumAudioParameters& p, const std::vector<float>& dry,
                 std::vector<float>& wet) {
    double envelope = 0.0;
    const double attack = onePoleCoefficient(30.0 + p.rate * 80.0, audio.sampleRate);
    const double release = onePoleCoefficient(4.0 + (1.0 - p.rate) * 28.0, audio.sampleRate);
    const double threshold = 0.012 + p.amount * 0.22;
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        double level = 0.0;
        for (int ch = 0; ch < audio.channels; ++ch)
            level += std::abs(static_cast<double>(dry[frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch)]));
        level /= audio.channels;
        const double a = level > envelope ? attack : release;
        envelope += a * (level - envelope);
        double gain = 1.0;
        if (envelope < threshold) {
            const double ratio = std::clamp(envelope / threshold, 0.0, 1.0);
            gain = plugin.variant == 2 ? ratio * ratio * ratio : std::pow(ratio, 1.7);
            if (plugin.variant == 0) gain = std::max(0.08, gain);
        }
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            wet[index] = static_cast<float>(static_cast<double>(dry[index]) * gain);
        }
    }
}

void processTransient(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                      const QuantumAudioParameters& p, const std::vector<float>& dry,
                      std::vector<float>& wet) {
    double fast = 0.0, slow = 0.0;
    const double fastAttack = onePoleCoefficient(70.0 + p.rate * 150.0, audio.sampleRate);
    const double slowAttack = onePoleCoefficient(5.0 + (1.0 - p.rate) * 15.0, audio.sampleRate);
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        double magnitude = 0.0;
        for (int ch = 0; ch < audio.channels; ++ch)
            magnitude += std::abs(static_cast<double>(dry[frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch)]));
        magnitude /= audio.channels;
        fast += (magnitude > fast ? fastAttack : slowAttack) * (magnitude - fast);
        slow += slowAttack * (magnitude - slow);
        const double contrast = std::clamp((fast - slow) * (plugin.variant == 7 ? 8.0 : 5.0), -0.7, 0.7);
        const double emphasis = (plugin.variant % 2 == 0 ? 1.0 : -0.48) * p.amount * 2.4;
        const double gain = 1.0 + contrast * emphasis;
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            wet[index] = static_cast<float>(static_cast<double>(dry[index]) * gain);
        }
    }
}

std::uint32_t hash32(std::uint32_t value) {
    value ^= value >> 16u;
    value *= 0x7feb352du;
    value ^= value >> 15u;
    value *= 0x846ca68bu;
    value ^= value >> 16u;
    return value;
}

void processGranular(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                     const QuantumAudioParameters& p, const std::vector<float>& dry,
                     std::vector<float>& wet) {
    const auto grain = std::max<std::size_t>(8u, static_cast<std::size_t>(
        audio.sampleRate * (0.006 + p.tone * 0.09)));
    const int direction = plugin.variant % 2 == 0 ? 1 : -1;
    const double depth = p.amount * (plugin.variant == 2 ? 0.94 : 0.72);
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        const std::size_t grainIndex = frame / grain;
        const std::size_t local = frame % grain;
        const std::uint32_t seed = hash32(static_cast<std::uint32_t>(grainIndex)
            ^ static_cast<std::uint32_t>(plugin.variant * 7919 + 17));
        const auto offset = static_cast<std::size_t>(seed) % std::max<std::size_t>(1u, grain / 3u);
        const std::size_t start = grainIndex * grain;
        std::size_t sourceLocal = (local + offset) % grain;
        if (direction < 0) sourceLocal = grain - 1u - sourceLocal;
        const std::size_t sourceFrame = std::min(start + sourceLocal, audio.frames() - 1u);
        const double phase = static_cast<double>(local) / static_cast<double>(grain);
        const double window = 0.72 + 0.28 * std::sin(kPi * phase);
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const auto sourceIndex = sourceFrame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const double grainSample = dry[sourceIndex] * window;
            wet[index] = static_cast<float>(static_cast<double>(dry[index]) * (1.0 - depth)
                                           + grainSample * depth);
        }
    }
}

void processResonator(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                      const QuantumAudioParameters& p, const std::vector<float>& dry,
                      std::vector<float>& wet) {
    const double frequency = 55.0 + p.tone * 900.0;
    const auto delayFrames = std::max<std::size_t>(2u, static_cast<std::size_t>(
        std::clamp(static_cast<double>(audio.sampleRate) / frequency, 2.0, 5000.0)));
    const double feedback = std::min(0.88, 0.12 + p.amount * 0.72);
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const double delayed = frame >= delayFrames
                ? static_cast<double>(wet[(frame - delayFrames) * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch)])
                : 0.0;
            const double polarity = plugin.variant % 2 == 0 ? 1.0 : -1.0;
            wet[index] = static_cast<float>(static_cast<double>(dry[index]) + delayed * feedback * polarity);
        }
    }
}

void processQuantizer(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                      const QuantumAudioParameters& p, const std::vector<float>& dry,
                      std::vector<float>& wet) {
    const int bits = std::clamp(static_cast<int>(std::lround(16.0 - p.amount * 12.0)), 3, 16);
    const double levels = std::pow(2.0, static_cast<double>(bits - 1));
    const std::size_t hold = plugin.variant % 2 == 0 ? 1u
        : std::max<std::size_t>(1u, static_cast<std::size_t>(1.0 + p.amount * 12.0));
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        const auto heldFrame = frame - (frame % hold);
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const auto source = heldFrame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const double x = dry[source];
            wet[index] = static_cast<float>(std::round(x * levels) / levels);
        }
    }
}

void processSubGenerator(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                         const QuantumAudioParameters& p, const std::vector<float>& dry,
                         std::vector<float>& wet) {
    const double detectorAlpha = onePoleCoefficient(90.0 + p.tone * 90.0, audio.sampleRate);
    const double frequency = 28.0 + p.tone * 52.0 + static_cast<double>(plugin.variant % 3) * 4.0;
    const double phaseStep = 2.0 * kPi * frequency / audio.sampleRate;
    double detector = 0.0, phase = 0.0;
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        double mono = 0.0;
        for (int ch = 0; ch < audio.channels; ++ch)
            mono += dry[frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch)];
        mono /= audio.channels;
        detector += detectorAlpha * (mono - detector);
        phase += phaseStep;
        if (phase >= 2.0 * kPi) phase -= 2.0 * kPi;
        const double oscillator = std::sin(phase) * std::abs(detector);
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const double centering = 1.0;
            wet[index] = static_cast<float>(static_cast<double>(dry[index])
                + oscillator * p.amount * 0.95 * centering);
        }
    }
}

void processNoise(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                  const QuantumAudioParameters& p, const std::vector<float>& dry,
                  std::vector<float>& wet) {
    std::uint32_t state = 0x9e3779b9u ^ static_cast<std::uint32_t>(plugin.variant * 7919 + 1);
    double colour = 0.0, envelope = 0.0;
    const double colourAlpha = onePoleCoefficient(1200.0 + p.tone * 10000.0, audio.sampleRate);
    const double envAlpha = onePoleCoefficient(12.0, audio.sampleRate);
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        double mag = 0.0;
        for (int ch = 0; ch < audio.channels; ++ch)
            mag += std::abs(static_cast<double>(dry[frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch)]));
        envelope += envAlpha * (mag / audio.channels - envelope);
        state ^= state << 13u; state ^= state >> 17u; state ^= state << 5u;
        const double white = static_cast<double>(state & 0xffffu) / 32767.5 - 1.0;
        colour += colourAlpha * (white - colour);
        const double noise = plugin.variant == 0 ? colour : (plugin.variant == 1 ? white : 0.65 * colour + 0.35 * white);
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const double stereoSign = (ch == 0 || audio.channels == 1) ? 1.0 : -1.0;
            wet[index] = static_cast<float>(static_cast<double>(dry[index])
                + noise * envelope * p.amount * 0.075 * stereoSign);
        }
    }
}

void processSpectrum(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                     const QuantumAudioParameters& p, const std::vector<float>& dry,
                     std::vector<float>& wet) {
    const double aLow = onePoleCoefficient(130.0 + p.tone * 260.0, audio.sampleRate);
    const double aHigh = onePoleCoefficient(1800.0 + p.tone * 10000.0, audio.sampleRate);
    const int mode = plugin.variant % 4;
    const double lowGain = mode == 3 ? 1.0 + p.amount * 0.9 : 1.0 - p.amount * 0.2;
    const double midGain = mode == 2 ? 1.0 - p.amount * 0.65 : 1.0 + p.amount * 0.22;
    const double highGain = mode == 1 ? 1.0 + p.amount * 1.1 : 1.0 + p.amount * 0.45;
    std::vector<double> lo(static_cast<std::size_t>(audio.channels), 0.0);
    std::vector<double> hi(static_cast<std::size_t>(audio.channels), 0.0);
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto channel = static_cast<std::size_t>(ch);
            const auto index = frame * static_cast<std::size_t>(audio.channels) + channel;
            const double x = dry[index];
            lo[channel] += aLow * (x - lo[channel]);
            hi[channel] += aHigh * (x - hi[channel]);
            wet[index] = static_cast<float>(lo[channel] * lowGain
                + (hi[channel] - lo[channel]) * midGain
                + (x - hi[channel]) * highGain);
        }
    }
}

void processTime(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                 const QuantumAudioParameters& p, const std::vector<float>& dry,
                 std::vector<float>& wet) {
    const double speed = 0.88 + p.amount * 0.24;
    const double modulation = 0.001 + p.rate * 0.016;
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        const double base = static_cast<double>(frame) * speed;
        const double wobble = std::sin(2.0 * kPi * (0.1 + p.rate * 1.5)
                                      * static_cast<double>(frame) / audio.sampleRate) * modulation * audio.sampleRate;
        double read = base + wobble;
        if (plugin.variant == 1) read = static_cast<double>(frame) - std::abs(wobble);
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const float shifted = interpolate(dry, audio.frames(), audio.channels, read, ch);
            wet[index] = static_cast<float>(static_cast<double>(dry[index]) * (1.0 - p.amount * 0.5)
                                           + static_cast<double>(shifted) * p.amount * 0.5);
        }
    }
}

void processVocoder(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                    const QuantumAudioParameters& p, const std::vector<float>& dry,
                    std::vector<float>& wet) {
    const double a1 = onePoleCoefficient(250.0, audio.sampleRate);
    const double a2 = onePoleCoefficient(950.0, audio.sampleRate);
    const double a3 = onePoleCoefficient(3600.0, audio.sampleRate);
    double low = 0.0, mid = 0.0, high = 0.0;
    double phase1 = 0.0, phase2 = 0.0, phase3 = 0.0;
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        double mono = 0.0;
        for (int ch = 0; ch < audio.channels; ++ch)
            mono += dry[frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch)];
        mono /= audio.channels;
        low += a1 * (mono - low);
        mid += a2 * (mono - mid);
        high += a3 * (mono - high);
        const double e1 = std::abs(low);
        const double e2 = std::abs(mid - low);
        const double e3 = std::abs(mono - high);
        phase1 += 2.0 * kPi * (110.0 + p.tone * 120.0) / audio.sampleRate;
        phase2 += 2.0 * kPi * (420.0 + p.tone * 300.0) / audio.sampleRate;
        phase3 += 2.0 * kPi * (1450.0 + p.tone * 800.0) / audio.sampleRate;
        const double carrier = std::sin(phase1) * e1 + std::sin(phase2) * e2 + std::sin(phase3) * e3;
        const double blend = p.amount * (plugin.variant == 2 ? 0.54 : 0.4);
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            wet[index] = static_cast<float>(static_cast<double>(dry[index]) * (1.0 - blend)
                + carrier * blend);
        }
    }
}

void processSynth(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                  const QuantumAudioParameters& p, const std::vector<float>& dry,
                  std::vector<float>& wet) {
    double envelope = 0.0, phase = 0.0;
    const double envAlpha = onePoleCoefficient(10.0 + p.rate * 35.0, audio.sampleRate);
    const double frequency = 42.0 + p.tone * 620.0;
    const double step = 2.0 * kPi * frequency / audio.sampleRate;
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        double mono = 0.0;
        for (int ch = 0; ch < audio.channels; ++ch)
            mono += dry[frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch)];
        mono /= audio.channels;
        envelope += envAlpha * (std::abs(mono) - envelope);
        phase += step;
        if (phase > 2.0 * kPi) phase -= 2.0 * kPi;
        double oscillator = std::sin(phase);
        if (plugin.variant == 1) oscillator = (phase / kPi) - 1.0;
        else if (plugin.variant == 2) oscillator = std::sin(phase) * 0.7 + std::sin(phase * 2.0) * 0.3;
        else if (plugin.variant == 3) oscillator = std::sin(phase) * (0.65 + 0.35 * std::sin(phase * 0.5));
        const double layer = oscillator * envelope * p.amount * 0.42;
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const double pan = audio.channels == 2 && ch == 0 ? 0.96 : 1.0;
            wet[index] = static_cast<float>(static_cast<double>(dry[index]) + layer * pan);
        }
    }
}

void processMorph(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                  const QuantumAudioParameters& p, const std::vector<float>& dry,
                  std::vector<float>& wet) {
    const double drive = 1.0 + p.amount * (2.0 + static_cast<double>(plugin.variant % 5));
    const double norm = std::tanh(drive);
    const double alpha = onePoleCoefficient(500.0 + p.tone * 5000.0, audio.sampleRate);
    std::vector<double> low(static_cast<std::size_t>(audio.channels), 0.0);
    double envelope = 0.0;
    for (std::size_t frame = 0; frame < audio.frames(); ++frame) {
        double mag = 0.0;
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto channel = static_cast<std::size_t>(ch);
            const auto index = frame * static_cast<std::size_t>(audio.channels) + channel;
            low[channel] += alpha * (static_cast<double>(dry[index]) - low[channel]);
            mag += std::abs(static_cast<double>(dry[index]));
        }
        envelope += onePoleCoefficient(8.0, audio.sampleRate) * (mag / audio.channels - envelope);
        const double comp = envelope > (0.7 - p.amount * 0.35)
            ? std::pow((0.7 - p.amount * 0.35) / envelope, 0.25 + p.amount * 0.5) : 1.0;
        for (int ch = 0; ch < audio.channels; ++ch) {
            const auto index = frame * static_cast<std::size_t>(audio.channels) + static_cast<std::size_t>(ch);
            const double x = static_cast<double>(dry[index]);
            const double shaped = std::tanh(x * drive) / std::max(norm, 1.0e-9);
            const double bright = x + (x - low[static_cast<std::size_t>(ch)]) * p.tone * 0.32;
            wet[index] = static_cast<float>((shaped * 0.6 + bright * 0.4) * comp);
        }
    }
}

void applyModel(const AudioBuffer& audio, const QuantumAudioPlugin& plugin,
                const QuantumAudioParameters& p, const std::vector<float>& dry,
                std::vector<float>& wet) {
    switch (plugin.model) {
    case QuantumDspModel::StereoField: processStereoField(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Dynamics: processDynamics(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Equalizer: processEqualizer(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Saturation: processSaturation(plugin, p, dry, wet); break;
    case QuantumDspModel::Exciter: processExciter(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Modulator: processModulator(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Delay: processDelay(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Reverb: processReverb(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Gate: processGate(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Transient: processTransient(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Granular: processGranular(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Resonator: processResonator(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Quantizer: processQuantizer(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::SubGenerator: processSubGenerator(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Noise: processNoise(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Spectrum: processSpectrum(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Time: processTime(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Vocoder: processVocoder(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Synth: processSynth(audio, plugin, p, dry, wet); break;
    case QuantumDspModel::Analyzer: wet = dry; break;
    case QuantumDspModel::Morph: processMorph(audio, plugin, p, dry, wet); break;
    }
}
} // namespace

const std::vector<QuantumAudioPlugin>& quantumAudioPlugins() noexcept {
    return kPlugins;
}

std::string_view quantumAudioCategoryName(QuantumAudioCategory category) noexcept {
    switch (category) {
    case QuantumAudioCategory::Quarks: return "Quarks & Hadron";
    case QuantumAudioCategory::Quantum: return "Quantum Mechanics";
    case QuantumAudioCategory::Atomic: return "Atoms & Orbitals";
    case QuantumAudioCategory::Fields: return "Fields & Relativity";
    case QuantumAudioCategory::Experimental: return "Experimental Instruments";
    }
    return "Unknown";
}

std::string_view quantumDspModelName(QuantumDspModel model) noexcept {
    switch (model) {
    case QuantumDspModel::StereoField: return "Stereo field";
    case QuantumDspModel::Dynamics: return "Dynamics";
    case QuantumDspModel::Equalizer: return "Equalizer";
    case QuantumDspModel::Saturation: return "Saturation";
    case QuantumDspModel::Exciter: return "Harmonic exciter";
    case QuantumDspModel::Modulator: return "Pitch / phase modulation";
    case QuantumDspModel::Delay: return "Delay";
    case QuantumDspModel::Reverb: return "Reverb";
    case QuantumDspModel::Gate: return "Gate";
    case QuantumDspModel::Transient: return "Transient shaper";
    case QuantumDspModel::Granular: return "Granular processor";
    case QuantumDspModel::Resonator: return "Resonator";
    case QuantumDspModel::Quantizer: return "Time / amplitude quantizer";
    case QuantumDspModel::SubGenerator: return "Sub generator";
    case QuantumDspModel::Noise: return "Noise generator";
    case QuantumDspModel::Spectrum: return "Multiband spectrum";
    case QuantumDspModel::Time: return "Time processor";
    case QuantumDspModel::Vocoder: return "Formant / vocoder";
    case QuantumDspModel::Synth: return "Signal-driven synthesizer";
    case QuantumDspModel::Analyzer: return "Signal analyzer";
    case QuantumDspModel::Morph: return "Hybrid morph processor";
    }
    return "Unknown";
}

const QuantumAudioPlugin* findQuantumAudioPlugin(std::string_view idOrName) noexcept {
    const auto found = std::find_if(kPlugins.begin(), kPlugins.end(), [&](const auto& plugin) {
        return equalFold(plugin.id, idOrName) || equalFold(plugin.name, idOrName);
    });
    return found == kPlugins.end() ? nullptr : &*found;
}

QuantumAudioProcessReport processQuantumAudio(std::string_view pluginId, AudioBuffer& audio,
                                               const QuantumAudioParameters& parameters) {
    QuantumAudioProcessReport report;
    const auto* plugin = findQuantumAudioPlugin(pluginId);
    if (plugin == nullptr) {
        report.message = "Unknown Quantum Audio module: " + std::string(pluginId);
        return report;
    }
    if (audio.channels != 1 && audio.channels != 2) {
        report.message = "DSP input must be mono or stereo.";
        return report;
    }
    if (audio.sampleRate < 8000 || audio.sampleRate > 192000 || audio.samples.empty()
        || audio.samples.size() % static_cast<std::size_t>(audio.channels) != 0u) {
        report.message = "Invalid sample rate or interleaved audio buffer.";
        return report;
    }
    const auto frames = audio.samples.size() / static_cast<std::size_t>(audio.channels);
    if (frames == 0u || frames > kMaxFrames) {
        report.message = "Audio buffer is outside the supported frame limit.";
        return report;
    }
    const std::array<double, 4> controls{parameters.amount, parameters.tone,
                                         parameters.rate, parameters.mix};
    if (!std::all_of(controls.begin(), controls.end(), [](double value) {
        return std::isfinite(value) && value >= 0.0 && value <= 1.0;
    })) {
        report.message = "DSP controls must be finite values in the 0..1 range.";
        return report;
    }
    if (!std::all_of(audio.samples.begin(), audio.samples.end(), [](float value) {
        return std::isfinite(value);
    })) {
        report.message = "Audio buffer contains NaN or infinite samples.";
        return report;
    }

    report.before = measure(audio);
    if (parameters.mix == 0.0 || plugin->model == QuantumDspModel::Analyzer) {
        report.success = true;
        report.after = report.before;
        report.message = plugin->model == QuantumDspModel::Analyzer
            ? "Analysis complete; signal was not modified."
            : "Dry mix selected; signal was not modified.";
        return report;
    }

    const std::vector<float> dry = audio.samples;
    std::vector<float> wet = dry;
    applyModel(audio, *plugin, parameters, dry, wet);
    if (wet.size() != dry.size()) {
        report.message = "DSP processor returned an invalid buffer length.";
        return report;
    }
    for (std::size_t i = 0; i < wet.size(); ++i) {
        const double value = static_cast<double>(dry[i]) * (1.0 - parameters.mix)
                           + static_cast<double>(wet[i]) * parameters.mix;
        if (!std::isfinite(value)) {
            report.message = "DSP processor produced a non-finite sample; input was left unchanged.";
            return report;
        }
        audio.samples[i] = static_cast<float>(std::clamp(value, -8.0, 8.0));
    }
    report.after = measure(audio);
    report.success = true;
    report.message = "Processed with " + std::string(plugin->name) + " (" +
        std::string(quantumDspModelName(plugin->model)) + ").";
    return report;
}

} // namespace ventu
