#include <chrono>       /* For time */
#include "sim_engine.hpp"

/* For solver engines */
#ifdef BSPICE_EIGEN_USE_KLU
    #include "KLUSupport"

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
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::run(void)
{
	using std::cout;
	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::milliseconds;    // Accuracy for report

	return_codes_e ret;
    auto analys_type = this->_mna_engine.getAnalysisType();

	/* Statistics */
	auto begin = high_resolution_clock::now();

	cout << "\n[INFO]: Starting simulation...\n";

	/* Depending on analysis, call the appropriate sub-simulator */
	switch(analys_type)
	{
		case OP: ret = OP_analysis(); break;
		case DC: ret = DC_analysis(); break;
		case TRAN: ret = TRAN_analysis(); break;
		case AC: ret = AC_analysis(); break;
		default: ret = OP_analysis(); break;
	}

	/* Statistics */
	auto end = high_resolution_clock::now();

	if(ret == RETURN_SUCCESS)
	{
        cout << "************************************\n";
        cout << "**********SIMULATION INFO***********\n";
        cout << "************************************\n";
	    cout << "Total simulation time: " << duration_cast<milliseconds>(end-begin).count() << "ms\n";
	    cout << "Total simulation points: " << this->_mna_engine.getSimDim() << "\n";
	    cout << "System size: " << this->_mna_engine.getSystemDim() << "\n";
	    cout << "************************************\n\n";

	    this->_run = true;
	}

	return ret;
}

/*!
	@brief      Performs an operating point (OP) simulation.
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::OP_analysis(void)
{
    using Eigen::Success; // Eigen success value

	/* Solver and matrices */
	direct_solver solver;
	SparMatD mat;
	DensVecD rh;

	/* 1) MNA formation */
	this->_mna_engine.CreateMNASystemOP(mat, rh);

	/* 2) Factorization/Symbolic analysis*/
	solver.compute(mat);

	/* Checks */
	if(solver.info() != Success) return FAIL_SIMULATOR_FACTORIZATION;

	/* 3) Solve */
	DensVecD res = solver.solve(rh);

	/* Checks */
	if(solver.info() != Success) return FAIL_SIMULATOR_SOLVE;

	/* Set results */
	setPlotResults(res);

	return RETURN_SUCCESS;
}

/*!
	@brief      Performs a direct current (DC) simulation.
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::DC_analysis(void)
{
    using Eigen::Success; // Eigen success value

	direct_solver solver;
	SparMatD mat;
    DensVecD rh, sol;

    /* The matrix has to be created once */
    this->_mna_engine.CreateMNASystemOP(mat, rh);

    /* 2) Factorization/Symbolic analysis*/
    solver.compute(mat);

    /* Checks */
    if(solver.info() != Success) return FAIL_SIMULATOR_FACTORIZATION;

    auto &sim_vec = this->_mna_engine.getSimVals();

    /* Solve */
    for(auto it : sim_vec)
    {
        /* Update the vector */
        this->_mna_engine.UpdateMNASystemDCVec(rh, it);

        /* Solve */
        sol = solver.solve(rh);
        setPlotResults(sol);

        /* Checks */
        if(solver.info() != Success) return FAIL_SIMULATOR_SOLVE;
    }

	return RETURN_SUCCESS;
}

/*!
	@brief      Performs a transient (TRAN) simulation.
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::TRAN_analysis(void)
{
    switch(this->_ode_method)
    {
        case BACKWARDS_EULER: return EulerODESolve();
        case TRAPEZOIDAL: return TrapODESolve();
        case GEAR2: return FAIL_SIMULATOR_FALLTHROUTH_ODE_OPTION; //TODO
        default: return FAIL_SIMULATOR_FALLTHROUTH_ODE_OPTION; /* Will never reach */
    }
}

/*!
	@brief      Performs an alternating current (AC) simulation.
	@return		Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::AC_analysis(void)
{
    using Eigen::Success; // Eigen success value

    direct_solver_c solver;
    SparMatCompD mat;
    DensVecCompD rh;

    /* Set up the right hand side */
    this->_mna_engine.CreateMNASystemAC(rh);

    /* Simulation values */
    auto &sim_vector = this->_mna_engine.getSimVals();

    for(size_t i = 0; i < sim_vector.size(); i++)
    {
        /* Create the array for this frequency */
        this->_mna_engine.CreateMNASystemAC(mat, sim_vector[i]);

        /* Solver */
        solver.compute(mat);

        /* Checks */
        if(solver.info() != Success) return FAIL_SIMULATOR_FACTORIZATION;

        /* Return the result */
        DensVecCompD tmp = solver.solve(rh);
        setPlotResultsCd(tmp);

        if(solver.info() != Success) return FAIL_SIMULATOR_SOLVE;
    }


	return RETURN_SUCCESS;
}



