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

#include <QString>


#include <segment3d.h>

#include <constants.h>
#include <segment2d.h>
#include <vector2d.h>

Segment3d::Segment3d()
{
    // initialize with something non-zero
    setNodes(Vector3d(0.0,0.0,0.0), Vector3d(1.0,0.0,0.0));
    m_bSplittable = false;
}


Segment3d::Segment3d(Node const &vtx0, Node const &vtx1)
{
    setNodes(vtx0, vtx1);
    m_bSplittable = false;
}


void Segment3d::setNodes(const Node *vtx)
{
    setNodes(vtx[0],vtx[1]);
}


void Segment3d::setNodes(Node const &vtx0, Node const &vtx1)
{
    m_S[0].setNode(vtx0);
    m_S[1].setNode(vtx1);

    m_CoG.x = (vtx0.x+vtx1.x)/2.0;
    m_CoG.y = (vtx0.y+vtx1.y)/2.0;
    m_CoG.z = (vtx0.z+vtx1.z)/2.0;

    m_Segment.x = vtx1.x-vtx0.x;
    m_Segment.y = vtx1.y-vtx0.y;
    m_Segment.z = vtx1.z-vtx0.z;

    m_Length = m_Segment.norm();

    if(m_Length>1.e-6)
    {
        m_U.x = m_Segment.x/m_Length;
        m_U.y = m_Segment.y/m_Length;
        m_U.z = m_Segment.z/m_Length;
    }
    else
        m_U.set(0,0,0);
}


void Segment3d::reset()
{
    m_S[1].setNode(m_S[0]);
    m_CoG = m_S[0];
    m_Segment.reset();
    m_Length = 0.0;
    m_U.set(0,0,0);
}


bool Segment3d::isOnSegment(Vector3d const &pt, double precision) const
{
    return isOnSegment(pt.x, pt.y, pt.z, precision);
}


bool Segment3d::isOnSegment(double x, double y, double z, double precision) const
{
    double dxs = m_S[1].x-m_S[0].x;
    double dys = m_S[1].y-m_S[0].y;
    double dzs = m_S[1].z-m_S[0].z;
    double dx0 = x-m_S[0].x;
    double dy0 = y-m_S[0].y;
    double dz0 = z-m_S[0].z;
    double dx1 = x-m_S[1].x;
    double dy1 = y-m_S[1].y;
    double dz1 = z-m_S[1].z;

    // return false if the distance of Pt(x,y) to the line is greater than precision
    double l = length();
    double proj = (dxs*dx0 + dys*dy0 + dzs*dz0)/l;
    double rx0 = dxs/l * proj;
    double ry0 = dys/l * proj;
    double rz0 = dzs/l * proj;
    double hx = dx0-rx0;
    double hy = dy0-ry0;
    double hz = dz0-rz0;

    double d = sqrt(hx*hx+hy*hy+hz*hz);
    if(d>precision) return false;

    //return false if the point is on the line but outside the segment
    if(dx0*dxs + dy0*dys + dz0*dzs<0.0) return false;
    if(dx1*dxs + dy1*dys + dz1*dzs>0.0) return false;

    // on the line and within the segment
    return true;
}


bool Segment3d::isEncroachedBy(const Vector3d &pt) const
{
    if(m_S[0].isSame(pt) || m_S[1].isSame(pt)) return false;

    Vector3d MidPoint = (m_S[0]+m_S[1])/2.0;
    double d2 = (pt.x-MidPoint.x)*(pt.x-MidPoint.x) + (pt.y-MidPoint.y)*(pt.y-MidPoint.y);
    Vector3d R = (m_S[1]-m_S[0])/2.0;
    double r2 = R.x*R.x + R.y*R.y;
    return d2<=r2;
}


/** returns a list made of the two half segments */
std::vector<Segment3d> Segment3d::split() const
{
    std::vector<Segment3d> splitList;
    Vector3d midPoint = (m_S[0]+m_S[1])/2.0;
    splitList.push_back({m_S[0], midPoint});
    splitList.push_back({midPoint, m_S[1]});
    return splitList;
}


std::vector<Segment3d> Segment3d::split(double maxsize) const
{
    std::vector<Segment3d> seglist;
    int nSegs = int(m_Length/maxsize);
    if(nSegs==0) nSegs=1;

    Vector3d U = (m_S[1]-m_S[0])/double(nSegs);
    Vector3d vtx0, vtx1;
    vtx0 = m_S[0];
    for(int i=0; i<nSegs; i++)
    {
        vtx1 = m_S[0] + U*double(i+1);
        seglist.push_back({vtx0, vtx1});
        vtx0 = vtx1;
    }

    return seglist;
}


