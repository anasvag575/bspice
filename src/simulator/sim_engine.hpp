#ifndef __SIM_ENGINE_H
#define __SIM_ENGINE_H

#include <vector>
#include "simulator_types.hpp"
#include "circuit.hpp"
#include "mna.hpp"

class simulator_engine
{
	public:

		simulator_engine()
		{
			_source_dim = 0;
			_total_dim = 0;
			_total_sim_steps = 0;
			_analysis = OP;
			_scale = DEC_SCALE;
			_valid = false;
		}

		void init(Circuit &circuit_manager)
		{
			this->_analysis = circuit_manager.getAnalysisType();
			this->_scale = circuit_manager.getAnalysisScale();
			this->_source_dim = circuit_manager.getCoils().size() +
								circuit_manager.getIVS().size();
			this->_total_dim = this->_source_dim +
							   circuit_manager.getNodes().size();

			this->_total_sim_steps = circuit_manager.getSimValues().size();
		}

		void reset(void)
		{
			_source_dim = 0;
			_total_dim = 0;
			_total_sim_steps = 0;
			_analysis = OP;
			_scale = DEC_SCALE;
			_valid = false;
			_results_d = DenseMatD::Zero(0,0);
			_results_cd = DenseMatCompD::Zero(0,0);
		}

		void run(Circuit &circuit_manager);
	private:
		void OP_analysis(Circuit &circuit_manager, MNA &mna_engine);
		void DC_analysis(Circuit &circuit_manager, MNA &mna_engine);
		void TRAN_analysis(Circuit &circuit_manager, MNA &mna_engine);
		void AC_analysis(Circuit &circuit_manager, MNA &mna_engine);

		size_t _source_dim;
		size_t _total_dim;
		size_t _total_sim_steps;
		analysis_t _analysis;
		as_scale_t _scale;
		bool _valid;

		/* Arrays containing the complete results */
		DenseMatD	_results_d;
		DenseMatCompD	_results_cd;
};

#endif // __SIM_ENGINE_H //