/*!
    @brief      Performs the common pre-ODE (for all methods) steps for TRAN analysis.
    @param      tran_mat    The transient MNA contribution matrix to be calculated.
    @param      op_mat      The OP MNA contribution matrix to be calculated.
    @param      op_res      The OP result vector (x(t)=0 for TRAN).
    @return     Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::TRANpresolve(SparMatD &tran_mat, SparMatD &op_mat, DensVecD &op_res)
{
    using Eigen::Success; // Eigen success value

    /* Solver */
    direct_solver solver;

    /* Copy the initial vector to the matrix */
    this->_mna_engine.CreateMNASystemOP(op_mat, op_res);

    /****** 1st step find the op vector (t = 0) ******/

    /* Factorization/Symbolic analysis */
    solver.compute(op_mat);

    /* Checks */
    if(solver.info() != Success) return FAIL_SIMULATOR_FACTORIZATION;

    /* t = 0 */
    op_res = solver.solve(op_res);

    /* Checks */
    if(solver.info() != Success) return FAIL_SIMULATOR_SOLVE;

    /* Create the transient matrix */
    this->_mna_engine.CreateMNASystemTRAN(tran_mat);

    return RETURN_SUCCESS;
}

/*!
    @brief      Performs a transient (TRAN) simulation using the Euler method.
    @return     Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::EulerODESolve(void)
{
    using Eigen::Success; // Eigen success value

    direct_solver solver;
    SparMatD tran_mat, op_mat;
    DensVecD cur;

    /* Perform the common transient pre-step */
    return_codes_e err_tmp = TRANpresolve(tran_mat, op_mat, cur);
    if(err_tmp != RETURN_SUCCESS) return err_tmp;

    /****** 2nd step compute the final transient array ******/
    auto &sim_vector = this->_mna_engine.getSimVals();
    double inverse_timestep = 1/this->_mna_engine.getSimStep();

    tran_mat = inverse_timestep * tran_mat;
    op_mat = op_mat + tran_mat; // A = G + 1/h * C

    /* Factorization/Symbolic analysis*/
    solver.compute(op_mat);

    /* Checks */
    if(solver.info() != Success) return FAIL_SIMULATOR_FACTORIZATION;

    /****** 3rd step Run for each simulation timepoint ******/

    /* Set initial t=0 */
    DensVecD old = cur;
    setPlotResults(old);

    /* Solve A*x = C*e(t) + x(t0)*/
    for(size_t i = 1; i < sim_vector.size(); i++) /* Skip t=0 timepoint (already computed from OP) */
    {
        /* Create the right hand side */
        cur = tran_mat * old;   // C/h*x(tk-1) + e(tk)
        this->_mna_engine.UpdateTRANVec(cur, sim_vector[i]);

        /* Save results */
        old = solver.solve(cur);
        setPlotResults(old);

        /* Checks */
        if(solver.info() != Success) return FAIL_SIMULATOR_SOLVE;
    }

    return RETURN_SUCCESS;
}

/*!
    @brief      Performs a transient (TRAN) simulation using the Trapezoidal method.
    @return     Error code in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e simulator_engine::TrapODESolve(void)
{
    using Eigen::Success; // Eigen success value

    direct_solver solver;
    SparMatD tran_mat, op_mat;
    DensVecD cur;

    /* Perform the common transient pre-step */
    return_codes_e err_tmp = TRANpresolve(tran_mat, op_mat, cur);
    if(err_tmp != RETURN_SUCCESS) return err_tmp;

    /****** 2nd step compute the final transient array ******/

    auto &sim_vector = this->_mna_engine.getSimVals();
    double inverse_timestep = 1/this->_mna_engine.getSimStep();

    /****** 2nd step compute the final transient array ******/
    SparMatD tmp = op_mat;
    tran_mat = 2*inverse_timestep * tran_mat;

    /* Left hand matrix => Gnew0 = 2*C/h + G */
    op_mat = op_mat + tran_mat;

    /* Right hand matrix => Gnew1 = 2*C/h - G */
    tran_mat = tran_mat - tmp;

    /* Factorization/Symbolic analysis*/
    solver.compute(op_mat);

    /* Checks */
    if(solver.info() != Success) return FAIL_SIMULATOR_FACTORIZATION;

    /****** 3rd step Run for each simulation timepoint ******/

    /* Set initial t=0 */
    DensVecD old = cur;
    setPlotResults(old);

    /* Solve A*x = C*e(t) + x(t0)*/
    for(size_t i = 1; i < sim_vector.size(); i++) /* Skip t=0 timepoint (already computed from OP) */
    {
        cur = tran_mat * old;
        this->_mna_engine.UpdateTRANVec(cur, sim_vector[i]);
        this->_mna_engine.UpdateTRANVec(cur, sim_vector[i - 1]);

        /* Checks */
        if(solver.info() != Success) return FAIL_SIMULATOR_SOLVE;

        old = solver.solve(cur);
        setPlotResults(old);
    }

    return RETURN_SUCCESS;
}

