#include <fstream>
#include <iomanip>
#include <cstdio>
#include <limits>
#include <math.h>
#include <unistd.h>
#include "plot.hpp"

class GNU_plotter
{

public:
    /*!
        @brief    Default constructor, creates the GNUPLOT pipe.
    */
    GNU_plotter()
    {
        this->_plot_num = 0;
        this->_pipe = popen("gnuplot -persistent", "w");

        /* TODO - Check that every opening went well */
        if(!this->_pipe)
        {  // TODO - Handle
            throw std::runtime_error("[CRITICAL ERROR]: Could not open files related to GNUPLOT");
        }
    }

    /*!
        @brief    Default destructor, closes GNUPLOT pipe and removes the files created.
    */
    ~GNU_plotter()
    {
        /* Close gnuplot */
        fprintf(this->_pipe, "pause -5\n\n quit\n");
        pclose(this->_pipe);

        /* Close and remove files */
        if(this->_data_file)  this->_data_file.close();
        for(auto &it : this->_file_names) remove(it.c_str());
    }

    /*!
        @brief    Creates the next plot window.
    */
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
        if(!this->_data_file) // TODO - Handle
            throw std::runtime_error("[CRITICAL ERROR]: Could not open files related to GNUPLOT");
    }

    /* File handler to the GNUPLOT sub-process pipe */
    FILE *_pipe;
    IntTp _plot_num;
    std::ofstream _data_file;
    std::vector<std::string> _file_names;

    /* Ploter function */
    void plot(Circuit &circuit_manager, simulator_engine &simulator_manager);
    void setPlotOptions(analysis_t type, as_scale_t scale, std::string &sweep, bool source, bool mag);
    void sendPlotData(std::vector<double> &xvals, std::vector<std::vector<double>> &yvals, bool log);
    void sendPlotData(std::vector<double> &xvals, std::vector<std::vector<std::complex<double>>> &yvals, bool mag, bool log);
    void finalize(std::vector<std::string> &plotnames);
};

/*!
    @brief      Sends the final plot command to GNUPLOT to display the graph.
    @param      plotnames     The legend names for the staff to plot.
*/
void GNU_plotter::finalize(std::vector<std::string> &plotnames)
{
    /* Send the final commands to plot the vectors */
    fprintf(this->_pipe, "plot '%s' using 1:2 title '%s' with lines", this->_file_names.back().c_str(), plotnames.front().c_str());

    for (size_t i = 1; i < plotnames.size(); i++)
    {
        fprintf(this->_pipe, ",'' using 1:%ld title '%s' with lines", (i + 2), plotnames[i].c_str());
    }

    /* Newline to finish command and flush for synchronization */
    fprintf(this->_pipe, "\n");
    fflush(this->_pipe);
}

/*!
    @brief      Sends the data that will be plotted on the current window.
    @param      xvals     The x values vector, simulation vector.
    @param      yvals     The y values vector(s), result(s) vector.
*/
void GNU_plotter::sendPlotData(std::vector<double> &xvals, std::vector<std::vector<double>> &yvals, bool log)
{
	using std::vector;
	using std::endl;

	for(size_t i = 0; i < xvals.size(); i++)
	{
	    auto &tmp = yvals[i];

		/* First iteration send also the x value */
		this->_data_file << xvals[i];

		/* Create the columns of data */
		if(log)
		{
		    for(size_t k = 0; k < tmp.size(); k++) this->_data_file << "\t" << 20 * log10(tmp[k]);
		}
		else
		{
		    for(size_t k = 0; k < tmp.size(); k++) this->_data_file << "\t" << tmp[k];
		}

		/* Next sim time */
		this->_data_file << "\n";
	}

	/* Endl also performs a flush */
	this->_data_file << endl;
}

