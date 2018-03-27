#include "FacilityLocation.h"

int CompareDoubleUlps(double x, double y, int ulpsTolerance = 4)
{
	double diff = x - y;

	__int64 nx = *((__int64*)&x);
	__int64 ny = *((__int64*)&y);

	if ((nx & 0x8000000000000000) != (ny & 0x8000000000000000))
	{
		if (x == y)
			return 0;

		return (diff > 0) ? 1 : -1;
	}

	__int64 ulpsDiff = nx - ny;
	if ((ulpsDiff >= 0 ? ulpsDiff : -ulpsDiff) <= ulpsTolerance)
		return 0;

	return (diff > 0) ? 1 : -1;
}


double FacilityLocation::LP_solve(void)
{
	IloEnv env;

	/* set and initialize connection variables */
	IloNumVar * x = new IloNumVar[NUM_OF_F * NUM_OF_C];
	//IloNumVar x[NUM_OF_C * NUM_OF_F];
	for (int i = 0; i < NUM_OF_F; ++i) {
		for (int j = 0; j < NUM_OF_C; ++j) {
			x[i*NUM_OF_C + j] = IloNumVar(env, 0, IloInfinity);
		}
	}

	/* set and initialize opening variables */
	IloNumVar *y = new IloNumVar[NUM_OF_F];
	//IloNumVar y[NUM_OF_F];
	for (int i = 0; i < NUM_OF_F; ++i)
		y[i] = IloNumVar(env, 0, IloInfinity);

	/* set ranges of sums of connection variables (1 <= sum_i(x_ij) <= 1 for all j) */
	IloExpr * sum_expr = new IloExpr[NUM_OF_C];
	IloRange * sum_condition = new IloRange[NUM_OF_C];
	//IloExpr sum_expr[NUM_OF_C];
	//IloRange sum_condition[NUM_OF_C];
	for (int j = 0; j < NUM_OF_C; ++j) {
		sum_expr[j] = IloExpr(env);
		for (int i = 0; i < NUM_OF_F; ++i) {
			sum_expr[j] += x[i*NUM_OF_C + j];
		}
		sum_condition[j] = (sum_expr[j] == 1.0);
	}


	/* set ranges of connection variables and opening variables (-inf <= x_ij - y_i <= 0 for all i, j) */
	IloRange * x_range = new IloRange[NUM_OF_C * NUM_OF_F];
	//IloRange x_range[NUM_OF_C * NUM_OF_F];
	for (int i = 0; i < NUM_OF_F; ++i) {
		for (int j = 0; j < NUM_OF_C; ++j) {
			x_range[i * NUM_OF_C + j] = IloRange(env, -IloInfinity, 0);
			x_range[i * NUM_OF_C + j].setLinearCoef(x[i*NUM_OF_C + j], 1);
			x_range[i * NUM_OF_C + j].setLinearCoef(y[i], -1);
		}
	}

	/* set the obj fct (minimize sum_i(y_i*f_i) + sum_{i,j}(x_ij*d(i,j)) )*/
	IloObjective obj = IloMinimize(env, 0);
	for (int i = 0; i < NUM_OF_F; ++i) {
		obj.setLinearCoef(y[i], this->opening_cost[i]);
		for (int j = 0; j < NUM_OF_C; ++j) {
			obj.setLinearCoef(x[i*NUM_OF_C + j], this->connection_cost[i][j]);
		}
	}

	/* compile the model */
	IloModel model(env);
	for (int j = 0; j < NUM_OF_C; ++j) {
		//model.add(sum_range[j]);
		model.add(sum_condition[j]);
		for (int i = 0; i < NUM_OF_F; ++i) {
			model.add(x_range[i*NUM_OF_C + j]);
		}
	}
	model.add(obj);

	/* solve the model */
	IloCplex solver(model);
	try {
		solver.solve();
	}
	catch (IloException &ex) {
		cerr << ex << endl;
	}
	cout << solver.getObjValue() << endl;
	cout << solver.getValue(x[0]) << endl;
	/* save results*/
	opening_variable = new double[NUM_OF_F]; // dynamic allocation
	connection_variable = new double*[NUM_OF_F]; 
	for (int i = 0; i < NUM_OF_F; ++i) {
		connection_variable[i] = new double[NUM_OF_C]; // dynamic allocation
		this->opening_variable[i] = solver.getValue(y[i]);
		for (int j = 0; j < NUM_OF_C; ++j) {
			this->connection_variable[i][j] = solver.getValue(x[i*NUM_OF_C + j]);
		}
	}

	return solver.getObjValue();
}

