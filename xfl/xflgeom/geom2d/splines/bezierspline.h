/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


#include <xflgeom/geom2d/splines/spline.h>


class BezierSpline : public Spline
{
    public:
        BezierSpline();

        double BezierBlend(int k, int n, double u) const;
        double Bernstein(int i, int n, double u) const;

        Spline* clone() const override {return new BezierSpline(*this);}

        void makeCurve() override;
        bool updateSpline() override {return true;}

        void splinePoint(double u, double &x, double &y) const override;
        void splineDerivative(double u, double &dx, double &dy) const override;
        bool serializeFl5(QDataStream &ar, bool bIsStoring) override;

        void getCamber(double &Camber, double &xc);
        void getSlopes(double &s0, double &s1);
        Vector2d getNormal(double const &x);

};

