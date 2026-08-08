#pragma once

#include <cmath>
#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>


inline constexpr double PI = std::acos(-1.0);

double verifyNormalization(const std::vector<double>& state);
std::vector<double> generateRandomQuantumState(int num_qubits);
void printState(const std::string& name, const std::vector<double>& state);
void printState(const std::vector<double>& Q);
void printAngles(const std::map<int, double>& angles);
void checkStateNorm(const std::vector<double>& Q);
void testTwoQubitUncontrolledRotation();
void testDistinctAmplitudeSystems();
void testStateTransformationY(std::vector<double> Q1, int N);
int old_test_1();
int old_test();
std::vector<double> extractEvenIndices(const std::vector<double>& Q1, int N);

typedef std::map<std::pair<int, int>, double> RecursiveThetaMap;
void recursiveApplyY(std::vector<double>& Q, int N, RecursiveThetaMap& theta_map, bool verbose = true);

std::vector<double>& applyControlledRy_naif(std::vector<double>& Q, int N, int J_mask, double theta);
std::vector<double>& applyControlledRy(std::vector<double>& Q, int N, int J_mask, double theta);
std::map<int, double> stateTransformationY(std::vector<double>& Q1, int N, bool verbose= true);
int testApplyControlledRyConsistency(int num_trials = 100, int max_qubits = 6);
