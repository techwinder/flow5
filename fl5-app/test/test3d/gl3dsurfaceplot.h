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

#include <QCheckBox>


#include <interfaces/opengl/testgl/gl3dsurface.h>


#include <api/panel2d.h>


class Stream2d;

class gl3dSurfacePlot: public gl3dSurface
{
    Q_OBJECT
    public:
        gl3dSurfacePlot(QWidget *pParent=nullptr);
        ~gl3dSurfacePlot() override;

        void clearVertices() {m_Vertices.clear();}
        void addVertex(Vector3d vtx) {m_Vertices.push_back(vtx);}
        void addVertex(double x, double y, double z) {m_Vertices.push_back({x,y,z});}

        void glMakePolygon();
        void glRenderView() override;
        void glMake3dObjects() override;
        void paintPolygon();

        void makeVertices(QVector<Vector3d> const & vertices);
        void make3dPotentialSurface();
        void make3dVelocitySurface();
        void make2dVortexStreamFct();
        void make2dSourceStreamFct();

        void make2dStreamFct(const Stream2d *pStream2d, float alpha, float Qinf);

        void setTriangle(Vector3d* vertices);

    private slots:
        void on3dTop() override;
        void on3dBot() override;

    private:
        Vector3d m_TriangleVertex[3];

        QVector<Panel2d> m_Panel2d;

        QOpenGLBuffer m_vboTriangle;
        QOpenGLBuffer m_vboPolygon;

        QVector<Vector3d> m_Vertices;


        double m_ValMin, m_ValMax;


        QCheckBox *m_pchGrid;
        QCheckBox *m_pchContour;
};

