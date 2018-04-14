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
	double LP;
	double RS;
	int iteration = 0;

	int file_num = -1;
	while (true) {
		ifstream fin;
		string in_file = "FLP_IN\\FLP_IN_";
		file_num++;
		string number = "0000000";
		int parsing = file_num;
		for (int i = 0; parsing > 0; i++) {
			number[6 - i] = (parsing % 10) + '0';
			parsing /= 10;
		}
		in_file.append(number).append(".TXT");
		fin.open(in_file, ifstream::in);
		if (fin.fail() == true)
			break;
	}

	do {
		string in_file = "FLP_IN\\FLP_IN_";
		string number = "0000000";
		int parsing = file_num;
		for (int i = 0; parsing > 0; i++) {
			number[6 - i] = (parsing % 10) + '0';
			parsing /= 10;
		}
		in_file.append(number).append(".TXT");
		string out_file_summary = "FLP_OUT\\FLP_OUT.TXT";
		//string out_file = in_file;
		//out_file = replace_all(out_file, "IN", "OUT");
		string out_file = "FLP_OUT\\FLP_OUT_CONNECTION.TXT";


		std::srand(unsigned(std::time(NULL)) + ++iteration * 10);
		FacilityLocation *fl = new FacilityLocation(argc, argv);
		printf("다만들어써\n");

		/// Debug
		int n_facilities = fl->get_n_facilities();
		int n_clients = fl->get_n_clients();
		///

		/* LP solver sovle the relaxed problem */
		double sol = fl->LP_solve();
		LP = sol;
		cout << "The obj val of relaxation : " << sol << endl;
		cout << endl;

		//cout << "The optimal val : " << fl->get_optimal() << endl;

		/* Find rounded solution */
		fl->round();
		double rounded_cost;
		cout << "The obj val of rounding alg : " << (rounded_cost = fl->get_rounded_cost()) << endl;
		RS = rounded_cost;
		//print_contents<bool>(oot, oct);


		if (true){//CompareDoubleUlps(LP, RS) != 0) {
			ofstream fout_result(out_file, ofstream::out | ofstream::app);
			ofstream fout_one(in_file);
			ofstream fout_summary(out_file_summary, ofstream::out | ofstream::app);
			double duality_gap;
			if (CompareDoubleUlps(RS, 0) == 0 && CompareDoubleUlps(LP, 0) == 0)
				duality_gap = 1;
			else
				duality_gap = RS / LP;
			/* summary file */
			fout_summary << file_num << "," << fl->n_facilities << "," << fl->n_clients << "," << duality_gap << endl;
			
			/* each input file */
			fout_one << fl->n_facilities << endl;
			fout_one << fl->n_clients << endl;
			for (int i = 0; i < fl->n_facilities; i++) {
				fout_one << fl->opening_cost[i] << " ";
			}
			fout_one << endl;
			for (int i = 0; i < fl->n_facilities; i++) {
				for (int j = 0; j < fl->n_clients; j++) {
						fout_one << i << " " << j << " " << fl->connection_cost[i][j] << endl;
				}
			}

			/* result file */
			fout_result << file_num << ',' << LP << ',';
			fout_result << RS << ',';
			for (int i = 0; i < fl->n_facilities; i++) {
				if (fl->opening_table[i] == true) {
					fout_result << i << " ";
				}
			}
			fout_result << ',';

			for (int i = 0; i < fl->n_facilities; i++) {
				for (int j = 0; j < fl->n_clients; j++) {
					if (fl->connection_table[i][j] == true)
						fout_result << i << "-" << j << ',';
				}
			}
			fout_result << endl;

			file_num++;
		}





		if (argc == 2) {
			string out_file;
			out_file = argv[1];
			out_file = replace_all(out_file, "IN", "OUT");
			ofstream out(out_file);
			out << sol << endl;
			out << rounded_cost << endl;
			bool* opening_table = fl->get_opening_table();
			for (int i = 0; i < n_facilities; i++) {
				if (opening_table[i] == true) {
					out << i << " ";
				}
			}
			out << endl;
			bool ** connection_table = fl->get_connection_table();
			for (int i = 0; i < n_facilities; i++) {
				for (int j = 0; j < n_clients; j++) {
					if (connection_table[i][j] == true) {
						out << i << "-" << j << endl;
					}
				}
			}
		}

		delete fl;
	} while (iteration < 3000000);
	//} while (CompareDoubleUlps(LP, RS) == 0);
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
