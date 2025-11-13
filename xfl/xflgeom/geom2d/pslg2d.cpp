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



#include "pslg2d.h"
#include <xflgeom/geom_globals/geom_global.h>
#include <xflgeom/geom2d/triangle2d.h>
#include <xflmath/constants.h>

PSLG2d::PSLG2d() : QVector<Segment2d>()
{
    m_bSplittable = true;
}


PSLG2d::PSLG2d(const QVector<Segment2d> &segs) : QVector<Segment2d>(segs)
{
    m_bSplittable = true;
    clear();
    append(segs);
}


bool PSLG2d::isVertex(Vector2d const &vtx, int &iSeg, double precision) const
{
    for(iSeg=0; iSeg<size(); iSeg++)
    {
        if(at(iSeg).vertexAt(0).isSame(vtx, precision)) return true;
        if(at(iSeg).vertexAt(1).isSame(vtx, precision)) return true;
    }
    iSeg=-1;
    return false;
}


bool PSLG2d::isSegment(Segment2d const &seg, int &iSeg, double precision) const
{
    for(iSeg=0; iSeg<size(); iSeg++)
    {
        if(at(iSeg).isSame(seg, precision)) return true;
    }
    iSeg=-1;
    return false;
}


bool PSLG2d::isIntersected(Segment2d const &seg, int &iSeg, Vector2d &I) const
{
    for(iSeg=0; iSeg<size(); iSeg++)
    {
        if(at(iSeg).intersects(seg, I, false, 1.e-5)) return true;
    }
    iSeg=-1;
    return false;
}


/**
 * Tests if a point is inside the polygon defined by the PSLG.
 * Assumes implicitely that the PSLG is a closed polygon
 * ----- Points on the PSLG are IN -----
 * The PSLG should be closed, but the segments need not be in connected order
 * Counts the number of intersections of a ray originating from the point
 * in a random direction.
 *   if even number of intersection --> is outside
 *   if odd                         --> is inside
 */
bool PSLG2d::contains(Vector2d const &pt) const
{
    Vector2d I2d;
     Vector2d dir(cos(3.0*PI/7), sin(3.0*PI/7)); // random direction
//   Vector2d dir(1,0);

    int nIntersections = 0;

    // if pt is on a pslg segment, return true
    for(int i=0; i<count(); i++)
    {
        if(at(i).isOnSegment(pt))
            return true;
    }

    // else check for intersection
    for(int i=0; i<count(); i++)
    {
        if(intersectRay(at(i).vertexAt(0), at(i).vertexAt(1), pt, dir, I2d ))
            nIntersections++;
    }

    if(nIntersections%2==0)    return false;
    else                    return true;
}


double PSLG2d::boxDiag(double &deltax, double &deltay) const
{
    double minx =  1e10, miny =  1e10;
    double maxx = -1e10, maxy = -1e10;
    for(int iseg=0; iseg<size(); iseg++)
    {
        Segment2d const &seg2d = at(iseg);
        minx = std::min(seg2d.vertexAt(0).x, minx);
        minx = std::min(seg2d.vertexAt(1).x, minx);

        maxx = std::max(seg2d.vertexAt(0).x, maxx);
        maxx = std::max(seg2d.vertexAt(1).x, maxx);

        miny = std::min(seg2d.vertexAt(0).y, miny);
        miny = std::min(seg2d.vertexAt(1).y, miny);

        maxy = std::max(seg2d.vertexAt(0).y, maxy);
        maxy = std::max(seg2d.vertexAt(1).y, maxy);
    }
    deltax = maxx-minx;
    deltay = maxy-miny;
    return sqrt(deltax*deltax + deltay*deltay);
}



void PSLG2d::listIntersected(Vector2d const &pt0, Vector2d const &pt1, QVector<int> &intersectedlist)
{
    intersectedlist.clear();
    Vector2d I;
    for(int ip=0; ip<size(); ip++)
    {
        if(at(ip).intersects(pt0, pt1, I, false, 1.e-5))
            intersectedlist.append(ip);
    }
}


int PSLG2d::cleanNullSegments()
{
    int removed = 0;
    for(int is=size()-1; is>=0; is--)
    {
        if(at(is).length()<LENGTHPRECISION)
        {
            removeAt(is);
            removed++;
        }
    }
    return removed;
}


void PSLG2d::setSplittable(bool bsplittable)
{
    m_bSplittable = bsplittable;

    QMutableVectorIterator<Segment2d> i(*this);
    while (i.hasNext())
        i.next().setSplittable(bsplittable);
}


