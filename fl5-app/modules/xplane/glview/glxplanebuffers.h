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


#include <QOpenGLBuffer>
#include <QVector>


class Vorton;
class Fuse;
class MainFrame;
class Node;
class Panel3;
class Panel4;
class PlaneXfl;
class PlaneOpp;
class Polar3d;
class PlanePolar;
class WingXfl;
class XPlane;
class Vector3d;


class glXPlaneBuffers
{
    friend class gl3dXPlaneView;
    friend class gl3dXSailView;

    public:
        glXPlaneBuffers();
        virtual ~glXPlaneBuffers();

        void clearBuffers();
        void setLiveVortons(double alpha, std::vector<std::vector<Vorton>> const &vortons);
        void clearLiveVortons();

        void resizeFuseGeometryBuffers(int nFuse);

        static void setXPlane(XPlane *pXPlane) {s_pXPlane = pXPlane;}

    private:


        double m_LiveAlpha;
        std::vector<std::vector<Vorton>> m_LiveVortons;

        QOpenGLBuffer m_vboCp;
        QOpenGLBuffer m_vboGamma;
        QOpenGLBuffer m_vboPanelForces;
        QOpenGLBuffer m_vboStreamLines;

        QOpenGLBuffer m_vboWakePanels;
        QOpenGLBuffer m_vboWakeEdges;
        QOpenGLBuffer m_vboWakeNormals;

        QOpenGLBuffer m_vboPanelNormals;

        QOpenGLBuffer m_vboPlaneStlTriangulation, m_vboPlaneStlOutline;
        QVector<QOpenGLBuffer> m_vboWingSurface, m_vboWingOutline;
        QVector<QOpenGLBuffer> m_vboFuseTriangulation, m_vboBodyOutline;
        QOpenGLBuffer m_vboContourLines, m_vboContourClrs;

        QOpenGLBuffer m_vboVortices; // VLM only


        QOpenGLBuffer m_vboGridVelocities;
        QOpenGLBuffer m_vboVortonLines;
        QOpenGLBuffer m_vboVortons;

        QOpenGLBuffer m_vboMesh;
        QOpenGLBuffer m_vboMeshEdges;
        QOpenGLBuffer m_vboNormals;
        QOpenGLBuffer m_vboFrames;
        QOpenGLBuffer m_vboTrans;
        QOpenGLBuffer m_vboInducedDrag, m_vboViscousDrag;
        QOpenGLBuffer m_vboMoments;
        QOpenGLBuffer m_vboStripLift;
        QOpenGLBuffer m_vboDownwash;




        static XPlane *s_pXPlane;     /**< A void pointer to the instance of the XPlane widget.*/

    public:
};

