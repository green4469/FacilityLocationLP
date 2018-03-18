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
using namespace std;

#define NUM_OF_F 100
#define NUM_OF_C 100

class FacilityLocation {
private:
	/* Rounded problem's objective function's cost */
	double rounded_cost;

	/* Original problem's objective function's optimal cost */
	double optimal_cost;

	/* Input of LP-solver */
	double opening_cost[NUM_OF_F];
	double connection_cost[NUM_OF_C * NUM_OF_F];

	/* output of LP-solver */
	double opening_variable[NUM_OF_F];
	double connection_variable[NUM_OF_C * NUM_OF_F];

	/* exponential clocks of facilities */
	double exponential_clock[NUM_OF_F];

	/* the order of the exponential clocks of the clients by ascending */
	int clock_of_client[NUM_OF_C];

	/* output of Rounding Algorithm */
	bool opening_table[NUM_OF_F];
	bool connection_table[NUM_OF_C * NUM_OF_F];

	/* output of Brute-force Algorithm */
	bool optimal_opening_table[NUM_OF_F];
	bool optimal_connection_table[NUM_OF_C * NUM_OF_F];

public:
	/* constructor, inside it initialize the oppening cost, connection cost, clients' clocks, facilities' clocks */
	// ÇÐ¼ö
	FacilityLocation();

	/* solve the LP-relaxed facility location problem */
	// ¸íÀå
	double LP_solve();

	/* round the LP-relaxed solution to the original problem's solution */
	// À¯¹Î
	void round();

	/* check all possible solutions and pick the minimum cost (brute-force) */
	// À¯¹Î
	void brute_force();

	friend void calculate_func(bool *connection_table, FacilityLocation *fcl, double *min);

	/* compare LP rounded solution and optimal solution */
	// ¸íÀå
	double objective(bool optimal = 0);
	
	double get_optimal_cost() {
		return this->optimal_cost;
	}

	double get_rounded_cost() {
		return this->rounded_cost;
	}

	double * get_connection_cost() {
		return this->connection_cost;
	}

	double * get_opening_cost() {
		return this->opening_cost;
	}

	bool * get_optimal_connection_table() {
		return this->optimal_connection_table;
	}

	bool * get_optimal_opening_table() {
		return this->optimal_opening_table;
	}

	double * get_opening_variable() {
		return this->opening_variable;
	}

	double * get_connection_variable() {
		return this->connection_variable;
	}

	bool * get_opening_table() {
		return this->opening_table;
	}

	bool * get_connection_table() {
		return this->connection_table;
	}

	double * get_exponential_clock() {
		return this->exponential_clock;
	}

	int * get_clock_of_client() {
		return this->clock_of_client;
	}
};

class Facility {
private:
	int x, y;

public:
	bool operator==(Facility _f) {
		if (this->get_x() == _f.get_x() && this->get_y() == _f.get_y()) return true;
		else return false;
	}
	int get_x() {return x;}
	int get_y() {return y;}
	void set_x(int _x) { x = _x; }
	void set_y(int _y) { y = _y; }
};

class Client{
private:
	int x, y;

public:
	bool operator==(Client _c) {
		if (this->get_x() == _c.get_x() && this->get_y() == _c.get_y())	return true;
		else return false;
	}
	int get_x() { return x; }
	int get_y() { return y; }
	void set_x(int _x) { x = _x; }
	void set_y(int _y) { y = _y; }

	
};