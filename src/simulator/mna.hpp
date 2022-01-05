#ifndef __MNA_H
#define __MNA_H

#include "circuit.hpp"
#include "matrix_types.hpp"
#include "math_util.hpp"

class MNA
{
	public:
		/*!
			@brief    Default constructor.
		*/
		MNA()
		{
			_sim_dim = 0;
			_system_dim = 0;
			_ivs_offset = 0;
			_coil_offset = 0;
			_sim_store_start = 0;
		}

		/*!
			@brief      Initializes the MNA engine with the parameters
			defined by the circuit input.
			@param		circuit_manager		The circuit.
		*/
		MNA(Circuit &circuit_manager)
		{
			/* Set up the problem dimensions and elements offsets in the MNA arrays/vectors */
			auto nodes_dim = circuit_manager.getNodes().size();
			this->_ivs_offset = nodes_dim;
			this->_coil_offset = nodes_dim + circuit_manager.getIVS().size();
			this->_system_dim = this->_coil_offset + circuit_manager.getCoils().size();
			this->_sim_store_start = 0; // TODO

			/* Create the simulation vector */
			auto sim_scale = circuit_manager.getAnalysisScale();
			auto sim_type = circuit_manager.getAnalysisType();
			double start = circuit_manager.getSimStart();
			double end = circuit_manager.getSimEnd();
			double steps = circuit_manager.getSimStep();

			/* Now create the simulation times/values vector */
			if(sim_scale == DEC_SCALE)
			{
				/* For AC we generate based on step (since argument is [points]) */
				if(sim_type == AC) linspaceVecGen(this->_sim_vals, start, end, static_cast<size_t>(steps));
				else StepVecGen(this->_sim_vals, start, end, steps);
			}
			else
			{
				/* Generate logarithmically spaced vector */
				logspaceVecGen(this->_sim_vals, start, end, static_cast<size_t>(steps));
			}

			this->_sim_dim = this->_sim_vals.size();
		}

		/*!
			@brief  Resets the state of the MNA engine.
		*/
		void clear(void)
		{
			this->_sim_dim = 0;
			this->_system_dim = 0;
			this->_ivs_offset = 0;
			this->_coil_offset = 0;
			this->_sim_store_start = 0;
			this->_sim_vals.clear();
		}

		/*!
			@brief      Returns the MNA system dimension.
			@return		The dimension.
		*/
		IntTp getSystemDim(void)
		{
			return this->_system_dim;
		}

		/*!
			@brief      Returns the starting offset of coils in the
			MNA matrix/rh vector.
			@return 	The offset.
		*/
		IntTp getCoilOffset(void)
		{
			return this->_coil_offset;
		}

		/*!
			@brief      Returns the starting offset of IVS(s) in the
			MNA matrix/rh vector.
			@return 	The offset.
		*/
		IntTp getIVSOffset(void)
		{
			return this->_ivs_offset;
		}

		/*!
			@brief      Returns the vectors containing the simulation
			values.
			@return 	The vector.
		*/
		std::vector<double> &getSimVals(void)
		{
			return this->_sim_vals;
		}

		/*!
			@brief      Returns the number of simulation points for the
			analysis.
			@return		The number.
		*/
		IntTp getSimDim(void)
		{
			return this->_sim_dim;
		}

		/* MNA and systems formation */
		void CreateMNASystemOP(Circuit &circuit_manager, SparMatD &mat, DensVecD &rh);
		void CreateMNASystemDC(Circuit &circuit_manager, SparMatD &mat, DenseMatD &rh);
		void CreateMNASystemTRAN(Circuit &circuit_manager, SparMatD &mat);
		void UpdateTRANVec(Circuit &circuit_manager, DensVecD &rh, double time);

		/* Formation of plot matrices */
		void CreatePLOTNodeVec(Circuit &circuit_manager, DensVecD &out, std::string node, DenseMatD &res);
	private:

		/* MNA stampers */
		void ResMNAStamp(tripletList_d &mat, Resistor &res);
		void CoilMNAStamp(tripletList_d &mat, IntTp offset, Coil &coil, const analysis_t type);
		void CapMNAStamp(tripletList_d &mat, Capacitor &cap, const analysis_t type);
		void IcsMNAStamp(DensVecD &rh, ics &source);
		void IvsMNAStamp(tripletList_d &mat, DensVecD &rh, IntTp offset, ivs &source);

		/* Transient sources evaluators */
		double EXPSourceEval(std::vector<double> &vvals, double time);
		double SINSourceEval(std::vector<double> &vvals, double time);
		double PULSESourceEval(std::vector<double> &vvals, double time);
		double PWLSourceEval(std::vector<double> &tvals, std::vector<double> &vvals, double time);

		/* Debug functionalities */
		void debug_triplet_mat(tripletList_d &mat);

		/* Information about the system */
		IntTp _sim_dim;
		IntTp _system_dim;
		IntTp _ivs_offset;
		IntTp _coil_offset;

		/* Simulation info */
		std::vector<double> _sim_vals;
		IntTp _sim_store_start;
};

#endif // __MNA_H //
