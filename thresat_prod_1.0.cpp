#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <set>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>
#include <utility>
#include <stdexcept>
#include "controlled_R_y_target_N.hpp"

// Represents a literal: positive integer for variable X, negative for NOT X
using Literal = int;

struct Clause {
    Literal l1, l2, l3;
};

// ==========================================
// 3-SAT Instance Class
// ==========================================
class ThreeSatInstance {
private:
    std::vector<Clause> clauses;
    int num_variables;

    // Helper to extract the maximum variable index present in the clauses
    int computeNumVariables(const std::vector<Clause>& clause_set) {
        int max_var = 0;
        for (const auto& c : clause_set) {
            max_var = std::max({max_var, std::abs(c.l1), std::abs(c.l2), std::abs(c.l3)});
        }
        return max_var;
    }

public:
    // Constructor accepts a set of clauses and calculates N automatically
    ThreeSatInstance(const std::vector<Clause>& clause_set) : clauses(clause_set) {
        num_variables = computeNumVariables(clauses);
    }

    int getNumVariables() const { return num_variables; }
    int getNumClauses() const { return clauses.size(); }

    // Checks if a given boolean vector assignment is a valid solution
    // Expects assignment size to be at least num_variables + 1 (1-indexed mapping)
    bool isSolution(const std::vector<bool>& assignment) const {
        for (const auto& c : clauses) {
            // A clause is true if AT LEAST one of its literals evaluates to true
            bool literal1_true = (c.l1 > 0) ? assignment[c.l1] : !assignment[std::abs(c.l1)];
            bool literal2_true = (c.l2 > 0) ? assignment[c.l2] : !assignment[std::abs(c.l2)];
            bool literal3_true = (c.l3 > 0) ? assignment[c.l3] : !assignment[std::abs(c.l3)];

            if (!literal1_true && !literal2_true && !literal3_true) {
                return false; // Found an unsatisfied clause
            }
        }
        return true; // All clauses satisfied
    }

    // Computes the set of all solutions.
    // Returns a vector of integers whose binary representations are valid solutions.
    std::vector<long long> computeAllSolutions(bool verbose = true) const {
        std::vector<long long> solution_indices;
        long long search_space = 1LL << num_variables; // 2^N elements

        // We use a 1-indexed vector internally to map to variable IDs seamlessly
        std::vector<bool> current_assignment(num_variables + 1, false);

        for (long long i = 0; i < search_space; ++i) {
            // Translate the integer 'i' into the boolean assignment vector
            // Bit 0 controls variable 1, Bit 1 controls variable 2, etc.
            for (int var_id = 1; var_id <= num_variables; ++var_id) {
                current_assignment[var_id] = (i & (1LL << (var_id - 1))) != 0;
            }
            std::string solution_binary = "Not a solution: ";
            // If it satisfies the instance, store the integer representation
            if (isSolution(current_assignment)) {
                solution_indices.push_back(static_cast<long long>(i));
                // print nicely the binary representation of the solution
                solution_binary = "Solution found: ";
                
            }
            if (verbose) {
                std::cout << solution_binary;
                for (int var_id = 1; var_id <= num_variables; ++var_id) {
                    std::cout << (current_assignment[var_id] ? "1" : "0");
                }
                std::cout <<" ["<< i<<  "]\n";
            }
        }
        return solution_indices;
    }

    void printDimacs() const {
        std::cout << "p cnf " << num_variables << " " << clauses.size() << "\n";
        for (const auto& c : clauses) {
            std::cout << c.l1 << " " << c.l2 << " " << c.l3 << " 0\n";
        }
    }
};

// ==========================================
// 3-SAT Instance Generator Class
// ==========================================
class ThreeSatGenerator {
private:
    std::mt19937 rng;

public:
    ThreeSatGenerator() {
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        rng.seed(seed);
    }

