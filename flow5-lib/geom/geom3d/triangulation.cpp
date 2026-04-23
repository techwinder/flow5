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


#include <QDataStream>

#include <triangulation.h>
#include <constants.h>


Triangulation::Triangulation()
{
}


#define MERGELENGTH 0.0001


/**
 * Make the array of unique Node objects from the list of triangles
 */
int Triangulation::makeNodes()
{
    //make the array of nodes and set the vertex indices
    m_Node.clear();
    m_Node.reserve(m_Triangle.size()*3);
    for(uint it=0; it<m_Triangle.size(); it++)
    {
        Triangle3d &t3 = m_Triangle[it];
        for(int iv=0; iv<3; iv++)
        {
            int iNode = isTriangleNode(t3.vertex(iv));
            if(iNode<0)
            {
                iNode = int(m_Node.size());
                m_Node.push_back(t3.vertex(iv));
            }
            m_Node[iNode].addTriangleIndex(it);
            t3.setVertexIndex(iv, iNode);
        }
    }
//    m_Node.squeeze();

    return int(m_Node.size());
}


/**
 * Assumes connections have been made
 * Sets the node normals
 */
void Triangulation::makeNodeNormals(bool bReversed)
{
    //make the node normals
    for(uint iNode=0; iNode<m_Node.size(); iNode++)
    {
        Node &node = m_Node[iNode];
        Vector3d normal(0.0,0.0,0.0);
        //what  is an average of 3d vectors???

        for(int it=0; it<node.triangleCount(); it++) // all indexes should be good to go, still...
        {
            if(node.triangleIndex(it)>=0 && node.triangleIndex(it)<int(m_Triangle.size()))
            {
                Triangle3d const &t3 = m_Triangle.at(node.triangleIndex(it));
                normal += t3.normal();
            }
        }
        double norm = normal.norm();
        if(fabs(norm)<LENGTHPRECISION)
        {
            // usually a flat surface, node normals cancel each other
            if(node.triangleCount()>0) normal = m_Triangle.at(node.triangleIndex(0)).normal();
            else
            {
                //error somewhere
                normal.set(0,0,1);
            }
            norm = normal.norm();
        }
        normal *= 1/norm;
        if(bReversed) normal.reverse();
        node.setNormal(normal);
    }

    //set the normals at the triangle nodes
    for(uint it=0; it<m_Triangle.size(); it++)
    {
        Triangle3d &t3 = m_Triangle[it];
        for(int iv=0; iv<3; iv++)
        {
            int iNode = t3.nodeIndex(iv);
            if(iNode>=0 && iNode<int(m_Node.size()))
                t3.setNodeNormal(iv, m_Node.at(iNode).normal());
        }
    }
}


int Triangulation::isTriangleNode(const Node &nd) const
{
    for(int idx=int(m_Node.size()-1); idx>=0; idx--)
    {
        if(fabs(m_Node.at(idx).x-nd.x)<MERGELENGTH)
        {
            if(fabs(m_Node.at(idx).y-nd.y)<MERGELENGTH)
            {
                if(fabs(m_Node.at(idx).z-nd.z)<MERGELENGTH)
                {
                    return idx;
                }
            }
        }
    }
    return -1;
}


void Triangulation::flipXZ()
{
    for(uint it=0; it<m_Triangle.size(); it++)
    {
        m_Triangle[it].flipXZ();
    }

    for(uint in=0; in<m_Node.size(); in++)
    {
        m_Node[in].y = m_Node[in].y;
    }
}


void Triangulation::scale(double XFactor, double YFactor, double ZFactor)
{
    for(uint it=0; it<m_Triangle.size(); it++)
    {
        m_Triangle[it].scale(XFactor, YFactor, ZFactor);
    }

    for(uint in=0; in<m_Node.size(); in++)
    {
        m_Node[in].x *= XFactor;
        m_Node[in].y *= YFactor;
        m_Node[in].z *= ZFactor;
    }
}


void Triangulation::translate(const Vector3d &T)
{
    for(uint it=0; it<m_Triangle.size(); it++)
    {
        m_Triangle[it].translate(T);
    }

    for(uint in=0; in<m_Node.size(); in++)
    {
        m_Node[in].translate(T);
    }
}


