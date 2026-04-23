/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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


#include <hermiteinterpolation.h>


float Hermite(float A, float B, float C, float D, float t)
{
    float a = -A/2.0f + (3.0f*B)/2.0f - (3.0f*C)/2.0f + D/2.0f;
    float b = A - (5.0f*B)/2.0f + 2.0f*C - D / 2.0f;
    float c = -A/2.0f + C/2.0f;
    float d = B;

    return a*t*t*t + b*t*t + c*t + d;
}


float point(float const *points, int n, int index)
{
    if      (index<0)  return points[0];
    else if (index>=n) return points[n-1];
    else               return points[index];
}


float interpolateHermite(float const *X, float const *Y, int n, float u)
{
    int index=0;
    for(index=0; index<n-1; index++)
    {
        if(X[index]<=u && u<X[index+1]) break;
    }

    float t = (u - X[index])/(X[index+1]-X[index]); // fraction of interval

    float A = point(Y, n, index-1);
    float B = point(Y, n, index+0);
    float C = point(Y, n, index+1);
    float D = point(Y, n, index+2);

    float y = Hermite(A, B, C, D, t);

    return y;
}


void testHermiteInterpolation()
{
/*    int n = 7;
    float X[] = {0.0f, 1.3f, 2.0f, 3.0f, 4.5f, 5.5f, 6.0f};
    float Y[] = {0.0f, 2.6f, 0.3f, -1.5f, -2.0f, 0.7f, 0.0f};

    int N = 100;
    for(int i=0; i<N; i++)
    {
        float t = float(i)/float(N-1) * float(n-1);
        float y = interpolateHermite(X, Y, 7, t);
        qDebug("  %13.5g  %13.7g", t, y);
    }*/
}






