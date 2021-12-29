#include <chrono> /* For time */
#include "sim_engine.hpp"

/* Some useful shortcuts */
typedef Eigen::SparseLU<SparMatD, Eigen::COLAMDOrdering<long int>> direct_solver;
typedef Eigen::SparseLU<SparMatCompD, Eigen::COLAMDOrdering<long int>> direct_solver_compl;
//typedef Eigen::BiCGSTAB<SparMatD, Eigen::IncompleteLUT<double, long int>> indirect_solver; /* TODO - ? */

/* TODO - Comment */
void simulator_engine::run(Circuit &circuit_manager)
{
	using namespace std;
	using namespace chrono;

	/* Statistics */
	auto begin = high_resolution_clock::now();

	/* Depending on analysis, call the appropriate sub-simulator */
	switch(this->_analysis)
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
	solve(mat, rh, res);

//	std::cout << "Res:" << std::endl;
//	for(auto it = circuit_manager.getNodes().begin(); it != circuit_manager.getNodes().end(); it++)
//	{
//		std::cout << it->first << "[" << it->second << "]\t" << res(it->second) << std::endl;
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
	solve(mat, rhs, res);

//	std::cout << "Res:" << std::endl;
//	for(auto it = circuit_manager.getNodes().begin(); it != circuit_manager.getNodes().end(); it++)
//	{
//		std::cout << it->first << "[" << it->second << "]\t" << res(it->second, 0) <<
//					"\t" << res(it->second, 1) << "\t" << res(it->second, 2) <<std::endl;
//	}
}

/* TODO - Implement */
void simulator_engine::TRAN_analysis(Circuit &circuit_manager)
{

}

/* TODO - Implement */
void simulator_engine::AC_analysis(Circuit &circuit_manager)
{

}

/* TODO - Comment */
void simulator_engine::solve(SparMatD &mat, DensVecD &rh, DensVecD &res)
{
	//if(DIRECT) TODO - Implement direct-indirect

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
void simulator_engine::solve(SparMatD &mat, DenseMatD &rhs, DenseMatD &res)
{
//	if(DIRECT) TODO - Implement direct-indirect

	direct_solver solver;

	/* Analyze pattern */
	solver.analyzePattern(mat);

	/* Compute numerical factorization */
	solver.factorize(mat);

	/* Solve */
	res = solver.solve(rhs);
}

/* TODO - Implement */
void simulator_engine::solve(SparMatCompD &mat, DensVecCompD &rh, DensVecCompD &res)
{

}
