# bloch_spheres_decomposition_of_threesat_solutions
C++ code to compute the Bloch spheres decomposition of a quantum system representin all the solutions of a 3-Sat problem.

The file prompt.txt contains the prompts used to generate the C++ code by using Gemini AI.

Them main file is "threesat_prod_1.0.cpp". It contains
* the 3-SAT Instance Class
* the 3-SAT Instance Generator Class
* the function to generate the experiments
* the function to execute the experiment and save the resuts

By lazyness I did not create a command line stand alnoe program. Give the size of the software all the parameters can be 
set in the source code in the main file. The compilers command line is 

//Command line used on Ubuntu 22.04 to compile the project:
> g++ -std=c++17 -O2 -Wall -Wextra threesat_prod_1.0.cpp controlled_R_y_target_N.cpp controlled_R_y_target_N_helpers.cpp -o t
