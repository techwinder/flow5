/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QLabel>

#include <xfl/opengl/gl3dxflview.h>
#include <xflobjects/objects3d/fuse/fuse.h>


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

