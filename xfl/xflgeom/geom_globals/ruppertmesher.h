/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QVector>
#include <QSettings>


#include <xflgeom/geom2d/pslg2d.h>
#include <xflgeom/geom2d/vector2d.h>
#include <xflgeom/geom2d/segment2d.h>
#include <xflgeom/geom2d/triangle2d.h>


class WingXfl;
class FuseXfl;

class RuppertMesher
{
    public:
        RuppertMesher();
        ~RuppertMesher();

        int makeDelaunayTriangulation(QVector<Vector2d> const &Vertices,
                                      QVector<Triangle2d> &triangles,
                                      Triangle2d &supertriangle, bool bClearSuperTriangles) const;
        int makeConstrainedDelaunay(const QVector<Vector2d> &vertices, PSLG2d &PSLG,
                                    QVector<Triangle2d> &triangles, int nPSLGInsertions, bool bClearSuperTriangle) const;

        bool triangulateClosedPolygon(const PSLG2d &pslg, QVector<Triangle2d> triangles);

        QVector<Triangle2d> doRuppert(const PSLG2d &contourpslg, int maxiters, int maxpanels, QString &log, bool &bConverged);

        void makeConnections();
        PSLG2d uniqueEdges(QVector<Triangle2d> const &panelList) const;

        void clearSuperTriangle(QVector<Triangle2d> &TriangleList, Triangle2d super) const;

        void addPSLGVertices(QVector<Vector2d> &VertexList, double precision=1.0e-5); // Occ error can be 1.e-6
        bool checkDelaunayProperty(QVector<Triangle2d> const &TriangleList) const;

        bool areVisible(Vector2d vtx0, Vector2d vtx1, PSLG2d const & PSLG);

        void insertVertex(int t2d_index, const Vector2d &newvtx, QVector<Triangle2d> &triangles, QVector<int> &removedlist, QVector<int> &addedlist, PSLG2d &PSLG, bool bCheckPSLGIntersections) const;

        int intersectPSLGSegment(const Segment2d &seg2d, Vector2d &I) const;
        int splitOversizePSLG();
        void splitPSLGSegment(int index);
        void splitTriangle(int index);
        int splitOversizedTriangles(bool bSplitSPLG=true);
        int flipDelaunay(QVector<int> &fliplist);

        void clearAll();

        void doAllMeshSteps(PSLG2d const &inPSLG, const PSLG2d &contourPSLG, QVector<PSLG2d> innerPSLG);

        void listNonCompliant(QVector<int>&encroachedSegs, QVector<int> &skinnyTris, QVector<int> &longTris) const;
        void listNonCompliant(const QVector<Triangle2d> &triangles, const PSLG2d &PSLG, QVector<int>&encroachedSegs, QVector<int> &skinnyTris, QVector<int> &longTris) const;

        int listSkinnyTriangles(QVector<Triangle2d> const &triangles, QVector<int> &skinnylist) const;
        int listLongTriangles(QVector<Triangle2d> const &triangles, QVector<int> &longTris) const;
        int listEncroachedSegments(const PSLG2d &PSLG, const QVector<Triangle2d> &triangles, QVector<int>&encroachedSegs) const;

        void setTriangles(QVector<Triangle2d> const &triangles) {m_LastTriangles = triangles;}
        void setPSLG(PSLG2d const &pslg) {m_LastPSLG = pslg;}

        QVector<Triangle2d> &lastTriangles() {return m_LastTriangles;}
        QVector<Triangle2d> const &lastTriangles() const {return m_LastTriangles;}
        PSLG2d  &lastPSLG()      {return m_LastPSLG;}
        PSLG2d  const &lastPSLG()      const {return m_LastPSLG;}

        int lastTriangleCount() const {return m_LastTriangles.size();}
        int lastPSLGCount()     const {return m_LastPSLG.size();}

        Triangle2d const &lastTriangle(int index) const {return m_LastTriangles.at(index);}
        Segment2d  const &lastPSLG(int index)     const {return m_LastPSLG.at(index);}

        double maxEdgeLength() const {return m_MaxEdgeLength;}
        void setMaxEdgeLength(double maxsize) {m_MaxEdgeLength=maxsize;}

        double minEdgeLength() const {return m_MinEdgeLength;}
        void setMinEdgeLength(double minsize) {m_MinEdgeLength=minsize;}

        int maxPanelCount() const {return m_MaxPanelCount;}
        void setMaxPanelCount(int maxcount) {m_MaxPanelCount=maxcount;}

        void setMaxIter(int iter) {m_MaxMeshIter = iter;}
        int maxMeshIter() const {return m_MaxMeshIter;}

        void addInnerPSLG(PSLG2d const&pslg) {m_InnerPSLG.append(pslg);}
        void addInnerPSLG(QVector<PSLG2d> const&PSLGs) {m_InnerPSLG.append(PSLGs);}

        void clearInnerPSLG() {m_InnerPSLG.clear();}
        int clearTrianglesInInnerPSLG(const QVector<PSLG2d> &innerPSLG);
        int clearTrianglesInPSLG(PSLG2d const &pslg);

        int clearTrianglesOutsidePSLG(PSLG2d const &pslg, QVector<Triangle2d> &triangles) const;

        static void setCancelled(bool bCancel) {s_bCancel = bCancel;}

    private:

        int m_MaxMeshIter;

        QVector<Triangle2d> m_LastTriangles;
        PSLG2d m_LastPSLG;
        QVector<PSLG2d> m_InnerPSLG;

        double m_MinEdgeLength;
        double m_MaxEdgeLength;
        int m_MaxPanelCount;

        static bool s_bCancel;
};



