#include "FacilityLocation.h"

double FacilityLocation::LP_solve(void)
{
	
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

FacilityLocation::FacilityLocation(void)
{

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

unsigned int FacilityLocation::objective(bool optimal) 
{


}