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

#include <complex>
#include <cstring>
#include <QString>

#include <fstream>
#include <iostream>
#include <cassert>
#include <thread>

#if defined ACCELERATE
  #include <Accelerate/Accelerate.h>
  #define lapack_int int
#elif defined INTEL_MKL
    #include <mkl.h>
#elif defined OPENBLAS
    #include <openblas/lapacke.h>
    #include <openblas/cblas.h>
#endif

#include <lapack_fortran.h>


#include <matrix.h>


#include <constants.h>


/** Inverts in place a 2x2 matrix */
bool matrix::invert22(double *l)
{
    double mat[4];
    memcpy(mat,l,sizeof(mat));
    return invert22(mat, l);
}


/** Inverts a 2x2 matrix */
bool matrix::invert22(double *in, double *out)
{
    double det(0);

    det  = in[0]*in[3] - in[1]*in[2];
    if(fabs(det)<1.e-15) return false;

    * out     =  in[3]/det;
    *(out+1)  = -in[1]/det;
    *(out+2)  = -in[2]/det;
    *(out+3)  =  in[0]/det;

    return true;
}

/** multiplies two 2x2 matrices */
void matrix::matMult22(double *a, double *b, double *ab)
{
    ab[0] = a[0]*b[0] + a[1]*b[2];
    ab[1] = a[0]*b[1] + a[1]*b[3];
    ab[2] = a[2]*b[0] + a[3]*b[2];
    ab[3] = a[2]*b[1] + a[3]*b[3];
}


/** Transposes in place a 3x3 matrix */
void matrix::transpose33(double *l)
{
    double temp(0);
    temp=l[1];   l[1]=l[3];  l[3]=temp;
    temp=l[2];   l[2]=l[6];  l[6]=temp;
    temp=l[5];   l[5]=l[7];  l[7]=temp;
}


/** Inverts in place a 3x3 matrix */
bool matrix::invert33(double *l)
{
    double mat[9];
    memcpy(mat,l,sizeof(mat));
    return invert33(mat, l);
}


/** Inverts a 3x3 matrix */
bool matrix::invert33(double *in, double *out)
{
    double det;

    det  = in[0] *(in[4] * in[8] - in[5]* in[7]);
    det -= in[1] *(in[3] * in[8] - in[5]* in[6]);
    det += in[2] *(in[3] * in[7] - in[4]* in[6]);
    if(fabs(det)<1.e-20) return false;

    * out     = (in[4] * in[8] - in[5] * in[7])/det;
    *(out+1)  = (in[2] * in[7] - in[1] * in[8])/det;
    *(out+2)  = (in[1] * in[5] - in[2] * in[4])/det;

    *(out+3)  = (in[5] * in[6] - in[3] * in[8])/det;
    *(out+4)  = (in[0] * in[8] - in[2] * in[6])/det;
    *(out+5)  = (in[2] * in[3] - in[0] * in[5])/det;

    *(out+6)  = (in[3] * in[7] - in[4] * in[6])/det;
    *(out+7)  = (in[1] * in[6] - in[0] * in[7])/det;
    *(out+8)  = (in[0] * in[4] - in[1] * in[3])/det;

    return true;
}


/**
 * @brief Av33 performs the product of the 3x3 matrix A and vector v. Stores the result in array p.
 * @param A a pointer to the matrix
 * @param v the input vector;
 * @param p the output vector;
 */
void matrix::AV33(double *A, double *v, double *p)
{
    p[0] = A[0]*v[0] + A[1]*v[1] + A[2]*v[2];
    p[1] = A[3]*v[0] + A[4]*v[1] + A[5]*v[2];
    p[2] = A[6]*v[0] + A[7]*v[1] + A[8]*v[2];
}


/** multiplies two 3x3 matrices */
void matrix::matMult33(double *a, double *b, double *ab)
{
    ab[0] = a[0]*b[0] + a[1]*b[3] + a[2]*b[6];
    ab[1] = a[0]*b[1] + a[1]*b[4] + a[2]*b[7];
    ab[2] = a[0]*b[2] + a[1]*b[5] + a[2]*b[8];

    ab[3] = a[3]*b[0] + a[4]*b[3] + a[5]*b[6];
    ab[4] = a[3]*b[1] + a[4]*b[4] + a[5]*b[7];
    ab[5] = a[3]*b[2] + a[4]*b[5] + a[5]*b[8];

    ab[6] = a[6]*b[0] + a[7]*b[3] + a[8]*b[6];
    ab[7] = a[6]*b[1] + a[7]*b[4] + a[8]*b[7];
    ab[8] = a[6]*b[2] + a[7]*b[5] + a[8]*b[8];
}


/**
 * Solves a linear system using Gauss partial pivot method
 * @param A a pointer to the single dimensionnal array of double values. Size is n².
 * @param n the size of the square matrix
 * @param B a pointer to the array of m RHS
 * @param m the number of RHS arrays to solve
 * @param pbCancel a pointer to the boolean variable which holds true if the operation should be interrupted.
 * @return true if the problem was successfully solved.
 */
bool matrix::Gauss(double *A, int n, double *B, int m, bool &bCancel)
{
    int row=0, i=0, j=0, pivot_row=0, k=0;
    double max=0, dum=0;
    double *pa=nullptr, *pA=nullptr, *A_pivot_row=nullptr;

    // for each variable find pivot row and perform forward substitution
    pa = A;
    for (row=0; row<n-1; row++, pa+=n)
    {
        if(bCancel) return false;

        //  find the pivot row
        A_pivot_row = pa;
        max = fabs(*(pa + row));
        pA = pa + n;
        pivot_row = row;
        for (i=row+1; i<n; pA+=n, i++)
        {
            if ((dum = fabs(*(pA+row)))>max)
            {
                max = dum;
                A_pivot_row = pA;
                pivot_row = i;
            }
        }

        if (max <= PRECISION)
            return false; // the matrix A is singular

        // and if it differs from the current row, interchange the two rows.
        if (pivot_row != row)
        {
            for (i=row; i<n; i++)
            {
                dum = *(pa + i);
                *(pa + i) = *(A_pivot_row + i);
                *(A_pivot_row + i) = dum;
            }
            for(k=0; k<m; k++)
            {
                dum = B[row+k*n];
                B[row+k*n] = B[pivot_row+k*n];
                B[pivot_row+k*n] = dum;
            }
        }

        // Perform forward substitution
        for (i= row+1; i<n; i++)
        {
            pA = A + i * n;
            dum = - *(pA + row) / *(pa + row);
            *(pA + row) = 0.0;
            for (j=row+1; j<n; j++) *(pA+j) += dum * *(pa + j);
            for (k=0; k<m; k++)
                B[i+k*n] += dum * B[row+k*n];
        }
    }

    // Perform backward substitution
    pa = A + (n-1) * n;
    for (row = n-1; row >= 0; pa -= n, row--)
    {
        if(bCancel) return false;

        if ( fabs(*(pa + row)) <PRECISION)
            return false;           // matrix is singular

        dum = 1.0 / *(pa + row);
        for (i=row+1; i<n; i++) *(pa + i) *= dum;
        for(k=0; k<m; k++) B[row+k*n] *= dum;
        for (i=0, pA=A; i<row; pA+= n, i++)
        {
            dum = *(pA + row);
            for (j=row+1; j<n; j++) *(pA + j) -= dum * *(pa+j);
            for(k=0; k<m; k++)
                B[i+k*n] -= dum * B[row+k*n];
        }
    }
    return true;
}



