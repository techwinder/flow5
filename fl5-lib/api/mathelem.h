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


#include <complex>

#include <vector>

#include <fl5lib_global.h>

namespace xfl {

/** @enum The different types of panel distribution on the wing */
    enum enumDistribution {UNIFORM, COSINE, SINE, INV_SINE, INV_SINH, TANH, EXP, INV_EXP};


    FL5LIB_EXPORT    std::vector<std::string> distributionTypes();
    FL5LIB_EXPORT    std::string distributionType(xfl::enumDistribution dist);
    FL5LIB_EXPORT    xfl::enumDistribution distributionType(const std::string &Dist);

    FL5LIB_EXPORT    void getPointDistribution(std::vector<double> &fraclist, int nPanels, xfl::enumDistribution DistType=xfl::COSINE);
    FL5LIB_EXPORT    double getDistribFraction(double tau, xfl::enumDistribution DistType=xfl::COSINE);

}


FL5LIB_EXPORT    bool LinBairstow(double *p, std::complex<double> *root, int n);
FL5LIB_EXPORT    double Laguerre(int alpha, int k, double x);
FL5LIB_EXPORT    int factorial(int n);
FL5LIB_EXPORT    std::complex<double> LaplaceHarmonic(int m, int l, double theta, double phi);
FL5LIB_EXPORT    void characteristicPol(double m[][4], double p[5]);
FL5LIB_EXPORT    void sortComplex(std::complex<double>*array, int ub);

double LegendreAssociated( int m, int l, double x);
int binomial(int n, int k);
int compareComplex(std::complex<double> a, std::complex<double>b);
void Legendre(int n, double *a);
void testEigen();

bool cubicSplineInterpolation(int n, const double *x, const double *y, double *a, double *b, double *c, double *d);



void testPointDistribution();

double err_func(double x);
double erf_inv(double a);

double interpolateLine(double x, double x0, double y0, double x1, double y1);

double interpolatePolyLine(double x, const std::vector<double> &xp, const std::vector<double> &yp, bool bExtend);

inline bool isEven(int n) {return n%2==0;}
inline bool isOdd(int n) {return n%2==1;}

bool isBetween(int f, int f1, int f2);
bool isBetween(int f, double f1, double f2);

FL5LIB_EXPORT    double bunchedParameter(double bunchdist, double bunchamp, double t);

FL5LIB_EXPORT    double sigmoid(double amplitude, double t);
FL5LIB_EXPORT    double doubleSigmoid(double amplitude, double t);

FL5LIB_EXPORT    bool linearRegression(int n, double const *x, double const*y, double &a, double &b);

FL5LIB_EXPORT    double HicksHenne(double x, double t1, double t2, double xmin, double xmax);
