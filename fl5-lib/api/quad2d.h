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


#include <vector2d.h>

class Segment2d;
class Triangle2d;

class FL5LIB_EXPORT Quad2d
{
    public:
        Quad2d();
//        Quad2d(Quad2d  const &quad2d);
        Quad2d(Vector2d const &S0, Vector2d const &S1, Vector2d const &S2, Vector2d const &S3);

        double area() const {return m_Area;}
        Vector2d vertex(int idx) const {return S[idx];}
        Segment2d edge(int i) const;

        void setPanelFrame();
        void setPanelFrame(const Vector2d &S0, const Vector2d &S1, const Vector2d &S2, const Vector2d &S3);
        void initialize();

        Triangle2d triangle(int iTriangle) const;

        bool contains(Vector2d const &pt) const;
        bool contains(double x, double y) const;


    private:
        double m_Area;

        Vector2d S[4];              /**< the four vertices, in circular order and in global coordinates*/

        Vector2d CoG_G;          /**< the panel's centroid, in global coordinates */
        Vector2d CoG_L;          /**< the panel's centroid, in local coordinates */
        Vector2d O;              /**< the origin of th local reference frame, in global coordinates */

};

