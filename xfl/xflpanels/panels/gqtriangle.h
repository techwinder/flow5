/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QVector>
#include <xflgeom/geom2d/vector2d.h>


class GQTriangle
{
    public:
        GQTriangle(int order=3);
        double testIntegral();
        int nPoints() {return m_point.size();}

    public:
        void makeCoeffs(int order);

        int m_iOrder;

        QVector<Vector2d> m_point;
        QVector<double> m_weight;
};

