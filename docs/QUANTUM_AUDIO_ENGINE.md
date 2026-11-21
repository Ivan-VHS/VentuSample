# Quantum Audio Engine 133

**Нативная DSP-подсистема VentuSample** · C++20 · пять семейств · 133 модуля · 21 алгоритмическая модель.

Quantum Audio Engine добавляет отдельную офлайн-лабораторию обработки аудио. Это не расширение каталога 18 системных инструментов VentuSample и не загрузчик сторонних DLL: 133 аудиомодуля регистрируются в собственном типизированном каталоге и используют проверяемые реализации DSP.

## Запуск

Открой меню **Плагины → Quantum Audio Engine — 133 DSP**. Загрузите поддерживаемый WAV, выберите модуль, настройте его параметры, добавьте модуль в цепочку, нажмите **Обработать цепочку** и экспортируйте результат в WAV PCM16.

В каталоге есть фильтр пяти семейств. Справа отображаются назначение, применяемая DSP-модель и концептуальная визуальная метафора; ниже находятся параметры выбранного модуля. Цепочка выполняется последовательно и сохраняет порядок. Она и значения модулей запоминаются локально через QSettings.

## Аудиоконтракт

- Вход: WAV PCM16, mono/stereo, 8–192 kHz (в пределах валидатора генератора).
- Внутренний формат: interleaved float samples.
- Выход: stereo PCM16 WAV; mono автоматически дублируется в L/R.
- Параметры: Amount, Tone, Rate и Wet/Dry в диапазоне 0…1.
- Dry mix 0% — точный bypass без изменения исходных samples.
- DSP проверяет число каналов, частоту дискретизации, размер буфера, конечность samples и параметров.
- Непредусмотренный NaN/Inf приводит к отказу до изменения входного буфера.
- Внутренний предохранительный предел результата: ±8.0 float; это не мастер-лимитер и не нормализация.

## Семейства и каталог

### I. Кварки, адроны и сильное взаимодействие (1–23)

