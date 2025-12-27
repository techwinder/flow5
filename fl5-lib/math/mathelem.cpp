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


#include <mathelem.h>
#include <constants.h>
#include <matrix.h>
#include <utils.h>

/**
* Bubble sort algorithm for complex numbers
*@param array the array of complex numbers to sort
*@param ub the size of the array
*/
void sortComplex(std::complex<double>*array, int n)
{
    std::complex<double> temp, temp2;
    int flipped=0;

    if (n<=1) return;

    int indx = 1;
    do
    {
        flipped = 0;
        for (int indx2 = n-1; indx2 >= indx; --indx2)
        {
            temp  = array[indx2];
            temp2 = array[indx2 - 1];
            if (compareComplex(temp2, temp) > 0)
            {
                array[indx2 - 1] = temp;
                array[indx2] = temp2;
                flipped = 1;
            }
        }
    } while ((++indx < n) && flipped);
}


/**
* Method for the comparison of two complex numbers
*@param a first complex number
*@param b second complex number
*@return 1 if Real(a) > Real(b), -1 if Real(a)<Real(b); if Real(a)=Real(b), returns 1 if Imag(a)>Image(b), -1 otherwise.
*/
int compareComplex(std::complex<double> a, std::complex<double>b)
{
    if(a.real()>b.real())       return  1;
    else if (a.real()<b.real()) return -1;
    else
    {    //same real part
        if(a.imag()>b.imag())         return  1;
        else if (a.imag()<b.imag())   return -1;
        else return 0;
    }
}




/**
* Returns the coefficients of the characteristic polynomial of a 4x4 matrix of double values. Thanks Mapple.
* The polynom can then be solved for complex roots using Bairstow's algorithm
*@param m the 4x4 matrix
*@param p the array holding the 5 coefficients of the matrix characteristic polynomial
*/
void characteristicPol(double m[][4], double p[5])
{
    // lambda^4
    p[4] =  1;

    // lambda^3
    p[3] =  - m[0][0]- m[1][1]-m[2][2]- m[3][3];

    // lambda^2
    p[2] =
        + m[0][0] * m[1][1]
        + m[0][0] * m[2][2]
        + m[0][0] * m[3][3]
        + m[1][1] * m[3][3]
        + m[1][1] * m[2][2]
        + m[2][2] * m[3][3]
        - m[1][0] * m[0][1]
        - m[2][1] * m[1][2]
        - m[2][0] * m[0][2]
        - m[2][3] * m[3][2]
        - m[3][1] * m[1][3]
        - m[3][0] * m[0][3];

    // lambda^1
    p[1] =
        + m[2][1] * m[1][2] * m[3][3]
        + m[0][0] * m[2][1] * m[1][2]
        - m[3][1] * m[1][2] * m[2][3]
        + m[3][1] * m[1][3] * m[2][2]
        + m[1][0] * m[0][1] * m[3][3]
        + m[1][0] * m[0][1] * m[2][2]
        - m[2][0] * m[0][1] * m[1][2]
        - m[1][0] * m[3][1] * m[0][3]
        - m[1][0] * m[2][1] * m[0][2]
        + m[3][0] * m[1][1] * m[0][3]
        - m[3][0] * m[0][1] * m[1][3]
        + m[2][0] * m[0][2] * m[3][3]
        - m[2][0] * m[3][2] * m[0][3]
        + m[2][0] * m[1][1] * m[0][2]
        - m[3][0] * m[0][2] * m[2][3]
        + m[3][0] * m[0][3] * m[2][2]
        - m[2][1] * m[3][2] * m[1][3]
        - m[0][0] * m[1][1] * m[2][2]
        + m[0][0] * m[2][3] * m[3][2]
        + m[1][1] * m[2][3] * m[3][2]
        - m[0][0] * m[2][2] * m[3][3]
        + m[0][0] * m[3][1] * m[1][3]
        - m[1][1] * m[2][2] * m[3][3]
        - m[0][0] * m[1][1] * m[3][3];

    // lambda^0
    p[0] =
        + m[2][0] * m[0][1] * m[1][2] * m[3][3]
        - m[2][0] * m[1][1] * m[0][2] * m[3][3]
        + m[2][0] * m[1][1] * m[3][2] * m[0][3]
        - m[2][0] * m[0][1] * m[3][2] * m[1][3]
        + m[1][0] * m[3][1] * m[0][3] * m[2][2]
        - m[1][0] * m[3][1] * m[0][2] * m[2][3]
        + m[1][0] * m[2][1] * m[0][2] * m[3][3]
        - m[1][0] * m[2][1] * m[3][2] * m[0][3]
        - m[3][0] * m[1][1] * m[0][3] * m[2][2]
        + m[3][0] * m[0][1] * m[1][3] * m[2][2]
        - m[3][0] * m[0][1] * m[1][2] * m[2][3]
        - m[2][0] * m[3][1] * m[1][2] * m[0][3]
        + m[2][0] * m[3][1] * m[0][2] * m[1][3]
        - m[3][0] * m[2][1] * m[0][2] * m[1][3]
        + m[3][0] * m[1][1] * m[0][2] * m[2][3]
        + m[3][0] * m[2][1] * m[1][2] * m[0][3]
        - m[0][0] * m[2][1] * m[1][2] * m[3][3]
        + m[0][0] * m[2][1] * m[3][2] * m[1][3]
        - m[1][0] * m[0][1] * m[2][2] * m[3][3]
        + m[1][0] * m[0][1] * m[2][3] * m[3][2]
        + m[0][0] * m[3][1] * m[1][2] * m[2][3]
        - m[0][0] * m[3][1] * m[1][3] * m[2][2]
        + m[0][0] * m[1][1] * m[2][2] * m[3][3]
        - m[0][0] * m[1][1] * m[2][3] * m[3][2];
}



