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

#include <interfaces/opengl/fl5views/gl3dxflview.h>


class PlaneSTL;
class gl3dPlaneSTLView : public gl3dXflView
{
	Q_OBJECT
    public:
        gl3dPlaneSTLView(QWidget *pParent = nullptr);
        ~gl3dPlaneSTLView();

        void setPlane(PlaneSTL *pPlane);

        void resetgl3dPlane() {m_bResetglPlane = true;}
        void resetNormals() {m_bResetglNormals = true;}

        void glRenderView() override;
        bool intersectTheObject(Vector3d const&AA, Vector3d const&BB, Vector3d &I) override;

        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;

        void glMake3dObjects() override;

        void keyPressEvent(QKeyEvent *pEvent) override;

    signals:
        void rectSelected();

    private:
        PlaneSTL* m_pPlaneSTL;

        bool m_bResetglPlane;
        bool m_bResetglNormals;

        QOpenGLBuffer m_vboTriangulation;
        QOpenGLBuffer m_vboOutline;
        QOpenGLBuffer m_vboTriMesh, m_vboTriEdges;
        QOpenGLBuffer m_vboTriangleNormals;
};