// Gauss no pivot, assuming no zero diagonal element
bool matrix::GaussNoPivot(double* A, int n, double* RHS, double *x)
{
    //Forward substitution
    for(int col=0; col<n; col++)
    {
        for(int row=col+1; row<n; row++)
        {
            if(fabs(A[col*n+col])<PRECISION) return false;
            double pivot = 1.0 / A[col*n+col]  * A[row*n+col];
            for(int j=col; j<n; j++)
            {
                 A[row*n+j] += - pivot * A[col*n+j];
            }
            RHS[row] +=  - pivot * RHS[col];
        }
    }

    //Backward substitution
    for(int row=n-1; row>=0; row--)
    {
        if(fabs(A[row*n+row])<PRECISION) return false;
        double pivot = 1.0 / A[row*n+row];
        x[row] = RHS[row] * pivot;
        for(int j=row+1; j<n; j++)
        {
            x[row] -= pivot * A[row*n+j] * x[j];
        }
    }
    return true;
}


/** Transposes in place a 4x4 matrix */
void matrix::transpose44f(float *l)
{
    float temp;
    temp=l[1];     l[1] =l[4];     l[4] =temp;
    temp=l[2];     l[2] =l[8];     l[8] =temp;
    temp=l[3];     l[3] =l[12];    l[12]=temp;
    temp=l[6];     l[6] =l[9];     l[9] =temp;
    temp=l[7];     l[7] =l[13];    l[13]=temp;
    temp=l[11];    l[11]=l[14];    l[14]=temp;
}


void matrix::transpose44f(float const *in, float *out)
{
    out[0]  = in[ 0];
    out[1]  = in[ 4];
    out[2]  = in[ 8];
    out[3]  = in[12];
    out[4]  = in[ 1];
    out[5]  = in[ 5];
    out[6]  = in[ 9];
    out[7]  = in[13];
    out[8]  = in[ 2];
    out[9]  = in[ 6];
    out[10] = in[13];
    out[11] = in[14];
    out[12] = in[ 3];
    out[13] = in[ 7];
    out[14] = in[11];
    out[15] = in[15];
}


/** Inverts in place a 4x4 matrix */
bool matrix::invert44(double *l)
{
    double mat[16];
    memcpy(mat,l,sizeof(mat));
    return invert44(mat, l);
}

/**
 * Inverts a real 4x4 matrix
 * @param ain in input, a pointer to a one-dimensional array holding the 16 complex values of the input matrix
 * @param aout in output, a pointer to a one-dimensional array holding the 16 complex values of the inverted matrix
 * @return if the inversion was successful
*/
bool matrix::invert44(double *ain, double *aout)
{
    //small size, use the direct method
    double det;
    double sign;

    det = det44(ain);

    if(fabs(det)<1.e-50) return false;

    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
//            sign = pow(-1.0,i+j);
            sign = -(((i+j)%2)*2-1);
            aout[4*j+i] = sign * cofactor44(ain, i, j)/det;
        }
    }
    return true;
}


/**
 * Inverts a complex 4x4 matrix
 * @param ain in input, a pointer to a one-dimensional array holding the 16 complex values of the input matrix
 * @param aout in output, a pointer to a one-dimensional array holding the 16 complex values of the inverted matrix
 * @return if the inversion was successful
*/
bool matrix::invert44(std::complex<double> *ain, std::complex<double> *aout)
{
    //small size, use the direct method
    std::complex<double> det;
    double sign;

    det = det44(ain);

    if(abs(det)<PRECISION) return false;

    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
//            sign = pow(-1.0,i+j);
            sign = -(((i+j)%2)*2-1);
            aout[4*j+i] = sign * cofactor44(ain, i, j)/det;
        }
    }
    return true;
}


/**
*Returns the determinant of a complex 3x3 matrix
*@param aij a pointer to a one-dimensional array holding the 9 complex values of the matrix
*@return the matrix's determinant
*/
std::complex<double> matrix::det33(std::complex<double> *aij)
{
    std::complex<double> det;

    det  = aij[0]*aij[4]*aij[8];
    det -= aij[0]*aij[5]*aij[7];

    det -= aij[1]*aij[3]*aij[8];
    det += aij[1]*aij[5]*aij[6];

    det += aij[2]*aij[3]*aij[7];
    det -= aij[2]*aij[4]*aij[6];

    return det;
}


/**
 * Returns the determinant of a 4x4 matrix
 * @param aij a pointer to a one-dimensional array holding the 16 double values of the matrix
 * @return the matrix's determinant
 */
double matrix::det44(double *aij)
{
    int p(0), q(0);
    double det(0), sign(0);
    double a33[9]{0};

    det = 0.0;

    for(int j=0; j<4; j++)
    {
        p = 0;
        for(int k=0; k<4; k++)
        {
            if(k!=0)
            {
                q = 0;
                for(int l=0; l<4; l++)
                {
                    if(l!=j)
                    {
                        *(a33+p*3+q) = *(aij+4*k+l);
                        q++;
                    }
                }
                assert(q==3);
                p++;
            }
        }
        assert(p==3);
//        sign = pow(-1.0,j);
        sign = -(((j)%2)*2-1);
        det += aij[j] * sign * det33(a33);
    }

    return det;
}


/** returns the cofactor of element i,j, in the 4x4 matrix aij */
float matrix::cofactor44(float *aij, int i, int j)
{
    int p(0), q(0);
    float a33[9]{0};

    p = 0;
    for(int k=0; k<4; k++)
    {
        if(k!=i)
        {
            q = 0;
            for(int l=0; l<4; l++)
            {
                if(l!=j)
                {
                    a33[p*3+q] = *(aij+4*k+l);
                    q++;
                }
            }
            p++;
        }
    }
    return det33(a33);
}