- **QUARK-CONFINER** — `quark-confiner` · StereoField. Locks the low-frequency image to mono while retaining stereo detail above the crossover. Визуальная метафора: Three luminous nodes held inside a taut force triangle.
- **GLUON-GLUE** — `gluon-glue` · Dynamics. Linked-bus compressor that makes separate layers move as one controlled cluster. Визуальная метафора: Coloured field tubes converge on one pulsing core.
- **COLOR-CHARGE EQ** — `color-charge-eq` · Equalizer. Three-band low, mid and high balance with a neutral summed reference. Визуальная метафора: A tri-colour spectral prism with a central balance point.
- **UP-DOWN HARMONIZER** — `up-down-harmonizer` · Modulator. Micro-pitch movement using a short, modulated dual-head delay approximation. Визуальная метафора: Opposing pitch vectors with fractional interval markings.
- **STRANGE-QUARK** — `strange-quark` · Saturation. Nonlinear phase-coloured saturation that emphasizes upper partials. Визуальная метафора: A violet unstable sphere shedding harmonic sparks.
- **CHARM-EXCITER** — `charm-exciter` · Exciter. Adds controlled upper-harmonic definition with a restrained high-pass residual. Визуальная метафора: A gold and turquoise crystal trail over the spectrum.
- **TOP-QUARK MASS** — `top-quark-mass` · Transient. Adds impact weight by increasing attack contrast and low-body perception. Визуальная метафора: A dense dark sphere bending the coordinate grid.
- **BOTTOM-QUARK DECAY** — `bottom-quark-decay` · Delay. Tempo-independent multi-tap decay with controlled feedback. Визуальная метафора: A heavy line branches into a fan of fine reflections.
- **HADRON-COLLIDER** — `hadron-collider` · Morph. Combines nonlinear collision, phase contrast and transient emphasis. Визуальная метафора: Two opposing beams flash at a circular detector.
- **BARYON-TRIO** — `baryon-trio` · Spectrum. Three-band saturation with linked gain and crossover protection. Визуальная метафора: Three orbital rings around one barycentre.
- **MESON-BRIDGE** — `meson-bridge` · Resonator. Short resonant echo that couples rhythmic attacks and low-frequency body. Визуальная метафора: A pulsing bridge between two orbiting nodes.
- **ASYMPTOTIC-FREE** — `asymptotic-free` · Dynamics. Keeps low-level material open and progressively restrains large peaks. Визуальная метафора: A free green region meets a red containment wall.
- **PENTAQUARK-CHORD** — `pentaquark-chord` · Vocoder. Adds a five-part harmonic colour from the signal envelope and selected formant bands. Визуальная метафора: A five-point star whose nodes respond to voice energy.
- **JET-FRAGMENTATION** — `jet-fragmentation` · Granular. Turns sharp attacks into deterministic short grain fragments. Визуальная метафора: A particle jet fans out from one transient point.
- **SEA-QUARK NOISE** — `sea-quark-noise` · Noise. Adds low-level, envelope-following stochastic texture in pauses and tails. Визуальная метафора: A dark field of rapidly appearing and fading sparks.
- **VALENCE-CORE** — `valence-core` · Spectrum. Highlights three dominant spectral regions and suppresses surrounding clutter. Визуальная метафора: Three bright spectral columns over a subdued floor.
- **STRING-TENSION** — `string-tension` · StereoField. Links stereo width to harmonic drive so wider material becomes denser, not louder. Визуальная метафора: An elastic line between left and right channels.
- **BAG-MODEL LIMITER** — `bag-model-limiter` · Dynamics. Soft containment stage that rounds peaks without hard digital clipping. Визуальная метафора: A flexible membrane deforms around incoming peaks.
- **FLAVOR-OSCILLATOR** — `flavor-oscillator` · Morph. Slowly changes filter and saturation colour using a cyclic modulation path. Визуальная метафора: A six-sector colour wheel with gradual transitions.
- **HYPERON-SUB** — `hyperon-sub` · SubGenerator. Adds a short, phase-stable sub-octave foundation from the incoming low-frequency envelope. Визуальная метафора: A deep blue funnel with a half-life decay marker.
- **PION-PUNCH** — `pion-punch` · Transient. Fast attack emphasis with a bounded transient envelope. Визуальная метафора: A white impulse needle above a rounded waveform body.
- **GLUEBALL-REVERB** — `glueball-reverb` · Reverb. Dense reflective reverb with short, metallic feedback paths. Визуальная метафора: A faceted knot of interwoven luminous loops.
- **LATTICE-QUANTIZER** — `lattice-quantizer` · Quantizer. Quantizes amplitude and local sample timing to a controllable lattice. Визуальная метафора: A cubic grid whose nodes flash on sample events.

### II. Квантовая механика и запутанность (24–46)

