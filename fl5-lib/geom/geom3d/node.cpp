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

#include <algorithm>



#include <node.h>


void Node::addTriangleIndex(int index)
{
//    if(!m_TriangleIndex.contains(index)) m_TriangleIndex.push_back(index);
    if(std::find(m_TriangleIndex.begin(), m_TriangleIndex.end(), index) == m_TriangleIndex.end())
    {
        m_TriangleIndex.push_back(index);
    }
}

void Node::addNeighbourIndex(int index)
{
    if(index==m_Index) return;

//    if(!m_NeighbourIndex.contains(index)) m_NeighbourIndex.push_back(index);
    if(std::find(m_NeighbourIndex.begin(), m_NeighbourIndex.end(), index) == m_NeighbourIndex.end())
    {
        m_NeighbourIndex.push_back(index);
    }
}


std::string Node::properties() const
{
    QString props;
    QString strong,str;

    props.append(QString::asprintf("Node %d:\n", m_Index));
    props.append(QString::asprintf("   position= (%9g, %9g, %9g)\n", x, y, z));
    props.append(QString::asprintf("   normal  = (%9g, %9g, %9g)\n", m_Normal.x, m_Normal.y, m_Normal.z));

    strong.clear();
    for(unsigned int i=0; i<m_TriangleIndex.size(); i++)
    {
        str = QString::asprintf(" %d", m_TriangleIndex.at(i));
        strong.append(str);
    }
    props.append("   connected triangles:").append(strong).append("\n");

    strong.clear();
    for(unsigned int in=0; in<m_NeighbourIndex.size(); in++)
    {
        str = QString::asprintf(" %d", m_NeighbourIndex.at(in));
        strong.append(str);
    }
    props.append("   connected nodes:").append(strong).append("\n");

    switch(m_Position)
    {
        case xfl::MIDSURFACE:  props.append("   MID SURFACE");   break;
        case xfl::BOTSURFACE:  props.append("   BOT SURFACE");   break;
        case xfl::TOPSURFACE:  props.append("   TOP SURFACE");   break;
        case xfl::SIDESURFACE: props.append("   SIDE SURFACE");  break;
        case xfl::WAKESURFACE: props.append("   WAKE SURFACE");  break;
        case xfl::FUSESURFACE: props.append("   FUSE SURFACE");  break;
        case xfl::NOSURFACE:   props.append("   NO SURFACE");    break;
    }

    return props.toStdString();
}
