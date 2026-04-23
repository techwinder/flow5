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
#include <iostream>
#include <QString>


#include <cubicinterpolation.h>


#include <constants.h>
#include <matrix.h>

/**
// Given an array of n+1 pairs (x[i], y[i]), with i ranging from 0 to n,
// this function calculates the 3rd order cubic spline which interpolate the pairs.
//
// The spline is defined for each interval [x[j], x[j+1]) by a third order polynomial function
//              p_j(x) = ax3 + bx2 + cx + d
// for a total of 4n unknowns
//
// The equations to determine the coefficients a,b,c,d are
//
// Interpolation : 2n conditions
//    p_j(x[j])   = y[j];
//    p_j(x[j+1]) = y[j+1];
//
// Continuity of 1st and 2nd order derivatives at internal points: 2(n-1) conditions
//    p_j'(x[j]) = p_j+1'(x[j])
//    p_j"(x[j]) = p_j+1"(x[j])
//
// BC - option 1
//   Second order derivative is zero at the end points : 2 conditions
//      p_j"(x[0]) =  p_j"(x[n]) =0
// BC - option 2
//   First order derivative are specified at the end points : 2 conditions
//      p_j'(x[0]) = fp0
//      p_j'(x[n]) = fp1
//
//
// This sets a linear system of size 4n which is solved for coefs a,b,c and d
// The RHS vector is
//      a[0]
//      b[0]
//      c[0]
//      d[0]
//      a[1]
//    ...
//      d[n-1]
*/
CubicInterpolation::CubicInterpolation()
{
    clearPoints();
    m_bSingularSpline = true;
    m_b2ndDerivativeLeft = m_b2ndDerivativeRight = true;
    m_fp0 = m_fp1 = 0.0;
}


CubicInterpolation::~CubicInterpolation()
{
    clearPoints();
}


void CubicInterpolation::clearPoints()
{
    x.clear();
    y.clear();

    a.clear();
    b.clear();
    c.clear();
    d.clear();
}


void CubicInterpolation::appendPoint(float xp, float yp)
{
    x.push_back(xp);
    y.push_back(yp);
}


void CubicInterpolation::setPoints(float const *xin, float const *yin, int n)
{
    x.resize(n);
    y.resize(n);
    for(int i=0; i<n; i++)
    {
        x[i] = xin[i];
        y[i] = yin[i];
    }
}


void CubicInterpolation::setPoints(std::vector<float> const &xin, std::vector<float> const &yin)
{
    x = xin;
    y = yin;
}