template <typename T>
vector<size_t> sort_indexes(const vector<T> &v) {

	// initialize original index locations
	vector<size_t> idx(v.size());
	iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in v
	sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return v[i1] < v[i2]; });

	return idx;
}

void FacilityLocation::round(void)
{
	/* Assumes 'opening_variable' and 'connection_variable' are already calculated by LP-solver.
	Return the rounded solution of above two variables each in 'opening_table', 'connection_table'.
	Return the rounded solution's objective cost in 'rounded_cost' */


	/* Preprocessing */
	// initialize costs (3 ~ 5)
	copied_opening_cost = new double*[NUM_OF_F];
	for (int i = 0; i < NUM_OF_F; i++) {
		copied_opening_cost[i] = new double[NUM_OF_C];
		for (int j = 0; j < NUM_OF_C; j++) {
			copied_opening_cost[i][j] = opening_cost[i];  // The copied facilities are defined to have the same opening cost as the original.
		}
	}

	copied_connection_cost = new double**[NUM_OF_F]; // dynamic allocation
	for (int i = 0; i < NUM_OF_F; i++) {
		copied_connection_cost[i] = new double*[NUM_OF_C]; // dynamic allocation
		for (int i_ = 0; i_ < NUM_OF_C; i_++) {
			copied_connection_cost[i][i_] = new double[NUM_OF_C]; // dynamic allocation
			for (int j = 0; j < NUM_OF_C; j++) {
				copied_connection_cost[i][i_][j] = connection_cost[i][j];  // The copied connections are defined to have the same connection cost as the original (ex, d(i, j) = d(i1, j) = d(i2, j) for all j in C)
			}
		}
	}


	// opening variable, connection variable (y', x') �ʱ�ȭ
	copied_opening_variable = new double*[NUM_OF_F]; // dynamic allocation
	for (int i = 0; i < NUM_OF_F; i++) {
		copied_opening_variable[i] = new double[NUM_OF_C]; // dynamic allocation
		for (int j = 0; j < NUM_OF_C; j++) {
			copied_opening_variable[i][j] = 0.0; 
		}
	}

	copied_connection_variable = new double**[NUM_OF_F]; // dynamic allocation
	for (int i = 0; i < NUM_OF_F; i++) {
		copied_connection_variable[i] = new double*[NUM_OF_C]; // dynamic allocation
		for (int i_ = 0; i_ < NUM_OF_C; i_++) {
			copied_connection_variable[i][i_] = new double[NUM_OF_C]; // dynamic allocation
			for (int j = 0; j < NUM_OF_C; j++) {
				copied_connection_variable[i][i_][j] = 0.0;
			}
		}
	}

	/// assign values to y', x' (6 ~ 14)
	for (int i = 0; i < NUM_OF_F; i++) {
		// connection variable index sorting (6 ~ 8)
		size_t *increasing_index;
		vector<double> v;  // v contains the index j of clients who are partially connected to facility i.
		
		for (int j = 0; j < NUM_OF_C; j++) {
			//if (CompareDoubleAbsolute(connection_variable[i][j], 0.0) <= 0)
			//	continue;
			v.push_back(connection_variable[i][j]);  // save connection variables to v
		}
		vector<size_t> vid = sort_indexes(v);  // sort the index by its connection variable value
		
		increasing_index = &vid[0];  // from vector to list.


		// assign values to copied_opening_variable (9 ~ 11)
		copied_opening_variable[i][0] = connection_variable[i][increasing_index[0]];
		for (int j = 1; j < NUM_OF_C; j++) {
			copied_opening_variable[i][j] = connection_variable[i][increasing_index[j]] - connection_variable[i][increasing_index[j - 1]];
		}
		// assign values to copied_connection-variable (12 ~ 14)
		for (int i_ = 0; i_ < NUM_OF_C; i_++) {
			for (int j = 0; j < i_; j++) {
				copied_connection_variable[i][i_][increasing_index[j]] = 0.0;
			}
			for (int j = i_; j < NUM_OF_C; j++) {
				copied_connection_variable[i][i_][increasing_index[j]] = copied_opening_variable[i][i_];
			}
		}

	}
	///

	// initialize copied_opening_table all 0 (15)
	copied_opening_table = new bool*[NUM_OF_F]; // dynamic allocation
	for (int i = 0; i < NUM_OF_F; i++) {
		copied_opening_table[i] = new bool[NUM_OF_C]; // dynamic allocation
		for (int j = 0; j < NUM_OF_C; j++) {
			copied_opening_table[i][j] = 0;
		}
	}
	// initialize copied_connection_table all 0
	copied_connection_table = new bool**[NUM_OF_F]; // dynamic allocation
	for (int i = 0; i < NUM_OF_F; i++) {
		copied_connection_table[i] = new bool*[NUM_OF_C]; // dynamic allocation
		for (int i_ = 0; i_ < NUM_OF_C; i_++) {
			copied_connection_table[i][i_] = new bool[NUM_OF_C]; // dynamic allocation
			for (int j = 0; j < NUM_OF_C; j++) {
				copied_connection_table[i][i_][j] = 0;
			}
		}
	}

	/* Rounding */
	int* order_of_client = new int[NUM_OF_C];
	//int order_of_client[NUM_OF_C] = { 0 };

	for (int i = 0; i < NUM_OF_C; i++) {
		for (int j = 0; j < NUM_OF_C; j++) {
			if (clock_of_client[j] == i)
				order_of_client[i] = j;  // index sorting (18 ~ 19)
		}
	}

	opening_table = new bool[NUM_OF_F]; // dynamic allocation
	for (int i = 0; i < NUM_OF_F; i++)  // a set of opened facilities (final output)
		opening_table[i] = 0;

	connection_table = new bool*[NUM_OF_F]; // dynamic allocation
	for (int i = 0; i < NUM_OF_F; i++) {  // connections (final output)
		connection_table[i] = new bool[NUM_OF_C]; // dynamic allocation
		for (int j = 0; j < NUM_OF_C; j++)
			connection_table[i][j] = 0;
	}

	// actual rounding algorithm (20 ~ 27)
	for (int j = 0; j < NUM_OF_C; j++) {  // j for client, i for facility, j_ for j'
		double min = DBL_MAX;
		int min_client = order_of_client[j];  // pick a client who has the most small  ==>  pick minimum clock element's index
		
		// find minimum clock facility
		int min_facility_r, min_facility_c, min_facility_client;
		for (int i = 0; i < NUM_OF_F; i++) {
			for (int i_ = 0; i_ < NUM_OF_C; i_++) {
				if (CompareDoubleUlps(copied_connection_variable[i][i_][min_client], 0.0) == 1 && exponential_clock[i][i_] < min) {  // find a facility which is connected to the client and has the smallest clock.
					min_facility_r = i;
					min_facility_c = i_;
					min = exponential_clock[i][i_];  // update minimum clock
				}
			}
		}

		// find minimum clock client 
		min = DBL_MAX;
		for (int j_ = 0; j_ < NUM_OF_C; j_++) {
			if (CompareDoubleUlps(copied_connection_variable[min_facility_r][min_facility_c][j_], 0.0) == 1 && clock_of_client[j_] < min) {
				min_facility_client = j_;
				min = clock_of_client[j_];
			}
		}

		if (min_client == min_facility_client) {  // if j == j'
			copied_opening_table[min_facility_r][min_facility_c] = 1;  // open i
			copied_connection_table[min_facility_r][min_facility_c][min_client] = 1;  // connect j to i
		}
		else {  // connect j to the same facility as j'
			int min_facility_client_facility_r = 0;
			int min_facility_client_facility_c = 0;
			for (int i = 0; i < NUM_OF_F; i++) {  // find the facility connected to j'(min_client_facility)
				for (int i_ = 0; i_ < NUM_OF_C; i_++) {
					if (copied_connection_table[i][i_][min_facility_client] == 1) {
						min_facility_client_facility_r = i;
						min_facility_client_facility_c = i_;
					}
				}
			}
			copied_connection_table[min_facility_client_facility_r][min_facility_client_facility_c][min_client] = 1;
		}
	}

	// post processing (29 ~ 34)
	for (int i = 0; i < NUM_OF_F; i++) {
		int sum = 0;
		for (int i_ = 0; i_ < NUM_OF_C; i_++) {
			sum += copied_opening_table[i][i_];
		}
		if (sum > 0) opening_table[i] = 1;
	}

	for (int j = 0; j < NUM_OF_C; j++) {
		int facility_r = 0;
		int facility_c = 0;

		for (int i = 0; i < NUM_OF_F; i++) {
			for (int i_ = 0; i_ < NUM_OF_C; i_++) {
				if (copied_connection_table[i][i_][j] == 1) {
					facility_r = i;
					facility_c = i_;
				}
			}
		}

		connection_table[facility_r][j] = 1;
	}
	/////////

	/* Calculate Cost ( objective function ) */
	int total_opening_cost = 0, total_connection_cost = 0;

	/* calculate total connection_cost */
	for (int j = 0; j < NUM_OF_C; j++) {  // calculate total connection_cost
		for (int i = 0; i < NUM_OF_F; i++) {
			total_connection_cost += connection_table[i][j] * connection_cost[i][j];
		}
	}

	/* calculate total opening cost */
	for (int i = 0; i < NUM_OF_F; i++) {
		total_opening_cost += opening_table[i] * opening_cost[i];
	}

	rounded_cost = total_opening_cost + total_connection_cost;
}