void Triangulation::rotate(Vector3d const &Origin, Vector3d const&axis, double theta)
{
    for(uint it=0; it<m_Triangle.size(); it++)
    {
        m_Triangle[it].rotate(Origin, axis, theta);
    }

    for(uint in=0; in<m_Node.size(); in++)
    {
        m_Node[in].rotate(Origin, axis, theta);
    }
}


bool Triangulation::intersect(Vector3d const &A, Vector3d const &B, Vector3d &Inear, Vector3d &N) const
{
    double dmax = +1.e10;
    bool bIntersect = false;
    Vector3d I;
    Vector3d U = (B-A).normalized();
    for(int t3=0; t3<nTriangles(); t3++)
    {
        Triangle3d const &t3d = triangleAt(t3);
        if(t3d.intersectRayInside(A, U, I))
        {
            double d = (A-I).norm();
            if(d<dmax)
            {
                dmax = d;
                Inear = I;
                N = t3d.normal();
                bIntersect = true;
            }
        }
    }

    return bIntersect;
}


void Triangulation::makeXZsymmetric()
{
    int nt = nTriangles();
    for(int it=0; it< nt; it++)
    {
        Triangle3d t3 = triangleAt(it);
        t3.makeXZsymmetric();
        appendTriangle(t3);
    }
}


void Triangulation::flipNormals()
{
    for(uint it=0; it<m_Triangle.size(); it++)
    {
        m_Triangle[it].reverseOrientation();
        for(int it=0; it<3; it++)
            m_Triangle[it].vertex(it).normal().reverse();
    }

    for(uint it=0; it<m_Node.size(); it++)
        m_Node[it].normal().reverse();
}


double Triangulation::wettedArea() const
{
    double area=0.0;
    for(uint it=0; it<m_Triangle.size(); it++)
    {
        area += m_Triangle.at(it).area();
    }
    return area;
}


void Triangulation::computeSurfaceProperties(double &lx, double &ly, double &lz, double &wettedarea)
{
    double xmax=0.0, xmin=0.0;
    double ymax=0.0, ymin=0.0;
    double zmax=0.0, zmin=0.0;
    wettedarea = 0.0;
    for(int i=0; i<nTriangles(); i++)
    {
        Triangle3d const &T = triangle(i);
        wettedarea += fabs(T.area());
        xmin = std::min(xmin, T.vertexAt(0).x);
        xmin = std::min(xmin, T.vertexAt(1).x);
        xmin = std::min(xmin, T.vertexAt(2).x);
        xmax = std::max(xmax, T.vertexAt(0).x);
        xmax = std::max(xmax, T.vertexAt(1).x);
        xmax = std::max(xmax, T.vertexAt(2).x);

        ymin = std::min(ymin, T.vertexAt(0).y);
        ymin = std::min(ymin, T.vertexAt(1).y);
        ymin = std::min(ymin, T.vertexAt(2).y);
        ymax = std::max(ymax, T.vertexAt(0).y);
        ymax = std::max(ymax, T.vertexAt(1).y);
        ymax = std::max(ymax, T.vertexAt(2).y);

        zmin = std::min(zmin, T.vertexAt(0).z);
        zmin = std::min(zmin, T.vertexAt(1).z);
        zmin = std::min(zmin, T.vertexAt(2).z);
        zmax = std::max(zmax, T.vertexAt(0).z);
        zmax = std::max(zmax, T.vertexAt(1).z);
        zmax = std::max(zmax, T.vertexAt(2).z);
    }
    lx = fabs(xmin-xmax);
    ly = fabs(ymax-ymin);
    lz = fabs(zmax-zmin);
}


