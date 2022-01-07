#include <fstream>
#include <iomanip>
#include <cstdio>
#include <limits>
#include "plot.hpp"
#include <unistd.h>

class GNU_plotter
{

public:
    GNU_plotter()
    {
        this->_plot_num = 0;
        this->_pipe = popen("gnuplot -persistent", "w");

        /* TODO - Check that every opening went well */
        if(!this->_pipe)
        {
            throw std::runtime_error("[CRITICAL ERROR]: Could not open files related to GNUPLOT");
        }
    }

    ~GNU_plotter()
    {
        /* Close gnuplot */
        fprintf(this->_pipe, "pause -5\n\n quit\n");
        pclose(this->_pipe);

        /* Close and remove files */
        if(this->_data_file)  this->_data_file.close();
        for(auto &it : this->_file_names) remove(it.c_str());
    }

    void next_plot()
    {
        /* Close previous file */
        if(this->_data_file) this->_data_file.close();

        /* Ascending number */
        std::string filename = ".gnuplotdata" + std::to_string(this->_plot_num) +  ".plt";
        this->_file_names.push_back(filename);

        this->_plot_num++;
        fprintf(this->_pipe, "set term qt %d\n", this->_plot_num);

        /* Create the new file */
        this->_data_file = std::ofstream(filename, std::ios::out);

        /* Throw */
        if(!this->_data_file)
            throw std::runtime_error("[CRITICAL ERROR]: Could not open files related to GNUPLOT");
    }

    /* File handler to the GNUPLOT sub-process pipe */
    FILE *_pipe;
    IntTp _plot_num;
    std::ofstream _data_file;
    std::vector<std::string> _file_names;

    /* Ploter function */
    void plot(Circuit &circuit_manager, simulator_engine &simulator_manager);
    void setPlotOptions(Circuit &circuit_manager, bool source);
    void sendPlotData(Circuit &circuit_manager, simulator_engine &simulator_manager, bool source);
    void finalize(std::vector<std::string> &plotnames);
};

/* TODO Comment */
void GNU_plotter::finalize(std::vector<std::string> &plotnames)
{
    /* Send the final commands to plot the vectors */
    fprintf(this->_pipe, "plot '%s' using 1:2 title '%s' with lines", this->_file_names.back().c_str(), plotnames.front().c_str());

    for (size_t i = 1; i < plotnames.size(); i++)
    {
        fprintf(this->_pipe, ",'' using 1:%ld title '%s' with lines", (i + 2), plotnames[i].c_str());
    }

    fprintf(this->_pipe, "\n");
    fflush(this->_pipe);
}

/* TODO - Comment */
void GNU_plotter::sendPlotData(Circuit &circuit_manager, simulator_engine &simulator_manager, bool source)
{
	using std::vector;

	/* Necessary components */
	auto &results = simulator_manager.getResultsD();
	auto &sim_vals = simulator_manager.getSimulationVec();
	vector<IntTp> plot_nodes_idx;

	/* Create a temporal vector of indices for accessing the results */
	if(source)
	{
	    auto &elementsmap = circuit_manager.getElementNames();
	    auto &plotsources = circuit_manager.getPlotSources();

        for(size_t i = 0; i < plotsources.size(); i++)
        {
            auto it = elementsmap.find(plotsources[i]);
            plot_nodes_idx.push_back(it->second + circuit_manager.getNodes().size());
        }
	}
	else
	{
	    auto &nodesmap = circuit_manager.getNodes();
	    auto &plotnodes = circuit_manager.getPlotNodes();

	    for(size_t i = 0; i < plotnodes.size(); i++)
	    {
	        auto it = nodesmap.find(plotnodes[i]);
	        plot_nodes_idx.push_back(it->second);
	    }
	}

	for(size_t i = 0; i < sim_vals.size(); i++)
	{
		/* First iteration send also the x value */
		IntTp x_idx = plot_nodes_idx.front();
		this->_data_file << sim_vals[i] << "\t" << results(x_idx, i);

		for(size_t k = 1; k < (plot_nodes_idx.size() - 1); k++)
		{
			x_idx = plot_nodes_idx[k];
			this->_data_file << "\t" << results(x_idx, i);
		}

		x_idx = plot_nodes_idx.back();
		this->_data_file << "\t" << results(x_idx, i) << "\n";
	}

	this->_data_file << std::endl;
}

