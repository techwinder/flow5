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


/** It's not planar-SLG, but it still is a Straight Line Graph */

#include "slg3d.h"
#include <xflgeom/geom_globals/geom_global.h>
#include <xflgeom/geom3d/triangle3d.h>
#include <xflmath/constants.h>

SLG3d::SLG3d()
{
    m_bSplittable = true;
}


SLG3d::SLG3d(const QVector<Segment3d> &segs) : QVector<Segment3d>(segs)
{
    m_bSplittable = true;
    clear();
    append(segs);
}


int SLG3d::cleanNullSegments()
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


void SLG3d::appendSegment(Segment3d const &seg)
{
    append(seg);
    last().setSplittable(m_bSplittable);
}


void SLG3d::appendSegment(Vector3d const &vtx0, Vector3d const &vtx1)
{
    append({vtx0, vtx1});
    last().setSplittable(m_bSplittable);
}


void SLG3d::appendSegments(QVector<Segment3d> const &pslg)
{
    for(int i=0; i<pslg.size(); i++)
    {
        appendSegment(pslg.at(i));
    }
}

/** @todo could be made simpler and faster if PSLG is assumed to be continuous and closed */
void SLG3d::makeNodes(QVector<Node> &nodes) const
{
    nodes.clear();
    for(ConstIterator it=begin(); it!=end(); it++)
    {
        for(int ivtx=0; ivtx<2; ivtx++)
        {
            Node const &nd = it->vertexAt(ivtx);
            bool bFound = false;
            for(QVector<Node>::ConstIterator in=nodes.constBegin(); in!=nodes.constEnd(); in++)
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
void SLG3d::nodesInTriangle(Triangle3d const &t3d, QVector<Node> &insidenodes) const
{
    Vector3d proj;
    for(int is=0; is<size(); is++)
    {
        Node const &nd = at(is).vertexAt(0);
        // discard distant nodes not only to save time, but
        // also to avoid projections from opposite side of the slg
        if(t3d.CoG_g().distanceTo(nd)>t3d.maxEdgeLength()/2.0) continue;

        if(t3d.containsPointProjection(nd, proj, 1.e-6))
        {
            if(!proj.isSame(t3d.vertexAt(0), 1.e-6) &&
               !proj.isSame(t3d.vertexAt(1), 1.e-6) &&
               !proj.isSame(t3d.vertexAt(2), 1.e-6))
                insidenodes.append(nd);
        }
    }
}


/**
 * removes all segments which are same as the input segment,
 * and returns the number of removed segments */
int SLG3d::removeSegments(Segment3d const &seg)
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



/**
 * Checks if an identical segment exists, and if not inserts it at index iseg
 * @return true if inserted, false otherwise
 */
bool SLG3d::insertNonExistent(int iseg, Segment3d const &seg)
{
    for(ConstIterator it=constBegin(); it!=constEnd(); it++)
    {
        if(it->isSame(seg, 1.e-6))
            return false;
    }
    insert(iseg, seg);
    return true;
}


/** List the segments which have their end vertex same as the input segment's start vertex */
void SLG3d::listPrevious(int iseg, QVector<int> &previous)
{
    previous.clear();

    if(iseg<0||iseg>=size()) return;

    Segment3d const &seg = at(iseg);
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
void SLG3d::listNext(int iseg, QVector<int> &next)
{
    next.clear();

    if(iseg<0||iseg>=size()) return;

    Segment3d const &seg = at(iseg);
    for(int it=0; it<size(); it++)
    {
        if(it!=iseg)
        {
            if(at(it).vertexAt(0).isSame(seg.vertexAt(1),1.e-6))
                next.append(it);
        }
    }
}


/**
 * finds the previous adjacent segment which makes the minimal angle with this segment
*/
void SLG3d::previous(int iseg, double &theta_prev, int &iprevious)
{
    QVector<int> previoussegs;
    Segment3d const &seg = at(iseg);
    listPrevious(iseg, previoussegs);
    iprevious = -1;
    theta_prev = 2*PI;
    for(int ip=0; ip<previoussegs.size(); ip++)
    {
        Segment3d const &previous = at(previoussegs.at(ip)); // don't take a reference on a changing array
        double theta = seg.angle(0, previous.unitDir()*(-1));
        if(theta<theta_prev)
        {
            theta_prev=theta;
            iprevious=previoussegs.at(ip);
        }
    }
}


/**
 * finds the next adjacent segment which makes the minimal angle with this segment
*/
void SLG3d::next(int iseg, double &theta_next, int &inext)
{
    QVector<int> nextsegs;
    // build the opposite segment from vertex 1 to vertex 0
    Segment3d seg = at(iseg).reversed();
    listNext(iseg, nextsegs);
    inext = -1;
    theta_next = 2*PI;
    for(int ip=0; ip<nextsegs.size(); ip++)
    {
        Segment3d const &next = at(nextsegs.at(ip));
        double theta = 2.0*PI-seg.angle(1, next.unitDir()); // looking for anti-trigonometric min angle
        if(theta<theta_next)
        {
            theta_next=theta;
            inext=nextsegs.at(ip);
        }
    }
}


/**
 * Checks if the input segment intersects any other segment.
 * Since both segments do not necessarily lie in the same plane, checks only that the projection
 * of the 2nd segments on the plane perpendicular to the average normal of the input segment
 * do not intersect the input segment.
 * No intersection possible if the distance of mid points is greater than the sum of the two half-lengths.
 * @param iSeg the index of the input segment
 * @param iIntersected the array of the intersected segements indexes
 * @I the array of the intersection points
 * @return true if there is an intersection
*/
bool SLG3d::intersect(Segment3d const & segment, QVector<int> &intersected, QVector<Vector3d> &I, double precision)
{
    // Make the average normal
    Vector3d k = (segment.vertexAt(0).normal()+segment.vertexAt(1).normal()).normalized();
    Vector3d i = segment.unitDir();
    Vector3d j = k*i;

    // define the distance beyond which there is no need to check for potential intersection
    double dist = 3.0*segment.length();
    Vector3d Vtx[2];
    Vector2d vtx2d[2];
    Vector2d I2d;

    // define a 2d referential using the segment's CoG, the segment's unit dir and the normal
    Segment2d seg2d({-segment.length()/2, 0}, {segment.length()/2.0, 0}); // aligned with the x-axis

    for(int is=0; is<size(); is++)
    {
        Segment3d const &frontseg = at(is);

         // not interested in the segment's self intersection
        if(segment.isSame(frontseg, precision)) continue;

        // not interested in intersection with adjacent segments
        if(frontseg.vertexAt(0).isSame(segment.vertexAt(0), precision)) continue;
        if(frontseg.vertexAt(1).isSame(segment.vertexAt(0), precision)) continue;
        if(frontseg.vertexAt(0).isSame(segment.vertexAt(1), precision)) continue;
        if(frontseg.vertexAt(1).isSame(segment.vertexAt(1), precision)) continue;

        if(segment.CoG().distanceTo(frontseg.CoG())>dist) continue; // discard distant segments

        // build the projected segment
        for(int iv=0; iv<2; iv++)
        {
            Vtx[iv] = frontseg.vertexAt(iv);
            // Substract the elevation
            Vtx[iv] -= k * (frontseg.CoG()-segment.CoG()).dot(k);

            vtx2d[iv].x = (Vtx[iv]-segment.CoG()).dot(i);
            vtx2d[iv].y = (Vtx[iv]-segment.CoG()).dot(j);
        }

        if(seg2d.intersects(vtx2d[0], vtx2d[1], I2d, true, precision))
        {
            // Convert the intersection point back to 3d;
            double tau = I2d.x; // coordinate along the 2d segment unit vector
            Vector3d I3d = segment.CoG()+ segment.segment()*tau; // mid point is the origin
            intersected.push_back(is);
            I.push_back(I3d);
        }
    }

    return intersected.size()>0;
}


void SLG3d::list(QString &logmsg, bool bLong)
{
    QString strange, prefix;
    int iseg = 0;
    prefix = "   ";
    for(ConstIterator it=constBegin(); it!=constEnd(); it++)
    {
        strange = QString::asprintf("Seg3d %d\n", iseg);
        logmsg += strange + it->properties(bLong, prefix) + "\n";
        iseg++;
    }
}


double SLG3d::boxDiag() const
{
    double dx=0, dy=0, dz=0;
    return boxDiag(dx,dy,dz);
}


double SLG3d::boxDiag(double &deltax, double &deltay, double &deltaz) const
{
    double minx= 1e10, miny= 1e10, minz=1e10;
    double maxx=-1e10, maxy=-1e10, maxz=-1e10;

    for(int iseg=0; iseg<size(); iseg++)
    {
        Segment3d const &seg3d = at(iseg);
        minx = std::min(seg3d.vertexAt(0).x, minx);
        minx = std::min(seg3d.vertexAt(1).x, minx);

        maxx = std::max(seg3d.vertexAt(0).x, maxx);
        maxx = std::max(seg3d.vertexAt(1).x, maxx);

        miny = std::min(seg3d.vertexAt(0).y, miny);
        miny = std::min(seg3d.vertexAt(1).y, miny);

        maxy = std::max(seg3d.vertexAt(0).y, maxy);
        maxy = std::max(seg3d.vertexAt(1).y, maxy);

        minz = std::min(seg3d.vertexAt(0).z, minz);
        minz = std::min(seg3d.vertexAt(1).z, minz);

        maxz = std::max(seg3d.vertexAt(0).z, maxz);
        maxz = std::max(seg3d.vertexAt(1).z, maxz);
    }
    deltax = maxx-minx;
    deltay = maxy-miny;
    deltaz = maxz-minz;
    return sqrt(deltax*deltax + deltay*deltay + deltaz*deltaz);
}


/** Assumes that the PSLG is continuous and closed, so that only the
 * first vertices of each segment need to be checked */
void SLG3d::nodesAroundCenter(Vector3d const &center, double radius, QVector<Node> &closenodes)
{
    for(int is=0; is<size(); is++)
    {
        Node const &nd = at(is).vertexAt(0);
        double dist = nd.distanceTo(center);
//        qDebug()<<"dist, raidus"<<dist<<radius;
        if(dist<radius)
            closenodes.append(nd);
    }
//    qDebug()<<"endsearch";
}














