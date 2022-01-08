#include <chrono>       /* For time */
#include "KLUSupport"   /* For Eigen KLU */
#include "sim_engine.hpp"

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
	using std::cout;
	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::milliseconds;    // Accuracy for report

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

	cout << "\n************************************\n";
	cout << "Total simulation time: " << duration_cast<milliseconds>(end-begin).count() << "ms\n";
	cout << "Total simulation points: " << this->_mna_engine.getSimDim() << "\n";
	cout << "System size: " << this->_mna_engine.getSystemDim() << "\n";
	cout << "************************************\n\n";

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
    using Eigen::Success; // Eigen success value

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

	return (solver.info() != Success) ? FAIL_SIMULATOR_SOLVE : RETURN_SUCCESS;
}

/*!
	@brief      Performs a direct current (DC) simulation.
	@param		circuit_manager		The circuit where the simulation is performed.
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::DC_analysis(Circuit &circuit_manager)
{
    using Eigen::Success; // Eigen success value

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

	return (solver.info() != Success) ? FAIL_SIMULATOR_SOLVE : RETURN_SUCCESS;
}

/*!
	@brief      Performs a transient (TRAN) simulation.
	@param		circuit_manager		The circuit where the simulation is performed.
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::TRAN_analysis(Circuit &circuit_manager)
{
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

	/* Factorization/Symbolic analysis */
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
	    /* Create the right hand side */
		cur = tran_mat * this->_results_d.col(i - 1);	// C/h*x(tk-1) + e(tk)
		this->_mna_engine.UpdateTRANVec(circuit_manager, cur, sim_vector[i]);

		/* */
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

	return RETURN_SUCCESS;
}

/*!
	@brief      Performs an alternating current (AC) simulation.
	@param		circuit_manager		The circuit where the simulation is performed.
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::AC_analysis(Circuit &circuit_manager)
{
    using Eigen::Success; // Eigen success value

	/* TODO - Implement */
	return RETURN_SUCCESS;
}

/*!
    @brief      Returns the results of the simulation for any non-AC analysis. The result vectors
    contain the sources currents and node voltages requested for the circuit.
    @param      circuit_manager     The circuit where the simulation is performed.
    @param      res_nodes           The results for the nodes.
    @param      res_sources         The results for the source.
*/
void simulator_engine::getPlotResults(Circuit &circuit_manager, std::vector<std::vector<double>> &res_nodes,
                                      std::vector<std::vector<double>> &res_sources)
{
    auto sim_size = this->_mna_engine.getSimDim();

    /* Get the indices from the MNA engine */
    std::vector<IntTp> nodes_idx, sources_idx;
    this->_mna_engine.CreateIdxVecs(circuit_manager, nodes_idx, sources_idx);

    /* We need the data in {t - col(x), col(x+1)} format */
    for(IntTp i = 0; i < sim_size; i++)
    {
        std::vector<double> tmp_vec, tmp_vec2;

        /* Create the vectors - nodes */
        for(auto it : nodes_idx)
        {
            tmp_vec.push_back(this->_results_d(it, i));
        }

        /* Create the vectors - sources */
        for(auto it : sources_idx)
        {
            tmp_vec2.push_back(this->_results_d(it, i));
        }

        /* Out */
        res_nodes.push_back(tmp_vec);
        res_sources.push_back(tmp_vec2);
    }
}

/*!
    @brief      Returns the results of the simulation for the AC analysis. The result vectors
    contain the sources currents and node voltages requested for the circuit.
    @param      circuit_manager     The circuit where the simulation is performed.
    @param      res_nodes           The results for the nodes.
    @param      res_sources         The results for the source.
*/
void simulator_engine::getPlotResults(Circuit &circuit_manager, std::vector<std::vector<std::complex<double>>> &res_nodes,
                                      std::vector<std::vector<std::complex<double>>> &res_sources)
{
    using std::vector;
    using std::complex;

    auto sim_size = this->_mna_engine.getSimDim();

    /* Get the indices from the MNA engine */
    vector<IntTp> nodes_idx, sources_idx;
    this->_mna_engine.CreateIdxVecs(circuit_manager, nodes_idx, sources_idx);

    /* We need the data in {t - col(x), col(x+1)} format */
    for(IntTp i = 0; i < sim_size; i++)
    {
        vector<complex<double>> tmp_vec, tmp_vec2;

        /* Create the vectors - nodes */
        for(auto it : nodes_idx)
        {
            tmp_vec.push_back(this->_results_cd(it, i));
        }

        /* Create the vectors - sources */
        for(auto it : sources_idx)
        {
            tmp_vec2.push_back(this->_results_cd(it, i));
        }

        /* Out */
        res_nodes.push_back(tmp_vec);
        res_sources.push_back(tmp_vec2);
    }
}

