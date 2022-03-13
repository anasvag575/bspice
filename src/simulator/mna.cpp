#include <math.h>	/* Legacy C for PI constant */
#include "mna.hpp"

/*!
    @brief    Default constructor.
*/
MNA::MNA() noexcept
{
    _system_dim = 0;
    _ivs_offset = 0;
    _coil_offset = 0;
    _vcvs_offset = 0;
    _ccvs_offset = 0;
    _sweep_source_idx = 0;
    _sim_step = 0;
    _analysis_type = OP;
    _scale = DEC_SCALE;
}

/*!
    @brief      Initializes the MNA engine with the parameters
    defined by the circuit input.
    @param      circuit_manager     The circuit.
*/
MNA::MNA(circuit &circuit_manager)
{
    /* Create the packed representation */
    CreatePackedVecs(circuit_manager);

    /* Set the parameters of the class */
    SetMNAParams(circuit_manager);

    /* Create the indices for plotting */
    CreatePlotIdx(circuit_manager);
}



/*!
    @brief      Returns the type of analysis performed.
    @return     The analysis type.
*/
analysis_t MNA::AnalysisType(void) noexcept { return _analysis_type; }

/*!
    @brief      Returns the type of scale used.
    @return     The scale.
*/
as_scale_t MNA::AnalysisScale(void) noexcept { return _scale; }

/*!
    @brief      Returns the MNA system dimension.
    @return     The dimension.
*/
IntTp MNA::SystemDim(void) noexcept { return _system_dim; }

/*!
    @brief      Returns the simulation step.
    @return     The simulation step.
*/
double MNA::SimStep(void) noexcept { return _sim_step; }

/*!
    @brief      Returns the vectors containing the simulation
    values.
    @return     The vector.
*/
const std::vector<double> &MNA::SimVals(void) noexcept { return _sim_vals; }

/*!
    @brief      Returns the nodes indices vector, according to plot order.
    @return     The indices vector.
*/
const std::vector<IntTp> &MNA::NodesIdx(void) noexcept { return _nodes_idx; }

/*!
    @brief      Returns the sources indices vector, according to plot order.
    @return     The indices vector.
*/
const std::vector<IntTp> &MNA::SourceIdx(void) noexcept { return _sources_idx; }



/*!
	@brief      Inserts the MNA stamp of the resistor in triplet form
	(i, j, val) inside the mat array.
	@param      mat    The triplet matrix to insert the stamp.
	@param 		res	   The resistor.
*/
void MNA::ResMNAStamp(tripletList_d &mat, resistor_packed &res)
{
	auto conduct = 1/res.Val();
	auto pos = res.PosNodeID(), neg = res.NegNodeID();

	if(pos != -1) mat.push_back(triplet_eig_d(pos, pos, conduct));
	if(neg != -1) mat.push_back(triplet_eig_d(neg, neg, conduct));

	if((pos != -1) && (neg != -1))
	{
	    mat.push_back(triplet_eig_d(neg, pos, -conduct));
		mat.push_back(triplet_eig_d(pos, neg, -conduct));
	}
}

/*!
	@brief      Inserts the MNA stamp of the coil in triplet form
	(i, j, val) inside the mat array.
	@param      mat    The triplet matrix to insert the stamp.
	@param		offset The offset of the stamp inside the array.
	@param		coil   The coil.
	@param      type   The type of analysis performed.

*/
void MNA::CoilMNAStamp(tripletList_d &mat, IntTp offset, coil_packed &coil, const analysis_t type)
{
    auto pos = coil.PosNodeID(), neg = coil.NegNodeID();

	if(type == OP)
	{
		if(pos != -1)
		{
			mat.push_back(triplet_eig_d(offset, pos, 1));
			mat.push_back(triplet_eig_d(pos, offset, 1));
		}
		if(neg != -1)
		{
			mat.push_back(triplet_eig_d(offset, neg, -1));
			mat.push_back(triplet_eig_d(neg, offset, -1));
		}
	}
	else if(type == TRAN)
	{
		mat.push_back(triplet_eig_d(offset, offset, -coil.Val()));
	}
}

/*!
    @brief      Inserts the MNA stamp of the capacitor in triplet form
    (i, j, val) inside the mat array.
    @param      mat    The triplet matrix to insert the stamp.
    @param		cap	   The capacitor.
    @param      type   The type of analysis performed.
*/
void MNA::CapMNAStamp(tripletList_d &mat, capacitor_packed &cap, const analysis_t type)
{
	if(type == TRAN)
	{
	    auto pos = cap.PosNodeID(), neg = cap.NegNodeID();
		auto capacitance = cap.Val();

		if(pos != -1) mat.push_back(triplet_eig_d(pos, pos, capacitance));
		if(neg != -1) mat.push_back(triplet_eig_d(neg, neg, capacitance));

		if((pos != -1) && (neg != -1))
		{
			mat.push_back(triplet_eig_d(pos, neg, -capacitance));
			mat.push_back(triplet_eig_d(neg, pos, -capacitance));
		}
	}
}