/** returns the cofactor of element i,j, in the 4x4 matrix aij */
double matrix::cofactor44(double *aij, int i, int j)
{
    int p(0), q(0);
    double a33[9]{0};

    p = 0;
    for(int k=0; k<4; k++)
    {
        if(k!=i)
        {
            q = 0;
            for(int l=0; l<4; l++)
            {
                if(l!=j)
                {
                    a33[p*3+q] = *(aij+4*k+l);
                    q++;
                }
            }
            p++;
        }
    }
    return det33(a33);
}

/**
*Returns the cofactor of an element in a 4x4 matrix of complex values.
*@param aij a pointer to a one-dimensional array holding the 16 complex values of the matrix.
*@param i the number of the element's line, starting at 0.
*@param j the number of the element's column, starting at 0.
*@return the cofactor of element (i,j).
*/
std::complex<double> matrix::cofactor44(std::complex<double> *aij, int i, int j)
{
    //returns the complex cofactor    of element i,j, in the 4x4 matrix aij
    int k(0),l(0),p(0),q(0);
    std::complex<double> a33[9];

    p = 0;
    for(k=0; k<4; k++)
    {
        if(k!=i)
        {
            q = 0;
            for(l=0; l<4; l++)
            {
                if(l!=j)
                {
                    a33[p*3+q] = *(aij+4*k+l);
                    q++;
                }
            }
            p++;
        }
    }
    return det33(a33);
}

/**
* Returns the determinant of a complex 4x4 matrix
* @param aij a pointer to a one-dimensional array holding the 16 complex double values of the matrix
* @return the matrix's determinant
*/
std::complex<double> matrix::det44(std::complex<double> *aij)
{
//    returns the determinant of a 4x4 matrix

    int i(0),p(0),q(0);
    double sign(0);
    std::complex<double> det, a33[16];
    det = 0.0;

    i=0;
    for(int j=0; j<4; j++)
    {
        p = 0;
        for(int k=0; k<4; k++)
        {
            if(k!=i)
            {
                q = 0;
                for(int l=0; l<4; l++)
                {
                    if(l!=j)
                    {
                        a33[p*3+q] = aij[4*k+l];
                        q++;
                    }
                }
                p++;
            }
        }
//        sign = pow(-1.0,i+j);
        sign = -(((i+j)%2)*2-1);
        det += sign * aij[4*i+j] * det33(a33);
    }

    return det;
}


/**________________________________________________________________________
* Finds the eigenvector associated to an eigenvalue.
* Solves the system A.V = lambda.V where A is a 4x4 complex matrix
* in input :
*    - matrix A
*    - the array of complex eigenvalues
* in output
*    - the array of complex eigenvectors
*
* The eigenvector is calculated by direct matrix inversion.
* One of the vector's component is set to 1, to avoid the trivial solution V=0;
*
* (c) Andre Deperrois October 2009
*@param a the complex two-dimensional 4x4 input matrix to diagonalize
*@param lambda the output array of four complex eigenvalues
*@param V the eigenvector as a one-dimensional array of complex values
*________________________________________________________________________ */
bool matrix::eigenVector(double a[][4], std::complex<double> lambda, std::complex<double> *V)
{
    std::complex<double> detm, detr;
    std::complex<double> r[9], m[9];
    int ii(0), jj(0), i(0), j(0), kp(0);

    // first find a pivot for which the  associated n-1 determinant is not zero
    bool bFound = false;
    kp=0;
    do
    {
        V[kp] = 1.0;
        ii= 0;
        for(i=0;i<4 ; i++)
        {
            if(i!=kp)
            {
                jj=0;
                for(j=0; j<4; j++)
                {
                    if(j!=kp)
                    {
                        m[ii*3+jj] = a[i][j];
                        jj++;
                    }
                }
                m[ii*3+ii] -= lambda;
                ii++;
            }
        }
        detm = det33(m);
        bFound = std::abs(detm)>0.0;
        if(bFound || kp>=3) break;
        kp++;
    }while(true);

    if(!bFound) return false;

    // at this point we have identified pivot kp
    // with a non-zero subdeterminant.
    // so solve for the other 3 eigenvector components.
    // using Cramer's rule

    //create rhs determinant
    jj=0;
    for(j=0; j<4; j++)
    {
        memcpy(r,m, 9*sizeof(std::complex<double>));
        if(j!=kp)
        {
            ii= 0;
            for(i=0; i<4; i++)
            {
                if(i!=kp)
                {
                    r[ii*3+jj] = - a[i][kp];
                    ii++;
                }
            }
            detr  = det33(r);
            V[j] = detr/detm;
            jj++;
        }
    }

    return true;
}


/** Simple routine for displaying a vector. */
void matrix::display_vec(double *vec, int n, std::string &list)
{
    list.clear();
    for(int i=0; i<n; i++)
        list += QString::asprintf("\t%17.9glg", vec[i]).toStdString();
}


void matrix::display_mat(std::vector<double> const &mat, int size, std::string &list)
{
    list.clear();
    display_mat(mat.data(), size, size, list);
}


void matrix::display_mat(double const *mat, int rows, int cols, std::string &list)
{
    list.clear();
    QString str, strong;
    if(cols<=0) cols = rows;
    for(int i=0; i<rows; i++)
    {
        strong=" ";
        for(int j=0; j<cols; j++)
        {
            str = QString::asprintf(" %17g ", mat[i*cols+j]);
            strong += str;
        }
//        std::string str = strong.toStdString();
//        const char* p = str.c_str();
        list += QString::asprintf("%s", strong.toStdString().c_str()).toStdString();//avoid inverted commas
    }
}


void matrix::display_mat(float const*mat, int rows, int cols, std::string &list)
{
    list.clear();
    QString str, strong;
    if(cols<=0) cols = rows;
    for(int i=0; i<rows; i++)
    {
        strong=" ";
        for(int j=0; j<cols; j++)
        {
            str = QString::asprintf(" %17g ", mat[i*cols+j]);
            strong += str;
        }

        list += QString::asprintf("%s", strong.toStdString().c_str()).toStdString();//avoid inverted commas
    }
}


void matrix::display_mat(std::vector<std::vector<double>> const &mat, std::string &list)
{
    list.clear();
    for(unsigned int i=0; i<mat.size(); i++)
    {
        QString strong="    ";
        for(unsigned int j=0; j<mat[i].size(); j++)
        {
            strong += QString::asprintf("%11.5g  ", mat[i][j]);
        }

        list += QString::asprintf("%s", strong.toStdString().c_str()).toStdString();//avoid inverted commas
    }
}


/** Simple routine for displaying a vector. */
void matrix::display_vec(float *vec, int n, bool bHorizontal, std::string &list)
{
    list.clear();
    if(!bHorizontal)
    {
        for(int i=0; i<n; i++)
            list += QString::asprintf("\t%17g", double(vec[i])).toStdString();
    }
    else
    {
        QString strange, strong;
        for(int i=0; i<n; i++)
        {
            strong = QString::asprintf("  %11g", double(vec[i]));
            strange += strong;
        }
        list += QString::asprintf("%s", strange.toStdString().c_str()).toStdString();
    }
}


