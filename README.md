# Bloch_spheres_decomposition_of_threesat_solutions
C++ code to compute the Bloch spheres decomposition of a quantum system representin all the solutions of a 3-Sat problem.

The file prompt.txt contains the prompts used to generate the C++ code by using Gemini AI. The prompts are separated by 
a small description, which was not used in the AI app, enclosed between two rows of "%"

Them main file is "threesat_prod_1.0.cpp". It contains
* the 3-SAT Instance Class
* the 3-SAT Instance Generator Class
* the function to generate the experiments
* the function to execute the experiment and save the resuts

By lazyness I did not create a command line stand alone program. Give the size of the software all the parameters can be 
set in the source code in the main file. The compilers command line is 

//Command line used on Ubuntu 22.04 to compile the project:
> g++ -std=c++17 -O2 -Wall -Wextra threesat_prod_1.0.cpp controlled_R_y_target_N.cpp controlled_R_y_target_N_helpers.cpp -o t

In the file controlled_R_y_target_N.cpp there are the functins to decompose an arbitrary quantum system |Q> in a set of block spheres

* The functions "applyControlledRy_naif" and "applyControlledRy" make the same job of applying to a quantum system |Q> a 
controlled Y rotation on the last N-th qubit, but the latter is time optimized.
* The function "stateTransformationY" take the quantum system |Q> with N qubits and transform it in a quantum system |Q1>|0>
where |Q1> has N-1 qubits. It returns the set of bloch spheres necessary to the transformation.
* The function "recursiveApplyY" recursively apply the function "stateTransformationY" until the system becomes
$\ket{0}^{\otimes N}

In the file controlled_R_y_target_N_helpers.cpp there are some helper functions and some functions used during the debug and the test of all the produced code.

The data output consistes in two csv files. The file three_sat_experiment_avg_results.csv contains for each number N of qubits and 
for each probability p={0.2,0.5,0.75, 0.9, 0.95, 1.0} the average number of bloch sphere necessary to decompose the quantum system containing all the solutions
of a random instance of the 3-Sat problem with N variable and a ratio between clauses and variables equal to 3.5. This ratio generates random instances of
the 3-Sat problem containing an exponentially low number of solutions with respect to 2^N.