/*!
    @brief      Inserts the MNA stamp of the ICS in triplet form
    (i, j, val) inside the mat array.
	@param      rh     The right hand side vector of the system.
    @param		source The ICS.
*/
void MNA::IcsMNAStamp(DensVecD &rh, ics_packed &source)
{
    auto pos = source.PosNodeID(), neg = source.NegNodeID();
	auto val = source.Val();

	if(pos != -1) rh[pos] -= val;
	if(neg != -1) rh[neg] += val;
}

/*!
    @brief      Inserts the MNA stamp of the IVS in triplet form
    (i, j, val) inside the mat array.
    @param      mat    The triplet matrix to insert the stamp.
    @param      rh     The right hand side vector of the system.
    @param		offset The offset of the stamp inside the array.
    @param		source The IVS.
*/
void MNA::IvsMNAStamp(tripletList_d &mat, DensVecD &rh, IntTp offset, ivs_packed &source)
{
    auto pos = source.PosNodeID(), neg = source.NegNodeID();

	if(pos != -1)
	{
		mat.push_back(triplet_eig_d(offset, pos, 1));
		mat.push_back(triplet_eig_d(pos, offset, 1));
	}

	if(neg != -1)
	{
		mat.push_back(triplet_eig_d(offset, neg, -1));
		mat.push_back(triplet_eig_d(neg, offset, -1));
	}

	rh[offset] += source.Val();
}

/*!
    @brief      Inserts the MNA stamp of the VCVS in triplet form
    (i, j, val) inside the mat array.
    @param      mat    The triplet matrix to insert the stamp.
    @param      offset The offset of the stamp inside the array.
    @param      source The VCVS.
*/
void MNA::VcvsMNAStamp(tripletList_d &mat, IntTp offset, vcvs_packed &source)
{
    auto pos = source.PosNodeID(), neg = source.NegNodeID();
    auto dep_pos = source.DepPosNodeID(), dep_neg = source.DepNegNodeID();
    auto val = source.Val();

    if(pos != -1)
    {
        mat.push_back(triplet_eig_d(offset, pos, 1));
        mat.push_back(triplet_eig_d(pos, offset, 1));
    }

    if(neg != -1)
    {
        mat.push_back(triplet_eig_d(offset, neg, -1));
        mat.push_back(triplet_eig_d(neg, offset, -1));
    }

    if(dep_pos != -1)
    {
        mat.push_back(triplet_eig_d(offset, dep_pos, -val));
    }

    if(dep_neg != -1)
    {
        mat.push_back(triplet_eig_d(offset, dep_neg, val));
    }
}

/*!
    @brief      Inserts the MNA stamp of the VCCS in triplet form
    (i, j, val) inside the mat array.
    @param      mat    The triplet matrix to insert the stamp.
    @param      source The VCCS.
*/
void MNA::VccsMNAStamp(tripletList_d &mat, vccs_packed &source)
{
    auto pos = source.PosNodeID(), neg = source.NegNodeID();
    auto dep_pos = source.DepPosNodeID(), dep_neg = source.DepNegNodeID();
    auto val = source.Val();

    if(pos != -1)
    {
        if(dep_pos != -1)
        {
            mat.push_back(triplet_eig_d(pos, dep_pos, val));
        }

        if(dep_neg != -1)
        {
            mat.push_back(triplet_eig_d(pos, dep_neg, -val));
        }
    }

    if(neg != -1)
    {
        if(dep_pos != -1)
        {
            mat.push_back(triplet_eig_d(neg, dep_pos, -val));
        }

        if(dep_neg != -1)
        {
            mat.push_back(triplet_eig_d(neg, dep_neg, val));
        }
    }
}

/*!
    @brief      Inserts the MNA stamp of the CCVS in triplet form
    (i, j, val) inside the mat array.
    @param      mat    The triplet matrix to insert the stamp.
    @param      offset The offset of the stamp inside the array
    @param      source The CCVS.
*/
void MNA::CcvsMNAStamp(tripletList_d &mat, IntTp offset, ccvs_packed &source)
{
    auto pos = source.PosNodeID(), neg = source.NegNodeID();
    auto source_idx = source.SourceID() + this->_ivs_offset;
    auto val = source.Val();

    if(pos != -1)
    {
        mat.push_back(triplet_eig_d(offset, pos, 1));
        mat.push_back(triplet_eig_d(pos, offset, 1));
    }

    if(neg != -1)
    {
        mat.push_back(triplet_eig_d(offset, neg, -1));
        mat.push_back(triplet_eig_d(neg, offset, -1));
    }

    mat.push_back(triplet_eig_d(offset, source_idx, -val));
}

