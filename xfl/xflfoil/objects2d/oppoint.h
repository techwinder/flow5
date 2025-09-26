/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

/**
 *@file
 *
 * This class implements the surface object on which the panels are constructed for the VLM and 3d-panel calculations.
 *
 */


#pragma once

#include <QColor>
#include <QString>
#include <QTextStream>
#include <QDataStream>


#include <xflcore/xflobject.h>

#include <xflfoil/objects2d/bldata.h>
#include <xflfoil/objects2d/blxfoil.h>


class Foil;
class Polar;

/**
*@class OpPoint
*@brief
 * The class which defines the operating point associated to Foil objects.

An OpPoint object stores the results of an XFoil calculation.
Each instance of an OpPoint is uniquely attached to a Polar object, which is itself attached uniquely to a Foil object.
The identification of the parent Polar and Foil are made using the QString names of the objects.
*/
class OpPoint : public XflObject
{
    public:
        OpPoint();

        bool isXFoil() const {return m_BLMethod==BL::XFOIL;}

        void duplicate(OpPoint const &opp);

        void setHingeMoments(const Foil *pFoil);

        void exportOpp(QTextStream &out, QString const &Version, bool bCSV, const QString &textseparator) const;

        void getOppProperties(QString &OpPointProperties, const QString &textseparator, bool bData=false) const;

        void resizeSurfacePoints(int N);
        int nPoints() const {return int(Cpv.size());}

        QString const &foilName()     const {return m_FoilName;}
        QString const &polarName()    const {return m_PlrName;}
        QString name()  const override;

        void setFoilName(QString const &newFoilName) {m_FoilName = newFoilName;}
        void setPolarName(QString const &plrName) {m_PlrName=plrName;}

        bool bViscResults() const {return m_bViscResults;}

        bool isPolarOpp(Polar const*pPolar) const;
        bool isFoilOpp(Foil const*pFoil) const;

        double aoa() const {return m_Alpha;}
        void setAoa(double alpha) {m_Alpha=alpha;}

        double Reynolds() const {return m_Reynolds;}
        void setReynolds(double Re) {m_Reynolds=Re;}

        double Mach() const {return m_Mach; }
        void setMach(double M) {m_Mach = M;}

        double theta() const {return m_Theta;}
        void setTheta(double controlvalue) {m_Theta=controlvalue;}

        BL::enumBLMethod BLMethod() const {return m_BLMethod;}
        void setBLMethod(BL::enumBLMethod blmethod) {m_BLMethod=blmethod;}

        bool serializeOppXFL(QDataStream &ar, bool bIsStoring, int ArchiveFormat=0);
        bool serializeOppFl5(QDataStream &ar, bool bIsStoring);

    public:

        QString m_FoilName;         /**< the name of the parent Foil */
        QString m_PlrName;          /**< the name of the parent Polar */

        bool m_bViscResults;        /**< true if viscous results are stored in this OpPoint */
        bool m_bBL;                 /**< true if boundary layer data is stored in this OpPoint */
        bool m_bTEFlap;             /**< true if the parent foil has a flap on the trailing edge */
        bool m_bLEFlap;             /**< true if the parent foil has a flap on the leading edge */

        double m_Reynolds;          /**< the Re number of the OpPoint */
        double m_Mach;              /**< the Mach number of the OpPoint */
        double m_Alpha;             /**< the aoa*/
        double m_Theta;             /**< the T.E. flap angle for a type 9 polar */
        double Cl;                  /**< the lift coefficient */
        double Cm;                  /**< the pitching moment coefficient */
        double Cd;                  /**< the drag coefficient - viscous only, since we are dealing with 2D analysis */
        double Cdp;                 /**< the pressure drag calculated indirectly from the SY formula using Cdp = Cd-Cdf; cf. XFoil doc. */
        double XTrTop;              /**< the laminar to turbulent transition point on the upper surface */
        double XTrBot;              /**< the laminar to turbulent transition point on the lower surface */
        double XLamSepTop;          /**< the point of laminar separation on the top surface */
        double XLamSepBot;          /**< the point of laminar separation on the bottom surface */
        double XTurbSepTop;         /**< the point of turbulent separation on the top surface */
        double XTurbSepBot;         /**< the point of turbulent separation on the bottom surface */
        double NCrit;               /**< the NCrit parameter which defines turbulent transition */
        double XCP;                 /**< the x-position of the centre of pressure */

        double m_TEHMom;            /**< the moment on the foil's trailing edge flap */
        double m_LEHMom;            /**< the moment on the foil's leading edge flap */
        double XForce;              /**< the y-component of the pressure forces */
        double YForce;              /**< the y-component of the pressure forces */
        double Cpmn;                /**< the min value of Cpv */

        BL::enumBLMethod m_BLMethod;

        // BL data
        BLXFoil m_BLXFoil;          /**< BL data from an XFoil analysis */

        QVector<float> Cpi;
        QVector<float> Cpv;
        QVector<double> Qv;             /**< the distribution of stream velocity on the surfaces for a viscous analysis */
        QVector<double> Qi;             /**< the distribution of stream velocity on the surfaces for an inviscid analysis */

};





