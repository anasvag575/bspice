#ifndef __MATRIX_TYPES_H
#define __MATRIX_TYPES_H

#include <Eigen>
#include <vector>
#include "base_types.hpp"

/** Eigen Triplet structure (Reals). */
typedef Eigen::Triplet<double, IntTp> triplet_eig_d;

/** Eigen Triplet Vector (Reals). */
typedef std::vector<triplet_eig_d> tripletList_d;

/** Eigen Triplet structure (Complex). */
typedef Eigen::Triplet<std::complex<double>, IntTp> triplet_eig_cd;

/** Eigen Triplet Vector (Complex). */
typedef std::vector<triplet_eig_cd> tripletList_cd;

/** Eigen sparse matrix of unknown size (Reals). */
typedef Eigen::SparseMatrix<double, Eigen::ColMajor, IntTp> SparMatD;

/** Eigen sparse matrix of unknown size (Complex). */
typedef Eigen::SparseMatrix<std::complex<double>, Eigen::ColMajor, IntTp> SparMatCompD;

/** Eigen dense matrix of unknown size (Reals). */
typedef Eigen::MatrixXd DenseMatD;

/** Eigen dense matrix of unknown size (Complex). */
typedef Eigen::MatrixXcd DenseMatCompD;

/** Eigen dense vector of unknown size (Reals). */
typedef Eigen::VectorXd DensVecD;

/** Eigen dense vector of unknown size (Complex). */
typedef Eigen::VectorXcd DensVecCompD;

#endif // __MATRIX_TYPES_H //
