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



/**
 *@file This class defines the sail section object class used to define sail geometries.
 */


#pragma once

#include <QDataStream>

#include <QString>

#include <xflcore/enums_objects.h>
#include <xflmath/mathelem.h>

#include <xflgeom/geom3d/vector3d.h>

/**
*@class SailSection
*@brief  The class which defines the sail section object used to construct thick sail geometries.
*/
class WingSailSection
{
    public:
        WingSailSection() : m_NXPanels{5}, m_NZPanels{3}, m_XPanelDist{xfl::COSINE}, m_ZPanelDist{xfl::COSINE}, m_Chord{1}, m_Twist{0}
        {
        }

        void sectionPoint(double t, xfl::enumSurfacePosition pos, double &x, double &y) const;

        bool serializeFl5(QDataStream &ar, bool bIsStoring);

        QString const &foilName()  const {return m_FoilName;}
        int nxPanels() const {return m_NXPanels;}
        int nzPanels() const {return m_NZPanels;}
        double chord() const {return m_Chord;}
        double twist() const {return m_Twist;}
        xfl::enumDistribution xDistType() const {return m_XPanelDist;}
        xfl::enumDistribution zDistType() const {return m_ZPanelDist;}

        void setFoilName(QString const &name) {m_FoilName = name;}
        void setNXPanels(int nx);
        void setNZPanels(int nx);
        void setChord(double ch) {m_Chord=ch;}
        void setTwist(double tw) {m_Twist=tw;}
        void setXPanelDistType(xfl::enumDistribution disttype) {m_XPanelDist=disttype;}
        void setZPanelDistType(xfl::enumDistribution disttype) {m_ZPanelDist=disttype;}

        void getPoints(QVector<Vector3d> &points, int nx, xfl::enumDistribution xdist=xfl::COSINE) const;

        // --------------- Variables -----------------------

        int m_NXPanels;          /**< number of mesh panels along the chord, for each sail panel */
        int m_NZPanels;         /**< number of mesh panels along the span, for each sail panel */
        xfl::enumDistribution m_XPanelDist;       /**< mesh distribution type, for each sail panel */
        xfl::enumDistribution m_ZPanelDist;       /**< mesh distribution type, for each sail panel */

        double m_Chord;         /**< Chord length at each panel side */
        double m_Twist;         /**< Twist value of each foil (measured to the sail root) */

        QString m_FoilName;  /**< The name of the foil on the left side of the sail */
};


