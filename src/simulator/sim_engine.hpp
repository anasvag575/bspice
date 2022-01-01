#ifndef __SIM_ENGINE_H
#define __SIM_ENGINE_H

#include "mna.hpp"

class simulator_engine
{
	public:

		/*!
			@brief    Default constructor.
		*/
		simulator_engine()
		{
			_valid = false;
		}

		/*!
			@brief      Initializes the simulator engine with the parameters
			defined by the circuit input.
			@param		circuit_manager		The circuit.
		*/
		simulator_engine(Circuit &circuit_manager)
		{
			this->_mna_engine = MNA(circuit_manager);
			_valid = true;
		}

		/*!
			@brief  Resets the state of the simulator engine.
		*/
		void clear(void)
		{
			_valid = false;
			_results_d = DenseMatD::Zero(0,0);
			_results_cd = DenseMatCompD::Zero(0,0);
		}

		void run(Circuit &circuit_manager);
	private:
		void OP_analysis(Circuit &circuit_manager);
		void DC_analysis(Circuit &circuit_manager);
		void TRAN_analysis(Circuit &circuit_manager);
		void AC_analysis(Circuit &circuit_manager);

		/* Linear solvers -> {Ax = b} - {Ax = B} problems */
		void solve_linear(SparMatD &mat, DensVecD &rh, DensVecD &res);
		void solve_linear(SparMatD &mat, DenseMatD &rh, DenseMatD &res);
		void solve_linear(SparMatCompD &mat, DensVecCompD &rh, DensVecCompD &res);

		/* ODE (1st order) solvers -> {Ax + B*dx/dt = c} - {Ax + B*dx/dt = C} problems */

		/* Simulator sub-engines */
		MNA _mna_engine;

		/* Base parameters of the circuit */
		bool _valid;

		/* Arrays containing the complete results */
		DenseMatD	_results_d;
		DenseMatCompD	_results_cd;
};

#endif // __SIM_ENGINE_H //
