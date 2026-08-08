#include "controlled_R_y_target_N.hpp"

#include <iostream>
#include <random>

/**
 * Performs a controlled Ry(theta) gate on a state vector of N qubits on the target qubit N.
 * This is a naif, and therefore slower version that cycle throw all 2^(N-1) prefixes to find the one that satisfy the control condition J.
 * 
 * 
 * @param Q      The state vector of size 2^N (passed by reference to modify in-place).
 * @param N      The number of qubits.
 * @param J_mask An integer representing the control set J as a bitmask.
 *               Bit i (1-indexed) must be set in the state index for the rotation to occur.
 * @param theta  The angle of rotation.
 * @return       The modified state vector Q.
 */
std::vector<double>& applyControlledRy_naif(std::vector<double>& Q, int N, int J_mask, double theta) {
    // 2^N is the size of the state vector
        
    double cos_t = std::cos(theta);
    double sin_t = std::sin(theta);

    // We iterate over all pairs of states (2x, 2x + 1).
    // Here, x is the prefix formed by the N-1 control qubits.
    long long num_prefixes = 1LL << (N - 1);

    for (long long x = 0; x < num_prefixes; ++x) {
        // Map the prefix 'x' to its actual position in the N-qubit space by shifting left by 1.
        // This leaves LSB (bit 0, target qubit N) as 0.
        long long idx_0 = x << 1; 
        long long idx_1 = idx_0 + 1;

        // Check if the control condition J is satisfied.
        // Since J is a subset of {1, ..., N-1}, we check if all bits set in J_mask
        // are also set in our state index.
        // (idx_0 & J_mask) == J_mask ensures J is a subset of S(idx_0).
        if ((idx_0 & J_mask) == J_mask) {
            double q0 = Q[idx_0];
            double q1 = Q[idx_1];

            // Apply Ry(theta) rotation matrix to the state pair note that the 
            // rotation matrix in this case is:
            // [  cos(theta)  -sin(theta) ]
            // [  sin(theta)   cos(theta) ]
            Q[idx_0] = cos_t * q0 - sin_t * q1;
            Q[idx_1] = sin_t * q0 + cos_t * q1;
        }
    }

    return Q;
}


/**
 * Performs a controlled Ry(theta) gate on a state vector of N qubits on the target qubit N.
 * This version is optimized to avoid unnecessary checks and directly computes the indices based on the control set J_mask.
 * 
 * @param Q      The state vector of size 2^N (passed by reference to modify in-place).
 * @param N      The number of qubits.
 * @param J_mask An integer representing the control set J as a bitmask. The dimension of J_mask is N. The last bit (bit 0) is the target qubit and should always be 0. 
 *               The other bits (1 to N-1) represent the control qubits.
 *               Bit i (1-indexed) must be set in the state index for the rotation to occur.
 * @param theta  The angle of rotation.
 * @return       The modified state vector Q.
 **/
std::vector<double>& applyControlledRy(std::vector<double>& Q, int N, int J_mask, double theta) {
    // 2^N is the size of the state vector
        
    double cos_t = std::cos(theta);
    double sin_t = std::sin(theta);

    // The full mask for N-1 qubits is (1 << (N - 1)) - 1, which gives us all bits set for the 
    // first N-1 qubits.
    uint64_t full_mask = (N == 64) ? ~0ULL : ((1ULL << (N - 1)) - 1);
    //std::cout << "N: " << N << ", J_mask: " << J_mask << "\n";
    
    // Iterate over all subsets U of the full mask that include J_mask
    // Adding 1 to U makes the rightmost zero bit of U become 1, and all bits to the right of it become 0.
    // The bitwise OR with (J_mask >> 1) ensures that all bits set in J_mask are also set in U.
    // This loop will generate all and only prefixes U that contains all the bits set to one in J_mask.

    for (uint64_t U = J_mask >> 1; U <= full_mask; U = (U + 1) | (J_mask>>1)) {
        // Compute the indices for the pair of states to be rotated.
        // The target qubit is the last qubit (N-th qubit), so we shift K left by 1 to make space for it.        
        long long idx_0 = U<<1; 
        long long idx_1 = idx_0 + 1;

        
        double q0 = Q[idx_0];
        double q1 = Q[idx_1];

        // Apply Ry(theta) rotation matrix to the state pair note that the 
        // rotation matrix in this case is:
        // [  cos(theta)  -sin(theta) ]
        // [  sin(theta)   cos(theta) ]
        Q[idx_0] = cos_t * q0 - sin_t * q1;
        Q[idx_1] = sin_t * q0 + cos_t * q1;
        
        // Avoid infinite loop when U reaches full_mask
        if (U == full_mask) break;
    } 

    return Q;
}



/**
 * State Vector Transformation Procedure Y
 * Computes the angles theta_{J,N} for all subsets J of {1, ..., N-1} that are necessary to transform the quantum state |Q1> into |Q1'>|0>
 * in which the last qubit (N-th qubit) is in the |0> state.
 * Essentyially, we need to make all the componnts of state vector |Q1> with odd indices (2x+1) equal to zero by using
 * appropriate unitary rotations R_y(theta).
 * 
 * @param Q1 The initial state vector (size 2^N). Will be modified in-place.
 * @param N  The number of variables/qubits.
 * @return A map of bitmasks J -> theta_{J,N}
 */
