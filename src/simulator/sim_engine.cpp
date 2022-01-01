#include <chrono> /* For time */
#include "sim_engine.hpp"
#include "KLUSupport"

// TODO - EIGEN_USE_KLU
#if 1
	/* KLU direct solvers - Supports both int/long */
	typedef Eigen::KLU<SparMatD> direct_solver;
	typedef Eigen::KLU<SparMatCompD> direct_solver_c;
#else
	/* Eigens direct solvers - Support both int/long int */
	typedef Eigen::SparseLU<SparMatD, Eigen::COLAMDOrdering<IntTp>> direct_solver;
	typedef Eigen::SparseLU<SparMatCompD, Eigen::COLAMDOrdering<IntTp>> direct_solver_c;
#endif


/* TODO - Comment */
void simulator_engine::run(Circuit &circuit_manager)
{
	using namespace std;
	using namespace chrono;

	/* Statistics */
	auto begin = high_resolution_clock::now();

	/* Depending on analysis, call the appropriate sub-simulator */
	switch(circuit_manager.getAnalysisType())
	{
		case OP: OP_analysis(circuit_manager); break;
		case DC: DC_analysis(circuit_manager); break;
		case TRAN: TRAN_analysis(circuit_manager); break;
		case AC: AC_analysis(circuit_manager); break;
		default: OP_analysis(circuit_manager); break;
	}

	/* Statistics */
	auto end = high_resolution_clock::now();

	std::cout << std::endl << "************************************" << std::endl;
	std::cout << "Total simulation time: " << duration_cast<milliseconds>(end-begin).count() << "ms"  << std::endl;
	std::cout << "************************************" << std::endl << std::endl;
}



/* TODO - Comment */
void simulator_engine::OP_analysis(Circuit &circuit_manager)
{
	SparMatD mat;
	DensVecD rh;
	DensVecD res;

	/* The matrix has to be created once */
	this->_mna_engine.CreateMNASystemOP(circuit_manager, mat, rh);

	/* Perform solve */
	solve_linear(mat, rh, res);

	/* TODO - Debug */
//	auto &nodes = circuit_manager.getNodes();
//	for(auto it = nodes.begin(); it != nodes.end(); it++)
//	{
//		std::cout << "V[" << it->first << "]:" << res[it->second] << std::endl;
//	}

	/* Copy result */
	this->_results_d = res;
}

/* TODO - Comment */
void simulator_engine::DC_analysis(Circuit &circuit_manager)
{
	SparMatD mat;
	DenseMatD rhs, res;

	/* The matrix has to be created once */
	this->_mna_engine.CreateMNASystemDC(circuit_manager, mat, rhs);

	/* Solve */
	solve_linear(mat, rhs, res);

//	/* TODO - Debug */
//	auto &nodes = circuit_manager.getNodes();
//	for(auto it = nodes.begin(); it != nodes.end(); it++)
//	{
//		std::cout << "V[" << it->first << "]:" <<
//					res(it->second, 0) << "\t" <<
//					res(it->second, 1) << "\t" <<
//					res(it->second, 2) << "\t" <<
//					std::endl;
//	}


	this->_results_d = res;
}

/* TODO - Implement */
void simulator_engine::TRAN_analysis(Circuit &circuit_manager)
{
	/* TODO - EULER */
	/* TODO - Start time is selected at 0 for now */

	/* First compute the operating point of the circuit.
	 * This serves as the initial x for time 0 */

	/* Matrices */
	SparMatD op_mat, tran_mat;
	DensVecD cur;
	DenseMatD res;

	/* Solver instance */
	direct_solver solver;

	/* Simulation vector */
	auto &sim_vector = this->_mna_engine.getSimVals();
	auto timestep = circuit_manager.getSimStep();
	auto mat_sz = this->_mna_engine.getSystemDim();
	auto sim_sz = this->_mna_engine.getSimDim();

	/* Copy the initial vector to the matrix */
	res.resize(mat_sz, sim_sz); // Also the initial timepoint 0

	/* Create the matrices */
	this->_mna_engine.CreateMNASystemOP(circuit_manager, op_mat, cur);
	this->_mna_engine.CreateMNASystemTRAN(circuit_manager, tran_mat);

	/****** 1st step find the op vector (t = 0) ******/

	/* Analyze pattern */
	solver.analyzePattern(op_mat);

	/* Compute numerical factorization */
	solver.factorize(op_mat);

	/* t = 0 */
	res.col(0) = solver.solve(cur);

	/****** 2nd step compute the final transient array ******/
	tran_mat = (1/timestep) * tran_mat;
	op_mat = op_mat + tran_mat;	// A = G + 1/h * C

	/* Analyze pattern */
	solver.analyzePattern(op_mat);

	/* Compute numerical factorization */
	solver.factorize(op_mat);

	/****** 3rd step Run for each simulation timepoint ******/
	IntTp rep = 0;

	/* Solve A*x = C*e(t) + x(t0)*/
	for(auto it : sim_vector)
	{
		/* Skip t=0 timepoint (already computed from OP) */
		if(it == 0.0) continue;

		cur = tran_mat * res.col(rep);	// C/h*x(tk-1) + e(tk)

		this->_mna_engine.UpdateTRANVec(circuit_manager, cur, it);

		res.col(rep + 1) = solver.solve(cur);

		rep++;
	}

	/* TODO - TRAP */

//	/* TODO - Debug */
//	auto &nodes = circuit_manager.getNodes();
//	for(auto it = nodes.begin(); it != nodes.end(); it++)
//	{
//		std::cout << "V[" << it->first << "]:" <<
//					res(it->second, 0) << "\t" <<
//					res(it->second, 1) << "\t" <<
//					res(it->second, 2) << "\t" <<
//					std::endl;
//	}


	/* Copy result */
	this->_results_d = res;
}

/* TODO - Implement */
void simulator_engine::AC_analysis(Circuit &circuit_manager)
{

}



/* TODO - Comment */
void simulator_engine::solve_linear(SparMatD &mat, DensVecD &rh, DensVecD &res)
{
	direct_solver solver;

	/* Analyze pattern */
	solver.analyzePattern(mat);

	/* Compute numerical factorization */
	solver.factorize(mat);

	/* Solve */
	res = solver.solve(rh);

//	indirect_solver solver2;
//
//	solver2.setTolerance(1e-3);
//
//	solver2.compute(mat);
//	res = solver2.solve(rh);

}

/* TODO - Implement */
void simulator_engine::solve_linear(SparMatD &mat, DenseMatD &rhs, DenseMatD &res)
{
	direct_solver solver;

	/* Analyze pattern */
	solver.analyzePattern(mat);

	/* Compute numerical factorization */
	solver.factorize(mat);

	/* Solve */
	res = solver.solve(rhs);
}

/* TODO - Implement */
void simulator_engine::solve_linear(SparMatCompD &mat, DensVecCompD &rh, DensVecCompD &res)
{
	direct_solver_c solver;

	/* Analyze pattern */
	solver.analyzePattern(mat);

	/* Compute numerical factorization */
	solver.factorize(mat);

	/* Solve */
	res = solver.solve(rh);
}

