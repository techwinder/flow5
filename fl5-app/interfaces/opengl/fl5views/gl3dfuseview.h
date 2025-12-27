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

#include <QLabel>

#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <api/fuse.h>


class gl3dFuseView : public gl3dXflView
{
    public:
        gl3dFuseView(QWidget *pParent = nullptr);
        ~gl3dFuseView() override;

        void setFuse(const Fuse *pFuse);

        void resetFuse()    {m_bResetglFuse = true;}
        void resetPanels()  {m_bResetglPanels = true;}
        void resetNormals() {m_bResetglNormals = true;}
        void resetFrameHighlight() {m_bResetglFrameHighlight=true;}

    private:
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;

        void glMake3dObjects() override;
        void glRenderView() override;

        bool intersectTheObject(Vector3d const &AA, Vector3d const &U, Vector3d &I) override;

        void paintEditTriMesh(bool bBackground);
        void glMakeBodyFrameHighlight(const FuseXfl *pFuseXfl, Vector3d const &bodyPos, int NHOOOP, int iFrame);


    protected:
        Fuse const *m_pFuse;

        bool m_bResetglFrameHighlight;
        bool m_bResetglFuse;
        bool m_bResetglPanels;
        bool m_bResetglNormals;


    protected:
        QOpenGLBuffer m_vboSurface, m_vboTess;
        QOpenGLBuffer m_vboOutline;
        QOpenGLBuffer m_vboTriangleNormals;
        QOpenGLBuffer m_vboHighlight;
        QOpenGLBuffer m_vboTriPanels, m_vboTriPanelEdges;
        QOpenGLBuffer m_vboFrames;

        int m_nHighlightLines, m_HighlightLineSize;

};

