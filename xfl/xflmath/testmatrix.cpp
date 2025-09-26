/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QDebug>

#include "testmatrix.h"
#include "matrix.h"

#include <complex>
#include <chrono>


#if defined ACCELERATE
  #include <Accelerate/Accelerate.h>
  #define lapack_int int
#elif defined INTEL_MKL
    #include <mkl.h>
#elif defined OPENBLAS
//    #include <cblas.h>
    #include <openblas/lapacke.h>
#endif




#include <xflmath/constants.h>


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
    QVector<double> A(n*n, 0), AMem(n*n, 0);

    for(int i=0; i<n; i++)
        for(int j=0; j<n; j++)
            A[i*n+j]=1.0/(0.7+(2.0*i-j)*(i-j+1));
    qDebug("A=");
    display_mat(A.data(),n,n);

    AMem = A;

    QVector<double>A1(n*n,0);

    if(n==2)
    {
        if(!invert22(A.data())) return;
    }
    else if(n==3)
    {
        if(!invert33(A.data())) return;
    }
    else
    {
        if(!inverseMatrixLapack(n, A)) return;
    }

    qDebug("A-1=");
    display_mat(A1.data(),n,n);

    qDebug(" ");
    qDebug("A.A-1=");
    QVector<double>AA(n*n,0);
    matMult(AMem.data(), A1.data(), AA.data(), n, 1);
    display_mat(AA.data(),n,n);
    qDebug("_______");
}


void testMatMult(int nThreads)
{
    int m = 7;
    int n = 9;
    int q = 5;

    QVector<double> A(m*n, 0);

    for(int i=0; i<m; i++)
        for(int j=0; j<n; j++)
            A[i*n+j]=1.0/(5.1+(2.0*double(i)-double(j))*double(i-j+1));
    qDebug("A=");
    display_mat(A.data(),m,n);

    QVector<double> B(n*q, 0);

    for(int i=0; i<n; i++)
        for(int j=0; j<q; j++)
            B[i*q+j]=1.0+double(i-j);
    qDebug()<<"B=";
    display_mat(B.data(),n,q);

    QVector<double> AB(m*q, 0);

    matMult(A.data(),B.data(), AB.data(), m, n, q, nThreads);
    qDebug("ABflt=");
    display_mat(AB.data(),m,q);


    matMultLAPACK(A.data(), B.data(), AB.data(), m, n, q, nThreads);
    qDebug("ABflt=");
    display_mat(AB.data(),m,q);
}



void testMatVecMult(int nThreads)
{
    int m = 500;
    int n = 11;
    int q = 1;

    QVector<double> A(m*n, 0);

    for(int i=0; i<m; i++)
        for(int j=0; j<n; j++)
            A[i*n+j]=1.0/(5.1+(2.0*double(i)-double(j))*double(i-j+1));
//    qDebug("A=");
//    display_mat(A.data(),m,n);

    QVector<double> B(n*q, 0);

    for(int i=0; i<n; i++)
        for(int j=0; j<q; j++)
            B[i*q+j]=1.0+double(i-j);
//    qDebug()<<"B=";
//    display_mat(B.data(),n,q);

    QVector<double> AB0(m*q);
    QVector<double> AB1(m*q);

    auto t0 = std::chrono::high_resolution_clock::now();
    matVecMultLapack(A.data(), B.data(), AB0.data(), m, n, nThreads);
    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    qDebug("matVecMultLapack %g ms", double(duration)/1000.0);

    auto t2 = std::chrono::high_resolution_clock::now();
    matVecMult(      A.data(), B.data(), AB1.data(), m, n);
    auto t3 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
    qDebug("matVecMult %g ms", double(duration)/1000.0);

//    qDebug("ABdble=");
//    displayArrays(AB0,AB1);
}


void testMatrixInverse()
{
    int n=4;
    QVector<double> A(n*n,0), AMem(n*n,0);

    for(int i=0; i<n; i++)
        for(int j=0; j<n; j++)
            A[i*n+j]=1.0/(0.7+(2.0*i-j)*(i-j+1));
    qDebug("A=");
    display_mat(A.data(),n,n);

    AMem = A;

    QVector<double>A1(n*n);
    invertMatrix(A,n);
    qDebug("A-1=");
    display_mat(A.data(),n,n);
    A1 = A;


    qDebug(" ");
    qDebug("A.A-1=");
    QVector<double>AA(n*n);
    matMult(AMem.data(), A1.data(), AA.data(), n, n, n, -1);
    display_mat(AA.data(),n,n);
    qDebug("_______");
}



