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

    m_GmshTessParams.m_MinSize    = 0.01; // fine for tessellation
    m_GmshTessParams.m_MaxSize    = 1.00;
    m_GmshTessParams.m_nCurvature = 30;
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


bool Part::serializePartFl5(QDataStream &ar, bool bIsStoring)
{
    int nIntSpares=0;
    int nDbleSpares=0;
    int n=0;
    double dble=0.0;
    QString strange;

    //500001: new fl5 format;
    //500002: added m_bReversed
    //500754: added GmshParams;

    int ArchiveFormat = 500754;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << QString::fromStdString(m_Name);
        ar << QString::fromStdString(m_Description);

        m_theStyle.serializeFl5(ar, bIsStoring);

        m_Inertia.serializeFl5(ar, bIsStoring);

        ar << m_bAutoInertia;
        bool bReverse=false; // deprecated
        ar << bReverse; // added format 500002

        ar << m_GmshTessParams.m_MinSize;
        ar << m_GmshTessParams.m_MaxSize;
        ar << m_GmshTessParams.m_nCurvature;

        ar << m_GmshParams.m_MinSize;
        ar << m_GmshParams.m_MaxSize;
        ar << m_GmshParams.m_nCurvature;

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares;
        n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>501000) return false;

        ar >> strange;    m_Name = strange.toStdString();
        ar >> strange;    m_Description = strange.toStdString();

        m_theStyle.serializeFl5(ar, bIsStoring);

        m_Inertia.serializeFl5(ar, bIsStoring);

        ar >> m_bAutoInertia;

        if(ArchiveFormat>=500002)
        {
            bool bReverse = false; // deprecated
            ar >> bReverse;
        }

        if(ArchiveFormat>=500754)
        {
            ar >> m_GmshTessParams.m_MinSize;
            ar >> m_GmshTessParams.m_MaxSize;
            ar >> m_GmshTessParams.m_nCurvature;

            if(m_GmshTessParams.m_MinSize<0.0005) m_GmshTessParams.m_MinSize = 0.0005; // avoid excessively long tessellations
            if(m_GmshTessParams.m_MaxSize<0.001) m_GmshTessParams.m_MaxSize = 10.0; // cleaning past errors
            if(m_GmshTessParams.m_nCurvature<=0) m_GmshTessParams.m_nCurvature= 20; // cleaning past errors

            ar >> m_GmshParams.m_MinSize;
            ar >> m_GmshParams.m_MaxSize;
            ar >> m_GmshParams.m_nCurvature;
//            if(m_GmshParams.m_MinSize<0.0001) m_GmshParams.m_MinSize = 0.0001; // avoid excessively long meshing times
            if(m_GmshParams.m_MaxSize<0.001)  m_GmshParams.m_MaxSize = 10.0; // cleaning past errors
            if(m_GmshParams.m_nCurvature<=0)  m_GmshParams.m_nCurvature= 20; // cleaning past errors
        }

        // space allocation
        ar >> nIntSpares;
        if(nIntSpares<0 || nIntSpares>100) return false;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        if(nDbleSpares<0 || nDbleSpares>100) return false;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}



