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

#include <string>
#include <vector>
#include <algorithm>

#include <geom_enums.h>
#include <vector3d.h>

/**
 * A Node is a vector3d, with a normal and a list of triangles of which this node is a vertex
 */
class FL5LIB_EXPORT Node : public Vector3d
{
    private:
        std::vector<int> m_TriangleIndex;         /** the array of indexes of panels of which this node is a vertex */
        std::vector<int> m_NeighbourIndex;        /** the array of neighbour node indexes */
        Vector3d m_Normal;                        /** the node's normal */
        int m_Index;                              /** the index of the node in the global node array */
        bool m_bTrailing;                         /** true if the node is trailing, false otherwise */
        xfl::enumSurfacePosition m_Position;      /** defines on which type of surface the node is positioned */
        int m_SurfaceIndex;                       /** defines to which surface the node belongs */

    public:
        Node() :
            m_Index{-1},  m_bTrailing{false}, m_Position{xfl::NOSURFACE}, m_SurfaceIndex{-1}
        {
        }

        Node(double xx, double yy, double zz) :
            Vector3d(xx,yy,zz), m_Index{-1},  m_bTrailing{false}, m_Position{xfl::NOSURFACE}, m_SurfaceIndex{-1}
        {
        }

        Node(Vector3d const &v3d) :
            Vector3d(v3d), m_Index{-1},  m_bTrailing{false}, m_Position{xfl::NOSURFACE}, m_SurfaceIndex{-1}

        {
        }

        Node(double xx, double yy, double zz, Vector3d const &N, int idx=-1, xfl::enumSurfacePosition pos=xfl::NOSURFACE) :
            Vector3d(xx,yy,zz), m_Normal{N}, m_Index{idx}, m_bTrailing{false}, m_Position{pos}, m_SurfaceIndex{-1}
        {

        }

        Node(Vector3d const &v3d, Vector3d const &N, int idx=-1, xfl::enumSurfacePosition pos=xfl::NOSURFACE) :
            Vector3d(v3d), m_Normal{N}, m_Index{idx}, m_bTrailing{false}, m_Position{pos}, m_SurfaceIndex{-1}

        {
        }

        virtual ~Node() = default;

        void setPosition(double xp, double yp, double zp)
        {
            x = xp;
            y = yp;
            z = zp;
        }

        void setPosition(Vector3d const &I)
        {
            x = I.x;
            y = I.y;
            z = I.z;
        }

        void setPosition(Node const &nd)
        {
            x = nd.x;
            y = nd.y;
            z = nd.z;
        }

        void setNode(Node const &node)
        {
            x = node.x;
            y = node.y;
            z = node.z;
            m_Index = node.index();
            m_Position = node.m_Position;

            m_Normal.copy(node.normal());
            m_TriangleIndex = node.m_TriangleIndex;
            m_NeighbourIndex = node.m_NeighbourIndex;

        }

        void setIndex(int index) {m_Index=index;}
        int index() const {return m_Index;}

        bool isWingNode() const {return m_Position==xfl::TOPSURFACE || m_Position==xfl::BOTSURFACE || m_Position==xfl::MIDSURFACE || m_Position==xfl::SIDESURFACE;}
        bool isFuseNode() const {return m_Position==xfl::FUSESURFACE;}

        Vector3d const &normal() const {return m_Normal;}
        Vector3d &normal() {return m_Normal;}

        void setNormal(Vector3d const &normal) {m_Normal.x=normal.x, m_Normal.y=normal.y, m_Normal.z=normal.z;}
        void setNormal(double nx, double ny, double nz) {m_Normal.x=nx; m_Normal.y=ny, m_Normal.z=nz;}

        void flipNormal() {m_Normal.reverse();}