#define TOLERANCE   1.e-8
#define MAXBAIRSTOWITER 100

#define POLYNOMORDER    6

/**
* Finds the complex roots of a polynom P(x) using Lin-Bairstow's method
* P(x) = Sum p_i x^i        i = 0..n;
* The polynoms coefficient are in array p
*
* Andre Deperrois October 2009
*@param p the array of the polynoms double's coefficients
*@param root the array of the polynom's complex roots
*@param n the polynom's order
*@return true if the extraction was successful
*/
bool LinBairstow(double *p, std::complex<double> *root, int n)
{
    double b[POLYNOMORDER], c[POLYNOMORDER];
    memset(b, 0, POLYNOMORDER*sizeof(double));
    memset(c, 0, POLYNOMORDER*sizeof(double));

    double d0(0),d1(0),d2(0);

    //T(x) = x2 -rx -s;
    //R(x) = u(x-r)+v //remainder of deivision of by Q
    //Q(x) = Sum b_i x^(i-2)

    //P(x) = (x2-rx+s) Q(x) + u(x-r) +v

    int nn=n ;//initial order is polynom order

    double r=-2.0;//initial guesses
    double s=-1.0;

    do
    {
        int iter = 0;
        do
        {
            //compute recursively the coefs b_i of polynom Q
            b[nn]   = p[nn];
            b[nn-1] = p[nn-1] + r * b[nn];
            for(int k=nn-2; k>=0; k--) b[k] = p[k] + r*b[k+1] + s*b[k+2];

            //build the partial derivatives c_i
            c[nn]   = b[nn];
            c[nn-1] = b[nn-1] + r * c[nn];
            for(int k=nn-2; k>=1; k--) c[k] = b[k] + r*c[k+1] + s*c[k+2];

            d0 = c[1]*c[3] - c[2]*c[2];
            d1 = (-b[0]*c[3]+b[1]*c[2])/d0;
            d2 = (-b[1]*c[1]+b[0]*c[2])/d0;
            r+=d1;
            s+=d2;
            iter++;
        } while((std::abs(d1)> TOLERANCE || std::abs(d2)> TOLERANCE) && iter < MAXBAIRSTOWITER);

        if(iter>=MAXBAIRSTOWITER)return false;
        //we have a division
        //so find the roots of the remainder R
        double Delta = r*r+4.0*s;
        if(Delta<0.0)
        {
            //complex roots
            root[nn-1] = std::complex<double>(r/2.0,  sqrt(fabs(Delta))/2.0);
            root[nn-2] = std::complex<double>(r/2.0, -sqrt(fabs(Delta))/2.0);
        }
        else
        {
            //real roots
            root[nn-1] = std::complex<double>(r/2.0 + sqrt(Delta)/2.0, 0.0);
            root[nn-2] = std::complex<double>(r/2.0 - sqrt(Delta)/2.0, 0.0);
        }

        //deflate polynom order
        for(int i=nn; i>=2; i--)
        {
            p[i-2] = b[i];
        }
        nn-=2;
        if(nn==2)
        {
            //last two roots, solve directly
            if(fabs(p[2])<PRECISION)
            {
                // one last root, but we should never get here
                if(fabs(p[1])>PRECISION)
                {
                    //last single root, real
                    root[0] = -p[0]/p[1];
                }
                else return false;
            }
            else
            {
                Delta = p[1]*p[1]-4.0*p[0]*p[2];
                if(Delta<0)
                {
                    //complex roots
                    root[nn-1] = std::complex<double>(-p[1]/2.0/p[2],  sqrt(fabs(Delta))/2.0/p[2]);
                    root[nn-2] = std::complex<double>(-p[1]/2.0/p[2], -sqrt(fabs(Delta))/2.0/p[2]);
                }
                else
                {
                    //real roots
                    root[nn-1] = std::complex<double>((-p[1]+sqrt(Delta))/2.0/p[2],  0.0);
                    root[nn-2] = std::complex<double>((-p[1]-sqrt(Delta))/2.0/p[2],  0.0);
                }
            }
            break;
        }
        if(nn==1)
        {
            if(fabs(p[1])>PRECISION)
            {
                //last single root, real
                root[0] = -p[0]/p[1];
            }
            else return false;
            break;
        }

    }while(nn>2);
    return true;
}