/*!
    @brief      Inserts the MNA stamp of the CCCS in triplet form
    (i, j, val) inside the mat array.
    @param      mat    The triplet matrix to insert the stamp.
    @param      source The CCCS.
*/
void MNA::CccsMNAStamp(tripletList_d &mat, cccs_packed &source)
{
    auto pos = source.PosNodeID(), neg = source.NegNodeID();
    auto source_idx = source.SourceID() + this->_ivs_offset;
    auto val = source.Val();

    if(pos != -1)
    {
        mat.push_back(triplet_eig_d(pos, source_idx, val));
    }

    if(neg != -1)
    {
        mat.push_back(triplet_eig_d(neg, source_idx, -val));
    }
}



/*!
    @brief      Inserts the MNA stamp of the resistor in triplet form
    (i, j, val) inside the mat array. For AC analysis only.
    @param      mat    The triplet matrix to insert the stamp.
    @param      res    The resistor.
*/
void MNA::ResMNAStamp(tripletList_cd &mat, resistor_packed &res)
{
    auto conduct = 1/res.Val();
    auto pos = res.PosNodeID(), neg = res.NegNodeID();
    std::complex<double> tmp(conduct, 0);

    if(pos != -1) mat.push_back(triplet_eig_cd(pos, pos, tmp));
    if(neg != -1) mat.push_back(triplet_eig_cd(neg, neg, tmp));

    if((pos != -1) && (neg != -1))
    {
        mat.push_back(triplet_eig_cd(neg, pos, -tmp));
        mat.push_back(triplet_eig_cd(pos, neg, -tmp));
    }
}

/*!
    @brief      Inserts the MNA stamp of the coil in triplet form
    (i, j, val) inside the mat array. Only for AC analysis.
    @param      mat    The triplet matrix to insert the stamp.
    @param      offset The offset of the stamp inside the array.
    @param      coil   The coil.
    @param      freq   The frequency we generate the matrix at.
*/
void MNA::CoilMNAStamp(tripletList_cd &mat, IntTp offset, coil_packed &coil, double freq)
{
    auto pos = coil.PosNodeID(), neg = coil.NegNodeID();
    auto coil_imag = 2 * M_PI *freq *coil.Val();
    std::complex<double> real_tmp(1, 0);
    std::complex<double> imag(0, coil_imag);

    if(pos != -1)
    {
        mat.push_back(triplet_eig_cd(offset, pos, real_tmp));
        mat.push_back(triplet_eig_cd(pos, offset, real_tmp));
    }
    if(neg != -1)
    {
        mat.push_back(triplet_eig_cd(offset, neg, -real_tmp));
        mat.push_back(triplet_eig_cd(neg, offset, -real_tmp));
    }

    mat.push_back(triplet_eig_cd(offset, offset, -imag));
}

/*!
    @brief      Inserts the MNA stamp of the capacitor in triplet form
    (i, j, val) inside the mat array. For AC analysis only.
    @param      mat    The triplet matrix to insert the stamp.
    @param      cap    The capacitor.
    @param      freq   The frequency we generate the matrix at.
*/
void MNA::CapMNAStamp(tripletList_cd &mat, capacitor_packed &cap, double freq)
{
    auto pos = cap.PosNodeID(), neg = cap.NegNodeID();
    auto cap_imag = 2 * M_PI *freq *cap.Val();
    std::complex<double> tmp(0, cap_imag);

    if(pos != -1) mat.push_back(triplet_eig_cd(pos, pos, tmp));
    if(neg != -1) mat.push_back(triplet_eig_cd(neg, neg, tmp));

    if((pos != -1) && (neg != -1))
    {
        mat.push_back(triplet_eig_cd(pos, neg, -tmp));
        mat.push_back(triplet_eig_cd(neg, pos, -tmp));
    }
}

/*!
    @brief      Inserts the MNA stamp of the ICS in triplet form
    (i, j, val) inside the right hand side vector. For AC analysis only.
    @param      rh     The right hand side vector of the system.
    @param      source The ICS.
*/
void MNA::IcsMNAStamp(DensVecCompD &rh, ics_packed &source)
{
    auto pos = source.PosNodeID(), neg = source.NegNodeID();
    auto val = source.ACVal();

    if(pos != -1) rh[pos] -= val;
    if(neg != -1) rh[neg] += val;
}