- **SUPERPOSITION-SYNTH** — `superposition-synth` · Synth. Blends several harmonic states into one evolving oscillator texture. Визуальная метафора: Ghost waveforms collapse into one clear trace.
- **SCHRÖDINGER-GATE** — `schrodinger-gate` · Gate. Probabilistic envelope gate with a deterministic threshold trigger. Визуальная метафора: A translucent state box with a 50/50 probability dial.
- **ENTANGLEMENT-STEREO** — `entanglement-stereo` · StereoField. Couples left and right movement through a linked mid-side modulation law. Визуальная метафора: Two linked spirals mirror each other's movement.
- **WAVEFUNCTION-COLLAPSE** — `wavefunction-collapse` · Transient. Compresses a diffuse envelope into a sharp, transient-focused shape. Визуальная метафора: A wide probability cloud contracts into one vertical line.
- **HEISENBERG-BLUR** — `heisenberg-blur` · Modulator. Trades fine timing definition for modulated pitch and phase movement. Визуальная метафора: An uncertainty ellipse spans time and frequency axes.
- **TUNNELING-FILTER** — `tunneling-filter` · Equalizer. Lets a controlled amount of energy pass a steep spectral boundary. Визуальная метафора: Wave packets leak through a closed filter wall.
- **QUBIT-SEQUENCER** — `qubit-sequencer` · Granular. Creates a stepped, state-driven sample pattern with repeatable probability. Визуальная метафора: A row of state spheres replaces conventional step buttons.
- **DECOHERENCE-DISSOLVE** — `decoherence-dissolve` · Reverb. Gradually de-correlates transients into a diffuse reflective tail. Визуальная метафора: A clear sine wave dissolves into pixel-like particles.
- **EIGENSTATE-RESONATOR** — `eigenstate-resonator` · Resonator. Emphasizes stable resonant bands with a tuned feedback comb. Визуальная метафора: Narrow laser lines stand at resonant peaks.
- **PLANCK-CRUSHER** — `planck-crusher` · Quantizer. Reduces amplitude resolution and sample update rate for microscopic texture. Визуальная метафора: A fine staircase grid at extreme magnification.
- **PROBABILITY-CLOUD DELAY** — `probability-cloud-delay` · Delay. Adds repeat timing offsets inside a bounded deterministic timing cloud. Визуальная метафора: Soft Gaussian repeat markers hover around a time grid.
- **BELL-STATE PANNER** — `bell-state-panner` · StereoField. Creates a four-state stereo trajectory while protecting the central image. Визуальная метафора: A four-lobed phase diagram in polar coordinates.
- **QUANTUM-ZENO FREEZE** — `quantum-zeno-freeze` · Granular. Captures short slices and sustains them with a controllable observation rate. Визуальная метафора: A strobe beam holds a waveform in one position.
- **SPIN-UP / SPIN-DOWN** — `spin-up-spin-down` · StereoField. Rotates stereo phase clockwise or counter-clockwise at a tempo-like rate. Визуальная метафора: A gyroscopic sphere carries an up/down spin vector.
- **PAULI-EXCLUSION EQ** — `pauli-exclusion-eq` · Spectrum. Makes complementary spectral slots for a less congested two-layer balance. Визуальная метафора: A checkerboard spectral matrix rejects occupied cells.
- **FEYNMAN-PATH REVERB** — `feynman-path-reverb` · Reverb. Accumulates multiple short reflection paths into a dense but bounded tail. Визуальная метафора: A web of reflected paths links source and listener.
- **ZERO-POINT SATURATOR** — `zero-point-saturator` · Saturation. Raises low-level harmonic detail through a restrained nonlinear floor. Визуальная метафора: A quiet baseline vibrates with fine golden micro-waves.
- **WAVE-PARTICLE MORPHER** — `wave-particle-morpher` · Morph. Morphs between smooth modulation and stepped granular pulses. Визуальная метафора: A sine trace transforms into a stream of bright spheres.
- **TELEPORT-SEND** — `teleport-send` · Delay. A phase-aware short routing delay for transferring a signal between offline stages. Визуальная метафора: A portal ring shows paired timing and phase states.
- **HAMILTONIAN-MOD** — `hamiltonian-mod` · Dynamics. Uses signal energy to drive a smooth amplitude and filter movement. Визуальная метафора: A two-colour energy pendulum moves between states.
- **BLOCH-SPHERE CHORUS** — `bloch-sphere-chorus` · Modulator. A multi-voice chorus with bounded phase paths and shared stereo motion. Визуальная метафора: A transparent globe carries points along its meridians.
- **OBSERVER-EFFECT** — `observer-effect` · Morph. Adds repeatable motion and nonlinear colour; it never depends on the desktop cursor or window focus. Визуальная метафора: A central iris reacts to the evolving spectral trace.
- **MULTIVERSE-BOUNCE** — `multiverse-bounce` · Dynamics. Creates four parallel gain and tone branches for a controllable parallel blend. Визуальная метафора: A branching tree of four channels with a branch selector.

### III. Атомы, орбитали и ядро (47–69)

