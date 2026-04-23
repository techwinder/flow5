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



/**
 *@file This class FL5LIB_EXPORT defines the sail section object class FL5LIB_EXPORT used to define sail geometries.
 */


#pragma once

#include <QDataStream>

#include <string>

#include <mathelem.h>
#include <geom_enums.h>
#include <vector3d.h>

/**
*@class FL5LIB_EXPORT SailSection
*@brief  The class FL5LIB_EXPORT which defines the sail section object used to construct thick sail geometries.
*/
class FL5LIB_EXPORT WingSailSection
{
    public:
        WingSailSection() : m_NXPanels{5}, m_NZPanels{3}, m_XPanelDist{xfl::COSINE}, m_ZPanelDist{xfl::COSINE}, m_Chord{1}, m_Twist{0}
        {
        }

        void sectionPoint(double t, xfl::enumSurfacePosition pos, double &x, double &y) const;

        bool serializeFl5(QDataStream &ar, bool bIsStoring);

        std::string const &foilName()  const {return m_FoilName;}
        int nxPanels() const {return m_NXPanels;}
        int nzPanels() const {return m_NZPanels;}
        double chord() const {return m_Chord;}
        double twist() const {return m_Twist;}
        xfl::enumDistribution xDistType() const {return m_XPanelDist;}
        xfl::enumDistribution zDistType() const {return m_ZPanelDist;}

        void setFoilName(std::string const &name) {m_FoilName = name;}
        void setNXPanels(int nx);
        void setNZPanels(int nx);
        void setChord(double ch) {m_Chord=ch;}
        void setTwist(double tw) {m_Twist=tw;}
        void setXPanelDistType(xfl::enumDistribution disttype) {m_XPanelDist=disttype;}
        void setZPanelDistType(xfl::enumDistribution disttype) {m_ZPanelDist=disttype;}

        void getPoints(std::vector<Vector3d> &points, int nx, xfl::enumDistribution xdist=xfl::COSINE) const;

        // --------------- Variables -----------------------

        int m_NXPanels;          /**< number of mesh panels along the chord, for each sail panel */
        int m_NZPanels;         /**< number of mesh panels along the span, for each sail panel */
        xfl::enumDistribution m_XPanelDist;       /**< mesh distribution type, for each sail panel */
        xfl::enumDistribution m_ZPanelDist;       /**< mesh distribution type, for each sail panel */

        double m_Chord;         /**< Chord length at each panel side */
        double m_Twist;         /**< Twist value of each foil (measured to the sail root) */

        std::string m_FoilName;  /**< The name of the foil on the left side of the sail */
};