void PSLG2d::appendSegment(Segment2d const &seg)
{
    append(seg);
    last().setSplittable(m_bSplittable);
}


void PSLG2d::appendSegment(Vector2d const &vtx0, Vector2d const &vtx1)
{
    append({vtx0, vtx1});
    last().setSplittable(m_bSplittable);
}


void PSLG2d::appendSegments(QVector<Segment2d> const &pslg)
{
    for(int i=0; i<pslg.size(); i++)
    {
        appendSegment(pslg.at(i));
    }
}


void PSLG2d::stretch(double uscale, double vscale)
{
    for(iterator it=begin(); it!=end(); ++it)
    {
        Segment2d &seg = *it;
        Vector2d s0 = seg.vertexAt(0);
        s0.x *=uscale;
        s0.y *=vscale;
        Vector2d s1 = seg.vertexAt(1);
        s1.x *=uscale;
        s1.y *=vscale;

        *it = {s0, s1};
    }
}


void PSLG2d::listPSLG(bool bLong, QString &list)
{

    QString strange, prefix;
    int iseg = 0;
    prefix = "   ";
    for(ConstIterator it=constBegin(); it!=constEnd(); it++)
    {
        strange = QString::asprintf("Seg2d %d\n", iseg);
        list += strange + it->properties(bLong, prefix);
        iseg++;
    }
}


Segment2d const & PSLG2d::previous(int iseg) const
{
    int previous=iseg-1;
    if(previous<0) previous = size()-1;

    return at(previous);
}


void PSLG2d::previous(int iseg, double &theta_prev, int &iprevious)
{
    QVector<int> previoussegs;
    Segment2d const &seg = at(iseg);
    listPrevious(iseg, previoussegs);
    iprevious = -1;
    theta_prev = 2*PI;
    for(int ip=0; ip<previoussegs.size(); ip++)
    {
        Segment2d const &previous = at(previoussegs.at(ip)); // don't take a reference on a changing array
        double theta = seg.angle(0, previous.unitDir()*(-1));
        if(theta<theta_prev)
        {
            theta_prev=theta;
            iprevious=previoussegs.at(ip);
        }
    }
}


void PSLG2d::next(int iseg, double &theta_next, int &inext)
{
    QVector<int> nextsegs;
    // build the opposite segment from vertex 1 to vertex 0
    Segment2d seg = at(iseg).reversed();
    listNext(iseg, nextsegs);
    inext = -1;
    theta_next = 2*PI;
    for(int ip=0; ip<nextsegs.size(); ip++)
    {
        Segment2d const &next = at(nextsegs.at(ip));
        double theta = 2.0*PI-seg.angle(1, next.unitDir()); // looking for anti-trigonometric min angle
        if(theta<theta_next)
        {
            theta_next=theta;
            inext=nextsegs.at(ip);
        }
    }
}


Segment2d const & PSLG2d::next(int iseg) const
{
    int next = iseg+1;
    if(next>=size()) next=0;
    return at(next);
}


void PSLG2d::removePrevious(int iseg)
{
    int previous=iseg-1;
    if(previous<0) previous = size()-1;

    if(previous>=0 && previous<size())
        removeAt(previous);
}


void PSLG2d::removeNext(int iseg)
{
    int next = iseg+1;
    if(next>=size()) next=0;

    if(next>=0 && next<size())
        removeAt(next);
}


/** Reverses the order of the segments, and swaps the vertices of each segments */
void PSLG2d::reverse()
{
    int size0 = size();
    for(int iLast = size()-1; iLast>=0; iLast--)
    {
        Segment2d const &seg = at(iLast);
        appendSegment({seg.vertexAt(1), seg.vertexAt(0)});
        removeAt(iLast);
    }
    Q_ASSERT(size0==size());


}


int PSLG2d::removeSegments(Segment2d const &seg)
{
    int nremoved=0;
    for(int it=size()-1; it>=0; it--)
    {

        if(at(it).isSame(seg, 1.e-6))
        {
            removeAt(it);
            nremoved++;
        }
    }
    return nremoved;
}


/** @todo could be made simpler and faster if PSLG is assumed to be continuous and closed */
void PSLG2d::makeNodes(QVector<Node2d> &nodes) const
{
    nodes.clear();
    for(ConstIterator it=begin(); it!=end(); it++)
    {
        for(int ivtx=0; ivtx<2; ivtx++)
        {
            Node2d const &nd = it->vertexAt(ivtx);
            bool bFound = false;
            for(QVector<Node2d>::ConstIterator in=nodes.constBegin(); in!=nodes.constEnd(); in++)
            {
                if(in->isSame(nd))
                {
                    bFound=true;
                    break;
                }
            }
            if(!bFound) nodes.append(nd);
        }
    }
}


