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

#include <QVector>


#include <xflgeom/geom2d/vector2d.h>


class Node2d : public Vector2d
{
    public:
        int m_Index = -1;         /** the index of the node in the array of foil nodes; useful when nodes are re-ordered in streamwise direction for BL calculations */
        bool m_bIsWakeNode = false;
        bool m_bIsFoilNode = false;

        Vector2d m_N;

        QVector<int> m_Triangles; /**< triangles of which this node is a vertex */
        QVector<int> m_Neigh;     /**< node neighbours */

    public:        
        Node2d() : Vector2d(), m_Index{-1}, m_bIsFoilNode{false}
        {
        }

        Node2d(double const &xi, double const &yi) : Vector2d(xi,yi), m_Index{-1}, m_bIsFoilNode{false}
        {
        }

        Node2d(Vector2d const&v2d) : Vector2d(v2d), m_Index{-1}, m_bIsFoilNode{false}
        {
        }


        void setNode(Vector2d const &V, Vector2d const &normal)
        {
            x = V.x;
            y = V.y;
            m_N = normal;
        }
        void setNode(Node2d const &node);

        int index() const {return m_Index;}
        void setIndex(int idx) {m_Index=idx;}

        bool isAirfoilNode() const {return !m_bIsWakeNode;}
        void setAirfoilNode(bool bAirfoilNode) {m_bIsWakeNode = !bAirfoilNode;}

        bool isWakeNode() const {return m_bIsWakeNode;}
        void setWakeNode(bool bWakeNode) {m_bIsWakeNode = bWakeNode;}

        Vector2d const T() const {return Vector2d(m_N.y, -m_N.x);}
        void setT(Vector2d const &tangent) {m_N.set(tangent.y, tangent.x);}
        void setT(double x, double y) {m_N.set(y, x);}

        Vector2d const &normal() const {return m_N;}
        void setNormal(Vector2d const &normal) {m_N=normal;}
        void setNormal(double nx, double ny) {m_N={nx,ny};}

        void flipNormal() {m_N.x=-m_N.x; m_N.y=-m_N.y;}

        void printNode() const
        {
            qDebug(" %11g   %11g   index=%3d   wakenode=%d", x, y, index(), isWakeNode());
        }

        void rotateZ(Vector2d const &O, double beta) override;
        void rotateZ(double beta) override;

        void clearNodeNeighbours(){m_Neigh.clear();}
        void clearTriangleNeighbours(){m_Triangles.clear();}
        void addNodeNeighbour(int idx) {if(!m_Neigh.contains(idx) && idx!=m_Index) m_Neigh.append(idx);}
        void addTriangleNeighbour(int idx) {if(!m_Triangles.contains(idx)) m_Triangles.append(idx);}

        int nNodes() const {return m_Neigh.size();}
        int nodeIndex(int k) const  {return m_Neigh.at(k);}
        int nTriangles() const {return m_Triangles.size();}
        int triangleIndex(int k) const {return m_Triangles.at(k);}
        int firstTriangle() const {return m_Triangles.first();}
        int lastTriangle() const {return m_Triangles.last();}

        void setNodeNeighbours(QVector<int> const &newnodes) {m_Neigh = newnodes;}
        void setTriangleNeigbours(QVector<int> const &newtriangles) {m_Triangles = newtriangles;}
};