void testBlockThomas()
{
    int n=5;
    int p=3;
    QVector<QVector<double>> A(n), B(n), C(n);
    QVector<QVector<double>> R(n), X(n);
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

/*    qDebug("A:");
    for(int i=0; i<n; i++) display_mat(A[i], p);
    qDebug("B:");
    for(int i=0; i<n; i++) display_mat(B[i], p);
    qDebug("AC:");
    for(int i=0; i<n; i++) display_mat(C[i], p);*/
    qDebug("R:");
    for(int i=0; i<n; i++) display_vec(R[i].data(), p);

    int o=p*n;
    QVector<double>M(o*o,0);
    QVector<double>H(o,0);

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

    qDebug("Mat:");
    display_mat(M.data(),o);

    if(!blockThomasSolve(A,B,C,R,X,n,p))
    {
        qDebug(" ");
        qDebug("***********Singular matrix***********");
        qDebug(" ");
    }
    else
    {
        for(int i=0; i<n; i++)
        {
            for(int j=0; j<p; j++) qDebug(" %13.7g", X[i][j]);
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

    CholevskiFactor(mat, L, n);
    CholevskiSolve(L, b, n);

    display_vec(b,4);
    return true;
}




void testLapacke()
{
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

    qDebug("A=");
    display_mat(A,4,4);
    display_vec(B, 4);

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



   qDebug("X:");
   display_vec(B, 4);
   double AX[4]{0};
   matVecMultLapack(AMem, B, AX, 4, 1);

   qDebug("AX=");
   display_vec(AX, 4);
}




void testLapacke3()
{
    lapack_int n, nrhs, lda, ldb, info;

    /** Default Value */
    n = 5; nrhs = 1;

    /** Local arrays */
    QVector<double>A(n*n,0);
    QVector<double>b(n*nrhs,0);
    QVector<lapack_int>ipiv(n,0);


    lda=n;
    ldb=n;

    for(int i=0; i<n; i++) {
        for(int j=0; j<n; j++) A[i+j*lda] = double(rand()) / double(RAND_MAX) - 0.5;
    }

    for(int i=0;i<n*nrhs;i++)
        b[i] = (double(rand())) / (double(RAND_MAX)) - 0.5;

    /** Print Entry Matrix */
    qDebug("A:");
    display_mat(A.data(), n, n);
    qDebug("b:");
    display_mat(b.data(),n,nrhs);

    /** Executable statements */
    qDebug( "LAPACKE_dgesv (row-major, high-level) Example Program Results\n" );

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
    qDebug("x:");
    display_mat(b.data(),n,nrhs);

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

    /** Default Value */
    n = rank; nrhs = 1;

    /** Local arrays */
    QVector<lapack_int>ipiv(n);
    QVector<double>A(n*n);
    for(int i=0; i<n; i++)
        for(int j=0; j<n; j++)
            A[i*n+j]=1.0/(1.0+(i-j)*(i-j)) +3*i-j;


    QVector<double>b(n);
    for(int i=0; i<n; i++) b[i]=1.0+1.0/(1.0+i*i);

    lda=n;
    ldb=n;

    /** Print Entry Matrix */
    qDebug("A:");
    display_mat(A.data(), n, n);
    qDebug("b:");
    display_mat(b.data(),n,nrhs);

    /** Executable statements */
    qDebug( "LAPACKE_dgesv (col-major, high-level) Example Program Results\n" );

    /** Solve the equations A*X = B */
    dgesv_(&n, &nrhs, A.data(), &lda, ipiv.data(), b.data(), &ldb, &info);

    /** Check for the exact singularity */
    if(info>0)
    {
        qDebug( "The diagonal element of the triangular factor of A,\n" );
        qDebug( "U(%i,%i) is zero, so that A is singular;\n", info, info );
        qDebug( "the solution could not be computed.\n" );
        return;
    }
    if (info <0) return;

    /** Print solution */
    qDebug("x:");
    display_mat(b.data(),n,nrhs);

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
    QVector<double> work(m*m);
#ifdef OPENBLAS
    dgels_(&trans,&m,&n,&nrhs,a,&lda,mu,&ldb, work.data(), &lwork, &info, 1);
#elif defined INTEL_MKL
    dgels_(&trans,&m,&n,&nrhs,a,&lda,mu,&ldb, work.data(), &lwork, &info);
#elif defined ACCELERATE
    dgels_(&trans,&m,&n,&nrhs,a,&lda,mu,&ldb, work.data(), &lwork, &info);
#endif
    if( info > 0 ) {
        qDebug( "The diagonal element of the triangular factor of A,\n" );
        qDebug( "U(%i,%i) is zero, so that A is singular;\n", info, info );
        qDebug( "the solution could not be computed.\n" );
        return;
    }
    /* Print least squares solution */
    display_mat(mu,3,1);
}


void testSolve(int n)
{
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



    qDebug("___A___");
    display_mat(A,n, n);
    qDebug()<<"                  ___b____";
    display_vec(b,n);

    memcpy(x, b, ulong(nrhs*n)*sizeof(double));

    if(n==2)
    {
        if(!solve2x2(A, x, nrhs))
        {
            qDebug("singular matrix");
        }
    }
    else if(n==3)
    {
        if(!solve3x3(A, x, nrhs))
        {
            qDebug("singular matrix");
        }
    }
    else if(n==4)
    {
        if(!solve4x4(A, x, nrhs))
        {
            qDebug("singular matrix");
        }
    }

    qDebug()<<"                _____x_____";
    display_vec(x,n);

    matVecMult(Amem, x, b, n);

    qDebug()<<"                _____Ax____";
    display_vec(b,n);

    delete[] A;
    delete[] Amem;
    delete[] x;
    delete[] b;

}