/*!
    @brief      Inserts the MNA stamp of the IVS in triplet form
    (i, j, val) inside the mat array. For AC analysis only.
    @param      mat    The triplet matrix to insert the stamp.
    @param      offset The offset of the stamp inside the array.
    @param      source The IVS.
*/
void MNA::IvsMNAStamp(tripletList_cd &mat, IntTp offset, ivs_packed &source)
{
    auto pos = source.PosNodeID(), neg = source.NegNodeID();
    std::complex<double> real_tmp(1, 0);

    if(pos != -1)
    {
        mat.push_back(triplet_eig_cd(offset, pos, real_tmp));
        mat.push_back(triplet_eig_cd(pos, offset, real_tmp));
    }

    if(neg != -1)
    {
        mat.push_back(triplet_eig_cd(offset, neg, -real_tmp));
        mat.push_back(triplet_eig_cd(neg, offset, -real_tmp));
    }
}

/*!
    @brief      Inserts the MNA stamp of the IVS in triplet form
    (i, j, val) inside the right hand side vector. For AC analysis only.
    @param      rh     The right hand side vector of the system.
    @param      offset The offset of the stamp inside the array.
    @param      source The IVS.
*/
void MNA::IvsMNAStamp(DensVecCompD &rh, IntTp offset, ivs_packed &source)
{
    rh[offset] += source.ACVal();
}

/*!
    @brief      Inserts the MNA stamp of the VCVS in triplet form
    (i, j, val) inside the mat array. For AC analysis only.
    @param      mat    The triplet matrix to insert the stamp.
    @param      offset The offset of the stamp inside the array.
    @param      source The VCVS.
*/
void MNA::VcvsMNAStamp(tripletList_cd &mat, IntTp offset, vcvs_packed &source)
{
    auto pos = source.PosNodeID(), neg = source.NegNodeID();
    auto dep_pos = source.DepPosNodeID(), dep_neg = source.DepNegNodeID();
    std::complex<double> val(source.Val(), 0);
    std::complex<double> real_tmp(1, 0);

    if(pos != -1)
    {
        mat.push_back(triplet_eig_cd(offset, pos, real_tmp));
        mat.push_back(triplet_eig_cd(pos, offset, real_tmp));
    }

    if(neg != -1)
    {
        mat.push_back(triplet_eig_cd(offset, neg, -real_tmp));
        mat.push_back(triplet_eig_cd(neg, offset, -real_tmp));
    }

    if(dep_pos != -1)
    {
        mat.push_back(triplet_eig_cd(offset, dep_pos, -val));
    }

    if(dep_neg != -1)
    {
        mat.push_back(triplet_eig_cd(offset, dep_neg, val));
    }
}

/*!
    @brief      Inserts the MNA stamp of the VCCS in triplet form
    (i, j, val) inside the mat array. For AC analysis only.
    @param      mat    The triplet matrix to insert the stamp.
    @param      source The VCCS.
*/
void MNA::VccsMNAStamp(tripletList_cd &mat, vccs_packed &source)
{
    auto pos = source.PosNodeID(), neg = source.NegNodeID();
    auto dep_pos = source.DepPosNodeID(), dep_neg = source.DepNegNodeID();
    std::complex<double> val(source.Val(), 0);

    if(pos != -1)
    {
        if(dep_pos != -1)
        {
            mat.push_back(triplet_eig_cd(pos, dep_pos, val));
        }

        if(dep_neg != -1)
        {
            mat.push_back(triplet_eig_cd(pos, dep_neg, -val));
        }
    }

    if(neg != -1)
    {
        if(dep_pos != -1)
        {
            mat.push_back(triplet_eig_cd(neg, dep_pos, -val));
        }

        if(dep_neg != -1)
        {
            mat.push_back(triplet_eig_cd(neg, dep_neg, val));
        }
    }
}

/*!
    @brief      Inserts the MNA stamp of the CCVS in triplet form
    (i, j, val) inside the mat array. For AC analysis only.
    @param      mat    The triplet matrix to insert the stamp.
    @param      offset The offset of the stamp inside the array.
    @param      source The CCVS.
*/
void MNA::CcvsMNAStamp(tripletList_cd &mat, IntTp offset, ccvs_packed &source)
{
    auto pos = source.PosNodeID(), neg = source.NegNodeID();
    auto source_idx = source.SourceID() + this->_ivs_offset;
    std::complex<double> val(source.Val(), 0);
    std::complex<double> real_tmp(1, 0);

    if(pos != -1)
    {
        mat.push_back(triplet_eig_cd(offset, pos, real_tmp));
        mat.push_back(triplet_eig_cd(pos, offset, real_tmp));
    }

    if(neg != -1)
    {
        mat.push_back(triplet_eig_cd(offset, neg, -real_tmp));
        mat.push_back(triplet_eig_cd(neg, offset, -real_tmp));
    }

    mat.push_back(triplet_eig_cd(offset, source_idx, -val));
}

