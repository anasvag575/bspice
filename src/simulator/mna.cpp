#include <math.h>	/* Legacy C for PI constant */
#include "mna.hpp"

/*!
	@brief      Inserts the MNA stamp of the resistor in triplet form
	(i, j, val) inside the mat array. For DC analysis resistors have 1 or 4 stamps.
	@param      mat    The triplet matrix to insert the stamp.
	@param 		res	   The resistor.
*/
void MNA::ResMNAStamp(tripletList_d &mat, Resistor &res)
{
	auto &arr = res.getNodeIDs();
	auto conduct = 1/res.getVal();

	if(arr[0] != -1) mat.push_back(triplet_eig_d(arr[0], arr[0], conduct));
	if(arr[1] != -1) mat.push_back(triplet_eig_d(arr[1], arr[1], conduct));

	if((arr[0] != -1) && (arr[1] != -1))
	{
	    mat.push_back(triplet_eig_d(arr[1], arr[0], -conduct));
		mat.push_back(triplet_eig_d(arr[0], arr[1], -conduct));
	}
}

/*!
	@brief      Inserts the MNA stamp of the coil in triplet form
	(i, j, val) inside the mat array. Since coils act like DC shorts
	they need to be inserted in the sub-arrays for sources, hence the
	offset. For DC analysis coils have 2 or 4 stamps.
	@param      mat    The triplet matrix to insert the stamp.
	@param		offset The offset of the stamp inside the array.
	@param		coil   The coil.

*/
void MNA::CoilMNAStamp(tripletList_d &mat, IntTp offset, Coil &coil, const analysis_t type)
{
	auto &arr = coil.getNodeIDs();

	if(type == OP)
	{
		if(arr[0] != -1)
		{
			mat.push_back(triplet_eig_d(offset, arr[0], 1));
			mat.push_back(triplet_eig_d(arr[0], offset, 1));
		}
		if(arr[1] != -1)
		{
			mat.push_back(triplet_eig_d(offset, arr[1], -1));
			mat.push_back(triplet_eig_d(arr[1], offset, -1));
		}
	}
	else if(type == TRAN)
	{
		mat.push_back(triplet_eig_d(offset, offset, -coil.getVal()));
	}
}

/*!
    @brief      Inserts the MNA stamp of the capacitor in triplet form
    (i, j, val) inside the mat array. For DC analysis capacitors have 0 stamps.
    @param      mat    The triplet matrix to insert the stamp.
    @param		cap	   The capacitor.
*/
void MNA::CapMNAStamp(tripletList_d &mat, Capacitor &cap, const analysis_t type)
{
	if(type == TRAN)
	{
		auto &arr = cap.getNodeIDs();
		auto capacitance = cap.getVal();

		if(arr[0] != -1) mat.push_back(triplet_eig_d(arr[0], arr[0], capacitance));
		if(arr[1] != -1) mat.push_back(triplet_eig_d(arr[1], arr[1], capacitance));

		if((arr[0] != -1) && (arr[1] != -1))
		{
			mat.push_back(triplet_eig_d(arr[0], arr[1], -capacitance));
			mat.push_back(triplet_eig_d(arr[1], arr[0], -capacitance));
		}
	}
}

/*!
    @brief      Inserts the MNA stamp of the ICS in triplet form
    (i, j, val) inside the mat array. For DC analysis ICS have 1 or 2 stamps.
	@param      rh     The right hand side vector of the system.
    @param		source The ICS.
*/
void MNA::IcsMNAStamp(DensVecD &rh, ics &source)
{
	auto &arr = source.getNodeIDs();
	auto val = source.getVal();

	if(arr[0] != -1) rh[arr[0]] -= val;
	if(arr[1] != -1) rh[arr[1]] += val;
}