/*!
    @brief      Sends the data that will be plotted on the current window, for AC analysis only.
    @param      xvals     The x values vector, simulation vector.
    @param      yvals     The y values vector(s), result(s) vector.
    @param      mag       Whether to send magnitude data or phase
*/
void GNU_plotter::sendPlotData(std::vector<double> &xvals, std::vector<std::vector<std::complex<double>>> &yvals, bool log, bool mag)
{
    using std::vector;
    using std::abs;
    using std::arg;
    using std::endl;

    for(size_t i = 0; i < xvals.size(); i++)
    {
        auto &tmp = yvals[i];

        /* First iteration send also the x value */
        this->_data_file << xvals[i];

        /* Create the columns of data */
        if(mag)
        {
            if(log)
            {
                for(size_t k = 0; k < tmp.size(); k++) this->_data_file << "\t" << 20 * log10(abs(tmp[k]));
            }
            else
            {
                for(size_t k = 0; k < tmp.size(); k++) this->_data_file << "\t" << abs(tmp[k]);
            }
        }
        else
        {
            /* Arg returns in radians convert to degrees */
            for(size_t k = 0; k < tmp.size(); k++) this->_data_file << "\t" << arg(tmp[k]) * 180/M_PI;
        }

        /* Next sim time */
        this->_data_file << "\n";
    }

    /* Endl also performs a flush */
    this->_data_file << endl;
}

/*!
    @brief      Sends the data that will be plotted on the current window.
    @param      type       The type of analysis.
    @param      scale      The scale of the analysis.
    @param      sweep      The sweeped source (for DC only)
    @param      source     Whether, we plot a source(true) or a node
    @param      mag        Whether, we plot magnitude(true) or phase (only for AC)
*/
void GNU_plotter::setPlotOptions(analysis_t type, as_scale_t scale, std::string &sweep, bool source, bool mag)
{
    using std::string;

    /* Base options */
    string title("set title ");
    string xlabel("set xlabel ");
    string ylabel("set ylabel ");
    string scaleauto("set autoscale\n");
    string grid("set grid\n");
    string legend("set key outside\nset key right top\n");

    switch(type)
    {
        case DC:
        {
            title += "'DC analysis results'";

            xlabel += "'";
            xlabel += sweep + " ";
            xlabel += (sweep[0] == 'I') ? "(A)" : "(V)";
            xlabel += "'";
            break;
        }
        case TRAN:
        {
            title += "'Transient analysis results'";
            xlabel += "'Time (s)'";
            break;
        }
        case AC: // TODO - Also have to check if it phase or
        {
            title += "'AC analysis results'";
            xlabel += "'Frequency (Hz)'";
            break;
        }
        default: // OP analysis is ignored
        {
            return;
        }
    }

    /* For y label we have same global config */
    ylabel += "'";
    if(mag)
    {
        ylabel += (source) ? "Source Current (A)" : "Node Voltage (V)";
        ylabel += (scale == LOG_SCALE) ? " - dB scale" : "";
    }
    else
    {
        ylabel += (source) ? "Source Current phase (degrees)" : "Node Voltage phase (degrees)";
    }
    ylabel += "'";

    /* Terminate with newline each command */
    title += "\n";
    xlabel += "\n";
    ylabel += "\n";

    /* Form the commands and send */
    string commands = title + xlabel + ylabel + legend + grid + scaleauto;
    fprintf(this->_pipe, "%s", commands.c_str());
}