/*!
    @brief      Inserts the MNA stamp of the CCCS in triplet form
    (i, j, val) inside the mat array. For AC analysis only.
    @param      mat    The triplet matrix to insert the stamp.
    @param      source The CCCS.
*/
void MNA::CccsMNAStamp(tripletList_cd &mat, cccs_packed &source)
{
    auto pos = source.PosNodeID(), neg = source.NegNodeID();
    auto source_idx = source.SourceID() + this->_ivs_offset;
    std::complex<double> val(source.Val(), 0);

    if(pos != -1)
    {
        mat.push_back(triplet_eig_cd(pos, source_idx, val));
    }

    if(neg != -1)
    {
        mat.push_back(triplet_eig_cd(neg, source_idx, -val));
    }
}



/*!
	@brief      Evaluates an EXPONENTIAL source at a given simulation time.
	@param		vvals		The parameters of the exponential source.
	@param		time		The simulation time.
*/
double MNA::EXPSourceEval(std::vector<double> &vvals, double time)
{
	const double i1 = vvals[0];
	const double i2 = vvals[1];
	const double td1 = vvals[2];
    const double tc1 = vvals[3];
	const double td2 = vvals[4];
	const double tc2 = vvals[5];

	if (time <= td1)
		return i1;
	else if ((time > td1) && (time <= td2))
		return (i1 + (i2 - i1) * (1 - exp(-(time - td1) / tc1)));
	else
		return (i1 + (i2 - i1) * (exp(-(time - td2) / tc2) - exp(-(time - td1) / tc1)));
}

/*!
	@brief      Evaluates a SINE source at a given simulation time.
	@param		vvals		The parameters of the sine source.
	@param		time		The simulation time.
*/
double MNA::SINSourceEval(std::vector<double> &vvals, double time)
{
	const double i1 = vvals[0];
	const double ia = vvals[1];
	const double fr = vvals[2];
	const double td = vvals[3];
	const double df = vvals[4];
    const double ph = vvals[5];

	if (time <= td)
		return i1 + ia * sin((2 * M_PI * ph) / 360);
	else
		return i1 + ia * sin(2 * M_PI * fr * (time - td) + 2 * M_PI * ph / 360) * exp(-(time - td) * df);

	/* Suppress compiler warnings */
	return -1;
}

/*!
	@brief      Evaluates a PULSE source at a given simulation time.
	@param		vvals		The parameters of the pulse source.
	@param		time		The simulation time.
*/
double MNA::PULSESourceEval(std::vector<double> &vvals, double time)
{
	const double i1 = vvals[0];
	const double i2 = vvals[1];
	const double td = vvals[2];
	const double tr = vvals[3];
	const double tf = vvals[4];
	const double pw = vvals[5];
	const double per = vvals[6];

	/* This normalizes the pulse inside a period's length */
	IntTp k = (time - td)/ per;
	k = (k <= 0) ? 0 : k;
	double temp_time = time - k *per;

	if(temp_time <= td)
		return i1;
	else if((temp_time > td) && (temp_time <= (td + tr)))
		return (i1 + ((i2 - i1) / tr) * (temp_time - td));
	else if((temp_time > (td + tr)) && (temp_time <= (td + tr + pw)))
		return i2;
	else if((temp_time > (td + tr + pw)) && (temp_time <= (td + tr + pw + tf)))
		return (i2 + ((i1 - i2) / tf) * (temp_time - (td + tr + pw)));
	else if((temp_time > (td + tr + pw + tf)) && (temp_time <= (td + per)))
		return i1;

	/* Suppress compiler warnings */
	return -1;
}

/*!
	@brief      Evaluates a PWL (Piese-Wise Linear) source at a given simulation time.
	@param		tvals		The time(x) values of the PWL.
	@param		vvals		The voltage/current values(y) of the PWL.
	@param		time		The simulation time.
*/
double MNA::PWLSourceEval(std::vector<double> &tvals, std::vector<double> &vvals, double time)
{
	/* Check if we are outside time table of PWL */
	if(time < tvals.front()) return vvals.front();
	else if(time > tvals.back()) return vvals.back();

	/* Else interpolate */
	return linearInterpolation(tvals, vvals, time);
}

