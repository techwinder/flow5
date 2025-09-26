/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

// implements the Savitzky-Golay smoothing filter

#include <QVector>


namespace sgsmooth
{
    void makeCoefficients();

    bool smooth_nonuniform(int deg, int w, const QVector<double> &x, const QVector<double> &y, QVector<double> &ysm);

    void smooth(int order, const QVector<double> &fin, QVector<double> &fout);

    void testSmooth(QVector<double> &f, QVector<double> &g5, QVector<double> &g7, QVector<double> &g9);
    void testSmoothNonUniform();

}

