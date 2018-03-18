#include "FacilityLocation.h"

template <typename T>
void print_contents(T a, T b);
int main() {
	FacilityLocation fl = FacilityLocation();
	double * connection_cost = fl.get_connection_cost();
	double * opening_cost = fl.get_opening_cost();
	double * connection_variable;
	double * opening_variable;
	bool * opening_table;
	bool * connection_table;

	//cout << "Facilitys' opening costs, connection costs" << endl;
	//print_contents(opening_cost, connection_cost);


	/* LP solver sovle the relaxed problem */
	cout << "before LP_solve" << endl;
	double sol = fl.LP_solve();
	cout << "The obj val of relaxation : " << sol << endl;
	cout << endl;


	/* Print LP_solver's output (Opening variables, Connection variables)*/
	//connection_variable = fl.get_connection_variable();
	//opening_variable = fl.get_opening_variable();

	//cout << "Facilitys' opening variables, connection variables" << endl;
	//print_contents(opening_variable, connection_variable);
	

	/* Find optimal solution through brute-force */
	//fl.brute_force();
	/*
	bool* optimal_connection_table = fl.get_optimal_connection_table();
	bool* optimal_opening_table = fl.get_optimal_opening_table();

	cout << "Facilitys' optimal opening variables, optimal connection variables" << endl;
	print_contents(optimal_opening_table, optimal_connection_table);
	*/
	//cout << "The obj val of brute-force alg : " << fl.get_optimal_cost() << endl;
	cout << endl;
	

	/* Find rounded solution */
	fl.round();
	cout << "The obj val of rounding alg : " << fl.get_rounded_cost() << endl;

	//cout << "Facilitys' rounded opening variables, rounded connection variables" << endl;
	//opening_table = fl.get_opening_table();
	//connection_table = fl.get_connection_table();
	//print_contents(opening_table, connection_table);

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
			cout << "x[" << i << "," << j << "]: " << b[i*NUM_OF_C + j] << endl;
		}
	}
	cout << endl;
}
