#ifndef __SIM_ENGINE_H
#define __SIM_ENGINE_H

#include "mna.hpp"
#include "simulator_types.hpp"

class simulator_engine
{
	public:

		/*!
			@brief    Default constructor.
		*/
		simulator_engine()
		{
			_run = false;
			_save_mem = false;
			_ode_method = BACKWARDS_EULER;
		}

		/*!
			@brief      Initializes the simulator engine with the parameters
			defined by the circuit input.
			@param		circuit_manager		The circuit.
		*/
		simulator_engine(Circuit &circuit_manager)
		{
			this->_mna_engine = MNA(circuit_manager);
			_run = false;
			_save_mem = circuit_manager.getMemMode();
            _ode_method = circuit_manager.getODEMethod();
		}

		/*!
			@brief  Resets the state of the simulator engine.
		*/
		void clear(void)
		{
			_run = false;
            _save_mem = false;
            _ode_method = BACKWARDS_EULER;
			this->_mna_engine.clear();
			_results_d = DenseMatD::Zero(0,0);
			_results_cd = DenseMatCompD::Zero(0,0);
			for(auto &it : this->_res_nodes) it.clear();
			for(auto &it : this->_res_nodes_cd) it.clear();
			for(auto &it : this->_res_sources) it.clear();
			for(auto &it : this->_res_sources_cd) it.clear();
		}

		/*!
			@brief  Returns whether the simulator is in a valid state (run valid).
			@return True, in case of active results, otherwise false.
		*/
		bool valid(void)
		{
			return this->_run;
		}

		/*!
			@brief  Returns the simulation vector used for the analysis
			@return The simulation vector (sweep-val)
		*/
		std::vector<double> &getSimulationVec(void)
		{
			return this->_mna_engine.getSimVals();
		}

        /*!
            @brief  Returns the results for the plot nodes
            @return The results
        */
		std::vector<std::vector<double>> &getNodesResults(void)
        {
            return this->_res_nodes;
        }

        /*!
            @brief  Returns the results for the plot source
            @return The results
        */
        std::vector<std::vector<double>> &getSourceResults(void)
        {
            return this->_res_sources;
        }

        /*!
            @brief  Returns the results for the plot nodes (AC analysis)
            @return The results
        */
        std::vector<std::vector<std::complex<double>>> &getNodesResultsCd(void)
        {
            return this->_res_nodes_cd;
        }

        /*!
            @brief  Returns the results for the plot source (AC analysis)
            @return The results
        */
        std::vector<std::vector<std::complex<double>>> &getSourceResultsCd(void)
        {
            return this->_res_sources_cd;
        }

		return_codes_e run(void);
	private:
		/* Analysis supported */
		return_codes_e OP_analysis(void);
		return_codes_e DC_analysis(void);
		return_codes_e TRAN_analysis(void);
		return_codes_e AC_analysis(void);

		/* Integration solvers */
		return_codes_e TRANpresolve(SparMatD &tran_mat, SparMatD &op_mat, DensVecD &op_res);
		return_codes_e EulerODESolve(void);
        return_codes_e TrapODESolve(void);
        return_codes_e Gear2ODESolve(void); // TODO

        /* Handling of results */
        void setPlotResults(void);
        void setPlotResultsCd(void);
        void setPlotResults(DensVecD &vec);
        void setPlotResultsCd(DensVecCompD &vec);

		/* Simulator sub-engines */
		MNA _mna_engine;

		/* Simulator state/parameters */
        bool _run;
		bool _save_mem;
		ODE_meth_t _ode_method;

		/* Vectors used to save the plot/save the results */
		std::vector<std::vector<double>> _res_nodes;
        std::vector<std::vector<double>> _res_sources;
		std::vector<std::vector<std::complex<double>>> _res_nodes_cd;
		std::vector<std::vector<std::complex<double>>> _res_sources_cd;

        /* Arrays containing the complete results - Optional use */
        DenseMatD   _results_d;
        DenseMatCompD   _results_cd;
};

#endif // __SIM_ENGINE_H //
