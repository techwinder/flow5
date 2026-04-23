/****************************************************************************

    flow5 application
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

#include <cmath>
#include <cstring>

#include <qrleastsquares.h>
#include <constants.h>

/**
 * Adapted from:
 * QR Least-Squares Solver with Column Pivoting
 *
 * Note: This code will overwrite the input matrix! Create a copy if needed.
 *
 * Implemented in ANSI C. To compile with gcc: gcc -o qr_fact qr_fact.c -lm
 *
 * Author: Michael Mazack, <mazack @ yahoo . com>
 *
 * Date: April 27th, 2010
 *
 * License: Public Domain. Redistribution and modification without
 * restriction is granted. If you find this code helpful, please let me know.
 */

// USE DGELS instead

typedef unsigned long ulong;

/** For updating the permutation vector in (virtual) column swaps. */
void swapcolumns(int *p, int i, int j)
{
    int temp;
    temp = p[i];
    p[i] = p[j];
    p[j] = temp;
}



/** Backsolving of a trianglular system. */
void back_solve(double const *mat, double const*rhs, int rows, int cols, double *sol, int *p)
{
    int i, j;
    int bottom = 0;
    double sum;

    /* Fill the solution with zeros initially. */
    for(i=0; i<cols; i++) sol[i] = 0.0;

    /* Find the first non-zero row from the bottom and start solving from here. */
    for(i=rows-1; i>=0; i--)
        if(fabs(mat[i*cols + p[cols - 1]]) > PRECISION)
        {
            bottom = i;
            break;
        }

    /* Standard back solving routine starting at the first non-zero diagonal. */
    for(i=bottom; i>=0; i--)
    {
        sum = 0.0;

        for(j=cols-1; j>=0; j--)
            if(j>i)    sum += sol[p[j]]*mat[i*cols + p[j]];

        if(mat[i*cols + p[i]] > PRECISION) sol[p[i]] = (rhs[i] - sum)/mat[i*cols + p[i]];
        else                               sol[p[i]] = 0.0;
    }
}


/** Apply a Householder transform to the matrix at a given spot. */
bool householder(double const*mat, int rows, int cols, int row_pos, int col_pos, double *result)
{
    double norm = 0;
    for(int i=row_pos; i<rows; i++)  norm += mat[i*cols + col_pos]*mat[i*cols + col_pos];

    if(fabs(norm) <PRECISION) return false;

    norm = sqrt(norm);
    result[0] = (mat[row_pos*cols + col_pos] - norm);

    for(int i=1; i<(rows - row_pos); i++) result[i] = mat[(i+row_pos)*cols + col_pos];

    norm = 0;
    for(int i=0; i < (rows-row_pos); i++) norm += result[i]*result[i];

    if(fabs(norm) <PRECISION) return false;

    norm = sqrt(norm);
    for(int i=0; i<(rows-row_pos); i++) result[i] *= (1.0/norm);

    return true;
}


/** Routine for applying the Householder transform to the matrix and the right hand side. */
void apply_householder(double *mat, double *rhs, int rows, int cols, double *house, int row_pos, int *p)
{
    // Get the dimensions for the Q matrix.
    int n = rows - row_pos;

    // Allocate memory.
    double *hhmat = new double[ulong(n*n)];
    double *mat_cpy = new double[ulong(rows*cols)];
    double *rhs_cpy = new double[ulong(rows)];


    // Copy the matrix.
    memcpy(mat_cpy, mat, ulong(rows*cols)*sizeof(double));

    // Copy the right hand side.
    memcpy(rhs_cpy, rhs, ulong(rows)*sizeof(double));

    // Build the Q matrix from the Householder transform.
    for(int j=0; j<n; j++)
        for(int i=0; i<n; i++)
            if(i!=j) hhmat[i*n+j] = -2.0*house[j]*house[i];
            else     hhmat[i*n+j] = 1.0 - 2.0*house[j]*house[i];

    // Multiply by the Q matrix.
    for(int k=0; k<cols; k++)
        for(int j=0; j<n; j++)
        {
            double sum = 0.0;
            for(int i=0; i<n; i++)
            {
                sum += hhmat[j*n+i]*mat_cpy[(i+row_pos)*cols + p[k]];
            }

            mat[(j+row_pos)*cols+p[k]] = sum;
        }

    // Multiply the rhs by the Q matrix.
    for(int j=0; j<n; j++)
    {
        double sum = 0.0;
        for(int i=0; i<n; i++)
            sum += hhmat[i*n+j]*rhs_cpy[i+row_pos];

        rhs[j+row_pos] = sum;
    }

    // Collect garbage.
    delete [] hhmat;
    delete [] mat_cpy;
    delete [] rhs_cpy;
}



