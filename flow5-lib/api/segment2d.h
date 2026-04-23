/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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
#include <string>

#include <geom_params.h>
#include <node2d.h>

class Triangle2d;

class FL5LIB_EXPORT Segment2d
{
    public:
        Segment2d();
        Segment2d(Node2d const &vtx0, Node2d const&vtx1);
        Segment2d(double x0, double y0, double x1, double y1);

        void setNodes(Node2d const&nd0, Node2d const&nd1) {m_Vtx0=nd0; m_Vtx1=nd1;}
        void setNode(int ivtx, Node2d const &nd) {if(ivtx==0) m_Vtx0=nd; else if(ivtx==1) m_Vtx1=nd;}
        void setVertices(Vector2d const&vtx0, Vector2d const&vtx1) {m_Vtx0=vtx0; m_Vtx1=vtx1;}
        void setVertices(Vector2d *vtx) {m_Vtx0=vtx[0]; m_Vtx1=vtx[1];}
        void setVertex(int ivtx, Vector2d pt) {if(ivtx==0) m_Vtx0=pt; else if(ivtx==1) m_Vtx1=pt;}

        Node2d const &vertexAt(int i) const {return i==0 ? m_Vtx0 : m_Vtx1;}
        Node2d &vertex(int i) {return i==0 ? m_Vtx0 : m_Vtx1;}

        Vector2d midPoint()    const {return (m_Vtx0+m_Vtx1)/2.0;}
        Vector2d unitDir()     const {return (m_Vtx1-m_Vtx0).normalized();}
        Vector2d segment()     const {return (m_Vtx1-m_Vtx0);}
        double length()        const {return (m_Vtx1-m_Vtx0).norm();}

        bool isSame(Segment2d otheredge, double precision = LENGTHPRECISION) const;

        bool isEncroachedBy(Vector2d const &pt) const;
        bool isEncroachedBy(Triangle2d const &t2d) const;

        std::vector<Segment2d> split() const;

        bool intersects(const Vector2d &Pt0, const Vector2d &Pt1, Vector2d &IPt, bool bPointsInside, double precision) const;
        bool intersects(const Segment2d &seg, Vector2d &IPt, bool bPointsInside, double precision) const;

        bool isOnSegment(const Vector2d &pt, double precision=LENGTHPRECISION) const;
        bool isOnSegment(double x, double y, double precision=LENGTHPRECISION) const;

        bool isEndPoint(Vector2d const&pt, double precision=LENGTHPRECISION) const;

        bool isSplittable() const {return m_bSplittable;}
        void setSplittable(bool bsplittable) {m_bSplittable=bsplittable;}

        double angle(int iv, Vector2d const &V) const;
        Segment2d reversed() const {return {vertexAt(1), vertexAt(0)};}

        Vector2d const&CoG() const{return m_CoG;}

        void setIndex(int idx) {m_Index=idx;}
        int index() const {return m_Index;}

        std::string properties(bool bLong=true, std::string prefx="") const;

    private:
        int m_Index;
        Node2d m_Vtx0, m_Vtx1;
        Vector2d m_U;
        Vector2d m_CoG;
        double m_Length;
        bool m_bSplittable; /** if true, the segment can be split during the refinement of the PSLG */
};












