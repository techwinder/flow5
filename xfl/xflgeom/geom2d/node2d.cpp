/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#include <xflgeom/geom2d/node2d.h>




void Node2d::rotateZ(Vector2d const &O, double beta)
{
    Vector2d::rotateZ(O, beta);
    m_N.rotateZ(beta);
}


void Node2d::rotateZ(double beta)
{
    Vector2d::rotateZ(beta);
    m_N.rotateZ(beta);
}

void Node2d::setNode(Node2d const &nd)
{
    x = nd.x;
    y = nd.y;
    m_N.set(nd.m_N);
    m_Index = nd.index();
    m_Triangles = nd.m_Triangles;
    m_Neigh = nd.m_Neigh;
}