/** Get the column with the largest sub-norm starting from i = p[j] = row_pos. */
int get_next_col(double const*mat, int rows, int cols, int row_pos, int *p)
{
    int max_loc = -1;
    double *col_norms = new double[ulong(cols)];

    // Compute the norms of the sub columns.
    for(int j=0; j<cols; j++)
    {
        col_norms[j] = 0;
        for(int i=row_pos; i<rows; i++) col_norms[j] += mat[i*cols + p[j]]*mat[i*cols + p[j]];
    }

    // Find the maximum location.
    double max = PRECISION;
    for(int i=0; i<cols; i++)
        if(col_norms[i] > max)
        {
            max = col_norms[i];
            max_loc = i;
        }

    // Collect garbge and return.
    delete [] col_norms;
    return max_loc;
}


/**
 * The star of the show. A QR least-squares solving routine for x = A\b.
 *
 * First argument : The row-major matrix (2D array), A.
 * Second argument: The right-hand side vector, b.
 * Third argument : The solution vector, x.
 * Fourth argument: The number of rows in A.
 * Fifth argument : The number of columns in A.
 *
 * WARNING: This routine will overwrite the matrix A and the right-hand side
 * vector b. In other words, A*x = b is solved using QR least-squares with,
 * column pivoting, but neither the A nor b are what you started with. However,
 * the solution x corresponds to the solution of both the modified and original
 * systems. Please be aware of this.
 *
 */
void QRLeastSquares(double const*A, double *b, double *sol, int rows, int cols)
{
    double *mat = new double[ulong(rows*cols)];
    double *rhs = new double[ulong(rows)];

    // Copy the matrix A into B since the QR routine will overwrite A.
    memcpy(mat, A, ulong(rows*cols)*sizeof(double));

    // Copy the vector b into d since the QR routine will overwrite b.
    memcpy(rhs, b, ulong(rows)*sizeof(double));


    /* Allocate memory for index vector and Householder transform vector. */
    int *p = new int[ulong(cols)];
    double *v = new double[ulong(rows)];

    /* Initial permutation vector. */
    for(int i=0; i<cols; i++) p[i] = i;

    /* Apply rotators to make R and Q'*b */
    for(int i=0; i<cols; i++)
    {
        int max_loc = get_next_col(mat, rows, cols, i, p);
        if(max_loc >= 0) swapcolumns(p, i, max_loc);

        householder(mat, rows, cols, i, p[i], v);
        apply_householder(mat, rhs, rows, cols, v, i, p);
    }

    /* Back solve Rx = Q'*b */
    back_solve(mat, rhs, rows, cols, sol, p);

    /* Collect garbage. */
    delete [] p;
    delete [] v;
    delete [] mat;
    delete [] rhs;
}


/** A very simple matrix vector product routine. */
void mat_vec(double const *mat, int rows, int cols, double const*vec, double *rhs)
{
    int i, j;
    double sum;

    for(i=0; i<rows; i++)
    {
        sum = 0.0;
        for(j=0; j<cols; j++) sum += mat[i*cols+j]*vec[j];

        rhs[i] = sum;
    }
}


/**
 * @brief A test program for the least quare method.
 * Solves the following least square problem:
 *   M set of data for N input parameters such that
 *       f(a[m,1]..a[m,N]) = b[m]      m is in [0, M-1]
 *   if(M>N) the system is over-determined and we  are searching for a solution
 *       g(a) = x1*a[m,1] + .. + xN * a[m,N]
 *       x1..xN are the unknown coefficients to be determined
 *       a is an array of M vectors of size N
 *   the unknowns are determined by a least square method such that
 *   the quadratic error
 *       r = sqrt(sum(b[m]-g(am)²)
 *   is minimal
 */
void testQRLeastSquare()
{

    // Dimensions for our matrix and vectors.
    // Space dimension is 3, and we have 6 data points
    // f(a1, a2, a3) = b
    int M = 6;
    int N = 3;

    // Allocate memory of the matrices and vectors.
    double *A = new double[ulong(M*N)];
    double *b = new double[ulong(M)];
    double *c = new double[ulong(M)];
    double *x = new double[ulong(M)];

    // Assign the matrix and the right hand side.
    b[0] = 3;
    b[1] = 9;
    b[2] = 15;
    b[3] = 22;
    b[4] = 27;
    b[5] = 33;

    for(int i=0; i<M*N; i++) A[i] = i+1;
    A[M*N-1] = -5;


    // Solve the least squares problem x = A\b.
    QRLeastSquares(A, b, x, M, N);

    // g(a1, a2, a3) = a1.x1 + a2.x2 + a3.x3
//    display_vec(x, N);

    // Use the copies of the initial matrix and vector to get the right hand side
    // which corresponds to the least squares solution.
    mat_vec(A, M, N, x, c);

//    display_vec(c, M);

    // Compute the 2-norm of the difference between the original right hand side
    // and the right hand side computed from the least squares solution.

/*    // The residual.
    double r = 0.0;
    for(int i=0; i<M; i++)    r += (c[i]-b[i]) * (c[i]-b[i]);
    r = sqrt(r); */

    /* Collect more garbage. */
    delete [] A;
    delete [] b;
    delete [] c;
    delete [] x;
}
