#ifndef __MNA_H
#define __MNA_H

#include "circuit.hpp"
#include "matrix_types.hpp"
#include "math_util.hpp"



//! A simulator class. The purpose of this class is to construct MNA matrices and vectors.
class MNA
{
	public:
        /* Constructors */
		MNA();
		MNA(circuit &circuit_manager);

		/* Getters */
        analysis_t AnalysisType(void);
        as_scale_t AnalysisScale(void);
		IntTp SystemDim(void);
        double SimStep(void);
		const std::vector<double> &SimVals(void);
		const std::vector<IntTp> &NodesIdx(void);
		const std::vector<IntTp> &SourceIdx(void);

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
		void VcvsMNAStamp(tripletList_d &mat, IntTp offset, vcvs_packed &source);
		void VccsMNAStamp(tripletList_d &mat, vccs_packed &source);
		void CcvsMNAStamp(tripletList_d &mat, IntTp offset, ccvs_packed &source);
		void CccsMNAStamp(tripletList_d &mat, cccs_packed &source);

		/* AC MNA stampers */
		void ResMNAStamp(tripletList_cd &mat, resistor_packed &res);
		void CoilMNAStamp(tripletList_cd &mat, IntTp offset, coil_packed &coil, double freq);
		void CapMNAStamp(tripletList_cd &mat, capacitor_packed &cap, double freq);
		void IvsMNAStamp(tripletList_cd &mat, IntTp offset, ivs_packed &source);
		void IvsMNAStamp(DensVecCompD &rh, IntTp offset, ivs_packed &source);
		void IcsMNAStamp(DensVecCompD &rh, ics_packed &source);
        void VcvsMNAStamp(tripletList_cd &mat, IntTp offset, vcvs_packed &source);
        void VccsMNAStamp(tripletList_cd &mat, vccs_packed &source);
        void CcvsMNAStamp(tripletList_cd &mat, IntTp offset, ccvs_packed &source);
        void CccsMNAStamp(tripletList_cd &mat, cccs_packed &source);

		/* Transient sources evaluators */
		double EXPSourceEval(std::vector<double> &vvals, double time);
		double SINSourceEval(std::vector<double> &vvals, double time);
		double PULSESourceEval(std::vector<double> &vvals, double time);
		double PWLSourceEval(std::vector<double> &tvals, std::vector<double> &vvals, double time);

        /* Assisting methods for constructor */
        void CreatePlotIdx(circuit &circuit_manager);
        void CreatePackedVecs(circuit &circuit_manager);
        void SetMNAParams(circuit &circuit_manager);

		/* Debug functionalities */
		void debug_triplet_mat(tripletList_d &mat);

		/* Information about the system */
		IntTp _system_dim;                      //!< The MNA matrix dimension (dim x dim).
		IntTp _ivs_offset;                      //!< The offset of the IVS in the MNA array/vectors.
		IntTp _coil_offset;                     //!< The offset of the coils in the MNA array/vectors.
        IntTp _vcvs_offset;                     //!< The offset of the VCVS in the MNA array/vectors.
        IntTp _ccvs_offset;                     //!< The offset of the CCVS in the MNA array/vectors.

		/* Elements */
        std::vector<resistor_packed> _res;      //!< Packed representation of resistors in the circuit.
        std::vector<capacitor_packed> _caps;    //!< Packed representation of capacitors in the circuit.
        std::vector<coil_packed> _coils;        //!< Packed representation of coils in the circuit.
        std::vector<ics_packed> _ics;           //!< Packed representation of ICS in the circuit.
        std::vector<ivs_packed> _ivs;           //!< Packed representation of IVS in the circuit.
        std::vector<vcvs_packed> _vcvs;         //!< Packed representation of VCVS in the circuit.
        std::vector<vccs_packed> _vccs;         //!< Packed representation of VCCS in the circuit.
        std::vector<ccvs_packed> _ccvs;         //!< Packed representation of CCVS in the circuit.
        std::vector<cccs_packed> _cccs;         //!< Packed representation of CCCS in the circuit.

        /* Simulation vector */
        std::vector<double> _sim_vals;          //!< The simulation vector for the MNA matrix.
        double _sim_step;                       //!< The simulation step for the MNA matrix.

        /* Indexing vectors, for results */
        std::vector<IntTp> _sources_idx;        //!< The indices of the sources, for plotting.
        std::vector<IntTp> _nodes_idx;          //!< The indices of the nodes, for plotting.
        IntTp _sweep_source_idx;                //!< The index of the source, in case of DC analysis.

        /* Simulation info */
		analysis_t _analysis_type;              //!< The type of analysis performed.
		as_scale_t _scale;                      //!< Scale of the analysis.
};

#endif // __MNA_H //
