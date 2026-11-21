#include "atomic_quantum.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace ventu {
namespace {
const std::vector<std::string_view> kFunctionIds{
    "atom_superpose_basis_01",
    "qubit_phase_rotate_02",
    "atom_ground_reset_03",
    "state_amplitude_normalize_04",
    "atom_rabi_pulse_05",
    "qubit_bloch_project_06",
    "atom_stark_shift_07",
    "state_coherent_blend_08",
    "atom_zeeman_split_09",
    "qubit_hadamard_twin_10",
    "atom_metastable_hold_11",
    "state_polar_encode_12",
    "atom_hyperfin_lock_13",
    "qubit_zx_twist_14",
    "atom_optical_trap_15",
    "state_phase_flip_mask_16",
    "atom_level_detune_17",
    "qubit_iswap_core_18",
    "atom_recoil_damp_19",
    "state_vacuum_seed_20",
    "atom_quadrupole_tilt_21",
    "qubit_sqrt_not_22",
    "entangle_bell_pair_23",
    "entangle_ghz_ring_24",
    "entangle_w_chain_25",
    "correlate_swap_gate_26",
    "entangle_cluster_grid_27",
    "correlate_concurrence_28",
    "entangle_singlet_lock_29",
    "entangle_triplet_mix_30",
    "correlate_bell_witness_31",
    "entangle_dicke_slice_32",
    "correlate_mutual_info_33",
    "entangle_star_hub_34",
    "correlate_negativity_35",
    "entangle_spin_squeeze_36",
    "correlate_discord_flux_37",
    "entangle_ladder_weave_38",
    "correlate_purity_check_39",
    "entangle_cat_super_40",
    "correlate_schmidt_rank_41",
    "entangle_braid_step_42",
    "correlate_trace_dist_43",
    "entangle_mesh_sync_44",
    "measure_projective_z_45",
    "measure_weak_probe_46",
    "measure_povm_frame_47",
    "feedback_phase_steer_48",
    "measure_parity_check_49",
    "measure_homodyne_quad_50",
    "feedback_error_synd_51",
    "measure_tomo_slice_52",
    "measure_bell_project_53",
    "feedback_gain_adapt_54",
    "measure_arrival_tick_55",
    "measure_double_click_56",
    "feedback_reset_gate_57",
    "measure_ramsey_fringe_58",
    "measure_fidelity_ref_59",
    "feedback_drift_trim_60",
    "measure_count_bucket_61",
    "measure_threshold_hit_62",
    "feedback_cooldown_hold_63",
    "measure_joint_moment_64",
    "measure_null_result_65",
    "feedback_loop_clamp_66",
    "noise_depolarize_ch_67",
    "noise_dephase_markov_68",
    "noise_amplitude_damp_69",
    "ecc_bitflip_code_70",
    "ecc_phaseflip_code_71",
    "ecc_surface_plaquette_72",
    "ecc_vertex_star_73",
    "noise_thermal_bath_74",
    "ecc_decoder_mwpm_75",
    "noise_crosstalk_leak_76",
    "ecc_logical_x_77",
    "ecc_logical_z_78",
    "noise_burst_spike_79",
    "ecc_flag_qubit_80",
    "noise_1f_flicker_81",
    "ecc_lattice_surgery_82",
    "noise_loss_erase_83",
    "ecc_magic_distill_84",
    "noise_non_markov_back_85",
    "ecc_steane_block_86",
    "noise_gate_overrot_87",
    "ecc_syndrome_history_88",
    "lattice_hop_tunnel_89",
    "lattice_hubbard_onsite_90",
    "atom_cool_sideband_91",
    "transport_shuttle_move_92",
    "lattice_peierls_flux_93",
    "atom_recoil_kick_94",
    "lattice_mott_gap_95",
    "transport_junction_split_96",
    "lattice_superfluid_phase_97",
    "atom_dark_state_98",
    "lattice_disorder_ramp_99",
    "transport_merge_wells_100",
    "lattice_rydberg_block_101",
    "atom_raman_transfer_102",
    "lattice_boundary_wrap_103",
    "transport_adiabatic_ramp_104",
    "lattice_dimer_pair_105",
    "atom_doppler_shift_106",
    "lattice_spin_exchange_107",
    "transport_trap_handoff_108",
    "lattice_anyon_braid_109",
    "atom_sisyphus_step_110",
    "algo_qaoa_cost_111",
    "algo_qaoa_mixer_112",
    "algo_vqe_ansatz_113",
    "algo_qpe_phase_114",
    "algo_grover_oracle_115",
    "algo_grover_diffuser_116",
    "algo_hhl_invert_117",
    "algo_walk_coin_118",
    "algo_walk_shift_119",
    "telem_coherence_time_120",
    "telem_gate_error_121",
    "telem_readout_conf_122",
    "telem_temp_jitter_123",
    "telem_entangle_depth_124",
    "telem_flux_balance_125",
    "algo_anneal_schedule_126",
    "algo_fourier_qft_127",
    "algo_amplitude_est_128",
    "telem_spectral_gap_129",
    "telem_drift_rate_130",
    "algo_teleport_link_131",
    "telem_packet_loss_132",
    "telem_system_health_133"
};

void validateProbability(double value, const char* label) {
    if (!std::isfinite(value) || value < 0.0 || value > 1.0)
        throw std::invalid_argument(std::string(label) + " must be finite and in [0, 1]");
}

std::size_t validateTarget(const AtomicQuantumRegister& state, std::size_t target) {
    atomicQuantumValidate(state);
    if (target >= state.qubits) throw std::out_of_range("target qubit is outside the register");
    return target;
}

template <typename Function>
void forTargetPairs(AtomicQuantumRegister& state, std::size_t target, Function&& function) {
    const std::size_t mask = std::size_t{1} << target;
    for (std::size_t zero = 0u; zero < state.amplitudes.size(); ++zero) {
        if ((zero & mask) == 0u) function(zero, zero | mask);
    }
}
} // namespace

