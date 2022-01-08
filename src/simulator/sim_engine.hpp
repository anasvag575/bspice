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
		}

		/*!
			@brief  Resets the state of the simulator engine.
		*/
		void clear(void)
		{
			_run = false;
			this->_mna_engine.clear();
			_results_d = DenseMatD::Zero(0,0);
			_results_cd = DenseMatCompD::Zero(0,0);
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
			@brief  Returns the simulation results for OP/DC/TRAN analysis
			@return The simulation results
		*/
		DenseMatD &getResultsD(void)
		{
			return this->_results_d;
		}

		/*!
			@brief  Returns the simulation results for AC analysis
			@return The simulation results
		*/
		DenseMatCompD &getResultsCD(void)
		{
			return this->_results_cd;
		}

		/*!
			@brief  Returns the simulation vector used for the analysis
			@return The simulation vector (sweep-val)
		*/
		std::vector<double> &getSimulationVec(void)
		{
			return this->_mna_engine.getSimVals();
		}

		return_codes_e run(Circuit &circuit_manager);
		void getPlotResults(Circuit &circuit_manager, std::vector<std::vector<double>> &res_nodes,
		                    std::vector<std::vector<double>> &res_sources);
		void getPlotResults(Circuit &circuit_manager, std::vector<std::vector<std::complex<double>>> &res_nodes,
                            std::vector<std::vector<std::complex<double>>> &res_sources);
	private:
		/* Analysis supported */
		return_codes_e OP_analysis(Circuit &circuit_manager);
		return_codes_e DC_analysis(Circuit &circuit_manager);
		return_codes_e TRAN_analysis(Circuit &circuit_manager);
		return_codes_e AC_analysis(Circuit &circuit_manager);

		/* Simulator sub-engines */
		MNA _mna_engine;

		/* Active results */
		bool _run;

		/* Arrays containing the complete results */
		DenseMatD	_results_d;
		DenseMatCompD	_results_cd;
};

#endif // __SIM_ENGINE_H //
