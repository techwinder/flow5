/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xflgeom/geom2d/splines/spline.h>




class s7Spline : public Spline
{
    public:
        s7Spline();
        virtual void getCamber(double &Camber, double &xc) = 0;
        virtual double getY(double x, bool bRelative) const override = 0;
        virtual void getSlopes(double &s0, double &s1) = 0;
        virtual Vector2d normal(double const &x) = 0;

        void makeCurve() override;

    protected:
        Vector2d m_SlopePoint[2];
};

