/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <xfl/opengl/gl3dxflview.h>


class Boat;
class Sail;
class NURBSSurface;

class gl3dBoatView : public gl3dXflView
{
    Q_OBJECT

    public:
        gl3dBoatView(QWidget *pParent=nullptr);
        ~gl3dBoatView();

        void setBoat(Boat* pBoat);

        void resetglBoat()  {m_bResetglBoat = true;}
        void resetglSail()  {m_bResetglSail = true;}
        void resetglHull()  {m_bResetglHull = true;}
        void resetglMesh()  {m_bResetglMesh = true;}
        void resetglSectionHighlight()  {m_bResetglSectionHighlight = true;}

        void resizeSailBuffers(int nSails);

    protected:
        void glRenderView() override;
        void glMake3dObjects() override;
        bool intersectTheObject(Vector3d const &AA, Vector3d const&BB, Vector3d &I) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;

        void glMakeHulls();
        void glMakeSails();
        void glMakeMesh();

        bool bOutline()    const override {return s_bOutline;}
        bool bSurfaces()   const override {return s_bSurfaces;}
        bool bVLMPanels()  const override {return s_bPanels;}
        bool bNormals()    const override {return s_bNormals;}

        void showOutline(bool boutline)   override {s_bOutline=boutline;}
        void showSurfaces(bool bsurfaces) override {s_bSurfaces=bsurfaces;}
        void showPanels(bool bpanels)     override {s_bPanels=bpanels;}
        void showNormals(bool bnormals)   override {s_bNormals=bnormals;}

    public slots:

        void onSurfaces(bool bChecked) override;
        void onPanels(bool bChecked) override;
        void onOutline(bool bChecked) override;
        void onPartSelClicked() override;

    protected:

        Boat *m_pBoat;

        bool m_bResetglSectionHighlight;
        bool m_bResetglBoat;
        bool m_bResetglSail;
        bool m_bResetglHull;
        bool m_bResetglMesh;


        QVector<QOpenGLBuffer> m_vboSailSurface;
        QVector<QOpenGLBuffer> m_vboSailOutline;
        QVector<QOpenGLBuffer> m_vboSailNormals;
        QOpenGLBuffer m_vboBoatMesh, m_vboBoatMeshEdges;
        QOpenGLBuffer m_vboNormals;

        QVector<QOpenGLBuffer> m_vboFuseTriangulation;
        QVector<QOpenGLBuffer> m_vboFuseOutline;

        static bool s_bOutline;
        static bool s_bSurfaces;
        static bool s_bPanels;
        static bool s_bNormals;
};

