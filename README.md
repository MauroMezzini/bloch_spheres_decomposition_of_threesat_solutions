# Bloch_spheres_decomposition_of_threesat_solutions
C++ code to compute the Bloch spheres decomposition of a quantum system representing all the solutions of a 3-Sat problem.

The file prompt.txt contains the prompts used to generate the C++ code by using Gemini AI. The prompts are separated by 
a small description, which was not used in the AI app, enclosed between two rows of "%"

The main file is "threesat_prod_1.0.cpp". It contains
* the 3-SAT Instance Class
* the 3-SAT Instance Generator Class
* the function to generate the experiments
* the function to execute the experiments and save the resuts

By lazyness I did not create a command line stand alone program. Given the size of the software and consequently the small time to compilation,
all the parameters can be set in the source code in the main file. The compilers command line is 

//Command line used on Ubuntu 22.04 to compile the project:
> g++ -std=c++17 -O2 -Wall -Wextra threesat_prod_1.0.cpp controlled_R_y_target_N.cpp controlled_R_y_target_N_helpers.cpp -o t

In the file controlled_R_y_target_N.cpp there are the functions to decompose an arbitrary quantum system $$\ket{Q}$$ in a set of block spheres

* The functions "applyControlledRy_naif" and "applyControlledRy" make the same job of applying to a quantum system $$\ket{Q}$$ a 
controlled Y rotation on the last $$N$$-th qubit, but the latter is time optimized.
* The function "stateTransformationY" take the quantum system $$\ket{Q}$$ with $$N$$ qubits and transform it in a quantum system $$\ket{Q_1}\ket{0}$$
where $$\ket{Q_1}$$ has $$N-1$$ qubits. It returns the set of bloch spheres necessary to the transformation.
* The function "recursiveApplyY" recursively apply the function "stateTransformationY" until the system becomes
$$\ket{0}^{\otimes N}$$. Returns the total set of bloch spheres necessary to the decomposition.

In the file controlled_R_y_target_N_helpers.cpp there are some helper functions and some functions used during the debug and the test of all the produced code.

The data output consistes in two csv files. The file three_sat_experiment_avg_results.csv contains for each number N of qubits and 
for each probability $$p=\{0.2,0.5,0.75, 0.9, 0.95, 1.0\}$$ the average number of bloch sphere necessary to decompose the quantum system containing all the solutions
of a random instance of the 3-Sat problem with $$N$$ variable and a ratio between clauses and variables equal to 3.5. This ratio generates random instances of
the 3-Sat problem containing an exponentially low number of solutions with respect to $$2^N$$.
The probability $$p$$ sets uniformly the amplitudes of all the components of the state vector corresponding to the solution of the 3-Sat problem  in a way that the 
probability of measuring a solution of the 3-Sat problem is equal to $$p$$. It also set uniformly the aplitudes of all the components of the state vector not 
corresponding to the solution of the 3-Sat problem  in a way that the probability of measuring not a solution of the 3-Sat problem is equal to $$1-p$$.


# Conclusions
In all the experiments the average number of bloch spheres necessary to decompose a quantum system of $$N$$ qubits representing
all the solutions of a 3-Sat instance is quite smaller than $$2^N$$. Nevertheless it increases exponentially with complexity $$O(a^N)$$ with $$a \approx 1.65$$ for all $$p<1.0$$. 
Even when $$p=1.0$$ the number of bloch spheres increases exponetially with $$N$$ with complexity $$O(b^N)$$ where $$b \approx 1.23$$
