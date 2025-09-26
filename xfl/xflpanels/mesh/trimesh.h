/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QVector>

#include <xflpanels/mesh/xflmesh.h>

#include <xflpanels/panels/panel3.h>
#include <xflgeom/geom3d/node.h>
#include <xflgeom/geom3d/triangulation.h>

class Node;

class TriMesh : public XflMesh
{
    public:
        TriMesh();
        ~TriMesh() = default;

        void clearMesh()   override {m_Panel3.clear(); m_Node.clear(); m_WakePanel3.clear();}
        void clearPanels() override {m_Panel3.clear(); m_WakePanel3.clear();}

        void meshInfo(QString &info) const override;
        void cleanMesh(QString &logmsg) override;
        int nPanels() const override {return m_Panel3.size();}
        int nWakePanels() const override {return m_WakePanel3.size();}
        int nWakeColumns() const override {return m_nWakeColumns;}
        int panelCount() const override {return m_Panel3.size();}
        void getMeshInfo(QString &log) const override;
        void getLastTrailingPoint(Vector3d &pt) const override;

        void rotate(double alpha, double beta, double phi) override;

        void rotatePanels(Vector3d const &O, const Vector3d &axis, double theta);

        void getFreeEdges(QVector<Segment3d> &freeedges) const;

        int cleanNullTriangles();

        void addPanel(Panel3 const &p3);
        void addPanels(QVector<Panel3>const &panel3list) {m_Panel3.append(panel3list);}
        void addPanels(QVector<Panel3>const &panel3list, Vector3d const &position);
        void appendMesh(TriMesh const &mesh);

        bool setPanel(int index, Panel3 const &p3);

        void connectNodes();
        int makeNodeArrayFromPanels(int firstnodeindex, QString &logmsg, const QString &prefix);

        void makeNodeNormals(bool bReversed=false);

        void clearConnections();
        void connectMeshes(int i0, int n0, int i1, int n1);
        void makeConnectionsFromNodeIndexes(int i0, int n0, int i1, int n1);
        void makeConnectionsFromNodePosition(bool bConnectTE, bool bMultiThread);

        void makeConnectionsFromNodePosition(int i0, int n3_0, double MERGEDISTANCE, bool bCheckSurfacePosition);

        void copyConnections(TriMesh const &mesh3);

        void checkElementSize(double minsize, QVector<int> &elements, QVector<double> &size);

        void serializePanelsFl5(QDataStream &ar, bool bIsStoring);
        void serializeMeshFl5(  QDataStream &ar, bool bIsStoring);

        void makeMeshFromTriangles(const QVector<Triangle3d> &triangulation, int firstindex, xfl::enumSurfacePosition pos, QString &logmsg, const QString &prefix);
        void makeMeshTriangleBlock();

        void checkPanels(QString &log, bool bSkinny, bool bMinAngle, bool bMinArea, bool bMinSize, QVector<int> &skinnylist, QVector<int> &minanglelist, QVector<int> &minarealist, QVector<int> &minsizelist, double qualityfactor, double minangle, double minarea, double minsize);

        void removePanelAt(int index);
        void removeLastPanel() {m_Panel3.pop_back();}

        Panel3 &panel(int index) {return m_Panel3[index];}
        Panel3 const &panelAt(int index) const {return m_Panel3.at(index);}

        Panel3 const &lastPanel() const {return m_Panel3.last();}
        Panel3 &lastPanel() {return m_Panel3.last();}

        QVector<Panel3> & panels() {return m_Panel3;}
        QVector<Panel3> const & panels() const {return m_Panel3;}

        QVector<Panel3> & wakePanels() {return m_WakePanel3;}
        QVector<Panel3> const & wakePanels() const {return m_WakePanel3;}
        Panel3 const &wakePanelAt(int index) const {return m_WakePanel3.at(index);}

        static void rebuildPanelsFromNodes(QVector<Panel3> &panel3, QVector<Node> const &node);
        void rebuildPanels();

        void translatePanels(Vector3d const &T);
        void translatePanels(double tx, double ty, double tz);
        void setNodePanels();

        void listPanels(bool bConnections=false);

        int cleanDoubleNodes(double precision, QString &logmsg, const QString &prefix);
        static int cleanDoubleNodes(QVector<Panel3> &panel3, QVector<Node> &nodes, double precision, QString &logmsg, QString const &prefix);

        void scale(double sx, double sy, double sz);

        void clearWakePanels() {m_WakePanel3.clear();}
        void makeWakePanels(int nxWakePanels, double wakepanelfactor, double TotalWakeLength, Vector3d const &WindDir, bool bAlignWakeTE);
        void connectWakePanels();

        bool connectTrailingEdges(QVector<int> &errorlist);
        void checkTrailingEdges(QVector<int> &errorlist);

        bool mergeNodes(int srcindex, int destindex, bool bDiscardNullPanels, QString &log, QString prefix);
        int mergeFuseToWingNodes(double precision, QString &logmsg, const QString &prefix);

        QVector<Vector3d> ctrlPts(bool bTriLin, double offset) const;

        static int makeWakePanels(QVector<Panel3> &Panel3List, int nxWakePanels, double wakepanelfactor, double TotalWakeLength, const Vector3d &WindDir, QVector<Panel3> &WakePanel3, int &nWakeColumn, bool bAlignWakeTE);
        static void makeNodeValues(QVector<Node> const & NodeList, QVector<Panel3> const &Panel3List,
                                   QVector<double> const &VertexValues, QVector<double> &NodeValues,
                                   double &valmin, double &valmax, double coef=1.0);

        static void cancelTask() {s_bCancel=true;}
        static void setCancelled(bool bCancel) {s_bCancel=bCancel;}

    private:
        void connectPanelBlock(int iBlock, bool bConnectTE);

        void savePanels(QDataStream &ar);
        void loadPanels(QDataStream &ar);

        void saveMesh(QDataStream &ar);
        void loadMesh(QDataStream &ar);

    private:
        QVector<Panel3> m_Panel3;
        QVector<Panel3> m_WakePanel3;
        int m_nBlocks;

        int m_nWakeColumns;

        static bool s_bCancel;

};

