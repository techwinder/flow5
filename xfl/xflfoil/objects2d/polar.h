/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

/** 
 * @file
 * This file implements the Polar class for the 2D analysis of Foil objects
 *
 */


#pragma once

#include <QColor>
#include <QVector>
#include <QTextStream>

#include <xflcore/xflobject.h>
#include <xflcore/enums_objects.h>
#include <xflfoil/objects2d/bldata.h>


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
class Polar : public XflObject
{
    public:
        enum enumPolarVariable {ALPHA, CTRL, CL, CD, CDE4, CDP, CM, HMOM, CPMIN, CLCD, CL32CD, CLM12, RE, XCP, XTRTOP, XTRBOT, XLSTOP, XLSBOT, XTSTOP, XTSBOT};



    public:
        Polar();
        Polar(double Re, double NCrit, double xTrTop, double xTrBot, BL::enumBLMethod blmethod);
        Polar(const Polar &polar);

/*        void setCtrl(int, double, double) {}
        double ctrlMin(int) {return 0.0;}
        double ctrlMax(int) {return 0.0;} */

        double interpolateFromAlpha(double alpha, Polar::enumPolarVariable PlrVar, bool &bOutAlpha) const;
        double interpolateFromCl(double Cl, Polar::enumPolarVariable PlrVar, bool &bOutCl) const;

        void addOpPointData(OpPoint *pOpPoint);

        void addPoint(double Alpha, double Cd, double Cdp, double Cl, double Cm, double HMom, double Cpmn, double Reynolds, double XCp, double Ctrl,
                      double Xtr1, double Xtr2, double XLSTop, double XLSBot, double XTSTop, double XTSBot);

        void exportPolar(QTextStream &out, QString const &versionName, bool bDataOnly, bool bCSV) const;
        void reset();

        void makeCurve(QPolygonF &pts, int XVar, int YVar)  const;

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

        QString const &foilName() const  {return m_FoilName;}
        void setFoilName(QString newfoilname) {m_FoilName = newfoilname;}

        void getProperties(const Foil *pFoil, QString &polarProps);
        const QVector<double> &getPlrVariable(Polar::enumPolarVariable var) const;

        double aoa()   const   {return m_aoaSpec;}
        void setAoa(double alpha) {m_aoaSpec = alpha;}

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

        const QVector<double> &getVariable(int iVar) const;
        bool hasPoints() const {return m_Alpha.size()>0;}
        int pointCount() const {return m_Alpha.size();}

        bool isXFoil()  const {return m_BLMethod==BL::XFOIL;}

        BL::enumBLMethod BLMethod() const {return m_BLMethod;}
        void setBLMethod(BL::enumBLMethod blmethod) {m_BLMethod=blmethod;}


        static int variableCount() {return s_VariableNames.size();}
        static QString variableName(int iVar) {return (iVar>=0 && iVar<variableCount()) ? s_VariableNames.at(iVar) : QString(); }
        static QStringList const &variableNames() {return s_VariableNames;}


    public:

        QVector<double> m_Alpha;             /**< the array of aoa values, in degrees */
        QVector<double> m_Cl;                /**< the array of lift coefficients */
        QVector<double> m_XCp;               /**< the array of centre of pressure positions */
        QVector<double> m_Cd;                /**< the array of drag coefficients */
        QVector<double> m_Cdp;               /**< the array of Cdp? */
        QVector<double> m_Cm;                /**< the array of pitching moment coefficients */
        QVector<double> m_XTrTop;            /**< the array of transition points on the top surface */
        QVector<double> m_XTrBot;            /**< the array of transition points on the bottom surface */
        QVector<double> m_XLamSepTop;        /**< the array of laminar separation points on the top surface */
        QVector<double> m_XLamSepBot;        /**< the array of laminar separation points on the bottom surface */
        QVector<double> m_XTurbSepTop;       /**< the array of turbulent separation points on the top surface */
        QVector<double> m_XTurbSepBot;       /**< the array of turbulent separation points on the bottom surface */
        QVector<double> m_HMom;              /**< the array of flap hinge moments */
        QVector<double> m_Cpmn;              /**< the array of Cpmn? */
        QVector<double> m_ClCd;              /**< the array of glide ratios */
        QVector<double> m_Cl32Cd;            /**< the array of power factors*/
        QVector<double> m_RtCl;              /**< the array of aoa values */
        QVector<double> m_Re;                /**< the array of Re coefficients */
        QVector<double> m_Control;           /**< the array of theta (TE Angle theta) parameters */

    public:

        QString m_FoilName;                 /**< The name of the parent Foil to which this Polar object is attached */

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


        static QStringList s_VariableNames;

};




