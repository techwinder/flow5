/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <xflgeom/geom3d/node.h>


QString Node::properties() const
{
    QString props;
    QString strong,str;
    props += QString::asprintf("Node %d:\n", m_Index);
    props += QString::asprintf("   position= (%9g, %9g, %9g)\n", x, y, z);
    props += QString::asprintf("   normal  = (%9g, %9g, %9g)\n", m_Normal.x, m_Normal.y, m_Normal.z);
    props += QString::asprintf("   upstream=%d  downstream=%d\n", m_iU, m_iD);

    strong.clear();
    for(int i=0; i<m_TriangleIndex.size(); i++)
    {
        str = QString::asprintf(" %d", m_TriangleIndex.at(i));
        strong += str;
    }
    props += "   connected triangles:" + strong +"\n";

    strong.clear();
    for(int in=0; in<m_NeighbourIndex.size(); in++)
    {
        str = QString::asprintf(" %d", m_NeighbourIndex.at(in));
        strong += str;
    }
    props += "   connected nodes:" + strong +"\n";

    switch(m_Position)
    {
        case xfl::MIDSURFACE:  props += "   MID SURFACE";   break;
        case xfl::BOTSURFACE:  props += "   BOT SURFACE";   break;
        case xfl::TOPSURFACE:  props += "   TOP SURFACE";   break;
        case xfl::SIDESURFACE: props += "   SIDE SURFACE";  break;
        case xfl::WAKESURFACE: props += "   WAKE SURFACE";  break;
        case xfl::FUSESURFACE: props += "   FUSE SURFACE";  break;
        case xfl::NOSURFACE:   props += "   NO SURFACE";    break;
    }

    return props;
}