//TODO - Page 147 of SPECTRE design and stuff paper, does not work seems unstable
return_codes_e simulator_engine::Gear2ODESolve(void)
{
    using Eigen::Success; // Eigen success value

    direct_solver solver;
    SparMatD tran_mat, op_mat, tmp_op_mat;
    DensVecD cur, nxt, old;

    /* Perform the common transient pre-step */
    return_codes_e err_tmp = TRANpresolve(tran_mat, op_mat, cur);
    if(err_tmp != RETURN_SUCCESS) return err_tmp;

    /****** 2nd step compute the final transient array ******/

    /* Simulation needed parameters */
    auto &sim_vector = this->_mna_engine.getSimVals();
    double inverse_timestep = 1/this->_mna_engine.getSimStep();

    /* Perform 1 step of Euler to get the next needed timepoint and then start GEAR2 */
    tran_mat = inverse_timestep * tran_mat;
    tmp_op_mat = op_mat + tran_mat;

    /* Factorization/Symbolic analysis*/
    solver.compute(tmp_op_mat);

    /* Checks */
    if(solver.info() != Success) return FAIL_SIMULATOR_FACTORIZATION;

    old = cur;
    setPlotResults(old);

    /* Create the right hand side */
    cur = tran_mat * old;   // C/h*x(tk-1) + e(tk)
    this->_mna_engine.UpdateTRANVec(cur, sim_vector[1]);

    /* Save results */
    cur = solver.solve(cur);
    setPlotResults(cur);

    /* Checks */
    if(solver.info() != Success) return FAIL_SIMULATOR_SOLVE;

    /* Common steps - Set up matrices for every side */
    op_mat = op_mat + 3/2*tran_mat;
    tmp_op_mat = -1 * tran_mat;
    tran_mat = 2 * tran_mat;

    /* Factorization/Symbolic analysis*/
    solver.compute(op_mat);

    /* Checks */
    if(solver.info() != Success) return FAIL_SIMULATOR_FACTORIZATION;

    /****** 3rd step Run for each simulation timepoint ******/

    for(size_t i = 2; i < sim_vector.size(); i++) /* Skip t=0 timepoint (already computed from OP) */
    {
        nxt = tmp_op_mat * old + tran_mat * cur;
        this->_mna_engine.UpdateTRANVec(nxt, sim_vector[i]);

        /* Checks */
        if(solver.info() != Success) return FAIL_SIMULATOR_SOLVE;

        nxt = solver.solve(nxt);
        setPlotResults(nxt);

        /* Copy vectors for next iteration */
        old = cur;
        cur = nxt;
    }

    return RETURN_SUCCESS;
}

/*!
    @brief      Sets the results of the simulation for any non-AC analysis.
    @param      vec   Vector containing the results of the current simulation point
*/
void simulator_engine::setPlotResults(DensVecD &vec)
{
    /* Get the indices from the MNA engine */
    auto &nodes_idx = this->_mna_engine.getNodesIdx();
    auto &sources_idx = this->_mna_engine.getSourceIdx();
    std::vector<double> tmp_vec, tmp_vec2;

    /* Create the vectors - nodes/sources */
    for(auto it : nodes_idx) tmp_vec.push_back(vec[it]);
    for(auto it : sources_idx) tmp_vec2.push_back(vec[it]);

    /* Out */
    this->_res_nodes.push_back(tmp_vec);
    this->_res_sources.push_back(tmp_vec2);
}

/*!
    @brief      Sets the results of the simulation for the AC analysis.
    @param      vec   Vector containing the results of the current simulation point
*/
void simulator_engine::setPlotResultsCd(DensVecCompD &vec)
{
    /* Get the indices from the MNA engine */
    auto &nodes_idx = this->_mna_engine.getNodesIdx();
    auto &sources_idx = this->_mna_engine.getSourceIdx();
    std::vector<std::complex<double>> tmp_vec, tmp_vec2;

    /* Create the vectors - nodes/sources */
    for(auto it : nodes_idx) tmp_vec.push_back(vec[it]);
    for(auto it : sources_idx) tmp_vec2.push_back(vec[it]);

    /* Out */
    this->_res_nodes_cd.push_back(tmp_vec);
    this->_res_sources_cd.push_back(tmp_vec2);
}
