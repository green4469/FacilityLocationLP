#include "FacilityLocation.h"

int main() {
	FacilityLocation fl = FacilityLocation();
	double * connection_cost = fl.get_connection_cost();
	double * opening_cost = fl.get_opening_cost();

	bool ba = true;
	ba += 1;
	cout << ba << endl;

	for (int i = 0; i < NUM_OF_F; ++i) {
		cout << "y[" << i << "]: " << opening_cost[i] << endl;
	}
	for (int i = 0; i < NUM_OF_F; ++i) {
		for (int j = 0; j < NUM_OF_C; ++j) {
			cout << "x[" << i << "," << j << "]: " << connection_cost[i*NUM_OF_C + j] << endl;
		}
	}
	double sol = fl.LP_solve();
	cout << "The obj val of relaxation : " << sol << endl;
	fl.brute_force();
	bool* optimal_connection_table = fl.get_optimal_connection_table();
	bool* optimal_opening_table = fl.get_optimal_opening_table();
	for (int i = 0; i < NUM_OF_F; ++i) {
		cout << "y[" << i << "]: " << optimal_opening_table[i] << endl;
	}
	for (int i = 0; i < NUM_OF_F; ++i) {
		for (int j = 0; j < NUM_OF_C; ++j) {
			cout << "x[" << i << "," << j << "]: " << optimal_connection_table[i*NUM_OF_C + j] << endl;
		}
	}

	cout << "The obj val of brute-force alg : " << fl.get_optimal_cost() << endl;
	fl.round();
	cout << "The obj val of rounding alg : " << fl.get_rounded_cost() << endl;
	system("pause");
}