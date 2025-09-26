/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

/** @class Data and methods common to Plane and Boat Polars */

#pragma once

#include <xflgeom/geom2d/splines/bspline.h>
#include <xflobjects/objects3d/analysis/extradrag.h>
#include <xflobjects/objects_globals/objects_global.h>
#include <xflcore/xflobject.h>

class Inertia;

class Polar3d : public XflObject
{
    friend class BoatOpp;
    friend class XflXmlReader;
    friend class XmlWPolarReader;

    public:
        enum enumAnalysisMethod {LLT, VLM1, VLM2, QUADS, TRILINEAR, TRIUNIFORM, NOMETHOD};
        enum enumBC {DIRICHLET, NEUMANN};
        enum enumRefDimension {PLANFORM, PROJECTED, CUSTOM, AUTODIMS}; // AUTO is for sails

    public:
        Polar3d();
        virtual ~Polar3d() = default;

    public:

        virtual void duplicateSpec(Polar3d const *pPolar3d);

        virtual bool serializeFl5v726(QDataStream &ar, bool bIsStoring);
        virtual bool serializeFl5v750(QDataStream &ar, bool bIsStoring);
        virtual double TrefftzDistance() const = 0;

        virtual double windFactor(double x, double y, double z) const {(void)x; (void)y; (void)z; return 1.0;}  // overriden by WindGradient of BoatPolars
        virtual double windFactor(Vector3d const &position) const {return windFactor(position.x, position.y, position.z);}

        virtual void setReferenceChordLength(double length) = 0;
        virtual double referenceChordLength() const = 0;

        int polarFormat() const {return m_PolarFormat;}

        void lock()   {m_bLocked=true;}
        void unlock() {m_bLocked=false;}
        bool isLocked() const {return m_bLocked;}

        bool isLLTMethod()        const  {return m_AnalysisMethod==Polar3d::LLT;}
        bool isVLM()              const  {return isVLM1() || isVLM2();}
        bool isVLM1()             const  {return m_AnalysisMethod==Polar3d::VLM1;}
        bool isVLM2()             const  {return m_AnalysisMethod==Polar3d::VLM2;}
        bool isPanel4Method()     const  {return m_AnalysisMethod==Polar3d::QUADS;}
        bool isQuadMethod()       const  {return isPanel4Method() || isVLM();}
        bool isTriUniformMethod() const  {return m_AnalysisMethod==Polar3d::TRIUNIFORM;}
        bool isTriLinearMethod()  const  {return m_AnalysisMethod==Polar3d::TRILINEAR;}
        bool isTriangleMethod()   const  {return isTriUniformMethod() || isTriLinearMethod();}
        bool isPanelMethod()      const  {return isQuadMethod() || isTriangleMethod();}

        void setVLM1()  {m_AnalysisMethod=Polar3d::VLM1;}
        void setVLM2()  {m_AnalysisMethod=Polar3d::VLM2;}

        xfl::enumPolarType type() const {return m_Type;}       /**< returns the type of the polar as an index in the enumeration. */
        void setType(xfl::enumPolarType type) {m_Type=type;}

        Polar3d::enumAnalysisMethod analysisMethod() const {return m_AnalysisMethod;}   /**< returns the analysis method of the polar as an index in the enumeration. */
        void setAnalysisMethod(Polar3d::enumAnalysisMethod method) {m_AnalysisMethod=method;}

        Polar3d::enumRefDimension referenceDim() const {return m_ReferenceDim;}
        void setReferenceDim(Polar3d::enumRefDimension dim) {m_ReferenceDim=dim;}
        bool bProjectedDim() const {return m_ReferenceDim==Polar3d::PROJECTED;}
        bool bPlanformDim()  const {return m_ReferenceDim==Polar3d::PLANFORM;}


        bool   bVortonWake()      const {return m_bVortonWake;}
        double vortonL0()         const {return m_VortonL0;}
        double bufferWakeFactor() const {return m_BufferWakeFactor;}
        virtual double bufferWakeLength() const  = 0;
        double vortonCoreSize()   const {return m_VortonCoreSize;}
        double VPWMaxLength()     const {return m_VPWMaxLength;}
        int VPWIterations()       const {return m_VPWIterations;}

