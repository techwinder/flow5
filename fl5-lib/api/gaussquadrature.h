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

#pragma once

#include <vector>

#include <fl5lib_global.h>

class FL5LIB_EXPORT GaussQuadrature
{
    public:
        GaussQuadrature(int order=3);

        void hardcodeWeigths();

        int order() const {return(_n);}

        bool error() const {return m_bError;}

        bool makeCoefficients(int degree);
        bool computeWeights();


        /** returns the integration point in the interval [0,1] */
        double xrel(int k) const {if(k<0 || k>=_n) return 0.0; return (1.0+_x.at(k))/2.0;}
        /** returns the integration point in the interval [a,b] */
        double xrel(int k, double a, double b) const {if(k<0 || k>=_n) return 0.0; return (a+b)/2.0+ (b-a)/2.0 * _x.at(k);}
        /** returns the weight in the interval [0,1] */
        double weight(int k) const {if(k<0 || k>=_n) return 0.0; return _w[k]*1.0/2.0;}
        /** returns the weight in the interval [a,b] */
        double weight(int k, double a, double b) const {if(k<0 || k>=_n) return 0.0; return _w[k]*(b-a)/2.0;}

        double quadrature(double (*pFunc)(double), double a, double b) const;
        double quadrature2(double (*pFunc2)(double, double), double a0, double b0, double a1, double b1) const;
        double quadrature2(double (*pFunc2)(double, double), double a0, double b0, double (*pFunc)(double)) const;
        double polynomial(int n, double *p, double x) const;
        double polynomialDerivative(int n, double *p, double x) const;


    private:
        int _n;
        std::vector<double> _p, _x, _w;

        bool m_bError;
};


void testQuadrature(int n);
void testPolynomial();
void testLegendrePolynomials();

double func(double x);
double func2(double x, double y);


