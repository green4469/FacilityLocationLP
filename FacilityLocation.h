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
#include <math.h>
#include <numeric>		// std::iota
using namespace std;

#define NUM_OF_F 100
#define NUM_OF_C 1000
class FacilityLocation {
private:
	/* Rounded problem's objective function's cost */
	double rounded_cost;

	/* Original problem's objective function's optimal cost */
	double optimal_cost;

	
	/* Input of LP-solver */
	//double opening_cost[NUM_OF_F];
	//double connection_cost[NUM_OF_F][NUM_OF_C];

	/* output of LP-solver */
	//double opening_variable[NUM_OF_F];
	//double connection_variable[NUM_OF_F][NUM_OF_C];

	/* exponential clocks of facilities */
	//double exponential_clock[NUM_OF_F][NUM_OF_C];

	/* the order of the exponential clocks of the clients by ascending */
	//int clock_of_client[NUM_OF_C];

	/* Preprocessing */
	//double copied_opening_cost[NUM_OF_F][NUM_OF_C];  // f'
	//double copied_connection_cost[NUM_OF_F][NUM_OF_C][NUM_OF_C];  // d'
	//double copied_opening_variable[NUM_OF_F][NUM_OF_C];  // y'
	//double copied_connection_variable[NUM_OF_F][NUM_OF_C][NUM_OF_C];  // x'
	//bool copied_opening_table[NUM_OF_F][NUM_OF_C];  // M
	//bool copied_connection_table[NUM_OF_F][NUM_OF_C][NUM_OF_C];  // M'

	/* output of Rounding Algorithm */
	//bool opening_table[NUM_OF_F];
	//bool connection_table[NUM_OF_F][NUM_OF_C];

	/* output of Brute-force Algorithm */
	//bool optimal_opening_table[NUM_OF_F];
	//bool optimal_connection_table[NUM_OF_F][NUM_OF_C];

	/* ---------------dynamic allocation version--------------------- */
	/* Input of LP-solver */
	double *opening_cost; // [NUM_OF_F] check
	double **connection_cost; //[NUM_OF_F][NUM_OF_C]; check

	/* output of LP-solver */
	double* opening_variable; //[NUM_OF_F]; check
	double** connection_variable; // [NUM_OF_F][NUM_OF_C]; check

	/* exponential clocks of facilities */
	double** exponential_clock; // [NUM_OF_F][NUM_OF_C];  check

	/* the order of the exponential clocks of the clients by ascending */
	int* clock_of_client; // [NUM_OF_C];   check

	/* Preprocessing */
	double** copied_opening_cost; // [NUM_OF_F][NUM_OF_C];  // f' check
	double*** copied_connection_cost; // [NUM_OF_F][NUM_OF_C][NUM_OF_C];  // d' check
	double** copied_opening_variable; // [NUM_OF_F][NUM_OF_C];  // y' check
	double*** copied_connection_variable; // [NUM_OF_F][NUM_OF_C][NUM_OF_C];  // x' check
	bool** copied_opening_table; // [NUM_OF_F][NUM_OF_C];  // M check
	bool*** copied_connection_table; // [NUM_OF_F][NUM_OF_C][NUM_OF_C];  // M' check

																 /* output of Rounding Algorithm */
	bool* opening_table; // [NUM_OF_F]; // check
	bool** connection_table; // [NUM_OF_F][NUM_OF_C]; check

	/* output of Brute-force Algorithm */
	bool* optimal_opening_table; // [NUM_OF_F]; check
	bool** optimal_connection_table; // [NUM_OF_F][NUM_OF_C]; check


public:
	/* constructor, inside it initialize the oppening cost, connection cost, clients' clocks, facilities' clocks */
	// �м�
	FacilityLocation();

	/* solve the LP-relaxed facility location problem */
	// ����
	double LP_solve();

	/* round the LP-relaxed solution to the original problem's solution */
	// ����
	void round();

	/* check all possible solutions and pick the minimum cost (brute-force) */
	// ����
	void brute_force();

	friend void calculate_func(bool *connection_table, FacilityLocation *fcl, double *min);

	/* compare LP rounded solution and optimal solution */
	// ����
	double objective(bool optimal = 0);

	double get_optimal_cost() {
		return this->optimal_cost;
	}

	double get_rounded_cost() {
		return this->rounded_cost;
	}
	
	double ** get_connection_cost(){
		return this->connection_cost;
	}

	double * get_opening_cost() {
		return this->opening_cost;
	}

	bool ** get_optimal_connection_table(){
		return this->optimal_connection_table;
	}

	bool * get_optimal_opening_table() {
		return this->optimal_opening_table;
	}

	double * get_opening_variable() {
		return this->opening_variable;
	}

	double ** get_connection_variable(){
		return this->connection_variable;
	}

	bool * get_opening_table() {
		return this->opening_table;
	}

	bool ** get_connection_table(){
		return this->connection_table;
	}

	double ** get_exponential_clock() {
		return this->exponential_clock;
	}

	int * get_clock_of_client() {
		return this->clock_of_client;
	}
};

int CompareDoubleAbsolute(double x, double y, double absTolerance = (1.0e-8));
