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
#include <xflgeom/geom2d/segment2d.h>

class PSLG2d : public QVector<Segment2d>
{
    public:
        PSLG2d();
        PSLG2d(QVector<Segment2d> const &segs);

        int cleanNullSegments();

        bool isIntersected(Segment2d const &seg, int &iSeg, Vector2d &I) const;
        bool isVertex(Vector2d const &vtx, int &iSeg, double precision=LENGTHPRECISION) const;
        bool isSegment(Segment2d const &seg, int &iSeg, double precision=LENGTHPRECISION) const;
        bool contains(Vector2d const &pt) const;

        double boxDiag(double &deltax, double &deltay) const;

        void stretch(double uscale, double vscale);

        bool isSplittable() const {return m_bSplittable;}
        void setSplittable(bool bsplittable);

        void appendSegment(Segment2d const &seg);
        void appendSegment(Vector2d const &vtx0, Vector2d const &vtx1);

        void appendSegments(const QVector<Segment2d> &pslg);

        bool sortPSLG(int iStartSeg, PSLG2d &newsegs);

        Segment2d const & previous(int iseg) const;
        void previous(int iseg, double &theta_prev, int &iprevious);
        Segment2d const & next(int iseg) const;
        void next(int iseg, double &theta_next, int &inext);

        void removePrevious(int iseg);
        void removeNext(int iseg);

        void reverse();

        double longestSegLength() const;
        double averageSegLength() const;

        void makeNodes(QVector<Node2d> &nodes) const;
        int removeSegments(Segment2d const &seg);
        void nodesAroundCenter(Vector2d const &center, double radius, QVector<Node2d> &closenodes);
        void nodesInTriangle(Triangle2d const &t2d, QVector<Node2d> &insidenodes) const;
        bool intersect(Segment2d const & segment, QVector<int> &intersected, QVector<Vector2d> &I, bool bPointsInside, double precision);

        void listPrevious(int iseg, QVector<int> &previous);
        void listNext(int iseg, QVector<int> &next);

        void listPSLG(bool bLong, QString &list);
        void listIntersected(Vector2d const &pt0, Vector2d const &pt1, QVector<int> &intersectedlist);


    private:
        bool m_bSplittable; /** If true, the PSLG can be split during the refinement of the PSLG.
                                By default a contour PSLG is not splittable to force a unique number of
                                points on shared edges.
                                Inner PSLG are spliitable, as are free edge PSLG if they can be identified. */

};

