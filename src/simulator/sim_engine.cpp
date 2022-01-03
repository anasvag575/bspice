#include <chrono> /* For time */
#include "sim_engine.hpp"
#include "KLUSupport"

#ifdef BSPICE_EIGEN_USE_KLU
	/* KLU direct solvers - Supports both int/long */
	typedef Eigen::KLU<SparMatD> direct_solver;
	typedef Eigen::KLU<SparMatCompD> direct_solver_c;
#else
	/* Eigens direct solvers - Support both int/long int */
	typedef Eigen::SparseLU<SparMatD, Eigen::COLAMDOrdering<IntTp>> direct_solver;
	typedef Eigen::SparseLU<SparMatCompD, Eigen::COLAMDOrdering<IntTp>> direct_solver_c;
#endif


/*!
	@brief      Performs a simulation run based on the initialization performed.
	@param		circuit_manager		The circuit where the simulation is performed.
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::run(Circuit &circuit_manager)
{
	using namespace std;
	using namespace chrono;

	return_codes_e ret;

	/* Statistics */
	auto begin = high_resolution_clock::now();

	/* Depending on analysis, call the appropriate sub-simulator */
	switch(circuit_manager.getAnalysisType())
	{
		case OP: ret = OP_analysis(circuit_manager); break;
		case DC: ret = DC_analysis(circuit_manager); break;
		case TRAN: ret = TRAN_analysis(circuit_manager); break;
		case AC: ret = AC_analysis(circuit_manager); break;
		default: ret = OP_analysis(circuit_manager); break;
	}

	/* Statistics */
	auto end = high_resolution_clock::now();

	std::cout << std::endl << "************************************" << std::endl;
	std::cout << "Total simulation time: " << duration_cast<milliseconds>(end-begin).count() << "ms"  << std::endl;
	std::cout << "Total simulation points: " << this->_mna_engine.getSimDim() << std::endl;
	std::cout << "System size: " << this->_mna_engine.getSystemDim() << std::endl;
	std::cout << "************************************" << std::endl << std::endl;

	if(ret == RETURN_SUCCESS) this->_run = true;

	return ret;
}


/*!
	@brief      Performs an operating point (OP) simulation.
	@param		circuit_manager		The circuit where the simulation is performed.
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::OP_analysis(Circuit &circuit_manager)
{
	using namespace Eigen;

	/* Solver and matrices */
	direct_solver solver;
	SparMatD mat;
	DensVecD rh;

	/* 1) MNA formation */
	this->_mna_engine.CreateMNASystemOP(circuit_manager, mat, rh);

	/* 2) Factorization/Symbolic analysis*/
	solver.compute(mat);

	/* Checks */
	if(solver.info() != Success) return FAIL_SIMULATOR_FACTORIZATION;

	/* Solve */
	this->_results_d = solver.solve(rh);

//	/* TODO - Debug */
//	auto &nodes = circuit_manager.getNodes();
//	for(auto it = nodes.begin(); it != nodes.end(); it++)
//	{
//		std::cout << "V[" << it->first << "]:" << this->_results_d(it->second, 0) << std::endl;
//	}

	return (solver.info() != Success) ? FAIL_SIMULATOR_SOLVE : RETURN_SUCCESS;
}

/*!
	@brief      Performs a direct current (DC) simulation.
	@param		circuit_manager		The circuit where the simulation is performed.
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::DC_analysis(Circuit &circuit_manager)
{
	using namespace Eigen;

	direct_solver solver;
	SparMatD mat;
	DenseMatD rhs;

	/* The matrix has to be created once */
	this->_mna_engine.CreateMNASystemDC(circuit_manager, mat, rhs);

	/* 2) Factorization/Symbolic analysis*/
	solver.compute(mat);

	/* Checks */
	if(solver.info() != Success) return FAIL_SIMULATOR_FACTORIZATION;

	/* Solve */
	this->_results_d = solver.solve(rhs);

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

	return (solver.info() != Success) ? FAIL_SIMULATOR_SOLVE : RETURN_SUCCESS;
}

