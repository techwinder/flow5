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

#include <spline.h>


class FL5LIB_EXPORT PointSpline : public Spline
{
    public:
        PointSpline();

        Spline* clone() const override {return new PointSpline(*this);}
        void makeCurve() override;
        bool updateSpline() override;
        Vector2d splinePoint(double u) const override;
        void splineDerivative(double u, double &dx, double &dy) const override;

        void getCamber(double &Camber, double &xc);
        double getY(double xinterp, bool bRel) const override;

        void getSlopes(double &s0, double &s1);

        bool serializeFl5(QDataStream &ar, bool bIsStoring) override;

    private:
        double m_Length;

};


