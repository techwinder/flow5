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


/**
 * Defines a 3d Straight Line Graph (SLG)
 */


#include <api/segment3d.h>

class Triangle3d;

class SLG3d : public std::vector<Segment3d>
{
    public:
        SLG3d();
//        SLG3d(const std::vector<Segment3d> &segs);

        int cleanNullSegments();

        inline bool isVertex(Vector3d const &vtx, int &iSeg, double precision=LENGTHPRECISION) const;
        inline bool isSegment(Segment3d const &seg, int &iSeg, double precision=LENGTHPRECISION) const;
        inline int isSegment(Segment3d const &seg, double precision=LENGTHPRECISION) const;

        double boxDiag() const;
        double boxDiag(double &deltax, double &deltay, double &deltaz) const;

        bool isSplittable() const {return m_bSplittable;}

        void makeNodes(std::vector<Node> &nodes) const;
        void nodesAroundCenter(const Vector3d &center, double radius, std::vector<Node> &closenodes);
        void nodesInTriangle(Triangle3d const &t3d, std::vector<Node> &insidenodes) const;

        void appendSegment(Segment3d const &seg);
        void appendSegment(Vector3d const &vtx0, Vector3d const &vtx1);

        void appendSegments(const std::vector<Segment3d> &pslg);

        void previous(int iseg, double &theta_prev, int &iprevious);
        void next(int iseg, double &theta_next, int &inext);

        int removeSegments(Segment3d const &seg);


        void append(SLG3d const &slg);

        inline bool isContinuous(double precision) const;

        bool intersect(Segment3d const & base, std::vector<int> &intersected, std::vector<Vector3d> &I, double precision);

        void insertAt(unsigned int index);
        void insertAt(unsigned int index, Segment3d const &seg);
        void removeAt(unsigned int index);
        bool isEmpty() const {return size()==0;}


        void listPrevious(int iseg, std::vector<int> &previous);
        void listNext(int iseg, std::vector<int> &next);

        void list(std::string &logmsg, bool bLong=false);

    private:
        bool m_bSplittable; /** If true, the PSLG can be split during the refinement of the PSLG.
                                By default a contour PSLG is not splittable to force a unique number of
                                points on shared edges.
                                Inner PSLG are spliitable, as are free edge PSLG if they can be identified. */

};




inline bool SLG3d::isVertex(Vector3d const &vtx, int &iSeg, double precision) const
{
    for(iSeg=0; iSeg<int(size()); iSeg++)
    {
        if(at(iSeg).vertexAt(0).isSame(vtx, precision)) return true;
        if(at(iSeg).vertexAt(1).isSame(vtx, precision)) return true;
    }
    iSeg=-1;
    return false;
}


inline bool SLG3d::isSegment(Segment3d const &seg, int &iSeg, double precision) const
{
    for(iSeg=0; iSeg<int(size()); iSeg++)
    {
        if(at(iSeg).isSame(seg, precision)) return true;
    }
    iSeg=-1;
    return false;
}


inline int SLG3d::isSegment(Segment3d const &seg, double precision) const
{
    for(int iSeg=0; iSeg<int(size()); iSeg++)
    {
        if(at(iSeg).isSame(seg, precision)) return iSeg;
    }
    return -1;
}



/** Checks that the pslg is continuous and closed */
inline bool SLG3d::isContinuous(double precision) const
{
    for(int i=0; i<int(size()); i++)
    {
        if(!at(i).vertexAt(1).isSame(at((i+1)%size()).vertexAt(0), precision)) return false;
    }
    return true;
}
