#include "FacilityLocation.h"

void FacilityLocation::random_sample(void)
{

}

double FacilityLocation::LP_solve(void)
{
	return 0;
}

void FacilityLocation::round(void)
{

}

int myrandom(int i) { return std::rand() % i; }
FacilityLocation::FacilityLocation(void)
{
	/* generation of the expoential clocks of the facilities */
	//double y_i; // facility가 열려있으면 1, 닫혀있으면 0, fraction variable
	std::default_random_engine generator;

	std::srand(unsigned(std::time(0)));

	//각각의 facility의 exponential_clock 값 설정
	for (int i = 0; i < NUM_OF_F; ++i) {
		double y_i = ((double)rand() / (RAND_MAX));
		std::exponential_distribution<double> distribution(y_i);
		//double number = distribution(generator);
		//exponential_clock[i] = y_i*exp(double(-1 * y_i*number));
		exponential_clock[i] = distribution(generator);
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
	for (std::vector<int>::iterator it = myvector.begin(); it != myvector.end(); ++it, ++i)
		order_of_client[i] = *it;


	/* settiing costs of openings and connections */
	for (int i = 0, j = 0; i < NUM_OF_F;) {
		connection_cost[i*NUM_OF_C + j] = (int)rand() % 100 + 1;
		j++;
		if (j == NUM_OF_C) {
			i++;
			j = 0;
		}
	}
	for (int i = 0; i < NUM_OF_F; i++) {
		opening_cost[i] = (int)rand() % 100 + 1;
	}

	// print out content:
	/*
	for (int i = 0; i < NUM_OF_C; i++) {
		cout << "c" << i << ": " << order_of_client[i] << endl;
	}
	for (int i = 0; i < NUM_OF_F; i++) {
		cout << "f" << i << ": " << exponential_clock[i] << endl;
	}
	cout << "---------cost---------------" << endl;

	for (int i = 0; i < NUM_OF_F*NUM_OF_C; i++) {
		cout << "c" << i << ": " << connection_cost[i] << endl;
	}

	for (int i = 0; i < NUM_OF_F; i++) {
		cout << "f" << i << ": " << opening_cost[i] << endl;
	}
	*/
}


void FacilityLocation::brute_force(void)
{

}

unsigned int FacilityLocation::objective(void) 
{
	return 0;
}