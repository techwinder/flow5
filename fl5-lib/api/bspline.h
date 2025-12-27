/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois 
    
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

#pragma once

#include "spline.h"
#include <node2d.h>

class FL5LIB_EXPORT BSpline : public Spline
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

        Vector2d splinePoint(double u) const override;
        void splineDerivative(double u, double &dx, double &dy) const override;

        bool splineKnots();
        void copySymmetric(const BSpline &bspline);
        void fromInterpolation(int N, const Vector2d *pt);

        bool approximate(int degree, int nPts, const std::vector<Node2d> &node);

        std::vector<double> const &knots() const {return m_knot;}

        static bool smoothFunction(int deg, int npts, std::vector<double> const& x0, std::vector<double> const& y0, std::vector<double> &y);
        static void testApproximation();
        static void testSmooth();

    private:
        std::vector<double> m_knot;            /**< the array of the values of the spline's knot */
        int m_degree;                   /**< the spline's degree */
};

