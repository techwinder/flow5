/****************************************************************************
 *
 * 	sail7 Application
 *
 * 	Copyright (C) 2025 André Deperrois 
 *
 * 	
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

 *
 *****************************************************************************/

#pragma once

#include <s7spline.h>
#include <bspline.h>



class FL5LIB_EXPORT ArcSpline : public s7Spline
{
    public:
        ArcSpline();

        void getCamber(double &Camber, double &xc) override;
        void getSlopes(double &s0, double &s1) override;
        Vector2d normal(double const &x) override;
        double getY(double xinterp, bool bRelative) const override;

        void duplicate(const Spline &arcspline) override;
        bool updateSpline() override;
        Vector2d splinePoint(double u) const override;
        void splineDerivative(double u, double &dx, double &dy) const override;


    private:
        BSpline m_BSpline1, m_BSpline2;

};

