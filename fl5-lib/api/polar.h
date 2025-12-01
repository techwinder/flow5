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
 * @file
 * This file implements the Polar class FL5LIB_EXPORT for the 2D analysis of Foil objects
 *
 */


#pragma once


#include <vector>

#include <xflobject.h>
#include <enums_objects.h>
#include <bldata.h>


class Foil;
class OpPoint;

/**
 *
 * @brief
 * This class defines the polar object for the 2D analysis of foils
 *
 * 	The class stores both the analysis parameters and the analysis results.
 * 	Each instance of this class is uniquely associated to an instance of a Foil object.
 */
class FL5LIB_EXPORT Polar : public XflObject
{
    public:
        enum enumPolarVariable {ALPHA, CTRL, CL, CD, CDE4, CDP, CM, HMOM, CPMIN, CLCD, CL32CD, CLM12, RE, XCP, XTRTOP, XTRBOT, XLSTOP, XLSBOT, XTSTOP, XTSBOT};



    public:
        Polar();
        Polar(double Re, double NCrit, double xTrTop, double xTrBot, BL::enumBLMethod blmethod);
        Polar(const Polar &polar);

        double interpolateFromAlpha(double alpha, Polar::enumPolarVariable PlrVar, bool &bOutAlpha) const;
        double interpolateFromCl(double Cl, Polar::enumPolarVariable PlrVar, bool &bOutCl) const;

        void addOpPointData(OpPoint *pOpPoint);

        void addPoint(double Alpha, double Cd, double Cdp, double Cl, double Cm, double HMom, double Cpmn, double Reynolds, double XCp, double Ctrl,
                      double Xtr1, double Xtr2, double XLSTop, double XLSBot, double XTSTop, double XTSBot);

        void exportToString(std::string &outstring, bool bDataOnly, bool bCSV) const;
        void reset();


        void copySpecification(const Polar *pPolar);
        void copySpecification(Polar const &polar);

        void copy(Polar *pPolar);
        void copy(Polar const &polar);

        void replaceOppDataAt(int pos, OpPoint const *pOpp);
        void insertOppDataAt( int pos, OpPoint const *pOpp);
        void insertPoint(int i);
        void removePoint(int i);

        double getCm0() const;
        double getZeroLiftAngle() const;
        void getStallAngles(double &negative, double &positive) const;

        void getAlphaLimits(double &amin, double &amax) const;
        void getClLimits(double &Clmin, double &Clmax) const;
        void getLinearizedCl(double &Alpha0, double &slope) const;

        std::string const &foilName() const  {return m_FoilName;}
        void setFoilName(std::string newfoilname) {m_FoilName = newfoilname;}

        std::string properties();

        const std::vector<double> &getPlrVariable(Polar::enumPolarVariable var) const;

        double aoaSpec()   const   {return m_aoaSpec;}
        void setAoaSpec(double alpha) {m_aoaSpec = alpha;}

        double Reynolds() const {return m_Reynolds;}
        void setReynolds(double Re) {m_Reynolds = Re;}

        double Mach()   const  {return m_Mach;}
        void setMach(double M) {m_Mach=M;}

        double NCrit()  const  {return m_ACrit;}
        void setNCrit(double N)    {m_ACrit = N;}

        double XTripTop() const  {return m_XTripTop;}
        void setXTripTop(double xtr) {m_XTripTop = xtr;}

        double XTripBot() const  {return m_XTripBot;}
        void setXTripBot(double xtr) {m_XTripBot = xtr;}

        int ReType()    const  {return m_ReType;}
        void setReType(int retype) {m_ReType=retype;}

        int MaType()    const  {return m_MaType;}
        void setMaType(int matype) {m_MaType=matype;}

        xfl::enumPolarType type() const {return m_Type;}
        void setType(xfl::enumPolarType type);

        void setTEFlapAngle(double theta) {m_TEFlapAngle=theta;}
        double TEFlapAngle() const {return m_TEFlapAngle;}

        bool hasOpp(const OpPoint *pOpp) const;

        int compareTo(Polar const*pPolar) const;