/**
// Given an array of n+1 pairs (x[i], y[i]), with i ranging from 0 to n,
// this function calculates the 3rd order cubic spline which interpolate the pairs.
//
// The spline is defined for each interval [x[j], x[j+1]) by n third order polynomial functions
//              p_j(x) = ax3 + bx2 + cx + d
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
// Second order derivative is zero at the end points : 2 conditions
//    p_j"(x[0]) =  p_j"(x[n]) =0
//
//
// This sets a linear system of size 4n which is solved by the Gauss algorithm for coefs a,b,c and d
// The RHS vector is
//      a[0]
//      b[0]
//      c[0]
//      d[0]
//      a[1]
//    ...
//      d[n-1]
*/
bool cubicSplineInterpolation(int n, double const *x, double const *y, double *a, double *b, double *c, double *d)
{
    if(n>50) return false;

    std::vector<double>M(16*n*n,0);
    std::vector<double>RHS(4*n,0);


    int size = 4*n;
//    Interpolation conditions
    for (int i=0; i<n; i++)
    {
        //pj(x[i]) = y[i]
        M[2*i*size +4*i]     = x[i]*x[i]*x[i];
        M[2*i*size +4*i + 1] = x[i]*x[i];
        M[2*i*size +4*i + 2] = x[i];
        M[2*i*size +4*i + 3] = 1.0;

        //pj(x[i+1]) = y[i+1]
        M[(2*i+1)*size +4*i]     = x[i+1]*x[i+1]*x[i+1];
        M[(2*i+1)*size +4*i + 1] = x[i+1]*x[i+1];
        M[(2*i+1)*size +4*i + 2] = x[i+1];
        M[(2*i+1)*size +4*i + 3] = 1.0;

        RHS[2*i]   = y[i];
        RHS[2*i+1] = y[i+1];
    }

//  Derivation conditions
    for (int i=1; i<n; i++)
    {
        //continuity of 1st order derivatives

        M[(2*n+i)*size + 4*(i-1)]     =  3.0*x[i]*x[i];
        M[(2*n+i)*size + 4*(i-1)+1]   =  2.0     *x[i];
        M[(2*n+i)*size + 4*(i-1)+2]   =  1.0;

        M[(2*n+i)*size + 4*i]   = -3.0*x[i]*x[i];
        M[(2*n+i)*size + 4*i+1] = -2.0     *x[i];
        M[(2*n+i)*size + 4*i+2] = -1.0;

        RHS[2*n+i]   = 0.0;

        //continuity of 2nd order derivatives
        M[(3*n+i)*size + 4*(i-1)]     =  6.0*x[i];
        M[(3*n+i)*size + 4*(i-1)+1]   =  2.0     ;

        M[(3*n+i)*size + 4*i]   = -6.0*x[i];
        M[(3*n+i)*size + 4*i+1] = -2.0     ;

        RHS[3*n+i]   = 0.0;
    }

//    second order derivative is zero at end points = "natural spline"
    M[2*n*size]     = 6.0*x[0];
    M[2*n*size+1]   = 2.0;
    RHS[2*n]        = 0.0;

    M[3*n*size + size-4]   = 6.0*x[n];
    M[3*n*size + size-3]   = 2.0;
    RHS[3*n+1]             = 0.0;

    bool bCancel = false;
    if(!matrix::Gauss(M.data(), 4*n, RHS.data(), 1, bCancel)) return false;

    for(int i=0; i<n; i++)
    {
        a[i] = RHS[4*i];
        b[i] = RHS[4*i+1];
        c[i] = RHS[4*i+2];
        d[i] = RHS[4*i+3];
    }

    return true;
}

