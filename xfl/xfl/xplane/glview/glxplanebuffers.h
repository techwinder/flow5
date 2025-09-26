/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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
class WPolar;
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
        void setLiveVortons(double alpha, QVector<QVector<Vorton>> const &vortons);
        void clearLiveVortons();

        void resizeFuseGeometryBuffers(int nFuse);

        static void setXPlane(XPlane *pXPlane) {s_pXPlane = pXPlane;}

    private:


        double m_LiveAlpha;
        QVector<QVector<Vorton>> m_LiveVortons;

        QOpenGLBuffer m_vboCp;
        QOpenGLBuffer m_vboGamma;
        QOpenGLBuffer m_vboPanelForces;
        QOpenGLBuffer m_vboStreamLines;

        QOpenGLBuffer m_vboWakePanels;
        QOpenGLBuffer m_vboWakeEdges;
        QOpenGLBuffer m_vboWakeNormals;
        QOpenGLBuffer m_vboDW;

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
        QOpenGLBuffer m_vboIndDrag, m_vboViscDrag;
        QOpenGLBuffer m_vboMoments;
        QOpenGLBuffer m_vboStripLift;




        static XPlane *s_pXPlane;     /**< A void pointer to the instance of the XPlane widget.*/

    public:
};

