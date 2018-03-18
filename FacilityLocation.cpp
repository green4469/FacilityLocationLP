#include "FacilityLocation.h"


double FacilityLocation::LP_solve(void)
{
	IloEnv env;

	/* set and initialize connection variables */
	IloNumVar x[NUM_OF_C * NUM_OF_F];
	for (int i = 0; i < NUM_OF_F; ++i) {
		for (int j = 0; j < NUM_OF_C; ++j) {
			x[i*NUM_OF_C + j] = IloNumVar(env, 0, IloInfinity);
		}
	}

	/* set and initialize opening variables */
	IloNumVar y[NUM_OF_F];
	for (int i = 0; i < NUM_OF_F; ++i)
		y[i] = IloNumVar(env, 0, IloInfinity);

	/* set ranges of sums of connection variables (1 <= sum_i(x_ij) <= 1 for all j) */
	IloRange sum_range[NUM_OF_C];
	for (int j = 0; j < NUM_OF_C; ++j) {
		sum_range[j] = IloRange(env, 1, 1);
		for (int i = 0; i < NUM_OF_F; ++i) {
			sum_range[j].setLinearCoef(x[i*NUM_OF_C + j], 1);
		}
	}

	/* set ranges of connection variables and opening variables (-inf <= x_ij - y_i <= 0 for all i, j) */
	IloRange x_range[NUM_OF_C * NUM_OF_F];
	for (int i = 0; i < NUM_OF_F; ++i) {
		for (int j = 0; j < NUM_OF_C; ++j) {
			x_range[i * NUM_OF_C + j] = IloRange(env, -IloInfinity, 0);
			x_range[i * NUM_OF_C + j].setLinearCoef(x[i*NUM_OF_C + j], 1);
			x_range[i * NUM_OF_C + j].setLinearCoef(y[i], -1);
		}
	}

	/* set the obj fct (minimize sum_i(y_i*f_i) + sum_{i,j}(x_ij*d(i,j)) )*/
	IloObjective obj = IloMinimize(env, 0);
	for (int i = 0; i < NUM_OF_F; ++i) {
		obj.setLinearCoef(y[i], this->opening_cost[i]);
		for (int j = 0; j < NUM_OF_C; ++j) {
			obj.setLinearCoef(x[i*NUM_OF_C + j], this->connection_cost[i*NUM_OF_C + j]);
		}
	}

	/* compile the model */
	IloModel model(env);
	for (int j = 0; j < NUM_OF_C; ++j) {
		model.add(sum_range[j]);
		for (int i = 0; i < NUM_OF_F; ++i) {
			model.add(x_range[i*NUM_OF_C + j]);
		}
	}
	model.add(obj);

	/* solve the model */
	IloCplex solver(model);
	try {
		solver.solve();
	}
	catch (IloException &ex) {
		cerr << ex << endl;
	}
	cout << solver.getObjValue() << endl;
	cout << solver.getValue(x[0]) << endl;
	/* save results*/
	for (int i = 0; i < NUM_OF_F; ++i) {
		this->opening_variable[i] = solver.getValue(y[i]);
		for (int j = 0; j < NUM_OF_C; ++j) {
			this->connection_variable[i*NUM_OF_C + j] = solver.getValue(x[i*NUM_OF_C + j]);
		}
	}
	return solver.getObjValue();
}