- **ORBITAL-HARMONICS (s, p, d, f)** — `orbital-harmonics` · Exciter. Shapes harmonic density and stereo motion using four orbital-style modes. Визуальная метафора: Rotating volumetric s, p, d and f orbital clouds.
- **ISOTOPE-DETUNE** — `isotope-detune` · Modulator. Adds small, bounded detuning offsets with stable modulation depth. Визуальная метафора: A nuclide map highlights stable and drifting lanes.
- **HALF-LIFE DECAY** — `half-life-decay` · Dynamics. Applies a measured exponential decay to the signal envelope and high-frequency detail. Визуальная метафора: An exponential decay curve is marked with T-half.
- **FISSION-SPLITTER** — `fission-splitter` · Spectrum. Splits the spectrum into two complementary bands and emphasizes the split transient. Визуальная метафора: A heavy nucleus separates into two energetic fragments.
- **FUSION-COMPRESSOR** — `fusion-compressor` · Dynamics. Links envelope compression and soft saturation for a dense combined signal. Визуальная метафора: Two spheres merge into one bright compressed core.
- **VALENCE-RING** — `valence-ring` · Saturation. Pairs saturation amount with stereo width in one macro-style control. Визуальная метафора: Concentric orbitals show one to eight valence points.
- **SPECTRAL-LINES (BALMER)** — `spectral-lines-balmer` · Spectrum. Emphasizes selected narrow spectral regions using a multi-band tone model. Визуальная метафора: A dark spectral strip carries bright emission lines.
- **IONIZATION-DRIVE** — `ionization-drive` · Saturation. A staged nonlinear drive that increases upper harmonic density with intensity. Визуальная метафора: Ionization stages discharge as the level rises.
- **COVALENT-SIDECHAIN** — `covalent-sidechain` · Dynamics. An envelope-linked sidechain-style processor for complementary layer control. Визуальная метафора: Two overlapping orbitals share a glowing centre.
- **NUCLEAR-FORCE SUB** — `nuclear-force-sub` · SubGenerator. Reinforces a narrow 30–60 Hz foundation without widening the low end. Визуальная метафора: A dense pulsing core has a sharply bounded force field.
- **PHOTON-EMISSION** — `photon-emission` · Exciter. Adds short, high-frequency harmonic flashes around detected transients. Визуальная метафора: Wave packets emit as short light-like bursts.
- **BROWNIAN-DRIFT** — `brownian-drift` · Modulator. Adds deterministic pseudo-random micro-motion to pitch, filter and stereo position. Визуальная метафора: A wandering particle draws an irregular trajectory.
- **CRYSTAL-LATTICE REVERB** — `crystal-lattice-reverb` · Reverb. Uses short, slightly inharmonic reflection lines for a crystalline spatial tail. Визуальная метафора: A molecular lattice carries reflected light pulses.
- **LASER-COHERENCE** — `laser-coherence` · StereoField. Improves low-band phase consistency and narrows unstable stereo differences. Визуальная метафора: A scattered beam focuses into one coherent line.
- **PLASMA-STATE** — `plasma-state` · Saturation. Combines bright asymmetric drive with a slowly moving spectral edge. Визуальная метафора: A luminous torus of plasma sits inside magnetic coils.
- **NEUTRON-STAR DENSITY** — `neutron-star-density` · Dynamics. A high-density compressor and soft peak shaper for heavy program material. Визуальная метафора: A compact star emits two narrow polar jets.
- **ALPHA-PARTICLE PUNCH** — `alpha-particle-punch` · Transient. Adds a hard, short attack while keeping the body under envelope control. Визуальная метафора: Four linked spheres punch through a waveform barrier.
- **BETA-DECAY SHIFTER** — `beta-decay-shifter` · Morph. Morphs a muted tone toward a brighter harmonic profile with a short upper transient. Визуальная метафора: A curve changes state and releases a bright particle trail.
- **GAMMA-BURST TRANSIENT** — `gamma-burst-transient` · Transient. Creates a focused, high-frequency attack flash with strict output bounds. Визуальная метафора: A narrow high-energy flash crosses the upper spectrum.
- **BOSON-CONDENSATE** — `boson-condensate` · Modulator. Reduces detune and phase spread for a stable unison-like image. Визуальная метафора: Independent waves merge into one smooth blue contour.
- **FERMION-STACK** — `fermion-stack` · Synth. Stacks several harmonic layers with deterministic frequency and phase separation. Визуальная метафора: A stepped energy ladder fills one state at a time.
- **MAGNETIC-MOMENT PAN** — `magnetic-moment-pan` · StereoField. Moves the stereo image along a smooth precessing pan orbit. Визуальная метафора: A spinning top carries a magnetic dipole vector.
- **AUGER-CASCADE** — `auger-cascade` · Exciter. Generates a controlled cascade of harmonics from the incoming envelope. Визуальная метафора: A chain of arrows jumps between orbital shells.

