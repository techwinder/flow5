/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once
#include <xfl/opengl/gl3dxflview.h>

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

        QVector<Node> const &nodes() const {return m_Nodes;}

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
        QVector<Panel3> m_Panel3;
        QVector<Node> m_Nodes;

        QOpenGLBuffer m_vboSurface, m_vboTessellation;
        QOpenGLBuffer m_vboOutline;
        QOpenGLBuffer m_vboSectionHighlight;
        QOpenGLBuffer m_vboTessNormals;
        QOpenGLBuffer m_vboTriMesh, m_vboTriEdges;
        QOpenGLBuffer m_vboNormals;

};