std::map<int, double> stateTransformationY(std::vector<double>& Q1, int N, bool verbose) {
    std::map<int, double> theta; 
    
    // -----------------------------------------------------
    // Step 2-6: Handle the empty set condition (J = empty/0)
    // -----------------------------------------------------
    if (std::abs(Q1[1])> 1e-6) {// if Q1[1] is non-zero, compute the angle of the block sphere
        int empty_set = 0; 
        if (std::abs(Q1[0]) > 1e-6) {  //if Q1[0] is not zero, compute the angle for the empty set
            theta[empty_set] = -2.0 * std::atan(Q1[1] / Q1[0]);
        } else {
            theta[empty_set] = -PI;
        }

        // Step 7: Apply Ry(theta/2) controlled by empty set
        
        applyControlledRy(Q1, N, empty_set, theta[empty_set] / 2.0);
        if (verbose) {
            std::cout << "Applying controlled Ry for J = " << empty_set 
                    << " with theta = " << theta[empty_set] / 2.0 << " rad\n";
            printState(Q1);
        }
    }
    
    
    // -----------------------------------------------------
    // Step 8-15: Iterate over all possible subsets using integer x
    // -----------------------------------------------------
    // 2^{N-1} is the upper limit for x
    int max_x = 1 << (N - 1); 
    
    for (int x = 1; x < max_x; ++x) {
        // Step 10: x directly serves as the bitmask for subset J
        // J is represented as an integer where each bit corresponds to a qubit in {1, ..., N-1}
        // We need to multiply by two because we are working on the first N-1 qubits and
        // the target qubit is the last one (N-th qubit).
        int J = 2 * x; 
        
        int idx_2x = 2 * x;
        int idx_2x_plus_1 = 2 * x + 1;

        // Step 11-15: Compute rotation angle with explicit negative sign
        if (std::abs(Q1[idx_2x_plus_1])> 1e-6) {  // if Q1[2x+1] is non-zero, compute the angle of the block sphere
            // If the amplitude for Q1[2x+1] is non-zero compute
            // the angle of the block sphere.
            // Otherwise if Q1[idx_2x_plus_1]==0 the block-sphere is not necessary
            if (Q1[idx_2x] != 0.0) {
                theta[J] = -std::atan(Q1[idx_2x_plus_1] / Q1[idx_2x]);
            } else {
                theta[J] = -PI / 2.0;
            }
            // Step 16: Apply Ry(theta) controlled by J
            applyControlledRy(Q1, N, J, theta[J]);
            if (verbose) {
                std::cout << "Applying controlled Ry for J = " << J << " with theta = " << theta[J] << " rad\n";
                printState(Q1);
            }
            
        }        
    }

    // Step 18: Return the angles
    return theta;
}


// Type definition for a map that uses a pair (J, target_qubit) as the key
typedef std::map<std::pair<int, int>, double> RecursiveThetaMap;

/**
 * Recursively applies stateTransformationY to a quantum system |Q> recursively. It applys the
 * stateTransformationY algorithm to the current N-qubit system, stores the resulting angles in a cumulative map,
 * then the system becomes a new system of N-1 qubits by extracting the amplitudes with even indices, and the process repeats
 * until N=1.
 * 
 * @param Q           The current state vector (modified in-place by transformations).
 * @param N           The current number of qubits.
 * @param theta_map   The accumulated map of (J, target_qubit) -> angle.
 */
void recursiveApplyY(std::vector<double>& Q, int N, RecursiveThetaMap& theta_map, bool verbose ) {
    // Base case: the recursion continues until the algorithm is applied for N=1.
    if (N < 1) {
        return;
    }

    // 1. Apply the stateTransformationY algorithm to the current N-qubit system.
    // This modifies Q in-place and returns the angles for the current target qubit N [1].
    std::map<int, double> current_angles = stateTransformationY(Q, N, verbose);

    // 2. Store the results in the cumulative map. 
    // The target qubit for this iteration is N [1].
    for (auto const& [J, angle] : current_angles) {
        theta_map[{N, J}] = angle;
    }

    // 3. If N == 1, we have reached the final qubit and stop the recursion.
    if (N == 1) {
        return;
    }

    // 4. Produce a new system Q2 of N-1 qubits using amplitudes with even indices.
    // The size of a system with N-1 qubits is 2^(N-1) [1, 2].
    long long size_next = 1LL << (N - 1);
    //printState(Q);
    
    std::vector<double> Q_next(size_next);
    for (long long i = 0; i < size_next; ++i) {
        // Amplitude extraction at even indices (0, 2, 4, ...) from the transformed Q.
        Q_next[i] = Q[2 * i];
    }
    if (verbose) {
        printState("Q_next", Q_next);
    }
    

    // 5. Recursively apply the process to the new (N-1)-qubit system.
    recursiveApplyY(Q_next, N - 1, theta_map, verbose);
}


/**
 * Helper to print the resulting angles in a readable format.
 */
void printRecursiveAngles(const RecursiveThetaMap& theta_map) {
    std::cout << "--- Angles for Uniform Superposition ---\n";
    std::cout << "Target Qubit |  J (Mask)  |  Angle (rad)\n";
    std::cout << "----------------------------------------\n";
    for (auto const& [key, angle] : theta_map) {
        int target_qubit = key.first;
        int J = key.second;
        std::cout << std::setw(12) << target_qubit << " | " 
                  << std::setw(10) << J << " | " 
                  << std::fixed << std::setprecision(6) << angle << "\n";
    }
}



