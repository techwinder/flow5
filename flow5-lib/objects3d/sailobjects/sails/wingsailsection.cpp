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

#include <wingsailsection.h>

#include <foil.h>
#include <mathelem.h>
#include <objects2d.h>


void WingSailSection::setNXPanels(int nx)
{
    m_NXPanels=std::max(nx, 2);
    m_NXPanels=std::min(m_NXPanels, 1000);
}


void WingSailSection::setNZPanels(int nx)
{
    m_NZPanels=std::max(nx, 1);
    m_NZPanels=std::min(m_NZPanels, 1000);
}


void WingSailSection::sectionPoint(double t, xfl::enumSurfacePosition pos, double &x, double &y) const
{
    Foil *pFoil = Objects2d::foil(m_FoilName);
    if(!pFoil)
    {
        x=t*chord();
        y=0.0;
        return;
    }
    Vector2d pt;
    Vector2d N;
    switch (pos)
    {
        default:
        case xfl::MIDSURFACE:
        {
            pt = pFoil->midYRel(t, N);
            y = pt.y*m_Chord;
            break;
        }
        case xfl::TOPSURFACE:
        {
            pt = pFoil->upperYRel(t, N);
            y = pt.y*m_Chord;
            break;
        }
        case xfl::BOTSURFACE:
        {
            pt = pFoil->lowerYRel(t, N);
            y = pt.y*m_Chord;
            break;
        }
    }
    x = t*m_Chord;
}


bool WingSailSection::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    int n(0), k(0);
    int ArchiveFormat=500001;// identifies the format of the file
    double dble(0);
    int nIntSpares(0);
    int nDbleSpares(0);
    QString strange;

    if(bIsStoring)
    {
        // storing code
        ar << ArchiveFormat;
        ar << QString::fromStdString(m_FoilName);
        ar << m_NXPanels << m_NZPanels;
        ar << m_Chord << m_Twist;

        switch(m_XPanelDist)
        {
            case xfl::COSINE:      ar <<  1;  break;
            case xfl::SINE:        ar <<  2;  break;
            case xfl::INV_SINE:    ar << -2;  break;
            case xfl::INV_SINH:    ar <<  3;  break;
            case xfl::TANH:        ar <<  4;  break;
            case xfl::EXP:         ar <<  5;  break;
            case xfl::INV_EXP:     ar <<  6;  break;
            case xfl::UNIFORM:
            default:               ar <<  0;  break;
        }

        switch(m_ZPanelDist)
        {
            case xfl::COSINE:      ar <<  1;  break;
            case xfl::SINE:        ar <<  2;  break;
            case xfl::INV_SINE:    ar << -2;  break;
            case xfl::INV_SINH:    ar <<  3;  break;
            case xfl::TANH:        ar <<  4;  break;
            case xfl::EXP:         ar <<  5;  break;
            case xfl::INV_EXP:     ar <<  6;  break;
            case xfl::UNIFORM:
            default:               ar <<  0;  break;
        }

        // dynamic space allocation for the future storage of more data, without need to change the format
        nIntSpares=0;
        ar << nIntSpares; n=0;
        for (int i=0; i<nIntSpares; i++) ar << n;
        nDbleSpares=0;
        ar << nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar << dble;

        return true;
    }
    else
    {
        // loading code
        ar >> ArchiveFormat;

        if (ArchiveFormat!=500001)  return false;

        ar >> strange;  m_FoilName = strange.toStdString();
        ar >> m_NXPanels >> m_NZPanels;
        ar >> m_Chord >> m_Twist;

        ar >> k;
        if     (k==1)  m_XPanelDist = xfl::COSINE;
        else if(k==2)  m_XPanelDist = xfl::SINE;
        else if(k==-2) m_XPanelDist = xfl::INV_SINE;
        else if(k==3)  m_XPanelDist = xfl::INV_SINH;
        else if(k==4)  m_XPanelDist = xfl::TANH;
        else if(k==5)  m_XPanelDist = xfl::EXP;
        else if(k==6)  m_XPanelDist = xfl::INV_EXP;
        else           m_XPanelDist = xfl::UNIFORM;

        ar >> k;
        if     (k==1)  m_ZPanelDist = xfl::COSINE;
        else if(k==2)  m_ZPanelDist = xfl::SINE;
        else if(k==-2) m_ZPanelDist = xfl::INV_SINE;
        else if(k==3)  m_ZPanelDist = xfl::INV_SINH;
        else if(k==4)  m_ZPanelDist = xfl::TANH;
        else if(k==5)  m_ZPanelDist = xfl::EXP;
        else if(k==6)  m_ZPanelDist = xfl::INV_EXP;
        else           m_ZPanelDist = xfl::UNIFORM;

        // space allocation
        ar >> nIntSpares;
        for (int i=0; i<nIntSpares; i++) ar >> n;
        ar >> nDbleSpares;
        for (int i=0; i<nDbleSpares; i++) ar >> dble;

        return true;
    }
}


void WingSailSection::getPoints(std::vector<Vector3d> &points, int nx, xfl::enumDistribution xdist) const
{
    points.clear();
    Foil *pFoil = Objects2d::foil(foilName());
    std::vector<double> fraclist;
    xfl::getPointDistribution(fraclist, nx+1, xdist);

    Vector2d N;
    Vector2d fp;
    for(uint i=0; i<fraclist.size(); i++)
    {
        if(pFoil)
        {
            double tau = fraclist.at(i);
            fp = pFoil->upperYRel(tau, N);
            points.push_back({fp.x*m_Chord, fp.y*m_Chord, 0.0});
        }
        else {
            points.push_back({fraclist.at(i)*m_Chord, 0.0,0.0});
        }
    }
    for(int i=int(fraclist.size()-1); i>=0; i--)
    {
        if(pFoil)
        {
            double tau = fraclist.at(i);
            fp = pFoil->lowerYRel(tau, N);
            points.push_back({fp.x*m_Chord, fp.y*m_Chord, 0.0});
        }
        else {
            points.push_back({fraclist.at(i)*m_Chord, 0.0,0.0});
        }
    }
}




