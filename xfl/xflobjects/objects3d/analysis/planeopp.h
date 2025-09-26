/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#pragma once


/** @file
 *
 * This class defines the operating point object for the 3D analysis of planes
 *
 */
#include <xflobjects/objects3d/analysis/opp3d.h>

#include <xflcore/enums_core.h>
#include <xflcore/enums_objects.h>
#include <xflobjects/objects3d/analysis/wingopp.h>
#include <xflcore/xflobject.h>
#include <xflcore/enums_objects.h>
#include <xflobjects/objects3d/analysis/stabderivatives.h>
#include <xflobjects/objects3d/analysis/aeroforces.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflpanels/panels/vorton.h>

#include <complex>

class Plane;
class WPolar;
class Panel4;

/**
*@brief
*	This class defines the operating point object which stores the data of plane analysis
*
    Each instance of this class is uniquely associated to an instance of a WPolar, which is itself uniquely
    associated to a Wing or a Plane object.
    The results associated to each of the plane's wing is stored in WingOpp objects, declared as member variables.
    The data is stored in International Standard Units, i.e. meters, seconds, kg, and Newtons.
    Angular data is stored in degrees.
*/
class PlaneOpp : public Opp3d
{
    friend class XPlane;
    friend class MainFrame;
    friend class WPolar;
    friend class PlaneTask;
    friend class LLTTask;

    public:
        PlaneOpp();
        PlaneOpp(Plane const *pPlane, WPolar const *pWPolar, int panel4ArraySize, int panel3ArraySize);

        void addWingOpp(int PanelArraySize);
        void initialize();
        void allocateMemory(int panel4ArraySize, int panel3ArraySize);


        void exportMainDataToString(const Plane *pPlane, QString &poppdata, xfl::enumTextFileType filetype, const QString &textsep) const;
        void exportPanel4DataToString(const Plane *pPlane, const WPolar *pWPolar, xfl::enumTextFileType exporttype, QString &paneldata) const;
        void exportPanel3DataToString(const Plane *pPlane, const WPolar *pWPolar, xfl::enumTextFileType exporttype, const QString &textsep, QString &paneldata) const;

        bool hasWOpp() const {return m_WingOpp.size()>0;}
        WingOpp const &WOpp(int iw) const {return m_WingOpp.at(iw);}
        WingOpp &WOpp(int iw) {return m_WingOpp[iw];}
        int nWOpps() const {return m_WingOpp.size();}

        QString const &planeName() const {return m_PlaneName;}
        void setPlaneName(QString const & name) {m_PlaneName=name;}

        QString const &polarName() const override {return m_WPlrName;}
        void setPolarName(QString const &name) override {m_WPlrName=name;}

        xfl::enumPolarType polarType() const {return m_PolarType;}