void testPointDistribution()
{
    std::vector<double> cos, sine, invsine, invsinh, tanh, exp, invexp;
    int nPanels = 50;
    xfl::getPointDistribution(cos,     nPanels, xfl::COSINE);
    xfl::getPointDistribution(sine,    nPanels, xfl::SINE);
    xfl::getPointDistribution(invsine, nPanels, xfl::INV_SINE);
    xfl::getPointDistribution(invsinh, nPanels, xfl::INV_SINH);
    xfl::getPointDistribution(tanh,    nPanels, xfl::TANH);
    xfl::getPointDistribution(exp,     nPanels, xfl::EXP);
    xfl::getPointDistribution(invexp,  nPanels, xfl::INV_EXP);
    qDebug("      COSINE         SINE      INVERSESINE   INVERSESINH        TANH          EXP       INVERSEEXP");
    for(int i=0; i<=nPanels; i++)
        qDebug("   %11.5g   %11.5g   %11.5g   %11.5g   %11.5g   %11.5g   %11.5g",
               cos[i], sine[i], invsine[i], invsinh[i], tanh[i], exp[i], invexp[i]);
}


/** @todo use std::erf instead */
double err_func(double x)
{
    // approximation from Abramowitz and Stegun (equations 7.1.25–28)
    double p = 0.3275911;
    double a1 = 0.254829592;
    double a2 = -0.284496736;
    double a3 = 1.421413741;
    double a4 = -1.453152027;
    double a5 = 1.061405429;

    // All of these approximations are valid for x>0
    // To use these approximations for negative x, use the fact that erf(x)
    // is an odd function, so erf(x) = −erf(−x).
    double ax = fabs(x);
    double t = 1.0/(1.0 + p*ax);
    double e = a1*t + a2*t*t + a3*t*t*t + a4*t*t*t*t + a5*t*t*t*t*t;
    e *= exp(-ax*ax);

#ifdef Q_OS_WIN
    double nada = erf(x);
    (void)nada;
#endif

    if(x>0.0) return 1.0-e;
    else      return -(1.0-e);
}

/* compute inverse error functions with maximum error of 2.35793 ulp */
double erf_inv(double a)
{
    float p(0), r(0), t(0);
    t = fmaf (a, 0.0f - a, 1.0f);
    t = log(t);
    if (fabsf(t) > 6.125f)
    { // maximum ulp error = 2.35793
        p =              3.03697567e-10f; //  0x1.4deb44p-32
        p = fmaf (p, t,  2.93243101e-8f); //  0x1.f7c9aep-26
        p = fmaf (p, t,  1.22150334e-6f); //  0x1.47e512p-20
        p = fmaf (p, t,  2.84108955e-5f); //  0x1.dca7dep-16
        p = fmaf (p, t,  3.93552968e-4f); //  0x1.9cab92p-12
        p = fmaf (p, t,  3.02698812e-3f); //  0x1.8cc0dep-9
        p = fmaf (p, t,  4.83185798e-3f); //  0x1.3ca920p-8
        p = fmaf (p, t, -2.64646143e-1f); // -0x1.0eff66p-2
        p = fmaf (p, t,  8.40016484e-1f); //  0x1.ae16a4p-1
    }
    else
    { // maximum ulp error = 2.35456
        p =              5.43877832e-9f;  //  0x1.75c000p-28
        p = fmaf (p, t,  1.43286059e-7f); //  0x1.33b458p-23
        p = fmaf (p, t,  1.22775396e-6f); //  0x1.49929cp-20
        p = fmaf (p, t,  1.12962631e-7f); //  0x1.e52bbap-24
        p = fmaf (p, t, -5.61531961e-5f); // -0x1.d70c12p-15
        p = fmaf (p, t, -1.47697705e-4f); // -0x1.35be9ap-13
        p = fmaf (p, t,  2.31468701e-3f); //  0x1.2f6402p-9
        p = fmaf (p, t,  1.15392562e-2f); //  0x1.7a1e4cp-7
        p = fmaf (p, t, -2.32015476e-1f); // -0x1.db2aeep-3
        p = fmaf (p, t,  8.86226892e-1f); //  0x1.c5bf88p-1
    }
    r = a * p;
    return r;
}