/*!
	@brief      Updates the right hand size vector with the time dependent voltage/current
	values during a TRAN analysis.
	@param		rh			The vector to be updated.
	@param		time		The simulation time.
*/
void MNA::UpdateTRANVec(DensVecD &rh, double time)
{
	auto ivs_start = this->_ivs_offset;

	/* Transient stamps for IVS */
	for(auto &it : this->_ivs)
	{
		auto &vvals = it.TranVals();
		double val;

		switch(it.Type())
		{
			case CONSTANT_SOURCE: val = it.Val(); break;
			case EXP_SOURCE: val = EXPSourceEval(vvals, time); break;
			case SINE_SOURCE: val = SINSourceEval(vvals, time); break;
            case PWL_SOURCE: val = PWLSourceEval(it.TranTimes(), vvals, time); break;
			case PULSE_SOURCE: val = PULSESourceEval(vvals, time); break;
			default: val = 0; // Will never reach here
		}

		rh[ivs_start] += val;
		ivs_start++;
	}

	/* Transient stamps for ICS */
	for(auto &it : this->_ics)
	{
		auto pos = it.PosNodeID();
		auto neg = it.NegNodeID();
		auto &vvals = it.TranVals();
		double val;

		switch(it.Type())
		{
			case CONSTANT_SOURCE: val = it.Val(); break;
			case EXP_SOURCE: val = EXPSourceEval(vvals, time); break;
			case SINE_SOURCE: val = SINSourceEval(vvals, time); break;
			case PWL_SOURCE: val = PWLSourceEval(it.TranTimes(), vvals, time); break;
			case PULSE_SOURCE: val = PULSESourceEval(vvals, time); break;
			default: val = 0; // Will never reach here
		}

        if(pos != -1) rh[pos] -= val;
        if(neg != -1) rh[neg] += val;
	}
}



/*!
	@brief      Creates the MNA system for the OP (Operating point) analysis,
	creating the left hand side matrix and the right hand side vector.
	@param		mat			The OP system matrix to be generated.
	@param		rh			The OP system right side vector to be generated.
*/
void MNA::CreateMNASystemOP(SparMatD &mat, DensVecD &rh)
{
	/* Define a temporal TripleMatrix for the creation of the final matrix */
	tripletList_d triplet_mat;

	/* Dimensions and indices */
	auto mat_sz = this->_system_dim;
	auto ivs_start = this->_ivs_offset;
	auto coil_start = this->_coil_offset;
	auto vcvs_start = this->_vcvs_offset;
	auto ccvs_start = this->_ccvs_offset;

	/* Resize for the insertions below */
	rh = DensVecD::Zero(mat_sz);

	/* 1) Iterate over all the passive elements */
	for(auto &it : this->_res) ResMNAStamp(triplet_mat, it);
	for(auto &it : this->_caps) CapMNAStamp(triplet_mat, it, OP);
	for(auto &it : this->_ics) IcsMNAStamp(rh, it);
	for(auto &it : this->_vccs) VccsMNAStamp(triplet_mat, it);
	for(auto &it : this->_cccs) CccsMNAStamp(triplet_mat, it);

	for(auto &it : this->_ivs)
	{
		IvsMNAStamp(triplet_mat, rh, ivs_start, it);
		ivs_start++;
	}

	for(auto &it : this->_coils)
	{
		CoilMNAStamp(triplet_mat, coil_start, it, OP);
		coil_start++;
	}

    for(auto &it : this->_vcvs)
    {
        VcvsMNAStamp(triplet_mat, vcvs_start, it);
        vcvs_start++;
    }

    for(auto &it : this->_ccvs)
    {
        CcvsMNAStamp(triplet_mat, ccvs_start, it);
        ccvs_start++;
    }

	/* 2) Compress the matrix into its final form */
	mat.resize(mat_sz, mat_sz);
	mat.setFromTriplets(triplet_mat.begin(), triplet_mat.end());
}

/*!
	@brief      Creates the MNA system for the DC (direct current) analysis,
	creating the left hand side matrix and the right hand side matrix.
	@param		mat			The DC system matrix to be generated.
	@param		rhs			The DC system right side matrix to be generated.
*/
void MNA::CreateMNASystemDC(SparMatD &mat, DenseMatD &rhs)
{
	/* Use this to get the initial state of the RH */
	DensVecD init_rh;
	IntTp sim_dim = this->_sim_vals.size();
	IntTp sweep_idx = this->_sweep_source_idx;

	/* Fill the Matrix and the vector */
	CreateMNASystemOP(mat, init_rh);

	/* Copy the initial vector to the matrix */
	rhs.resize(this->_system_dim, sim_dim);

	/* Copy initial vector and then modify only the simulated source */
	for(IntTp k = 0; k < sim_dim; k++)
	{
		rhs.col(k) = init_rh;
		rhs(sweep_idx, k) = this->_sim_vals[k];
	}
}

