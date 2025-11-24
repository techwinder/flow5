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
#include <QString>

#include <iostream>
#include <cstring>

#include "testmatrix.h"
#include <matrix.h>

#include <complex>
#include <chrono>


#if defined ACCELERATE
  #include <Accelerate/Accelerate.h>
  #define lapack_int int
#elif defined INTEL_MKL
    #include <mkl.h>
#elif defined OPENBLAS
    #include <openblas/lapacke.h>
#endif




#include <constants.h>


void makeMatrix(double *A, int n)
{
    for(int i=0; i<n; i++)
        for(int j=0; j<n; j++)
            A[i*n+j] = 1.0+1.0/(1.0+double(i-j)*double(i-j)) + 3 * cos(double(i)*PI/double(n));
}


void makeRHS(double *b, int n, int nrhs)
{
    for(int j=0; j<nrhs; j++)
    {
        for(int i=0; i<n; i++)
        {
            b[j*n+i] = 1.17156+1.0/(1.0+i*i) * cos(double(j)*3.*PI + 0.08481);
        }
    }
}


void testMatrix()
{
    int n=3;
    std::vector<double> A(n*n, 0), AMem(n*n, 0);

    for(int i=0; i<n; i++)
        for(int j=0; j<n; j++)
            A[i*n+j]=1.0/(0.7+(2.0*i-j)*(i-j+1));
    std::cout<<"A="<<std::endl;

    std::string out;
    matrix::display_mat(A.data(),n,n,out);
    std::cout<< out << std::endl;

    AMem = A;

    std::vector<double>A1(n*n,0);

    if(n==2)
    {
        if(!matrix::invert22(A.data())) return;
    }
    else if(n==3)
    {
        if(!matrix::invert33(A.data())) return;
    }
    else
    {
        if(!matrix::inverseMatrixLapack(n, A)) return;
    }

    std::cout << "A-1=" << std::endl;
    matrix::display_mat(A1.data(),n,n,out);
    std::cout << out << std::endl;

    std::cout << " "<< std::endl;
    std::cout << "A.A-1="<< std::endl;
    std::vector<double>AA(n*n,0);
    matrix::matMult(AMem.data(), A1.data(), AA.data(), n, 1);
    matrix::display_mat(AA.data(),n,n,out);
    std::cout << out << std::endl;
    std::cout << "_______"<< std::endl;
}


void testMatMult(int nThreads)
{
    std::string out;
    int m = 7;
    int n = 9;
    int q = 5;

    std::vector<double> A(m*n, 0);

    for(int i=0; i<m; i++)
        for(int j=0; j<n; j++)
            A[i*n+j]=1.0/(5.1+(2.0*double(i)-double(j))*double(i-j+1));
    std::cout << "A=" << std::endl;
    matrix::display_mat(A.data(),m,n,out);
    std::cout << out << std::endl;

    std::vector<double> B(n*q, 0);

    for(int i=0; i<n; i++)
        for(int j=0; j<q; j++)
            B[i*q+j]=1.0+double(i-j);
    std::cout <<"B=" << std::endl;
    matrix::display_mat(B.data(),n,q, out);
    std::cout << out << std::endl;

    std::vector<double> AB(m*q, 0);

    matrix::matMult(A.data(),B.data(), AB.data(), m, n, q, nThreads);
    std::cout << "ABflt=" << std::endl;
    matrix::display_mat(AB.data(),m,q,out);
    std::cout << out  << std::endl;


    matrix::matMultLAPACK(A.data(), B.data(), AB.data(), m, n, q, nThreads);
    std::cout << "ABflt=" << std::endl;
    matrix::display_mat(AB.data(),m,q,out);
    std::cout << out  << std::endl;
}



void testMatVecMult(int nThreads)
{
    int m = 500;
    int n = 11;
    int q = 1;

    std::vector<double> A(m*n, 0);

    for(int i=0; i<m; i++)
        for(int j=0; j<n; j++)
            A[i*n+j]=1.0/(5.1+(2.0*double(i)-double(j))*double(i-j+1));
//    std::cout << "A=");
//    display_mat(A.data(),m,n);

    std::vector<double> B(n*q, 0);

    for(int i=0; i<n; i++)
        for(int j=0; j<q; j++)
            B[i*q+j]=1.0+double(i-j);
//    std::cout << )<<"B=";
//    display_mat(B.data(),n,q);

    std::vector<double> AB0(m*q);
    std::vector<double> AB1(m*q);

    auto t0 = std::chrono::high_resolution_clock::now();
    matrix::matVecMultLapack(A.data(), B.data(), AB0.data(), m, n, nThreads);
    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "matVecMultLapack " << double(duration)/1000.0 << " ms" << std::endl;

    auto t2 = std::chrono::high_resolution_clock::now();
    matrix::matVecMult(      A.data(), B.data(), AB1.data(), m, n);
    auto t3 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
    std::cout << "matVecMult " << double(duration)/1000.0 << " ms"  << std::endl;;

//    std::cout << "ABdble=");
//    displayArrays(AB0,AB1);
}


