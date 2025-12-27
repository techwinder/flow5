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

class WingXfl;

class gl3dWingView : public gl3dXflView
{
    public:
        gl3dWingView(QWidget *pParent = nullptr);
        ~gl3dWingView();

        void setWing(WingXfl *pWing){m_pWing = pWing;}

        void glMake3dObjects() override;

        void paintWingSectionHighlight();

        void resetglWing() {m_bResetglWing=true;}
        void resetglSectionHighlight(int iSection, bool bRightSide){m_bResetglSectionHighlight=true; m_iSection = iSection; m_bRightSide=bRightSide;}


        void setHighlighting(bool bHighlighting) {m_bHighlightSection=bHighlighting; update();}
        void toggleHighlighting() {m_bHighlightSection=!m_bHighlightSection; update();}

        std::vector<Node> const &nodes() const;

    private:
        void glRenderView() override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        bool intersectTheObject(Vector3d const &AA,  Vector3d const&BB, Vector3d &I) override;

        void glMakeWingSectionHighlight(const WingXfl *pWing, int iSectionHighLight, bool bRightSide);
        void glMakeWingSectionHighlight_seg(const WingXfl *pWing, int iSectionHighLight);

    private:
        WingXfl *m_pWing;

        bool m_bResetglWing;
        bool m_bResetglSectionHighlight;
        bool m_bHighlightSection;
        bool m_bRightSide;

        int m_iSection;
        int m_nHighlightLines, m_HighlightLineSize;

        QOpenGLBuffer m_vboSurface, m_vboTessellation;
        QOpenGLBuffer m_vboOutline;
        QOpenGLBuffer m_vboSectionHighlight;
        QOpenGLBuffer m_vboTessNormals;
        QOpenGLBuffer m_vboTriMesh, m_vboTriEdges;
        QOpenGLBuffer m_vboNormals;

};
