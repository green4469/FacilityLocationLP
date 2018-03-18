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
	for (int i = 0; i < NUM_OF_F; i++)
		opening_table[i] = 0;

	for (int j = 0; j < NUM_OF_C; j++) {
		double min = 99999999;
		int min_client = order_of_client[j];
		int min_facility, min_facility_client;
		for (int i = 0; i < NUM_OF_F; i++) {
			if (connection_variable[min_client * NUM_OF_F + i] > 0 && exponential_clock[i] < min) {
				min_facility = i;
				min = exponential_clock[i];
			}
		}
		min = 999999999;
		for (int i = 0; i < NUM_OF_C; i++) {
			if (connection_variable[i * NUM_OF_F + min_facility] > 0 && order_of_client[i] < min) {
				min_facility_client = i;
				min = order_of_client[i];
			}
		}

		if (min_client == min_facility_client) {  // if j == j'
			opening_table[min_facility] = 1;  // open i
			connection_table[min_client * NUM_OF_F + min_facility] = 1;  // connect j to i
		}
		else {  // connect j to the same facility as j'
			int min_facility_client_facility = 0;
			for (int i = 0; i < NUM_OF_F; i++) {  // find the facility connected to j'(min_client_facility)
				if (connection_table[min_facility_client * NUM_OF_F + i] == 1)
					min_facility_client_facility = i;
			}
			connection_table[j * NUM_OF_F + min_facility_client_facility] = 1;
		}
	}

	/* Calculate Cost ( objective function ) */
	bool tmp_opening_table[NUM_OF_F] = { 0 };
	int total_opening_cost = 0, total_connection_cost = 0;

		/* calculate total connection_cost */
	for (int i = 0; i < NUM_OF_C; i++) {  // calculate total connection_cost
		for (int j = 0; j < NUM_OF_F; j++) {
			total_connection_cost += connection_table[i * NUM_OF_F + j] * connection_cost[i * NUM_OF_F + j];
			tmp_opening_table[j] = connection_table[i * NUM_OF_F + j];
		}
	}

		/* calculate total opening cost */
	for (int i = 0; i < NUM_OF_F; i++) {
		total_opening_cost += tmp_opening_table[i] * opening_cost[i];
	}

	rounded_cost = total_opening_cost + total_connection_cost;
}

int myrandom(int i) { return std::rand() % i; }
FacilityLocation::FacilityLocation(void)
{
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
		order_of_client[i] = *it;


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
	/*
	for (int i = 0; i < NUM_OF_C; i++) {
	cout << "c" << i << ": " << order_of_client[i] << endl;
	}
	for (int i = 0; i < NUM_OF_F; i++) {
	cout << "f" << i << ": " << exponential_clock[i] << endl;
	}
	cout << "---------cost---------------" << endl;

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
	for (int i = 0; i < NUM_OF_C; i++) {
		for (int j = 0; j < NUM_OF_F; j++) {
			total_connection_cost += connection_table[i * NUM_OF_F + j] * (fcl->connection_cost[i * NUM_OF_F + j]);
			opening_table[j] = connection_table[i * NUM_OF_F + j];
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
		for (int i = 0; i < NUM_OF_C; i++)
			for (int j = 0; j < NUM_OF_F; j++)
				fcl->optimal_connection_table[i * NUM_OF_F + j] = connection_table[i * NUM_OF_F + j];
	}

	fcl->optimal_cost = total_opening_cost + total_connection_cost;
}

void recursive_func(bool *connection_table, int index, FacilityLocation *fcl, int *min)
{
	if (index >= NUM_OF_C)	return;  // base case
	for (int i = 0; i < NUM_OF_F; i++) {
		connection_table[index * NUM_OF_F + i] = 1;
		recursive_func(connection_table, index + 1, fcl, min); // recursive call
		calculate_func(connection_table, fcl, min);
		connection_table[index * NUM_OF_F + i] = 0;
	}
}

void FacilityLocation::brute_force(void)
{
	/* check all possible solutions */
	/* find the optimal solution : min(opening cost + connection cost) */
	/* save the optimal solution to 'optimal_opening_table', 'optimal_connection_table' */

	bool connection_table[NUM_OF_C * NUM_OF_F] = { 0 };
	int min = 999999999;

	recursive_func(connection_table, 0, this, &min);
	cout << "Optimal Solution: " << min << endl;
}

unsigned int FacilityLocation::objective(bool optimal = 0)
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