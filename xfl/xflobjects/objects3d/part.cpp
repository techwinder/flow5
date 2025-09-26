/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QRandomGenerator>

#include "part.h"
#include <xflobjects/objects3d/objects3d.h>


Part::Part()
{
    m_UniqueIndex = -1;

    m_FirstPanel3Index = 0;
    m_FirstPanel4Index = 0;
    m_FirstNodeIndex = 0;

    m_theStyle.m_bIsVisible  = true;

    m_theStyle.m_Color.setHsv(QRandomGenerator::global()->bounded(360),
                              QRandomGenerator::global()->bounded(55)+30,
                              QRandomGenerator::global()->bounded(55)+150);

    m_theStyle.m_Stipple = Line::SOLID;
    m_theStyle.m_Width = 1;

    m_Name = "Part Name";
    m_Description = QString();

    m_Inertia.reset();

    m_rx=m_ry=m_rz=0.0;

    m_bLocked = false;
    m_bIsMerged = true;

    m_Length = 0.0;

    m_bAutoInertia = true;

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
    m_bIsMerged = part.m_bIsMerged;

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

    //500001: new fl5 format;
    //500002:  added m_bReversed
    int ArchiveFormat = 500002;
    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << m_Name;
        ar << m_Description;

        m_theStyle.serializeFl5(ar, bIsStoring);

        m_Inertia.serializeFl5(ar, bIsStoring);

        ar << m_bAutoInertia;
        bool bReverse=false; // deprecated
        ar << bReverse; // added format 500002

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
        if(ArchiveFormat<500000 || ArchiveFormat>500002) return false;
        m_Name.clear();
        ar >> m_Name;
        ar >> m_Description;

        m_theStyle.serializeFl5(ar, bIsStoring);

        m_Inertia.serializeFl5(ar, bIsStoring);

        ar >> m_bAutoInertia;

        if(ArchiveFormat>=500002)
        {
            bool bReverse = false; // deprecated
            ar >> bReverse;
        }

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;
    }
    return true;
}



