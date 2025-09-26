/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <xflpanels/panels/panel.h>
#include <xflmath/constants.h>


double Panel::s_RFF = 10.0;


Panel::Panel() : m_Area{0}, m_index{-1}, m_iWake{-1}, m_iWakeColumn{-1}, m_MaxSize{0.0},
                 m_bIsLeftWingPanel{false}, m_bFlapPanel{false}, m_bIsLeading{false}, m_bIsTrailing{false}, m_bIsInSymPlane{false},
                 m_iPL{-1}, m_iPR{-1}, m_iPU{-1}, m_iPD{-1},
                 m_Pos{xfl::NOSURFACE}
{
}

    /**< a measure of the panel's minimum cross length, for RFF estimations */

bool Panel::isOppositeSurface(xfl::enumSurfacePosition pos) const
{
    if(isTopPanel()) return pos==xfl::BOTSURFACE;
    if(isBotPanel()) return pos==xfl::TOPSURFACE;
    return false;
}

