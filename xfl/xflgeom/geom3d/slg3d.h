/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once


/**
 * Defines a 3d Straight Line Graph (SLG)
 */


#include <QVector>
#include <xflgeom/geom3d/segment3d.h>

class Triangle3d;

class SLG3d : public QVector<Segment3d>
{
    public:
        SLG3d();
        SLG3d(QVector<Segment3d> const &segs);

        int cleanNullSegments();

        inline bool isVertex(Vector3d const &vtx, int &iSeg, double precision=LENGTHPRECISION) const;
        inline bool isSegment(Segment3d const &seg, int &iSeg, double precision=LENGTHPRECISION) const;
        inline int isSegment(Segment3d const &seg, double precision=LENGTHPRECISION) const;

        double boxDiag() const;
        double boxDiag(double &deltax, double &deltay, double &deltaz) const;

        bool isSplittable() const {return m_bSplittable;}

        void makeNodes(QVector<Node> &nodes) const;
        void nodesAroundCenter(const Vector3d &center, double radius, QVector<Node> &closenodes);
        void nodesInTriangle(Triangle3d const &t3d, QVector<Node> &insidenodes) const;

        void appendSegment(Segment3d const &seg);
        void appendSegment(Vector3d const &vtx0, Vector3d const &vtx1);

        void appendSegments(const QVector<Segment3d> &pslg);

        void previous(int iseg, double &theta_prev, int &iprevious);
        void next(int iseg, double &theta_next, int &inext);

        int removeSegments(Segment3d const &seg);

        bool insertNonExistent(int iseg, Segment3d const &seg);

        inline bool isContinuous(double precision) const;

        bool intersect(Segment3d const & base, QVector<int> &intersected, QVector<Vector3d> &I, double precision);

        void listPrevious(int iseg, QVector<int> &previous);
        void listNext(int iseg, QVector<int> &next);

        void list(QString &logmsg, bool bLong=false);

    private:
        bool m_bSplittable; /** If true, the PSLG can be split during the refinement of the PSLG.
                                By default a contour PSLG is not splittable to force a unique number of
                                points on shared edges.
                                Inner PSLG are spliitable, as are free edge PSLG if they can be identified. */

};




inline bool SLG3d::isVertex(Vector3d const &vtx, int &iSeg, double precision) const
{
    for(iSeg=0; iSeg<size(); iSeg++)
    {
        if(at(iSeg).vertexAt(0).isSame(vtx, precision)) return true;
        if(at(iSeg).vertexAt(1).isSame(vtx, precision)) return true;
    }
    iSeg=-1;
    return false;
}


inline bool SLG3d::isSegment(Segment3d const &seg, int &iSeg, double precision) const
{
    for(iSeg=0; iSeg<size(); iSeg++)
    {
        if(at(iSeg).isSame(seg, precision)) return true;
    }
    iSeg=-1;
    return false;
}


inline int SLG3d::isSegment(Segment3d const &seg, double precision) const
{
    for(int iSeg=0; iSeg<size(); iSeg++)
    {
        if(at(iSeg).isSame(seg, precision)) return iSeg;
    }
    return -1;
}



/** Checks that the pslg is continuous and closed */
inline bool SLG3d::isContinuous(double precision) const
{
    for(int i=0; i<size(); i++)
    {
        if(!at(i).vertexAt(1).isSame(at((i+1)%size()).vertexAt(0), precision)) return false;
    }
    return true;
}