void FacilityLocation::round(void)
{
	/* Assumes 'opening_variable' and 'connection_variable' are already calculated by LP-solver.
	   Return the rounded solution of above two variables each in 'opening_table', 'connection_table'.
	   Return the rounded solution's objective cost in 'rounded_cost' */

	int order_of_client[NUM_OF_C] = { 0 };

	for (int i = 0; i < NUM_OF_C; i++) {
		for (int j = 0; j < NUM_OF_C; j++) {
			if (clock_of_client[j] == i)
				order_of_client[i] = j;
		}
	}

	for (int i = 0; i < NUM_OF_F; i++)  // M[i] <- 0
		opening_table[i] = 0;

	for (int j = 0; j < NUM_OF_C; j++)  // N[i, j] <- 0
		for (int i = 0; i < NUM_OF_F; i++)
			connection_table[i * NUM_OF_C + j] = 0;

	for (int j = 0; j < NUM_OF_C; j++) {  // j for client, i for facility, j_ for j'
		double min = 99999999;
		int min_client = order_of_client[j];  // pick a client who has the most small  ==>  pick minimum clock element's index
		int min_facility, min_facility_client;
		for (int i = 0; i < NUM_OF_F; i++) {
			double cv = connection_variable[min_client + i * NUM_OF_C];
			bool debug = connection_variable[min_client + i * NUM_OF_C] > 0 && exponential_clock[i] < min;
			if (connection_variable[min_client + i*NUM_OF_C] > 0 && exponential_clock[i] < min) {  // find a facility which is connected to the client and has the smallest clock.
				min_facility = i;
				min = exponential_clock[i];  // update minimum clock
			}
		}
		min = 999999999;
		for (int j_ = 0; j_ < NUM_OF_C; j_++) {
			double cv = connection_variable[j_ + min_facility * NUM_OF_C];
			bool debug = connection_variable[j_ + min_facility * NUM_OF_C] > 0 && clock_of_client[j_] < min;
			if (connection_variable[j_ + min_facility*NUM_OF_C] > 0 && clock_of_client[j_] < min) {
				min_facility_client = j_;
				min = clock_of_client[j_];
			}
		}

		if (min_client == min_facility_client) {  // if j == j'
			opening_table[min_facility] = 1;  // open i
			connection_table[min_client + min_facility*NUM_OF_C] = 1;  // connect j to i
		}
		else {  // connect j to the same facility as j'
			int min_facility_client_facility = 0;
			for (int i = 0; i < NUM_OF_F; i++) {  // find the facility connected to j'(min_client_facility)
				if (connection_table[min_facility_client + i * NUM_OF_C] == 1)
					min_facility_client_facility = i;
			}
			connection_table[min_client + min_facility_client_facility * NUM_OF_C] = 1;
		}
	}

	/* Calculate Cost ( objective function ) */
	int total_opening_cost = 0, total_connection_cost = 0;

	/* calculate total connection_cost */
	for (int j = 0; j < NUM_OF_C; j++) {  // calculate total connection_cost
		for (int i = 0; i < NUM_OF_F; i++) {
			total_connection_cost += connection_table[i * NUM_OF_C + j] * connection_cost[i * NUM_OF_C + j];
		}
	}

	/* calculate total opening cost */
	for (int i = 0; i < NUM_OF_F; i++) {
		total_opening_cost += opening_table[i] * opening_cost[i];
	}

	rounded_cost = total_opening_cost + total_connection_cost;
}

int myrandom(int i) { return std::rand() % i; }
FacilityLocation::FacilityLocation(void)
{
	/*
	clock_of_client[0] = 2;
	clock_of_client[1] = 0;	
	clock_of_client[2] = 1;

	exponential_clock[0] = 2.46;
	exponential_clock[1] = 9.94;

	opening_cost[0] = 90;
	opening_cost[1] = 74;

	connection_cost[0 * NUM_OF_C + 0] = 60;
	connection_cost[0 * NUM_OF_C + 1] = 32;
	connection_cost[0 * NUM_OF_C + 2] = 58;
	connection_cost[1 * NUM_OF_C + 0] = 12;
	connection_cost[1 * NUM_OF_C + 1] = 66;
	connection_cost[1 * NUM_OF_C + 2] = 21;

	opening_variable[0] = (double)0;
	opening_variable[1] = (double)1;

	connection_variable[0 * NUM_OF_C + 0] = double(0);
	connection_variable[0 * NUM_OF_C + 1] = double(0);
	connection_variable[0 * NUM_OF_C + 2] = double(0);
	connection_variable[1 * NUM_OF_C + 0] = double(1);
	connection_variable[1 * NUM_OF_C + 1] = double(1);
	connection_variable[1 * NUM_OF_C + 2] = double(1);
	*/

	/* generation of the expoential clocks of the facilities */
	//double y_i; // facility가 열려있으면 1, 닫혀있으면 0, fraction variable
	std::default_random_engine generator;

	std::srand(unsigned(std::time(0)));

	//각각의 facility의 exponential_clock 값 설정
	for (int i = 0; i < NUM_OF_F; ++i) {
		double y_i = ((double)rand() / (RAND_MAX));
		std::exponential_distribution<double> distribution(y_i);
		//double number = distribution(generator);
		//exponential_clock[i] = y_i*exp(double(-1 * y_i*number));
		exponential_clock[i] = distribution(generator);
	}


	//std::cout << "exponential_distribution(" << y_i << "):" << std::endl;
	//std::cout << std::fixed; std::cout.precision(1);

	/*
	for (int i = 0; i < NUM_OF_F; ++i) {
	//cout << i << " : " << p[i] << endl;
	printf("%lf\n", p[i]);
	}*/

	/* generation of the orders of the cliendts */
	std::vector<int> myvector;

	// set some values:
	for (int i = 0; i < NUM_OF_C; ++i) myvector.push_back(i); // 1 2 3 4 5 6 7 8 9

															  // using built-in random generator:
	std::random_shuffle(myvector.begin(), myvector.end());

	// using myrandom:
	std::random_shuffle(myvector.begin(), myvector.end(), myrandom);

	int i = 0;
	for (std::vector<int>::iterator it = myvector.begin(); it != myvector.end(); ++it, ++i)
		clock_of_client[i] = *it;


	/* settiing costs of openings and connections */
	for (int i = 0, j = 0; i < NUM_OF_F;) {
		connection_cost[i*NUM_OF_C + j] = (int)rand() % 100 + 1;
		j++;
		if (j == NUM_OF_C) {
			i++;
			j = 0;
		}
	}
	for (int i = 0; i < NUM_OF_F; i++) {
		opening_cost[i] = (int)rand() % 100 + 1;
	}

	// print out content:
	
	for (int i = 0; i < NUM_OF_C; i++) {
	cout << "c" << i << ": " << clock_of_client[i] << endl;
	}
	for (int i = 0; i < NUM_OF_F; i++) {
	cout << "f" << i << ": " << exponential_clock[i] << endl;
	}
	cout << "---------cost---------------" << endl;

	/*
	for (int i = 0; i < NUM_OF_F*NUM_OF_C; i++) {
	cout << "c" << i << ": " << connection_cost[i] << endl;
	}

	for (int i = 0; i < NUM_OF_F; i++) {
	cout << "f" << i << ": " << opening_cost[i] << endl;
	}
	*/
}

