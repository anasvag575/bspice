#ifndef __SIM_ENGINE_H
#define __SIM_ENGINE_H

#include "mna.hpp"
#include "simulator_types.hpp"

//! A simulator class. The purpose of this class is to represent the simulation engine.
/*!
  This class has all the methods needed to perform simulation on the given SPICE circuit.
  Therefore, in this class, the following are provided:
  - Different solvers depending on configuration.
  - Simulation methods (TRAN, AC, DC, OP).
  - Different ODE methods for TRAN.
  - Access to the results of the simulation.
*/
class simulator
{
	public:

        /* Constructors */
        simulator(circuit &circuit_manager);

        /* Getters */
		bool valid(void) noexcept;
		const std::vector<double> &SimulationVec(void) noexcept;
		const std::vector<std::vector<double>> &NodesResults(void) noexcept;
		const std::vector<std::vector<double>> &SourceResults(void) noexcept;
		const std::vector<std::vector<std::complex<double>>> &NodesResultsCd(void) noexcept;
		const std::vector<std::vector<std::complex<double>>> &SourceResultsCd(void) noexcept;

		/* Methods */
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
        void setPlotResults(DensVecD &vec);
        void setPlotResultsCd(DensVecCompD &vec);

		/* Simulator sub-engines */
		MNA _mna_engine;                //!< The MNA engine, generates MNA matrices and vectors.

		/* Simulator state/parameters */
        bool _run;                      //!< Flag that indicates whether the results are valid or not.
		ODE_meth_t _ode_method;         //!< ODE method to be used for transient analysis.

		/* Vectors used to save the plot/save the results */
		std::vector<std::vector<double>> _res_nodes;                    //!< Results for nodes voltages, used in plotting.
        std::vector<std::vector<double>> _res_sources;                  //!< Results for sources current, used in plotting.
		std::vector<std::vector<std::complex<double>>> _res_nodes_cd;   //!< Results for sources current, used in plotting (AC only).
		std::vector<std::vector<std::complex<double>>> _res_sources_cd; //!< Results for sources current, used in plotting (AC only).
};

#endif // __SIM_ENGINE_H //
