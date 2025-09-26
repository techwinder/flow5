/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <vector>


void  calcRungeKutta(int order, std::vector<double> const &y0,  double *A, double *B, double (*pFunc)(double), std::vector<std::vector<double>> &result);


double ctrlfunc(double t);
double nullfunc(double t);

void testRungeKutta();

