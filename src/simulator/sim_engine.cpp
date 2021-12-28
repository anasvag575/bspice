#include "sim_engine.hpp"

/* Run analysis OP/DC/TRAN */
void simulator_engine::run(Circuit &circuit_manager)
{
	/* Set up MNA engine */
	MNA mna_engine;

	/* TODO - Set up a simulation core */


	/* Depending on analysis, call the appropriate sub-simulator */
	switch(this->_analysis)
	{
		case OP: OP_analysis(circuit_manager, mna_engine); break;
		case DC: DC_analysis(circuit_manager, mna_engine); break;
		case TRAN: TRAN_analysis(circuit_manager, mna_engine); break;
		case AC: AC_analysis(circuit_manager, mna_engine); break;
		default: OP_analysis(circuit_manager, mna_engine); break;
	}
}

/* Simplest method */
void simulator_engine::OP_analysis(Circuit &circuit_manager, MNA &mna_engine)
{
	SparMatD mat;
	DensVecD rh;

	/* The matrix has to be created once */
	mna_engine.CreateMNASystemOP(circuit_manager, mat, rh);

	/* Perform solve step and return */
}

/* TODO - Implement */
void simulator_engine::DC_analysis(Circuit &circuit_manager, MNA &mna_engine)
{

}

/* TODO - Implement */
void simulator_engine::TRAN_analysis(Circuit &circuit_manager, MNA &mna_engine)
{

}

/* TODO - Implement */
void simulator_engine::AC_analysis(Circuit &circuit_manager, MNA &mna_engine)
{

}
