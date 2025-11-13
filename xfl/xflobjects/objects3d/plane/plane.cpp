/****************************************************************************

    flow5
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


#include <xflobjects/objects3d/plane/plane.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflobjects/objects3d/fuse/fuseocc.h>
#include <xflobjects/objects3d/fuse/fusestl.h>
#include <xflobjects/objects3d/wing/surface.h>
#include <xflobjects/objects3d/inertia/pointmass.h>
#include <xflobjects/objects3d/wing/wingxfl.h>
#include <xflobjects/objects3d/analysis/wpolar.h>
#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflgeom/geom_globals/geom_global.h>
#include <xflpanels/panels/panel3.h>
#include <xflpanels/panels/panel4.h>
#include <xflcore/units.h>
#include <xflmath/constants.h>

Plane::Plane()
{
    m_bLocked = false;
    m_bIsActive = false;
    m_bIsInitialized = false;
    m_bAutoInertia = true;

    m_theStyle.m_Color = Qt::gray;
    m_theStyle.m_Width = 2;

    clearPointMasses();

    m_Name  = "Plane name";
}


bool Plane::hasWPolar(WPolar const*pWPolar) const {return pWPolar->planeName().compare(m_Name)==0;}
bool Plane::hasPOpp(PlaneOpp const*pPOpp)   const {return pPOpp->planeName().compare(m_Name)==0;}



void Plane::duplicate(Plane const*pOtherPlane)
{
    m_Name           = pOtherPlane->m_Name;
    m_Description    = pOtherPlane->m_Description;
    m_theStyle       = pOtherPlane->theStyle();
    m_bIsInitialized = pOtherPlane->isInitialized();
    m_bAutoInertia   = pOtherPlane->m_bAutoInertia;
    m_Inertia        = pOtherPlane->m_Inertia;

}