        // neighbour nodes
        void setTrailing(bool bTrailing) {m_bTrailing=bTrailing;}
        bool isTrailing() const {return m_bTrailing;}
        void clearNeighbourNodes() {m_NeighbourIndex.clear();}
        void addNeighbourIndex(int index);
        void resizeNodeNeighbours(int nd) {m_NeighbourIndex.resize(nd);}
        int neighbourNodeCount() const {return int(m_NeighbourIndex.size());}
        int neigbourNodeIndex(int in) const {return m_NeighbourIndex.at(in);}
        int nodeNeighbourIndex(int in) const {return m_NeighbourIndex.at(in);}
        void setNodeNeighbourIndex(int in, int idx){m_NeighbourIndex[in]=idx;}
        std::vector<int>const &nodeNeighbourIndexes() const {return m_NeighbourIndex;}
        void setNeighbourNodeIndexes(std::vector<int> const&indexes) {m_NeighbourIndex=indexes;}

        // neighbour triangles
        void clearTriangles() {m_TriangleIndex.clear();}
        void addTriangleIndex(int index);
        void resizeTriangles(int nt) {m_TriangleIndex.resize(nt);}
        int triangleCount() const {return int(m_TriangleIndex.size());}
        int triangleIndex(int it) const {return m_TriangleIndex.at(it);}
        void setTriangleIndex(int it, int idx) {m_TriangleIndex[it]=idx;}
        std::vector<int>const &triangleIndexes() const {return m_TriangleIndex;}
        void setTriangleIndexes(std::vector<int> const&indexes) {m_TriangleIndex=indexes;}
        bool hasTriangleIndex(int idx) const {return std::find(m_TriangleIndex.begin(), m_TriangleIndex.end(), idx) != m_TriangleIndex.end();}


        inline void rotate(Vector3d const &R, double Angle) override;
        inline void rotate(const Vector3d &O, Vector3d const &R, double Angle) override;
        inline void rotateX(Vector3d const &O, double rx) override;
        inline void rotateY(Vector3d const &O, double ry) override;
        inline void rotateZ(Vector3d const &O, double rz) override;


        std::string properties() const;

        xfl::enumSurfacePosition surfacePosition() const {return m_Position;}
        void setSurfacePosition(xfl::enumSurfacePosition pos) {m_Position=pos;}

        int surfaceIndex() const {return m_SurfaceIndex;}
        void setSurfaceIndex(int idx) {m_SurfaceIndex=idx;}

        // hide base operators
        Node operator +(Node const &V) const
        {
            Vector3d N = (m_Normal+V.normal()).normalized();
            return Node(x+V.x, y+V.y, z+V.z, N);
        }

        Node operator -(Node const &V) const
        {
            Vector3d N = (m_Normal+V.normal()).normalized();
            return Node(x-V.x, y-V.y, z-V.z, N);
        }

        Node operator *(double const &d) const
        {
            return Node(x*d, y*d, z*d, m_Normal);
        }

        Node operator *(Node const &T) const
        {
            Node C;
            C.x =  y*T.z - z*T.y;
            C.y = -x*T.z + z*T.x;
            C.z =  x*T.y - y*T.x;
            C.m_Normal = m_Normal*T.normal();
            return C;
        }


        Node operator  /(double const &d) const
        {
            Node T(x/d, y/d, z/d, m_Normal);
            return T;
        }
};


inline void Node::rotate(Vector3d const &R, double Angle)
{
    Vector3d::rotate(R,Angle);
    m_Normal.rotate(R,Angle);
}


inline void Node::rotate(const Vector3d &O, Vector3d const &R, double Angle)
{
    Vector3d::rotate(O,R,Angle);
    m_Normal.rotate(R,Angle);
}


inline void Node::rotateX(Vector3d const &O, double rx)
{
    Vector3d::rotate(O, Vector3d(1,0,0), rx);
    m_Normal.rotate(Vector3d(1,0,0),rx);

}


inline void Node::rotateY(Vector3d const &O, double ry)
{
    Vector3d::rotate(O, Vector3d(0,1,0), ry);
    m_Normal.rotate(Vector3d(0,1,0),ry);
}


inline void Node::rotateZ(Vector3d const &O, double rz)
{
    Vector3d::rotate(O, Vector3d(0,0,1), rz);
    m_Normal.rotate(Vector3d(0,0,1),rz);
}

