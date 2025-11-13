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

#include <xflgeom/geom3d/node.h>


QString Node::properties() const
{
    QString props;
    QString strong,str;
    props += QString::asprintf("Node %d:\n", m_Index);
    props += QString::asprintf("   position= (%9g, %9g, %9g)\n", x, y, z);
    props += QString::asprintf("   normal  = (%9g, %9g, %9g)\n", m_Normal.x, m_Normal.y, m_Normal.z);

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