int myrandom(int i) { return std::rand() % i; }
FacilityLocation::FacilityLocation(void)
{
	/*
	clock_of_client[0] = 2;
	clock_of_client[1] = 0;
	clock_of_client[2] = 1;

	exponential_clock[0] = 2.46;
	exponential_clock[1] = 9.94;

	opening_cost[0] = 90;
	opening_cost[1] = 74;

	connection_cost[0 * NUM_OF_C + 0] = 60;
	connection_cost[0 * NUM_OF_C + 1] = 32;
	connection_cost[0 * NUM_OF_C + 2] = 58;
	connection_cost[1 * NUM_OF_C + 0] = 12;
	connection_cost[1 * NUM_OF_C + 1] = 66;
	connection_cost[1 * NUM_OF_C + 2] = 21;

	opening_variable[0] = (double)0;
	opening_variable[1] = (double)1;

	connection_variable[0 * NUM_OF_C + 0] = double(0);
	connection_variable[0 * NUM_OF_C + 1] = double(0);
	connection_variable[0 * NUM_OF_C + 2] = double(0);
	connection_variable[1 * NUM_OF_C + 0] = double(1);
	connection_variable[1 * NUM_OF_C + 1] = double(1);
	connection_variable[1 * NUM_OF_C + 2] = double(1);
	*/

	/* generation of the expoential clocks of the facilities */
	//double y_i; // facility�� ���������� 1, ���������� 0, fraction variable
	std::default_random_engine generator;

	std::srand(unsigned(std::time(0)));

	//������ facility�� exponential_clock �� ����

	exponential_clock = new double*[NUM_OF_F]; // dynamic allocation
	for (int i = 0; i < NUM_OF_F; ++i) {
		exponential_clock[i] = new double[NUM_OF_C]; // dynamic allocation
		for (int i_ = 0; i_ < NUM_OF_C; i_++) {
			double y_i = ((double)rand() / (RAND_MAX));
			std::exponential_distribution<double> distribution(y_i);
			//double number = distribution(generator);
			//exponential_clock[i] = y_i*exp(double(-1 * y_i*number));
			exponential_clock[i][i_] = distribution(generator);
		}
	}


	//std::cout << "exponential_distribution(" << y_i << "):" << std::endl;
	//std::cout << std::fixed; std::cout.precision(1);

	/*
	for (int i = 0; i < NUM_OF_F; ++i) {
	//cout << i << " : " << p[i] << endl;
	printf("%lf\n", p[i]);
	}*/

	/* generation of the orders of the cliendts */
	std::vector<int> myvector;

	// set some values:
	for (int i = 0; i < NUM_OF_C; ++i) myvector.push_back(i); // 1 2 3 4 5 6 7 8 9

															  // using built-in random generator:
	std::random_shuffle(myvector.begin(), myvector.end());

	// using myrandom:
	std::random_shuffle(myvector.begin(), myvector.end(), myrandom);

	int i = 0;
	clock_of_client = new int[NUM_OF_C]; // dynamic allocation
	for (std::vector<int>::iterator it = myvector.begin(); it != myvector.end(); ++it, ++i)
		clock_of_client[i] = *it;


	/* settiing costs of openings and connections */
	connection_cost = new double*[NUM_OF_F]; // dynamic allocation
	for (int i = 0; i < NUM_OF_F; i++) {
		connection_cost[i] = new double[NUM_OF_C]; // dynamic allocation
	}
	for (int i = 0, j = 0; i < NUM_OF_F;) {
		//connection_cost[i][j] = (int)rand() % CONNECTION_COST_MAX + 1;
		set_connection_cost_between_Fi_and_Cj(i, j);
		j++;
		if (j == NUM_OF_C) {
			i++;
			j = 0;
		}
	}
	opening_cost = new double[NUM_OF_F]; // dynamic allocation
	for (int i = 0; i < NUM_OF_F; i++) {
		//opening_cost[i] = (int)rand() % OPENING_COST_MAX + 1;
		opening_cost[i] = (double)rand() / RAND_MAX *( OPENING_COST_MAX - 1 ) + 1;
	}

	// print out content:

	/*
	for (int i = 0; i < NUM_OF_C; i++) {
	cout << "c" << i << ": " << clock_of_client[i] << endl;
	}
	for (int i = 0; i < NUM_OF_F; i++) {
	cout << "f" << i << ": " << exponential_clock[i] << endl;
	}
	cout << "---------cost---------------" << endl;
	*/

	/*
	for (int i = 0; i < NUM_OF_F*NUM_OF_C; i++) {
	cout << "c" << i << ": " << connection_cost[i] << endl;
	}

	for (int i = 0; i < NUM_OF_F; i++) {
	cout << "f" << i << ": " << opening_cost[i] << endl;
	}
	*/
}

