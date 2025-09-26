/****************************************************************************
 *
 * 	sail7 Application
 *
 * 	Copyright (C) Andre Deperrois 
 *
 * 	All rights reserved.
 *
 *****************************************************************************/

#pragma once

#include <xflobjects/sailobjects/splines7/s7spline.h>
#include <xflgeom/geom2d/splines/bspline.h>



class ArcSpline : public s7Spline
{
    public:
        ArcSpline();

        void getCamber(double &Camber, double &xc) override;
        void getSlopes(double &s0, double &s1) override;
        Vector2d normal(double const &x) override;
        double getY(double xinterp, bool bRelative) const override;

        void duplicate(const Spline &arcspline) override;
        bool updateSpline() override;
        void splinePoint(double u, double &x, double &y) const override;
        void splineDerivative(double u, double &dx, double &dy) const override;


    private:
        BSpline m_BSpline1, m_BSpline2;

};

