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

#include <panel3.h>
#include <node.h>


class Node;

class FL5LIB_EXPORT XflMesh
{
    public:
        XflMesh() = default;
        virtual ~XflMesh() = default;

    public:
        virtual void meshInfo(std::string &info) const = 0;
        virtual void cleanMesh(std::string &logmsg) = 0;

        virtual void clearMesh() = 0;
        virtual void clearPanels() = 0;
        virtual int nPanels() const = 0;
        virtual int panelCount() const = 0;
        virtual int nWakePanels() const = 0;
        virtual int nWakeColumns() const = 0;
        virtual void getMeshInfo(std::string &log) const = 0;
        virtual void getLastTrailingPoint(Vector3d &pt) const = 0;

        virtual void rotate(double alpha, double beta, double phi) = 0;



        void clearNodes()  {m_Node.clear();}
        void setNodes(std::vector<Node> const &meshnodes) {m_Node=meshnodes;}
        Node const &nodeAt(int index) const {return m_Node.at(index);}
        Node &node(int index)  {return m_Node[index];}
        Node const &lastNode() const  {return m_Node.back();}
        Node &lastNode() {return m_Node.back();}
        std::vector<Node> const  & nodes() const {return m_Node;}
        std::vector<Node>   & nodes()  {return m_Node;}
        int nNodes() const {return int(m_Node.size());}
        int nodeCount() const {return int(m_Node.size());}
        void addNode(Node const &nd) {m_Node.push_back(nd);}
        void appendNodes(std::vector<Node> const &nodes) {m_Node.insert(m_Node.end(), nodes.begin(), nodes.end());}
        inline int isNode(const Node &nd, double dist) const;
        inline bool setNode(int index, Node const &nd);

        void listNodes();

        static void setNodeMergeDistance(double l) {s_NodeMergeDistance=l;}
        static double nodeMergeDistance() {return s_NodeMergeDistance;}

    protected:
        std::vector<Node> m_Node;

    public:
        static std::vector<Vector3d> s_DebugPts;

        static double s_NodeMergeDistance;
};


inline bool XflMesh::setNode(int index, Node const &nd)
{
    if(index<0||index>=int(m_Node.size())) return false;
    m_Node[index] = nd;
    return true;
}

inline int XflMesh::isNode(Node const &nd, double dist) const
{
    // start search with the last added node to increase success rate
    for(int idx=int(m_Node.size()-1); idx>=0; idx--)
    {
        if(m_Node.at(idx).isSame(nd, dist))
        {
            return idx;
        }
    }
    return -1;
}

