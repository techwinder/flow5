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

#include <string>


#include <aeroforces.h>
#include <anglecontrol.h>
#include <ctrlrange.h>
#include <eigenvalues.h>
#include <enums_objects.h>
#include <polar3d.h>
#include <vector3d.h>

#include <complex>


class Plane;
class Fuse;
class WingOpp;
class PlaneOpp;


class FL5LIB_EXPORT PlanePolar : public Polar3d
{        

    public:
        enum enumFuseDragMethod {KARMANSCHOENHERR, PRANDTLSCHLICHTING, MANUALFUSECF};

    public:
        PlanePolar();
        virtual ~PlanePolar() = default;

        virtual void duplicateSpec(const Polar3d *pPolar3d) override;
        void clearWPolarData();

        void setDefaults() override;
        void setDefaultSpec(const Plane *pPlane);

        bool bViscousLoop() const {return m_bViscLoop;}
        void setViscousLoop(bool b) {m_bViscLoop=b;}

        void addPlaneOpPointData(PlaneOpp const *pPOpp);
        void replacePOppDataAt(int pos, PlaneOpp const*pPOpp);
        void insertPOppDataAt(int pos, const PlaneOpp *pPOpp);
        void insertDataAt(int pos, double Alpha, double Beta, double Phi, double QInf, double Ctrl, double Cb, double XNP);
        void calculatePoint(int iPt);
        virtual void copy(PlanePolar const *pWPolar);

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

        std::string const &planeName()  const {return m_PlaneName;}      /**< returns the name of the polar's parent object as a QString object. */
        void setPlaneName(std::string const &planename) {m_PlaneName = planename;}

        // Flap angle controls for linear polars T123578
        std::string flapCtrlsSetName() const;
        bool hasActiveFlap() const; /** returns true if one flap at least is non-zero */
        void clearFlapCtrls() {m_FlapControls.clear();}
        void resetFlapCtrls();
        void setFlapCtrls(std::vector<AngleControl> const &anglectrls) {m_FlapControls=anglectrls;}
        void resizeFlapCtrls(PlaneXfl const*pPlaneXfl);
        void resizeFlapCtrls(int iWing, int nFlaps) {m_FlapControls[iWing].resizeValues(nFlaps);}
        int nFlapCtrls() const {return int(m_FlapControls.size());}
        int nFlapCtrls(int iWing) const {return m_FlapControls.at(iWing).nValues();}
        AngleControl &addFlapCtrl() {m_FlapControls.push_back(AngleControl()); return m_FlapControls.back();}
        AngleControl &addFlapCtrl(AngleControl const &angle) {m_FlapControls.push_back(angle); ; return m_FlapControls.back();}
        std::vector<AngleControl> &flapCtrls() {return m_FlapControls;}
        AngleControl & flapCtrls(int iWing) {return m_FlapControls[iWing];}
        AngleControl const & flapCtrls(int iWing) const {return m_FlapControls.at(iWing);}
        double flapAngleValue(int iWing, int iFlap) const;
        void setFlapAngleValue(int iWing, int iFlap, double g);
        bool checkFlaps(PlaneXfl const*pPlaneXfl, std::string &log) const;

        void clearAVLCtrls() {m_AVLControls.clear();}
        int nAVLCtrls() const {return int(m_AVLControls.size());}
        void addAVLControl() {m_AVLControls.push_back(AngleControl());}
        void addAVLControl(AngleControl const &avlc) {m_AVLControls.push_back(avlc);}
        void insertAVLControl(int ic, AngleControl const &avlc) {m_AVLControls.insert(m_AVLControls.begin()+ic, avlc);}
        void removeAVLControl(int ic) {if(ic>=0 && ic<int(m_AVLControls.size())) m_AVLControls.erase(m_AVLControls.begin()+ic);}
        bool hasActiveAVLControl() const; /** returns true if one control at least is active */
        AngleControl &AVLCtrl(int ic) {return m_AVLControls[ic];}
        AngleControl const &AVLCtrl(int ic) const {return m_AVLControls[ic];}
        std::string AVLCtrlName(int ic) const;
        double AVLGain(int iAVLCtrl, int iCtrlSurf) const;
        void setGain(int iAVLCtrl, int iCtrlSurf, double g);

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

