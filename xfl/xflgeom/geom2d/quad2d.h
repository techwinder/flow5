/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once
#include <QString>
#include <xflgeom/geom2d/vector2d.h>

class Segment2d;
class Triangle2d;

class Quad2d
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

        void displayNodes(const QString &msg) const;

    private:
        double m_Area;

        Vector2d S[4];              /**< the four vertices, in circular order and in global coordinates*/

        Vector2d CoG_G;          /**< the panel's centroid, in global coordinates */
        Vector2d CoG_L;          /**< the panel's centroid, in local coordinates */
        Vector2d O;              /**< the origin of th local reference frame, in global coordinates */

};

