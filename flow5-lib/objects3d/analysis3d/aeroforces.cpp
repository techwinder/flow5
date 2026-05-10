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

#include <iostream>
#include <format>

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



void AeroForces::displayAF()
{
    std::cout << std::format("  Area={:13.5f}   Chord={:13.5f}   Span={:13.5f}", m_RefArea, m_RefChord, m_RefSpan) << std::endl;
    std::cout << std::format("  CX ={:13.5f}  CY ={:13.5f}  CL={:13.5f}", Cx(), Cy(), Cz()) << std::endl;
    std::cout << std::format("  CDv={:13.5f}  ", CDv()) << std::endl;
    std::cout << std::format("  Cmi={:13.5f}  Cmv={:13.5f}  ", Cmi(), Cmv()) << std::endl;
    std::cout << std::format("  Cli={:13.5f}  Cni={:13.5f}  ", Cli(), Cni()) << std::endl;
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


