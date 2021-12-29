#ifndef __MNA_H
#define __MNA_H

#include "simulator_types.hpp"
#include "circuit.hpp"

class MNA
{
	public:
		void CreateMNASystemOP(Circuit &circuit_manager, SparMatD &mat, DensVecD &rh);
		void CreateMNASystemDC(Circuit &circuit_manager, SparMatD &mat, DenseMatD &rh);
//		void CreateMnaDC(Circuit &circuit_manager);
//		void CreateMnaAC(Circuit &circuit_manager);
//		void CreateMnaTRAN();

	private:
//		void EvalTranSources(std::vector<>);
//		void EvalDCSource();
};

#endif // __MNA_H //
