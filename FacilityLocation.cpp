#include "FacilityLocation.h"

int CompareDoubleUlps(double x, double y, int UlpsTolerance)
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

	__int64 UlpsDiff = nx - ny;
	if ((UlpsDiff >= 0 ? UlpsDiff : -UlpsDiff) <= UlpsTolerance)
		return 0;

	return (diff > 0) ? 1 : -1;
}

double FacilityLocation::LP_solve(void)
{
	IloEnv env;
	env.setDeleter(IloSafeDeleterMode);
	/* set and initialize connection variables */
	/*
	IloNumVar * x = new IloNumVar[n_facilities * n_clients];
	//IloNumVar x[n_clients * n_facilities];
	for (int i = 0; i < n_facilities; ++i) {
	for (int j = 0; j < n_clients; ++j) {
	x[i*n_clients + j] = IloNumVar(env, 0, IloInfinity);
	}
	}
	*/
	IloNumVarArray x(env, (IloInt)(n_facilities * n_clients), (IloNum)0, IloInfinity);


	/* set and initialize opening variables */
	/*
	IloNumVar *y = new IloNumVar[n_facilities];
	//IloNumVar y[n_facilities];
	for (int i = 0; i < n_facilities; ++i)
	y[i] = IloNumVar(env, 0, IloInfinity);
	*/
	IloNumVarArray y(env, (IloInt)n_facilities, (IloNum)0, IloInfinity);


	/* set ranges of sums of connection variables (1 <= sum_i(x_ij) <= 1 for all j) */
	/*
	IloExpr * sum_expr = new IloExpr[n_clients];
	IloRange * sum_condition = new IloRange[n_clients];
	//IloExpr sum_expr[n_clients];
	//IloRange sum_condition[n_clients];
	for (int j = 0; j < n_clients; ++j) {
	sum_expr[j] = IloExpr(env);
	for (int i = 0; i < n_facilities; ++i) {
	sum_expr[j] += x[i*n_clients + j];
	}
	sum_condition[j] = (sum_expr[j] == 1.0);
	}
	*/
	IloExprArray sum_expr(env);
	IloRangeArray sum_condition(env);
	for (int j = 0; j < n_clients; ++j) {
		sum_expr.add(IloExpr(env));
		for (int i = 0; i < n_facilities; ++i) {
			sum_expr[j] += x[i*n_clients + j];
		}
		sum_condition.add(sum_expr[j] == 1.0);
	}



	/* set ranges of connection variables and opening variables (-inf <= x_ij - y_i <= 0 for all i, j) */
	/*
	IloRange * x_range = new IloRange[n_clients * n_facilities];
	//IloRange x_range[n_clients * n_facilities];
	for (int i = 0; i < n_facilities; ++i) {
	for (int j = 0; j < n_clients; ++j) {
	x_range[i * n_clients + j] = IloRange(env, -IloInfinity, 0);
	x_range[i * n_clients + j].setLinearCoef(x[i*n_clients + j], 1);
	x_range[i * n_clients + j].setLinearCoef(y[i], -1);
	}
	}
	*/
	IloRangeArray x_range(env);
	for (int i = 0; i < n_facilities; ++i) {
		for (int j = 0; j < n_clients; ++j) {
			x_range.add(IloRange(env, -IloInfinity, 0));
			x_range[i * n_clients + j].setLinearCoef(x[i*n_clients + j], 1);
			x_range[i * n_clients + j].setLinearCoef(y[i], -1);
		}
	}

	/* set the obj fct (minimize sum_i(y_i*f_i) + sum_{i,j}(x_ij*d(i,j)) )*/
	IloObjective obj = IloMinimize(env, 0);
	for (int i = 0; i < n_facilities; ++i) {
		obj.setLinearCoef(y[i], this->opening_cost[i]);
		for (int j = 0; j < n_clients; ++j) {
			obj.setLinearCoef(x[i*n_clients + j], this->connection_cost[i][j]);
		}
	}

	/* compile the model */
	
	IloModel model(env);
	for (int j = 0; j < n_clients; ++j) {
		//model.add(sum_range[j]);
		model.add(sum_condition[j]);
		for (int i = 0; i < n_facilities; ++i) {
			model.add(x_range[i*n_clients + j]);
		}
	}
	
	//IloModel model(env);
	//model.add(sum_condition);
	//model.add(x_range);
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
	for (int i = 0; i < n_facilities; ++i) {
		this->opening_variable[i] = solver.getValue(y[i]);
		for (int j = 0; j < n_clients; ++j) {
			this->connection_variable[i][j] = solver.getValue(x[i*n_clients + j]);
		}
	}
	double objval = solver.getObjValue();
	/* delete objects */
	solver.end();
	model.end();
	obj.end();
	x_range.end();
	sum_condition.end();
	sum_expr.end();
	y.end();
	x.end();
	env.end();
	return objval;
}