    // Generates a random 3-SAT instance.
    // To ensure the solutions are exponentially lower than 2^N, use a high alpha ratio (> 4.26)
    ThreeSatInstance generate(int num_variables, int num_clauses) {
        std::vector<Clause> clause_set;
        clause_set.reserve(num_clauses);
        std::set<std::vector<int>> unique_clauses;

        while (clause_set.size() < static_cast<size_t>(num_clauses)) {
            std::vector<int> vars;
            
            // 1. Pick 3 distinct variables
            while (vars.size() < 3) {
                std::uniform_int_distribution<int> dist(1, num_variables);
                int v = dist(rng);
                if (std::find(vars.begin(), vars.end(), v) == vars.end()) {
                    vars.push_back(v);
                }
            }
            std::sort(vars.begin(), vars.end());

            // 2. Randomly negate literals
            std::uniform_int_distribution<int> coin_flip(0, 1);
            Literal l1 = coin_flip(rng) ? vars[0] : -vars[0];
            Literal l2 = coin_flip(rng) ? vars[1] : -vars[1];
            Literal l3 = coin_flip(rng) ? vars[2] : -vars[2];

            // 3. Enforce strict unique clauses
            std::vector<int> clause_key = {l1, l2, l3};
            if (unique_clauses.find(clause_key) == unique_clauses.end()) {
                unique_clauses.insert(clause_key);
                clause_set.push_back({l1, l2, l3});
            }
        }

        return ThreeSatInstance(clause_set);
    }
};

// Creates a random 3-SAT instance, computes all exact solutions,
// builds a quantum state |Q> with probability p of measuring a satisfying assignment,
// and decomposes it into block spheres using recursiveApplyY.
// Returns the rotation angles and the number of solution to the 3-SAT instance.
std::pair<RecursiveThetaMap, int> generateThreeSatQuantumExperiment(int num_variables,
                                                                    double clause_ratio,
                                                                    double solution_probability,
                                                                    bool verbose = true) {
    
    if (num_variables < 1) {
        throw std::invalid_argument("num_variables must be >= 1");
    }
    if (clause_ratio <= 0.0) {
        throw std::invalid_argument("clause_ratio must be > 0");
    }
    if (solution_probability <= 0.0 || solution_probability > 1.0) {
        throw std::invalid_argument("solution_probability must be in the interval (0, 1]");
    }

    int num_clauses = static_cast<int>(std::ceil(num_variables * clause_ratio));
    ThreeSatGenerator generator;
    ThreeSatInstance instance = generator.generate(num_variables, num_clauses);

    /*
    std::cout << "--- Generated Random 3-SAT Instance ---\n";
    std::cout << "Variables: " << num_variables << "\n";
    std::cout << "Clauses: " << num_clauses << "\n";
    */
    if (verbose) {
        std::cout << "Clause list:\n";
        instance.printDimacs();
    }
    

    // Try to compute all solutions, retrying up to 3 times if none are found
    std::vector<long long> solutions;
    int attempts = 0;
    do {    
        solutions = instance.computeAllSolutions(verbose);
        attempts++;
    } while (solutions.empty() and attempts < 3);

    int solution_count = static_cast<int>(solutions.size());
    
    if (solution_count == 0) {
        return {RecursiveThetaMap(), 0};
    }

    // Initialize the quantum state vector |Q> with the appropriate size (2^N)
    long long state_size = 1LL << num_variables;
    std::vector<double> Q(state_size, 0.0);

    // Each satisfying assignment should together account for `solution_probability` total.
    // We spread that probability evenly across all found solutions.
    double per_solution_prob = solution_probability / static_cast<double>(solutions.size());
    double total_solution_prob = per_solution_prob * static_cast<double>(solutions.size());

    // The remaining probability mass is assigned uniformly across all non-solution basis states.
    // Note: if there are N total states and S satisfying states, there are N-S non-solution states.
    // The probability for each non-solution basis state is therefore (1 - total_solution_prob) / (N-S).
    double non_solution_prob = (1.0 - total_solution_prob) / static_cast<double>(state_size - solutions.size());

    // Convert probability to amplitude by taking the square root.
    double solution_amplitude = std::sqrt(per_solution_prob);
    double non_solution_amplitude = non_solution_prob > 0.0 ? std::sqrt(non_solution_prob) : 0.0;

    // Populate the quantum state vector |Q> with the computed amplitudes.
    for (long long sol_index : solutions) {
        Q[sol_index] = solution_amplitude;
    }

    if (non_solution_prob > 0.0) {
        for (long long idx = 0; idx < state_size; ++idx) {
            if (Q[idx] == 0.0) {
                Q[idx] = non_solution_amplitude;
            }
        }
    }


    if (verbose) {
        printState("Quantum state |Q>", Q);
    }
    
    RecursiveThetaMap theta_map;
    // Decompose the quantum state |Q> into block sphere rotations using the recursiveApplyY function.
    recursiveApplyY(Q, num_variables, theta_map, verbose);

    if (verbose) {
        std::cout << "--- Returned block sphere decomposition angles ---\n";
        for (auto const& [key, angle] : theta_map) {
            std::cout << "target=" << key.second << " J=" << key.first << " theta="
                    << std::fixed << std::setprecision(6) << angle << "\n";
        }
    }

    return {theta_map, solution_count};
}



