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

#include <interfaces/opengl/testgl/gl3dtestglview.h>

class gl3dSurface : public gl3dTestGLView
{
    public:
        gl3dSurface(QWidget *pParent=nullptr);
        void keyPressEvent(QKeyEvent *pEvent) override;
        void glRenderView() override;
        void glMake3dObjects() override;
        bool intersectTheObject(Vector3d const&, Vector3d const&, Vector3d &)  override {return false;}

    protected:
        void paintGrid();

        void glMakeSurface();
        void make3dPlane(double a, double b, double c);
        double makeTestSurface();
        double error(const Vector2d &pos, bool bMinimum) const;
        double function(double x, double y) const;

    protected:
        bool m_bGrid, m_bContour;
        double m_HalfSide;
        int m_Size_x, m_Size_y;
        QVector<Vector3d> m_PointArray; // Size x Size

        QOpenGLBuffer m_vboSurface, m_vboGrid;

        QOpenGLBuffer m_vboContourLines;

        bool m_bResetSurface;
        bool m_bDisplaySurface;
        bool m_bDoubleDipSurface;


        double m_ValMin, m_ValMax;

};


extern double c0, c1, c2; // random function coefficients



