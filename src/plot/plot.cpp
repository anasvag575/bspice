#include <fstream>
#include <iomanip>
#include <cstdio>
#include <limits>
#include <math.h>
#include <unistd.h>
#include "plot.hpp"

//! Class that sets up a connection with GNUPLOT for plotting.
/*!
  Supporting class used only for organization purposes and safety reasons
  (exception handling). The methods provided:
  - Connect with Gnuplot utility and manage communication.
  - Create and manage different plot windows.
  - Configure the plots/legends.
*/
class GNU_plotter
{

    public:
        GNU_plotter();
        ~GNU_plotter();
        void plot(circuit &circuit_manager, simulator &simulator_manager);

    private:
        void nextPlot(void);
        void setPlotOptions(analysis_t type, as_scale_t scale, std::string &sweep, bool source, bool mag);
        void sendPlotData(const std::vector<double> &xvals,
                          const std::vector<std::vector<double>> &yvals,
                          bool log);
        void sendPlotData(const std::vector<double> &xvals,
                          const std::vector<std::vector<std::complex<double>>> &yvals,
                          bool mag, bool log);
        void finalize(const std::vector<std::string> &plotnames);

        FILE *_pipe;                            //! File handler for the GNUPLOT sub-process pipe.
        std::ofstream _data_file;               //! The active output data file.
        std::vector<std::string> _file_names;   //! The vector of all the file names used.
};

/*!
    @brief    Default constructor, creates the GNUPLOT pipe.
*/
GNU_plotter::GNU_plotter()
{
    _pipe = popen("gnuplot -persistent", "w");

    // TODO - Handle exceptions?
    if(!this->_pipe)
    {
        throw std::runtime_error("[CRITICAL ERROR]: Could not open files related to GNUPLOT");
    }
}

/*!
    @brief    Default destructor, closes GNUPLOT pipe and removes the files created.
*/
GNU_plotter::~GNU_plotter()
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
void GNU_plotter::nextPlot()
{
    /* Close previous file */
    if(this->_data_file) this->_data_file.close();

    /* Ascending number */
    std::string filename = ".gnuplotdata" + std::to_string(_file_names.size()) +  ".plt";
    _file_names.push_back(filename);

    fprintf(this->_pipe, "set term qt %ld\n", _file_names.size() - 1);

    /* Create the new file */
    _data_file = std::ofstream(filename, std::ios::out);

    // TODO - Handle
    if(!this->_data_file)
    {
        throw std::runtime_error("[CRITICAL ERROR]: Could not open files related to GNUPLOT");
    }
}

/*!
    @brief      Sends the final plot command to GNUPLOT to display the graph.
    @param      plotnames     The legend names for the plots.
*/
void GNU_plotter::finalize(const std::vector<std::string> &plotnames)
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
    @param      log       Whether the data is in logarithmic scale or not.
*/
void GNU_plotter::sendPlotData(const std::vector<double> &xvals, const std::vector<std::vector<double>> &yvals, bool log)
{
	for(size_t i = 0; i < xvals.size(); i++)
	{
	    auto &tmp = yvals[i];

		/* First iteration send also the x value */
		this->_data_file << xvals[i];

		/* Create the columns of data */
		if(log)
		{
		    for(size_t k = 0; k < tmp.size(); k++)
		        this->_data_file << "\t" << 20 * std::log10(tmp[k]);
		}
		else
		{
		    for(size_t k = 0; k < tmp.size(); k++)
		        this->_data_file << "\t" << tmp[k];
		}

		/* Next sim time */
		this->_data_file << "\n";
	}

	/* Endl also performs a flush */
	this->_data_file << std::endl;
}

/*!
    @brief      Sends the data that will be plotted on the current window, for AC analysis only.
    @param      xvals     The x values vector, simulation vector.
    @param      yvals     The y values vector(s), result(s) vector.
    @param      mag       Whether to send magnitude data or phase.
    @param      log       Whether the data is in logarithmic scale or not.
*/
void GNU_plotter::sendPlotData(const std::vector<double> &xvals, const std::vector<std::vector<std::complex<double>>> &yvals, bool log, bool mag)
{
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
                for(size_t k = 0; k < tmp.size(); k++)
                    this->_data_file << "\t" << 20 * std::log10(std::abs(tmp[k]));
            }
            else
            {
                for(size_t k = 0; k < tmp.size(); k++)
                    this->_data_file << "\t" << std::abs(tmp[k]);
            }
        }
        else
        {
            /* Arg returns in radians convert to degrees */
            for(size_t k = 0; k < tmp.size(); k++)
                this->_data_file << "\t" << std::arg(tmp[k]) * 180/M_PI;
        }

        /* Next sim time */
        this->_data_file << "\n";
    }

    /* Endl also performs a flush */
    this->_data_file << std::endl;
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
    @param      simulator_manager   The simulator engine.