void executeThreeSatQuantumExperiment(int min_qubits, int max_qubits, double clause_ratio,
                                      const std::vector<double>& solution_probabilities,
                                      int number_of_iterations, std::string csv_filename, 
                                      std::string avg_csv_filename, 
                                      bool verbose = true) {
    std::ofstream csv_file(csv_filename, std::ios::trunc);
    

    if (!csv_file) {
        throw std::runtime_error("Unable to open CSV file for writing: " + csv_filename);
    }

    csv_file << "num_qubits,clause_ratio,solution_probability,solution_count,theta_angle_count\n";
    std::ofstream avg_csv_file(avg_csv_filename, std::ios::trunc);

    if (!avg_csv_file) {
        throw std::runtime_error("Unable to open average CSV file for writing: " + avg_csv_filename);
    }
    avg_csv_file << "num_qubits,solution_probability,average_theta_angles\n";

    if (min_qubits >max_qubits or min_qubits < 1) {
        min_qubits = max_qubits;
    }
    for (int N = min_qubits; N <= max_qubits; ++N) {
        
        for (double p : solution_probabilities) {
            long long total_theta_angles = 0;
            std::cout << "\n--- Running 3-SAT Quantum Experiment ---\n";
                std::cout << "Qubits (N): " << N << ", Clause Ratio: " << clause_ratio
                        << ", Solution Probability: " << p << "\n";
            
            int actual_iteration = 0; // To count only iterations with solutions
            for (int iter = 0; iter < number_of_iterations; ++iter) {
                auto experiment_result = generateThreeSatQuantumExperiment(N, clause_ratio, p, verbose);
                RecursiveThetaMap theta_map = experiment_result.first;
                int solution_count = experiment_result.second;

                
                if (solution_count > 0) { // Only log to CSV if there are solutions
                    csv_file << N << ',' << std::fixed << std::setprecision(3) << clause_ratio << ','
                        << p << ',' << solution_count << ',' << theta_map.size() << '\n';
                    total_theta_angles += theta_map.size();
                    actual_iteration++;
                }
                

            }
        
            avg_csv_file << N << ',' << p << ',' << static_cast<double>(total_theta_angles) / actual_iteration << '\n';
            std::cout << "---------------------------------------------\n"; 

        }
    }

    csv_file.close();
    avg_csv_file.close();
    std::cout << "Results written to " << csv_filename << "\n";
}


int main() {
    try {
        int min_qubits = 6;
        int max_qubits = 22;
        const double clause_ratio = 3.5;
        std::vector<double> solution_probabilities = {0.2,0.5, 0.75, 0.9,0.95, 1.0 };  
        
        const std::string csv_filename = "three_sat_experiment_results.csv";
        const std::string avg_csv_filename = "three_sat_experiment_avg_results.csv";
        int number_of_iterations = 300;
    
        executeThreeSatQuantumExperiment(min_qubits, max_qubits, clause_ratio, solution_probabilities, 
                                        number_of_iterations, csv_filename, avg_csv_filename, false);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Experiment failed with exception: " << ex.what() << "\n";
        return 1;
    }
}


//Command line used on Ubuntu 22.04 to compile the project:
//g++ -std=c++17 -O2 -Wall -Wextra thresat_prod_1.0.cpp controlled_R_y_target_N.cpp controlled_R_y_target_N_helpers.cpp -o t