/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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


#include <aeroforces.h>

AeroForces::AeroForces()
{
    resetAll();
    resetResults();
}


void AeroForces::makeFrames()
{
    Vector3d Origin;
    double cosa = cos(m_Alpha*PI/180.0);
    double sina = sin(m_Alpha*PI/180.0);

    m_CFWind.setFrame(Origin, { cosa, 0,  sina},    {0,1,0},    {-sina, 0,  cosa});
    m_CFStab.setFrame(Origin, {-cosa, 0, -sina},    {0,1,0},    { sina, 0, -cosa});
}


double AeroForces::CL() const
{
//    return m_RefArea>MINREFAREA ? m_Fff.dot(windNormal(   m_Alpha, m_Beta)/m_RefArea) : 0.0;
    // changed in v7.24
    if(m_RefArea<MINREFAREA) return 0.0;
    return m_Fff.dot(m_CFWind.Kdir())/m_RefArea;
}


double AeroForces::CSide() const
{
    return m_RefArea>MINREFAREA ? m_Fff.dot(m_CFWind.Jdir())/m_RefArea : 0.0;
}


double AeroForces::CDi() const
{
    return m_RefArea>MINREFAREA ? m_Fff.dot(m_CFWind.Idir())/m_RefArea : 0.0;
}


double AeroForces::Cli() const
{
    if(m_RefArea<MINREFAREA || m_RefChord<MINREFDIMENSION) return 0.0;
    // changed in v7.24
    return m_Mi.dot(m_CFWind.Idir())/m_RefSpan/m_RefArea;
}


double AeroForces::Cmi() const
{
    if(m_RefArea<MINREFAREA || m_RefChord<MINREFDIMENSION) return 0.0;

//    return  m_Mi.y/m_RefChord/m_RefArea; // because y_stab is same as y_geom
    // changed in v7.24
    return m_Mi.dot(m_CFWind.Jdir())/m_RefChord/m_RefArea;
}


double AeroForces::Cmv() const
{
    if(m_RefArea<MINREFAREA || m_RefChord<MINREFDIMENSION) return 0.0;

//    return  m_Mv.y/m_RefChord/m_RefArea;
    // changed in v7.24
    return m_Mv.dot(m_CFWind.Jdir())/m_RefChord/m_RefArea;
}


double AeroForces::Cm() const
{
    return Cmi()+Cmv();
}


double AeroForces::Cni() const
{
    if(m_RefArea<MINREFAREA || m_RefChord<MINREFDIMENSION) return 0.0;
//    return m_Mi.z/m_RefSpan/m_RefArea;

    // changed in v7.24
    return m_Mi.dot(m_CFWind.Kdir())/m_RefSpan/m_RefArea;
}


double AeroForces::Cnv() const
{
    if(m_RefArea<MINREFAREA || m_RefChord<MINREFDIMENSION) return 0.0;
    return m_Mv.dot(m_CFWind.Kdir())/m_RefSpan/m_RefArea;
}


double AeroForces::Cn() const
{
    return Cni()+Cnv();
}


void AeroForces::duplicate(AeroForces const & ac)
{
    m_Alpha       = ac.m_Alpha;
    m_Beta        = ac.m_Beta;
    m_QInf        = ac.m_QInf;
    m_RefArea     = ac.m_RefArea;
    m_RefChord    = ac.m_RefChord;
    m_RefSpan     = ac.m_RefSpan;

    m_Fff         = ac.m_Fff;
    m_Fsum        = ac.m_Fsum;
    m_ProfileDrag = ac.m_ProfileDrag;
    m_FuseDrag    = ac.m_FuseDrag;
    m_ExtraDrag   = ac.m_ExtraDrag;
    m_Mi          = ac.m_Mi;
    m_Mv          = ac.m_Mv;
    m_M0          = ac.m_M0;
}

void AeroForces::resetAll()
{
    m_Alpha = 0.0;
    m_Beta  = 0.0;
    m_Phi   = 0.0;
    m_QInf  = 0.0;

    m_RefArea  = 1.0;
    m_RefChord = 1.0;
    m_RefSpan  = 1.0;
    resetResults();
}


void AeroForces::resetResults()
{
    m_Fff.set(0.0,0.0,0.0);
    m_Fsum.set(0.0,0.0,0.0);
    m_ProfileDrag = m_FuseDrag = m_ExtraDrag = 0.0;
    m_Mi.set(0.0,0.0,0.0);
    m_Mv.set(0.0,0.0,0.0);
    m_M0.set(0.0,0.0,0.0);
}

