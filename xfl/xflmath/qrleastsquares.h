/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once




void  swapcolumns(int *p, int i, int j);
void  back_solve(const double *mat, const double *rhs, int rows, int cols, double *sol, int *p);
bool  householder(const double *mat, int rows, int cols, int row_pos, int col_pos, double *result);
void  apply_householder(double *mat, double *rhs, int rows, int cols, double *house, int row_pos, int *p);
int   get_next_col(const double *mat, int rows, int cols, int row_pos, int *p);
void  QRLeastSquares(const double *A, double *b, double *sol, int rows, int cols);
void  mat_vec(const double *mat, int rows, int cols, const double *vec, double *rhs);


void testQRLeastSquare();

