/****************************************************************************

    flow5 application

    Copyright (C) Andre Deperrois 

    All rights reserved.

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
class CubicSpline : public Spline
{
    public:
        CubicSpline();

        Spline* clone() const override {return new CubicSpline(*this);}

        void makeCurve() override;
        bool updateSpline() override;

        bool serializeFl5(QDataStream &ar, bool bIsStoring) override;
        void splinePoint(double u, double &x, double &y) const override;
        void splineDerivative(double u, double &dx, double &dy) const override;

        void buildMatrix(int size, int coordinate, double *aij, double *RHS);
        bool solve();
        void splineSecondDerivative(double u, double &d2x, double &d2y) const;
        double curvature(double u) const;
        double length(double u0, double u1) const override;
        void computeArcLengths();
        void computeArcCurvatures(QVector<double> &arccurvatures) const;
        double totalLength() const;
        double totalCurvature(double u0, double u1) const;

        void rePanel(int N);

        bool getPoint(bool bBefore, double sfrac, Vector2d const &A, Vector2d const &B, Vector2d &I) const;

        bool approximate(int nPts, const QVector<Vector2d> &node);

    private:

        QVector<double> m_ArcLengths;

        QVector<double> m_cx, m_cy;
        QVector<double> m_segVal;     /**  the array of spline parameter values u for each control point. */
};

