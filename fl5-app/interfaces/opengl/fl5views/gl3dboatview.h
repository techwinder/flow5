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

