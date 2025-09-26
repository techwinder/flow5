/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QCheckBox>


#include <xfl3d/testgl/gl3dsurface.h>


#include <xflfoil/panels2d/panel2d.h>


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