void matrix::saveMatrixtoFile(std::string filename, std::vector<double> const &mat, int N)
{
    std::ofstream out(filename);

    QString strange, strong;
    for(int row=0; row<N; row++)
    {
        strange.clear();
        for(int col=0; col<N; col++)
        {
            strong = QString::asprintf(" %11g;", mat.at(row*N+col));
            strange += strong;
        }
        strange += "\n";
        out << strange.toStdString();
    }
    out.close();
}

/**
* L is lower triangular with unit values on the diagonal
* U is upper triangular
* LU is stored in A
*/
void matrix::CholevskiFactor(double *A, double *L, int n)
{
    for(int i=0; i<n*n; i++) L[i]=0.0;
    for(int i=0; i<n;   i++) L[i*n+i]=1.0;

    for (int i=0; i<n; i++)
    {
        for (int j=0; j<(i+1); j++)
        {
            double s = 0;
            for (int k=0; k<j; k++)
                s += L[i*n+k] * L[j*n+k];
            if(i==j) L[i*n+j] = sqrt(A[i*n+i] - s);
            else     L[i*n+j] = (1.0 / L[j*n+j] * (A[i*n+j] - s));
        }
    }
}


bool matrix::CholevskiSolve(double const*A, double* b, int n)
{
    double sum = 0.0;
    //forward substitute
    for (int i=0; i<n; i++)
    {
        if(fabs(A[i*n+i])<0.0000000001) return false;
        sum = b[i];
        for (int l=0; l<i; l++)
        {
            sum -= A[i*n+l] * b[l];
        }

        b[i] = sum/A[i*n+i];
    }

    //back-substitute. Now.
    for (int i=n-1; i>=0; i--)
    {
        sum = b[i];
        for (int j=i+1; j<n; j++)
            sum -= A[j*n+i] * b[j];
        b[i] = sum / A[i*n+i];
    }

    return true;
}


/**
* A is m rows x n columns
* B is n rows x q columns
* AB is m x q
*/
void matrix::matMult_SingleThread(double* const A, double* const B, double* AB, int m, int n, int q)
{
    if(m==2 && n==2 && q==2)
    {
        matMult22(A,B,AB);
        return;
    }
    else if(m==3 && n==3 && q==3)
    {
        matMult33(A,B,AB);
        return;
    }

    // access elements by address rather than by indices
    // and unroll two rows of A
    int half = m/2;
    double ytmp1=0, ytmp2=0;

    double *Apos1 = A;
    double *Apos2 = A+n;
    double *Bpos = B;
    double btmp;

    for(int j=0; j<q; j++)
    {
        for(int i=0; i<half; i++)
        {
            ytmp1 = ytmp2 = 0.0;

            for(int k=0; k<n; k++)
            {
                btmp = *(Bpos+k*q);
                ytmp1 += *(Apos1++) * btmp;
                ytmp2 += *(Apos2++) * btmp;
            }
            AB[ 2*i   *q +j] = ytmp1;
            AB[(2*i+1)*q +j] = ytmp2;

            Apos1 += n;
            Apos2 += n;
        }
        if(2*half!=m)
        {
            //odd number of rows, process last line
            ytmp1 = 0.0;

            for(int k=0; k<n; k++)
            {
                ytmp1 += *(Apos1++) * *(Bpos+k*q);
            }
            AB[(m-1)*q +j] = ytmp1;
        }

        Apos1 = A;
        Apos2 = A+n;
        Bpos++;
    }
}


void matrix::matMult(double* const A, double* const B, double* AB, int n, int nThreads)
{
    if(n==2)
    {
        matMult22(A,B,AB);
        return;
    }
    else if(n==3)
    {
        matMult33(A,B,AB);
        return;
    }
    else
    {
        if(nThreads<0) nThreads=std::thread::hardware_concurrency();;
        matMult(A,B,AB,n,n,1,nThreads);
    }
}


void matrix::matMultLAPACK(double const *A, double const *B, double* AB, int m, int k, int n, int nThreads)
{
#ifdef INTEL_MKL
    if(nThreads>0)
    {
        nThreads = std::min(nThreads, int(std::thread::hardware_concurrency()));
        MKL_Set_Num_Threads_Local(nThreads);
    }
#else
    (void)nThreads;
#endif

    double alpha=1.0;
    double beta=0.0;


#ifdef MAC_OS
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, A, k, B, n, beta, AB, n);
#elif defined WIN_OS
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, A, k, B, n, beta, AB, n);
#elif defined LINUX_OS
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, A, k, B, n, beta, AB, n);
#endif
}


void matrix::matVecMultLapack(float const *A, float const *X, float* Y, int n, int m, int nThreads)
{
#ifdef INTEL_MKL
    if(nThreads>0)
    {
        nThreads = std::min(nThreads, int(std::thread::hardware_concurrency()));
        MKL_Set_Num_Threads_Local(nThreads);
    }
#else
    (void)nThreads;
#endif
    lapack_int lda = n;

    int incx = 1, incy=1; // elements are contiguous
    float alpha=1;
    float beta=0;


#ifdef MAC_OS
    cblas_sgemv(CblasColMajor, CblasTrans, n, m, alpha, A, lda, X, incx, beta, Y, incy);
#elif defined WIN_OS
#if defined(OPENBLAS)
    cblas_sgemv(CblasColMajor, CblasTrans, n, m, alpha, A, lda, X, incx, beta, Y, incy);
#else
    char trans = 'T';
    sgemv(&trans, &n, &m, &alpha, A, &lda, X, &incx, &beta, Y, &incy);
#endif
#elif defined LINUX_OS
//    char trans = 'T';
//    sgemv(&trans, &n, &m, &alpha, A, &lda, X, &incx, &beta, Y, &incy);
    cblas_sgemv(CblasColMajor, CblasTrans, n, m, alpha, A, lda, X, incx, beta, Y, incy);
#endif
}


void matrix::matVecMultLapack(double const *A, double const *X, double* Y, int m, int n, int nThreads)
{
#ifdef INTEL_MKL
    if(nThreads>0)
    {
        nThreads = std::min(nThreads, int(std::thread::hardware_concurrency()));
        MKL_Set_Num_Threads_Local(nThreads);
    }
#else
    (void)nThreads;
#endif


    int incx = 1, incy=1; // elements are contiguous
    float alpha=1.0;
    float beta=0.0;


#ifdef MAC_OS
    cblas_dgemv(CblasColMajor, CblasTrans, n, m, alpha, A, n, X, incx, beta, Y, incy);
#elif defined WIN_OS
    cblas_dgemv(CblasRowMajor, CblasNoTrans, m, n, alpha, A, n, X, incx, beta, Y, incy);
#elif defined LINUX_OS
    cblas_dgemv(CblasRowMajor, CblasNoTrans, m, n, alpha, A, n, X, incx, beta, Y, incy);
#endif
}