        bool isType1()     const {return m_Type==xfl::T1POLAR;}
        bool isType2()     const {return m_Type==xfl::T2POLAR;}
        bool isType3()     const {return m_Type==xfl::T3POLAR;}
        bool isType12()    const {return m_Type==xfl::T1POLAR || m_Type==xfl::T2POLAR;}
        bool isType123()   const {return m_Type==xfl::T1POLAR || m_Type==xfl::T2POLAR || m_Type==xfl::T3POLAR;}
        bool isType4()     const {return m_Type==xfl::T4POLAR;}
        bool isType6()     const {return m_Type==xfl::T6POLAR;}

        bool isFixedSpeedPolar()  const {return m_Type==xfl::T1POLAR;}
        bool isFixedLiftPolar()   const {return m_Type==xfl::T2POLAR;}
        bool isFixedaoaPolar()    const {return m_Type==xfl::T4POLAR;}
        bool isRubberChordPolar() const {return m_Type==xfl::T3POLAR;}
        bool isControlPolar()     const {return m_Type==xfl::T6POLAR;}

        bool serializePolarXFL(QDataStream &ar, bool bIsStoring);
        bool serializePolarFl5(QDataStream &ar, bool bIsStoring);

        const std::vector<double> &getVariable(int iVar) const;
        bool hasData() const {return m_Alpha.size()>0;}
        int dataSize() const {return int(m_Alpha.size());}
        void resizeData(int n);

        bool isXFoil()  const {return m_BLMethod==BL::XFOIL;}

        BL::enumBLMethod BLMethod() const {return m_BLMethod;}
        void setBLMethod(BL::enumBLMethod blmethod) {m_BLMethod=blmethod;}


        static int variableCount() {return int(s_VariableNames.size());}
        static std::string variableName(int iVar) {return (iVar>=0 && iVar<variableCount()) ? s_VariableNames.at(iVar) : std::string(); }
        static std::vector<std::string> const &variableNames() {return s_VariableNames;}


    public:

        std::vector<double> m_Alpha;             /**< the array of aoa values, in degrees */
        std::vector<double> m_Cl;                /**< the array of lift coefficients */
        std::vector<double> m_XCp;               /**< the array of centre of pressure positions */
        std::vector<double> m_Cd;                /**< the array of drag coefficients */
        std::vector<double> m_Cdp;               /**< the array of Cdp? */
        std::vector<double> m_Cm;                /**< the array of pitching moment coefficients */
        std::vector<double> m_XTrTop;            /**< the array of transition points on the top surface */
        std::vector<double> m_XTrBot;            /**< the array of transition points on the bottom surface */
        std::vector<double> m_XLamSepTop;        /**< the array of laminar separation points on the top surface */
        std::vector<double> m_XLamSepBot;        /**< the array of laminar separation points on the bottom surface */
        std::vector<double> m_XTurbSepTop;       /**< the array of turbulent separation points on the top surface */
        std::vector<double> m_XTurbSepBot;       /**< the array of turbulent separation points on the bottom surface */
        std::vector<double> m_HMom;              /**< the array of flap hinge moments */
        std::vector<double> m_Cpmn;              /**< the array of Cpmn? */
        std::vector<double> m_ClCd;              /**< the array of glide ratios */
        std::vector<double> m_Cl32Cd;            /**< the array of power factors*/
        std::vector<double> m_RtCl;              /**< the array of aoa values */
        std::vector<double> m_Re;                /**< the array of Re coefficients */
        std::vector<double> m_Control;           /**< the array of theta (TE Angle theta) parameters */

    public:

        std::string m_FoilName;                 /**< The name of the parent Foil to which this Polar object is attached */

        double m_Reynolds;

        //Analysis specification
        xfl::enumPolarType m_Type;     /**< the Polar type */
        int m_ReType;                       /**< the type of Reynolds number input, cf. XFoil documentation */
        int m_MaType;                       /**< the type of Mach number input, cf. XFoil documentation */
        double m_aoaSpec;                   /**< the specified aoa in the case of Type 4 polars */
        double m_Mach;                      /**< the Mach number */
        double m_ACrit;                     /**< the transition criterion */
        double m_XTripTop;                  /**< the point of forced transition on the upper surface */
        double m_XTripBot;                  /**< the point of forced transition on the lower surface */

        double m_TEFlapAngle;    /**< the trailing edge flap angle, in degrees*/

        BL::enumBLMethod m_BLMethod;


        static std::vector<std::string> s_VariableNames;

};




