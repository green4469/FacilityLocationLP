#include "FacilityLocation.h"

void FacilityLocation::random_sample(void)
{

}

void FacilityLocation::LP_solve(void)
{
	
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