/* TODO Comment */
void GNU_plotter::setPlotOptions(Circuit &circuit_manager, bool source)
{
    using std::string;

    auto &source_name = circuit_manager.getDCSource();
    auto analysis_type = circuit_manager.getAnalysisType();
    auto analysis_scale = circuit_manager.getAnalysisScale();

    /* Base options */
    string title("set title ");
    string xlabel("set xlabel ");
    string ylabel("set ylabel ");
    string scaleauto("set autoscale\n");
    string grid("set grid\n");
    string scale = (analysis_scale == LOG_SCALE) ? "set logscale y 10\n":"\n"; /* TODO - Logscale */
    string legend("set key outside\nset key right top\n");

    switch(analysis_type)
    {
        case DC:
        {
            title += "'DC analysis results'";

            xlabel += "'";
            xlabel += source_name + " ";
            xlabel += (source_name[0] == 'I') ? "(A)" : "(V)";
            xlabel += "'";

            ylabel += "'";
            ylabel += (source) ? "Source Current (A)": "Node Voltage (V)";
            ylabel += "'";

            break;
        }
        case TRAN:
        {
            title += "'Transient analysis results'";
            xlabel += "'Time (s)'";

            ylabel += "'";
            ylabel += (source) ? "Source Current (A)": "Node Voltage (V)";
            ylabel += "'";

            break;
        }
        case AC: // TODO - Also have to check if it phase or
        {
            title += "'AC analysis results'";
            xlabel += "'Frequency (Hz)'";

            ylabel += "'";
            ylabel += (source) ? "Source Current (A)": "Node Voltage (V)";
            ylabel += "'";

            break;
        }
        default: // OP analysis is ignored
        {
            return;
        }
    }

    /* Terminate with newline each command */
    title += "\n";
    xlabel += "\n";
    ylabel += "\n";

    /* Form the commands and send */
    string commands = title + xlabel + ylabel + legend + grid + scale + scaleauto;
    fprintf(this->_pipe, "%s", commands.c_str());
}

/* TODO - Comment */
void GNU_plotter::plot(Circuit &circuit_manager, simulator_engine &simulator_manager)
{
	using namespace std;

	auto &plotsources = circuit_manager.getPlotSources();
	auto &plotnodes = circuit_manager.getPlotNodes();

    /* Send */
    if(plotsources.size())
    {
        this->next_plot();
        sendPlotData(circuit_manager, simulator_manager, true);
        setPlotOptions(circuit_manager, true);
        finalize(plotsources);
    }

    /* Send */
    if(plotnodes.size())
    {
        this->next_plot();
        sendPlotData(circuit_manager, simulator_manager, false);
        setPlotOptions(circuit_manager, false);
        finalize(plotnodes);
    }
}

/* TODO - Comment */
return_codes_e plot(Circuit &circuit_manager, simulator_engine &simulator_manager)
{
    using namespace std;

    /* Checks */
    if(!circuit_manager.valid()) return FAIL_PLOTTER_CIRCUIT_INVALID;
    if(!simulator_manager.valid()) return FAIL_PLOTTER_RESULTS_INVALID;
    if(!circuit_manager.getPlotNodes().size() && !circuit_manager.getPlotSources().size())
        return FAIL_PLOTTER_NOTHING_TO_PLOT;

    /* Set precision */
    streamsize cout_stream_sz = std::cout.precision(std::numeric_limits<double>::digits10 + 2);

    /* OP analysis need only printing of the values */
    if(circuit_manager.getAnalysisType() != OP)
    {
        GNU_plotter plotter;
        plotter.plot(circuit_manager, simulator_manager);


    }
    else
    { /* TODO - Implement printing for OP */

    }


    /* Reset precision */
    std::cout.precision(cout_stream_sz);

    return RETURN_SUCCESS;
}
