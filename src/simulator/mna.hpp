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
			_system_dim = 0;
			_ivs_offset = 0;
			_coil_offset = 0;
			_sweep_source_idx = 0;
			_sim_store_start = 0;
			_sim_step = 0;
			_analysis_type = OP;
			_scale = DEC_SCALE;
		}

		/*!
			@brief  Resets the state of the MNA engine.
		*/
		void clear(void)
		{
            _system_dim = 0;
            _ivs_offset = 0;
            _coil_offset = 0;
            _sweep_source_idx = 0;
            _sim_store_start = 0;
            _sim_step = 0;
            _analysis_type = OP;
            _scale = DEC_SCALE;

            /* Clear vectors */
            this->_res.clear();
            this->_caps.clear();
            this->_coils.clear();
            this->_ics.clear();
            this->_ivs.clear();
			this->_sim_vals.clear();
			this->_nodes_idx.clear();
            this->_sources_idx.clear();
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
			return this->_sim_vals.size();
		}

        /*!
            @brief      Returns the simulation step.
            @return     The simulation step.
        */
        double getSimStep(void)
        {
            return _sim_step;
        }

        /*!
            @brief      Returns the type of analysis performed.
            @return     The analysis type.
        */
		analysis_t getAnalysisType(void)
		{
		    return this->_analysis_type;
		}

        /*!
            @brief      Returns the type of scale used.
            @return     The scale.
        */
		as_scale_t getAnalysisScale(void)
		{
		    return this->_scale;
		}

        /*!
            @brief      Returns the nodes indices vector, according to plot order.
            @return     The indices vector.
        */
		std::vector<IntTp> &getNodesIdx(void)
		{
		    return this->_nodes_idx;
		}

        /*!
            @brief      Returns the sources indices vector, according to plot order.
            @return     The indices vector.
        */
        std::vector<IntTp> &getSourceIdx(void)
        {
            return this->_sources_idx;
        }

        /* Constructors */
        MNA(Circuit &circuit_manager);

        /* MNA and systems formation */
        void CreateMNASystemOP(SparMatD &mat, DensVecD &rh);
		void CreateMNASystemDC(SparMatD &mat, DenseMatD &rh);
		void UpdateMNASystemDCVec(DensVecD &rh, double sweep_val);
		void CreateMNASystemTRAN(SparMatD &mat);
		void UpdateTRANVec(DensVecD &rh, double time);
        void CreateMNASystemAC(SparMatCompD &mat, double freq);
        void CreateMNASystemAC(DensVecCompD &rh);
	private:

		/* MNA stampers */
		void ResMNAStamp(tripletList_d &mat, resistor_packed &res);
		void CoilMNAStamp(tripletList_d &mat, IntTp offset, coil_packed &coil, const analysis_t type);
		void CapMNAStamp(tripletList_d &mat, capacitor_packed &cap, const analysis_t type);
		void IcsMNAStamp(DensVecD &rh, ics_packed &source);
		void IvsMNAStamp(tripletList_d &mat, DensVecD &rh, IntTp offset, ivs_packed &source);

		/* AC MNA stampers */
		void ResMNAStamp(tripletList_cd &mat, resistor_packed &res);
		void CoilMNAStamp(tripletList_cd &mat, IntTp offset, coil_packed &coil, double freq);
		void CapMNAStamp(tripletList_cd &mat, capacitor_packed &cap, double freq);
		void IvsMNAStamp(tripletList_cd &mat, IntTp offset, ivs_packed &source);
		void IvsMNAStamp(DensVecCompD &rh, IntTp offset, ivs_packed &source);
		void IcsMNAStamp(DensVecCompD &rh, ics_packed &source);

		/* Transient sources evaluators */
		double EXPSourceEval(std::vector<double> &vvals, double time);
		double SINSourceEval(std::vector<double> &vvals, double time);
		double PULSESourceEval(std::vector<double> &vvals, double time);
		double PWLSourceEval(std::vector<double> &tvals, std::vector<double> &vvals, double time);

		/* Debug functionalities */
		void debug_triplet_mat(tripletList_d &mat);

		/* Information about the system */
		IntTp _system_dim;
		IntTp _ivs_offset;
		IntTp _coil_offset;

		/* Elements */
        std::vector<resistor_packed> _res;
        std::vector<capacitor_packed> _caps;
        std::vector<coil_packed> _coils;
        std::vector<ics_packed> _ics;
        std::vector<ivs_packed> _ivs;

        /* Simulation vector */
        std::vector<double> _sim_vals;
        double _sim_step;

        /* Indexing vectors, for results */
        std::vector<IntTp> _sources_idx;
        std::vector<IntTp> _nodes_idx;
        IntTp _sweep_source_idx;

        /* Simulation info */
		analysis_t _analysis_type;
		as_scale_t _scale;
		IntTp _sim_store_start; //TODO
};

#endif // __MNA_H //
