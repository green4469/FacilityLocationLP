#include "FacilityLocation.h"

int main(void)
{
	FacilityLocation ex;
	ex.LP_solve();
	ex.random_sample();
	ex.round();
	ex.brute_force();
	ex.objective();

	system("pause");
	return 0;
}