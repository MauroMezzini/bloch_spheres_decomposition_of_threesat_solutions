#include "controlled_R_y_target_N.hpp"

#include <iomanip>
#include <iostream>
#include <random>
/*
 This file contains helper functions for the controlled_R_y_target_N.cpp implementation. It includes functions for generating random quantum states,
 printing state vectors, verifying normalization, and testing the state transformation procedure Y. The functions are designed to assist in debugging 
 and validating the correctness of the quantum state transformations.

 Many of these function are used during the debug and thest of the software, and are not part of the final production code. 
 They are included here for completeness and to facilitate testing and validation of the quantum state transformation algorithms.
*/

double verifyNormalization(const std::vector<double>& state) {
    double sum_sq = 0.0;
    for (double amplitude : state) {
        sum_sq += amplitude * amplitude;
    }
    return sum_sq;
}

std::vector<double> generateRandomQuantumState(int num_qubits) {
    long long state_size = 1LL << num_qubits;
    std::vector<double> state(state_size);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    double sum_of_squares = 0.0;

    for (long long i = 0; i < state_size; ++i) {
        state[i] = dist(gen);
        sum_of_squares += state[i] * state[i];
    }

    double norm_factor = std::sqrt(sum_of_squares);
    for (long long i = 0; i < state_size; ++i) {
        state[i] /= norm_factor;
    }

    return state;
}

void printState(const std::string& name, const std::vector<double>& state) {
    std::cout << "--- " << name << " (" << state.size() << " states) ---\n";
    for (size_t i = 0; i < state.size(); ++i) {
        std::cout << "|" << std::setw(2) << std::setfill('0') << i << ">: "
                  << std::fixed << std::setprecision(5) << std::showpos << state[i] << "\n";
    }
    double check_sum = verifyNormalization(state);
    std::cout << std::noshowpos << ">> Sum of squared amplitudes: "
              << std::fixed << std::setprecision(5) << check_sum << "\n\n";
}

void printState(const std::vector<double>& Q) {
    for (size_t i = 0; i < Q.size(); ++i) {
        std::cout << "|" << i << ">: " << std::fixed << std::setprecision(4) << Q[i] << "\n";
    }
    std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
}

void printAngles(const std::map<int, double>& angles) {
    std::cout << "Theta values:\n";
    for (const auto& [J, value] : angles) {
        std::cout << "  J = " << J << " -> " << std::fixed << std::setprecision(4) << value << " rad\n";
    }
}

void checkStateNorm(const std::vector<double>& Q) {
    double norm = 0.0;
    for (double amp : Q) {
        norm += amp * amp;
    }
    std::cout << "State vector norm: " << std::fixed << std::setprecision(6) << norm << "\n";
}

void testGenerationRandomState() {
    int N1 = 3;
    int N2 = 4;

    std::vector<double> system_3_qubits = generateRandomQuantumState(N1);
    std::vector<double> system_4_qubits = generateRandomQuantumState(N2);

    printState("System 1 (3 Qubits)", system_3_qubits);
    printState("System 2 (4 Qubits)", system_4_qubits);
}

void testTwoQubitUncontrolledRotation() {
    int N = 2;
    std::vector<double> Q = {
        std::sqrt(0.25), std::sqrt(0.25),
        std::sqrt(0.25), std::sqrt(0.25)
    };

    double theta = std::acos(-1.0) / 4.0;
    int J_mask = 0;

    std::cout << "--- 2-Qubit Test: Uncontrolled Ry(45 degrees) ---\n";
    std::cout << "Initial state vector:\n";
    printState(Q);

    applyControlledRy(Q, N, J_mask, theta);

    std::cout << "\nState after applying Ry(45 degrees) with J = empty set:\n";
    printState(Q);
}

