#ifndef __SIMULATOR_TYPES_H
#define __SIMULATOR_TYPES_H

/** Enumeration containing all the error codes used in the program. */
typedef enum return_enum_codes
{
    RETURN_SUCCESS = 0,                             //!< Success opcode.
    FAIL_ARG_NUM = 1,                               //!< Wrong number of arguments.
    FAIL_LOADING_FILE = 2,                          //!< Failure in opening the input netlist file.
    FAIL_PARSER_INVALID_FORMAT = 3,                 //!< Invalid format in a SPICE element/card.
    FAIL_PARSER_ELEMENT_EXISTS = 4,                 //!< Element already exists (redefinition).
	FAIL_PARSER_ELEMENT_NOT_EXISTS = 5,             //!< Element does not exist (dependency of element/card).
    FAIL_PARSER_UNKNOWN_ELEMENT = 6,                //!< Unknown type of element.
    FAIL_PARSER_SHORTED_ELEMENT = 7,                //!< Shorted element.
	FAIL_PARSER_UNKNOWN_SPICE_CARD = 8,             //!< Unknown type of card.
	FAIL_PARSER_SOURCE_SPEC_ARGS = 9,               //!< Source spec unknown/invalid arguments.
	FAIL_PARSER_SOURCE_SPEC_ARGS_NUM = 10,          //!< Source spec wrong number of arguments.
	FAIL_PARSER_SOURCE_SPEC_ARGS_FORMAT = 11,       //!< Source spec invalid format.
	FAIL_PARSER_ANALYSIS_INVALID_ARGS = 12,         //!< Invalid conditions on analysis SPICE cards (e.g., step).
	FAIL_PARSER_UKNOWN_OPTION_OR_REPETITION = 13,   //!< Unknown option or repetition during OPTIONS card evaluation.

	//TODO
	FAIL_PARSER_AC_SPEC_NEG = 22,                   //!< AC spec is invalid (negative magnitude).

	FAIL_SIMULATOR_RUN = 14,                        //!< Failure during the simulator's run.
	FAIL_SIMULATOR_EMPTY = 15,                      //!< Empty results (simulator results).
	FAIL_SIMULATOR_FACTORIZATION = 16,              //!< Failure during the simulator's factorization step.
	FAIL_SIMULATOR_SOLVE = 17,                      //!< Failure during the simulator's solve step.
	FAIL_PLOTTER_IO_OPERATIONS = 21,                //!< Failure during plotter's IO operation (pipe/file).
    FAIL_SIMULATOR_FALLTHROUTH_ODE_OPTION = 23,     //!< Unknown ODE option (debug only).
} return_codes_e;

/** Enumeration containing all the SPICE cards supported by the simulator. */
typedef enum analysis_types
{
    OP = 0,             //!< Operating point analysis.
    DC,                 //!< Direct current analysis.
    TRAN,               //!< Transient analysis.
    AC,                 //!< Alternating current analysis.
    PLOT,               //!< Plot SPICE command.
} analysis_t;

/** Enumeration for the scales used for analysis. */
typedef enum analysis_scale_types
{
    DEC_SCALE = 0,      //!< Decimal scale.
    LOG_SCALE,          //!< Logarithmic scale.
} as_scale_t;

/** Enumeration for the different transient sources. */
typedef enum transient_sources
{
    CONSTANT_SOURCE = 0,    //!< Constant source (DC).
    EXP_SOURCE,             //!< Exponential source (TRAN).
    SINE_SOURCE,            //!< Sinusoidal source (TRAN).
    PWL_SOURCE,             //!< Piece-Wise Linear source (TRAN).
    PULSE_SOURCE,           //!< Pulse source (TRAN).
} tran_source_t;

/** Enumeration for the different ODE solver methods. */
typedef enum ODE_methods
{
    BACKWARDS_EULER = 0,    //!< Backwards Euler differentiation method.
    TRAPEZOIDAL,            //!< Trapezoidal differentiation method.
    GEAR2,                  //!< Gear 2 differentiation method.
} ODE_meth_t;

/* TODO - More C++ way of defining it */
#define TRANSIENT_SOURCE_TYPENUM 5

#endif // __SIMULATOR_TYPES_H //