void calculate_func(bool *connection_table, FacilityLocation *fcl, int *min)
{
	bool opening_table[NUM_OF_F] = { 0 };
	int total_opening_cost = 0, total_connection_cost = 0;

	/* calculate total connection_cost */
	for (int j = 0; j < NUM_OF_C; j++) {
		for (int i = 0; i < NUM_OF_F; i++) {
			total_connection_cost += connection_table[i * NUM_OF_C + j] * (fcl->connection_cost[i * NUM_OF_C + j]);
			opening_table[i] += connection_table[i * NUM_OF_C + j];
		}
	}

	/* calculate total opening cost */
	for (int i = 0; i < NUM_OF_F; i++) {
		total_opening_cost += opening_table[i] * (fcl->opening_cost[i]);
	}

	/* update minimum cost solution */
	if ((total_connection_cost + total_opening_cost) < *min) {
		*min = total_connection_cost + total_opening_cost;
		for (int i = 0; i < NUM_OF_F; i++)
			fcl->optimal_opening_table[i] = opening_table[i];
		for (int j = 0; j < NUM_OF_C; j++)
			for (int i = 0; i < NUM_OF_F; i++)
				fcl->optimal_connection_table[i * NUM_OF_C + j] = connection_table[i * NUM_OF_C + j];

		fcl->optimal_cost = total_opening_cost + total_connection_cost;

	}
}

void recursive_func(bool *connection_table, int index, FacilityLocation *fcl, int *min)
{
	if (index >= NUM_OF_C)	return;  // base case
	for (int i = 0; i < NUM_OF_F; i++) {
		connection_table[i*NUM_OF_C + index] = 1;
		recursive_func(connection_table, index + 1, fcl, min); // recursive call
		calculate_func(connection_table, fcl, min);
		connection_table[i*NUM_OF_C + index] = 0;
	}
	connection_table[index] = 1;
}

void FacilityLocation::brute_force(void)
{
	/* Runing Time O(F^C) */
	/* check all possible solutions */
	/* find the optimal solution : min(opening cost + connection cost) */
	/* save the optimal solution to 'optimal_opening_table', 'optimal_connection_table' */

	bool connection_table[NUM_OF_C * NUM_OF_F] = { 0 };
	int min = 999999999;

	recursive_func(connection_table, 0, this, &min);
}

unsigned int FacilityLocation::objective(bool optimal)
{

	unsigned int sol = 0;
	if (optimal) {
		for (int i = 0; i < NUM_OF_F; ++i) {
			sol += (unsigned int)this->optimal_opening_table[i] * this->opening_cost[i];
			for (int j = 0; j < NUM_OF_C; ++j) {
				sol += (unsigned int)this->optimal_connection_table[i*NUM_OF_C + j] * this->connection_cost[i*NUM_OF_C + j];
			}
		}
	}
	else {
		for (int i = 0; i < NUM_OF_F; ++i) {
			sol += (unsigned int)this->opening_table[i] * this->opening_cost[i];
			for (int j = 0; j < NUM_OF_C; ++j) {
				sol += (unsigned int)this->connection_table[i*NUM_OF_C + j] * this->connection_cost[i*NUM_OF_C + j];
			}
		}

	}
	return sol;
}