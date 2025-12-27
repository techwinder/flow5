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

#include <QDataStream>

#include <triangle3d.h>
#include <node.h>



class FL5LIB_EXPORT Triangulation
{
    public:
        Triangulation();

        Triangle3d const & triangleAt(int idx) const {return m_Triangle.at(idx);}
        Triangle3d &triangle(int idx) {return m_Triangle[idx];}

        void setTriangles(std::vector<Triangle3d> const& trianglelist) {m_Triangle=trianglelist;}
        void setTriangle(int it, Triangle3d const &t3d) {if(it<0 || it>=int(m_Triangle.size())) return; else m_Triangle[it]=t3d;}
        void appendTriangle(Triangle3d const& triangle) {m_Triangle.push_back(triangle);}
        void appendTriangles(std::vector<Triangle3d> const& trianglelist) {m_Triangle.insert(m_Triangle.end(), trianglelist.begin(),trianglelist.end());}

        void makeXZsymmetric();
        void clearConnections();
        void makeTriangleConnections();
        int makeNodes();
        void makeNodeNormals(bool bReversed=false);

        int isTriangleNode(const Node &nd) const;

        void translate(const Vector3d &T);
        void rotate(const Vector3d &Origin, const Vector3d &axis, double theta);
        void scale(double XFactor, double YFactor, double ZFactor);
        void flipXZ();

        bool areNeighbours(Triangle3d const &t1, Triangle3d const &t2) const;

        void clear() {m_Triangle.clear();  m_Node.clear();}
        int nTriangles() const {return int(m_Triangle.size());}
        void setTriangleCount(int ntriangles) {m_Triangle.resize(ntriangles);}

        bool intersect(const Vector3d &A, const Vector3d &B, Vector3d &Inear, Vector3d &N) const;

        std::vector<Triangle3d> &triangles() {return m_Triangle;}
        std::vector<Triangle3d> const &triangles() const {return m_Triangle;}

        std::vector<Node> &nodes() {return m_Node;}
        std::vector<Node> const &nodes() const{return m_Node;}
        int nNodes()    const {return int(m_Node.size());}
        int nodeCount() const {return int(m_Node.size());}
        void setNodeCount(int nnodes) {m_Node.resize(nnodes);}

        void clearNodes() {m_Node.clear();}
        void setNodes(std::vector<Node> const &nodes) {m_Node=nodes;}
        void setNode(int in, Node const &nd) {if(in<0||in>nodeCount()) return; else m_Node[in]=nd;}
        void appendNode(Node const &node) {m_Node.push_back(node);}
        void appendNodes(std::vector<Node> const &nodes) {m_Node.insert(m_Node.end(), nodes.begin(), nodes.end());}
        Node const & nodeAt(int inode) const {return m_Node.at(inode);}
        Node & node(int inode) {return m_Node[inode];}

        double wettedArea() const;
        void computeSurfaceProperties(double &lx, double &ly, double &lz, double &wettedarea);

        void getFreeEdges(std::vector<Segment3d> &freeedges) const;

        void flipNormals();

        bool serializeFl5(QDataStream &ar, bool bIsStoring);

    private:
        std::vector<Triangle3d> m_Triangle;
        std::vector<Node> m_Node;
};




