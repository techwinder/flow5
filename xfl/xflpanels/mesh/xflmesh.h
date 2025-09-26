/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QVector>

#include <xflpanels/panels/panel3.h>
#include <xflgeom/geom3d/node.h>


class Node;

class XflMesh
{
    public:
        XflMesh() = default;
        virtual ~XflMesh() = default;

    public:
        virtual void meshInfo(QString &info) const = 0;
        virtual void cleanMesh(QString &logmsg) = 0;

        virtual void clearMesh() = 0;
        virtual void clearPanels() = 0;
        virtual int nPanels() const = 0;
        virtual int panelCount() const = 0;
        virtual int nWakePanels() const = 0;
        virtual int nWakeColumns() const = 0;
        virtual void getMeshInfo(QString &log) const = 0;
        virtual void getLastTrailingPoint(Vector3d &pt) const = 0;

        virtual void rotate(double alpha, double beta, double phi) = 0;



        void clearNodes()  {m_Node.clear();}
        void setNodes(QVector<Node> const &meshnodes) {m_Node=meshnodes;}
        Node const &nodeAt(int index) const {return m_Node.at(index);}
        Node &node(int index)  {return m_Node[index];}
        Node const &lastNode() const  {return m_Node.back();}
        Node &lastNode() {return m_Node.back();}
        QVector<Node> const  & nodes() const {return m_Node;}
        QVector<Node>   & nodes()  {return m_Node;}
        int nNodes() const {return m_Node.size();}
        int nodeCount() const {return m_Node.size();}
        void addNode(Node const &nd) {m_Node.push_back(nd);}
        void appendNodes(QVector<Node> const &nodes) {m_Node.append(nodes);}
        inline int isNode(const Node &nd, double dist) const;
        inline bool setNode(int index, Node const &nd);

        void listNodes();

        static void setNodeMergeDistance(double l) {s_NodeMergeDistance=l;}
        static double nodeMergeDistance() {return s_NodeMergeDistance;}

    protected:
        QVector<Node> m_Node;

    public:
        static QVector<Vector3d> s_DebugPts;

        static double s_NodeMergeDistance;
};


inline bool XflMesh::setNode(int index, Node const &nd)
{
    if(index<0||index>=m_Node.size()) return false;
    m_Node[index] = nd;
    return true;
}

inline int XflMesh::isNode(Node const &nd, double dist) const
{
    // start search with the last added node to increase success rate
    for(int idx=m_Node.count()-1; idx>=0; idx--)
    {
        if(m_Node.at(idx).isSame(nd, dist))
        {
            return idx;
        }
    }
    return -1;
}