void matrix::matVecMult(double const *A, double const *X, double *Y, int n)
{
    if(n<2) return;
    if     (n==2) matVecMult2x2(A, X, Y);
    else if(n==3) matVecMult3x3(A, X, Y);
    else if(n==4) matVecMult4x4(A, X, Y);
    else          matVecMultLapack(A, X, Y, n, n);
}


void matrix::matVecMult2x2(double const *A, double const *X, double *Y)
{
    Y[0] = A[0]*X[0] + A[1]*X[1];
    Y[1] = A[2]*X[0] + A[3]*X[1];
}


void matrix::matVecMult3x3(double const *A, double const *X, double *Y)
{
    Y[0] = A[0]*X[0] + A[1]*X[1] + A[2]*X[2];
    Y[1] = A[3]*X[0] + A[4]*X[1] + A[5]*X[2];
    Y[2] = A[6]*X[0] + A[7]*X[1] + A[8]*X[2];
}


void matrix::matVecMult4x4(double const *A, double const *X, double *Y)
{
    Y[0] = A[0] *X[0] + A[1] *X[1] + A[2] *X[2] + A[3] *X[3];
    Y[1] = A[4] *X[0] + A[5] *X[1] + A[6] *X[2] + A[7] *X[3];
    Y[2] = A[8] *X[0] + A[9] *X[1] + A[10]*X[2] + A[11]*X[3];
    Y[3] = A[12]*X[0] + A[13]*X[1] + A[14]*X[2] + A[15]*X[3];
}



/**
* A is m rows x n columns
* B is n rows x q columns
* AB is m x q
*/
void matrix::matMult(double* const A, double* const B, double* AB, int nRows, int n, int nCols, int nThreads)
{
    if(nThreads<0) nThreads=std::thread::hardware_concurrency();
    int m_nBlocks = nThreads;
    if(m_nBlocks>nCols) m_nBlocks=nCols;
    MatMultData mmd(A, B, AB, nRows, n, nCols);

    int iblock(0);
    std::vector<std::thread> threads;

    for (iblock=0; iblock<nThreads; iblock++)
    {
        threads.push_back(std::thread(matMultBlock, iblock, &mmd, nThreads));
    }

    for (iblock=0; iblock<nThreads; iblock++)
    {
        threads[iblock].join();
    }
}


/**
 * @brief matMultBlock multiplies a block of vector columns by a block of row vectors
 * @param iBlock
 * @param mmd
 */
void matrix::matMultBlock(int iBlock, MatMultData *mmd, int nThreads)
{
    int nBlocks = nThreads;
    if(nBlocks>mmd->nCols) nBlocks=mmd->nCols;
    int blocksize = mmd->nCols/nBlocks;

    int j0 = iBlock*blocksize;

    //adjust last block size to compensate for non exact column division

    int half = mmd->nRows/2;

    double ytmp1, ytmp2;

    double *Apos1 = mmd->A;
    double *Apos2 = mmd->A+mmd->n;
    double *Bpos = mmd->B+iBlock*blocksize;
    double btmp;
    if(iBlock==nBlocks-1)
        blocksize = mmd->nCols - iBlock*blocksize;

    // access elements by address rather than by indices and unroll two rows of A
    for(int j=0; j<blocksize; j++)
    {
        for(int i=0; i<half; i++)
        {
            ytmp1 = ytmp2 = 0.0;

            for(int k=0; k<mmd->n; k++)
            {
                btmp = *(Bpos+k*mmd->nCols);
                ytmp1 += *(Apos1++) * btmp;
                ytmp2 += *(Apos2++) * btmp;
            }
            mmd->AB[ 2*i   *mmd->nCols +j0+j] = ytmp1;
            mmd->AB[(2*i+1)*mmd->nCols +j0+j] = ytmp2;

            Apos1 += mmd->n;
            Apos2 += mmd->n;
        }
        if(2*half!=mmd->nRows)
        {
            //odd number of rows, process last line
            ytmp1 = 0.0;

            for(int k=0; k<mmd->n; k++)
            {
                ytmp1 += *(Apos1++) * *(Bpos+k*mmd->nCols);
            }
            mmd->AB[(mmd->nRows-1)*mmd->nCols +j0+j] = ytmp1;
        }

        Apos1 = mmd->A;
        Apos2 = mmd->A+mmd->n;
        Bpos++;
    }
}


/** orders the matrix array by column-major index
 * @param n    the size of the square matrix
 * @param aij  the input matrix ordered with row-major index
 * @param taij the output matrix ordered with column-major index
 */
void matrix::rowToColumnMajorIndex(int n, double *aij, double  *taij)
{
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j++)
        {
            taij[i + j*n] = aij[i*n + j];
        }
    }
}


/** orders the matrix array by column-major index
 * @param n    the size of the square matrix
 * @param aij  the input matrix ordered with row-major index
 * @param taij the output matrix ordered with column-major index
 */
void matrix::columnToRowMajorIndex(int n, double *aij, double  *taij)
{
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j++)
        {
            taij[i*n + j] = aij[i + j*n];
        }
    }
}


bool matrix::inverseMatrix_old(int n, double *aij, double *invAij)
{
    // make identity RHS
    // tempMat is ordered in column major index
    std::vector<double>tempMat(n*n);
    memset(tempMat.data(), 0, int(n*n)*sizeof(double));
    for(int i=0; i<n; i++) tempMat[i+i*n] = 1.0;

    std::vector<double> RHS(n);
    memset(RHS.data(), 0, int(n)*sizeof(double));
    RHS[0]=1;

    //solve each column vector by Gaussian elemination
    bool bCancel=false;

    if(!Gauss(aij, n, tempMat.data(), n, bCancel)) return false;

    // transpose back to row-major index
    columnToRowMajorIndex(n, tempMat.data(), invAij);

    return true;
}


bool matrix::inverseMatrixLapack(int n, std::vector<double> &aij, int nThreads)
{
#ifdef INTEL_MKL
    if(nThreads>0)
    {
        nThreads = std::min(nThreads, int(std::thread::hardware_concurrency()));
        MKL_Set_Num_Threads_Local(nThreads);
    }
#else
    (void)nThreads;
#endif

    lapack_int lda = n;
    std::vector<lapack_int> ipiv;  /** the array of pivot indices for the LAPACK LU solver */
    ipiv.resize(n);
    memset(ipiv.data(), 0, int(n)*sizeof(int));

    //compute LU factorization
    lapack_int info=0;

    dgetrf_(&n, &n, aij.data(), &lda, ipiv.data(), &info);

    if(info>0 || info <0)
    {
        return false;
    }

    int lwork = n;
    std::vector<double>work(n*n);

    // invert
    work.resize(lwork);
    dgetri_(&n, aij.data(), &lda, ipiv.data(), work.data(), &lwork, &info);

    return !info;
}