void testDistinctAmplitudeSystems() {
    std::cout << "--- Test 1: 3-qubit system with distinct amplitudes ---\n";
    int N3 = 3;
    std::vector<double> q3 = {0.10, 0.25, 0.40, 0.55, 0.70, 0.85, 1.00, 1.15};
    std::vector<double> q3_copy = q3;
    checkStateNorm(q3_copy);

    std::map<int, double> angles3 = stateTransformationY(q3_copy, N3, true);
    printAngles(angles3);
    std::cout << "Final state:\n";
    printState(q3_copy);

    std::cout << "\n--- Test 2: 4-qubit system with distinct amplitudes ---\n";
    int N4 = 4;
    std::vector<double> q4 = {0.05, 0.11, 0.17, 0.23, 0.29, 0.37, 0.43, 0.51,
                              0.59, 0.67, 0.73, 0.79, 0.83, 0.89, 0.97, 1.03};
    std::vector<double> q4_copy = q4;
    checkStateNorm(q4_copy);

    std::map<int, double> angles4 = stateTransformationY(q4_copy, N4, true);
    printAngles(angles4);
    std::cout << "Final state:\n";
    printState(q4_copy);
}

void testStateTransformationY(std::vector<double> Q1, int N) {
    std::map<int, double> angles = stateTransformationY(Q1, N, true);

    std::cout << "\n--- Computed Transformation Angles (Theta) ---\n";
    for (auto const& [J, val] : angles) {
        std::cout << "Subset J mask: " << J << " -> Angle: " << val << " rad\n";
    }

    std::cout << "\n--- Final Mutated State Vector ---\n";
    for (size_t i = 0; i < Q1.size(); ++i) {
        std::cout << "|" << i << ">: " << std::fixed << std::setprecision(4) << Q1[i] << "\n";
    }
    // Verify normalization
    std::cout << "\n--- Verification of State Vector Normalization ---\n";
    std::cout << "Sum of squared amplitudes: " << std::fixed << std::setprecision(6) << verifyNormalization(Q1) << "\n";
    if (std::abs(verifyNormalization(Q1) - 1.0) < 1e-6) {
        std::cout << "State vector is normalized.\n";
    } else {
        std::cout << "Warning: State vector is NOT normalized!\n";
    }
    
}   


std::vector<double> extractEvenIndices(const std::vector<double>& Q1, int N) {
    // The size of a system with N-1 qubits is 2^(N-1)
    long long size_Q2 = 1LL << (N - 1);
    std::vector<double> Q2(size_Q2);

    for (long long i = 0; i < size_Q2; ++i) {
        // Mapping index i of Q2 to the even index 2*i of Q1
        Q2[i] = Q1[2 * i];
    }

    return Q2;
}


int testRandomSystem() {
    // Declare the vectors for the two quantum systems
    int N1 = 3;
    int N2 = 4;

    std::vector<double> system_3_qubits = generateRandomQuantumState(N1);
    std::vector<double> system_4_qubits = generateRandomQuantumState(N2);

    // Print and verify System 1
    printState("System 1 (3 Qubits)", system_3_qubits);
    testStateTransformationY(system_3_qubits, N1);


    // Print and verify System 2
    printState("System 2 (4 Qubits)", system_4_qubits);
    testStateTransformationY(system_4_qubits, N2);


    return 0;
}

/**
 * Test function: Given N qubits, it creates a uniform superposition state
 * and returns the map of rotation angles generated by the recursive
 * stateTransformationY process.
 */
RecursiveThetaMap testUniformSuperpositionAngles(int N) {
    // 1. Calculate state vector size (2^N)
    long long state_size = 1LL << N;
    
    // 2. In a uniform superposition, all amplitudes are equal to 1/sqrt(2^N)
    double uniform_amplitude = 1.0 / std::sqrt(static_cast<double>(state_size));
    
    // 3. Initialize the state vector with the uniform amplitude
    std::vector<double> Q(state_size, uniform_amplitude);
    printState("Initial Uniform Superposition State", Q);
    
    // 4. Map to store the resulting angles for each qubit level and bitmask J
    RecursiveThetaMap theta_map;
    
    // 5. Apply the recursive algorithm (from conversation history)
    // This modifies Q in-place at each step and populates theta_map
    recursiveApplyY(Q, N, theta_map, true);
    
    return theta_map;
}