/*!
	@brief      Performs a transient (TRAN) simulation.
	@param		circuit_manager		The circuit where the simulation is performed.
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::TRAN_analysis(Circuit &circuit_manager)
{
	/* TODO - EULER */
	/* TODO - Start time is selected at 0 for now */

	/* First compute the operating point of the circuit.
	 * This serves as the initial x for time 0 */

	/* Matrices */
	SparMatD op_mat, tran_mat;
	DensVecD cur;

	/* Solver instance */
	direct_solver solver;

	/* Simulation vector */
	auto &sim_vector = this->_mna_engine.getSimVals();
	auto timestep = circuit_manager.getSimStep();
	double inverse_timestep = 1/timestep;
	auto mat_sz = this->_mna_engine.getSystemDim();
	auto sim_sz = this->_mna_engine.getSimDim();

	/* Copy the initial vector to the matrix */
	this->_results_d.resize(mat_sz, sim_sz); // Also the initial timepoint 0

	/* Create the matrices */
	this->_mna_engine.CreateMNASystemOP(circuit_manager, op_mat, cur);
	this->_mna_engine.CreateMNASystemTRAN(circuit_manager, tran_mat);

	/****** 1st step find the op vector (t = 0) ******/

	std::cout << "Sim System Coil IVS: " << this->_mna_engine.getSimDim() << " "
										 << this->_mna_engine.getSystemDim()<< " "
										 << this->_mna_engine.getCoilOffset()<< " "
										 << this->_mna_engine.getIVSOffset()<< "\n\n";

	/* Factorization/Symbolic analysis*/
	solver.compute(op_mat);

	/* t = 0 */
	this->_results_d.col(0) = solver.solve(cur);

#ifdef BSPICE_TRAN_USE_EULER
	/****** 2nd step compute the final transient array ******/\

	tran_mat = inverse_timestep * tran_mat;
	op_mat = op_mat + tran_mat;	// A = G + 1/h * C

	/* Factorization/Symbolic analysis*/
	solver.compute(op_mat);

	/****** 3rd step Run for each simulation timepoint ******/

	/* Solve A*x = C*e(t) + x(t0)*/
	for(size_t i = 1; i < sim_vector.size(); i++) /* Skip t=0 timepoint (already computed from OP) */
	{
		cur = tran_mat * this->_results_d.col(i - 1);	// C/h*x(tk-1) + e(tk)
//		this->_mna_engine.UpdateTRANVec(circuit_manager, cur, sim_vector[i]);

		DensVecD tmp = DensVecD::Zero(mat_sz);
		this->_mna_engine.UpdateTRANVec(circuit_manager, tmp, sim_vector[i]);

//		if(i == 500)
//		{
//			std::cout << "Time is: " << i * timestep << "\n";
//			std::cout << std::scientific;
//			auto &nodes = circuit_manager.getNodes();
//
//			for(auto it = nodes.begin(); it != nodes.end(); it++)
//			{
//				std::cout << "Cur V[" << it->first << "]:" <<
//						cur[it->second] << "\t" << std::endl;
//			}
//
//			for(auto it = nodes.begin(); it != nodes.end(); it++)
//			{
//				std::cout << "Tmp V[" << it->first << "]:" <<
//						tmp[it->second] << "\t" << std::endl;
//			}
//
//		}
		cur = cur + tmp;

		this->_results_d.col(i) = solver.solve(cur);
	}
#else
	/****** 2nd step compute the final transient array ******/\
	SparMatD tmp;
	tmp = op_mat;
	tran_mat = 2*inverse_timestep * tran_mat;

	/* Left hand matrix => Gnew0 = 2*C/h + G */
	op_mat = op_mat + tran_mat;

	/* Right hand matrix => Gnew1 = 2*C/h - G */
	tran_mat = tran_mat - tmp;

	/* Factorization/Symbolic analysis*/
	solver.compute(op_mat);

	/****** 3rd step Run for each simulation timepoint ******/

	/* Solve A*x = C*e(t) + x(t0)*/
	for(size_t i = 1; i < sim_vector.size(); i++) /* Skip t=0 timepoint (already computed from OP) */
	{
		cur = tran_mat * this->_results_d.col(i - 1);
		this->_mna_engine.UpdateTRANVec(circuit_manager, cur, sim_vector[i]);
		this->_mna_engine.UpdateTRANVec(circuit_manager, cur, sim_vector[i - 1]);

		this->_results_d.col(i) = solver.solve(cur);
	}
#endif

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

	return RETURN_SUCCESS;
}

/*!
	@brief      Performs an alternating current (AC) simulation.
	@param		circuit_manager		The circuit where the simulation is performed.
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::AC_analysis(Circuit &circuit_manager)
{
	/* TODO - Implement */
	return RETURN_SUCCESS;
}