        void setVortonWake(bool bVW)       {m_bVortonWake = bVW;}
        void setVortonL0(double l0)        {m_VortonL0=l0;}
        void setBufferWakeFactor(double l) {m_BufferWakeFactor=l;}
        void setVortonCoreSize(double l)   {m_VortonCoreSize=l;}
        void setVPWMaxLength(double l)     {m_VPWMaxLength=l;}
        void setVPWIterations(int n)       {m_VPWIterations=n;}

        int NXBufferWakePanels() const;

        void setNXWakePanel4(int nx) {m_nXWakePanel4=nx;}
        int NXWakePanel4()       const  {return m_nXWakePanel4;}
        void setTotalWakeLengthFactor(double f) {m_TotalWakeLengthFactor=f;}
        double totalWakeLengthFactor() const  {return m_TotalWakeLengthFactor;}
        void setWakePanelFactor(double factor) {m_WakePanelFactor=factor;}
        double wakePanelFactor() const  {return m_WakePanelFactor;}

        virtual double wakeLength() const = 0;

        bool bTrefftz() const {return m_bTrefftz;}
        void setTrefftz(bool bInFFPlane) {m_bTrefftz=bInFFPlane;}

        Polar3d::enumBC boundaryCondition() const {return m_BC;}
        void setBoundaryCondition(Polar3d::enumBC bc) {m_BC=bc;}
        bool bDirichlet() const {return m_BC==Polar3d::DIRICHLET;}
        bool bNeumann()   const {return m_BC==Polar3d::NEUMANN;}

        bool isViscous() const {return m_bViscous;}
        void setViscous(bool bViscous) {m_bViscous = bViscous;}

        bool isViscInterpolated() const {return !m_bViscOnTheFly;}
        void setViscInterpolated(bool b) {m_bViscOnTheFly=!b;}

        bool isViscOnTheFly() const {return m_bViscOnTheFly;}
        void setViscOnTheFly(bool b) {m_bViscOnTheFly=b;}

        bool isViscFromCl() const {return m_bViscFromCl;}
        void setViscFromCl(bool bFromCl) {m_bViscFromCl=bFromCl;}

        double density()      const  {return m_Density;}        /**< returns the fluid's density, in IS units. */
        void setDensity(double dens) {m_Density=dens;}

        double viscosity()    const    {return m_Viscosity;}      /**< returns the fluid's kinematic viscosity, in IS units. */
        void setViscosity(double visc) {m_Viscosity=visc;}

        bool bIgnoreBodyPanels() const {return m_bIgnoreBodyPanels;}
        void setIgnoreBodyPanels(bool bIgnore) {m_bIgnoreBodyPanels=bIgnore;}

        double NCrit() const {return m_NCrit;}
        void setNCrit(double ncrit) {m_NCrit = ncrit;}

        double XTrTop() const {return m_XTrTop;}
        void setXTrTop(double xtop) {m_XTrTop=xtop;}

        double XTrBot() const {return m_XTrBot;}
        void setXTrBot(double xbot) {m_XTrBot=xbot;}

        bool bHPlane() const {return m_bGround || m_bFreeSurface;}
        bool bGroundEffect() const {return m_bGround;}
        void setGroundEffect(bool bGround) {m_bGround = bGround;}

        bool bFreeSurfaceEffect() const {return m_bFreeSurface;}
        void setFreeSurfaceEffect(bool bFreeSurf) {m_bFreeSurface = bFreeSurf;}

        void setGroundHeight(double h) {m_GroundHeight=h;}
        double groundHeight() const {return m_GroundHeight;}

        bool bAutoInertia() const {return m_bAutoInertia;}
        void setAutoInertia(bool bAuto) {m_bAutoInertia=bAuto;}

        void setInertia(Inertia const &inertia);

        double const*inertia() const {return m_Inertia;}

        double Ixx() const {return m_Inertia[0];}
        double Iyy() const {return m_Inertia[1];}
        double Izz() const {return m_Inertia[2];}
        double Ixz() const {return m_Inertia[3];}

        void setInertiaTensor(double Ixx, double Iyy, double Izz, double Ixz) {m_Inertia[0]=Ixx; m_Inertia[1]=Iyy; m_Inertia[2]=Izz; m_Inertia[3]=Ixz;}
        void setIxx(double Ixx) {m_Inertia[0]=Ixx;}
        void setIyy(double Iyy) {m_Inertia[1]=Iyy;}
        void setIzz(double Izz) {m_Inertia[2]=Izz;}
        void setIxz(double Ixz) {m_Inertia[3]=Ixz;}

