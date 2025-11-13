/****************************************************************************

    flow5
    Copyright (C) 2025 André Deperrois 
    
    This file is part of flow5.

    flow5 is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    flow5 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flow5.
    If not, see <https://www.gnu.org/licenses/>.


*****************************************************************************/


#pragma once

#include <QVector>

#include <complex>
#include <vector>


/** Data structure for multithreaded matrix multiplication */
struct MatMultData
{
    MatMultData(double* _A, double* _B, double* _AB, int _nRows, int _n, int _nCols)
    {
        A     = _A;
        B     = _B;
        AB    = _AB;
        nRows = _nRows;
        n     = _n;
        nCols = _nCols;
    }

    double* A=nullptr;
    double* B=nullptr;
    double* AB=nullptr;
    int nRows=0;
    int n=0;
    int nCols=0;
};

/** Inverts in place a 2x2 matrix */
bool  invert22(double *l);
/** Inverts a 2x2 matrix */
bool  invert22(double *in, double *out);
/** multiplies two 2x2 matrices */
void  matMult22(double *a, double *b, double *ab);

/** Transposes in place a 3x3 matrix */
void  transpose33(double *l);
/** Inverts in place a 3x3 matrix */
bool  invert33(double *l);
/** Inverts a 3x3 matrix */
bool  invert33(double *in, double *out);

void  AV33(double *A, double *v, double *p);

/** multiplies two 3x3 matrices */
void  matMult33(double *a, double *b, double *ab);

float   det33(float *M);
double  det33(double *aij);
std::complex<double>  det33(std::complex<double> *aij);


/** Transposes in place a 4x4 matrix */
void  transpose44f(float *l);

/** Transposes a 4x4 matrix */
void  transpose44f(const float *in, float *out);


double  det44(double *aij);


std::complex<double>  det44(std::complex<double> *aij);

float   cofactor44(float *aij, int i, int j);
double  cofactor44(double *aij, int i, int j);
std::complex<double>  cofactor44(std::complex<double> *aij, int i, int j);

bool  invert44(double *l);
bool  invert44(double *ain, double *aout);
bool  invert44(std::complex<double> *ain, std::complex<double> *aout);

bool  invertMatrix(QVector<double> &mat, int N);


/**
 * Solves a linear system with a tridiagonal block matrix using Thomas algorithm
 * n = size of A,B,C, and R vectors                     n>=1
 * p = dimension of Ai, Bi, Ci matrices and Ri vector   p>=1
 *
 *   | B0 C0                          |   | X0 |    | R0 |
 *   | A1 B1 C1                       |   |    |    |    |
 *   |    A2 B2 C2                    |   |    |    |    |
 *   |                                | . |    | =  |    |
 *   |              ...               |   |    |    |    |
 *   |                                |   |    |    |    |
 *   |                 An-2 Bn-2 Cn-2 |   |    |    |    |
 *   |                      An-1 Bn-1 |   |Xn-1|    |Rn-1|
*/
bool  blockThomasSolve(QVector<QVector<double> > &A, QVector<QVector<double> > &B, QVector<QVector<double> > &C, QVector<QVector<double> > &R, QVector<QVector<double> > &X, int n, int p);


bool  Gauss(double *A, int n, double *B, int m, bool &bCancel);
bool  GaussNoPivot(double* A, int n, double* RHS, double * x);

bool  eigenVector(double a[][4], std::complex<double> lambda, std::complex<double> *V);

void  display_vec(double *vec, int n);
void  display_vec(float *vec, int n, bool bHorizontal=false);

void  display_mat(double const *mat, int rows, int cols=-1);
void  display_mat(float const*mat, int rows, int cols=-1);
void  display_mat(std::vector<std::vector<double>> const &mat);
void  display_mat(QVector<QVector<double>> const &mat);
void  display_mat(QVector<double> const &mat, int size);

void  saveMatrixtoFile(std::vector<double> const &mat, int N);

void  listArray(double *array, int size);

void  listArray(std::vector<double> const &array);
void  listArrays(std::vector<double> const &array1, std::vector<double> const &array2);
void  listArrays(std::vector<double> const &array1, std::vector<double> const &array2, std::vector<double> const &array3);

void  listArray(QVector<double> const &array);
void  listArrays(QVector<double> const &array1, QVector<double> const &array2);
void  listArrays(QVector<float> const &array1, QVector<float> const &array2);

void  matMult_SingleThread(double * const A, double * const B, double* AB, int m, int n, int q);
void  matMult(double* const A, double* const B, double* AB, int n, int nThreads=-1);
void  matMult(double* const A, double* const B, double* AB, int nRows, int n, int nCols, int nThreads=-1);
void  matMultBlock(int iBlock, MatMultData &mmd, int nThreads);
void  matMultLAPACK(double const *A, double const *B, double* AB, int m, int k, int n, int nThreads=-1);


void  CholevskiFactor(double* A, double* L, int n);
bool  CholevskiSolve(const double *A, double* b, int n);

void  matVecMultLapack(const float *A, const float *X, float* Y, int n, int m, int nThreads=-1);
void  matVecMultLapack(const double *A, const double *X, double* Y, int m, int n, int nThreads=-1);
void  matVecMult(double const *A, double const *V, double *res, int nRows, int nCols);
void  matVecMult(double const *A, double const *X, double *Y, int n);
void  matVecMult2x2(const double *A, const double *X, double* Y);
void  matVecMult3x3(const double *A, const double *X, double* Y);
void  matVecMult4x4(const double *A, const double *X, double* Y);

void  rowToColumnMajorIndex(int n, double *aij, double  *taij);
void  columnToRowMajorIndex(int n, double *aij, double  *taij);
bool  inverseMatrix_old(int n, double *aij, double *invAij);
bool  inverseMatrixLapack(int n, QVector<double> &aij, int nThreads=-1);

void  print_matrix_colmajor(const char* desc, int m, int n, double* a, int lda );


double  determinant(double *mat, int n);

bool  solve2x2(double *M, double *rhs, int nrhs);
bool  solve3x3(double *M, double *rhs, int nrhs);
bool  solve4x4(double *M, double *rhs, int nrhs);

int  solveLinearSystem(int rank, float *M, int nrhs, float *rhs, int nThreads=-1);
int  solveLinearSystem(int rank, double *M, int nrhs, double *rhs, int nThreads=-1);

void setIdentityMatrix(double *M, int n);

bool makeILUC(double const*A, double *ILU, int n, int p);
bool makeSGS(double const *A, double *SGSLU, int n);
void ILUC(double const*A, double *ILU, int n, int p);
