/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xflgeom/geom2d/splines/spline.h>


class PointSpline : public Spline
{
    public:
        PointSpline();

        Spline* clone() const override {return new PointSpline(*this);}
        void makeCurve() override;
        bool updateSpline() override;
        void splinePoint(double u, double &x, double &y) const override;
        void splineDerivative(double u, double &dx, double &dy) const override;

        void getCamber(double &Camber, double &xc);
        double getY(double xinterp, bool bRel) const override;

        void getSlopes(double &s0, double &s1);

        bool serializeFl5(QDataStream &ar, bool bIsStoring) override;

    private:
        double m_Length;

};