        bool isPlanePolar() const {return isType123() || isType5() || isType6() || isType7() || isType8();}
        bool isBoatPolar() const  {return m_Type==xfl::BOATPOLAR;}

        bool isType1()           const {return m_Type==xfl::T1POLAR;}   /**< returns true if the polar is of the FIXEDSPEEDPOLAR type, false otherwise >*/
        bool isType2()           const {return m_Type==xfl::T2POLAR;}    /**< returns true if the polar is of the FIXEDLIFTPOLAR type, false otherwise >*/
        bool isType3()           const {return m_Type==xfl::T3POLAR;}        /**< returns true if the polar is of the GLIDEPOLAR type, false otherwise >*/
        bool isType4()           const {return m_Type==xfl::T4POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isType5()           const {return m_Type==xfl::T5POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isType6()           const {return m_Type==xfl::T6POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isType7()           const {return m_Type==xfl::T7POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isType8()           const {return m_Type==xfl::T8POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isType123()         const {return isType1() || isType2() || isType3();}
        bool isType12358()       const {return isType123() || isType5() || isType8();}
        bool isFixedSpeedPolar() const {return m_Type==xfl::T1POLAR;}   /**< returns true if the polar is of the FIXEDSPEEDPOLAR type, false otherwise >*/
        bool isFixedLiftPolar()  const {return m_Type==xfl::T2POLAR;}    /**< returns true if the polar is of the FIXEDLIFTPOLAR type, false otherwise >*/
        bool isGlidePolar()      const {return m_Type==xfl::T3POLAR;}        /**< returns true if the polar is of the GLIDEPOLAR type, false otherwise >*/
        bool isFixedaoaPolar()   const {return m_Type==xfl::T4POLAR;}     /**< returns true if the polar is of the FIXEDAOAPOLAR type, false otherwise >*/
        bool isBetaPolar()       const {return m_Type==xfl::T5POLAR;}         /**< returns true if the polar is of the BETAPOLAR type, false otherwise >*/
        bool isControlPolar()    const {return m_Type==xfl::T6POLAR;}      /**< returns true if the polar is of the CONTROLPOLAR type, false otherwise >*/
        bool isStabilityPolar()  const {return m_Type==xfl::T7POLAR;}    /**< returns true if the polar is of the STABILITYPOLAR type, false otherwise >*/
        bool isType8Polar()      const {return m_Type==xfl::T8POLAR;}
        bool isExternalPolar()   const {return m_Type==xfl::EXTERNALPOLAR;}

        double betaSpec()  const   {return m_BetaSpec;}
        void setBeta(double b) {m_BetaSpec=b;}

        double phi()   const   {return m_BankAngle;}
        void setPhi(double f) {m_BankAngle=f;}


        double mass() const {return m_Mass;}
        void setMass(double m) {m_Mass=m;}

        Vector3d const &CoG() const {return m_CoG;}
        void setCoG(Vector3d const &cog) {m_CoG=cog;}
        void setCoGx(double x) {m_CoG.x=x;}
        void setCoGy(double y) {m_CoG.y=y;}
        void setCoGz(double z) {m_CoG.z=z;}

        bool hasExtraDrag() const;
        void clearExtraDrag() {m_ExtraDrag.clear();}
        int extraDragCount() const {return m_ExtraDrag.size();}
        double constantDrag() const;
        virtual double extraDragTotal(double CL) const;
        ExtraDrag const &extraDrag(int idx) const {return m_ExtraDrag.at(idx);}
        ExtraDrag &extraDrag(int idx) {return m_ExtraDrag[idx];}
        void appendExtraDrag(ExtraDrag const &xdrag) {m_ExtraDrag.push_back(xdrag);}
        void appendExtraDrag(QString const &tag, double area, double coef) {m_ExtraDrag.push_back({tag, area, coef});}
        QVector<ExtraDrag> const &extraDragList() const {return m_ExtraDrag;}
        void setExtraDrag(QVector<ExtraDrag> const &extra) {m_ExtraDrag=extra;}

        bool bAVLDrag() const {return m_bAVLDrag;}
        double AVLDrag(double CL) const;
        void setAVLDrag(bool b, BSpline const &dragspline) {m_bAVLDrag=b; m_AVLSpline=dragspline;}

        BSpline &AVLSpline() {return m_AVLSpline;}
        BSpline const &AVLSpline() const {return m_AVLSpline;}

        static Polar3d::enumAnalysisMethod analysisMethod(QString const &strAnalysisMethod);
        static QString analysisMethod(Polar3d::enumAnalysisMethod analysisMethod);

        static Polar3d::enumBC boundaryCondition(QString const &strBC);
        static QString boundaryCondition(Polar3d::enumBC boundaryCondition);

        static QString referenceDimension(Polar3d::enumRefDimension refDimension);
        static Polar3d::enumRefDimension referenceDimension(const QString &strRefDimension);

    protected:
        int  m_PolarFormat;        /**< the identification number which references the format used to serialize the data */

        Polar3d::enumRefDimension m_ReferenceDim;        /**< Describes the origin of the refernce area : 1 if planform area, else projected area */

        bool m_bLocked;            /**< true if the instance of the object is currently used by a running analysis */
        Polar3d::enumBC m_BC;                         /**< The type of boundary conditions. UNUSED: Neuman BC is permanently disabled. */
        Polar3d::enumAnalysisMethod m_AnalysisMethod;  /**< The method used for the analysis. May be one of the following types : LLTMETHOD, VLMMETHOD, PANELMETHOD */
        xfl::enumPolarType m_Type;      /**< The type of analysis. May be one of the following types :FIXEDSPEEDPOLAR, FIXEDLIFTPOLAR, FIXEDAOAPOLAR, STABILITYPOLAR */

        bool     m_bGround;            /**< true if ground effect should be taken into account in the analysis */
        bool     m_bFreeSurface;       /**< true if a free surface effect should be taken into account in the analysis */
        double   m_GroundHeight;       /**< the height above the ground for a plane, 0 for a boat */

        bool     m_bIgnoreBodyPanels;  /**< true if the body panels should be ignored in the analysis */


        bool     m_bTrefftz;           /**< true if the lift and drag are evaluated in the Trefftz plane, false if evaluation is made by summation of panel forces */

        bool     m_bViscous;           /**< true if the analysis is viscous */
        bool     m_bViscOnTheFly;      /**< true if the viscous properties are interpolated on the 2d polar mesh, false if on the fly */
        bool     m_bViscFromCl;        /**< true if the viscous properties are interpolated from the lift coefficient, i.e. xflr5 method */
        double   m_NCrit;              /**< the 2d free transition parameter for on the fly viscous calculations */
        double   m_XTrTop;             /**< the 2d forced top transition location for on the fly viscous calculations; unit is (x/c) */
        double   m_XTrBot;             /**< the 2d forced bottom transition location for on the fly viscous calculations; unit is (x/c) */

        double   m_BetaSpec;           /**< The sideslip angle for type 1,2, 4 polars */
        double   m_BankAngle;          /**< The bank angle */

        double   m_Density;            /**< The fluid's density */
        double   m_Viscosity;          /**< The fluid's kinematic viscosity */

        int      m_nXWakePanel4;             /**< the number of quad wake panels in each streamwise column */
        double   m_WakePanelFactor;          /**< the ratio between the length of two wake panels in the x direction */
        double   m_TotalWakeLengthFactor;    /**< the wake's length/MAC; defines the position of the Trefftz plane */

        bool     m_bVortonWake;        /**< true if wake roll-up  should be taken into account in the analysis */
        double   m_VortonL0;           /**< the distance to the first row of vortons, in MAC units; i.e. d = L0 x MAC */
        double   m_BufferWakeFactor;   /**< the length of the buffer wake panels in MAC units */
        double   m_VortonCoreSize;     /**< the vorton's core size in meters used to calculate the mollification factor */
        double   m_VPWMaxLength;       /**< vortons further downstream than this length will be discarded - MAC units */
        int      m_VPWIterations;      /**< the number of VPW iterarions */

        bool     m_bAutoInertia;       /**< true if the inertia to be taken into account is the one of the parent plane */

        double m_Inertia[4];           /**< Ixx, Iyy, Izz, Ixz */

        Vector3d m_CoG;
        double m_Mass;

        QVector<ExtraDrag> m_ExtraDrag;

        bool m_bAVLDrag;
        mutable BSpline m_AVLSpline;


};