### IV. Поля, волны и релятивистская акустика (70–92)

- **INTERFERENCE-SLIT** — `interference-slit` · Resonator. Creates moving comb-filter interference from a pair of short phase paths. Визуальная метафора: Two slits produce a live interference pattern.
- **DOPPLER-HORIZON** — `doppler-horizon` · Modulator. Simulates a moving source using controlled pitch and stereo modulation. Визуальная метафора: A passing source compresses wavefronts ahead and stretches them behind.
- **TIME-DILATION STRETCH** — `time-dilation-stretch` · Time. Uses bounded variable-rate granular reads to create time and pitch movement. Визуальная метафора: A space-time grid stretches along the time axis.
- **EVENT-HORIZON LIMITER** — `event-horizon-limiter` · Dynamics. A fast soft-knee ceiling with an optional restrained harmonic halo. Визуальная метафора: A black disc with a bright accretion ring contains the peaks.
- **HAWKING-RADIATION** — `hawking-radiation` · Noise. Adds a tiny filtered noise tail around low-level decays. Визуальная метафора: A fine glow rises from the event horizon edge.
- **GRAVITATIONAL-WAVE** — `gravitational-wave` · StereoField. Modulates mid-side width in a slow quadrature movement. Визуальная метафора: A quadrupole grid pulses on two opposing axes.
- **CHERENKOV-GLOW** — `cherenkov-glow` · Exciter. Adds a bright upper-harmonic glow when attacks cross a level threshold. Визуальная метафора: A blue shock cone surrounds a fast transient.
- **FARADAY-ROTATOR** — `faraday-rotator` · StereoField. Rotates the stereo image with low-frequency energy as its control signal. Визуальная метафора: A polarization vector turns inside a magnetic coil.
- **STANDING-WAVE ROOM** — `standing-wave-room` · Resonator. Adds tuned comb resonances that emulate a small virtual resonator. Визуальная метафора: A stationary wave shows bright nodes and dark antinodes.
- **SOLITON-PULSE** — `soliton-pulse` · Transient. Strengthens the transient contour while preserving the central pulse envelope. Визуальная метафора: A single bell-shaped pulse travels without visible decay.
- **PHASE-VELOCITY EQ** — `phase-velocity-eq` · Spectrum. Shapes low, middle and high bands with separate phase-colour emphasis. Визуальная метафора: A dispersion curve contrasts early and late frequency groups.
- **CASIMIR-GAP** — `casimir-gap` · Resonator. Uses a narrow tuned resonator with a short low-frequency boundary. Визуальная метафора: Two mirrored plates frame a microscopic resonant gap.
- **MAGNETO-STRICTIVE** — `magneto-strictive` · Saturation. Adds transformer-like hysteresis using asymmetric saturation and recovery. Визуальная метафора: A metal core deforms inside magnetic flux lines.
- **WAVEGUIDE-TUBE** — `waveguide-tube` · Resonator. Uses variable comb reflection geometry to emulate a resonant waveguide. Визуальная метафора: A curved metal tube carries moving internal modes.
- **DIFFRACTION-GRATING** — `diffraction-grating` · Spectrum. Redistributes spectral emphasis into a set of narrow, bright bands. Визуальная метафора: A grating separates one beam into a frequency fan.
- **WORMHOLE-DELAY** — `wormhole-delay` · Delay. Uses a pre-echo and short reverse-grain cue to suggest a time-fold effect. Визуальная метафора: A curved tunnel joins two points on a time line.
- **TACHYON-TRANSIENT** — `tachyon-transient` · Transient. Creates a short pre-attack cue from a look-ahead envelope copy in offline processing. Визуальная метафора: A light pulse leads the main waveform.
- **VACUUM-POLARIZATION** — `vacuum-polarization` · Noise. Adds subtle nonlinear interaction to quiet passages and signal gaps. Визуальная метафора: Virtual pairs align along a field boundary.
- **HOLOGRAPHIC-PRINCIPLE** — `holographic-principle` · Morph. Combines mid-side projection and three-band balancing into a compact spatial image. Визуальная метафора: An interference plate reconstructs a volumetric audio image.
- **DARK-MATTER SUB** — `dark-matter-sub` · SubGenerator. Adds a restrained low-frequency anchor from the incoming bass envelope. Визуальная метафора: An invisible halo bends the lower spectral grid.
- **DARK-ENERGY EXPANDER** — `dark-energy-expander` · StereoField. Expands stereo width smoothly as the signal decays. Визуальная метафора: Points move outward across a widening spatial field.
- **PLASMON-SURFACE** — `plasmon-surface` · Exciter. Adds near-field upper-mid harmonics for a closer, more articulated sound. Визуальная метафора: A surface wave travels along a reflective boundary.
- **LORENTZ-CONTRACTION** — `lorentz-contraction` · Time. Applies short, bounded variable-rate contraction to transient-rich material. Визуальная метафора: A waveform packet compresses along its horizontal axis.