/*!
    @brief      Inserts the MNA stamp of the IVS in triplet form
    (i, j, val) inside the mat array. IVS need to be inserted
    in the sub-arrays for sources, hence the offset.
    For DC analysis coils have 3 or 5 stamps.
    @param      mat    The triplet matrix to insert the stamp.
    @param      rh     The right hand side vector of the system.
    @param		offset The offset of the stamp inside the array
    @param		source The IVS.
*/
void MNA::IvsMNAStamp(tripletList_d &mat, DensVecD &rh, IntTp offset, ivs &source)
{
	auto &arr = source.getNodeIDs();

	if(arr[0] != -1)
	{
		mat.push_back(triplet_eig_d(offset, arr[0], 1));
		mat.push_back(triplet_eig_d(arr[0], offset, 1));
	}

	if(arr[1] != -1)
	{
		mat.push_back(triplet_eig_d(offset, arr[1], -1));
		mat.push_back(triplet_eig_d(arr[1], offset, -1));
	}

	rh[offset] += source.getVal();
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

	/* Supress compiler warnings */
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

	/* TODO - Debug */
	std::cout << "ERROR: " << std::endl;

	/* Supress compiler warnings */
	return -1;
}

/*!
	@brief      Evaluates a PWL (Piese-Wise Linear) source at a given simulation time.
	@param		tvals		The time(x) values of the PWL.
	@param		vvals		The voltage/current(y) values of the PWL.
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
	@param		circuit		The circuit that is analyzed.
	@param		rh			The vector to be updated.
	@param		time		The simulation time.
*/
void MNA::UpdateTRANVec(Circuit &circuit_manager, DensVecD &rh, double time)
{
	auto &ivs = circuit_manager.getIVS();
	auto &ics = circuit_manager.getICS();
	auto ivs_start = this->_ivs_offset;

	/* Transient stamps for IVS */
	for(auto &it : ivs)
	{
		auto &vvals = it.getTranVals();

		switch(it.getType())
		{
			case CONSTANT_SOURCE:
			{
				rh[ivs_start] += it.getVal();
				break;
			}
			case EXP_SOURCE:
			{
				rh[ivs_start] += EXPSourceEval(vvals, time);
				break;
			}
			case SINE_SOURCE:
			{
				rh[ivs_start] += SINSourceEval(vvals, time);
				break;
			}
			case PWL_SOURCE:
			{
				auto &tvals = it.getTranTimes();
				rh[ivs_start] += PWLSourceEval(tvals, vvals, time);
				break;
			}
			case PULSE_SOURCE:
			{
				rh[ivs_start] += PULSESourceEval(vvals, time);
				break;
			}
			default:
			{
				std::cout << "ERROR MNA UPDATE TRAN:" << std::endl;
			}
		}

		ivs_start++;
	}

	/* Transient stamps for ICS */
	for(auto &it : ics)
	{
		auto pos = it.getNodeIDs()[0];
		auto neg = it.getNodeIDs()[1];
		auto &vvals = it.getTranVals();

		switch(it.getType())
		{
			case CONSTANT_SOURCE:
			{
				if(pos != -1) rh[pos] -= it.getVal();
				if(neg != -1) rh[neg] += it.getVal();
				break;
			}
			case EXP_SOURCE:
			{
				auto val = EXPSourceEval(vvals, time);
				if(pos != -1) rh[pos] -= val;
				if(neg != -1) rh[neg] += val;
				break;
			}
			case SINE_SOURCE:
			{
				auto val = SINSourceEval(vvals, time);
				if(pos != -1) rh[pos] -= val;
				if(neg != -1) rh[neg] += val;
				break;
			}
			case PWL_SOURCE:
			{
				auto &tvals = it.getTranTimes();
				auto val = PWLSourceEval(tvals, vvals, time);
				if(pos != -1) rh[pos] -= val;
				if(neg != -1) rh[neg] += val;
				break;
			}
			case PULSE_SOURCE:
			{
				auto val = PULSESourceEval(vvals, time);
				if(pos != -1) rh[pos] -= val;
				if(neg != -1) rh[neg] += val;

				break;
			}
			default:
			{
				std::cout << "ERROR MNA UPDATE TRAN:" << std::endl;
			}
		}
	}

}