/*!
    @brief      Updates the vector for the DC analysis step.
    @param      rh          The DC system right side vector, OP initial results only.
    @param      sweep_val   The current value of the DC source that is sweeped.
*/
void MNA::UpdateMNASystemDCVec(DensVecD &rh, double sweep_val)
{
    IntTp sweep_idx = this->_sweep_source_idx;

    /* Update only the sweep value */
    rh[sweep_idx] = sweep_val;
}

/*!
	@brief      Creates the MNA system for the TRAN (transient) analysis,
	creating the left hand side matrix.
	@param		mat			The TRAN system matrix to be generated.
*/
void MNA::CreateMNASystemTRAN(SparMatD &mat)
{
	/* Define a temporal TripleMatrix for the creation of the final matrix */
	tripletList_d triplet_mat;

	/* Dimensions and indices */
	auto mat_sz = this->_system_dim;
	auto coil_start = this->_coil_offset;

	/* 1) Iterate over all the passive elements */
	for(auto &it : this->_caps) CapMNAStamp(triplet_mat, it, TRAN);

	for(auto &it : this->_coils)
	{
		CoilMNAStamp(triplet_mat, coil_start, it, TRAN);
		coil_start++;
	}

	/* 2) Compress the matrix into its final form */
	mat.resize(mat_sz, mat_sz);
	mat.setFromTriplets(triplet_mat.begin(), triplet_mat.end());
}

/*!
    @brief      Creates the MNA system for the AC (alternating current) analysis,
    creating the left hand side matrix.
    @param      mat         The AC system matrix to be generated.
    @param      freq        The frequency value the matrix has to be generated for.
*/
void MNA::CreateMNASystemAC(SparMatCompD &mat, double freq)
{
    /* Define a temporal TripleMatrix for the creation of the final matrix */
    tripletList_cd triplet_mat;

    /* Dimensions and indices */
    auto mat_sz = this->_system_dim;
    auto coil_start = this->_coil_offset;
    auto ivs_start = this->_ivs_offset;
    auto vcvs_start = this->_vcvs_offset;
    auto ccvs_start = this->_ccvs_offset;

    /* 1) Iterate over all the passive elements */
    for(auto &it : this->_caps) CapMNAStamp(triplet_mat, it, freq);
    for(auto &it : this->_res) ResMNAStamp(triplet_mat, it);
    for(auto &it : this->_vccs) VccsMNAStamp(triplet_mat, it);
    for(auto &it : this->_cccs) CccsMNAStamp(triplet_mat, it);

    for(auto &it : this->_coils)
    {
        CoilMNAStamp(triplet_mat, coil_start, it, freq);
        coil_start++;
    }

    for(auto &it : this->_ivs)
    {
        IvsMNAStamp(triplet_mat, ivs_start, it);
        ivs_start++;
    }

    for(auto &it : this->_vcvs)
    {
        VcvsMNAStamp(triplet_mat, vcvs_start, it);
        vcvs_start++;
    }

    for(auto &it : this->_ccvs)
    {
        CcvsMNAStamp(triplet_mat, ccvs_start, it);
        ccvs_start++;
    }

    /* 2) Compress the matrix into its final form */
    mat.resize(mat_sz, mat_sz);
    mat.setFromTriplets(triplet_mat.begin(), triplet_mat.end());
}

/*!
    @brief      Creates the MNA system for the AC (alternating current) analysis,
    creating the right hand side vector.
    @param      rh         The AC system right hand side vector to be generated.
*/
void MNA::CreateMNASystemAC(DensVecCompD &rh)
{
    /* Dimensions and indices */
    auto mat_sz = this->_system_dim;

    /* Resize and initialize */
    rh = DensVecD::Zero(mat_sz);
    auto ivs_start = this->_ivs_offset;

    /* 1) Iterate over all the passive elements */
    for(auto &it : this->_ics) IcsMNAStamp(rh, it);

    for(auto &it : this->_ivs)
    {
        IvsMNAStamp(rh, ivs_start, it);
        ivs_start++;
    }
}