### V. Инструменты и экспериментальные процессоры (93–133)

- **ATOM-DRUM** — `atom-drum` · Synth. Adds a compact percussive oscillator and transient layer to the incoming signal. Визуальная метафора: A planet-like atom assigns drum roles to nucleus and shells.
- **QUARK-BASS** — `quark-bass` · SubGenerator. A three-part low-frequency oscillator with bounded phase separation. Визуальная метафора: Three bass oscillators are linked by a force triangle.
- **QUANTUM-PAD** — `quantum-pad` · Synth. Adds a slowly evolving multi-part harmonic bed from the input envelope. Визуальная метафора: A multidimensional probability cloud changes colour over time.
- **SCHRÖDINGER-ARP** — `schrodinger-arp` · Granular. Creates probabilistic, repeatable note-like grain steps from the source signal. Визуальная метафора: A branched note grid highlights its active path.
- **ANTIMATTER-INVERTER** — `antimatter-inverter` · StereoField. Uses phase inversion and mid-side cancellation to reduce correlated conflicts. Визуальная метафора: Opposing waves meet and cancel at a central point.
- **ANNIHILATION-DISTORTION** — `annihilation-distortion` · Saturation. A collision-driven asymmetric distortion for energetic harmonic bursts. Визуальная метафора: A photon flash appears at the collision threshold.
- **CHRONON-SLICER** — `chronon-slicer` · Granular. Cuts signal into repeatable short time cells with optional reverse ordering. Визуальная метафора: A timeline of draggable time quanta.
- **SPIN-NETWORK REVERB** — `spin-network-reverb` · Reverb. A network of short feedback reflections with an irregular spatial pattern. Визуальная метафора: A 3D graph carries pulses across linked nodes.
- **COHERENCE-METER** — `coherence-meter` · Analyzer. Measures peak, RMS, crest and stereo correlation without changing the audio. Визуальная метафора: A correlation matrix sits beside a live interferometer trace.
- **WAVE-PACKET GENERATOR** — `wave-packet-generator` · Synth. Adds a Gaussian-windowed oscillator packet shaped by the incoming envelope. Визуальная метафора: A Gaussian envelope contains a high-frequency carrier.
- **SUPERFLUID-DELAY** — `superfluid-delay` · Delay. Creates clear feedback repeats with minimal high-frequency damping. Визуальная метафора: A closed loop carries a continuously travelling pulse.
- **SUPERCONDUCTOR-BUS** — `superconductor-bus` · Dynamics. A fast linked-channel compressor that preserves attack contrast. Визуальная метафора: A cryogenic blue channel carries an unrestricted signal flow.
- **JOSEPHSON-JUNCTION** — `josephson-junction` · Quantizer. Adds a nonlinear sample-and-hold distortion with high-rate modulation. Визуальная метафора: Two superconductors face a thin oscillating barrier.
- **COPENHAGEN-MIXER** — `copenhagen-mixer` · Dynamics. Balances the signal with a tone-dependent energy reference; no cursor tracking is used. Визуальная метафора: A focus reticle sits above a multichannel spectrum.
- **MANY-WORLDS COMP** — `many-worlds-comp` · Dynamics. Blends multiple parallel compression strengths into one controlled result. Визуальная метафора: Eight translucent compression curves merge together.
- **PLANCK-LENGTH GATE** — `planck-length-gate` · Gate. Uses a fast, smoothed envelope gate with sample-accurate offline control. Визуальная метафора: A microscopic shutter follows a smooth S-curve.
- **NEUTRINO-PASS** — `neutrino-pass` · Equalizer. Emphasizes a selected pass band with a gentle minimum-phase tone model. Визуальная метафора: A ghost particle passes through a dense spectral wall.
- **CHAMBER-TRACKER** — `chamber-tracker` · Analyzer. Analyzes envelope peaks and converts them into a repeatable event-density report. Визуальная метафора: A bubble chamber draws spiral particle tracks.
- **CRYSTAL-QUARK** — `crystal-quark` · Granular. Applies deterministic grain symmetry and harmonic scattering. Визуальная метафора: A faceted gemstone contains internal luminous nodes.
- **QUANTUM-DOT VOCODER** — `quantum-dot-vocoder` · Vocoder. A three-band envelope vocoder with discrete formant-like carrier components. Визуальная метафора: A grid of nanocrystals changes colour with frequency.
- **TELEPORT-PRESET** — `teleport-preset` · Morph. Morphs between dry, saturated and spatial states using one continuous macro value. Визуальная метафора: Two portal rings exchange their parameter states.
- **EIGEN-VOCAL** — `eigen-vocal` · Vocoder. Separates low body, mid formants and high articulation with an envelope-guided model. Визуальная метафора: Four orthogonal vectors represent vocal components.
- **MAGNETAR-LIMITER** — `magnetar-limiter` · Dynamics. A dense peak limiter with magnetic-style asymmetric soft saturation. Визуальная метафора: A pulsing star is wrapped in strong magnetic field lines.
- **QUARK-GLUON PLASMA** — `quark-gluon-plasma` · Saturation. High-density nonlinear drive with controlled cross-band harmonic mixing. Визуальная метафора: A boiling field of coloured quarks and connecting strings.
- **PHASE-ENTANGLER** — `phase-entangler` · StereoField. Aligns stereo movement and reduces unstable inter-channel phase differences. Визуальная метафора: Interwoven wavefronts align into a coherent pattern.
- **SUB-ATOMIC TRANSIENT** — `sub-atomic-transient` · Transient. Shapes attack into click, impulse and body zones with independent envelope emphasis. Визуальная метафора: The first millisecond is split into three microscopic regions.
- **ZERO-LATENCY ORACLE** — `zero-latency-oracle` · Dynamics. An offline look-ahead soft peak shaper; it does not claim zero live-processing latency. Визуальная метафора: A ghost waveform forecasts the next few milliseconds.
- **HYBRID-BOSON** — `hybrid-boson` · Morph. Combines moderate compression, saturation and spatial control in one field. Визуальная метафора: One central field exposes dynamics, tone and width axes.
- **QUANTUM-WALK LFO** — `quantum-walk-lfo` · Modulator. Uses deterministic random-walk modulation with bounded drift and repeatability. Визуальная метафора: A branching modulation path travels across a step grid.
- **SPECTRAL-TUNNEL** — `spectral-tunnel` · Spectrum. Reduces low-mid masking while restoring upper-band definition. Визуальная метафора: An energy tunnel crosses a gap in the middle spectrum.
- **ATOM-CLOCK SYNC** — `atom-clock-sync` · Time. Applies deterministic micro-timing and phase correction to short events. Визуальная метафора: A precision atomic clock marks a fine time grid.
- **FRACTAL-ORBITAL** — `fractal-orbital` · Modulator. A nested chorus/flanger motion with bounded multi-rate modulation. Визуальная метафора: Fractal epicycles orbit around one another.
- **RELATIVITY-PAN** — `relativity-pan` · StereoField. Pan movement includes a short interaural timing offset for spatial depth. Визуальная метафора: A curved coordinate grid surrounds the listening position.
- **STRING-LANDSCAPE** — `string-landscape` · Morph. A macro-controlled texture morph across harmonics, modulation and stereo. Визуальная метафора: A multidimensional manifold folds into one control surface.
- **QUANTUM-NOISE GATE** — `quantum-noise-gate` · Gate. Envelope and noise-floor gate with a soft release and hysteresis-like hold. Визуальная метафора: An entropy filter separates irregular noise from structured signal.
- **NUCLEON-PUNCH** — `nucleon-punch` · SubGenerator. Reinforces bass and lower-mid body with a linked mono low-frequency envelope. Визуальная метафора: A proton and neutron pulse as a coupled pair.
- **PHOTON-DELAY** — `photon-delay` · Delay. A clear short delay with a filtered feedback loop and stereo reflection offset. Визуальная метафора: Laser beams bounce across a bank of ideal mirrors.
- **WAVE-COLLAPSE COMP** — `wave-collapse-comp` · Dynamics. Fast envelope compression with a soft-knee peak clamp. Визуальная метафора: Amplitude collapses at the moment it crosses a threshold.
- **ENTROPY-REVERSER** — `entropy-reverser` · Granular. Uses deterministic reverse-grain windows to pull scattered tails toward the transient. Визуальная метафора: Particles gather from a cloud into a defined crystal.
- **SINGULARITY-BASS** — `singularity-bass` · SubGenerator. Builds a focused mono sub layer with strict low-frequency stability. Визуальная метафора: A single bright point anchors the centre of the low spectrum.
- **OMEGA-FIELD** — `omega-field` · Morph. A final macro processor combining tone, dynamics, phase and spatial balance. Визуальная метафора: One unified sphere joins the spectral and spatial fields.

