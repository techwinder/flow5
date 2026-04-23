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




void  swapcolumns(int *p, int i, int j);
void  back_solve(const double *mat, const double *rhs, int rows, int cols, double *sol, int *p);
bool  householder(const double *mat, int rows, int cols, int row_pos, int col_pos, double *result);
void  apply_householder(double *mat, double *rhs, int rows, int cols, double *house, int row_pos, int *p);
int   get_next_col(const double *mat, int rows, int cols, int row_pos, int *p);
void  QRLeastSquares(const double *A, double *b, double *sol, int rows, int cols);
void  mat_vec(const double *mat, int rows, int cols, const double *vec, double *rhs);


void testQRLeastSquare();