        virtual int dataSize() const {return int(m_Alpha.size());}
        virtual void resizeData(int newsize);

        virtual bool hasPoints() const {return m_Alpha.size()>0;}

        void makeDataArrays();

        void getProperties(std::string &props, const Plane *pPlane) const;

        std::string exportToString(const std::string &separator) const;

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

        PlanePolar::enumFuseDragMethod fuseDragMethod() const {return m_FuseDragMethod;}
        void setFuseDragMethod(PlanePolar::enumFuseDragMethod method) {m_FuseDragMethod=method;}

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

        static std::vector<std::string> const &variableNames() {return s_VariableNames;}
        static void setVariableNames();
        static std::string variableName(int iVar) {return s_VariableNames.at(iVar);}
        static int variableCount() {return int(s_VariableNames.size());}

        // debug
        void listVariable(int iVar);

    protected:

        Plane const *m_pPlane;
        std::string m_PlaneName;          /**< the name of the parent wing or plane */

        double m_RefArea;          /**< The reference area for the calculation of aero coefficients */
        double m_RefChord;   /**< The reference length = the mean aero chord, for the calculation of aero coefficients */
        double m_RefSpan;    /**< The reference span for the calculation of aero coefficients */
        bool m_bOtherWingsArea;  /**< If true, the area of the second and other wings will be included in the reference area */

        PlanePolar::enumFuseDragMethod m_FuseDragMethod;
        double m_FuseCf;

        bool m_bWingTipMi;           /**< true if the contribution of inviscid pressure forces acting on the wing tip panels should be included in the calculation of moments*/

        bool m_bFuseMi;            /**< true if the contribution of inviscid pressure forces acting on the fuselage panels should be included in the calculation of moments*/
        bool m_bFuseDrag;          /**< true if fuselage friction drag should be included in the analysis */

        bool m_bThinSurfaces;      /**< true if VLM, false if 3D-panels */

        bool m_bViscLoop;
        bool m_bAdjustedVelocity;  /** for a control polar, this flag determins if the velocity is calculated to balance the weight */


        double   m_AlphaSpec;          /**< the angle of attack for type 4 & 5 polars */
        double   m_QInfSpec;           /**< the freestream velocity for type 1 & 5 polars */

        double m_XNeutralPoint;       /**< Neutral point position, calculated from d(XCp.Cl)/dCl >*/

    public:

        std::vector<AngleControl> m_FlapControls;      /**< T1234578 polar: list of preset flap angles for each wing */
        std::vector<AngleControl> m_AVLControls;       /**< T1234578 polar: list of flap gains for each wing */

        std::vector<CtrlRange> m_OperatingRange;        /**< the operating range for a control polar: Velocity, alpha, beta, phi*/
        std::vector<CtrlRange> m_InertiaRange;          /**< the range of inertia parameters for a control polar: mass, CoG.x, CoG.z */
        std::vector<std::vector<CtrlRange>> m_AngleRange;   /**< the range of angle parameters for a control polar */

        std::vector<double>  m_Alpha;      /**< the angle of attack */
        std::vector<double>  m_Ctrl;       /**< Ctrl variable */
        std::vector<double>  m_Beta;       /**< the sideslip angle */
        std::vector<double>  m_Phi;       /**< the bank angle */
        std::vector<double>  m_QInfinite;  /**< the free stream speed - type 2 WPolars */

        std::vector<AeroForces> m_AF;

        std::vector<double>  m_XNP;        /**< the position of the neutral point, as a result of stability analysis only */

        std::vector<double>  m_Mass_var;   /**< the mass calculated as ref_mass + gain*control */
        std::vector<double>  m_CoG_x;      /**< the CoG position calculated as ref_CoG_x + gain*control */
        std::vector<double>  m_CoG_z;      /**< the CoG position calculated as ref_CoG_z + gain*control */

        std::vector<double>  m_MaxBending; /**< the max bending moment at the root chord */

        std::vector<EigenValues>  m_EV;        /**< the data structiure of eignevalues and related variables */

        static std::vector<std::string> s_VariableNames;
};

