#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstdio>
#include <limits>
#include "plot.hpp"

/* TODO - Comment */
static void sendPlotData(Circuit &circuit_manager, simulator_engine &simulator_manager, std::ofstream &stream)
{
	using namespace std;

	/* Necessary components */
	auto &nodesmap = circuit_manager.getNodes();
	auto &results = simulator_manager.getResultsD();
	auto &sim_vals = simulator_manager.getSimulationVec();
	auto &plotnodes = circuit_manager.getPlotNodes();
	vector<IntTp> plot_nodes_idx;

	/* Create a temporal vector of indices for accessing the results */
	for(size_t i = 0; i < plotnodes.size(); i++)
	{
		auto it = nodesmap.find(plotnodes[i]);
		plot_nodes_idx.push_back(it->second);
	}

	for(size_t i = 0; i < sim_vals.size(); i++)
	{
		/* First iteration send also the x value */
		IntTp x_idx = plot_nodes_idx.front();
		stream << sim_vals[i] << "\t" << results(x_idx, i);

		for(size_t k = 0; k < (plot_nodes_idx.size() - 1); k++)
		{
			x_idx = plot_nodes_idx[k];
			stream << "\t" << results(x_idx, i);
		}

		x_idx = plot_nodes_idx.back();
		stream << "\t" << results(x_idx, i) << "\n";
	}
}

/* TODO - Comment */
return_codes_e plot(Circuit &circuit_manager, simulator_engine &simulator_manager)
{
	using namespace std;

	/* Checks */
	if(!circuit_manager.valid()) return FAIL_PLOTTER_CIRCUIT_INVALID;
	if(!simulator_manager.valid()) return FAIL_PLOTTER_RESULTS_INVALID;

	/* TODO - Checks if we really have anything to plot */
	if(!circuit_manager.getPlotNodes().size())
	    return RETURN_SUCCESS;

	/* Set precision and gnuplot environment */
	std::cout.precision(std::numeric_limits<double>::digits10 + 2);
	FILE *gnuplotPipe = popen("gnuplot -persistent", "w");
	ofstream data_file(".gnuplotdata", ios::out);

	/* Checks */
	if(!data_file || !gnuplotPipe) return FAIL_PLOTTER_IO_OPERATIONS;

	/* TODO - Commands */
	std::string commands("set title 'Simulation results'\n");
	commands += "set xlabel 'Sweep var'\n";
	commands += "set ylabel 'Node voltage(V)'\n";

	/* Send the commands and the plot data */
	fprintf(gnuplotPipe, commands.c_str());
	sendPlotData(circuit_manager, simulator_manager, data_file);

	auto &plotnodes = circuit_manager.getPlotNodes();

	/* Send the final commands to plot the vectors */
	fprintf(gnuplotPipe, "plot '.gnuplotdata' using 1:2 title '%s' with lines", plotnodes.front().c_str());

	for (size_t i = 1; i < plotnodes.size(); i++)
	{
		fprintf(gnuplotPipe, ",'' using 1:%ld title '%s' with lines", (i + 2), plotnodes[i].c_str());
	}
	fprintf(gnuplotPipe, "\nquit\n");

	/* Close files */
	data_file.close();
	pclose(gnuplotPipe);
	remove(".gnuplotdata");

	return RETURN_SUCCESS;
}