/*!
    @brief      Sends the data that will be plotted on the current window.
    @param      circuit_manager     The simulated circuit.
    @param      source     Whether, we plot a source(true) or a node.
    @param      mag        Whether, we plot magnitude(true) or phase (only for AC).
*/
void GNU_plotter::plot(Circuit &circuit_manager, simulator_engine &simulator_manager)
{
	using std::vector;
	using std::complex;

	auto &plotsources = circuit_manager.PlotSources();
	auto &plotnodes = circuit_manager.PlotNodes();

	auto analysis_type = circuit_manager.AnalysisType();
	auto analysis_scale = circuit_manager.AnalysisScale();
	auto sweep_source = circuit_manager.DCSource();

    /* Get the x-values */
    auto &x_simvals = simulator_manager.getSimulationVec();

    /* For TRAN we have only 1 value to print */
	if(analysis_type != AC)
	{
        /* Send */
        if(plotsources.size())
        {
            auto &res = simulator_manager.getSourceResults();

            this->next_plot();
            sendPlotData(x_simvals, res, analysis_scale == LOG_SCALE);
            setPlotOptions(analysis_type, analysis_scale, sweep_source, true, true);
            finalize(plotsources);
        }

        /* Send */
        if(plotnodes.size())
        {
            auto &res = simulator_manager.getNodesResults();

            this->next_plot();
            sendPlotData(x_simvals, res, analysis_scale == LOG_SCALE);
            setPlotOptions(analysis_type, analysis_scale, sweep_source, false, true);
            finalize(plotnodes);
        }
	}
	else
	{
        /* Send */
        if(plotsources.size())
        {
            auto &res = simulator_manager.getSourceResultsCd();

            /* Magnitude */
            this->next_plot();
            sendPlotData(x_simvals, res, analysis_scale == LOG_SCALE, true);
            setPlotOptions(analysis_type, analysis_scale, sweep_source, true, true);
            finalize(plotsources);

            /* Phase */
            this->next_plot();
            sendPlotData(x_simvals, res, false, false);
            setPlotOptions(analysis_type, analysis_scale, sweep_source, true, false);
            finalize(plotsources);
        }

        /* Send */
        if(plotnodes.size())
        {
            auto &res = simulator_manager.getNodesResultsCd();

            /* Magnitude */
            this->next_plot();
            sendPlotData(x_simvals, res, analysis_scale == LOG_SCALE, true);
            setPlotOptions(analysis_type, analysis_scale, sweep_source, false, true);
            finalize(plotnodes);

            /* Phase */
            this->next_plot();
            sendPlotData(x_simvals, res, false, false);
            setPlotOptions(analysis_type, analysis_scale, sweep_source, false, false);
            finalize(plotnodes);
        }
	}
}

/*!
    @brief      Prints the data (PRINT or PLOT cards) to standard output, only for OP analysis.
    @param      circuit_manager     The simulated circuit.
    @param      simulator_manager   The simulator engines, containing the results of the simulation.
*/
void print_cout(Circuit &circuit_manager, simulator_engine &simulator_manager)
{
    using std::vector;
    using std::cout;

    auto &plotsources = circuit_manager.PlotSources();
    auto &plotnodes = circuit_manager.PlotNodes();

    cout << "***PLOT - RESULTS***\n";

    /* Send */
    if(plotsources.size())
    {
        cout << "Branch currents:\n";

        auto &res = simulator_manager.getSourceResults();
        auto &tmp = res.front();

        for(size_t i = 0; i < tmp.size(); i++)
        {
            cout << "\t" << plotsources[i] << ": "  << tmp[i] << "\n";
        }
    }

    /* Send */
    if(plotnodes.size())
    {
        cout << "Node Voltages:\n";

        auto &res = simulator_manager.getNodesResults();
        auto &tmp = res.front();

        for(size_t i = 0; i < tmp.size(); i++)
        {
            cout << "\t" << plotnodes[i] << ": "  << tmp[i] << "\n";
        }
    }
}

/*!
    @brief      Wrapper that calls the appropriate routines for plotting of data.
    @param      circuit_manager     The simulated circuit.
    @param      simulator_manager   The simulator engines, containing the results of the simulation.
*/
return_codes_e plot(Circuit &circuit_manager, simulator_engine &simulator_manager)
{
    using std::streamsize;
    using std::cout;
    using std::numeric_limits;

    /* Checks */
    if(!circuit_manager.valid()) return FAIL_PLOTTER_CIRCUIT_INVALID;
    if(!simulator_manager.valid()) return FAIL_PLOTTER_RESULTS_INVALID;
    if(!circuit_manager.PlotNodes().size() && !circuit_manager.PlotSources().size())
        return FAIL_PLOTTER_NOTHING_TO_PLOT;

    /* Set precision */
    streamsize cout_stream_sz = cout.precision(numeric_limits<double>::digits10 + 2);

    /* OP analysis needs only printing of the values */
    if(circuit_manager.AnalysisType() != OP)
    {
        GNU_plotter plotter;
        plotter.plot(circuit_manager, simulator_manager);
    }
    else
    {
        print_cout(circuit_manager, simulator_manager);
    }

    /* Reset precision */
    cout.precision(cout_stream_sz);

    return RETURN_SUCCESS;
}