double FacilityLocation::get_optimal(void)
{
	IloEnv env;

	/* set and initialize connection variables */
	IloIntVarArray x(env, (IloInt)(n_facilities * n_clients), (IloInt)0, INT_MAX);


	/* set and initialize opening variables */
	IloIntVarArray y(env, (IloInt)n_facilities, (IloInt)0, INT_MAX);


	/* set ranges of sums of connection variables (1 <= sum_i(x_ij) <= 1 for all j) */
	IloExprArray sum_expr(env);
	IloRangeArray sum_condition(env);
	for (int j = 0; j < n_clients; ++j) {
		sum_expr.add(IloExpr(env));
		for (int i = 0; i < n_facilities; ++i) {
			sum_expr[j] += x[i*n_clients + j];
		}
		sum_condition.add(sum_expr[j] == 1.0);
	}



	/* set ranges of connection variables and opening variables (-inf <= x_ij - y_i <= 0 for all i, j) */
	IloRangeArray x_range(env);
	for (int i = 0; i < n_facilities; ++i) {
		for (int j = 0; j < n_clients; ++j) {
			x_range.add(IloRange(env, -IloInfinity, 0));
			x_range[i * n_clients + j].setLinearCoef(x[i*n_clients + j], 1);
			x_range[i * n_clients + j].setLinearCoef(y[i], -1);
		}
	}

	/* set the obj fct (minimize sum_i(y_i*f_i) + sum_{i,j}(x_ij*d(i,j)) )*/
	IloObjective obj = IloMinimize(env, 0);
	for (int i = 0; i < n_facilities; ++i) {
		obj.setLinearCoef(y[i], this->opening_cost[i]);
		for (int j = 0; j < n_clients; ++j) {
			obj.setLinearCoef(x[i*n_clients + j], this->connection_cost[i][j]);
		}
	}

	/* compile the model */
	IloModel model(env);
	model.add(sum_condition);
	model.add(x_range);
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
	for (int i = 0; i < n_facilities; ++i) {
		this->opening_variable[i] = solver.getValue(y[i]);
		for (int j = 0; j < n_clients; ++j) {
			this->connection_variable[i][j] = solver.getValue(x[i*n_clients + j]);
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

void FacilityLocation::post_process(void)
{
	for (int j = 0; j < n_clients; j++) {
		/* Find the index of original connected facility */
		int connected_facility_index = 0;
		for (int i = 0; i < n_facilities; i++) {
			if (connection_table[i][j] == true) {
				connected_facility_index = i;
			}
		}

		/* Among opened facilities, find the facility whose connection cost between itself and the client */
		double min_connection_cost = connection_cost[connected_facility_index][j];
		int min_facility_index = connected_facility_index;
		for (int i = 0; i < n_facilities; i++) {
			if ((opening_table[i] == true) and (CompareDoubleUlps(connection_cost[i][j], min_connection_cost) == -1)) {
				min_connection_cost = connection_cost[i][j];
				min_facility_index = i;
			}
		}

		/* Disconnect original connection & connect to the optimal facility */
		connection_table[connected_facility_index][j] = false;
		connection_table[min_facility_index][j] = true;
	}
}

double FacilityLocation::round(void)
{
	/* Assumes 'opening_variable' and 'connection_variable' are already calculated by LP-solver.
	Return the rounded solution of above two variables each in 'opening_table', 'connection_table'.
	Return the rounded solution's objective cost in 'rounded_cost' */


	/* Preprocessing */
	// initialize copied opening & connection costs (3 ~ 5) with opening & connection costs
	for (int i = 0; i < n_facilities; i++) {
		for (int j = 0; j < n_clients; j++) {
			copied_opening_cost[i][j] = opening_cost[i];  // The copied facilities are defined to have the same opening cost as the original.
		}
	}

	for (int i = 0; i < n_facilities; i++) {
		for (int i_ = 0; i_ < n_clients; i_++) {
			for (int j = 0; j < n_clients; j++) {
				copied_connection_cost[i][i_][j] = connection_cost[i][j];  // The copied connections are defined to have the same connection cost as the original (ex, d(i, j) = d(i1, j) = d(i2, j) for all j in C)
			}
		}
	}
	
	// initialize opening variable, connection variable (y', x') as 0
	for (int i = 0; i < n_facilities; i++) {
		for (int j = 0; j < n_clients; j++) {
			copied_opening_variable[i][j] = 0.0; 
		}
	}

	for (int i = 0; i < n_facilities; i++) {
		for (int i_ = 0; i_ < n_clients; i_++) {
			for (int j = 0; j < n_clients; j++) {
				copied_connection_variable[i][i_][j] = 0.0;
			}
		}
	}

	/// assign values to y', x' (6 ~ 14) [Spreading Facility]
	for (int i = 0; i < n_facilities; i++) {
		// connection variable index sorting (6 ~ 8)
		size_t *increasing_index;
		vector<double> v;  // v contains the index j of clients who are partially connected to facility i.
		
		for (int j = 0; j < n_clients; j++) {
			//if (CompareDoubleUlps(connection_variable[i][j], 0.0) <= 0)
			//	continue;
			v.push_back(connection_variable[i][j]);  // save connection variables to v
		}
		vector<size_t> vid = sort_indexes(v);  // sort the index by its connection variable value
		
		increasing_index = &vid[0];  // from vector to list.


		// assign values to copied_opening_variable (9 ~ 11)
		copied_opening_variable[i][0] = connection_variable[i][increasing_index[0]];
		for (int j = 1; j < n_clients; j++) {
			copied_opening_variable[i][j] = connection_variable[i][increasing_index[j]] - connection_variable[i][increasing_index[j - 1]];
		}
		// assign values to copied_connection-variable (12 ~ 14)
		for (int i_ = 0; i_ < n_clients; i_++) {
			for (int j = 0; j < i_; j++) {
				copied_connection_variable[i][i_][increasing_index[j]] = 0.0;
			}
			for (int j = i_; j < n_clients; j++) {
				copied_connection_variable[i][i_][increasing_index[j]] = copied_opening_variable[i][i_];
			}
		}
	}
	///

	// initialize copied_opening_table all 0 (15)
	for (int i = 0; i < n_facilities; i++) {
		for (int j = 0; j < n_clients; j++) {
			copied_opening_table[i][j] = false;
		}
	}
	// initialize copied_connection_table all 0
	for (int i = 0; i < n_facilities; i++) {
		for (int i_ = 0; i_ < n_clients; i_++) {
			for (int j = 0; j < n_clients; j++) {
				copied_connection_table[i][i_][j] = false;
			}
		}
	}

	/* Facilities Exponential Clocks Assignment */
	std::default_random_engine generator;
	for (int i = 0; i < n_facilities; ++i) {
		for (int i_ = 0; i_ < n_clients; i_++) {
			double y_i = copied_opening_variable[i][i_];
			std::exponential_distribution<double> distribution(y_i);
			exponential_clock[i][i_] = distribution(generator);
			if (CompareDoubleUlps(y_i, 0.0) == 0) {
				exponential_clock[i][i_] = DBL_MAX;
			}
		}
	}

	/* Rounding */
	int* order_of_client = new int[n_clients];

	for (int i = 0; i < n_clients; i++) {
		for (int j = 0; j < n_clients; j++) {
			if (clock_of_client[j] == i)
				order_of_client[i] = j;  // index sorting (18 ~ 19)
		}
	}

	for (int i = 0; i < n_facilities; i++)  // a set of opened facilities (final output)
		opening_table[i] = 0;

	for (int i = 0; i < n_facilities; i++) {  // connections (final output)
		for (int j = 0; j < n_clients; j++)
			connection_table[i][j] = 0;
	}

	// actual rounding algorithm (20 ~ 27)
	for (int j = 0; j < n_clients; j++) {  // j for client, i for facility, j_ for j'
		double min = DBL_MAX;
		int min_client = order_of_client[j];  // pick a client who has the most small  ==>  pick minimum clock element's index
		
		// find minimum clock facility
		int min_facility_r, min_facility_c, min_facility_client;
		for (int i = 0; i < n_facilities; i++) {
			for (int i_ = 0; i_ < n_clients; i_++) {
				if (CompareDoubleUlps(copied_connection_variable[i][i_][min_client], 0.0) == 1 && exponential_clock[i][i_] < min) {  // find a facility which is connected to the client and has the smallest clock.
					min_facility_r = i;
					min_facility_c = i_;
					min = exponential_clock[i][i_];  // update minimum clock
				}
			}
		}

		// find minimum clock client 
		min = DBL_MAX;
		for (int j_ = 0; j_ < n_clients; j_++) {
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
			for (int i = 0; i < n_facilities; i++) {  // find the facility connected to j'(min_client_facility)
				for (int i_ = 0; i_ < n_clients; i_++) {
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
	for (int i = 0; i < n_facilities; i++) {
		int sum = 0;
		for (int i_ = 0; i_ < n_clients; i_++) {
			sum += copied_opening_table[i][i_];
		}
		if (sum > 0) opening_table[i] = 1;
	}

	for (int j = 0; j < n_clients; j++) {
		int facility_r = 0;
		int facility_c = 0;

		for (int i = 0; i < n_facilities; i++) {
			for (int i_ = 0; i_ < n_clients; i_++) {
				if (copied_connection_table[i][i_][j] == 1) {
					facility_r = i;
					facility_c = i_;
				}
			}
		}

		connection_table[facility_r][j] = 1;
	}
	/////////
	/* Print for Debug 
	for (int j = 0; j < n_clients; j++) {
		int sum = 0;
		int connected_facility = 0;
		for (int i = 0; i < n_facilities; i++) {
			sum += connection_table[i][j];
			if (connection_table[i][j] == true)
				connected_facility = i;
		}
		cout << j << "th client's connection sum: " << sum << ", connected facility is " << connected_facility << ", connection cost is " << connection_cost[connected_facility][j] << endl;
	}
	*/
	/////////
	/* Calculate Cost ( objective function ) */
	double total_opening_cost = 0, total_connection_cost = 0.0;

	/* calculate total connection_cost */
	for (int j = 0; j < n_clients; j++) {  // calculate total connection_cost
		for (int i = 0; i < n_facilities; i++) {
			total_connection_cost += connection_table[i][j] * connection_cost[i][j];
		}
	}

	/* calculate total opening cost */
	for (int i = 0; i < n_facilities; i++) {
		total_opening_cost += opening_table[i] * opening_cost[i];
	}

	/* Connect optimally */
	post_process();

	rounded_cost = total_opening_cost + total_connection_cost;
	delete order_of_client;
	return rounded_cost;
}


int myrandom(int i) { return std::rand() % i; }
FacilityLocation::FacilityLocation(int argc, char* argv[])
{
	std::vector<int> myvector;
	std::default_random_engine generator;
	int i;

	ifstream in(argv[1]);
	in >> n_facilities;
	in >> n_clients;
	cout << "n_facilities = " << n_facilities << endl;
	cout << "n_clients = " << n_clients << endl;

	/* memory allocation of opening_cost(f), connection_cost(d) */
	opening_cost = new double[n_facilities];
	for (int i = 0; i < n_facilities; i++) {
		in >> opening_cost[i];
	}

	connection_cost = new double*[n_facilities]; // dynamic allocation
	for (int i = 0; i < n_facilities; i++) {
		connection_cost[i] = new double[n_clients]; // dynamic allocation
		for (int j = 0; j < n_clients; j++) {
			connection_cost[i][j] = _HUGE_ENUF;
		}
	}
	int f, c;
	while (in) {
		in >> f >> c;
		in >> connection_cost[f][c];
	}

	/* memory allocation of opening_variable(y), connection_variable(x)*/
	opening_variable = new double[n_facilities];
	connection_variable = new double*[n_facilities];
	for (int i = 0; i < n_facilities; ++i) {
		connection_variable[i] = new double[n_clients];
	}

	/* memory allocation of the expoential clocks of the facilities */
	exponential_clock = new double*[n_facilities];
	for (int i = 0; i < n_facilities; ++i) {
		exponential_clock[i] = new double[n_clients];
	}

	/* memory allocation & generation of clock_of_client */
	// set some values:
	for (int i = 0; i < n_clients; ++i) myvector.push_back(i); // 1 2 3 4 5 6 7 8 9
															   // using built-in random generator:
	std::random_shuffle(myvector.begin(), myvector.end());

	// using myrandom:
	std::random_shuffle(myvector.begin(), myvector.end(), myrandom);
	i = 0;
	clock_of_client = new int[n_clients]; // dynamic allocation
	for (std::vector<int>::iterator it = myvector.begin(); it != myvector.end(); ++it, ++i)
		clock_of_client[i] = *it;

	/* memory allocation of copied_opening_cost(f'), copied_connection_cost(d') */
	copied_opening_cost = new double*[n_facilities];
	for (int i = 0; i < n_facilities; ++i) {
		copied_opening_cost[i] = new double[n_clients];
	}
	copied_connection_cost = new double**[n_facilities];
	for (int i = 0; i < n_facilities; i++) {
		copied_connection_cost[i] = new double*[n_clients];
		for (int i_ = 0; i_ < n_clients; i_++) {
			copied_connection_cost[i][i_] = new double[n_clients];
		}
	}

	/* memory allocation of copied_opening_variable(y'), copied_connection_variable(x') */
	copied_opening_variable = new double*[n_facilities];
	for (int i = 0; i < n_facilities; ++i) {
		copied_opening_variable[i] = new double[n_clients];
	}
	copied_connection_variable = new double**[n_facilities];
	for (int i = 0; i < n_facilities; i++) {
		copied_connection_variable[i] = new double*[n_clients];
		for (int i_ = 0; i_ < n_clients; i_++) {
			copied_connection_variable[i][i_] = new double[n_clients];
		}
	}

	/* memory allocation of copied_opening_table(M), copied_connection_table(M') */
	copied_opening_table = new bool*[n_facilities];
	for (int i = 0; i < n_facilities; ++i) {
		copied_opening_table[i] = new bool[n_clients];
	}
	copied_connection_table = new bool**[n_facilities];
	for (int i = 0; i < n_facilities; i++) {
		copied_connection_table[i] = new bool*[n_clients];
		for (int i_ = 0; i_ < n_clients; i_++) {
			copied_connection_table[i][i_] = new bool[n_clients];
		}
	}

	/* memory allocation of opening_table, connection_table */
	opening_table = new bool[n_facilities];
	connection_table = new bool*[n_facilities];
	for (int i = 0; i < n_facilities; i++) {
		connection_table[i] = new bool[n_clients];
	}

	triangular_inequality();


}

FacilityLocation::~FacilityLocation()
{

	/* memory deallocation of opening_cost(f), connection_cost(d) */
	delete opening_cost;
	for (int i = 0; i < n_facilities; i++) {
		delete[] connection_cost[i];
	}
	delete[] connection_cost;

	/* memory deallocation of opening_variable(y), connection_variable(x)*/
	delete opening_variable;
	for (int i = 0; i < n_facilities; i++) {
		delete[] connection_variable[i];
	}
	delete[] connection_variable;

	/* memory deallocation of the expoential clocks of the facilities */
	for (int i = 0; i < n_facilities; i++) {
		delete[] exponential_clock[i];
	}
	delete[] exponential_clock;

	/* memory deallocation & generation of the orders of the clients */
	delete clock_of_client;

	/* memory deallocation of copied_opening_cost(f'), copied_connection_cost(d') */
	for (int i = 0; i < n_facilities; i++) {
		delete[] copied_opening_cost[i];
	}
	delete[] copied_opening_cost;

	for (int i = 0; i < n_facilities; i++) {
		for (int j = 0; j < n_clients; j++) {
			delete[] copied_connection_cost[i][j];
		}
		delete[] copied_connection_cost[i];
	}
	delete[] copied_connection_cost;

	/* memory deallocation of copied_opening_variable(y'), copied_connection_variable(x') */
	for (int i = 0; i < n_facilities; i++) {
		delete[] copied_opening_variable[i];
	}
	delete[] copied_opening_variable;

	for (int i = 0; i < n_facilities; i++) {
		for (int j = 0; j < n_clients; j++) {
			delete[] copied_connection_variable[i][j];
		}
		delete[] copied_connection_variable[i];
	}
	delete[] copied_connection_variable;

	/* memory deallocation of copied_opening_table(M), copied_connection_table(M') */
	for (int i = 0; i < n_facilities; i++) {
		delete[] copied_opening_table[i];
	}
	delete[] copied_opening_table;

	for (int i = 0; i < n_facilities; i++) {
		for (int j = 0; j < n_clients; j++) {
			delete[] copied_connection_table[i][j];
		}
		delete[] copied_connection_table[i];
	}
	delete[] copied_connection_table;

	/* memory deallocation of opening_table, connection_table */
	delete opening_table;
	for (int i = 0; i < n_facilities; i++) {
		delete[] connection_table[i];
	}
	delete[] connection_table;
}

int minDistance(double dist[], bool sptSet[], unsigned int V)
{
	// Initialize min value
	double min = DBL_MAX;
	int min_index;

	for (int v = 0; v < V; v++)
		if (sptSet[v] == false && dist[v] <= min)
			min = dist[v], min_index = v;

	return min_index;
}

void FacilityLocation::triangular_inequality(void) {
	/* Redefine the bipartite graph */
	double **graph = new double*[n_clients + n_facilities];
	for (int i = 0; i < n_clients + n_facilities; ++i) {
		graph[i] = new double[n_clients + n_facilities];
		for (int j = 0; j < n_clients + n_facilities; ++j) {
			graph[i][j] = 1e+200;
		}
	}
	/* draw the graph */
	for (int i = 0; i < n_facilities; ++i) {
		for (int j = 0; j < n_clients; ++j) {
			graph[i + n_clients][j] = connection_cost[i][j];
			graph[j][i + n_clients] = connection_cost[i][j];
		}
	}
	/* for each client */
	for (int j = 0; j < n_clients; ++j) {
		/* get shortest paths for each facility */
		int src = j;
		double *dist = new double[n_clients + n_facilities];     // The output array.  dist[i] will hold the shortest
																 // distance from src to i

		bool *sptSet = new bool[n_clients + n_facilities]; // sptSet[i] will true if vertex i is included in shortest
														   // path tree or shortest distance from src to i is finalized

														   // Initialize all distances as INFINITE and stpSet[] as false
		for (int i = 0; i < n_clients + n_facilities; i++)
			dist[i] = DBL_MAX, sptSet[i] = false;

		// Distance of source vertex from itself is always 0
		dist[src] = 0;

		// Find shortest path for all vertices
		for (int count = 0; count < n_clients + n_facilities - 1; count++)
		{
			// Pick the minimum distance vertex from the set of vertices not
			// yet processed. u is always equal to src in first iteration.
			int u = minDistance(dist, sptSet, n_clients + n_facilities);

			// Mark the picked vertex as processed
			sptSet[u] = true;

			// Update dist value of the adjacent vertices of the picked vertex.
			for (int v = 0; v < n_clients + n_facilities; v++)

				// Update dist[v] only if is not in sptSet, there is an edge from 
				// u to v, and total weight of path from src to  v through u is 
				// smaller than current value of dist[v]

				if (!sptSet[v] && graph[u][v] && CompareDoubleUlps(dist[u], DBL_MAX) != 0
					&& CompareDoubleUlps(dist[u] + graph[u][v], dist[v]) < 0)
					dist[v] = dist[u] + graph[u][v];
		}

		/* assign min(dist[u], e(i, src)) to connection_cost[i][src] for each facility i */
		for (int i = n_clients; i < this->n_facilities + this->n_clients; ++i) {
			if (dist[i] < this->connection_cost[i - n_clients][src])
				this->connection_cost[i - n_clients][src] = dist[i];
		}
		delete dist;
		delete sptSet;

	}
	for (int i = 0; i < n_clients + n_facilities; ++i) {
		delete graph[i];
	}
	delete graph;
}