bool CubicInterpolation::makeSpline()
{
    sortPoints();

//    x.shrink_to_fit();
//    y.shrink_to_fit();

    int np = int(x.size()); // the number of points
    int ns = np-1;     // the number ofsplines
    int size = 4*ns;       // the square matrix size

    std::vector<float> M(size*size, 0);
    std::vector<float> RHS(size, 0);

    int nrow = 0;
    //    Interpolation conditions
    for (int i=0; i<np-1; i++)
    {
        //pj(x[i]) = y[i]
        M[nrow*size +4*i]     = x[i]*x[i]*x[i];
        M[nrow*size +4*i + 1] = x[i]*x[i];
        M[nrow*size +4*i + 2] = x[i];
        M[nrow*size +4*i + 3] = 1.0;
        RHS[nrow]   = y[i];
        nrow++;

        //pj(x[i+1]) = y[i+1]
        M[nrow*size +4*i]     = x[i+1]*x[i+1]*x[i+1];
        M[nrow*size +4*i + 1] = x[i+1]*x[i+1];
        M[nrow*size +4*i + 2] = x[i+1];
        M[nrow*size +4*i + 3] = 1.0;

        RHS[nrow] = y[i+1];
        nrow++;
    }

    //  Derivation conditions
    for (int i=1; i<np-1; i++)
    {
        //continuity of 1st order derivatives
        M[nrow*size + 4*(i-1)]     =  3.0f*x[i]*x[i];
        M[nrow*size + 4*(i-1)+1]   =  2.0f     *x[i];
        M[nrow*size + 4*(i-1)+2]   =  1.0f;

        M[nrow*size + 4*i]   = -3.0f*x[i]*x[i];
        M[nrow*size + 4*i+1] = -2.0f     *x[i];
        M[nrow*size + 4*i+2] = -1.0f;

        RHS[nrow]   = 0.0f;
        nrow++;

        //continuity of 2nd order derivatives
        M[nrow*size + 4*(i-1)]     =  6.0f*x[i];
        M[nrow*size + 4*(i-1)+1]   =  2.0f     ;

        M[nrow*size + 4*i]   = -6.0f*x[i];
        M[nrow*size + 4*i+1] = -2.0f     ;

        RHS[nrow]   = 0.0f;
        nrow++;
    }

    if(m_b2ndDerivativeLeft)
    {
        //    second order derivative is zero at end points = "natural spline"
        M[nrow*size]   = 6.0f*x[0];
        M[nrow*size+1] = 2.0f;
        RHS[nrow]      = 0.0f;
    }
    else
    {
        // first order derivatives are set at end points
        M[nrow*size]   = 3.0f*x.front()*x.front();
        M[nrow*size+1] = 2.0f*x.front();
        M[nrow*size+2] = 1.0f;
        RHS[nrow]      = m_fp0;
    }

    nrow++;

    if(m_b2ndDerivativeRight)
    {
        //    second order derivative is zero at end points = "natural spline"
        M[nrow*size + size-4] = 6.0f*x[np-1];
        M[nrow*size + size-3] = 2.0f;
        RHS[nrow]             = 0.0f;
    }
    else
    {
        // first order derivatives are set at end points
        M[nrow*size + size-4] = 3.0f*x.back()*x.back();
        M[nrow*size + size-3] = 2.0f*x.back();
        M[nrow*size + size-2] = 1.0f;
        RHS[nrow]             = m_fp1;
    }
//    display_mat(M.data(), size, size);

    int info = matrix::solveLinearSystem(size, M.data(), 1, RHS.data());
    if(info!=0)
//    if(!Gauss(M.data(), size, RHS.data(), 1, bCancel))
    {
        m_bSingularSpline = true;
        return false;
    }
    m_bSingularSpline = false;

    a.clear();
    b.clear();
    c.clear();
    d.clear();
    for(int i=0; i<ns; i++)
    {
        a.push_back(RHS[4*i]);
        b.push_back(RHS[4*i+1]);
        c.push_back(RHS[4*i+2]);
        d.push_back(RHS[4*i+3]);
    }

    return true;
}


// The spline is defined for each interval [x[j], x[j+1]) by n third order polynomial functions
//              p_j(x) = ax3 + bx2 + cx + d
// extend artificially the spline before the first point and after the last one.
float CubicInterpolation::splineValue(float t) const
{
    if(m_bSingularSpline) return 0.0;

    if(t<=x.front()) return a.front()*t*t*t + b.front()*t*t + c.front()*t + d.front();
    for(unsigned int i=0; i<x.size()-1; i++)
    {
        if(x.at(i)<=t && t<x[i+1]) return a.at(i)*t*t*t + b.at(i)*t*t + c.at(i)*t + d.at(i);
    }
    return a.back()*t*t*t + b.back()*t*t + c.back()*t + d.back();
}


// The spline is defined for each interval [x[j], x[j+1]) by n third order polynomial functions
//              p_j(x) = ax3 + bx2 + cx + d
// extend artificially the spline before the first point and after the last one.
float CubicInterpolation::splineDerivative(float t) const
{
    if(m_bSingularSpline) return 0.0f;

    if(t<x.front()) return 3.0f*a.front()*t*t + 2.0f*b.front()*t + c.front();
    for(unsigned int i=0; i<x.size()-1; i++)
    {
        if(x.at(i)<=t && t<x[i+1]) return 3.0f*a.at(i)*t*t + 2.0f*b.at(i)*t + c.at(i);
    }

    return 3.0f*a.back()*t*t + 2.0f*b.back()*t + c.back();
}


/**
 * Bubble sort algorithm for nodes
 * @param array the array of complex numbers to sort
 * @param ub the size of the array
 */
void CubicInterpolation::sortPoints()
{
    int indx(0), indx2(0);
    float xtemp(0), xtemp2(0), ytemp(0), ytemp2(0);
    int flipped(0);

    if (x.size() <= 1) return;

    indx = 1;
    do
    {
        flipped = 0;
        for(indx2 = int(x.size() - 1); indx2 >= indx; --indx2)
        {
            xtemp  = x[indx2];
            xtemp2 = x[indx2 - 1];
            ytemp  = y[indx2];
            ytemp2 = y[indx2 - 1];
            if (xtemp2> xtemp)
            {
                x[indx2 - 1] = xtemp;
                x[indx2] = xtemp2;
                y[indx2 - 1] = ytemp;
                y[indx2] = ytemp2;
                flipped = 1;
            }
        }
    } while ((++indx < int(x.size())) && flipped);
}


