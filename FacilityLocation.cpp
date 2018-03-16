#include "FacilityLocation.h"

void FacilityLocation::random_sample(void)
{

}

double FacilityLocation::LP_solve(void)
{
	IloEnv env;

	/* set and initialize connection variables */
	IloNumVar x[NUM_OF_C * NUM_OF_F];
	fill_n(x, NUM_OF_C * NUM_OF_F, IloNumVar(env, 0, IloInfinity));

	/* set and initialize opening variables */
	IloNumVar y[NUM_OF_F];
	fill_n(x, NUM_OF_C, IloNumVar(env, 0, IloInfinity));

	/* set ranges of sums of connection variables (1 <= sum_i(x_ij) <= 1 for all j) */
	IloRange sum_range[NUM_OF_C];
	fill_n(sum_range, NUM_OF_C, IloRange(env, 1, 1));
	for (int j = 0; j < NUM_OF_C; ++j) {
		for (int i = 0; i < NUM_OF_F; ++i) {
			sum_range[j].setLinearCoef(x[i*NUM_OF_C + j], 1);
		}
	}

	/* set ranges of connection variables and opening variables (-inf <= x_ij - y_i <= 0 for all i, j) */
	IloRange x_range[NUM_OF_C * NUM_OF_F];
	fill_n(sum_range, NUM_OF_C, IloRange(env, -IloInfinity, 0));
	for (int j = 0; j < NUM_OF_C; ++j) {
		for (int i = 0; i < NUM_OF_F; ++i) {
			x_range[i * NUM_OF_C + j].setLinearCoef(x[i*NUM_OF_C + j], 1);
			x_range[i * NUM_OF_C + j].setLinearCoef(y[i], -1);
		}
	}

	/* set the obj fct (minimize sum_i(y_i*f_i) + sum_{i,j}(x_ij*d(i,j)) )*/
	IloObjective obj = IloMinimize(env, 0);
	for (int i = 0; i < NUM_OF_F; ++i) {
		obj.setLinearCoef(y[i], FacilityLocation::opening_cost[i]);
		for (int j = 0; j < NUM_OF_C; ++j) {
			obj.setLinearCoef(x[i*NUM_OF_C + j], FacilityLocation::connection_cost[i*NUM_OF_C + j]);
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

	/* solve the model */
	IloCplex solver(model);
	solver.solve();

	/* save results*/
	for (int i = 0; i < NUM_OF_F; ++i) {
		FacilityLocation::opening_variable[i] = solver.getValue(y[i]);
		for (int j = 0; j < NUM_OF_C; ++j) {
			FacilityLocation::connection_variable[i*NUM_OF_C + j] = solver.getValue(x[i*NUM_OF_C + j]);
		}
	}

	return solver.getObjValue();
}

void FacilityLocation::round(void)
{

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


void FacilityLocation::brute_force(void)
{

}

unsigned int FacilityLocation::objective(bool optimal = 0)
{
	unsigned int sol = 0;
	if (optimal) {
		for (int i = 0; i < NUM_OF_F; ++i) {
			sol += (unsigned int)FacilityLocation::optimal_opening_table[i] * FacilityLocation::opening_cost[i];
			for (int j = 0; j < NUM_OF_C; ++j) {
				sol += (unsigned int)FacilityLocation::optimal_connection_table[i*NUM_OF_C + j] * FacilityLocation::connection_cost[i*NUM_OF_C + j];
			}
		}
	}
	else {
		for (int i = 0; i < NUM_OF_F; ++i) {
			sol += (unsigned int)FacilityLocation::opening_table[i] * FacilityLocation::opening_cost[i];
			for (int j = 0; j < NUM_OF_C; ++j) {
				sol += (unsigned int)FacilityLocation::connection_table[i*NUM_OF_C + j] * FacilityLocation::connection_cost[i*NUM_OF_C + j];
			}
		}

	}
	return sol;
}