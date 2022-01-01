#include <iostream>
#include <fstream>

#include "bspice.hpp"
#include "circuit.hpp"
#include "sim_engine.hpp"
#include "plot.hpp"

int main(int argc, char **argv)
{
    using namespace std;
    const string syntax = "./v2b_simulator <filename>";

    /* Check for valid number of input arguments */
    if (argc != 2)
    {
        cout << "[ERROR - " << FAIL_ARG_NUM << "]: Invalid number of input arguments." << endl;
        cout << "[ERROR - " << FAIL_ARG_NUM << "]: Syntax is as follows => " << syntax << endl;
        return FAIL_ARG_NUM;
    }

    /* Step 1 - Open file */
    string input_file_name = argv[1];
    fstream input_file(input_file_name, ios::in);

    /* Failure - Error and return */
    if(!input_file)
    {
        cout << "[ERROR - " << FAIL_INPUT_FILE << "]: Unable to open input file <" << input_file_name << ">." <<endl;
        return FAIL_INPUT_FILE;
    }

    /* Step 2 - Instantiate a circuit and parse the file */
    Circuit circuit_manager;
    if(circuit_manager.CreateCircuit(input_file) != RETURN_SUCCESS)
    {
        input_file.close();
        cout << "[ERROR - " << FAIL_LOADING_FILE << "]: Unable to load input file <" << input_file_name << ">." <<endl;
        return FAIL_LOADING_FILE;
    }

    /* Close the netlist file */
    input_file.close();

    /* Step 3 - Proceed to the simulator engine */
    simulator_engine sim_manager(circuit_manager);
    sim_manager.run(circuit_manager);

    /* TODO - Leave Plot as is for now */
    /* Step 4 - Output the results either by plotting or printing to a file */
//    plot_rand();

    return 1;
}
