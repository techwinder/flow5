/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


/** @file
 *
 * This class defines the polar object for the 3D analysis of wings and planes
 *
 */

#pragma once

/**
*@brief
*	This class defines the polar object used in 2D and 3D calculations
*
    The class stores both the analysis parameters and the analysis results.

    Each instance of this class is uniquely associated to an instance of a Wing or a Plane object.
    The data is stored in International Standard Units, i.e. meters, seconds, kg, and Newtons.
    Angular data is stored in degrees
*/
#include <QVector>
#include <QColor>



#include <xflcore/enums_objects.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects3d/analysis/aeroforces.h>
#include <xflobjects/objects3d/analysis/eigenvalues.h>
#include <xflobjects/objects3d/analysis/polar3d.h>
#include <xflobjects/objects_globals/anglecontrol.h>
#include <xflobjects/objects_globals/ctrlrange.h>

#include <complex>


class Plane;
class Fuse;
class WingOpp;
class PlaneOpp;


class WPolar : public Polar3d
{        

    public:
        enum enumFuseDragMethod {KARMANSCHOENHERR, PRANDTLSCHLICHTING, MANUALFUSECF};

    public:
        WPolar();
        virtual ~WPolar() = default;

        virtual void duplicateSpec(const Polar3d *pPolar3d) override;
        void clearWPolarData();

        void setDefaultSpec(const Plane *pPlane);

        bool bViscousLoop() const {return m_bViscLoop;}
        void setViscousLoop(bool b) {m_bViscLoop=b;}

        void addPlaneOpPointData(PlaneOpp const *pPOpp);
        void replacePOppDataAt(int pos, PlaneOpp const*pPOpp);
        void insertPOppDataAt(int pos, const PlaneOpp *pPOpp);
        void insertDataAt(int pos, double Alpha, double Beta, double Phi, double QInf, double Ctrl, double Cb, double XNP);
        void calculatePoint(int iPt);
        virtual void copy(WPolar const *pWPolar);

        double variable(int iVariable, int index) const;
        virtual double getVariable(int iVar, int index) const;
        virtual void setData(int iVariable, int index, double value);

        virtual void insertDataPointAt(int index, bool bAfter);
        virtual void removeAt(int index);
        void removeAoA(double alphaT4);


        bool serializeWPlrXFL(QDataStream &ar, bool bIsStoring);
        virtual bool serializeFl5v726(QDataStream &ar, bool bIsStoring) override;
        virtual bool serializeFl5v750(QDataStream &ar, bool bIsStoring) override;

        void setPlane(Plane const *pPlane) {m_pPlane = pPlane;}

        QString const &planeName()  const {return m_PlaneName;}      /**< returns the name of the polar's parent object as a QString object. */
        void setPlaneName(QString const &planename) {m_PlaneName = planename;}

        // Flap angle controls for linear polars T123578
        QString flapCtrlsSetName() const;
        bool hasActiveFlap() const; /** returns true if one flap at least is non-zero */
        void clearFlapCtrls() {m_FlapControls.clear();}
        void resetFlapCtrls();
        void setFlapCtrls(QVector<AngleControl> const &anglectrls) {m_FlapControls=anglectrls;}
        void resizeFlapCtrls(PlaneXfl const*pPlaneXfl);
        void resizeFlapCtrls(int iWing, int nFlaps) {m_FlapControls[iWing].resizeValues(nFlaps);}
        int nFlapCtrls() const {return m_FlapControls.size();}
        int nFlapCtrls(int iWing) const {return m_FlapControls.at(iWing).nValues();}
        AngleControl &addFlapCtrl() {m_FlapControls.append(AngleControl()); return m_FlapControls.last();}
        AngleControl &addFlapCtrl(AngleControl const &angle) {m_FlapControls.append(angle); ; return m_FlapControls.last();}
        QVector<AngleControl> &flapCtrls() {return m_FlapControls;}
        AngleControl & flapCtrls(int iWing) {return m_FlapControls[iWing];}
        AngleControl const & flapCtrls(int iWing) const {return m_FlapControls.at(iWing);}
        double flapAngleValue(int iWing, int iFlap) const;
        void setFlapAngleValue(int iWing, int iFlap, double g);
        bool checkFlaps(PlaneXfl const*pPlaneXfl, QString &log) const;