*/
void GNU_plotter::plot(circuit &circuit_manager, simulator &simulator_manager)
{
	auto &plotsources = circuit_manager.PlotSources();
	auto &plotnodes = circuit_manager.PlotNodes();

	auto analysis_type = circuit_manager.AnalysisType();
	auto analysis_scale = circuit_manager.AnalysisScale();
	auto sweep_source = circuit_manager.DCSource();

    /* Get the x-values */
    auto &x_simvals = simulator_manager.SimulationVec();

    /* For TRAN/DC we have only 1 value to print */
	if(analysis_type != AC)
	{
        /* Send */
        if(plotsources.size())
        {
            auto &res = simulator_manager.SourceResults();

            this->nextPlot();
            sendPlotData(x_simvals, res, analysis_scale == LOG_SCALE);
            setPlotOptions(analysis_type, analysis_scale, sweep_source, true, true);
            finalize(plotsources);
        }

        /* Send */
        if(plotnodes.size())
        {
            auto &res = simulator_manager.NodesResults();

            this->nextPlot();
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
            auto &res = simulator_manager.SourceResultsCd();

            /* Magnitude */
            this->nextPlot();
            sendPlotData(x_simvals, res, analysis_scale == LOG_SCALE, true);
            setPlotOptions(analysis_type, analysis_scale, sweep_source, true, true);
            finalize(plotsources);

            /* Phase */
            this->nextPlot();
            sendPlotData(x_simvals, res, false, false);
            setPlotOptions(analysis_type, analysis_scale, sweep_source, true, false);
            finalize(plotsources);
        }

        /* Send */
        if(plotnodes.size())
        {
            auto &res = simulator_manager.NodesResultsCd();

            /* Magnitude */
            this->nextPlot();
            sendPlotData(x_simvals, res, analysis_scale == LOG_SCALE, true);
            setPlotOptions(analysis_type, analysis_scale, sweep_source, false, true);
            finalize(plotnodes);

            /* Phase */
            this->nextPlot();
            sendPlotData(x_simvals, res, false, false);
            setPlotOptions(analysis_type, analysis_scale, sweep_source, false, false);
            finalize(plotnodes);
        }
	}
}

/*!
    @brief      Prints the data (PRINT or PLOT cards) to standard output, only for OP analysis.
    @param      circuit_manager     The simulated circuit.
    @param      simulator_manager   The simulator engine.
*/
void print_cout(circuit &circuit_manager, simulator &simulator_manager)
{
    auto &plotsources = circuit_manager.PlotSources();
    auto &plotnodes = circuit_manager.PlotNodes();

    std::cout << "***PLOT - RESULTS***\n";

    /* Send */
    if(plotsources.size())
    {
        std::cout << "Branch currents:\n";

        auto &res = simulator_manager.SourceResults();
        auto &tmp = res.front();

        for(size_t i = 0; i < tmp.size(); i++)
        {
            std::cout << "\t" << plotsources[i] << ": "  << tmp[i] << "\n";
        }
    }

    /* Send */
    if(plotnodes.size())
    {
        std::cout << "Node Voltages:\n";

        auto &res = simulator_manager.NodesResults();
        auto &tmp = res.front();

        for(size_t i = 0; i < tmp.size(); i++)
        {
            std::cout << "\t" << plotnodes[i] << ": "  << tmp[i] << "\n";
        }
    }
}

/*!
    @brief      Wrapper that calls the appropriate routines for plotting of data.
    @param      circuit_manager     The simulated circuit.
    @param      simulator_manager   The simulator engine, containing the results of the simulation.
    @return     The error code, in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e plot(circuit &circuit_manager, simulator &simulator_manager)
{
    /* Set precision */
    std::streamsize cout_stream_sz = std::cout.precision(std::numeric_limits<double>::digits10 + 2);

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
    std::cout.precision(cout_stream_sz);

    return RETURN_SUCCESS;
}