## Реализуемые DSP-модели

| Модель | Обработка |
|---|---|
| StereoField | M/S stereo field, mono low-band anchoring, bounded phase/pan motion |
| Dynamics | Envelope compressor, gate, soft knee and saturation-linked peak control |
| Equalizer | Stateful three-band IIR tone shaping |
| Saturation | Tanh, atan, asymmetric drive and soft-fold variants |
| Exciter | High-band extraction and transient/envelope-gated harmonic lift |
| Modulator | Short modulated delay, pitch/phase drift and deterministic cyclic movement |
| Delay | Feedback and multi-tap delay |
| Reverb | Four-line feedback reflection network |
| Gate | Smoothed envelope gate with soft release |
| Transient | Fast/slow envelope contrast shaping |
| Granular | Deterministic bounded grain rearrangement and reverse-window variants |
| Resonator | Tuned feedback comb/resonator |
| Quantizer | Amplitude bit-depth and sample-hold reduction |
| SubGenerator | Envelope-following mono low-frequency oscillator |
| Noise | Deterministic coloured and white-noise texture |
| Spectrum | Three-band spectral balance |
| Time | Offline variable-rate interpolation / transient contraction |
| Vocoder | Three-band envelope/carrier formant approximation |
| Synth | Signal-driven oscillator layers |
| Analyzer | Read-only peak, RMS and crest analysis |
| Morph | Hybrid nonlinear tone, envelope and dynamics morph |

The 133 catalogue entries are individual module identities and parameter presets over these reusable DSP kernels. Variants alter the model's response; the catalogue does not claim 133 entirely unrelated low-level algorithms.

## Architectural boundaries

This version is an **offline WAV processor**, not a real-time DAW. It does not provide a live audio callback, device routing, VST/AU hosting, true zero-latency processing, MIDI-triggered state collapse, quantum hardware, or physical particle simulation. "Teleport", "quantum", "quark", "relativity" and similar terms describe creative signal-processing metaphors. Time/pitch movement uses interpolation and modulation; no high-quality phase-vocoder time-stretch is claimed.

The visual descriptions in each catalogue entry are interface directions, while the current lab displays a compact signal scope and per-module textual visual model. Dedicated animated orbital/field visualizers can be layered on this host without changing the DSP API.

## Build and tests

The audio engine is part of `ventusample_core`; the Qt laboratory is included when Qt 6 Widgets is available. The independent test target exercises all 133 entries.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
