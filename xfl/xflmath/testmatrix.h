/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

void makeMatrix(double *A, int n);
void makeRHS(double *b, int n, int nrhs);


void testMatrixInverse();
void testMatMult(int nThreads);
void testMatrix();

void testMatVecMult(int nThreads);


void testLapacke();
void testLapacke3();
void testLapacke4(int rank);

void testLapacke12();


void testBlockThomas();

void testSolve(int n);

bool testCholevski4();