/** Adapted from Numerical recipes in C p.115-116
 *  Builds the array y2 of second derivatives*/
void spline2nd(float const x[], float const y[], int n, float yp1, float ypn, float y2[])
{
    float p(0),qn(0),sig(0),un(0);
    float *u = new float[n+1];
    memset(u, 0, n*sizeof(float));

    if (yp1>=LARGEVALUE)
        y2[0]=u[0]=0.0;
    else {
        y2[0] = -0.5f;
        u[0]=(3.0f/(x[2]-x[1]))*((y[2]-y[1])/(x[2]-x[1])-yp1);
    }

    for (int i=1; i<=n-2; i++)
    {
        sig = (x[i]-x[i-1])/(x[i+1]-x[i-1]);
        p = sig*y2[i-1]+2.0f;
        y2[i]=(sig-1.0f)/p;

        u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
        u[i]=(6.0f*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
    }

    if (ypn>=LARGEVALUE)
        qn=un=0.0;
    else
    {
        qn=0.5f;
        un=(3.0f/(x[n-1]-x[n-2]))*(ypn-(y[n-1]-y[n-2])/(x[n-1]-x[n-2]));
    }

    y2[n-1]=(un-qn*u[n-2])/(qn*y2[n-2]+1.0f);

    for (int k=n-2; k>=0; k--)
        y2[k]=y2[k]*y2[k+1]+u[k];

    delete [] u;
}



/** Adapted from Numerical recipes in C p.115-116 .*/
float c3Recipe(float const xa[], float const ya[], float const *y2a, int n, float x)
{
    int klo(0),khi(0),k(0);
    float h(0),b(0),a(0);

    klo=0;
    khi=n-1;
    while (khi-klo > 1)
    {
        k=(khi+klo) >> 1;
        if (xa[k] > x) khi=k;
        else klo=k;
    } //klo and khi now bracket the input value of x.

    h = xa[khi]-xa[klo];
    if (fabs(h)<PRECISION) std::cout<<("Bad xa input to routine splint")<<std::endl;
    a = (xa[khi]-x)/h;
    b = (x-xa[klo])/h;
    return a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])*(h*h)/6.0f;
}




void testCubicInterpolation()
{
    CubicInterpolation c3;
    float X[] = {0.0f, 1.3f, 2.0f, 3.0f, 4.5f, 5.5f, 6.0f};
    float Y[] = {0.0f, 2.6f, 0.3f, -1.5f, -2.0f, 0.7f, 0.0f};
    c3.setPoints(X, Y, 7);

    c3.setBC2ndType(true);
    /*    c3.setFrontDerivative(0.0);
    c3.setBackDerivative(-0.5);*/
    if(!c3.makeSpline())
    {
        std::cout<<(" ----- singular spline -----")<<std::endl;
        return;
    }

    int N = 100;
    for(int i=0; i<N; i++)
    {
        float t=c3.x_().front()+ (c3.x_().back()-c3.x_().front())*float(i)/float(N-1);
        std::cout << QString::asprintf("  %13.7g  %13.7g", t, c3.splineValue(t)).toStdString()<<std::endl;
    }
}


void testRecipe()
{
    int n = 7;
    float X[] = {0.0f, 1.3f, 3.5f, 6.0f, 7.5f, 8.5f, 11.0f};
    float Y[] = {0.0f, -2.0f, 0.7f, 0.0f, 2.0f, 0.7f, 0.0f};
/*    int n = 4;
    float X[] = {            3.5f, 6.0f, 7.5f, 8.5f};
    float Y[] = {            0.7f, 0.0f, 2.0f, 0.7f};*/
/*    int n=2; // incorrect
    float X[] = {                  6.0f, 7.5f};
    float Y[] = {                  0.0f, 2.0f};
*/
    float *y2a = new float[n];
    memset(y2a, 0, n*sizeof(float));
    spline2nd(X, Y, n, LARGEVALUE*2, LARGEVALUE*2, y2a); // make second derivatives
//    spline2nd(X, Y, n, -1.0f, 2.0f, y2a); // make second derivatives

    int N = 100;
    for(int i=0; i<N; i++)
    {
        float t = X[0] + float(i)/float(N-1) * (X[n-1]-X[0]);

        float y = c3Recipe(X, Y, y2a, n, t);
        std::cout << QString::asprintf("  %13.5g  %13.7g", t, y).toStdString();
    }

    delete [] y2a;
}



