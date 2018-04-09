#include "FacilityLocation.h"


template <typename T>
void print_contents(T a, T b);

int main(int argc, char **argv) {
	FacilityLocation fl = FacilityLocation();

	/// Debug
	bool ** oct = fl.get_optimal_connection_table();
	bool * oot = fl.get_optimal_opening_table();
	///

	/* LP solver sovle the relaxed problem */
	double sol = fl.LP_solve();
	cout << "The obj val of relaxation : " << sol << endl;
	cout << endl;

	/* Find rounded solution */
	fl.round();
	cout << "The obj val of rounding alg : " << fl.get_rounded_cost() << endl;
	//print_contents<bool>(oot, oct);
}

template <typename T>
void print_contents(T a, T b)
{
	for (int i = 0; i < NUM_OF_F; ++i) {
		cout << "y[" << i << "]: " << a[i] << endl;
	}

	cout << endl;
	for (int i = 0; i < NUM_OF_F; ++i) {
		for (int j = 0; j < NUM_OF_C; ++j) {
			cout << "x[" << i << "," << j << "]: " << b[i][j] << ' ';
			//printf("%x\n", b[i*NUM_OF_C + j]);
		}
	}
	cout << endl;
}
