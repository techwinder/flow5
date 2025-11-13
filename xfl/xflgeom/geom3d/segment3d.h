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
#include <QVarLengthArray>

#include <xflgeom/geom3d/node.h>

class Segment3d
{
    public:
        Segment3d();
        Segment3d(Node const &vtx0, Node const &vtx1);

        void setNodes(Node const &vtx0, Node const &vtx1);
        void setNodes(Node const*vtx);

        Node &vertex(int iv) {return m_S[iv%2];}
        Node const &vertexAt(int iv) const {return m_S[iv%2];}
        void setVertex(int iv, Node const &nd) {m_S[iv%2]=nd;}

        Vector3d const &CoG()  const {return m_CoG;}
        Vector3d const &midPoint() const {return m_CoG;}
        void getPoint(double xrel, Vector3d &pt) const;

        bool isNull() const {return m_Length<1.e-6;}

        double const &length() const {return m_Length;}
        Vector3d averageNormal() const {return (m_S[0].normal()+m_S[1].normal()).normalized();}

        Vector3d const &segment() const {return m_Segment;}
        Vector3d oppSegment() const {return Vector3d(-m_Segment.x, -m_Segment.y, -m_Segment.z);}
        Vector3d const &unitDir() const {return m_U;}

        Segment3d reversed() const {return {vertexAt(1), vertexAt(0)};}

        void reset();

        double angle(int iv, Vector3d const &V) const;

        bool isSame(Segment3d newEdge, double precision) const;
        bool isOnSegment(Vector3d const &pt, double precision) const;
        bool isOnSegment(double x, double y, double z, double precision) const;
        bool isEncroachedBy(Vector3d const &pt) const;

        bool intersectsProjected(Segment3d const &seg, Node &I, double precision) const;

        void setNodeIndex(int ivtx, int index);
        int nodeIndex(int ivtx) const;

        QVector<Segment3d> split() const;
        QVector<Segment3d> split(double maxsize) const;

        bool isSplittable() const {return m_bSplittable;}
        void setSplittable(bool bsplittable) {m_bSplittable=bsplittable;}

        QString properties(bool bLong=false, const QString &prefix=QString()) const;

        void listVertices() const {qDebug("V0= %13g %13g %13g  V1= %13g %13g %13g", m_S[0].x, m_S[0].y, m_S[0].z, m_S[1].x, m_S[1].y, m_S[1].z);}

    protected:
        Vector3d m_Segment;
        Node m_S[2];
        Vector3d m_U;
        Vector3d m_CoG;
        double m_Length;

        bool m_bSplittable; /** if true, the segment can be split during the refinement of the PSLG */
};