/*
int generation_and_creation_all_solution_test() {
    int N = 3;          // Number of variables
    double alpha = 2;  // High ratio -> Exponentially fewer solutions than 2^N
    int M = static_cast<int>(N * alpha);

    std::cout << "--- Generating 3-SAT Instance ---\n";
    std::cout << "Variables (N): " << N << " (Max search space = " << (1 << N) << ")\n";
    std::cout << "Clauses (M): " << M << " (Alpha ratio = " << alpha << ")\n\n";

    ThreeSatGenerator generator;
    // Generate the instance directly using our new structure
    ThreeSatInstance instance = generator.generate(N, M);

    std::cout << "Successfully generated standard structure! Computed vars: " 
              << instance.getNumVariables() << "\n\n";
        
    instance.printDimacs();
    // 1. Test standard evaluation on an arbitrary assignment (all false)
    std::vector<bool> test_assignment(N + 1, false);
    std::cout << "Is 'all-false' assignment a solution? " 
              << (instance.isSolution(test_assignment) ? "YES" : "NO") << "\n\n";

    // 2. Extract and count all satisfying vectors
    std::cout << "Gathering satisfying solution vectors...\n";
    std::vector<long long> solutions = instance.computeAllSolutions();

    std::cout << "Found " << solutions.size() << " valid solution(s).\n";
    
    if (!solutions.empty()) {
        std::cout << "Solutions (as raw integer values representing binary configurations):\n";
        for (long long sol : solutions) {
            std::cout << " -> S[i] = " << sol << " (Binary config index)\n";
        }
    }

    return 0;
}
*/

void new_test_for_applyControlledRy(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> theta_dist(-2.0 * PI, 2.0 * PI);
    std::uniform_real_distribution<double> amplitude_dist(-1.0, 1.0);

    double sum_of_squares = 0.0;
    int N = 4;
    int state_size = 1LL << N;
    std::vector<double> reference_state(state_size);
    for (long long i = 0; i < state_size; ++i) {
        double value = amplitude_dist(gen);
        reference_state[i] = value;
        sum_of_squares += value * value;
    }
    for (long long i = 0; i < state_size; ++i) {
        reference_state[i] /= std::sqrt(sum_of_squares);
    }
    int J_mask = 10;
        
    double theta = theta_dist(gen);

    applyControlledRy(reference_state, N, J_mask, theta);
}


int testApplyControlledRyConsistency(int num_trials, int max_qubits) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> qubit_dist(1, max_qubits);
    std::uniform_real_distribution<double> theta_dist(-2.0 * PI, 2.0 * PI);
    std::uniform_real_distribution<double> amplitude_dist(-1.0, 1.0);

    for (int trial = 0; trial < num_trials; ++trial) {
        int N = qubit_dist(gen);
        long long state_size = 1LL << N;

        std::vector<double> reference_state(state_size);
        std::vector<double> naive_state(state_size);

        double sum_of_squares = 0.0;
        for (long long i = 0; i < state_size; ++i) {
            double value = amplitude_dist(gen);
            reference_state[i] = value;
            naive_state[i] = value;
            sum_of_squares += value * value;
        }
        for (long long i = 0; i < state_size; ++i) {
            reference_state[i] /= std::sqrt(sum_of_squares);
            naive_state[i] /= std::sqrt(sum_of_squares);
        }


        int J_mask = 0;
        if (N > 1) {
            int max_mask = (1 << (N - 1)) - 1;
            std::uniform_int_distribution<int> mask_dist(0, max_mask);
            J_mask = mask_dist(gen);
        }
        J_mask <<= 1; // Shift left to ensure the target qubit (N-th qubit) is not part of J
        double theta = theta_dist(gen);

        applyControlledRy_naif(naive_state, N, J_mask, theta);
        applyControlledRy(reference_state, N, J_mask, theta);

        bool match = true;
        for (size_t i = 0; i < reference_state.size(); ++i) {
            if (std::abs(reference_state[i] - naive_state[i]) > 1e-12) {
                match = false;
                break;
            }
        }

        if (!match) {
            std::cerr << "Mismatch found for trial " << trial
                      << " with N=" << N << ", J_mask=" << J_mask
                      << ", theta=" << theta << "\n";
            printState("naive", naive_state);
            printState("optimized", reference_state);
            return 1;
        }
    }

    std::cout << "All " << num_trials << " randomized controlled-Ry checks matched.\n";
    return 0;
}