/**
 * returns the ordinate y corresponding to coordinate x
 * on a line defined by points (x0,y0) and (x1,y1)
 */
double interpolateLine(double x, double x0, double y0, double x1, double y1)
{
    if(fabs(x1-x0)<PRECISION) return 0.0;
    double a = (y1-y0)/(x1-x0);
    return y0+a*(x-x0);
}


/** if bExtend is true, then interpolations will be made outside the interval defined by the x array
 * based on the end points
 * if false, then interpolation will return the first or last y-value */
double interpolatePolyLine(double x, std::vector<double> const &xp, std::vector<double> const &yp, bool bExtend)
{
    if(xp.size() != yp.size()) return 0.0;

    size_t n = xp.size();
    if(n<2) return 0.0; // cannot interpolate


    if(x<=xp[0])
    {
        if(bExtend) return interpolateLine(x, xp[0], yp[0], xp[1], yp[1]);
        else        return yp.front();
    }
    if(x>=xp[n-1])
    {
        if(bExtend) return interpolateLine(x, xp[n-2], yp[n-2], xp[n-1], yp[n-1]);
        else        return yp.back();
    }

    for(uint i=1; i<xp.size()-1; i++)
    {
        if(xp[i]<=x && x<=xp[i+1]) return interpolateLine(x, xp[i], yp[i], xp[i+1], yp[i+1]);
    }
    return 0.0; // never reached
    assert(true);
}


xfl::enumDistribution xfl::distributionType(const std::string &Dist)
{
    std::string strDist = Dist;
    xfl::trim(strDist);

    if     (strDist.compare("COSINE"      )==0) return xfl::COSINE;
    else if(strDist.compare("UNIFORM"     )==0) return xfl::UNIFORM;
    else if(strDist.compare("SINE"        )==0) return xfl::SINE;
    else if(strDist.compare("INVERSESINE" )==0) return xfl::INV_SINE; // deprecated
    else if(strDist.compare("INV_SINE"    )==0) return xfl::INV_SINE;
    else if(strDist.compare("INV_SINH"    )==0) return xfl::INV_SINH;
    else if(strDist.compare("TANH"        )==0) return xfl::TANH;
    else if(strDist.compare("EXP"         )==0) return xfl::EXP;
    else if(strDist.compare("INV_EXP"     )==0) return xfl::INV_EXP;
    else
    {
        std::cout<<"String distrib not found"<<strDist<<std::endl;
        return xfl::UNIFORM;
    }
}

/**
* Tests if a given integer is between two other integers
*@param f the integer to test
*@param f1 the first bound
*@param f2 the second bound
*@return true if f1<f<f2 or f2<f<f1
*/
bool isBetween(int f, int f1, int f2)
{
    if (f2 < f1)
    {
        int tmp = f2;
        f2 = f1;
        f1 = tmp;
    }
    if(f<f1) return false;
    else if(f>f2) return false;
    return true;
}


/**
 * Tests if a given integer is between two double values
 * @param f the integer to test
 * @param f1 the first bound
 * @param f2 the second bound
 * @return true if f1<f<f2 or f2<f<f1
 */
bool isBetween(int f, double f1, double f2)
{
    double ff = f;
    if (f2 < f1)
    {
        double tmp = f2;
        f2 = f1;
        f1 = tmp;
    }
    if(ff<f1)       return false;
    else if(ff>f2) return false;
    return true;
}


/** input uniformly spaced in [0,1], ouput bunched in [0,1]
    BunchAmp:  k=0.0 --> uniform bunching, k=1-->full varying bunch
    BunchDist: k=0.0 --> uniform bunching, k=1 weigth on endpoints
*/
double bunchedParameter(double bunchdist, double bunchamp, double t)
{
    // complex mix of three functions
    // linear, tanh, and atanh
    double flin = -1.0+t*2.0;

    if(bunchdist<0.5)
    {
        //mix linear and tanh
        double mix = 2.0*bunchdist;
        double a0 = (0.001+tanh(bunchamp))*3.5;
        double fth = tanh(a0*(2.0*t-1.0))/tanh(a0);
        return (1.0+mix*flin + (1.0-mix)*fth)/2.0;
    }
    else
    {
        //mix linear and atanh
        double mix = 2.0*(bunchdist-0.5);
        double a1 = (0.001+tanh(3.0*bunchamp))*0.99;
        double fath = atanh(a1*(2.0*t-1.0))/atanh(a1);
        return (1.0+(1.0-mix)*flin + mix*fath)/2.0;
    }
}


