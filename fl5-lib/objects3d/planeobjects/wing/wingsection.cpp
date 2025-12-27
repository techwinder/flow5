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

#include <QString>
#include <QDataStream>

#include <wingsection.h>



WingSection::WingSection() : m_NXPanels{0}, m_NYPanels{0}, m_XPanelDist{xfl::COSINE}, m_YPanelDist{xfl::COSINE},
    m_Chord{0}, m_Length{0}, m_YPosition{0}, m_YProj{0}, m_Offset{0}, m_Dihedral{0}, m_ZPos{0}, m_Twist{0}
{
}


bool WingSection::serializeFl5(QDataStream &ar, bool bIsStoring)
{
    int k=0;
    double dble=0;
    QString strange;

    // 500001 : new fl5 format;
    int ArchiveFormat = 500001;

    if(bIsStoring)
    {
        ar << ArchiveFormat;

        ar << QString::fromStdString(m_RightFoilName);
        ar << QString::fromStdString(m_LeftFoilName);
        ar << chord();
        ar << yPosition();
        ar << offset();
        ar << dihedral();
        ar << twist();
        ar << nXPanels();
        ar << nYPanels();

        switch(xDistType())
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

        switch(yDistType())
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

        // space allocation for the future storage of more data, without need to change the format
        int nSpares=10;
        ar << nSpares;
        for (int i=0; i<nSpares; i++) ar << 0;
        for (int i=0; i<nSpares; i++) ar << 0.0;

        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat!=500001) return false;

        ar >> strange;   m_RightFoilName = strange.toStdString();
        ar >> strange;   m_LeftFoilName  = strange.toStdString();
        ar >> m_Chord;
        ar >> m_YPosition;
        ar >> m_Offset;
        ar >> m_Dihedral;
        ar >> m_Twist;
        ar >> m_NXPanels;
        ar >> m_NYPanels;

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
        if     (k==1)  m_YPanelDist = xfl::COSINE;
        else if(k==2)  m_YPanelDist = xfl::SINE;
        else if(k==-2) m_YPanelDist = xfl::INV_SINE;
        else if(k==3)  m_YPanelDist = xfl::INV_SINH;
        else if(k==4)  m_YPanelDist = xfl::TANH;
        else if(k==5)  m_YPanelDist = xfl::EXP;
        else if(k==6)  m_YPanelDist = xfl::INV_EXP;
        else           m_YPanelDist = xfl::UNIFORM;


        // space allocation
        int nSpares=0;
        ar >> nSpares;
        for (int i=0; i<nSpares; i++) ar >> k;
        for (int i=0; i<nSpares; i++) ar >> dble;

        return true;
    }
}