const std::vector<std::string_view>& atomicQuantumFunctionIds() noexcept { return kFunctionIds; }

int findAtomicQuantumFunction(std::string_view id) noexcept {
    const auto it = std::find(kFunctionIds.begin(), kFunctionIds.end(), id);
    return it == kFunctionIds.end() ? -1 : static_cast<int>(std::distance(kFunctionIds.begin(), it));
}

AtomicQuantumRegister atomicQuantumZeroState(std::size_t qubits) {
    if (qubits == 0u || qubits > kAtomicQuantumMaxQubits)
        throw std::invalid_argument("qubit count must be in 1..10");
    AtomicQuantumRegister state;
    state.qubits = qubits;
    state.amplitudes.assign(std::size_t{1} << qubits, {0.0, 0.0});
    state.amplitudes[0] = {1.0, 0.0};
    return state;
}

AtomicQuantumRegister atomicQuantumBasisState(std::size_t qubits, std::size_t basisIndex) {
    auto state = atomicQuantumZeroState(qubits);
    if (basisIndex >= state.amplitudes.size())
        throw std::out_of_range("basis index is outside the register");
    std::fill(state.amplitudes.begin(), state.amplitudes.end(), std::complex<double>{0.0, 0.0});
    state.amplitudes[basisIndex] = {1.0, 0.0};
    return state;
}

void atomicQuantumValidate(const AtomicQuantumRegister& state, bool requireNormalized) {
    if (state.qubits == 0u || state.qubits > kAtomicQuantumMaxQubits ||
        state.amplitudes.size() != (std::size_t{1} << state.qubits))
        throw std::invalid_argument("invalid atomic quantum register dimensions");
    long double normSquared = 0.0L;
    for (const auto& amplitude : state.amplitudes) {
        if (!std::isfinite(amplitude.real()) || !std::isfinite(amplitude.imag()))
            throw std::invalid_argument("state amplitudes must be finite");
        normSquared += static_cast<long double>(std::norm(amplitude));
    }
    if (!std::isfinite(normSquared) || normSquared <= 0.0L)
        throw std::invalid_argument("state norm must be finite and positive");
    if (requireNormalized && std::abs(normSquared - 1.0L) > 1e-8L)
        throw std::invalid_argument("state vector must be normalized");
}

