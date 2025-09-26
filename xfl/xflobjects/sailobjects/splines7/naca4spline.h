/****************************************************************************

    sail7 Application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#pragma once

#include <xflobjects/sailobjects/splines7/s7spline.h>


class Naca4Spline : public s7Spline
{
    public:
        Naca4Spline();

        void getCamber(double &Camber, double &xc) override;
        double getY(double x, bool bRelative) const override;
        void getSlopes(double &s0, double &s1) override;
        Vector2d normal(double const &x) override;

        bool serialize(QDataStream &ar, bool bIsStoring);


        void duplicate(Spline const &n4spline) override;
        bool updateSpline() override;
        void makeCurve() override;
        void splinePoint(double u, double &x, double &y) const override;
        void splineDerivative(double u, double &dx, double &dy) const override;


    private:
        double p, m;
};