        void clearAVLCtrls() {m_AVLControls.clear();}
        int nAVLControls() const {return m_AVLControls.size();}
        void addAVLControl() {m_AVLControls.append(AngleControl());}
        void addAVLControl(AngleControl const &avlc) {m_AVLControls.append(avlc);}
        void insertAVLControl(int ic, AngleControl const &avlc) {m_AVLControls.insert(ic, avlc);}
        void removeAVLControl(int ic) {if(ic>=0 && ic<m_AVLControls.size()) m_AVLControls.removeAt(ic);}

        AngleControl &AVLCtrl(int ic) {return m_AVLControls[ic];}
        AngleControl const &AVLCtrl(int ic) const {return m_AVLControls[ic];}
        QString AVLCtrlName(int ic) const {if(ic>=0 && ic<m_AVLControls.size()) return m_AVLControls.at(ic).name(); else return QString();}
        double AVLGain(int iAVLCtrl, int iCtrlSurf) const {if(iAVLCtrl>=0 && iAVLCtrl<m_AVLControls.size()) return m_AVLControls.at(iAVLCtrl).value(iCtrlSurf); else return 0.0;}
        void setGain(int iAVLCtrl, int iCtrlSurf, double g) {if(iAVLCtrl>=0 && iAVLCtrl<m_AVLControls.size()) m_AVLControls[iAVLCtrl].setValue(iCtrlSurf, g); }

        bool hasActiveAVLControl() const; /** returns true if one control at least is active */

        void retrieveInertia(Plane const *pPlane);

        void clearAngleRangeList();
        CtrlRange angleRange(int iWing, int iCtrl) const;
        int nAngleRangeCtrls() const;
        void resetAngleRanges(Plane const*pPlane);

        double referenceArea()  const {return m_RefArea;}
        void setReferenceArea(double area) {m_RefArea=area;}

        double referenceSpanLength()  const {return m_RefSpan;}
        void setReferenceSpanLength(double length) {m_RefSpan=length;}

        double referenceChordLength() const override {return m_RefChord;}
        void setReferenceChordLength(double length) override {m_RefChord=length;}

        double velocity() const {return m_QInfSpec;}
        void setVelocity(double QInf) {m_QInfSpec=QInf;}

        double alphaSpec() const {return m_AlphaSpec;}
        void setAlphaSpec(double aoa) {m_AlphaSpec=aoa;}

        bool isAdjustedVelocity() const {return m_bAdjustedVelocity;}
        void setAdjustedVelocity(bool bAdjusted) {m_bAdjustedVelocity = bAdjusted;}

        virtual int dataSize() const {return m_Alpha.size();}
        virtual void resizeData(int newsize);

        virtual bool hasPoints() const {return m_Alpha.size()>0;}

        void makeDataArrays();

        void getProperties(QString &PolarProps, const Plane *pPlane,
                           double lenunit, double massunit, double speedunit, double areaunit,
                           QString const &lenlab, const QString &masslab, QString const &speedlab, QString const &arealab) const;

        void getWPolarData(QString &polardata, QString const & sep, double speedunit, QString speedlab) const;

        bool hasPOpp(PlaneOpp const *pPOpp) const;

        double TrefftzDistance()  const override {return m_RefChord * m_TotalWakeLengthFactor;}
        double wakeLength()       const override {return m_RefChord * m_TotalWakeLengthFactor;}
        double bufferWakeLength() const override {return m_RefChord * m_BufferWakeFactor;}

        std::complex<double> eigenValue(int iMode, int index) const;

        double Cl32Cd(int index) const;
        double Vx(int index) const;
        double Vz(int index) const;

        double extraDragForce(int index) const;

        bool bFuseMi() const {return m_bFuseMi;}
        void setIncludeFuseMi(bool bMi) {m_bFuseMi=bMi;}

        bool bWingTipMi() const {return m_bWingTipMi;}
        void setIncludeWingTipMi(bool bMi) { m_bWingTipMi=bMi;}

        bool hasFuseDrag() const {return m_bFuseDrag;}
        void setIncludeFuseDrag(bool bFuseDrag) {m_bFuseDrag=bFuseDrag;}

        WPolar::enumFuseDragMethod fuseDragMethod() const {return m_FuseDragMethod;}
        void setFuseDragMethod(WPolar::enumFuseDragMethod method) {m_FuseDragMethod=method;}

        double customFuseCf() const {return m_FuseCf;}
        void setCustomFuseCf(double cf) {m_FuseCf=cf;}

        double fuseDrag(const Fuse *pFuse, double QInf) const;
        double fuseDragCoef(double Reynolds) const;
        double KarmanSchoenherrCoef(double Re) const;
        double PrandtlSchlichtingCoef(double Re) const;