void calculate_func(bool *connection_table, FacilityLocation *fcl, double *min)
{
	//bool opening_table[NUM_OF_F] = { 0 };
	bool* opening_table = new bool[NUM_OF_F]; // dynamic allocation
	for (int i = 0; i < NUM_OF_F; i++) {
		opening_table[i] = 0; // initialize
	}

	double total_opening_cost = 0.0, total_connection_cost = 0.0;

	/* calculate total connection_cost */
	for (int j = 0; j < NUM_OF_C; j++) {
		for (int i = 0; i < NUM_OF_F; i++) {
			total_connection_cost += connection_table[i * NUM_OF_C + j] * (fcl->connection_cost[i][j]);
			opening_table[i] += connection_table[i * NUM_OF_C + j];
		}
	}

	/* calculate total opening cost */
	for (int i = 0; i < NUM_OF_F; i++) {
		total_opening_cost += opening_table[i] * (fcl->opening_cost[i]);
	}

	/* update minimum cost solution */
	fcl->optimal_opening_table = new bool[NUM_OF_F]; // dynamic allocation
	fcl->optimal_connection_table = new bool*[NUM_OF_F]; // dynamic allocation
	if ((total_connection_cost + total_opening_cost) < *min) {
		*min = total_connection_cost + total_opening_cost;
		for (int i = 0; i < NUM_OF_F; i++)
			fcl->optimal_opening_table[i] = opening_table[i];
		for (int i = 0; i < NUM_OF_F; i++) {
			fcl->optimal_connection_table[i] = new bool[NUM_OF_C]; // dynamic allocation
			for (int j = 0; j < NUM_OF_C; j++)
				fcl->optimal_connection_table[i][j] = connection_table[i * NUM_OF_C + j];
		}
		fcl->optimal_cost = total_opening_cost + total_connection_cost;

	}
}