/*!
    @brief      Create the plot nodes/sources indices.
    @param      circuit_manager     The circuit.
*/
void MNA::CreatePlotIdx(circuit &circuit_manager)
{
    /* Create the indices vector for nodes or IVS currents */
    auto &node_names = circuit_manager.PlotNodes();
    auto &source_names = circuit_manager.PlotSources();

    /* Get the maps needed */
    auto &nodesmap = circuit_manager.Nodes();
    auto &elementsmap = circuit_manager.ElementNames();

    for(auto &it : node_names)
    {
        /* Search the map - Always exists */
        auto tmp = nodesmap.find(it);

        /* For nodes idx is the unique node ID */
        this->_nodes_idx.push_back(tmp->second);
    }

    for(auto &it : source_names)
    {
        /* Search the map - Always exists */
        auto tmp = elementsmap.find(it);

        /* For voltage sources, (IVSoffset + <idx in the IVS vector>) */
        this->_sources_idx.push_back(tmp->second + this->_ivs_offset);
    }
}

/*!
    @brief      Create the packed representation of the devices.
    @param      circuit_manager     The circuit.
*/
void MNA::CreatePackedVecs(circuit &circuit_manager)
{
    /* Conversion for every element */
    for(auto &it : circuit_manager.Capacitors())
    {
        auto cp = it;
        this->_caps.push_back({cp});
    }

    for(auto &it : circuit_manager.Resistors())
    {
        auto cp = it; // Make a copy
        this->_res.push_back({cp});
    }

    for(auto &it : circuit_manager.Coils())
    {
        auto cp = it;
        this->_coils.push_back({cp});
    }

    for(auto &it : circuit_manager.ICS())
    {
        auto cp = it;
        this->_ics.push_back({cp, cp});
    }

    for(auto &it : circuit_manager.IVS())
    {
        auto cp = it;
        this->_ivs.push_back({cp, cp});
    }

    for(auto &it : circuit_manager.VCVS())
    {
        auto cp = it;
        this->_vcvs.push_back({cp});
    }

    for(auto &it : circuit_manager.VCCS())
    {
        auto cp = it;
        this->_vccs.push_back({cp});
    }

    for(auto &it : circuit_manager.CCVS())
    {
        auto cp = it;
        this->_ccvs.push_back({cp});
    }

    for(auto &it : circuit_manager.CCCS())
    {
        auto cp = it;
        this->_cccs.push_back({cp});
    }
}

/*!
    @brief      Set all the parameters of the MNA engine. This also,
    sets the offsets of each voltage-source-like element inside the MNA system.
    @param      circuit_manager     The circuit.
*/
void MNA::SetMNAParams(circuit &circuit_manager)
{
    /* Set up system dimension */
    auto nodes_dim = circuit_manager.Nodes().size();

    // SOS! This is the organization of the voltage like elements in the matrix
    _ivs_offset = nodes_dim;
    _coil_offset = nodes_dim + _ivs.size();
    _vcvs_offset = _coil_offset + _coils.size();
    _ccvs_offset = _vcvs_offset + _vcvs.size();
    _system_dim = _ccvs_offset + _ccvs.size();
    // SOS!

    _analysis_type = circuit_manager.AnalysisType();
    _scale = circuit_manager.AnalysisScale();

    /* Create the simulation vector */
    double start = circuit_manager.SimStart();
    double end = circuit_manager.SimEnd();
    double steps = circuit_manager.SimStep();
    _sim_step = steps;

    if(this->_analysis_type != OP)
    {
        /* Now create the simulation times/values vector */
        if(this->_scale == DEC_SCALE)
        {
            /* For AC we generate based on step (since argument is [points]) */
            if(this->_analysis_type == AC) linspaceVecGen(this->_sim_vals, start, end, static_cast<size_t>(steps));
            else StepVecGen(this->_sim_vals, start, end, steps);
        }
        else
        {
            /* Generate logarithmically spaced vector */
            logspaceVecGen(this->_sim_vals, start, end, static_cast<size_t>(steps));
        }
    }

    /* In case we have a DC analysis source */
    if(this->_analysis_type == DC)
    {
        /* 2-level index to get the exact position of the source */
        const std::string &src_dut = circuit_manager.DCSource();
        auto &namesmap = circuit_manager.ElementNames();

        /* Get idx to access the appropriate vector */
        this->_sweep_source_idx = namesmap.find(src_dut)->second;

        /* Voltage sweep needs an offset */
        if(src_dut[0] == 'V') this->_sweep_source_idx += this->_ivs_offset;
    }
}

/*!
    @brief      Prints the triplet matrix given. Used only for debugging.
    @param      mat     The triplet matrix to be printed.
*/
void MNA::debug_triplet_mat(tripletList_d &mat)
{
    /* Set precision */
    std::cout.precision(15);
    std::cout << std::fixed;

    /* Output to console */
    std::cout << "Triplet matrix:\n";
    for(auto &it : mat)
    {
        std::cout << it.row() << " " << it.col() << " " << it.value() << "\n";
    }
}
