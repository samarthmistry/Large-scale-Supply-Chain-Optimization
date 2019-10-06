// Include the header file
#include <ilcplex/ilocplex.h>
#include<vector>

// Use the macro before class definition
ILOSTLBEGIN

// Define two-dimensional variable array
typedef IloArray<IloNumVarArray>    IloNumVarArray2;

// Defining Solve function having IP formulation
int main()
{
	// Create the environment object
	IloEnv env;
	
	// Write try-except clause for exception handling
	try
	{

		// Define input file and load values
		const char* filename = "UFL.dat";
		ifstream file(filename);
		if (!file) {
			cerr << "No such file: " << filename << endl;
			// Define output file
			ofstream myfile("UFL_output.txt", ios_base::app);

			throw(-1);
		}

		// Load data 
		// First array is client demand, second array is distribution cost matrix (between a facility and a client), and last array is facility fixed cost
		IloNumArray client_dem(env), dist_cost(env), fac_cost(env);
		file >> client_dem >> dist_cost >> fac_cost;

		// Define no. of clients, cost array size and facility cost array size 
		IloInt clients = client_dem.getSize();
		IloInt co = dist_cost.getSize();
		IloInt fac = fac_cost.getSize();

		// Uncomment to check if data is loaded properly and values are correct
		/*cout << "no. of clients = " << clients << endl;
		cout << "no. of costs = " << co << endl;
		cout << "no. of facilities = " << fac << endl;*/

		// Define model
		IloModel mod(env);

		// Define counters
		IloInt p, q;

		// Define variables - Constraint 1 (Also, equation 4 in the pdf formulation)

		// Define Xpq variables - Proportion of client q's demand satisfied by facility p
		IloNumVarArray2 x(env);
		for (p = 0; p < fac; p++) {
			x.add(IloNumVarArray(env, clients, 0.0, IloInfinity));
		}
		// Define boolean variable for facility
		// y = 1 if facility is open, else 0
		IloBoolVarArray y(env);
		for (p = 0; p < fac; p++) {
			y.add(IloBoolVar(env));
		}
		
		// Define Objective function (Equation 1 in the pdf formulation)
		// Total cost would be equal to delivery cost from facility p to client Q in addition with facility open fixed cost

		IloExpr TotalDistCost(env), TotalFacCost(env);
		for (p = 0; p < fac; p++) {
			for (q = 0; q < clients; q++) {
				TotalDistCost += dist_cost[clients*p + q] * x[p][q];
			}
			TotalFacCost += fac_cost[p] * y[p];
		}
		
		// Add objective function
		
		// We want to minimize total cost (Equation 1 in pdf formulation)
		mod.add(IloMinimize(env, TotalDistCost + TotalFacCost));
		TotalDistCost.end();
		TotalFacCost.end();

		// 	Constraint 2 (Equation 2 in the pdf formulation) - for each client, total quantity delivered from all facilities p is equal to total demand at that client
		for (q = 0; q < clients; q++) {
			IloExpr constr2(env);
			for (p = 0; p < fac; p++) {
				constr2 += x[p][q];
			}
			mod.add(constr2 == client_dem[q]);
			constr2.end();
		}

		// Constraint 3 (Equation 3 in the pdf formulation) - for each facility (If open), total quantity delivered to all clients should be lesser than or equal to net demand of all clients
		for (p = 0; p < fac; p++) {
			IloExpr constr3(env), dem(env);
			for (q = 0; q < clients; q++) {
				constr3 += x[p][q];
				dem += client_dem[q];
			}
			mod.add(constr3 <= dem * y[p]);
			constr3.end();
		}

		// Output formulation 
		IloCplex cplex(mod);
		cplex.exportModel("UFL.lp");

		// Define start time and end time variables to know computation time taken 
		IloNum StartTime, Endtime;
		StartTime = cplex.getTime();
	
		// Command to disable the cuts [uncomment for section B, C, and D results]
		//	cplex.setParam(IloCplex::Param::MIP::Limits::CutsFactor, 1.0);

		// Command to set up MIP interval  [uncomment for section C and D results]
		//	cplex.setParam(IloCplex::Param::MIP::Interval, 50000);

		// Commands for preprocessing, node and variable selection strategies [uncomment for section D results]
			// Part- D.a - Turn preprocessing off
			// cplex.setParam(IloCplex::Param::Preprocessing::Presolve, 0);

			// Part- D.b - MIP Node selection strategy
				// cplex.setParam(IloCplex::Param::MIP::Strategy::NodeSelect, 0); // for DFS
				// cplex.setParam(IloCplex::Param::MIP::Strategy::NodeSelect, 1); // for BBS
				// MIP Solve time = 273.016
				// No.of branch and bound nodes = 482663

			// Part- D.c
				 // cplex.setParam(IloCplex::Param::MIP::Strategy::VariableSelect, -1); //Minimum infeasibility
				 // cplex.setParam(IloCplex::Param::MIP::Strategy::VariableSelect, 1); //Maximum infeasibility

		cplex.solve();

		Endtime = cplex.getTime();
		
		// Disable logging for LP solution
		cplex.setOut(env.getNullStream());

		// Section A and Section B results -

		// Part a -- List of facilities with non-zero production in the optimal solution for this MIP instance along with the total production at each of these facilities
		cout << "(a) List of facilities with non-zero production in the optimal solution with the total production at each of these facilities" << endl;
		
		for (p = 0; p < fac; p++) {
			int prod_fac = 0;

			if (cplex.getValue(y[p]) > 0) {
				for (q = 0; q < clients; q++) {
					if (cplex.getValue(x[p][q]) > 0) {
						// cout << "Production quantity at client " << q + 1 << " with quantity = " << cplex.getValue(x[p][q]) << endl;
						prod_fac += cplex.getValue(x[p][q]);
					}
				}
				cout << "Facility " << p + 1 << " produces " << prod_fac << " units." << endl;
			}
		}
		
		cout << "\n" << endl;
		
		// Part b -- Optimal objective function value for this MIP instance
		cout << "(b) Optimal objective function value for this MIP instance = " << cplex.getObjValue() << endl << endl;

		// store MIP objective value for Part-h
		IloNum MIPobjVal = cplex.getObjValue();

		// Part c -- No. of integer and continuous variables
		cout << "(c)" << endl << "No. of integer variables = " << y.getSize() << endl;

		int c1=0;
		for (p = 0; p < fac; p++) {
			c1 += x[p].getSize();
		}
		cout << "No. of continuous variables = " << c1 << endl;

		// No. of constraints
		cout << "No. of constraints in the model = " << cplex.getNrows() << endl << endl;

		// Part f
		cout << "(f) MIP Solve time = " << Endtime - StartTime << endl << endl;

		// Part g
		cout << "(g) No. of branch and bound nodes = " << cplex.getNnodes() << endl << endl;

		// Part i
		// No. of cuts
		
		int num_cuts = 0;
		for (IloCplex::CutType c : {
			IloCplex::CutCover,
				IloCplex::CutGubCover,
				IloCplex::CutFlowCover,
				IloCplex::CutClique,
				IloCplex::CutFrac,
				IloCplex::CutMCF,
				IloCplex::CutMir,
				IloCplex::CutFlowPath,
				IloCplex::CutDisj,
				IloCplex::CutImplBd,
				IloCplex::CutZeroHalf,
				IloCplex::CutLocalCover,
				IloCplex::CutTighten,
				IloCplex::CutObjDisj,
				IloCplex::CutLiftProj,
				IloCplex::CutUser,
				IloCplex::CutTable,
				IloCplex::CutSolnPool}) {
			num_cuts += cplex.getNcuts(c);
		}
		cout << "(i) No. of all default cuts generated by CPLEX = " << num_cuts << endl << endl;
	
		// For part- d, e and h
		// LP relaxation solution
		// env.out() << endl << endl << "***Solving for LP relaxation solution***" << endl << endl;

		// convert y binary variable to float values
		 mod.add(IloConversion(env, y, ILOFLOAT));

		 // Define start time and end time variables to calculate computation time taken by LP relaxation
		 IloNum LP_StartTime, LP_Endtime;
		 LP_StartTime = cplex.getTime();

		 cplex.solve();

		 LP_Endtime = cplex.getTime();

		 // LP relaxation output
		 cout << "(d) LP relaxation Solve time = " << LP_Endtime - LP_StartTime << endl << endl;
		 // env.out() << "LP relaxation Solution status: " << cplex.getStatus() << endl;
		 cout << "(e) Optimal objective function value for LP relaxation = " << cplex.getObjValue() << endl << endl;

		 // store LP objective value for Part-h
		 IloNum LPobjVal = cplex.getObjValue();

		 env.out() << "(h) Percentage of gap between LP optimal objective value and MIP objective value = " << 100 * (MIPobjVal - LPobjVal) / MIPobjVal << "%" << endl;

	}

	// Write catch clause for exception handling
	catch (IloException& ex) {
		cerr << "Error: " << ex << endl;
	}
	catch (...) {
		cerr << "Error: Unknown exception caught!" << endl;
	}

	int BP1;
	cin >> BP1;

	// Destroy the environment
	env.end();

	return 0;
}
