/****************************************************************************

    flow5
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

#include <QObject>

#include <xflcore/flow5events.h>
#include <xflgeom/geom3d/slg3d.h>
#include <xflgeom/geom3d/triangle3d.h>
#include <xflpanels/mesh/shell/edgesplit.h>

#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Edge.hxx>

class PSLG2d;
class SLG3d;
class Sail;

class AFMesher : public QObject
{
    Q_OBJECT

    public:
        AFMesher();
        void setParent(QObject *pParent) {m_pParent=pParent;}

        void setSplittableInnerPSLG(bool bSplit) {m_bSplittableInnerPSLG=bSplit;}
        void setEdgeSplit(QVector<QVector<EdgeSplit>> const &splits) {m_EdgeSplit=splits;}
        bool triangulateShell(TopoDS_Shell const &shell, QVector<Triangle3d> &triangles, QString &logmsg);
        bool makeTriangles(const TopoDS_Face &aFace,
                           SLG3d &slg3d,
                           QVector<Triangle3d> &facetriangles, double MaxEdgeLength, double MaxPanelCount,
                           QString &logmsg) const;
        void makeDelaunayFlips(QVector<Triangle3d> &, int &nFlips, int &nIter) const;


        QVector<Triangle3d> const &triangles() const {return m_Triangles;}

        bool bMeshError() const {return m_bError;}

        void clearShapes() {m_Shapes.Clear();}
        void appendShape(TopoDS_Shape const &shape) {m_Shapes.Append(shape);}

        static void setMaxIterations(int maxiter) {s_MaxIterations=maxiter;}
        static int maxIterations() {return s_MaxIterations;}

        static void setTraceFaceIndex(int idx) {s_TraceFaceIdx=idx;}
        static int traceFaceIndex() {return s_TraceFaceIdx;}

        static bool isAnimating() {return s_bIsAnimating;}
        static void setAnimating(bool b) {s_bIsAnimating=b;}

        static int animationPause() {return s_AnimationPause;}
        static void setAnimationPause(int pause_ms) {s_AnimationPause=pause_ms;}

        static double searchRadiusFactor() {return s_SearchRadiusFactor;}
        static void setSearchRadiusFactor(double r) {s_SearchRadiusFactor=r;}

        static double growthFactor() {return s_GrowthFactor;}
        static void setGrowthFactor(double g) {s_GrowthFactor=g;}

        static void setCancelled(bool bCancel) {s_bCancel = bCancel;}
        static void cancelTriangulation() {s_bCancel=true;}

        static void setDelaunayFlips(bool b) {s_bDelaunay=b;}
        static bool bDelaunayFlips() {return s_bDelaunay;}


        static void setMaxPanelCount(int imax) {s_MaxPanelCount=imax;}
        static int maxPanelCount() {return s_MaxPanelCount;}

        static void setMaxEdgeLength(double l) {s_MaxEdgeLength=l;}
        static double maxEdgeLength() {return s_MaxEdgeLength;}

    private:
        void postAnimateEvent() const;
        void postMessageEvent(const QString &msg) const;

        void getEdgeSplit(TopoDS_Edge const &edge, QVector<double> &uval) const;
        bool triangulateFace(TopoDS_Face const &aFace, int iFace);


        void makeFacePSLG2d(const TopoDS_Face &aFace,
                            QVector<PSLG2d> &innerpslgUV, PSLG2d &contourpslgUV, QString &logmsg) const;

        void makeFaceSLG3dFrom2d(TopoDS_Face const &aFace, PSLG2d const &pslg2d, SLG3d &pslg3d, QString &logmsg) const;
        void makeFaceSLG3d(const TopoDS_Face &aFace, QVector<SLG3d> &innerslg, SLG3d &contourpslgUV, QString &logmsg) const;

        // Sail mesh
        bool makeEquiTriangleOnSail(const Sail *pSail, Segment3d const &baseseg, double maxedgelength, double growthfactor, Triangle3d &triangle);

    signals:
        void meshFinished();
        void animateMesh();

    public slots:
        void triangulateShells();
        bool makeThinSailMesh();

    public:
        bool m_bError;

        QVector<Triangle3d> m_Triangles;

        TopoDS_ListOfShape m_Shapes;

        void setSail(Sail*pSail) {m_pSail=pSail;}
        Sail *m_pSail;

        bool m_bSplittableInnerPSLG;
        QVector<QVector<EdgeSplit>> m_EdgeSplit; // for each face<each edge>

        QObject *m_pParent;


        QVector<QVector<Triangle3d>> m_FaceTriangles;
        QVector<SLG3d> m_Segs;

        static int s_MaxIterations;

        static int s_TraceFaceIdx;
        static int s_AnimationPause;
        static bool s_bIsAnimating;

        static SLG3d s_SLG; /** Debug and animation */
        static QVector<Triangle3d> s_Triangles; /** Debug and animation */

        static QVector<Vector3d> s_DebugPts;
        static bool s_bCancel;

        static double s_SearchRadiusFactor;
        static double s_MaxEdgeLength;
        static double s_GrowthFactor;
        static int s_MaxPanelCount;
        static bool s_bDelaunay;
};



class MeshEvent : public QEvent
{

    public:
        MeshEvent(): QEvent(MESH_UPDATE_EVENT)
        {
            m_Triangles = AFMesher::s_Triangles;
            m_SLG = AFMesher::s_SLG;
            m_bFinal  = false;
        }

        MeshEvent(QVector<Triangle3d> const & triangles): QEvent(MESH_UPDATE_EVENT)
        {
            m_Triangles = triangles;
            m_SLG = AFMesher::s_SLG;
            m_bFinal  = false;
        }

        QVector<Triangle3d> const & triangles() const {return m_Triangles;}
        SLG3d const &SLG() const {return m_SLG;}
        bool isFinal() const {return m_bFinal;}
        void setFinal(bool bFinal) {m_bFinal=bFinal;}

    private:
        SLG3d m_SLG;
        QVector<Triangle3d> m_Triangles;
        bool m_bFinal; // if true, the mesh is complete
};
