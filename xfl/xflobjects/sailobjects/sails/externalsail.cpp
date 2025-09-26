/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include "externalsail.h"

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

