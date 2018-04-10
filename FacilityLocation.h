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

#define NUM_OF_F 10
#define NUM_OF_C 100
#define CONNECTION_COST_MAX 100


class FacilityLocation {
private:
	int n_facilities;
	int n_clients;

	/* Rounded problem's objective function's cost */
	double rounded_cost;

	/* Original problem's objective function's optimal cost */
	double optimal_cost;

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

public:
	/* constructor, inside it initialize the oppening cost, connection cost, clients' clocks, facilities' clocks */
	FacilityLocation(int argc, char* argv[]);

	/* Deconstructor, delete the dynamically allocated memory of class members. */
	~FacilityLocation();

	/* solve the LP-relaxed facility location problem */
	double LP_solve();
	double get_optimal();

	/* round the LP-relaxed solution to the original problem's solution */
	void round();

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

	int get_n_facilities() {
		return n_facilities;
	}
	int get_n_clients() {
		return n_clients;
	}
};

int CompareDoubleUlps(double x, double y, int ulpsTolerance = 4);
