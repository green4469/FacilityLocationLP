#ifndef _HEADER_
#define _HEADER_
#include "FacilityLocation.h"
#endif
string replace_all(
	__in const std::string &message,
	__in const std::string &pattern,
	__in const std::string &replace
);

template <typename T>
void print_contents(T a, T b);

int main(int argc, char* argv[]) // argv : file name (ex: FLP_IN_0001.txt)
{
	FacilityLocation fl = FacilityLocation(argc,argv);
	printf("다만들어써\n");
	string out_file;
	out_file = argv[1];
	out_file = replace_all(out_file, "IN", "OUT");
	ofstream out(out_file);
	/// Debug
	int n_facilities = fl.get_n_facilities();
	int n_clients = fl.get_n_clients();
	bool ** oct = fl.get_optimal_connection_table();
	bool * oot = fl.get_optimal_opening_table();
	///
	/* LP solver sovle the relaxed problem */
	double sol = fl.LP_solve();
	cout << "The obj val of relaxation : " << sol << endl;
	cout << endl;

	/* Find rounded solution */
	fl.round();
	double rounded_cost;
	cout << "The obj val of rounding alg : " << (rounded_cost = fl.get_rounded_cost()) << endl;
	//print_contents<bool>(oot, oct);
	out << sol << endl;
	out << rounded_cost << endl;
	bool* opening_table = fl.get_opening_table();
	for (int i = 0; i < n_facilities; i++) {
		if (opening_table[i] == true) {
			out << i << " ";
		}
	}
	out << endl;
	bool ** connection_table = fl.get_connection_table();
	for (int i = 0; i < n_facilities; i++) {
		for (int j = 0; j < n_clients; j++) {
			if (connection_table[i][j] == true) {
				out << i << "-" << j << endl;
			}
		}
	}
	return 0;
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

string replace_all(
	__in const std::string &message,
	__in const std::string &pattern,
	__in const std::string &replace
) {

	std::string result = message;
	std::string::size_type pos = 0;
	std::string::size_type offset = 0;

	while ((pos = result.find(pattern, offset)) != std::string::npos)
	{
		result.replace(result.begin() + pos, result.begin() + pos + pattern.size(), replace);
		offset = pos + replace.size();
	}

	return result;
}