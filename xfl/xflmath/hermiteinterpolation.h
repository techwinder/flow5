/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/




float Hermite(float A, float B, float C, float D, float t);
float point(float const *points, int n, int index);
float interpolateHermite(const float *x, const float *y, int n, float u);

void testHermiteInterpolation();