void testMatrixInverse()
{
    std::string out;

    int n=4;
    std::vector<double> A(n*n,0), AMem(n*n,0);

    for(int i=0; i<n; i++)
        for(int j=0; j<n; j++)
            A[i*n+j]=1.0/(0.7+(2.0*i-j)*(i-j+1));
    std::cout << "A=" << std::endl;
    matrix::display_mat(A.data(),n,n,out);
    std::cout << out  << std::endl;

    AMem = A;

    std::vector<double>A1(n*n);
    matrix::invertMatrix(A,n);
    std::cout << "A-1=" << std::endl;
    matrix::display_mat(A.data(),n,n,out);
    std::cout << out  << std::endl;
    A1 = A;


    std::cout << " " << std::endl;
    std::cout << "A.A-1=" << std::endl;
    std::vector<double>AA(n*n);
    matrix::matMult(AMem.data(), A1.data(), AA.data(), n, n, n, -1);
    matrix::display_mat(AA.data(),n,n,out);
    std::cout << out  << std::endl;

    std::cout << "_______" << std::endl;
}



void testBlockThomas()
{
    int n=5;
    int p=3;
    std::vector<std::vector<double>> A(n), B(n), C(n);
    std::vector<std::vector<double>> R(n), X(n);
    for(int i=0; i<n; i++)
    {
        A[i].resize(p*p);
        B[i].resize(p*p);
        C[i].resize(p*p);
        R[i].resize(p);
        X[i].resize(p);
    }

    for(int i=0; i<n; i++)
    {
        for(int k=0; k<p*p; k++)
        {
            B[i][k]= 1.0+sin(double(i+k));

            A[i][k]= double(i+k+7);

            C[i][k]= double(i-k-7);

        }
        for(int k=0; k<p; k++)
            R[i][k] = double(2*i+k);
    }

/*    std::cout << "A:");
    for(int i=0; i<n; i++) display_mat(A[i], p);
    std::cout << "B:");
    for(int i=0; i<n; i++) display_mat(B[i], p);
    std::cout << "AC:");
    for(int i=0; i<n; i++) display_mat(C[i], p);*/
    std::cout << "R:" << std::endl;
    std::string out;

    for(int i=0; i<n; i++)
    {
        matrix::display_vec(R[i].data(), p, out);
        std::cout << out  << std::endl;
    }

    int o=p*n;
    std::vector<double>M(o*o,0);
    std::vector<double>H(o,0);

    for(int i=0; i<n; i++)
    {
        for(int j=0; j<p; j++) // rows of submatrix
        {
            for(int k=0; k<p; k++) // columns of submatrix
            {
                if(i>0)   M[(i*p+j)*o-p + i*p+ k]  = A[i][j*p+k];
                M[(i*p+j)*o   + i*p +k]  = B[i][j*p+k];
                if(i<n-1) M[(i*p+j)*o+p + i*p+ k]  = C[i][j*p+k];
            }
        }

        //make the RHS
        for(int j=0; j<p; j++)
            H[i*p+j] = R[i][j];
    }

    std::cout << "Mat:" << std::endl;
    matrix::display_mat(M.data(),p,n,out); // untested
    std::cout << out  << std::endl;

    if(!matrix::blockThomasSolve(A,B,C,R,X,n,p))
    {
        std::cout << " " << std::endl;
        std::cout << "***********Singular matrix***********" << std::endl;
        std::cout << " " << std::endl;
    }
    else
    {
        for(int i=0; i<n; i++)
        {
            for(int j=0; j<p; j++) std::cout << QString::asprintf(" %13.7g", X[i][j]).toStdString() << std::endl;
        }
    }
}


bool testCholevski4()
{
    int n=4;
    double mat[16] = {18, 22,  54,  42,
                      22, 70,  86,  62,
                      54, 86, 174, 134,
                      42, 62, 134, 106};

    double L[16];

    double b[4] = {1, 2, 3, 4};

    matrix::CholevskiFactor(mat, L, n);
    matrix::CholevskiSolve(L, b, n);

    std::string out;
    matrix::display_vec(b,4, out);
    std::cout << out  << std::endl;

    return true;
}




