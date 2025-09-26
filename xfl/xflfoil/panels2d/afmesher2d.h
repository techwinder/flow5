/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QObject>

#include <xflcore/flow5events.h>
#include <xflgeom/geom2d/pslg2d.h>
#include <xflgeom/geom2d/triangle2d.h>

class AFMesher2d  : public QObject
{
    Q_OBJECT
    public:
        AFMesher2d();

        void setParent(QObject *pParent) {m_pParent=pParent;}
        void setPSLG(PSLG2d const &pslg, QVector<Node2d> const &nodes) {m_PSLG=pslg; m_Nodes=nodes;}

        bool makeTriangles(QString &logmsg);

        bool bMeshError() const {return m_bError;}
        QVector<Triangle2d> const &triangles() const {return m_Triangles;}
        PSLG2d  const &PSLG() const {return m_PSLG;}
        QVector<Node2d> const& nodes() const {return m_Nodes;}

        static void setMaxIterations(int maxiter) {s_MaxIterations=maxiter;}
        static int maxIterations() {return s_MaxIterations;}

        static double searchRadiusFactor() {return s_SearchRadiusFactor;}
        static void setSearchRadiusFactor(double r) {s_SearchRadiusFactor=r;}

        static double growthFactor() {return s_GrowthFactor;}
        static void setGrowthFactor(double g) {s_GrowthFactor=g;}

        static void setCancelled(bool bCancel) {s_bCancel = bCancel;}
        static void cancelTriangulation() {s_bCancel=true;}

        static void setMultiThreaded(bool b) {s_bMultiThread=b;}
        static bool bMultiThread() {return s_bMultiThread;}

        static void setMaxPanelCount(int imax) {s_MaxPanelCount=imax;}
        static int maxPanelCount() {return s_MaxPanelCount;}

        static void setMaxEdgeLength(double l) {s_MaxEdgeLength=l;}
        static double maxEdgeLength() {return s_MaxEdgeLength;}

        static void setNodeMergeDistance(double l) {s_NodeMergeDistance=l;}
        static float nodeMergeDistance() {return s_NodeMergeDistance;}

    private:
        void postMessageEvent(const QString &msg) const;
        bool makeEquiTriangle(Segment2d const &baseseg, double maxedgelength, double growthfactor, int nodeindex, Triangle2d &triangle);

    public slots:
        void onMakeTriangles();

    signals:
        void meshFinished();
        void animateMesh();

    private:

        QObject *m_pParent;

        PSLG2d m_PSLG; /** The input PSLG */
        QVector<Triangle2d> m_Triangles;
        QVector<Node2d> m_Nodes;

        bool m_bError;

        static int s_MaxIterations;

        static bool s_bCancel;

        static double s_SearchRadiusFactor;
        static double s_MaxEdgeLength;
        static float s_NodeMergeDistance;
        static double s_GrowthFactor;
        static int s_MaxPanelCount;

        static bool s_bMultiThread;

};


class Mesh2dEvent : public QEvent
{
    public:
        Mesh2dEvent(QVector<Triangle2d> const & triangles, PSLG2d const &PSLG, QVector<Node2d> const &nodes): QEvent(MESH2D_UPDATE_EVENT)
        {
            m_Triangles = triangles;
            m_SLG = PSLG;
            m_Nodes = nodes;
            m_bFinal  = false;
        }

        QVector<Triangle2d> const & triangles() const {return m_Triangles;}
        PSLG2d const &SLG() const {return m_SLG;}
        QVector<Node2d> const &nodes() {return m_Nodes;}
        bool isFinal() const {return m_bFinal;}
        void setFinal(bool bFinal) {m_bFinal=bFinal;}

    private:
        PSLG2d m_SLG;
        QVector<Triangle2d> m_Triangles;
        QVector<Node2d> m_Nodes;
        bool m_bFinal; // if true, the mesh is complete
};

