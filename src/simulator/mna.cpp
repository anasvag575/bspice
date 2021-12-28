#include <mna.hpp>
#include <string>
#include <cassert>

/* Main MNA creator for OP system */
void MNA::CreateMNASystemOP(Circuit &circuit_manager, SparMatD &mat, DensVecD &rh)
{
	/* Define a temporal TripleMatrix for the creation of the final matrix */
	tripletList triplet_mat;

	/* 1) Load all the elements, nodesmap and namesmap */
	auto &nodesmap = circuit_manager.getNodes();
	auto &caps = circuit_manager.getCapacitors();
	auto &coils = circuit_manager.getCoils();
	auto &res = circuit_manager.getResistors();
	auto &ics = circuit_manager.getICS();
	auto &ivs = circuit_manager.getIVS();

	/* Size of the matrix */
	long int source_dim = coils.size() + ivs.size();
	long int nodes_dim = nodesmap.size();
	long int source_idx = nodes_dim;

	/* Resize for the insertions below */
//	rh.resize(nodes_dim + source_dim);
	rh = DensVecD::Zero(nodes_dim + source_dim);

	/* 1) Iterate over all the passive elements */
	for(auto &it : res) it.MNAStampDC(triplet_mat);
	for(auto &it : caps) it.MNAStampDC(triplet_mat);
	for(auto &it : ics) it.MNAStampDC(rh);

	for(auto &it : coils)
	{
		it.MNAStampDC(triplet_mat, source_idx);
		source_idx++;
	}

	for(auto &it : ivs)
	{
		it.MNAStampDC(triplet_mat, rh, source_idx);
		source_idx++;
	}

	/* 2) Compress the matrix into its final form */
	mat.resize(nodes_dim + source_dim, nodes_dim + source_dim);
	mat.setFromTriplets(triplet_mat.begin(), triplet_mat.end());

//	std::cout << "Eigen Vector:\n"<<std::endl;
//	std::cout << rh << std::endl;
//
//	std::cout << "Eigen Matrix:\n"<<std::endl;
//	std::cout << mat << std::endl;

}

/* Main MNA creator for OP system */
void MNA::CreateMNASystemDC(Circuit &circuit_manager, SparMatD &mat, DenseMatD &rhs)
{
	/* Use this to get the initial state of the RH */
	DensVecD init_rh;

	long int total_size =  circuit_manager.getIVS().size() + circuit_manager.getCoils().size()
							+ circuit_manager.getNodes().size();

	/* Fill the Matrix and the vector */
	CreateMNASystemOP(circuit_manager, mat, init_rh);

	/* 2-level index to get the exact position of the source */
	std::string &src_dut = circuit_manager.getDCSource();
	auto &namesmap = circuit_manager.getElementNames();

	/* Get idx to access the appropriate vector */
	long int offset_idx = namesmap.find(src_dut)->second;

	if(src_dut[0] == 'V') /* Voltage sweep needs an offset */
	{
		offset_idx += circuit_manager.getIVS().size() + circuit_manager.getCoils().size();
	}

	auto &sim_vector = circuit_manager.getSimValues();
	long int sim_size = sim_vector.size();

	/* Copy the initial vector to the matrix */
	rhs.resize(total_size, sim_size);

	for(long int k = 0; k < sim_size; k++)
	{
		rhs.col(k) = init_rh;
		rhs(offset_idx, k) = sim_vector[k];
	}
}