void testLapacke()
{
    std::string out;

    char trans = 'N';
    int dim = 4;
    int nrhs = 1;
    int LDA = dim;
    int LDB = dim;
    int info;

    double A[16] = {18, 22,  54,  42,
                    22, 70,  86,  62,
                    54, 86, 174, 134,
                    42, 62, 134, 106};

    double B[4] = {1, 2, 3, 4};

    std::cout << "A=" << std::endl;
    matrix::display_mat(A,4,4, out);
    std::cout << out  << std::endl;
    matrix::display_vec(B, 4,out);
    std::cout << out  << std::endl;

    double AMem[16];
    memcpy(AMem, A, 16*sizeof(double));

    int ipiv[4];

    dgetrf_(&dim, &dim, A, &LDA, ipiv, &info);
#ifdef OPENBLAS
    dgetrs_(&trans, &dim, &nrhs, A, &LDA, ipiv, B, &LDB, &info, 1);
#elif defined INTEL_MKL
    dgetrs_(&trans, &dim, &nrhs, A, &LDA, ipiv, B, &LDB, &info);
#elif defined ACCELERATE
    dgetrs_(&trans, &dim, &nrhs, A, &LDA, ipiv, B, &LDB, &info);
#endif



   std::cout << "X:" << std::endl;
   matrix::display_vec(B, 4, out);
   std::cout << out  << std::endl;
   double AX[4]{0};
   matrix::matVecMultLapack(AMem, B, AX, 4, 1);

   std::cout << "AX=" << std::endl;
   matrix::display_vec(AX, 4,out);
   std::cout << out  << std::endl;
}


void testLapacke3()
{
    lapack_int n(0), nrhs(0), lda(0), ldb(0), info(0);
    std::string out;

    /** Default Value */
    n = 5; nrhs = 1;

    /** Local arrays */
    std::vector<double>A(n*n,0);
    std::vector<double>b(n*nrhs,0);
    std::vector<lapack_int>ipiv(n,0);


    lda=n;
    ldb=n;

    for(int i=0; i<n; i++) {
        for(int j=0; j<n; j++) A[i+j*lda] = double(rand()) / double(RAND_MAX) - 0.5;
    }

    for(int i=0;i<n*nrhs;i++)
        b[i] = (double(rand())) / (double(RAND_MAX)) - 0.5;

    /** Print Entry Matrix */
    std::cout << "A:" << std::endl;
    matrix::display_mat(A.data(), n, n, out);
    std::cout << out  << std::endl;

    std::cout << "b:" << std::endl;
    matrix::display_mat(b.data(),n,nrhs,out);
    std::cout << out  << std::endl;

    /** Executable statements */
    std::cout <<  "LAPACKE_dgesv (row-major, high-level) Example Program Results\n"  << std::endl;

    /** Solve the equations A*X = B */

    dgesv_(&n, &nrhs, A.data(), &lda, ipiv.data(), b.data(), &ldb, &info);

    /** Check for the exact singularity */
    if(info>0)
    {
        printf( "The diagonal element of the triangular factor of A,\n" );
        printf( "U(%i,%i) is zero, so that A is singular;\n", info, info );
        printf( "the solution could not be computed.\n" );
        return;
    }
    if (info <0) return;

    /** Print solution */
    std::cout << "x:" << std::endl;
    matrix::display_mat(b.data(),n,nrhs,out);
    std::cout << out  << std::endl;

    /** Print details of LU factorization */
    //print_matrix_colmajor( "Details of LU factorization", n, n, A, lda );
    /** Print pivot indices */
    //       print_vector( "Pivot indices", n, ipiv );

    /** End of LAPACKE_dgesv Example */
}

/** col major exameple - not of much use*/
void testLapacke4(int rank)
{
    lapack_int n=0, nrhs=0, lda=0, ldb=0, info=0;
    std::string out;

    /** Default Value */
    n = rank; nrhs = 1;

    /** Local arrays */
    std::vector<lapack_int>ipiv(n);
    std::vector<double>A(n*n);
    for(int i=0; i<n; i++)
        for(int j=0; j<n; j++)
            A[i*n+j]=1.0/(1.0+(i-j)*(i-j)) +3*i-j;


    std::vector<double>b(n);
    for(int i=0; i<n; i++) b[i]=1.0+1.0/(1.0+i*i);

    lda=n;
    ldb=n;

    /** Print Entry Matrix */
    std::cout << "A:" << std::endl;
    matrix::display_mat(A.data(), n, n, out);
    std::cout << out  << std::endl;
    std::cout << "b:" << std::endl;
    matrix::display_mat(b.data(),n,nrhs,out);
    std::cout << out  << std::endl;

    /** Executable statements */
    std::cout <<  "LAPACKE_dgesv (col-major, high-level) Example Program Results\n"  << std::endl;

    /** Solve the equations A*X = B */
    dgesv_(&n, &nrhs, A.data(), &lda, ipiv.data(), b.data(), &ldb, &info);

    /** Check for the exact singularity */
    if(info>0)
    {
        std::cout <<  "The diagonal element of the triangular factor of A,\n"  << std::endl;
        std::cout <<  QString::asprintf("U(%d,%d) is zero, so that A is singular;\n", info, info).toStdString() << std::endl;
        std::cout <<  "the solution could not be computed.\n"  << std::endl;
        return;
    }
    if (info <0) return;

    /** Print solution */
    std::cout << "x:" << std::endl;
    matrix::display_mat(b.data(),n,nrhs,out);
    std::cout << out  << std::endl;

    /** Print details of LU factorization */
    //print_matrix_colmajor( "Details of LU factorization", n, n, A, lda );
    /** Print pivot indices */
    //       print_vector( "Pivot indices", n, ipiv );
    /** End of LAPACKE_dgesv Example */
}


