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

#include <cstring>
#include <cmath>


#include <rungekutta.h>
#include <constants.h>


/**
* Solves a multi-variable differential system using the 4th order Runge-Kutta method
*
* [x']  = [A][x] + f(t)[B]
*
* @param order the order of the differential system
* @param A the matrix operator of size order²
* @param B the control vector of size order; i.e. the input function is of the type f(t).B
* @param y0 the array initial values
* @param res the computed values
*/
void calcRungeKutta(int order, std::vector<double> const &y0,  double *A, double *B, double (*pFunc)(double),
                    std::vector<std::vector<double>> &result)
{
    int TotalPoints, PlotInterval;

    double t=0, dt=0, ctrl_t=0;

    std::vector<std::vector<double>> m(5);
    for(int j=0; j<5; j++)
    {
        m[j].resize(order);
        memset(m[j].data(), 0, order*sizeof(double));
    }

    std::vector<double> y(order), yp(order);

    memset(y.data(),  0, order*sizeof(double));
    memset(yp.data(), 0, order*sizeof(double));

    double deltat    = 0.005;
    double m_TotalTime = 1.0;
    dt = 0.005;
    if(dt<deltat) dt = deltat;

    TotalPoints  = std::min(1000, (int)(m_TotalTime/dt));
    PlotInterval = std::max(1,    (int)(TotalPoints/200));

    result.resize(order);
    for(int io=0; io<order; io++)
    {
        result[io].clear();
    }

    // we are considering forced response from initial steady state, so set
    // initial conditions to 0
    t = 0.0;

    for(int io=0; io<order; io++)
    {
        y[io] = y0.at(io);
        result[io].push_back(y0.at(io));
    }

    //Runge-Kutta method
    for(int i=0; i<TotalPoints; i++)
    {
        //initial slope m1
        for(int io=0; io<order; io++)
        {
            m[0][io] = A[io*order+0]*y[0] + A[io*order+1]*y[1] + A[io*order+2]*y[2] + A[io*order+3]*y[3];
        }
        ctrl_t = pFunc(t);

        for(int io=0; io<order; io++)
        {
          m[0][io] += B[io] * ctrl_t;
        }

        //middle point m2
        for(int io=0; io<order; io++)
        {
            yp[io] = y[io] + dt/2.0 * m[0][io];
        }

        for(int io=0; io<order; io++)
        {
            m[1][io] = A[io*order+0]*yp[0] + A[io*order+1]*yp[1] + A[io*order+2]*yp[2] + A[io*order+3]*yp[3];
        }

        ctrl_t = pFunc(t+dt/2.0);
        for(int io=0; io<order; io++)
        {
            m[1][0] += B[io] * ctrl_t;
        }

        //second point m3
        for(int io=0; io<order; io++)
        {
           yp[io] = y[io] + dt/2.0 * m[1][io];
        }

        for(int io=0; io<order; io++)
        {
            m[2][io] = A[io*order+0]*yp[0] + A[io*order+1]*yp[1] + A[io*order+2]*yp[2] + A[io*order+3]*yp[3];
        }

        ctrl_t = pFunc(t+dt/2.0);

        for(int io=0; io<order; io++)
        {
           m[2][io] += B[io] * ctrl_t;
        }

        //third point m4
        for(int io=0; io<order; io++)
        {
            yp[io] = y[io] + dt * m[2][io];
        }

        for(int io=0; io<order; io++)
        {
            m[3][io] = A[io*order+0]*yp[0] + A[io*order+1]*yp[1] + A[io*order+2]*yp[2] + A[io*order+3]*yp[3];
        }

        ctrl_t = pFunc(t+dt);

        for(int io=0; io<order; io++)
        {
            m[3][io] += B[io] * ctrl_t;
        }

        //final slope m5
        for(int io=0; io<order; io++)
        {
            m[4][io] = 1./6. * (m[0][io] + 2.0*m[1][io] + 2.0*m[2][io] + m[3][io]);
        }

        for(int io=0; io<order; io++)
        {
            y[io] += m[4][io] * dt;
        }

        t +=dt;
        if(fabs(y[0])>1.e10 || fabs(y[1])>1.e10 || fabs(y[2])>1.e10  || fabs(y[3])>1.e10 ) break;

        if(i%PlotInterval==0)
        {
            for(int io=0; io<order; io++)
                result[io].push_back(y[io]);
        }
    }
}


double ctrlfunc(double t)
{
    if(0.0<=t && t<1.0) return sin(t*PI);
    else                return 0.0;
}


double nullfunc(double)
{
    return 0.0;
}


void testRungeKutta()
{
    int order = 4;
    std::vector<double> y0(order);
    std::vector<double> A(order*order);
    std::vector<double> B(order);

    std::vector<std::vector<double>> result;

    A[0] =0.0374;    A[1] = -0.1155;    A[2] = 0.0;        A[3] = -9.81;
    A[4] =0.8384;    A[5] = -12.9214;   A[6] = 21.1977;    A[7] = 0.0;
    A[8] =0.0;       A[9] = -4.8587;    A[10]= -7.4173;    A[11]= 0.0;
    A[12]=0.0;       A[13]= 0.0;        A[14]= 1.0;        A[15]= 0.0;

    for(int io=0; io<order; io++) B[io] = 0.0;
    B[1]=0.1;

    for(int io=0; io<order; io++) y0[io]=0.0;
//    y0[1]=0.1;


    calcRungeKutta(order, y0, A.data(), B.data(), &ctrlfunc, result);

/*    for(uint i=0; i<result[0].size(); i++)
    {
        qDebug(" %13.7g   %13.7g   %13.7g   %13.7g  ", result[0][i], result[1][i], result[2][i], result[3][i]);
    }*/
}