void AeroForces::scaleForces(double q)
{
    m_Fff         *= q;
    m_Fsum        *= q;
    m_ProfileDrag *= q;
    m_FuseDrag    *= q;
    m_ExtraDrag   *= q;
    m_Mi          *= q;
    m_Mv          *= q;
}


void AeroForces::serializeFl5_b17(QDataStream &ar, bool bIsStoring)
{
    double dble(0);
    if(bIsStoring)
    {
        //deprecated
    }
    else
    {
        ar >> m_RefArea >> m_RefChord >> m_RefSpan;
        ar >> dble >> dble >> dble;
        ar >> m_Fff.x  >> m_Fff.y  >> m_Fff.z;
        ar >> m_Fsum.x >> m_Fsum.y >> m_Fsum.z;
        ar >> m_ProfileDrag >> m_FuseDrag >> m_ExtraDrag;
        ar >> m_Mi.x >> m_Mi.y >> m_Mi.z;
        ar >> m_Mv.x >> m_Mv.y >> m_Mv.z;
    }
}

bool AeroForces::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat = 500750;
    // beta18: added Archive format, alpha, beta ,QInf;
    // 5000750:  v750 removed Cp and added M0

    double dble(0);

    if(bIsStoring)
    {
        ar << ArchiveFormat;
        ar << m_Alpha << m_Beta << m_QInf;
        ar << m_RefArea << m_RefChord << m_RefSpan;
        ar << dble << dble << dble;  // formerly CP
        ar << m_Fff.x<<m_Fff.y<<m_Fff.z;
        ar << m_Fsum.x<<m_Fsum.y<<m_Fsum.z;
        ar << m_ProfileDrag << m_FuseDrag << m_ExtraDrag;
        ar << m_Mi.x<<m_Mi.y<<m_Mi.z;
        ar << m_Mv.x<<m_Mv.y<<m_Mv.z;
        ar << m_M0.x<<m_M0.y<<m_M0.z;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<500000 || ArchiveFormat>510000) return false;
        ar >> m_Alpha >> m_Beta >> m_QInf;
        ar >> m_RefArea >> m_RefChord >> m_RefSpan;
        ar >> dble >> dble >> dble; // formerly CP
        ar >> m_Fff.x  >> m_Fff.y  >> m_Fff.z;
        ar >> m_Fsum.x >> m_Fsum.y >> m_Fsum.z;
        ar >> m_ProfileDrag >> m_FuseDrag >> m_ExtraDrag;
        ar >> m_Mi.x >> m_Mi.y >> m_Mi.z;
        ar >> m_Mv.x >> m_Mv.y >> m_Mv.z;

        if(ArchiveFormat>=500750)
            ar >> m_M0.x >> m_M0.y >> m_M0.z;

        if(ArchiveFormat<500002)
        {
            m_Mi.x=-m_Mi.x; /*  m_Mi.y=-m_Mi.y; */  m_Mi.z=-m_Mi.z;
            m_Mv.x=-m_Mv.x; /*  m_Mv.y=-m_Mv.y; */  m_Mv.z=-m_Mv.z;
        }
    }
    return true;
}


void AeroForces::displayAF()
{
    qDebug("  Area=%13.5f   Chord=%13.5f   Span=%13.5f", m_RefArea, m_RefChord, m_RefSpan);
    qDebug("  CX =%13.5f  CY =%13.5f  CL=%13.5f", Cx(), Cy(), Cz());
    qDebug("  CDv=%13.5f  ", CDv());
    qDebug("  Cmi=%13.5f  Cmv=%13.5f  ", Cmi(), Cmv());
    qDebug("  Cli=%13.5f  Cni=%13.5f  ", Cli(), Cni());
}


/**
 * No universal definition for the center of pressure; using:
 * in the case of planes, sum((Fi.windnormal) x ri) / sum((Fi.windnormal)
 * @todo in the case of boats, sum((Fi.windside) x ri) / sum((Fi.windside)
 * Fi is the panel force acting at point ri
 */
Vector3d AeroForces::centreOfPressure() const
{
    double norm = m_Fsum.dot(m_CFWind.Kdir());
    return m_M0/norm;
}


