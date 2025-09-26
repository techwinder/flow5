/****************************************************************************

	flow5 application
	Copyright (C) Andre Deperrois 
	All rights reserved.

*****************************************************************************/
 
#pragma once

#include <xfl/opengl/gl3dxflview.h>


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

