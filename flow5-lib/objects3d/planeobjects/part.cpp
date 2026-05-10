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


#include <part.h>
#include <objects3d.h>
#include <utils.h>

bool Part::s_bOccTessellator = false;

GmshParams Part::s_GmshTessDefault;

Part::Part()
{
    m_UniqueIndex = -1;

    m_FirstPanel3Index = 0;
    m_FirstPanel4Index = 0;
    m_FirstNodeIndex = 0;

    m_theStyle.m_bIsVisible  = true;

    m_theStyle.m_Color.setRgb(111, 131, 157);

    m_theStyle.m_Stipple = Line::SOLID;
    m_theStyle.m_Width = 1;

    m_Name = "Part name";
    m_Description.clear();

    m_Inertia.reset();

    m_rx=m_ry=m_rz=0.0;

    m_bLocked = false;

    m_Length = 0.0;

    m_bAutoInertia = true;

    m_GmshTessParams = s_GmshTessDefault;
 //   m_GmshTessParams.m_MinSize    = 0.015; // fine for tessellation
 //   m_GmshTessParams.m_MaxSize    = 1.00;
 //   m_GmshTessParams.m_nCurvature = 20;
}


Part::~Part()
{

}


void Part::setUniqueIndex()
{
    m_UniqueIndex = Objects3d::newUniquePartIndex();
}


void Part::duplicatePart(Part const &part)
{
    //    m_UniqueIndex = part.uniqueIndex();
    m_bLocked   = part.m_bLocked;

    m_theStyle = part.theStyle();
    m_Name  = part.m_Name;
    m_Description = part.m_Description;

    m_Length = part.m_Length;

    m_LE = part.m_LE;
    m_rx = part.m_rx;
    m_ry = part.m_ry;
    m_rz = part.m_rz;

    m_FirstPanel3Index = part.firstPanel3Index();
    m_FirstPanel4Index = part.firstPanel4Index();
    m_FirstNodeIndex = part.firstNodeIndex();

    m_bAutoInertia = part.m_bAutoInertia;
    copyInertia(part);

    m_GmshTessParams = part.m_GmshTessParams;
    m_GmshParams     = part.m_GmshParams;
}


void Part::copyInertia(Part const &part)
{
    m_Inertia = part.m_Inertia;
}




