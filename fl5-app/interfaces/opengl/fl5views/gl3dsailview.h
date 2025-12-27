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

#include <TopoDS_Edge.hxx>

#include <interfaces/opengl/fl5views/gl3dxflview.h>


class Sail;

class gl3dSailView : public gl3dXflView
{
    Q_OBJECT

    public:
        gl3dSailView(QWidget *pSailDlg=nullptr);
        ~gl3dSailView() override;

        void setSail(const Sail *pSail);
        void resetglSail()             {m_bResetglSail=true;}
        void resetglSectionHighlight() {m_bResetglSectionHighlight=true;}

    private:
        void glRenderView() override;
        void glMake3dObjects() override;
        bool intersectTheObject(Vector3d const &AA, Vector3d const &BB, Vector3d &I) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;

        void paintControlPoints();
        void setHighlightedEdge(TopoDS_Edge const &Edge);
        void glMakeSailNormals(const Sail *pSail, float length, QOpenGLBuffer &vbo); // debug only
        void clearHighlightedEdge() {m_vboHighEdge.destroy();}


    protected:
        Sail const *m_pSail;


        bool m_bResetglSectionHighlight;
        bool m_bResetglSail;

        QOpenGLBuffer m_vboTriangle;

        QOpenGLBuffer m_vboTriangulation, m_vboTess;
        QOpenGLBuffer m_vboSailOutline;
        QOpenGLBuffer m_vboTriangleNormals;
        QOpenGLBuffer m_vboSectionHighlight;
        QOpenGLBuffer m_vboSailMesh, m_vboSailMeshEdges;

        QOpenGLBuffer m_vboSailSurfNormals; // debug only
        QOpenGLBuffer m_vboHighEdge, m_vboPickedEdge;
};

