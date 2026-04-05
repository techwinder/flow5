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
#include <api/vector3d.h>


class gl3dTestPoints : public gl3dTestGLView
{
    Q_OBJECT

    public:
        gl3dTestPoints(QWidget *pParent = nullptr);

    protected:
        void glRenderView() override;
        void glMake3dObjects() override;
        bool intersectTheObject(Vector3d const&, Vector3d const &, Vector3d &) override {return false;}
        QSize sizeHint() const override {return QSize(1500, 900);}
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

    protected:
        bool m_bResetPoints;

        int m_iRenderer;
        int m_iShape;

        QOpenGLBuffer m_vboPoints;
};

