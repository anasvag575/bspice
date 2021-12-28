#ifndef __SIMULATOR_CODES_H
#define __SIMULATOR_CODES_H

/* For the types used for the simulation */
#include <Eigen>
#include <vector>

/* Triplet - Intermediate to sparse */
typedef Eigen::Triplet<double, long int> triplet_eig;
typedef std::vector<triplet_eig> tripletList;

/* Sparse Matrices */
typedef Eigen::SparseMatrix<double, Eigen::ColMajor, long int> SparMatD;
typedef Eigen::SparseMatrix<std::complex<double>, Eigen::ColMajor, long int> SparMatCompD;

/* Dense Matrices */
typedef Eigen::MatrixXd DenseMatD;
typedef Eigen::MatrixXcd DenseMatCompD;

/* Vectors */
typedef Eigen::VectorXd DensVecD;

/* A global table containing all the error codes used in the program
 * They are divided by their sub-classes in the comments below */
typedef enum return_enum_codes
{
    /* General opcodes - Used inside the v2b_simulator.cpp */
    RETURN_SUCCESS = 0,
    FAIL_ARG_NUM,
    FAIL_INPUT_FILE,

    /* Parser opcodes - Used inside the circuit.cpp */
    FAIL_LOADING_FILE,
    FAIL_PARSER_INVALID_FORMAT,
    FAIL_PARSER_ELEMENT_EXISTS,
	FAIL_PARSER_ELEMENT_NOT_EXISTS,
    FAIL_PARSER_UKNOWN_ELEMENT,
    FAIL_PARSER_SHORTED_ELEMENT,
	FAIL_PARSER_UKNOWN_SPICE_CARD,
	FAIL_PARSER_ANALYSIS_INVALID_ARGS,

} return_codes_e;

/* A global table containing all the SPICE cards supported by the simulator */
typedef enum analysis_types
{
    OP,
    DC,
    TRAN,
    AC,
    PLOT,
} analysis_t;

/* The scales used for different analysis */
typedef enum analysis_scale_types
{
    DEC_SCALE,
    LOG_SCALE,
} as_scale_t;

/* The scales used for different analysis */
typedef enum transient_sources
{
    CONSTANT,
    EXP,
    SINE,
    PWL,
    PULSE,
} tran_source_t;

#endif // __SIMULATOR_CODES_H //