double matrix::determinant(double *a,int n)
{
    double det = 0;
    double *m = nullptr;

    if (n<1)       return 0.0;
    else if (n==1) return a[0];
    else if (n==2) return a[0]*a[3] - a[1]*a[2];
    else if (n==3)
    {
        return    a[0]*a[4]*a[8]  - (a[0]*a[5]*a[7])
                -(a[1]*a[3]*a[8]) +  a[1]*a[5]*a[6]
                + a[2]*a[3]*a[7]  - (a[2]*a[4]*a[6]);
    }
    else
    {
        det = 0;
        for (int row=0; row<n; row++)
        {
            m = new double[int((n-1) * (n-1))*sizeof(double)];

            for(int i=0; i<n; i++)
            {
                if(i<row)
                    memcpy(m+i*(n-1), a + i*n+1, int(n-1)*sizeof(double));
                else if(i>row)
                    memcpy(m+(i-1)*(n-1), a + i*n+1, int(n-1)*sizeof(double));
            }
            det += pow(-1.0,row) * a[row*n+0] * determinant(m, n-1);
            delete [] m;
        }
    }
    return(det);
}


////////////////////////////////////////////////////////* Auxiliary routine: printing a matrix */
void print_matrix_rowmajor(const char* desc, lapack_int m, lapack_int n, double* a, lapack_int lda )
{
    lapack_int i, j;
    printf( "\n [%s]\n", desc );
    for(i=0; i<m; i++)
    {
        for( j = 0; j < n; j++ )
        {
            printf(" %6f", a[i*lda+j]);
        }
        printf( "\n" );
    }
}

////////////////////////////////////////////////////////* Auxiliary routine: printing a matrix */
void print_matrix_colmajor(const char* desc, int m, int n, double* a, int lda )
{
    lapack_int i, j;
    printf( "\n [%s]\n", desc );
    for(i=0; i<m; i++)
    {
        for(j=0; j<n; j++)
        {
            printf(" %6.2f", a[j*lda+i]);
        }
        printf( "\n" );
    }
}


/** Simple routine for displaying a vector. */
void listArray(double *array, int size, std::string &list)
{
    list.clear();
    for(int i=0; i<size; i++)
        list += QString::asprintf("\t%17g", array[i]).toStdString();
}


void matrix::listArray(const std::vector<double> &array, std::string &list)
{
    list.clear();
    for(unsigned int i=0; i<array.size(); i++)
        list += QString::asprintf("\t%17g", array.at(i)).toStdString();
}


void matrix::listArrays(std::vector<double> const &array1, std::vector<double> const &array2, std::string &list)
{
    list.clear();
    int maxsize = std::min(int(array1.size()), int(array2.size()));
    for(int i=0; i<maxsize; i++)
    {
        list += QString::asprintf("\t%17g  \t%17g  ", array1.at(i), array2.at(i)).toStdString();
    }
}


void matrix::listArrays(std::vector<double> const &array1, std::vector<double> const &array2, std::vector<double> const &array3, std::string &list)
{
    list.clear();
    int maxsize = std::min(int(array1.size()), int(array2.size()));
    maxsize = std::min(maxsize, int(array3.size()));
    for(int i=0; i<maxsize; i++)
    {
        list += QString::asprintf("\t%17g  \t%17g  \t%17g", array1.at(i), array2.at(i), array3.at(i)).toStdString();
    }
}


void matrix::listArrays(const std::vector<float> &array1, const std::vector<float> &array2, std::string &list)
{
    list.clear();
    for(unsigned int i=0; i<array1.size(); i++)
    {
        list += QString::asprintf("\t%17g  \t%17g  ", double(array1.at(i)), double(array2.at(i))).toStdString();
    }
}



/** inverts a matrix in place */
bool matrix::invertMatrix(std::vector<double> &mat, int N)
{
    if(N==1)
    {
        if(fabs(mat[0])<1.e-15) return false;
        mat[0] = 1.0/(mat[0]);
        return true;
    }
    else if(N==2)
    {
        return invert22(mat.data());
    }
    else if(N==3)
    {
        return invert33(mat.data());
    }
    else if(N==4)
    {
        return invert44(mat.data());
    }

    return inverseMatrixLapack(N, mat);
}


/**
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
bool matrix::blockThomasSolve(std::vector<std::vector<double>> &A, std::vector<std::vector<double>> &B, std::vector<std::vector<double>> &C,
                      std::vector<std::vector<double>> &R, std::vector<std::vector<double>> &X,
                      int n, int p)
{
    // temporary matrices
    std::vector<double> M(p*p);
    std::vector<double> Inv(p*p);
    std::vector<double> MC(p*p);
    std::vector<double> dp(p);

    std::vector<std::vector<double>> gam(n);
    std::vector<std::vector<double>> beta(n);

    for(int i=0; i<n; i++)
    {
        gam[i].resize(p*p);
        beta[i].resize(p);
    }

    // forward elimination
    for(int q=0; q<p*p; q++) Inv[q] = B.at(0)[q];
    if(!invertMatrix(Inv, p)) return false;

    matMult(Inv.data(), C[0].data(), gam[0].data(),p,1);

    for(int l=0; l<p; l++)
    {
        beta[0][l] = 0.0;
        for(int q=0; q<p; q++)
        {
            beta[0][l] += Inv[l*p+q]*R.at(0)[q];
        }
    }

    for(int k=1; k<n; k++)
    {
        matMult(A[k].data(), gam[k-1].data(), M.data(),p,1);
        for(int l=0; l<p*p; l++) MC[l] = B.at(k)[l]-M[l];
        invertMatrix(MC, p);
        matMult(MC.data(), C[k].data(), gam[k].data(),p,1);

        for(int l=0; l<p; l++)
        {
            dp[l] = R.at(k)[l];
            for(int q=0; q<p; q++)
                dp[l] -= A.at(k)[l*p+q]*beta.at(k-1)[q];
        }

        for(int l=0; l<p; l++)
        {
            beta[k][l] = 0.0;
            for(int q=0; q<p; q++)
                beta[k][l] += MC[l*p+q] * dp[q];
        }
    }

    // back substitution
    for(int l=0; l<p; l++)
        X[n-1][l] = beta.at(n-1)[l];

    for(int k=n-2; k>=0; k--)
    {
        for(int l=0; l<p; l++)
        {
            X[k][l] = beta.at(k)[l];
            for(int q=0; q<p; q++)
            {
                X[k][l] -= gam.at(k)[l*p+q]*X.at(k+1)[q];
            }
        }
    }

    return true;
}


/** solves a 2x2 linear system */
bool matrix::solve2x2(double *M, double *rhs, int nrhs)
{
    double det  = M[0]*M[3] - M[1]*M[2];
    if(fabs(det)<1.e-20) return false;

    for(int i=0; i<nrhs; i++)
    {
        double a = rhs[2*i];
        double b = rhs[2*i+1];
        rhs[2*i]   =  (a*M[3] - b*M[1])/det;
        rhs[2*i+1] = -(a*M[2] - b*M[0])/det;
    }
    return true;
}


