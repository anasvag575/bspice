#ifndef __SIMULATOR_TYPES_H
#define __SIMULATOR_TYPES_H

/* A global table containing all the error codes used in the program
 * They are divided by their sub-classes in the comments below */
typedef enum return_enum_codes
{
    /* General opcodes - Used inside the bspice.cpp */
    RETURN_SUCCESS = 0,
    FAIL_ARG_NUM,
    FAIL_INPUT_FILE,

    /* Parser opcodes - Used inside circuit.cpp */
    FAIL_LOADING_FILE,
    FAIL_PARSER_INVALID_FORMAT,
    FAIL_PARSER_ELEMENT_EXISTS,
	FAIL_PARSER_ELEMENT_NOT_EXISTS,
    FAIL_PARSER_UKNOWN_ELEMENT,
    FAIL_PARSER_SHORTED_ELEMENT,
	FAIL_PARSER_UKNOWN_SPICE_CARD,
	FAIL_PARSER_SOURCE_SPEC_ARGS,
	FAIL_PARSER_ANALYSIS_INVALID_ARGS,

	/* Simulator engine opcodes - Used inside mna/sim_engine.cpp */

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

#endif // __SIMULATOR_TYPES_H //
