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

#define _MATH_DEFINES_DEFINED


#include <externalsail.h>

ExternalSail::ExternalSail() : Sail()
{
    m_Lx = m_Ly = m_Lz = 0.0;
    m_WettedArea=0.0;
}


void ExternalSail::duplicate(Sail const*pSail)
{
    Sail::duplicate(pSail);

    ExternalSail const *pSTLSail = dynamic_cast<ExternalSail const*>(pSail);

    m_Lx = pSTLSail->m_Lx;
    m_Ly = pSTLSail->m_Ly;
    m_Lz = pSTLSail->m_Lz;
    m_WettedArea = pSTLSail->m_WettedArea;
}


void ExternalSail::computeProperties()
{
    m_Triangulation.computeSurfaceProperties(m_Lx, m_Ly, m_Lz, m_WettedArea);
}


double ExternalSail::size() const
{
    double size = m_Lx;
    size = std::max(size, m_Ly);
    size = std::max(size, m_Lz);
    return size;
}