        bool isType1()           const {return m_PolarType==xfl::T1POLAR;}   /**< returns true if the polar is of the FIXEDSPEEDPOLAR type, false otherwise >*/
        bool isType2()           const {return m_PolarType==xfl::T2POLAR;}    /**< returns true if the polar is of the FIXEDLIFTPOLAR type, false otherwise >*/
        bool isType3()           const {return m_PolarType==xfl::T3POLAR;}        /**< returns true if the polar is of the GLIDEPOLAR type, false otherwise >*/
        bool isType4()           const {return m_PolarType==xfl::T4POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isType5()           const {return m_PolarType==xfl::T5POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isType6()           const {return m_PolarType==xfl::T6POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isType7()           const {return m_PolarType==xfl::T7POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isType8()           const {return m_PolarType==xfl::T8POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isType123()         const {return isType1() || isType2() || isType3();}
        bool isType12358()       const {return isType123() || isType5() || isType8();}
        bool isFixedSpeedPolar() const {return m_PolarType==xfl::T1POLAR;}   /**< returns true if the polar is of the FIXEDSPEEDPOLAR type, false otherwise >*/
        bool isFixedLiftPolar()  const {return m_PolarType==xfl::T2POLAR;}    /**< returns true if the polar is of the FIXEDLIFTPOLAR type, false otherwise >*/
        bool isGlidePolar()      const {return m_PolarType==xfl::T3POLAR;}        /**< returns true if the polar is of the GLIDEPOLAR type, false otherwise >*/
        bool isFixedaoaPolar()   const {return m_PolarType==xfl::T4POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isBetaPolar()       const {return m_PolarType==xfl::T5POLAR;}         /**< returns true if the polar is of the BETAPOLAR type, false otherwise >*/
        bool isControlPolar()    const {return m_PolarType==xfl::T6POLAR;}      /**< returns true if the polar is of the CONTROLPOLAR type, false otherwise >*/
        bool isStabilityPolar()  const {return m_PolarType==xfl::T7POLAR;}    /**< returns true if the polar is of the STABILITYPOLAR type, false otherwise >*/
        bool isType8Polar()      const {return m_PolarType==xfl::T8POLAR;}
        bool isExternalPolar()   const {return m_PolarType==xfl::EXTERNALPOLAR;}


        bool isOut() const {return m_bOut;}

        bool serializePOppXFL(QDataStream &ar, bool bIsStoring);
        bool serializeFl5(QDataStream &ar, bool bIsStoring);

        void getProperties(const Plane *pPlane, const WPolar *pWPolar, QString &PlaneOppProperties) const;


        QString name() const override;
        QString title(bool bLong) const override;

        void computeStabilityInertia(const double *Inertia);
        void buildStateMatrices(int nAVLCtrls);
        bool solveEigenvalues(QString &log);
        void outputEigen(QString &log);

        double mass() const {return m_Mass;}
        Vector3d const &cog() const {return m_CoG;}

        std::complex<double> eigValue(int i) const {return m_EigenValue[i];}
        std::complex<double> eigVector(int i, int j) const {return m_EigenVector[i][j];}

        static void setVariableNames(const QString &strForce, const QString &strMoment);
        static QStringList const &variableNames() {return s_POppVariables;}
        static QString variableName(int iVar);
        static int variableCount() {return s_POppVariables.size();}

    private:

        QString m_PlaneName;        /**< the pPane's name to which the PlaneOpp is attached */
        QString m_WPlrName;         /**< the WPolar's name to which the PlaneOpp is attached */

        double m_Span;             /**< the parent's Wing span */
        double m_MAChord;          /**< the parent's Wing mean aerodynamic chord*/

        bool m_bOut;               /**<  true if the interpolation of viscous properties was outside the Foil Polar mesh */

    public:

        xfl::enumPolarType m_PolarType;   /**< defines the type of the parent WPolar */
        QVector<WingOpp> m_WingOpp;      /**< An array of pointers to the four WingOpp objects associated to the four wings */

        std::complex<double> m_EigenValue[8];      /**< the eigenvalues of the four longitudinal and four lateral modes */
        std::complex<double> m_EigenVector[8][4];  /**< the longitudinal and lateral eigenvectors (4 longitudinal + 4 lateral) x 4 components */

        std::complex<double> m_phiPH;      /**< complex phugoid eigenvalue computed using Phillip's formula */
        std::complex<double> m_phiDR;      /**< complex Dutch roll eigenvalue computed using Phillip's formula */

        double m_ALong[4][4];	                  /**< the longitudinal state matrix */
        double m_ALat[4][4];	                  /**< the lateral state matrix */
        QVector<QVector<double>> m_BLong;      /**< the longitudinal control vectors; as many as there are avl-type controls */
        QVector<QVector<double>> m_BLat;       /**< the lateral control vectors; as many as there are avl-type controls */

        double m_Mass;          /**< the calculated mass in case of a parametric Type 7 analysis, the reference weight otherwise */
        Vector3d m_CoG;         /**< the calculated CoG position in case of a parametric Type 7 analysis, the reference CoG position otherwise */
        double m_Inertia[4];      /**< In the CoG reference frame; Ixx, Iyy, Izz, Ixz*/

        double m_Is[3][3];	          /**< Inertia tensor in stability axis */

        StabDerivatives m_SD;

        QVector<AeroForces> m_FuseAF;    /**< Forces acting on the fuselages */

        static QStringList s_POppVariables;
};
