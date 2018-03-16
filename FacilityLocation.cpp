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

FacilityLocation::FacilityLocation(void)
{

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