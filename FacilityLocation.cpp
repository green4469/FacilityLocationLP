#include <iostream>
#include <ilcplex/ilocplex.h>

using namespace std;

int main() {

	IloEnv env;
	IloModel model(env);

	IloNumVar x1 = IloNumVar(env, 0, IloInfinity);
	IloNumVar x2 = IloNumVar(env, 0, IloInfinity);

	IloRange range1 = IloRange(env, -IloInfinity, 8);
	IloRange range2 = IloRange(env, -IloInfinity, 9);
	IloObjective obj = IloMaximize(env, 0);

	range1.setLinearCoef(x1, 3);
	range1.setLinearCoef(x2, 5);

	range2.setLinearCoef(x1, 7);
	range2.setLinearCoef(x2, 4);

	obj.setLinearCoef(x1, 1);
	obj.setLinearCoef(x2, 1);

	model.add(obj);
	model.add(range1);
	model.add(range2);

	IloCplex solver(model);

	solver.solve();

	cout << solver.getObjValue() << endl;
	cout << solver.getValue(x1) << " " << solver.getValue(x2) << endl;

	system("pause");
}