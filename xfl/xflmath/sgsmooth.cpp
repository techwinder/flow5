
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


#include <cstring>
#include <iostream>





#include "sgsmooth.h"
#include <xflmath/constants.h>
#include <xflmath/matrix.h>



void sgsmooth::makeCoefficients()
{
    std::vector<std::vector<double>> csm;
    csm.resize(10);
    for(uint i=0; i<=9; i++)
    {
        csm[i].resize(5);
        memset(csm[i].data(), 0, 5*sizeof(double));
    }
    csm[5][0]=17;   csm[5][1]=12;   csm[5][2]=-3;
    csm[7][0]=7;    csm[7][1]=6;    csm[7][2]=3;     csm[7][3]=-2;
    csm[9][0]=59;   csm[9][1]=54;   csm[9][2]=39;    csm[9][3]=14;    csm[9][4]=-21;
}

/** savitzky-golay */
void sgsmooth::smooth(int order, QVector<double> const &fin, QVector<double> &fout)
{
    if(order!=5 && order !=7 && order!=9) order=5;
    int N = fin.size();
    fout.resize(fin.size());

    for(int i=0; i<fin.size(); i++)
    {
        //use 5-point quadratc polynomial
        if(order==5)
        {
            if(i>2 && i<N-2)
                fout[i] = (-3.0*fin[i-2] +12.0*fin[i-1] +17*fin[i] +12.0*fin[i+1]-3.0*fin[i+2])/35.0;
            else if(i==1 || i==N-2)
                fout[i] = (fin[i-1] +2.0*fin[i] +fin[i+1] )/4.0; // empirical
            else
                fout[i] = fin[i];
        }
        else if(order==7)
        {
            //use 7-point quadratc polynomial
            if(i==0 || i==N-1)
                fout[i] = fin[i];
            else if(i<3 || i>N-4)
            {
                fout[i] = (fin[i-1] +2.0*fin[i] +fin[i+1] )/4.0; // empirical
            }
            else
            {
                fout[i] = -2.0*fin[i-3]+3.0*fin[i-2] +6.0*fin[i-1]
                               +7*fin[i]
                          -2.0*fin[i+3]+3.0*fin[i+2] +6.0*fin[i+1] ;
                fout[i] = fout[i]/21.0;
            }
        }
        else if(order==9)
        {
            //use 7-point quadratc polynomial
            if(i==0 || i==N-1)
                fout[i] = fin[i];
            else if(i<4 || i>N-5)
            {
                fout[i] = (fin[i-1] +2.0*fin[i] +fin[i+1] )/4.0; // empirical
            }
            else
            {
                fout[i] = -21.0*fin[i-4]+14.0*fin[i-3]+39.0*fin[i-2]+54.0*fin[i-1]
                                +59*fin[i]
                          -21.0*fin[i+4]+14.0*fin[i+3]+39.0*fin[i+2]+54.0*fin[i+1];
                fout[i] = fout[i]/231.0;
            }
        }
    }
}



/**
 * @brief smooth_nonuniform
 * Implements the method described in  https://dsp.stackexchange.com/questions/1676/savitzky-golay-smoothing-filter-for-not-equally-spaced-data
 * @param n the half size of the smoothing sample
 * @param the degree of the local polynomial fit, e.g. deg=2 for a parabolic fit
 */
bool sgsmooth::smooth_nonuniform(int deg, int n, QVector<double>const &x, QVector<double> const &y, QVector<double>&ysm)
{
    if(x.size()<5)         return false;
    if(x.size()!=y.size()) return false; // don't even try
    if(x.size()<=2*n)      return false; // not enough data to start the smoothing process
//    if(2*n+1<=deg+1)       return false; // need at least deg+1 points to make the polynomial

    int m0 = 2*n+1; // the size of the filter window
    int o0 = deg+1; // the smoothing order

    // adjusted order for data beginning and end
    int o = o0;
    int w = n;
    int m = m0;

    QVector<double> A(m*o, 0);
    QVector<double> tA(m*o, 0);
    QVector<double> tAA(o*o,0 );

    QVector<double> t(m, 0);

    int sz = y.size();
    ysm.resize(sz);           memset(ysm.data(), 0, ulong(sz)*sizeof(double));

    // do not smooth start and end data
    for(int i=0; i<n+2; i++)
    {
        ysm[i] = y[i];
        ysm[sz-i-1] = y[sz-i-1];
    }

    // start smoothing
    for(int i=n+2; i<x.size()-n-2; i++)
    {
        // adjust order at beginning and end
        o = std::min(i, o0);
        o = std::min(o, int(x.size()-i-1));

        // adjust window size at beginning and end
        // need at least o point to smooth at degree deg
        w = std::min(n, o);
//        w = std::min(w, x.size()-i-1);
        m = 2*w+1;
//qDebug("  %3d  %3d  %3d ", i, o, w);

        // make A and tA
        for(int j=0; j<m; j++)
        {
            t[j] = x[i+j-w] - x[i];
        }
        for(int j=0; j<m; j++)
        {
            double r = 1.0;
            for(int k=0; k<o; k++)
            {
                A[j*o+k] = r;
                tA[k*m+j] = r;
                r *= t[j];
            }
        }

        // make tA.A
        matMult(tA.data(), A.data(), tAA.data(), o, m, o);

        // make (tA.A)-¹ in place
        if(!invertMatrix(tAA, o)) return false;

        // make (tA.A)-¹.tA
        matMult(tAA.data(), tA.data(), A.data(), o, o, m); // re-uses memory allocated for matrix A

        // compute the polynomial's value at the center of the sample
        ysm[i] = 0.0;
        for(int j=0; j<m; j++)
        {
            ysm[i] += A[j]*y[i+j-w];
        }
    }

    return true;
}


/**
 * @brief testSmooth
 * @param order 5, 7 or 9
 */
void testSmooth(QVector<double> &f, QVector<double> &g5,QVector<double> &g7,QVector<double> &g9)
{
    int N = f.size();

    for(int i=0; i<N; i++)
    {
        double di = double(i)/double(N-1);
        f[i] = 1.0*cos(di);
        f[i] += 2.0*sin(di*100.0);
        f[i] += (double(rand())/double(RAND_MAX)-0.5);
    }

    sgsmooth::smooth(5, f, g5);
    sgsmooth::smooth(7, f, g7);
    sgsmooth::smooth(9, f, g9);

//    qDebug("         n            f           f5           f7           f9");
//    for(int i=0; i<N; i++)  qDebug(" %u %13.5g  %13.5g  %13.5g  %13.5g",i,f[i],g5[i],g7[i],g9[i]);
}


void testSmoothNonUniform()
{
    int N = 200;
    double dN = double(N);
    QVector<double> x0(N);
    QVector<double> y0(N);

    for(int i=0; i<N; i++)
    {
//                    x0[i] =  double(i)/double(N-1);
        x0[i]  =  double(i);
        x0[i] +=  1.0*(double(rand())/double(RAND_MAX)-0.5); // uneven spacing

        y0[i]  = 0.5*cos(x0[i]/dN*5.0*PI);   //  low freq
        y0[i] += 1.0*sin(x0[i]/dN*17.0*PI);   //  high(er) freq
        y0[i] += 1.0*(double(rand())/double(RAND_MAX)-0.5); // the noise
    }

    QVector<double> y;
    int degree = 4;
    int nPts = 12;
    bool bSmooth = sgsmooth::smooth_nonuniform(degree, nPts, x0, y0, y);
    if(!bSmooth)
    {
        QString strange;
        strange = QString::asprintf("  Smoothing failed deg=%u  nPts=%u\n",degree, nPts);
        return;
    }
}