std::string Segment3d::properties(bool bLong, std::string pref) const
{
    QString props, strong;
    QString prefix = QString::fromStdString(pref);
    strong = QString::asprintf("v0  = (%11.7f, %11.7f, %11.7f)\n",  m_S[0].x, m_S[0].y, m_S[0].z);
    props = prefix.append(strong);
    strong = QString::asprintf("v1  = (%11.7f, %11.7f, %11.7f)",    m_S[1].x, m_S[1].y, m_S[1].z);
    props += prefix.append(strong);

    if(!bLong) return props.toStdString();

    props += "\n";

    strong = QString::asprintf("CoG = ( %9g, %9g, %9g )\n", m_CoG.x, m_CoG.y, m_CoG.z);
    props += prefix.append(strong);

    strong = QString::asprintf("U   = ( %9g, %9g, %9g )\n", m_U.x, m_U.y, m_U.z);
    props += prefix + strong;

    strong = QString::asprintf("Length = %9g", m_Length);
    props += prefix + strong;

    return props.toStdString();
}


/** Returns the angle in the range [pi, 2 pi] made by the vector [iv, (iv+1)%2]
 * with another 3d vector V.
 * Positive orientation is given by the normal vector N at node iv.
 * Both segments are assumede to lie in the plane normal to N
*/
double Segment3d::angle(int iv, Vector3d const &V) const
{
    Vector3d const &N = vertexAt(iv).normal();
    Vector3d v=V;
    v.normalize();
    if(N.norm()<1.e-6) return 0.0; // the normal has not been set

    double costheta = m_U.dot(v);
    double sintheta = (m_U * v).dot(N);

    if     (costheta>=0 && sintheta>=0)
        return  acos(costheta);  //first quadrant
    else if(costheta<0  && sintheta>=0)
        return  acos(costheta);  //second quadrant
    else if(costheta<0  && sintheta<0)
        return -acos(costheta) + 2*PI;  //third quadrant
    else if(costheta>=0 && sintheta<0)
        return -acos(costheta) + 2*PI;  //fourth quadrant

//    qDebug("Quadrant error");
    return 0;
}


bool Segment3d::intersectsProjected(Segment3d const &seg, Node &I, double precision) const
{
    // Make the average normal
    Vector3d k = (vertexAt(0).normal()+vertexAt(1).normal()).normalized();
    Vector3d i = unitDir();
    Vector3d j = k*i;

    // define the distance beyond which there is no need to check for potential intesection
    double dist = 2.0*length();
    Vector3d Vtx[2];
    Vector2d vtx[2];
    Vector2d I2d;

    // define a 2d referential using the segment's CoG, the segment's unit dir and the normal
    Segment2d seg2d({-length()/2, 0}, {length()/2.0, 0}); // aligned with the x-axis

    // not interested in the segment's self intersection
    if(isSame(seg, precision)) return false;

    // not interested in intersection with adjacent segments
    if(seg.vertexAt(0).isSame(vertexAt(0), precision)) return false;
    if(seg.vertexAt(1).isSame(vertexAt(0), precision)) return false;
    if(seg.vertexAt(0).isSame(vertexAt(1), precision)) return false;
    if(seg.vertexAt(1).isSame(vertexAt(1), precision)) return false;

    if(CoG().distanceTo(seg.CoG())>dist) return false; // discard distant segments

    // build the projected segment
    for(int iv=0; iv<2; iv++)
    {
        Vtx[iv] = seg.vertexAt(iv);
        // Substract the elevation
        Vtx[iv] -= k * (seg.CoG()-CoG()).dot(k);

        vtx[iv].x = (Vtx[iv]-CoG()).dot(i);
        vtx[iv].y = (Vtx[iv]-CoG()).dot(j);
    }

    if(seg2d.intersects(vtx[0], vtx[1], I2d, true, precision))
    {
        // Convert the intersection point back to 3d;
        double tau = I2d.x; // coordinate along the 2d segment unit vector
        I = CoG()+ segment()*tau; // mid point is the origin
        return true;
    }

    return false;
}


void Segment3d::getPoint(double xrel, Vector3d &pt) const
{
    pt.x = m_S[0].x * (1.0-xrel) + m_S[1].x * xrel;
    pt.y = m_S[0].y * (1.0-xrel) + m_S[1].y * xrel;
    pt .z= m_S[0].z * (1.0-xrel) + m_S[1].z * xrel;
}


int Segment3d::nodeIndex(int ivtx) const
{
    if(ivtx<0||ivtx>1) return -1;
    return m_S[ivtx].index();
}


void Segment3d::setNodeIndex(int ivtx, int index)
{
    if(ivtx<0||ivtx>1) return;
    m_S[ivtx].setIndex(index);
}