/** solves a 3x3 linear system */
bool matrix::solve3x3(double *M, double *rhs, int nrhs)
{
    double x(0), y(0), z(0);
    double det = det33(M);
    if(fabs(det)<PRECISION) return false;
    for(int i=0; i<nrhs; i++)
    {
        x = rhs[3*i]*(M[4]*M[8]  -M[5]*M[7])       - rhs[3*i+1]*(M[1]*M[8]-M[2]*M[7])       + rhs[3*i+2]*(M[1]*M[5]-M[2]*M[4]);
        y = M[0]*(rhs[3*i+1]*M[8]-M[5]*rhs[3*i+2]) - M[3]*(rhs[3*i]*M[8]  -M[2]*rhs[3*i+2]) + M[6]*(rhs[3*i]*M[5]  -M[2]*rhs[3*i+1]);
        z = M[0]*(M[4]*rhs[3*i+2]-rhs[3*i+1]*M[7]) - M[3]*(M[1]*rhs[3*i+2]-rhs[3*i]*M[7])   + M[6]*(M[1]*rhs[3*i+1]-rhs[3*i]*M[4]);

        rhs[3*i]   = x /det;
        rhs[3*i+1] = y /det;
        rhs[3*i+2] = z /det;
    }

    return true;
}


double matrix::det33(double *M)
{
    return M[0]*(M[4]*M[8]-M[5]*M[7]) - M[3]*(M[1]*M[8]-M[2]*M[7]) + M[6]*(M[1]*M[5]-M[2]*M[4]);
}


float matrix::det33(float *M)
{
    float det(0);

    det  = M[0]*(M[4]*M[8]-M[5]*M[7]);
    det -= M[3]*(M[1]*M[8]-M[2]*M[7]);
    det += M[6]*(M[1]*M[5]-M[2]*M[4]);

    return det;
}


/** solves a 4x4 linear system */
bool matrix::solve4x4(double *M, double *rhs, int nrhs)
{
    if(!invert44(M)) return false;
    double x(0), y(0), z(0), t(0);
    for(int i=0; i<nrhs; i++)
    {
        x = M[0] *rhs[4*i] + M[1] *rhs[4*i+1] + M[2] *rhs[4*i+2] + M[3] *rhs[4*i+3];
        y = M[4] *rhs[4*i] + M[5] *rhs[4*i+1] + M[6] *rhs[4*i+2] + M[7] *rhs[4*i+3];
        z = M[8] *rhs[4*i] + M[9] *rhs[4*i+1] + M[10]*rhs[4*i+2] + M[11]*rhs[4*i+3];
        t = M[12]*rhs[4*i] + M[13]*rhs[4*i+1] + M[14]*rhs[4*i+2] + M[15]*rhs[4*i+3];
        rhs[4*i]   = x;
        rhs[4*i+1] = y;
        rhs[4*i+2] = z;
        rhs[4*i+3] = t;
    }

    return true;
}


int matrix::solveLinearSystem(int rank, float *M, int nrhs, float *rhs, int nThreads)
{
#ifdef INTEL_MKL
    if(nThreads>0)
    {
        nThreads = std::min(nThreads, int(std::thread::hardware_concurrency()));
        MKL_Set_Num_Threads_Local(nThreads);
    }
#else
    (void)nThreads;
#endif

    // LAPACK methods are implemented in column-major order so use the transpose matrix
    char trans = 'T';
    lapack_int nl=0, lda=0, ldb=0, info=0;
    std::vector<lapack_int> ipiv(rank);

    nl = rank;
    lda = rank;

    sgetrf_(&nl, &nl, M, &lda, ipiv.data(), &info);

    if(info>0 || info <0)
    {
//        xfl::trace("dgetrf_ error:",info);
        return int(info);
    }

    // apparently LAPACK expects rhs to be row major same as the matrix
    // so need to perform back substitutions one at a time
    for(int k=0; k<nrhs; k++)
    {
        ldb = 1;
#ifdef OPENBLAS
        sgetrs_(&trans, &nl, &ldb, M, &lda, ipiv.data(), rhs+k*rank, &nl, &info, 1);
#elif defined INTEL_MKL
        sgetrs_(&trans, &nl, &ldb, M, &lda, ipiv.data(), rhs+k*rank, &nl, &info);
#elif defined ACCELERATE
        sgetrs_(&trans, &nl, &ldb, M, &lda, ipiv.data(), rhs+k*rank, &nl, &info);
#endif


        if(info!=0)
        {
//            xfl::trace("dgetrs_ error:",info);
            return int(info);
        }
    }
    return 0;
}


int matrix::solveLinearSystem(int rank, double *M, int nrhs, double *rhs, int nThreads)
{
#ifdef INTEL_MKL
    if(nThreads>0)
    {
        nThreads = std::min(nThreads, int(std::thread::hardware_concurrency()));
        MKL_Set_Num_Threads_Local(nThreads);
    }
#else
    (void)nThreads;
#endif

    // LAPACK methods are implemented in column-major order so use the transpose matrix
    char trans = 'T';
    lapack_int nl=0, lda=0, ldb=0, info=0;
    std::vector<lapack_int> ipiv(rank);

    nl = rank;
    lda = rank;

    dgetrf_(&nl, &nl, M, &lda, ipiv.data(), &info);

    if(info>0 || info <0)
    {
//        xfl::trace("dgetrf_ error:",info);
        return int(info);
    }

    // apparently LAPACK expects rhs to be row major same as the matrix
    // so need to perform back substitutions one at a time
    for(int k=0; k<nrhs; k++)
    {
        ldb = 1;
#ifdef OPENBLAS
        dgetrs_(&trans, &nl, &ldb, M, &lda, ipiv.data(), rhs+k*rank, &nl, &info, 1);
#elif defined INTEL_MKL
        dgetrs_(&trans, &nl, &ldb, M, &lda, ipiv.data(), rhs+k*rank, &nl, &info);
#elif defined ACCELERATE
        dgetrs_(&trans, &nl, &ldb, M, &lda, ipiv.data(), rhs+k*rank, &nl, &info);
#endif


        if(info!=0)
        {
//            xfl::trace("dgetrs_ error:",info);
            return int(info);
        }
    }
    return 0;
}


