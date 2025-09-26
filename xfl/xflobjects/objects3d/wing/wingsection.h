/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



/**
 *@file This class defines the wing section object used to define wing geometries.
 */


#pragma once



#include <QString>

#include <xflcore/enums_objects.h>
#include <xflmath/mathelem.h>


/**
*@class WingSection
*@brief  The class which defines the wing section object used to construct wing geometries.
*/
class WingSection
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
        QString const &rightFoilName()  const {return m_RightFoilName;}
        QString const &leftFoilName()   const {return m_LeftFoilName;}


        void setXDistType(xfl::enumDistribution type) {m_XPanelDist=type;}
        void setYDistType(xfl::enumDistribution type) {m_YPanelDist=type;}
        void setNX(int nx) {m_NXPanels = std::max(1,nx);}
        void setNY(int ny) {m_NYPanels = std::max(1,ny);}
        void setChord(double ch) {m_Chord=ch;}
        void setLength(double l) {m_Length=l;}
        void setyPosition(double y) {m_YPosition=y;}
        void setOffset(double off) {m_Offset=off;}
        void setDihedral(double dih) {m_Dihedral=dih;}
        void setzPos(double z) {m_ZPos=z;}
        void setTwist(double tw) {m_Twist=tw;}
        void setyProj(double y)  {m_YProj=y;}
        void setRightFoilName(QString const &FoilName) {m_RightFoilName=FoilName;}
        void setLeftFoilName(QString const &FoilName)  {m_LeftFoilName=FoilName;}

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

        QString m_LeftFoilName;  /**< The name of the foil on the leftt side of the section */
        QString m_RightFoilName; /**< The name of the foil on the right side of the section */
};

