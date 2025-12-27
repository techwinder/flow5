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

#include <vector>

#include <xflmesh.h>

#include <panel3.h>
#include <node.h>
#include <triangulation.h>

class Node;

class FL5LIB_EXPORT TriMesh : public XflMesh
{
    public:
        TriMesh();
        ~TriMesh() = default;

        void clearMesh()   override {m_Panel3.clear(); m_Node.clear(); m_WakePanel3.clear();}
        void clearPanels() override {m_Panel3.clear(); m_WakePanel3.clear();}

        void meshInfo(std::string &info) const override;
        void cleanMesh(std::string &logmsg) override;
        int nPanels() const override {return int(m_Panel3.size());}
        int nWakePanels() const override {return int(m_WakePanel3.size());}
        int nWakeColumns() const override {return m_nWakeColumns;}
        int panelCount() const override {return int(m_Panel3.size());}
        void getMeshInfo(std::string &logmsg) const override;
        void getLastTrailingPoint(Vector3d &pt) const override;

        void rotate(double alpha, double beta, double phi) override;

        void rotatePanels(Vector3d const &O, const Vector3d &axis, double theta);

        void getFreeEdges(std::vector<Segment3d> &freeedges) const;

        int cleanNullTriangles();

        void addPanel(Panel3 const &p3);
        void addPanels(std::vector<Panel3>const &panel3list) {m_Panel3.insert(m_Panel3.end(), panel3list.begin(), panel3list.end());}
        void addPanels(std::vector<Panel3>const &panel3list, Vector3d const &position);
        void appendMesh(TriMesh const &mesh);

        bool setPanel(int index, Panel3 const &p3);

        void connectNodes();
        int makeNodeArrayFromPanels(int firstnodeindex, std::string &logmsg, const std::string &prefix);

        void makeNodeNormals(bool bReversed=false);

        void clearConnections();
        void connectMeshes(int i0, int n0, int i1, int n1);
        void makeConnectionsFromNodeIndexes(int i0, int n0, int i1, int n1);
        void makeConnectionsFromNodePosition(bool bConnectTE, bool bMultiThread);

        void makeConnectionsFromNodePosition2(int i0, int n3_0, double MERGEDISTANCE);

        void copyConnections(TriMesh const &mesh3);

        void checkElementSize(double minsize, std::vector<int> &elements, std::vector<double> &size);

        void serializePanelsFl5(QDataStream &ar, bool bIsStoring);
        void serializeMeshFl5(  QDataStream &ar, bool bIsStoring);

        void makeMeshFromTriangles(const std::vector<Triangle3d> &triangulation, int firstindex, xfl::enumSurfacePosition pos, std::string &logmsg, const std::string &prefix);
        void makeMeshTriangleBlock();

        void checkPanels(std::string &logmsg, bool bSkinny, bool bMinAngle, bool bMinArea, bool bMinSize, std::vector<int> &skinnylist, std::vector<int> &minanglelist, std::vector<int> &minarealist, std::vector<int> &minsizelist, double qualityfactor, double minangle, double minarea, double minsize);

        void removePanelAt(int index);
        void removeLastPanel() {m_Panel3.pop_back();}

        Panel3 &panel(int index) {return m_Panel3[index];}
        Panel3 const &panelAt(int index) const {return m_Panel3.at(index);}

        Panel3 const &lastPanel() const {return m_Panel3.back();}
        Panel3 &lastPanel() {return m_Panel3.back();}

        std::vector<Panel3> & panels() {return m_Panel3;}
        std::vector<Panel3> const & panels() const {return m_Panel3;}

        std::vector<Panel3> & wakePanels() {return m_WakePanel3;}
        std::vector<Panel3> const & wakePanels() const {return m_WakePanel3;}
        Panel3 const &wakePanelAt(int index) const {return m_WakePanel3.at(index);}

        static void rebuildPanelsFromNodes(std::vector<Panel3> &panel3, const std::vector<Node> &node);
        void rebuildPanels();

        void translatePanels(Vector3d const &T);
        void translatePanels(double tx, double ty, double tz);
        void setNodePanels();

        void listPanels(bool bConnections=false);

        int cleanDoubleNodes(double precision, std::string &logmsg, const std::string &prefx);
        static int cleanDoubleNodes(std::vector<Panel3> &panel3, std::vector<Node> &nodes, double precision, std::string &logmsg, const std::string &prefx);

        void scale(double sx, double sy, double sz);

        void clearWakePanels() {m_WakePanel3.clear();}
        void makeWakePanels(int nxWakePanels, double wakepanelfactor, double TotalWakeLength, Vector3d const &WindDir, bool bAlignWakeTE);
        void connectWakePanels();

        bool connectTrailingEdges(std::vector<int> &errorlist);
        void checkTrailingEdges(std::vector<int> &errorlist);

        bool mergeNodes(int srcindex, int destindex, bool bDiscardNullPanels, std::string &logmsg, std::string prefx);
        int mergeFuseToWingNodes(double precision, std::string &logmsg, const std::string &prefx);


        static int makeWakePanels(std::vector<Panel3> &Panel3List, int nxWakePanels, double wakepanelfactor, double TotalWakeLength, const Vector3d &WindDir, std::vector<Panel3> &WakePanel3, int &nWakeColumn, bool bAlignWakeTE);
        static void makeNodeValues(const std::vector<Node> &NodeList, const std::vector<Panel3> &Panel3List,
                                   std::vector<double> const &VertexValues, std::vector<double> &NodeValues,
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
        std::vector<Panel3> m_Panel3;
        std::vector<Panel3> m_WakePanel3;
        int m_nBlocks;

        int m_nWakeColumns;

        static bool s_bCancel;

};

