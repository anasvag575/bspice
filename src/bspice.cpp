#include "bspice.hpp"
#include "circuit.hpp"
#include "sim_engine.hpp"
#include "plot.hpp"

/*!
    @brief      Reports the potential errors using the error code returns,
    during the simulator run.
    @param      errcode  The error code in question.
    @return     The string to print to output
*/
static std::string bspice_error_report(return_codes_e errcode)
{
    std::string ret_str = "[ERROR - " + std::to_string(errcode) + "]: ";

    switch(errcode)
    {
        case RETURN_SUCCESS: ret_str = ""; break;
        case FAIL_ARG_NUM: ret_str += "Invalid number of input arguments. Syntax is as follows => ./bspice <filename>"; break;

        /* Parser */
        case FAIL_LOADING_FILE: ret_str += "Unable to open input file"; break;
        case FAIL_PARSER_INVALID_FORMAT: ret_str += "Invalid specification in spice netlist."; break;
        case FAIL_PARSER_ELEMENT_EXISTS: ret_str += "Element already exists in spice netlist (SPICE element assertion)."; break;
        case FAIL_PARSER_ELEMENT_NOT_EXISTS: ret_str += "Element does not exist in spice netlist (SPICE card assertion)."; break;
        case FAIL_PARSER_UNKNOWN_ELEMENT: ret_str += "Element type is not supported by the simulator."; break;
        case FAIL_PARSER_SHORTED_ELEMENT: ret_str += "Element is shorted, 2 or more nodes are the same."; break;
        case FAIL_PARSER_UNKNOWN_SPICE_CARD: ret_str += "Uknown spice card option."; break;
        case FAIL_PARSER_SOURCE_SPEC_ARGS: ret_str += "Element source spec uknown or syntax failure."; break;
        case FAIL_PARSER_SOURCE_SPEC_ARGS_NUM: ret_str += "Element source spec number of arguments failure."; break;
        case FAIL_PARSER_SOURCE_SPEC_ARGS_FORMAT: ret_str += "Element source spec syntax failure."; break;
        case FAIL_PARSER_ANALYSIS_INVALID_ARGS: ret_str += "SPICE card invalid arguments or syntax."; break;
        case FAIL_PARSER_UKNOWN_OPTION_OR_REPETITION: ret_str += "SPICE card (.OPTION) uknown option or reinstatiation"; break;

        /* Simulator engine opcodes - Used inside mna/sim_engine.cpp */
        case FAIL_SIMULATOR_RUN: ret_str += "Failure during simulation run."; break;
        case FAIL_SIMULATOR_EMPTY: ret_str += "No circuit is has been loaded, can't run empty simulation."; break;
        case FAIL_SIMULATOR_FACTORIZATION: ret_str += "Failure during factorization (Singular matrix)."; break;
        case FAIL_SIMULATOR_SOLVE: ret_str += "Failure during backwards solving (Solve failure)."; break;

        /* Plotter engine opcodes - Used inside plot.cpp */
        case FAIL_PLOTTER_IO_OPERATIONS: ret_str += "Failure in opening plot necessary plot I/O."; break;

        /* Debug codes, invisible to the user */
        case FAIL_SIMULATOR_FALLTHROUTH_ODE_OPTION: ret_str += "ODE failure, not a known ODE Method."; break;
        default: ret_str = "UKNOWN ERROR"; break; // Nothing, can't reach this point
    }

    return ret_str;
}

/*!
    @brief      The entire simulation run, non-interactive.
    @param      argc The command line process's number of arguments.
    @param      argv The command line process's arguments vector.
    @return     Error code in case of error, otherwise RETURN_SUCESS.
*/
static return_codes_e bspice_single_run(int argc, char **argv)
{
    return_codes_e errcode = RETURN_SUCCESS;
    std::string input_file_name(argv[1]);

    /* Step 2 - Instantiate a circuit */
    circuit circuit_manager(input_file_name);
    errcode = circuit_manager.errcode();
    if(errcode != RETURN_SUCCESS) return errcode;

    /* Step 3 - Proceed to the simulator engine */
    simulator sim_manager(circuit_manager);
    errcode = sim_manager.run();
    if(errcode != RETURN_SUCCESS) return errcode;

    /* Step 4 - Output the results */
    return plot(circuit_manager, sim_manager);
}

/*!
    @brief      The program entry point.
    @param      argc The command line process's number of arguments.
    @param      argv The command line process's arguments vector.
    @return     Error code in case of error, otherwise RETURN_SUCESS.
*/
int main(int argc, char **argv)
{
    /* Step 1 - Check for valid number of input arguments */
    if (argc != 2)
    {
        std::cout << bspice_error_report(FAIL_ARG_NUM) << std::endl;
        return FAIL_ARG_NUM;
    }

    /* Enter BSPICE */
    return_codes_e err = bspice_single_run(argc, argv);
    std::cout << bspice_error_report(err) << std::endl;

    return err;
}