/** https://dinodini.wordpress.com/2010/04/05/normalized-tunable-sigmoid-functions/
 *      ____
 *     /
 *     |
 * ____/
 * 0  1/2   1
 */
double sigmoid(double amplitude, double x)
{
    x= 2.0*x-1;
    double y = (x-amplitude*x)/(amplitude-2*amplitude*fabs(x)+1);
    return (y+1.0)/2.0;
}


/**
 * bunch function used to increase node density around the LE and the TE
 *
 *            ___
 *           /
 *           |
 *      _____/
 *     /
 *     |
 * ___/
 * 0     1/2     1
 */
double doubleSigmoid(double amplitude, double t)
{
    if(t<=0.5)
        return  sigmoid(amplitude*0.85, t*2)/2.0;
    else
        return sigmoid(amplitude*0.85, (t-0.5)*2.0)/2.0 + 0.5;
}


/** Performs a linear regression of the array of n points (x_i, y_i) and
 * calculater the coefficients of line y = ax+b.
 * Returns true if the line could be calculated, false if not */
bool linearRegression(int n, double const *x, double const*y, double &a, double &b)
{
    a = b = 0;

    double sum_x=0, sum_y=0.0;
    double sum_x2=0.0;
    double sum_xy= 0;

    for(int i=0; i<n; i++)
    {
        sum_x  += x[i];
        sum_y  += y[i];
        sum_xy += x[i]*y[i];
        sum_x2 += x[i]*x[i];
    }
    double dn = double(n);
    double denom = dn*sum_x2 - sum_x*sum_x;
    if(fabs(denom)<1.e-6) return false;

    a = (dn*sum_xy-sum_x*sum_y)/denom;
    b = (sum_y - a*sum_x)/dn;

    return true;
}


/** Hicks-Henne bump function
 * parameter t1 controls the bump's position and t2 its width
 */
double HicksHenne(double x, double t1, double t2, double xmin, double xmax)
{
    if(x<=xmin || x>=xmax) return 0.0;
    double xrel = (x-xmin)/(xmax-xmin);
    return pow(sin(PI*pow(xrel, log(0.5)/log(t1))), t2);
}


/**
 * Calculates the coefficients of Legendre's polynomial of degree n
 * Uses the recursive formula n.Pnp1(x) = (2n-1).x.Pnm1(x) - (n-1) Pnm2(x)
 * P0(x) = 1; P1(x) = x;
 */
void Legendre(int n, double *a)
{
    if(n==0)
    {
        a[0] = 1.0;
    }
    else if(n==1)
    {
        a[0] = 0.0;
        a[1] = 1.0;
    }
    else
    {
        std::vector<double> a_nm1(n,0);
        std::vector<double> a_nm2(n-1,0);

        Legendre(n-1, a_nm1.data());
        for(int i=0; i<n; i++)
            a[i+1]  = double(2*n-1)/double(n) * a_nm1[i];

        Legendre(n-2, a_nm2.data());
        for(int i=0; i<n-1; i++)
            a[i] += -double(n-1)/double(n) * a_nm2[i];
    }
}


double Laguerre(int alpha, int k, double x)
{
    if(k==0) return 1.0;
    if(k==1) return double(1+alpha)-x;
    return (double(alpha+1)-x)/double(k) * Laguerre(alpha+1, k-1, x) - x/double(k) * Laguerre(alpha+2, k-2, x);
}


int factorial(int n)
{
    if     (n==0) return 1;
    else if(n==1) return 1;
    else if(n==2) return 2;
    else if(n==3) return 6;
    else if(n==4) return 24;
    else if(n==5) return 120;
    else if(n==6) return 750;
    else if(n==7) return 5040;

    return n*factorial(n-1);
}


int binomial(int n, int k)
{
    if(k==0) return 1;
    if(k==n) return 1;
    return binomial(n-1, k-1) + binomial(n-1, k);
}


