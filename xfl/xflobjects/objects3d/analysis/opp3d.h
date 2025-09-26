/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#pragma once



#include <QColor>
#include <QDataStream>
#include <QVector>

#include <xflcore/enums_core.h>
#include <xflcore/enums_objects.h>
#include <xflcore/xflobject.h>
#include <xflobjects/objects3d/analysis/aeroforces.h>
#include <xflobjects/objects3d/analysis/polar3d.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflpanels/panels/vorton.h>
#include <xflpanels/panels/vortex.h>


class Opp3d : public XflObject
{
    friend class XPlane;
    friend class WPolar;
    friend class gl3dXPlaneView;
    friend class gl3dXSailView;
    friend class CrossFlowCtrls;
    friend class VortonTestDlg;
    friend class OptimCp3d;
    friend class gl3dOptimXflView;
    friend class POpp3dCtrls;

    public:
        Opp3d();

        void setAnalysisMethod(Polar3d::enumAnalysisMethod method) {m_AnalysisMethod=method;}
        Polar3d::enumAnalysisMethod analysisMethod() const {return m_AnalysisMethod;}
        bool isLLTMethod()        const {return m_AnalysisMethod==Polar3d::LLT;}
        bool isVLMMethod()        const {return isVLM1() || isVLM2();}
        bool isVLM1()             const {return m_AnalysisMethod==Polar3d::VLM1;}
        bool isVLM2()             const {return m_AnalysisMethod==Polar3d::VLM2;}
        bool isPanel4Method()     const {return m_AnalysisMethod==Polar3d::QUADS;}
        bool isQuadMethod()       const {return isPanel4Method() || isVLMMethod();}
        bool isTriUniformMethod() const {return m_AnalysisMethod==Polar3d::TRIUNIFORM;}
        bool isTriLinearMethod()  const {return m_AnalysisMethod==Polar3d::TRILINEAR;}
        bool isTriangleMethod()   const {return isTriUniformMethod() || isTriLinearMethod();}
        bool isPanelMethod()      const {return isPanel4Method() || isTriangleMethod();}

        QVector<double> &gamma() {return m_gamma;}
        QVector<double> &sigma() {return m_sigma;}
        QVector<double> &Cp()    {return m_Cp;}
        QVector<double> const &gamma() const {return m_gamma;}
        QVector<double> const &sigma() const {return m_sigma;}
        QVector<double> const &Cp()    const {return m_Cp;}
        double gamma(int index) const {return m_gamma.at(index);}
        double sigma(int index) const {return m_sigma.at(index);}
        double Cp(int index)    const {return m_Cp.at(index);}

        int nPanel4() const {return m_nPanel4;}
        int nPanel3() const {return m_nPanel3;}

        bool bHPlane() const {return m_bGround || m_bFreeSurface;}
        bool bGround() const {return m_bGround;}
        void setGroundEffect(bool b) {m_bGround=b;}
        double groundHeight() const {return m_GroundHeight;}
        void setGroundHeight(double h) {m_GroundHeight=h;}

        int vortonRows() const {return m_Vorton.count();}
        int vortonCount() const;
        bool hasVortons() const {return m_Vorton.size()>0;}
        void getVortonVelocity(Vector3d const &pt, double CoreSize, Vector3d &V) const;
        QVector<Vector3d> vortonLines() const;
        void setVortons(QVector<QVector<Vorton>> const &vortons) {m_Vorton=vortons;}
        void setVortexNeg(QVector<Vortex> const &vortexNeg) {m_VortexNeg=vortexNeg;}

        double nodeValue(int index) const {if(index>=0 && index<m_NodeValue.size()) return m_NodeValue.at(index); else return 0.0;}
        QVector<double> &nodeValues() {return m_NodeValue;}

        double nodeValMin() const {return m_NodeValMin;}
        double nodeValMax() const {return m_NodeValMax;}
        void setNodeValRange(double min, double max) {m_NodeValMin=min; m_NodeValMax=max;}

        AeroForces const &aeroForces() const {return m_AF;}
        AeroForces &aeroForces() {return m_AF;}
        void setAeroForces(AeroForces const &ac) {m_AF=ac;}

        void setQInf(double v) {m_QInf=v;}
        double QInf() const {return m_QInf;}

        double alpha() const {return m_Alpha;}
        void setAlpha(double alfa) {m_Alpha=alfa;}

        double beta() const {return m_Beta;}
        void setBeta(double b) {m_Beta=b;}

        double phi() const {return m_Phi;}
        void setPhi(double f) {m_Phi=f;}

        double Ry() const {return m_Ry;}
        void setRy(double f) {m_Ry=f;}

        double ctrl() const {return m_Ctrl;}
        void setCtrl(double c) {m_Ctrl=c;}

        bool bThinSurfaces()  const   {return m_bThinSurface;}  /**< returns true if the analysis is using thin surfaces, i.e. VLM, false if 3D Panels for the Wing objects. */
        void setThinSurfaces(bool bThin) {m_bThinSurface=bThin;}

        bool bThickSurfaces()  const   {return !m_bThinSurface;}
        void setThickSurfaces(bool bThin) {m_bThinSurface=!bThin;}

        virtual QString title(bool bLong) const = 0;
        virtual QString const &polarName() const =0;
        virtual void setPolarName(QString const &name) = 0;

        static bool bStoreOpps3d()   {return s_bStoreOpps3d;}
        static void setStoreOpps3d(bool bStore) {s_bStoreOpps3d=bStore;}

    protected:
        bool m_bThinSurface;        /**< true if the WingOpp is the results of a calculation on the middle surface */
        Polar3d::enumAnalysisMethod m_AnalysisMethod;    /**< the analysis method of the parent polar */

        int m_nPanel4;                        /**< the number of VLM or 3D-panels */
        int m_nPanel3;                        /**< the number of triangle panels; not necessarily 2*nPanel4, some quads may be degenerate */

        double m_Alpha;            /**< the angle of attack*/
        double m_Beta;             /**< the sideslip angle */
        double m_Phi;              /**< the bank angle */
        double m_Ry;               /**< The rotation around the y-axis, special for windsurfs */
        double m_Ctrl;             /**< the value of the control variable */
        double m_QInf;

        QVector<double> m_Cp;           /**< pressure coeffs on panels */
        QVector<double> m_gamma;        /**< vortice or doublet strengths */
        QVector<double> m_sigma;        /**< source strengths */



        QVector<QVector<Vorton>> m_Vorton; /** The array of vorton rows. Vortons are organized in rows. Each row is located in a crossflow plane. The number of vortons is varable for each row, due to vortex stretching and vorton redistribution. */
        QVector<Vortex> m_VortexNeg;    /** The array of negating vortices at the trailing edge of the trailing wake panel of each wake column. cf. Willis 2005 fig. 3*/


        QVector<double> m_NodeValue;          /**< The array of values at nodes. Temoprary array are constructed on demand, depending on whether Cp or Gamma is selected for the 3d-view. Only used for linear triangular analysis. */
        double m_NodeValMin;                  /**< The min value of the array m_NodeValue. Temporary variable used for colour map plots */
        double m_NodeValMax;                  /**< The max value of the array m_NodeValue. Temporary variable used for colour map plots */
        AeroForces m_AF;                /**< The force acting on the boat, in (N/q) and (N.m/q) */

        bool m_bGround;
        bool m_bFreeSurface;
        double m_GroundHeight;

        static bool s_bStoreOpps3d;

};