#define N 5
#define NRHS 3
#define LDA N
#define LDB NRHS


/** linear regression */
void testLapacke12()
{
    std::string out;
    // Compute coefficients c0..c2 for a set of 4 equations
    // c0 + c1.x + c2.y = mu

    // a has 4 rows, on for each mu
    double a[15] = {1,-1, 2,
                    1, 3, 4,
                    1, 5, 2,
                    1, 2, 2};

    // 4 doublet values
    double mu[4] = {3.5,18,14,10};

    char trans = 'N';
    lapack_int info,m,n,lda,ldb,nrhs;
    m = 4;
    n = 3;
    nrhs = 1;
    lda = 3;
    ldb = 1;

    lapack_int lwork = m;
    std::vector<double> work(m*m);
#ifdef OPENBLAS
    dgels_(&trans,&m,&n,&nrhs,a,&lda,mu,&ldb, work.data(), &lwork, &info, 1);
#elif defined INTEL_MKL
    dgels_(&trans,&m,&n,&nrhs,a,&lda,mu,&ldb, work.data(), &lwork, &info);
#elif defined ACCELERATE
    dgels_(&trans,&m,&n,&nrhs,a,&lda,mu,&ldb, work.data(), &lwork, &info);
#endif
    if( info > 0 ) {
        std::cout <<  "The diagonal element of the triangular factor of A,\n"  << std::endl;
        std::cout <<  QString::asprintf("U%d,%d) is zero, so that A is singular;\n", info, info).toStdString()  << std::endl;
        std::cout <<  "the solution could not be computed.\n"  << std::endl;
        return;
    }
    /* Print least squares solution */
    matrix::display_mat(mu,3,1,out);
    std::cout << out  << std::endl;
}


void testSolve(int n)
{
    std::string out;
    double *A    = new double[n*n];
    double *Amem = new double[n*n];
    for(int i=0; i<n; i++)
        for(int j=0; j<n; j++)
            A[i*n+j]=1.0/(1.0+(i-j)*(i-j)) +3*i-j;

    memcpy(Amem, A, n*n*sizeof(double));

    int nrhs = 1;
    double *x = new double[nrhs*n];
    double *b = new double[nrhs*n];
    for(int i=0; i<n; i++)    b[i]=1.0+1.0/(1.0+i*i);
//    for(int i=n; i<2*n; i++)  b[i]=7.0+1.0/(3.0-i*i);



    std::cout << "___A___" << std::endl;
    matrix::display_mat(A,n, n, out);
    std::cout << out  << std::endl;
    std::cout << "                  ___b____" << std::endl;
    matrix::display_vec(b,n, out);
    std::cout << out  << std::endl;

    memcpy(x, b, ulong(nrhs*n)*sizeof(double));

    if(n==2)
    {
        if(!matrix::solve2x2(A, x, nrhs))
        {
            std::cout << "singular matrix" << std::endl;
        }
    }
    else if(n==3)
    {
        if(!matrix::solve3x3(A, x, nrhs))
        {
            std::cout << "singular matrix" << std::endl;
        }
    }
    else if(n==4)
    {
        if(!matrix::solve4x4(A, x, nrhs))
        {
            std::cout << "singular matrix" << std::endl;
        }
    }

    std::cout <<"                _____x_____" << std::endl;
    matrix::display_vec(x,n,out);
    std::cout << out  << std::endl;

    matrix::matVecMult(Amem, x, b, n);

    std::cout <<"                _____Ax____" << std::endl;
    matrix::display_vec(b,n,out);
    std::cout << out  << std::endl;

    delete[] A;
    delete[] Amem;
    delete[] x;
    delete[] b;

}






