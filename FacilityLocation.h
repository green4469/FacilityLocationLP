#pragma once
#include <iostream>
#include <cstdlib>
#include <ilcplex/ilocplex.h>
#include <random>
#include <string>
#include <algorithm>    // std::random_shuffle
#include <vector>       // std::vector
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
using namespace std;

#define NUM_OF_F 10
#define NUM_OF_C 100
class FacilityLocation {
private:
	/* Input of LP-solver */
	unsigned int opening_cost[NUM_OF_F];
	unsigned int connection_cost[NUM_OF_C * NUM_OF_F];

	/* output of LP-solver */
	double opening_variable[NUM_OF_F];
	double connection_variable[NUM_OF_C * NUM_OF_F];

	/* exponential clocks of facilities */
	double exponential_clock[NUM_OF_F];
	/* the order of the exponential clocks of the clients by ascending*/
	int order_of_client[NUM_OF_C];

	/* output of Rounding Algorithm */
	bool opening_table[NUM_OF_F];
	bool connection_table[NUM_OF_C * NUM_OF_F];

	/* output of Brute-force Algorithm */
	bool optimal_opening_table[NUM_OF_F];
	bool optimal_connection_table[NUM_OF_C * NUM_OF_F];

public:
	/* constructor, inside it initialize the oppening cost, connection cost */
	// 학수
	FacilityLocation();

	/* randomly sample expnential clocks of facilities */
	// 학수
	void random_sample();

	/* solve the LP-relaxed facility location problem */
	// 명장
	double LP_solve();

	/* round the LP-relaxed solution to the original problem's solution */
	// 유민
	void round();

	/* check all possible solutions and pick the minimum cost (brute-force) */
	// 유민
	void brute_force();

	/* compare LP rounded solution and optimal solution */
	// 명장
	unsigned int objective(bool optimal = 0);

};