/**
 * Makes an array of the nodes with normal projection inside the triangle
 * Rules out the vertices of the triangle
 */
void PSLG2d::nodesInTriangle(Triangle2d const &t2d, QVector<Node2d> &insidenodes) const
{
    Vector2d proj;
    for(int is=0; is<size(); is++)
    {
        Node2d const &nd = at(is).vertexAt(0);
        // discard distant nodes not only to save time, but
        // also to avoid projections from opposite side of the slg
        if(t2d.CoG().distanceTo(nd)>t2d.maxEdgeLength()/2.0) continue;

        if(t2d.contains(nd, false))
        {
            if(!proj.isSame(t2d.vertexAt(0), 1.e-6) &&
               !proj.isSame(t2d.vertexAt(1), 1.e-6) &&
               !proj.isSame(t2d.vertexAt(2), 1.e-6))
                insidenodes.append(nd);
        }
    }
}


/** Assumes that the PSLG is continuous and closed, so that only the
 * first vertices of each segment need to be checked */
void PSLG2d::nodesAroundCenter(Vector2d const &center, double radius, QVector<Node2d> &closenodes)
{
    for(int is=0; is<size(); is++)
    {
        Node2d const &nd = at(is).vertexAt(0);
        double dist = nd.distanceTo(center);
//        qDebug()<<"dist, raidus"<<dist<<radius;
        if(dist<radius)
            closenodes.append(nd);
    }
//    qDebug()<<"endsearch";
}


/**
 * Checks if the input segment intersects any other segment.
 * No intersection possible if the distance of mid points is greater than the sum of the two half-lengths.
 * @param iSeg the index of the input segment
 * @param iIntersected the array of the intersected segements indexes
 * @I the array of the intersection points
 * @return true if there is an intersection
*/
bool PSLG2d::intersect(Segment2d const & segment, QVector<int> &intersected, QVector<Vector2d> &I, bool bPointsInside, double precision)
{
    Vector2d Ipt;
    for(int iseg=0; iseg<size(); iseg++)
    {
        Segment2d const&s2d = at(iseg);
        if(s2d.intersects(segment, Ipt, bPointsInside, precision))
        {
            intersected.push_back(iseg);
            I.push_back(Ipt);
        }
    }
    return intersected.size()>0;
}


/** List the segments which have their end vertex same as the input segment's start vertex */
void PSLG2d::listPrevious(int iseg, QVector<int> &previous)
{
    previous.clear();

    if(iseg<0||iseg>=size()) return;

    Segment2d const &seg = at(iseg);
    for(int it=0; it<size(); it++)
    {
        if(it!=iseg)
        {
            if(at(it).vertexAt(1).isSame(seg.vertexAt(0),1.e-6))
                previous.append(it);
        }
    }
}


/** List the segments which have their start vertex same as the input segment's last vertex */
void PSLG2d::listNext(int iseg, QVector<int> &next)
{
    next.clear();

    if(iseg<0||iseg>=size()) return;

    Segment2d const &seg = at(iseg);
    for(int it=0; it<size(); it++)
    {
        if(it!=iseg)
        {
            if(at(it).vertexAt(0).isSame(seg.vertexAt(1),1.e-6))
                next.append(it);
        }
    }
}


bool PSLG2d::sortPSLG(int iStartSeg, PSLG2d &newsegs)
{
    if(iStartSeg<0 || iStartSeg>=size()) return false;

    Segment2d lastseg = at(iStartSeg);
    newsegs.append(lastseg);
    while(newsegs.size()<size())
    {
        bool bFound(false);
        for(int iseg=0; iseg<size(); iseg++)
        {
            Segment2d const&seg = at(iseg);
            if(seg.vertexAt(0).index() == lastseg.vertexAt(1).index())
            {
                newsegs.append(seg);
                lastseg = seg;
                bFound = true;
                break;
            }
        }
        if(!bFound) return false;
    }

    return true;
}


double PSLG2d::longestSegLength() const
{
    if(isEmpty()) return 0.0;
    double maxlength = 0.0;
    for(int is=0; is<size(); is++)
    {
        maxlength = std::max(maxlength, at(is).length());
    }
    return maxlength;
}


double PSLG2d::averageSegLength() const
{
    if(isEmpty()) return 0.0;

    int averagelength = 0;
    for(int is=0; is<size(); is++)
    {
        averagelength += at(is).length();
    }
    return averagelength/double(size());
}