void atomicQuantumNormalize(AtomicQuantumRegister& state) {
    atomicQuantumValidate(state, false);
    long double normSquared = 0.0L;
    for (const auto& amplitude : state.amplitudes)
        normSquared += static_cast<long double>(std::norm(amplitude));
    const double scale = 1.0 / std::sqrt(static_cast<double>(normSquared));
    if (!std::isfinite(scale)) throw std::invalid_argument("normalization scale is not finite");
    for (auto& amplitude : state.amplitudes) amplitude *= scale;
    atomicQuantumValidate(state);
}

void atomicQuantumReset(AtomicQuantumRegister& state) {
    atomicQuantumValidate(state, false);
    std::fill(state.amplitudes.begin(), state.amplitudes.end(), std::complex<double>{0.0, 0.0});
    state.amplitudes[0] = {1.0, 0.0};
}

void atomicQuantumHadamard(AtomicQuantumRegister& state, std::size_t target) {
    validateTarget(state, target);
    constexpr double scale = 0.7071067811865475244;
    forTargetPairs(state, target, [&](std::size_t zero, std::size_t one) {
        const auto a0 = state.amplitudes[zero];
        const auto a1 = state.amplitudes[one];
        state.amplitudes[zero] = (a0 + a1) * scale;
        state.amplitudes[one] = (a0 - a1) * scale;
    });
    atomicQuantumValidate(state);
}

void atomicQuantumPhaseRotate(AtomicQuantumRegister& state, std::size_t target, double theta) {
    validateTarget(state, target);
    if (!std::isfinite(theta)) throw std::invalid_argument("theta must be finite");
    const std::complex<double> phase{std::cos(theta), std::sin(theta)};
    const std::size_t mask = std::size_t{1} << target;
    for (std::size_t i = 0u; i < state.amplitudes.size(); ++i)
        if (i & mask) state.amplitudes[i] *= phase;
    atomicQuantumValidate(state);
}

void atomicQuantumPauliX(AtomicQuantumRegister& state, std::size_t target) {
    validateTarget(state, target);
    forTargetPairs(state, target, [&](std::size_t zero, std::size_t one) {
        std::swap(state.amplitudes[zero], state.amplitudes[one]);
    });
    atomicQuantumValidate(state);
}

void atomicQuantumPauliZ(AtomicQuantumRegister& state, std::size_t target) {
    validateTarget(state, target);
    const std::size_t mask = std::size_t{1} << target;
    for (std::size_t i = 0u; i < state.amplitudes.size(); ++i)
        if (i & mask) state.amplitudes[i] = -state.amplitudes[i];
    atomicQuantumValidate(state);
}

void atomicQuantumCnot(AtomicQuantumRegister& state, std::size_t control, std::size_t target) {
    validateTarget(state, control);
    validateTarget(state, target);
    if (control == target) throw std::invalid_argument("control and target must differ");
    const std::size_t controlMask = std::size_t{1} << control;
    const std::size_t targetMask = std::size_t{1} << target;
    for (std::size_t i = 0u; i < state.amplitudes.size(); ++i) {
        if ((i & controlMask) && !(i & targetMask))
            std::swap(state.amplitudes[i], state.amplitudes[i | targetMask]);
    }
    atomicQuantumValidate(state);
}

void atomicQuantumSwap(AtomicQuantumRegister& state, std::size_t left, std::size_t right) {
    validateTarget(state, left);
    validateTarget(state, right);
    if (left == right) return;
    const std::size_t leftMask = std::size_t{1} << left;
    const std::size_t rightMask = std::size_t{1} << right;
    for (std::size_t i = 0u; i < state.amplitudes.size(); ++i)
        if (!(i & leftMask) && (i & rightMask))
            std::swap(state.amplitudes[i], state.amplitudes[i ^ leftMask ^ rightMask]);
    atomicQuantumValidate(state);
}

AtomicQuantumRegister atomicQuantumBellPair() {
    auto state = atomicQuantumZeroState(2u);
    atomicQuantumHadamard(state, 0u);
    atomicQuantumCnot(state, 0u, 1u);
    return state;
}

