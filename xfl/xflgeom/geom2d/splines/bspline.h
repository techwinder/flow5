/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include "spline.h"
#include <xflgeom/geom2d/node2d.h>

class BSpline : public Spline
{
    public:
        BSpline();

        Spline* clone() const override {return new BSpline(*this);}

        void resetSpline() override;
        void makeCurve() override;
        bool updateSpline() override;

        void duplicate(const Spline &spline) override;
        int degree() const override {return m_degree;}
        void setDegree(int iDegree) override {m_degree = iDegree;}
        bool serializeFl5(QDataStream &ar, bool bIsStoring) override;

        void splinePoint(double u, double &x, double &y) const override;
        void splineDerivative(double u, double &dx, double &dy) const override;

        bool splineKnots();
        void copySymmetric(const BSpline &bspline);
        void fromInterpolation(int N, const Vector2d *pt);

        bool approximate(int degree, int nPts, const QVector<Vector2d> &node);

        QVector<double> const &knots() const {return m_knot;}

        static bool smoothFunction(int deg, int npts, QVector<double> const& x0, QVector<double> const& y0, QVector<double> &y);
        static void testApproximation();
        static void testSmooth();

    private:
        QVector<double> m_knot;            /**< the array of the values of the spline's knot */
        int m_degree;                   /**< the spline's degree */
};

