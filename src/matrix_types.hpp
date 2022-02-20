#ifndef __MATRIX_TYPES_H
#define __MATRIX_TYPES_H

/* For the types used for the simulation */
#include <Eigen>
#include <vector>
#include "base_types.hpp"

/* Triplet - Intermediate to sparse */
typedef Eigen::Triplet<double, IntTp> triplet_eig_d;
typedef std::vector<triplet_eig_d> tripletList_d;
typedef Eigen::Triplet<std::complex<double>, IntTp> triplet_eig_cd;
typedef std::vector<triplet_eig_cd> tripletList_cd;

/* Sparse Matrices */
typedef Eigen::SparseMatrix<double, Eigen::ColMajor, IntTp> SparMatD;
typedef Eigen::SparseMatrix<std::complex<double>, Eigen::ColMajor, IntTp> SparMatCompD;

/* Dense Matrices */
typedef Eigen::MatrixXd DenseMatD;
typedef Eigen::MatrixXcd DenseMatCompD;

/* Vectors */
typedef Eigen::VectorXd DensVecD;
typedef Eigen::VectorXcd DensVecCompD;

#endif // __MATRIX_TYPES_H //