AtomicQuantumRegister atomicQuantumGHZ(std::size_t qubits) {
    auto state = atomicQuantumZeroState(qubits);
    atomicQuantumHadamard(state, 0u);
    for (std::size_t target = 1u; target < qubits; ++target)
        atomicQuantumCnot(state, 0u, target);
    return state;
}

std::vector<double> atomicQuantumProbabilities(const AtomicQuantumRegister& state) {
    atomicQuantumValidate(state);
    std::vector<double> result;
    result.reserve(state.amplitudes.size());
    for (const auto& amplitude : state.amplitudes) result.push_back(std::norm(amplitude));
    return result;
}

AtomicQuantumMeasurement atomicQuantumMeasureZ(
    AtomicQuantumRegister& state, std::size_t target, double sample) {
    validateTarget(state, target);
    if (!std::isfinite(sample) || sample < 0.0 || sample >= 1.0)
        throw std::invalid_argument("measurement sample must be in [0, 1)");
    const std::size_t mask = std::size_t{1} << target;
    long double p1 = 0.0L;
    for (std::size_t i = 0u; i < state.amplitudes.size(); ++i)
        if (i & mask) p1 += static_cast<long double>(std::norm(state.amplitudes[i]));
    const unsigned outcome = static_cast<long double>(sample) < p1 ? 1u : 0u;
    for (std::size_t i = 0u; i < state.amplitudes.size(); ++i)
        if (((i & mask) != 0u) != (outcome != 0u)) state.amplitudes[i] = {0.0, 0.0};
    atomicQuantumNormalize(state);
    return {outcome, static_cast<double>(p1)};
}

void atomicQuantumDephase(
    AtomicQuantumRegister& state, std::size_t target, double probability, double sample) {
    validateTarget(state, target);
    validateProbability(probability, "probability");
    if (!std::isfinite(sample) || sample < 0.0 || sample >= 1.0)
        throw std::invalid_argument("sample must be in [0, 1)");
    if (sample < probability) atomicQuantumPauliZ(state, target);
}

void atomicQuantumAmplitudeDamp(
    AtomicQuantumRegister& state, std::size_t target, double gamma, double sample) {
    validateTarget(state, target);
    validateProbability(gamma, "gamma");
    if (!std::isfinite(sample) || sample < 0.0 || sample >= 1.0)
        throw std::invalid_argument("sample must be in [0, 1)");
    const std::size_t mask = std::size_t{1} << target;
    long double p1 = 0.0L;
    for (std::size_t i = 0u; i < state.amplitudes.size(); ++i)
        if (i & mask) p1 += static_cast<long double>(std::norm(state.amplitudes[i]));
    const bool jump = static_cast<long double>(sample) < static_cast<long double>(gamma) * p1;
    const double scale = std::sqrt(jump ? gamma : 1.0 - gamma);
    forTargetPairs(state, target, [&](std::size_t zero, std::size_t one) {
        const auto a0 = state.amplitudes[zero];
        const auto a1 = state.amplitudes[one];
        if (jump) {
            state.amplitudes[zero] = a1 * scale;
            state.amplitudes[one] = {0.0, 0.0};
        } else {
            state.amplitudes[zero] = a0;
            state.amplitudes[one] = a1 * scale;
        }
    });
    atomicQuantumNormalize(state);
}

double atomicQuantumFidelity(
    const AtomicQuantumRegister& left, const AtomicQuantumRegister& right) {
    atomicQuantumValidate(left);
    atomicQuantumValidate(right);
    if (left.qubits != right.qubits) throw std::invalid_argument("register sizes must match");
    std::complex<long double> overlap{0.0L, 0.0L};
    for (std::size_t i = 0u; i < left.amplitudes.size(); ++i) {
        const std::complex<long double> a{
            static_cast<long double>(left.amplitudes[i].real()),
            static_cast<long double>(left.amplitudes[i].imag())};
        const std::complex<long double> b{
            static_cast<long double>(right.amplitudes[i].real()),
            static_cast<long double>(right.amplitudes[i].imag())};
        overlap += std::conj(a) * b;
    }
    const long double value = std::norm(overlap);
    if (!std::isfinite(value)) throw std::invalid_argument("fidelity is not finite");
    return std::clamp(static_cast<double>(value), 0.0, 1.0);
}

} // namespace ventu
