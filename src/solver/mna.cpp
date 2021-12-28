#include "mna.hpp"

void plot_rand(void)
{
	std::cout << "Hello from PLOT" << std::endl;

	Gnuplot gp;

	/** CUSTOM OPTIONS FOR GNUPLOT **/
//	gp << "set xrange [-2:2]\nset yrange [-2:2]\n"; /* Setting specific range for x or y axsi*/
	gp << "set title 'Simulation results'\n" << std::endl;		/* Base title */
	gp << "set time\n" << std::endl;

	// Create a script which can be manually fed into gnuplot later:
	//    Gnuplot gp(">script.gp");
	// Create script and also feed to gnuplot:
	//    Gnuplot gp("tee plot.gp | gnuplot -persist");
	// Or choose any of those options at runtime by setting the GNUPLOT_IOSTREAM_CMD
	// environment variable.

	// Gnuplot points require two columns: (x,y)
	std::vector<boost::tuple<double, double>> pts_A;

	for(size_t i = 0; i < 100; i++)
	{
		pts_A.push_back(boost::make_tuple(i, i * 100));
	}

	// Don't forget to put "\n" at the end of each line!
	// '-' means read from stdin.  The send1d() function sends data to gnuplot's stdin.

//    gp << "set output 'Currents'\n";
	gp << "plot '-' with points title 'pts_A'\n";
	gp.send1d(pts_A);
}

/* Set the default options for a plot window */
void SetPlotWindowOptions(void)
{

}

