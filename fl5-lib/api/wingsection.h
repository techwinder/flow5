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
 *@file This class defines the wing section object used to define wing geometries.
 */


#pragma once



#include <string>

#include <fl5lib_global.h>

#include <enums_objects.h>
#include <mathelem.h>


/**
*@class WingSection
*@brief  The class which defines the wing section object used to construct wing geometries.
*/
class FL5LIB_EXPORT WingSection
{
    public:
        WingSection();


        bool serializeFl5(QDataStream &ar, bool bIsStoring);


        xfl::enumDistribution xDistType() const {return m_XPanelDist;}
        xfl::enumDistribution yDistType() const {return m_YPanelDist;}
        int nXPanels()     const {return m_NXPanels;}
        int nYPanels()     const {return m_NYPanels;}
        double chord()     const {return m_Chord;}
        double length()    const {return m_Length;}
        double yPosition() const {return m_YPosition;}
        double yProj()     const {return m_YProj;}
        double offset()    const {return m_Offset;}
        double dihedral()  const {return m_Dihedral;}
        double zPos()      const {return m_ZPos;}
        double twist()     const {return m_Twist;}
        std::string const &rightFoilName()  const {return m_RightFoilName;}
        std::string const &leftFoilName()   const {return m_LeftFoilName;}


        void setXDistType(xfl::enumDistribution type) {m_XPanelDist=type;}
        void setYDistType(xfl::enumDistribution type) {m_YPanelDist=type;}
        void setNX(int nx) {m_NXPanels = std::max(1,nx);}
        void setNY(int ny) {m_NYPanels = std::max(1,ny);}
        void setChord(double ch) {m_Chord=ch;}
        void setLength(double l) {m_Length=l;}
        void setYPosition(double y) {m_YPosition=y;}
        void setXOffset(double off) {m_Offset=off;}
        void setDihedral(double dih) {m_Dihedral=dih;}
        void setzPos(double z) {m_ZPos=z;}
        void setTwist(double tw) {m_Twist=tw;}
        void setyProj(double y)  {m_YProj=y;}
        void setRightFoilName(std::string const &FoilName) {m_RightFoilName=FoilName;}
        void setLeftFoilName(std::string const &FoilName)  {m_LeftFoilName=FoilName;}

         // --------------- Variables -----------------------

        int m_NXPanels;         /**< VLM Panels along the chord, for each Wing Panel */
        int m_NYPanels;         /**< VLM Panels along the span, for each Wing Panel */
        xfl::enumDistribution m_XPanelDist;       /**< VLM Panel distribution type, for each Wing Panel */
        xfl::enumDistribution m_YPanelDist;       /**< VLM Panel distribution type, for each Wing Panel */

        double m_Chord;         /**< Chord length at each panel side */
        double m_Length;        /**< the length of each panel */
        double m_YPosition;     /**< b-position of each panel end on developed surface */
        double m_YProj;         /**< b-position of each panel end projected on tbe xy plane */
        double m_Offset;        /**< b-position of each panel end */
        double m_Dihedral;      /**< b-position of each panel end */
        double m_ZPos;          /**< vertical offset - calculation result only */
        double m_Twist;         /**< Twist value of each foil (measured to the wing root) */

        std::string m_LeftFoilName;  /**< The name of the foil on the leftt side of the section */
        std::string m_RightFoilName; /**< The name of the foil on the right side of the section */
};

