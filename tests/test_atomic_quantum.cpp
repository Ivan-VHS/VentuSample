#include "atomic_quantum.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

namespace {
void require(bool value, const char* message) {
    if (!value) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}
}

int main() {
    const auto& ids = ventu::atomicQuantumFunctionIds();
    require(ids.size() == 133u, "atomic quantum catalogue contains 133 IDs");
    require(ids.front() == "atom_superpose_basis_01", "first stable ID");
    require(ids.back() == "telem_system_health_133", "last stable ID");
    std::set<std::string_view> unique(ids.begin(), ids.end());
    require(unique.size() == 133u, "all atomic quantum IDs are unique");
    require(ventu::findAtomicQuantumFunction("entangle_bell_pair_23") == 22,
            "function lookup resolves a stable ID");

    auto bell = ventu::atomicQuantumBellPair();
    const auto bellProbabilities = ventu::atomicQuantumProbabilities(bell);
    require(std::abs(bellProbabilities[0] - 0.5) < 1e-12, "Bell state has 50% |00>");
    require(std::abs(bellProbabilities[3] - 0.5) < 1e-12, "Bell state has 50% |11>");
    require(std::abs(ventu::atomicQuantumFidelity(bell, bell) - 1.0) < 1e-12,
            "identical state fidelity is one");

    auto ghz = ventu::atomicQuantumGHZ(3u);
    const auto ghzProbabilities = ventu::atomicQuantumProbabilities(ghz);
    require(std::abs(ghzProbabilities[0] - 0.5) < 1e-12, "GHZ state has 50% |000>");
    require(std::abs(ghzProbabilities[7] - 0.5) < 1e-12, "GHZ state has 50% |111>");

    auto measured = bell;
    const auto measurement = ventu::atomicQuantumMeasureZ(measured, 0u, 0.8);
    require(measurement.outcome == 1u, "seeded sample determines reproducible measurement");
    const auto collapsed = ventu::atomicQuantumProbabilities(measured);
    require(std::count_if(collapsed.begin(), collapsed.end(), [](double p) { return p > 0.0; }) == 1,
            "projective readout collapses one measured branch");

    auto noisy = bell;
    ventu::atomicQuantumDephase(noisy, 0u, 1.0, 0.0);
    ventu::atomicQuantumAmplitudeDamp(noisy, 1u, 0.2, 0.0);
    ventu::atomicQuantumHadamard(noisy, 0u);
    ventu::atomicQuantumPhaseRotate(noisy, 1u, 0.4);
    ventu::atomicQuantumPauliX(noisy, 0u);
    ventu::atomicQuantumPauliZ(noisy, 0u);
    ventu::atomicQuantumSwap(noisy, 0u, 1u);
    ventu::atomicQuantumCnot(noisy, 0u, 1u);
    ventu::atomicQuantumReset(noisy);
    require(std::abs(ventu::atomicQuantumFidelity(noisy, ventu::atomicQuantumZeroState(2u)) - 1.0) < 1e-12,
            "reset returns to ground state");

    bool rejected = false;
    try { (void)ventu::atomicQuantumZeroState(ventu::kAtomicQuantumMaxQubits + 1u); }
    catch (const std::invalid_argument&) { rejected = true; }
    require(rejected, "qubit limit is enforced");
    std::cout << "Ventu atomic quantum state simulator tests passed (133 catalogue IDs).\n";
    return 0;
}
