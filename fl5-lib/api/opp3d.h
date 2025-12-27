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



#pragma once




#include <QDataStream>
#include <vector>

#include <utils.h>
#include <enums_objects.h>
#include <aeroforces.h>
#include <polar3d.h>
#include <vector3d.h>
#include <vorton.h>
#include <vortex.h>


class FL5LIB_EXPORT Opp3d : public XflObject
{
    friend class  XPlane;
    friend class  WPolar;
    friend class  gl3dXPlaneView;
    friend class  gl3dXSailView;
    friend class  CrossFlowCtrls;
    friend class  VortonTestDlg;
    friend class  OptimCp3d;
    friend class  gl3dOptimXflView;
    friend class  POpp3dCtrls;

    public:
        Opp3d();

        void setAnalysisMethod(xfl::enumAnalysisMethod method) {m_AnalysisMethod=method;}
        xfl::enumAnalysisMethod analysisMethod() const {return m_AnalysisMethod;}
        bool isLLTMethod()        const {return m_AnalysisMethod==xfl::LLT;}
        bool isVLMMethod()        const {return isVLM1() || isVLM2();}
        bool isVLM1()             const {return m_AnalysisMethod==xfl::VLM1;}
        bool isVLM2()             const {return m_AnalysisMethod==xfl::VLM2;}
        bool isPanel4Method()     const {return m_AnalysisMethod==xfl::QUADS;}
        bool isQuadMethod()       const {return isPanel4Method() || isVLMMethod();}
        bool isTriUniformMethod() const {return m_AnalysisMethod==xfl::TRIUNIFORM;}
        bool isTriLinearMethod()  const {return m_AnalysisMethod==xfl::TRILINEAR;}
        bool isTriangleMethod()   const {return isTriUniformMethod() || isTriLinearMethod();}
        bool isPanelMethod()      const {return isPanel4Method() || isTriangleMethod();}

        std::vector<double> &gamma() {return m_gamma;}
        std::vector<double> &sigma() {return m_sigma;}
        std::vector<double> &Cp()    {return m_Cp;}
        std::vector<double> const &gamma() const {return m_gamma;}
        std::vector<double> const &sigma() const {return m_sigma;}
        std::vector<double> const &Cp()    const {return m_Cp;}
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

        int vortonRows() const {return int(m_Vorton.size());}
        int vortonCount() const;
        bool hasVortons() const {return m_Vorton.size()>0;}
        void getVortonVelocity(Vector3d const &pt, double CoreSize, Vector3d &V) const;
        std::vector<Vector3d> vortonLines() const;
        void setVortons(std::vector<std::vector<Vorton>> const &vortons) {m_Vorton=vortons;}
        void setVortexNeg(std::vector<Vortex> const &vortexNeg) {m_VortexNeg=vortexNeg;}

        double nodeValue(int index) const {if(index>=0 && index<int(m_NodeValue.size())) return m_NodeValue.at(index); else return 0.0;}
        std::vector<double> &nodeValues() {return m_NodeValue;}

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

        virtual std::string title(bool bLong) const = 0;
        virtual std::string const &polarName() const =0;
        virtual void setPolarName(std::string const &name) = 0;


    protected:
        bool m_bThinSurface;        /**< true if the WingOpp is the results of a calculation on the middle surface */
        xfl::enumAnalysisMethod m_AnalysisMethod;    /**< the analysis method of the parent polar */

        int m_nPanel4;                        /**< the number of VLM or 3D-panels */
        int m_nPanel3;                        /**< the number of triangle panels; not necessarily 2*nPanel4, some quads may be degenerate */

        double m_Alpha;            /**< the angle of attack*/
        double m_Beta;             /**< the sideslip angle */
        double m_Phi;              /**< the bank angle */
        double m_Ry;               /**< The rotation around the y-axis, special for windsurfs */
        double m_Ctrl;             /**< the value of the control variable */
        double m_QInf;

        std::vector<double> m_Cp;           /**< pressure coeffs on panels */
        std::vector<double> m_gamma;        /**< vortice or doublet strengths */
        std::vector<double> m_sigma;        /**< source strengths */



        std::vector<std::vector<Vorton>> m_Vorton; /** The array of vorton rows. Vortons are organized in rows. Each row is located in a crossflow plane. The number of vortons is varable for each row, due to vortex stretching and vorton redistribution. */
        std::vector<Vortex> m_VortexNeg;    /** The array of negating vortices at the trailing edge of the trailing wake panel of each wake column. cf. Willis 2005 fig. 3*/


        std::vector<double> m_NodeValue;          /**< The array of values at nodes. Temoprary array are constructed on demand, depending on whether Cp or Gamma is selected for the 3d-view. Only used for linear triangular analysis. */
        double m_NodeValMin;                  /**< The min value of the array m_NodeValue. Temporary variable used for colour map plots */
        double m_NodeValMax;                  /**< The max value of the array m_NodeValue. Temporary variable used for colour map plots */
        AeroForces m_AF;                /**< The force acting on the boat, in (N/q) and (N.m/q) */

        bool m_bGround;
        bool m_bFreeSurface;
        double m_GroundHeight;


};