double LegendreAssociated(int m, int l, double x)
{
    if(m<0)
        return -1.0* double(factorial(l+m))/double(factorial(l-m))*LegendreAssociated(-m, l, x);

    if(m==0  && l==0) return 1;
    if(m==0  && l==1) return x;
    if(m==1  && l==1) return -sqrt(1.0-x*x);
    if(m==-1 && l==1) return 0.5*sqrt(1.0-x*x);

    if(l==m)   return - double(2*l-1) * sqrt(1.0-x*x) * LegendreAssociated(m-1, l-1, x);
    if(m==l-1) return x*double(2*l+1) * LegendreAssociated(m,m,x);

    return 1.0/double(l-m+1) * ( double(2*l+1) * x * LegendreAssociated(m, l-1, x)
                                -double(l+m)       * LegendreAssociated(m, l-2, x));
}


/** theta and phi in radians */
std::complex<double> LaplaceHarmonic(int m, int l, double theta, double phi)
{
    std::complex<double> harmonic(0,0);
    if(abs(m)>l) return harmonic;

    double N = sqrt(double(2*l+1)/4.0/PI* factorial(l-m)/factorial(l+m));
//    if(m%2==1)  N=-N;
    double LA = LegendreAssociated(m, l, cos(theta));

    harmonic.real(N * LA * cos(m*phi));
    harmonic.imag(N * LA * sin(m*phi));

    return harmonic;
}



/**
 * Returns an array of (nPanels+1) double values spaced between 0 and 1, i.a.w. the specified distribution.
 */
void xfl::getPointDistribution(std::vector<double> &fraclist, int nPanels, xfl::enumDistribution DistType)
{
    fraclist.clear();
    fraclist.reserve(nPanels+1);
    double dN = double(nPanels);
    for(int i=0; i<=nPanels; i++)
    {
        double di = double(i)/dN;
        double d  = xfl::getDistribFraction(di, DistType);
        fraclist.push_back(d);
    }
}




double xfl::getDistribFraction(double tau, xfl::enumDistribution DistType)
{
    switch (DistType)
    {
        case xfl::COSINE:
        {
            return 1.0/2.0*(1.0-cos(tau*PI));
        }
        case xfl::SINE:
        {
            return 1.0*(1.-cos(tau*PI/2.0));
        }
        case xfl::INV_SINE:
        {
            return  1.0*(sin( tau*PI/2.0));
        }
        case xfl::INV_SINH:
        {
            double c = 5;
            double asc = 1.0/asinh(c);
            return 0.5*(1.0+asinh(c*(2.*tau-1.0))*asc);
        }
        case xfl::TANH:
        {
            double c = 1.35;
            double atc = 1.0/tanh(c);
            return 0.5*(1.0+tanh(c*(2.*tau-1.0)) * atc);
        }
        case xfl::EXP:
        {
            double c = 1.5;
            return pow(tau, c);
        }
        case xfl::INV_EXP:
        {
            double c = 1.5;
            return 1.0-pow(1-tau, c);
        }
        default: //UNIFORM
        case xfl::UNIFORM:
        {
            return tau;
        }
    }
}


std::vector<std::string> xfl::distributionTypes()
{
    std::vector<std::string> list;

    list.push_back(xfl::distributionType(xfl::UNIFORM));
    list.push_back(xfl::distributionType(xfl::COSINE));
    list.push_back(xfl::distributionType(xfl::SINE));
    list.push_back(xfl::distributionType(xfl::INV_SINE));
    list.push_back(xfl::distributionType(xfl::INV_SINH));
    list.push_back(xfl::distributionType(xfl::TANH));
    list.push_back(xfl::distributionType(xfl::EXP));
    list.push_back(xfl::distributionType(xfl::INV_EXP));

    return list;
}


std::string xfl::distributionType(xfl::enumDistribution dist)
{
    switch(dist)
    {
        case xfl::COSINE:
            return "COSINE";
        case xfl::UNIFORM:
            return "UNIFORM";
        case xfl::SINE:
            return "SINE";
        case xfl::INV_SINE:
            return "INV_SINE";
        case xfl::INV_SINH:
            return "INV_SINH";
        case xfl::TANH:
            return "TANH";
        case xfl::EXP:
            return "EXP";
        case xfl::INV_EXP:
            return "INV_EXP";
        default:
        {
            return "";
        }
    }
}