        bool bThinSurfaces()  const   {return m_bThinSurfaces;}  /**< returns true if the analysis is using thin surfaces, i.e. VLM, false if 3D Panels for the Wing objects. */
        void setThinSurfaces(bool bThin) {m_bThinSurfaces=bThin;}

        bool bThickSurfaces()  const   {return !m_bThinSurfaces;}
        void setThickSurfaces(bool bThin) {m_bThinSurfaces=!bThin;}

        bool bIncludeOtherWingAreas() const {return m_bOtherWingsArea;}
        void setIncludeOtherWingAreas(bool bInclude) {m_bOtherWingsArea=bInclude;}

        double extraDragTotal(double CL) const override;
        void recalcExtraDrag();

        double aoaCtrl(double ctrl) const;
        double betaCtrl(double ctrl) const;
        double phiCtrl(double ctrl) const;
        double QInfCtrl(double ctrl) const;
        double massCtrl(double ctrl) const;
        Vector3d CoGCtrl(double ctrl) const;

        AeroForces const &aeroForce(int index) const {return m_AF.at(index);}

        static QStringList const &variableNames() {return s_VariableNames;}
        static void setVariableNames(const QString &strLength, const QString &strSpeed, QString strMass, const QString &strForce, const QString &strMoment);
        static QString variableName(int iVar) {return s_VariableNames.at(iVar);}
        static int variableCount() {return s_VariableNames.size();}

        // debug
        void listVariable(int iVar);

    protected:

        Plane const *m_pPlane;
        QString  m_PlaneName;          /**< the name of the parent wing or plane */


        double m_RefArea;          /**< The reference area for the calculation of aero coefficients */
        double m_RefChord;   /**< The reference length = the mean aero chord, for the calculation of aero coefficients */
        double m_RefSpan;    /**< The reference span for the calculation of aero coefficients */
        bool m_bOtherWingsArea;  /**< If true, the area of the second and other wings will be included in the reference area */

        WPolar::enumFuseDragMethod m_FuseDragMethod;
        double m_FuseCf;

        bool m_bWingTipMi;           /**< true if the contribution of inviscid pressure forces acting on the wing tip panels should be included in the calculation of moments*/

        bool m_bFuseMi;            /**< true if the contribution of inviscid pressure forces acting on the fuselage panels should be included in the calculation of moments*/
        bool m_bFuseDrag;          /**< true if fuselage friction drag should be included in the analysis */

        bool m_bThinSurfaces;      /**< true if VLM, false if 3D-panels */

        bool m_bViscLoop;

    public:
        bool m_bAdjustedVelocity;  /** for a control polar, this flag determins if the velocity is calculated to balance the weight */

        QVector<AngleControl> m_FlapControls;      /**< T123578 polar: list of flap angles for each wing */
        QVector<AngleControl> m_AVLControls;

        QVector<CtrlRange> m_OperatingRange;        /**< the operating range for a control polar: Velocity, alpha, beta, phi*/
        QVector<CtrlRange> m_InertiaRange;          /**< the range of inertia parameters for a control polar: mass, CoG.x, CoG.z */
        QVector<QVector<CtrlRange>> m_AngleRange;   /**< the range of angle parameters for a control polar */

        double   m_AlphaSpec;          /**< the angle of attack for type 4 & 5 polars */
        double   m_QInfSpec;           /**< the freestream velocity for type 1 & 5 polars */


        QVector<double>  m_Alpha;      /**< the angle of attack */
        QVector<double>  m_Ctrl;       /**< Ctrl variable */
        QVector<double>  m_Beta;       /**< the sideslip angle */
        QVector<double>  m_Phi;       /**< the bank angle */
        QVector<double>  m_QInfinite;  /**< the free stream speed - type 2 WPolars */

        QVector<AeroForces> m_AF;

        QVector<double>  m_XNP;        /**< the position of the neutral point, as a result of stability analysis only */

        QVector<double>  m_Mass_var;   /**< the mass calculated as ref_mass + gain*control */
        QVector<double>  m_CoG_x;      /**< the CoG position calculated as ref_CoG_x + gain*control */
        QVector<double>  m_CoG_z;      /**< the CoG position calculated as ref_CoG_z + gain*control */

        QVector<double>  m_MaxBending; /**< the max bending moment at the root chord */

        QVector<EigenValues>  m_EV;        /**< the data structiure of eignevalues and related variables */


        double m_XNeutralPoint;       /**< Neutral point position, calculated from d(XCp.Cl)/dCl >*/


        static QStringList s_VariableNames;
};

