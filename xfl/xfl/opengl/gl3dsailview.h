/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <TopoDS_Edge.hxx>

#include <xfl/opengl/gl3dxflview.h>


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
        void keyPressEvent(QKeyEvent *pEvent) override;

        void paintControlPoints();
        void setHighlightedEdge(TopoDS_Edge const &Edge);
        void glMakeSailNormals(const Sail *pSail, float length, QOpenGLBuffer &vbo); // debug only
        void clearHighlightedEdge() {m_vboHighEdge.destroy();}


    protected:
        Sail const *m_pSail;


        bool m_bResetglSectionHighlight;
        bool m_bResetglSail;

        QOpenGLBuffer m_vboTriangle;

        QOpenGLBuffer m_vboTriangulation;
        QOpenGLBuffer m_vboSailOutline;
        QOpenGLBuffer m_vboTriangleNormals;
        QOpenGLBuffer m_vboSectionHighlight;
        QOpenGLBuffer m_vboSailMesh, m_vboSailMeshEdges;

        QOpenGLBuffer m_vboSailSurfNormals; // debug only
        QOpenGLBuffer m_vboHighEdge, m_vboPickedEdge;
};

