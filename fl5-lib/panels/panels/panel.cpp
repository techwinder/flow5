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


#include <panel.h>
#include <constants.h>


double Panel::s_RFF = 10.0;


Panel::Panel() : m_Area{0}, m_index{-1}, m_iWake{-1}, m_iWakeColumn{-1}, m_MaxSize{0.0}, m_SurfaceIndex{-1},
                 m_bIsLeftWingPanel{false}, m_bFlapPanel{false}, m_bIsLeading{false}, m_bIsTrailing{false}, m_bIsInSymPlane{false},
                 m_iPL{-1}, m_iPR{-1}, m_iPU{-1}, m_iPD{-1},
                 m_Pos{xfl::NOSURFACE}, m_bVisible{true}
{
}


bool Panel::isOppositeSurface(xfl::enumSurfacePosition pos) const
{
    if(isTopPanel()) return pos==xfl::BOTSURFACE;
    if(isBotPanel()) return pos==xfl::TOPSURFACE;
    return false;
}

