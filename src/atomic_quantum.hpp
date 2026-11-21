#pragma once

#include <complex>
#include <cstddef>
#include <string_view>
#include <vector>

namespace ventu {

inline constexpr std::size_t kAtomicQuantumMaxQubits = 10u;

struct AtomicQuantumRegister {
    std::size_t qubits{0u};
    std::vector<std::complex<double>> amplitudes;
};

struct AtomicQuantumMeasurement {
    unsigned outcome{0u};
    double probabilityOne{0.0};
};

const std::vector<std::string_view>& atomicQuantumFunctionIds() noexcept;
int findAtomicQuantumFunction(std::string_view id) noexcept;

AtomicQuantumRegister atomicQuantumZeroState(std::size_t qubits = 1u);
AtomicQuantumRegister atomicQuantumBasisState(std::size_t qubits, std::size_t basisIndex);
void atomicQuantumValidate(const AtomicQuantumRegister& state, bool requireNormalized = true);
void atomicQuantumNormalize(AtomicQuantumRegister& state);
void atomicQuantumReset(AtomicQuantumRegister& state);
void atomicQuantumHadamard(AtomicQuantumRegister& state, std::size_t target = 0u);
void atomicQuantumPhaseRotate(AtomicQuantumRegister& state, std::size_t target, double theta);
void atomicQuantumPauliX(AtomicQuantumRegister& state, std::size_t target = 0u);
void atomicQuantumPauliZ(AtomicQuantumRegister& state, std::size_t target = 0u);
void atomicQuantumCnot(AtomicQuantumRegister& state, std::size_t control, std::size_t target);
void atomicQuantumSwap(AtomicQuantumRegister& state, std::size_t left, std::size_t right);
AtomicQuantumRegister atomicQuantumBellPair();
AtomicQuantumRegister atomicQuantumGHZ(std::size_t qubits = 3u);
std::vector<double> atomicQuantumProbabilities(const AtomicQuantumRegister& state);
AtomicQuantumMeasurement atomicQuantumMeasureZ(
    AtomicQuantumRegister& state, std::size_t target, double sample);
void atomicQuantumDephase(
    AtomicQuantumRegister& state, std::size_t target, double probability, double sample);
void atomicQuantumAmplitudeDamp(
    AtomicQuantumRegister& state, std::size_t target, double gamma, double sample);
double atomicQuantumFidelity(
    const AtomicQuantumRegister& left, const AtomicQuantumRegister& right);

} // namespace ventu
