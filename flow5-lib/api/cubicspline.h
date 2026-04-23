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

class Node2d;

/**
 * @brief The CubicSpline class. Constructs a 2d cubic spline from a set of points.
 *   (1) add the points;
 *   (2) build the matrix and solve
 *   (3) retrieve the points as a function of parameter t
 */
class FL5LIB_EXPORT CubicSpline : public Spline
{
    public:
        CubicSpline();

        Spline* clone() const override {return new CubicSpline(*this);}

        void makeCurve() override;
        bool updateSpline() override;

        bool serializeFl5(QDataStream &ar, bool bIsStoring) override;
        Vector2d splinePoint(double u) const override;
        void splineDerivative(double u, double &dx, double &dy) const override;

        void buildMatrix(int size, int coordinate, double *aij, double *RHS);
        bool solve();
        void splineSecondDerivative(double u, double &d2x, double &d2y) const;
        double curvature(double u) const;
        double length(double u0, double u1) const override;
        void computeArcLengths();
        void computeArcCurvatures(std::vector<double> &arccurvatures) const;
        double totalLength() const;
        double totalCurvature(double u0, double u1) const;

        void rePanel(int N);

        bool getPoint(bool bBefore, double sfrac, Vector2d const &A, Vector2d const &B, Vector2d &I) const;

        bool approximate(int nPts, const std::vector<Node2d> &node);

    private:

        std::vector<double> m_ArcLengths;

        std::vector<double> m_cx, m_cy;
        std::vector<double> m_segVal;     /**  the array of spline parameter values u for each control point. */
};

