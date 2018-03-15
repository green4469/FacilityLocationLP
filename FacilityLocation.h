#pragma once
#include <iostream>
#include <ilcplex/ilocplex.h>

using namespace std;

#define NUM_OF_F 10
#define NUM_OF_C 100
class FacilityLocation {
private:
	/* Input of LP-solver */
	int opening_cost[NUM_OF_F];
	int connection_cost[NUM_OF_C * NUM_OF_F];

	/* output of LP-solver */
	double opening_variable[NUM_OF_F];
	double connection_variable[NUM_OF_C * NUM_OF_F];

	/* exponential clocks of facilities */
	double exponential_clock[NUM_OF_F];

	/* output of Rounding Algorithm */
	bool opening_table[NUM_OF_F];
	bool connection_table[NUM_OF_C * NUM_OF_F];

	/* output of Brute-force Algorithm */
	bool optimal_opening_table[NUM_OF_F];
	bool optimal_connection_table[NUM_OF_C * NUM_OF_F];

public:
	/* constructor, inside it initialize the oppening cost, connection cost */
	// �м�
	FacilityLocation();

	/* randomly sample expnential clocks of facilities */
	// �м�
	void random_sample();
	
	/* solve the LP-relaxed facility location problem */
	// ����
	void LP_solve();

	/* round the LP-relaxed solution to the original problem's solution */
	// ����
	void round();
	 
	/* check all possible solutions and pick the minimum cost (brute-force) */
	// ����
	void brute_force();

	/* compare LP rounded solution and optimal solution */
	// ����
	void objective();
};