bool Triangulation::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    int n(0);
    float xf(0),yf(0),zf(0);

    Vector3d V0,V1,V2;

    // 500001: new fl5 format
    int ArchiveFormat = 500002;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << nTriangles();
        for(int i=0; i<nTriangles(); i++)
        {
            Triangle3d const &t3d = triangleAt(i);
            ar << t3d.vertexAt(0).xf() << t3d.vertexAt(0).yf() << t3d.vertexAt(0).zf();
            ar << t3d.vertexAt(1).xf() << t3d.vertexAt(1).yf() << t3d.vertexAt(1).zf();
            ar << t3d.vertexAt(2).xf() << t3d.vertexAt(2).yf() << t3d.vertexAt(2).zf();
        }
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500001 || ArchiveFormat>500010) return false;

        m_Triangle.clear();
        m_Node.clear();

        ar >> n;
        m_Triangle.resize(n);
        for(int i3=0; i3<n; i3++)
        {
            ar >> xf >> yf >> zf;
            V0.set(double(xf), double(yf), double(zf));

            ar >> xf >> yf >> zf;
            V1.set(double(xf), double(yf), double(zf));

            ar >> xf >> yf >> zf;
            V2.set(double(xf), double(yf), double(zf));

            m_Triangle[i3].setTriangle(V0, V1, V2);
        }
    }
    return true;
}


/** Assumes that the connections have been made */
void Triangulation::getFreeEdges(std::vector<Segment3d> &freeedges) const
{
    freeedges.clear();
    for(uint i3=0; i3<m_Triangle.size(); i3++)
    {
        Triangle3d const &p3 = m_Triangle.at(i3);
        for(int i=0; i<3; i++)
        {
//            if(p3.neighbourEdgeIndex(0)!=i && p3.neighbourEdgeIndex(1)!=i && p3.neighbourEdgeIndex(2)!=i)
            if(p3.neighbour(i)<0)
            {
                // this edge is a free edge
                Segment3d const &seg = p3.edge(i);
                freeedges.push_back(seg);
            }
        }
    }
}


bool Triangulation::areNeighbours(Triangle3d const &t1, Triangle3d const &t2) const
{
    int nsharednodes=0;
    for(int iv=0; iv<3; iv++)
    {
        if(t1.hasVertex(t2.nodeIndex(iv))) nsharednodes++;
    }
    return(nsharednodes>1);
}

void Triangulation::clearConnections()
{
    for(uint it0=0; it0<m_Triangle.size(); it0++)
    {
        m_Triangle[it0].clearConnections();
    }
}

/**
 * Typically used to connect the triangles i.e. define the neighbours of a triangulation.
 * Time expensive, so should be done only when needed.
 * Sets the neighbours
 * Sets the indexes of the vertices
 * Makes the nodes
 */
void Triangulation::makeTriangleConnections()
{
    double const MAXDISTANCE = 1.e-4;

    clearConnections();

    for(uint it0=0; it0<m_Triangle.size(); it0++)
    {
        Triangle3d &t30 = m_Triangle[it0];
        // seek neighbours around the triangle's index first moving upwards and downwards
        for(int it1=it0-1; it1>=0; it1--)
        {
            Triangle3d &t31 = m_Triangle[it1];
            if(t30.neighbourCount()>=3) break; // a triangle has no more than 3 neighbours
            for(int iEdge=0; iEdge<3; iEdge++)
            {
                int nEdge = t30.edgeIndex(t31.edge(iEdge), MAXDISTANCE);
                if(nEdge>=0)
                {
                    t30.setNeighbour(it0, nEdge);
                    // take this opportunity to connect the other triangle too
                    nEdge = t31.edgeIndex(t30.edge(nEdge), MAXDISTANCE);
                    t31.setNeighbour(it0, nEdge);
                    break;
                }
            }
        }
        for(uint it1=it0+1; it1<m_Triangle.size(); it1++)
        {
            Triangle3d &t31 = m_Triangle[it1];
            if(t30.neighbourCount()>=3) break; // a triangle has no more than 3 neighbours
            for(int iEdge=0; iEdge<3; iEdge++)
            {
                int nEdge = t30.edgeIndex(t31.edge(iEdge), MAXDISTANCE);
                if(nEdge>=0)
                {
                    t30.setNeighbour(it0, nEdge);

                    // take this opportunity to connect the other triangle too
                    nEdge = t31.edgeIndex(t30.edge(nEdge), MAXDISTANCE);
                    t31.setNeighbour(it0, nEdge);
                    break;
                 }
            }
        }
    }
}