void recursive_func(bool *connection_table, int index, FacilityLocation *fcl, double *min)
{
	if (index >= NUM_OF_C)	return;  // base case
	for (int i = 0; i < NUM_OF_F; i++) {
		connection_table[i*NUM_OF_C + index] = 1;
		recursive_func(connection_table, index + 1, fcl, min); // recursive call
		calculate_func(connection_table, fcl, min);
		connection_table[i*NUM_OF_C + index] = 0;
	}
	connection_table[index] = 1;
}

void FacilityLocation::brute_force(void)
{
	/* Runing Time O(F^C) */
	/* check all possible solutions */
	/* find the optimal solution : min(opening cost + connection cost) */
	/* save the optimal solution to 'optimal_opening_table', 'optimal_connection_table' */

	//bool connection_table[NUM_OF_C * NUM_OF_F] = { 0 };
	bool* connection_table = new bool[NUM_OF_C * NUM_OF_F];
	double min = DBL_MAX;

	recursive_func(connection_table, 0, this, &min);
}

double FacilityLocation::objective(bool optimal)
{

	double sol = 0;
	if (optimal) {
		for (int i = 0; i < NUM_OF_F; ++i) {
			sol += (double)this->optimal_opening_table[i] * this->opening_cost[i];
			for (int j = 0; j < NUM_OF_C; ++j) {
				sol += (double)this->optimal_connection_table[i][j] * this->connection_cost[i][j];
			}
		}
	}
	else {
		for (int i = 0; i < NUM_OF_F; ++i) {
			sol += (double)this->opening_table[i] * this->opening_cost[i];
			for (int j = 0; j < NUM_OF_C; ++j) {
				sol += (double)this->connection_table[i][j] * this->connection_cost[i][j];
			}
		}

	}
	return sol;
}

void FacilityLocation::set_connection_cost_between_Fi_and_Cj(int i, int j) {
	if (i == 0 || j == 0) {
		//connection_cost[i][j] = (int)rand() % CONNECTION_COST_MAX + 1;
		connection_cost[i][j] = (double)rand() / RAND_MAX * (CONNECTION_COST_MAX - 1) + 1;
		return;
	}
	double min = DBL_MAX;
	/* for triangle inequality */
	for (int ip = 0; ip < i; ip++) {
		for (int jp = 0; jp < j; jp++) {
			double d = get_distance(j, jp, ip);
			if (min > d + connection_cost[i][jp]) {
				min = d + connection_cost[i][jp];
			}
		}
	}
	double random;
	while ((random = (double)rand() / RAND_MAX) == 1) {} // 0 <= random < 1
	connection_cost[i][j] = random * (min-1) + 1; // 1 <= connection_cost < min 
	//connection_cost[i][j] = rand() % (int)min + 1; // 0 < connection_cost < min 
}

double FacilityLocation::get_distance(int j, int jp, int ip) {
	return connection_cost[ip][j] + connection_cost[ip][jp];
}