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
 *@file
 *
 * This class FL5LIB_EXPORT implements the surface object on which the panels are constructed for the VLM and 3d-panel calculations.
 *
 */


#pragma once


#include <QDataStream>


#include <xflobject.h>

#include <bldata.h>
#include <blxfoil.h>
#include <enums_objects.h>

class Foil;
class Polar;

/**
*@class OpPoint
*@brief
 * The class which defines the operating point associated to Foil objects.

An OpPoint object stores the results of an XFoil calculation.
Each instance of an OpPoint is uniquely attached to a Polar object, which is itself attached uniquely to a Foil object.
The identification of the parent Polar and Foil are made using the names of the objects.
*/
class FL5LIB_EXPORT OpPoint : public XflObject
{
    public:
        OpPoint();

        bool isXFoil() const {return m_BLMethod==BL::XFOIL;}

        void duplicate(OpPoint const &opp);

        void setHingeMoments(const Foil *pFoil);

        void exportOpp(std::string &out, const std::string &Version, bool bCSV, const std::string &textseparator) const;

        std::string properties(const std::string &textseparator, bool bData=false) const;

        void resizeSurfacePoints(int N);
        int nPoints() const {return int(m_Cpv.size());}

        std::string const &foilName()     const {return m_FoilName;}
        std::string const &polarName()    const {return m_PlrName;}
        std::string name()  const override;
        std::string fullName() const;

        void setFoilName(std::string const &newFoilName) {m_FoilName = newFoilName;}
        void setPolarName(std::string const &plrName) {m_PlrName=plrName;}

        bool bViscResults() const {return m_bViscResults;}

        bool isPolarOpp(Polar const*pPolar) const;
        bool isFoilOpp(Foil const*pFoil) const;

        double aoa() const {return m_Alpha;}
        void setAoaSpec(double alpha) {m_Alpha=alpha;}

        double Reynolds() const {return m_Reynolds;}
        void setReynolds(double Re) {m_Reynolds=Re;}

        double Mach() const {return m_Mach; }
        void setMach(double M) {m_Mach = M;}

        double theta() const {return m_Theta;}
        void setTheta(double controlvalue) {m_Theta=controlvalue;}

        BL::enumBLMethod BLMethod() const {return m_BLMethod;}
        void setBLMethod(BL::enumBLMethod blmethod) {m_BLMethod=blmethod;}

        void setPolarType(xfl::enumPolarType type) {m_PolarType=type;}
        xfl::enumPolarType polarType() const {return m_PolarType;}

        bool isType1()           const {return m_PolarType==xfl::T1POLAR;}   /**< returns true if the polar is of the FIXEDSPEEDPOLAR type, false otherwise >*/
        bool isType2()           const {return m_PolarType==xfl::T2POLAR;}    /**< returns true if the polar is of the FIXEDLIFTPOLAR type, false otherwise >*/
        bool isType3()           const {return m_PolarType==xfl::T3POLAR;}        /**< returns true if the polar is of the GLIDEPOLAR type, false otherwise >*/
        bool isType4()           const {return m_PolarType==xfl::T4POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isType6()           const {return m_PolarType==xfl::T6POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/

        bool serializeOppXFL(QDataStream &ar, bool bIsStoring, int ArchiveFormat=0);
        bool serializeOppFl5(QDataStream &ar, bool bIsStoring);

    public:
        xfl::enumPolarType m_PolarType;   /**< defines the type of the parent PlanePolar */

        std::string m_FoilName;         /**< the name of the parent Foil */
        std::string m_PlrName;          /**< the name of the parent Polar */

        bool m_bViscResults;        /**< true if viscous results are stored in this OpPoint */
        bool m_bBL;                 /**< true if boundary layer data is stored in this OpPoint */
        bool m_bTEFlap;             /**< true if the parent foil has a flap on the trailing edge */
        bool m_bLEFlap;             /**< true if the parent foil has a flap on the leading edge */

        double m_Reynolds;          /**< the Re number of the OpPoint */
        double m_Mach;              /**< the Mach number of the OpPoint */
        double m_Alpha;             /**< the aoa*/
        double m_Theta;             /**< the T.E. flap angle for a type 9 polar */
        double m_Cl;                  /**< the lift coefficient */
        double m_Cm;                  /**< the pitching moment coefficient */
        double m_Cd;                  /**< the drag coefficient - viscous only, since we are dealing with 2D analysis */
        double m_Cdp;                 /**< the pressure drag calculated indirectly from the SY formula using Cdp = Cd-Cdf; cf. XFoil doc. */
        double m_XTrTop;              /**< the laminar to turbulent transition point on the upper surface */
        double m_XTrBot;              /**< the laminar to turbulent transition point on the lower surface */
        double m_XLamSepTop;          /**< the point of laminar separation on the top surface */
        double m_XLamSepBot;          /**< the point of laminar separation on the bottom surface */
        double m_XTurbSepTop;         /**< the point of turbulent separation on the top surface */
        double m_XTurbSepBot;         /**< the point of turbulent separation on the bottom surface */
        double m_NCrit;               /**< the NCrit parameter which defines turbulent transition */
        double m_XCP;                 /**< the x-position of the centre of pressure */

        double m_TEHMom;            /**< the moment on the foil's trailing edge flap */
        double m_m_LEHMom;            /**< the moment on the foil's leading edge flap */
        double m_XForce;              /**< the y-component of the pressure forces */
        double m_YForce;              /**< the y-component of the pressure forces */
        double m_Cpmn;                /**< the min value of Cpv */

        BL::enumBLMethod m_BLMethod;

        // BL data
        BLXFoil m_BLXFoil;          /**< BL data from an XFoil analysis */

        std::vector<float> m_Cpi;
        std::vector<float> m_Cpv;
        std::vector<double> m_Qv;             /**< the distribution of stream velocity on the surfaces for a viscous analysis */
        std::vector<double> m_Qi;             /**< the distribution of stream velocity on the surfaces for an inviscid analysis */

};