/*!
	@brief      Creates the MNA system for the OP (Operating point) analysis,
	creating the left hand side matrix and the right hand side vector.
	@param		circuit		The circuit that is analyzed.
	@param		mat			The OP system matrix to be generated.
	@param		rh			The OP system right side vector to be generated.
*/
void MNA::CreateMNASystemOP(Circuit &circuit_manager, SparMatD &mat, DensVecD &rh)
{
	/* Define a temporal TripleMatrix for the creation of the final matrix */
	tripletList_d triplet_mat;

	/* 1) Load all the elements */
	auto &caps = circuit_manager.getCapacitors();
	auto &coils = circuit_manager.getCoils();
	auto &res = circuit_manager.getResistors();
	auto &ics = circuit_manager.getICS();
	auto &ivs = circuit_manager.getIVS();

	/* Dimensions and indices */
	auto mat_sz = this->_system_dim;
	auto ivs_start = this->_ivs_offset;
	auto coil_start = this->_coil_offset;

	/* Resize for the insertions below */
	rh = DensVecD::Zero(mat_sz);

	/* 1) Iterate over all the passive elements */
	for(auto &it : res) ResMNAStamp(triplet_mat, it);
	for(auto &it : caps) CapMNAStamp(triplet_mat, it, OP);
	for(auto &it : ics) IcsMNAStamp(rh, it);

	for(auto &it : ivs)
	{
		IvsMNAStamp(triplet_mat, rh, ivs_start, it);
		ivs_start++;
	}

	for(auto &it : coils)
	{
		CoilMNAStamp(triplet_mat, coil_start, it, OP);
		coil_start++;
	}

	/* 2) Compress the matrix into its final form */
	mat.resize(mat_sz, mat_sz);
	mat.setFromTriplets(triplet_mat.begin(), triplet_mat.end());
}

/*!
	@brief      Creates the MNA system for the DC (direct current) analysis,
	creating the left hand side matrix and the right hand side matrix.
	@param		circuit		The circuit that is analyzed.
	@param		mat			The DC system matrix to be generated.
	@param		rh			The DC system right side matrix to be generated.
*/
void MNA::CreateMNASystemDC(Circuit &circuit_manager, SparMatD &mat, DenseMatD &rhs)
{
	/* Use this to get the initial state of the RH */
	DensVecD init_rh;

	/* Fill the Matrix and the vector */
	CreateMNASystemOP(circuit_manager, mat, init_rh);

	/* 2-level index to get the exact position of the source */
	std::string &src_dut = circuit_manager.getDCSource();
	auto &namesmap = circuit_manager.getElementNames();

	/* Get idx to access the appropriate vector */
	auto ivs_idx = namesmap.find(src_dut)->second;

	/* Voltage sweep needs an offset */
	if(src_dut[0] == 'V') ivs_idx += this->_ivs_offset;

	/* Copy the initial vector to the matrix */
	rhs.resize(this->_system_dim, this->_sim_dim);

	/* Copy initial vector and then modify only the simulated source */
	for(IntTp k = 0; k < this->_sim_dim; k++)
	{
		rhs.col(k) = init_rh;
		rhs(ivs_idx, k) = this->_sim_vals[k];
	}
}

/*!
	@brief      Creates the MNA system for the TRAN (transient) analysis,
	creating the left hand side matrix.
	@param		circuit		The circuit that is analyzed.
	@param		mat			The TRAN system matrix to be generated.
*/
void MNA::CreateMNASystemTRAN(Circuit &circuit_manager, SparMatD &mat)
{
	/* Define a temporal TripleMatrix for the creation of the final matrix */
	tripletList_d triplet_mat;

	/* 1) Load all the elements for the MNA matrix */
	auto &caps = circuit_manager.getCapacitors();
	auto &coils = circuit_manager.getCoils();

	/* Dimensions and indices */
	auto mat_sz = this->_system_dim;
	auto coil_start = this->_coil_offset;

	/* 1) Iterate over all the passive elements */
	for(auto &it : caps) CapMNAStamp(triplet_mat, it, TRAN);

	for(auto &it : coils)
	{
		CoilMNAStamp(triplet_mat, coil_start, it, TRAN);
		coil_start++;
	}


	/* 2) Compress the matrix into its final form */
	mat.resize(mat_sz, mat_sz);
	mat.setFromTriplets(triplet_mat.begin(), triplet_mat.end());
}



/*!
    @brief      Prints the triplet matrix given. Used only for debugging.
    @param      circuit     The triplet matrix to be printed
*/
void debug_triplet_mat(tripletList_d &mat)
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