void matrix::setIdentityMatrix(double *M, int n)
{
    memset(M, 0, n*n*sizeof(double));
    for(int i=0; i<n; i++) M[i*n+i]=1.0;
}


/**
 * @brief makeILU. Builds an incomplete LU factorization of matrix A.
 * As a dropping rule, all elements further away than p indices from the diagonal are ignored in input
 * and are set to zero in output.
 * Both matrices are assumed to have been pre-allocated.
 * @param A the input matrix
 * @param ILU the resulting ILU preconditioner
 * @param n the size of the matrix
 * @param p the dropping rule's parameter
 * @return true if the resulting ILU is invertible, false if it is singular. @todo not implemented.
 */
bool matrix::makeILUC(const double *A, double *ILU, int n, int p)
{
    memcpy(ILU, A, n*n*sizeof(double));
    ILUC(A, ILU,n, p);
    for(int i=0; i<n; i++) if(fabs(ILU[i*n+i])<PRECISION) return false;
    return true;
}


/**
 * @brief ILUC performs the incomplete LU factorization of a square matrix using Crout's algorithm.
 * We only consider the elements in a band of width pIn in the input matrix, i.e. abs(i-j)<pIn.
 * The rest of the coefficients is ignored, i.e. assumed to be zero.
 * We only output the elements inside a band of width pOut, i.e. abs(i-j)<pOut
 * @param ILU the output matrix
 * @param n the size of the matrices.
 * @param p the dropping rule's parameter
 */
void matrix::ILUC(double const*A, double *ILU, int n, int p)
{
    /**  Algorithm 10.8    */
    for (int k=0; k<n; k++)                     // 1. For k = 1 : n Do :
    {
        for(int i=0; i<k; i++)                  // 2. For i = 1 : k − 1 and
        {
            if(fabs(A[k*n+i])>0.0)            // 2. ....    if aki!=0 Do :
            {
                for(int l=k; l<n; l++)          // 3. ak,k:n = ak,k:n − aki * ai,k:n
                {
                    ILU[k*n+l] = A[k*n+l] - ILU[k*n+i] * ILU[i*n+l];
                }
            }
        }
        for(int i=0; i<k; i++)                  // 5. For i = 1 : k − 1 and if aik!=0 Do :
        {
            for(int l=k+1; l<n; l++)            // 6. ak+1:n.k = ak+1:n,k − aik * ak+1:n,i
            {
                ILU[l*n +k] -= ILU[i*n+k] * ILU[l*n+i];
            }
        }
        for(int i=k+1; i<n; i++)                // 8. aik = aik / akk for i = k + 1, ..., n
        {
            ILU[i*n+k] = ILU[i*n+k]/ILU[k*n+k];
        }
    }

    /** apply the output drop rule
     * @todo include this in the above calculations
     * @todo apply pIn filter */
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j++)
        {
            if (abs(i-j)>p) ILU[i*n+j] = 0.0;
        }
    }
}


/** Builds the symmetric Gauss-Seidel preconditioner in LU form
 * @brief makeSGS
 * @param A
 * @param SGSLU
 * @param n
 * @param p
 * @return true if the resulting LU factorization is invertible, false if it is singular. @todo not implemented.
 */
bool matrix::makeSGS(double const *A, double *SGSLU, int n)
{
    for(int i=0; i<n; i++)
    {
        if(fabs(A[i*n+i])<PRECISION) return false;
    }

    memcpy(SGSLU, A, n*n*sizeof(double));

    // D is the diagonal of A, −E its strict lower part, and −F its strict upper part
    // L = I-E.D-1
    // U = D-F   --> is A

    // make L - is unit lower triangular lij = eij/djj
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<i; j++)
        {
            SGSLU[i*n+j] = A[i*n+j]/A[i*n+i];
        }
    }
    return true;
}



/**
 * @brief Performs the (fast) multiplication of a matrix by a vector.
 * @param A the nxn square matrix
 * @param V the column vector
 * @param n the size of the matrix and vecgor
 * @param res the resulting product
 */
void matrix::matVecMult(double const *A, double const *V, double *res, int nRows, int nCols)
{
    // Access matrix and vector elements by adresses rather than by indices
    // gives the compiler possibility for optimization

    // unroll two rows and four columns
    int half = nRows/2;
    double ytemp1(0);
    double ytemp2(0);
    for(int i=0; i+1<nRows; i+=2)
    {
        ytemp1=ytemp2=0;
        double const *xpos = V;

        double const *Apos1 = A +  i   *nCols;
        double const *Apos2 = A + (i+1)*nCols;

        int nBlocks = nCols / 8;
        double x0=0;
        double x1=0;

        for(int j=0; j<nBlocks; j++)
        {
            x0 = *(xpos+0);
            x1 = *(xpos+1);

            ytemp1 += x0 * *(Apos1+0);
            ytemp2 += x0 * *(Apos2+0);
            ytemp1 += x1 * *(Apos1+1);
            ytemp2 += x1 * *(Apos2+1);

            x0 = *(xpos+2);
            x1 = *(xpos+3);

            ytemp1 += x0 * *(Apos1+2);
            ytemp2 += x0 * *(Apos2+2);
            ytemp1 += x1 * *(Apos1+3);
            ytemp2 += x1 * *(Apos2+3);

            x0 = *(xpos+4);
            x1 = *(xpos+5);

            ytemp1 += x0 * *(Apos1+4);
            ytemp2 += x0 * *(Apos2+4);
            ytemp1 += x1 * *(Apos1+5);
            ytemp2 += x1 * *(Apos2+5);

            x0 = *(xpos+6);
            x1 = *(xpos+7);

            ytemp1 += x0 * *(Apos1+6);
            ytemp2 += x0 * *(Apos2+6);
            ytemp1 += x1 * *(Apos1+7);
            ytemp2 += x1 * *(Apos2+7);

            xpos  += 8;
            Apos1 += 8;
            Apos2 += 8;
        }

        Apos1 = A +  i   *nCols;
        Apos2 = A + (i+1)*nCols;

        for(int j=nBlocks*8; j<nCols; j++)
        {
            ytemp1 += (*(Apos1 +j)) * (*(V+j));
            ytemp2 += (*(Apos2 +j)) * (*(V+j));
        }

        res[i]   = ytemp1;
        res[i+1] = ytemp2;
    }

    if(2*half!=nRows)
    {
        // compute last row if odd number
        double const *Apos1 = A+2*half*nCols;
        double const *xpos = V;
        double ytemp=0;
        for(int j=0; j<nCols; j++)
        {
            ytemp += (*Apos1++) * (*xpos++);
        }
        res[2*half] = ytemp;
    }
}




