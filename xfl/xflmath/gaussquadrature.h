/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <complex>
#include <QVector>



class  GaussQuadrature
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

        int _n;
        QVector<double> _p,_x,_w;

        bool m_bError;
};


void testQuadrature(int n);
void testPolynomial();
void testLegendrePolynomials();

double func(double x);
double func2(double x, double y);


