#ifndef __SIMULATOR_TYPES_H
#define __SIMULATOR_TYPES_H

/* A global table containing all the error codes used in the program
 * They are divided by their sub-classes in the comments below */
typedef enum return_enum_codes
{
    /* General opcodes - Used inside the bspice.cpp */
    RETURN_SUCCESS = 0,
    FAIL_ARG_NUM = 1,

    /* Parser opcodes - Used inside circuit.cpp/parser.cpp */
    FAIL_LOADING_FILE = 2,
    FAIL_PARSER_INVALID_FORMAT = 3,
    FAIL_PARSER_ELEMENT_EXISTS = 4,
	FAIL_PARSER_ELEMENT_NOT_EXISTS = 5,
    FAIL_PARSER_UNKNOWN_ELEMENT = 6,
    FAIL_PARSER_SHORTED_ELEMENT = 7,
	FAIL_PARSER_UNKNOWN_SPICE_CARD = 8,
	FAIL_PARSER_SOURCE_SPEC_ARGS = 9,
	FAIL_PARSER_SOURCE_SPEC_ARGS_NUM = 10,
	FAIL_PARSER_SOURCE_SPEC_ARGS_FORMAT = 11,
	FAIL_PARSER_ANALYSIS_INVALID_ARGS = 12,
	FAIL_PARSER_UKNOWN_OPTION_OR_REPETITION = 13,

	/* Simulator engine opcodes - Used inside mna/sim_engine.cpp */
	FAIL_SIMULATOR_RUN = 14,
	FAIL_SIMULATOR_EMPTY = 15,
	FAIL_SIMULATOR_FACTORIZATION = 16,
	FAIL_SIMULATOR_SOLVE = 17,

	/* Plotter engine opcodes - Used inside plot.cpp */
	FAIL_PLOTTER_CIRCUIT_INVALID = 18,
	FAIL_PLOTTER_RESULTS_INVALID = 19,
	FAIL_PLOTTER_NOTHING_TO_PLOT = 20,
	FAIL_PLOTTER_IO_OPERATIONS = 21,

	/* GUI connective opcodes */
    FAIL_PARSER_EMPTY_COMMAND_GUI = 22,
    FAIL_GUI_IS_NOT_AVAILABLE = 23,
    FAIL_GUI_NO_CIRC_LOADED = 24,
    FAIL_GUI_NO_SIM_LOADED = 25,

	/* Debug codes, invisible to the user */
    FAIL_SIMULATOR_FALLTHROUTH_ODE_OPTION = 26,          /* Somehow an uknown enum was passed to the sim_engine */
} return_codes_e;

/* A global table containing all the SPICE cards supported by the simulator */
typedef enum analysis_types
{
    OP = 0,
    DC,
    TRAN,
    AC,
    PLOT,
} analysis_t;

/* The scales used for analysis */
typedef enum analysis_scale_types
{
    DEC_SCALE = 0,
    LOG_SCALE,
} as_scale_t;

/* The different transient sources */
typedef enum transient_sources
{
    CONSTANT_SOURCE = 0,
    EXP_SOURCE,
    SINE_SOURCE,
    PWL_SOURCE,
    PULSE_SOURCE,
} tran_source_t;

/* The different ODE solver methods */
typedef enum ODE_methods
{
    BACKWARDS_EULER = 0,
    TRAPEZOIDAL,
    GEAR2,
} ODE_meth_t;

/* TODO - More C++ way of defining it */
#define TRANSIENT_SOURCE_TYPENUM 5

#endif // __SIMULATOR_TYPES_H //
