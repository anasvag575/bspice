#include <iostream>
#include <fstream>

#include "bspice.hpp"
#include "circuit.hpp"
#include "sim_engine.hpp"
#include "plot.hpp"

/* TODO - This is non-interactive execution */
static return_codes_e bspice_single_run(int argc, char **argv)
{
    return_codes_e errcode = RETURN_SUCCESS;
    using namespace std;

    /* Check for valid number of input arguments */
    if (argc != 2)
    {
        cout << "[ERROR - " << FAIL_ARG_NUM << "]: Invalid number of input arguments." << endl;
        cout << "\tSyntax is as follows => " << "./bspice <filename>" << endl;
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

    /* Step 2 - Instantiate a circuit, parse and then close the file */
    Circuit circuit_manager;
    errcode = circuit_manager.createCircuit(input_file);
    input_file.close();

    if(errcode != RETURN_SUCCESS)
    {
        cout << "[ERROR]: Unable to load input file <" << input_file_name << ">." <<endl;
        return FAIL_LOADING_FILE;
    }

    /* Step 3 - Proceed to the simulator engine */
    simulator_engine sim_manager(circuit_manager);
    errcode = sim_manager.run(circuit_manager);

    if(errcode != RETURN_SUCCESS)
    {
        cout << "[ERROR]: Unable to properly simulate circuit." <<endl;
        return FAIL_SIMULATOR_RUN;
    }

    /* Step 4 - Output the results */
    return plot(circuit_manager, sim_manager);
}


int main(int argc, char **argv)
{
//    return_codes_e errcode = RETURN_SUCCESS;

    /* TODO - Only non-interactive run exists now */
    return bspice_single_run(argc, argv);
}


