#include "FacilityLocation.h"

int main(int argc, char* argv[]) // argv : file name (ex: FLP_IN_0001.txt)
{
	double LP = 0.0;
	double RS = 0.0;
	double IG = 0.0;

	srand((unsigned)time(NULL));
	rand();

	clock_t RT_LP, RT_RD;

	FacilityLocation *FL = new FacilityLocation(argc, argv);

	RT_LP = clock();
	LP = FL->LP_solve();
	RT_LP = clock() - RT_LP;

	RT_RD = clock();
	RS = FL->round();
	IG = RS / LP;
	RT_RD = clock() - RT_RD;

	float RT_LP_S = ((float)RT_LP) / CLOCKS_PER_SEC;
	float RT_RD_S = ((float)RT_RD) / CLOCKS_PER_SEC;

	cout << "Relexed Solution: " << LP << endl;
	cout << "Rounded Solution: " << RS << endl;
	cout << "Integrality Gap: " << IG << endl;
	cout << "Running Time: " << RT_LP_S << " + " << RT_RD_S << " = " << RT_LP_S + RT_RD_S << endl;

	string fnum;
	string fname = argv[1];
	fnum.append(fname, 9, 7);
	cout << "fnum: " << fnum << endl;

	ofstream fout("FLP_OUT.TXT", ofstream::out | ofstream::app);
	//fout << fnum << ' ' << FL->n_facilities << ' ' << FL->n_clients << ' ' << LP << ' ' << RS << ' ' << IG  << ' ' << RT_LP_S << ' ' << RT_RD_S << ' ' << RT_LP_S + RT_RD_S << endl;
	fout << fnum << ' ' << FL->n_facilities << ' ' << FL->n_clients << ' ' << LP << ' ' << RS << ' ' << IG << ' ' << RT_LP_S + RT_RD_S << endl;


	if (argc == 3) {  // Save LP solver's output
		ofstream fout_LP("FLP_OUT_LP.TXT", ofstream::out | ofstream::app);
		fout_LP << fnum << endl;

		for (int i = 0; i < FL->n_facilities - 1; i++) {
			fout_LP << FL->opening_variable[i] << ' ';
		}
		fout_LP << FL->opening_variable[FL->n_facilities - 1] << endl;

		for (int i = 0; i < FL->n_facilities; i++) {
			for (int j = 0; j < FL->n_clients - 1; j++) {
				fout_LP << FL->connection_variable[i][j] << ' ';
			}
			fout_LP << FL->connection_variable[i][FL->n_clients - 1] << endl;
		}
	}

	delete FL;
	


	return 0;
}
