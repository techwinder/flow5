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

#include <vector>

#include <task3d.h>
#include <t8opp.h>
#include <aeroforces.h>
#include <spandistribs.h>
#include <stabderivatives.h>
#include <vorton.h>

class Plane;
class PlaneXfl;
class WingXfl;
class PlanePolar;
class PlaneOpp;
class Node;
class Surface;
class Panel3;
class Panel4;
class AngleControl;
class Polar;
class XFoilTask;


class FL5LIB_EXPORT PlaneTask : public Task3d
{
    public:
        PlaneTask();
        ~PlaneTask();

        void cancelTask() override;

        void setOppList(const std::vector<double> &opplist);
        void setCtrlOppList(const std::vector<double> &opplist);
        void setStabOppList(const std::vector<double> &opplist);
        void setT8OppList(const std::vector<T8Opp> &ranges);

        double ctrl() const {return m_Ctrl;}
        double aoa()  const {return m_Alpha;}

        void setObjects(Plane *pPlane, PlanePolar *pWPolar);
        std::vector<PlaneOpp*> const & planeOppList() const {return m_PlaneOppList;}

        Plane *plane()  const {return m_pPlane;}
        PlanePolar*wPolar() const {return m_pPlPolar;}
        void loop() override;
        bool initializeTask();

        void getVelocityVector(Vector3d const &C, double coreradius, bool bMultiThread, Vector3d &velocity) const;

        void setComputeDerivatives(bool b) {m_bDerivatives=b;}

        void run() override;

        static void setViscousLoopSettings(bool bInitVTwist, double relaxfactor, double alphaprec, int maxiters);
        static void setMaxViscIter(int n) {s_ViscMaxIter=n;}
        static void setMaxViscError(double err) {s_ViscAlphaPrecision=err;}
        static void setViscRelaxFactor(double r) {s_ViscRelax=r;}
        static int maxViscIter() {return s_ViscMaxIter;}
        static double maxViscError() {return s_ViscAlphaPrecision;}
        static double viscRelaxFactor() {return s_ViscRelax;}
        static bool bViscInitVTwist() {return s_bViscInitTwist;}
        static void setViscInitVTwist(bool bInit) {s_bViscInitTwist=bInit;}

    private:
        bool T123458Loop();
        bool T6Loop();
        bool T7Loop();

        void allocatePlaneResultArrays();
        void outputStateMatrices(PlaneOpp const *pPOpp);
        bool checkWPolarData(const Plane *pPlane, PlanePolar *pWPolar);
        double computeBalanceSpeeds(double Alpha, double mass, bool &bWarning, const std::string &prefix, std::string &log);
        double computeGlideSpeed(double Alpha, double mass, std::string &log);
        void computeControlDerivatives(double t7ctrl, double alphaeq, double u0, StabDerivatives &SD);
        void outputNDStabDerivatives(double u0, const StabDerivatives &SD);
        PlaneOpp *computePlane(double ctrl, double Alpha, double Beta, double phi, double QInf, double mass, const Vector3d &CoG, bool bInGeomAxes);
        void computeInviscidAero(const std::vector<Panel3> &panel3, const double *Cp3Vtx, const PlanePolar *pWPolar, double Alpha, AeroForces &AF) const;
        void computeInducedForces(double alpha, double beta, double QInf);
        void computeInducedDrag(double alpha, double beta, double QInf);
        bool computeViscousDrag(WingXfl *pWing, double alpha, double beta, double QInf, const PlanePolar *pWPolar, Vector3d const &cog, int iStation0, SpanDistribs &SpanResFF, std::string &logmsg) const;
        bool computeViscousDragOTF(WingXfl *pWing, double alpha, double beta, double QInf, const PlanePolar *pWPolar, Vector3d const &cog, const AngleControl &TEFlapAngles, SpanDistribs &SpanResFF, std::string &logmsg);
        bool computeSurfaceDragOTF(Surface const &surf, int iStartStation, double theta, SpanDistribs &spandist);
        bool computeSectionDragOTF(XFoilTask *pTask) const;

        PlaneOpp *createPlaneOpp(double ctrl, double alpha, double beta, double phi, double QInf, double mass, const Vector3d &CoG, const double *Cp, const double *Gamma, const double *Sigma, bool bCpOnly=false) const;
        void addTwistedVelField(double Qinf, double alpha, std::vector<Vector3d> &VField) const;

        void scaleResultsToSpeed(double vOld, double vNew);

        void setControlPositions(PlaneXfl const*pPlaneXfl, PlanePolar const*pWPolar, std::vector<Panel4> &panel4, double deltactrl, int iAVLCtrl, std::string &outstring);
        void setControlPositions(PlaneXfl const *pPlaneXfl, PlanePolar const *pWPolar, std::vector<Panel3> &panel3, const std::vector<Node> &refnodes, double deltactrl, int iAVLCtrl, std::string &outstring);

        void makeVortonRow(int qrhs) override;

        bool updateVirtualTwist(double QInf, double &error, std::string &log);

        bool setLinearSolution();

        bool computeStability(PlaneOpp *pPOpp, bool bOutput);

        void storePOpp(PlaneOpp *pPOpp);

    private:

        Plane *m_pPlane;
        PlanePolar *m_pPlPolar;
        std::vector<PlaneOpp*> m_PlaneOppList;

        bool m_bDerivatives;       /**< if true, computes the eigenthings when running a T123458 polar */

        double m_Ctrl;             /**< the oppoint currently calculated */
        double m_Alpha;            /**< the aoa currently calculated */
        double m_Beta;             /**< the current sideslip */
        double m_Phi;              /**< the current bank angle*/
        double m_QInf;             /**< the current freestream velocity */

        std::vector<double> m_AngleList, m_T6CtrlList, m_T7CtrlList;   /**< The list of operating points to analyze for each polar type*/
        std::vector<T8Opp> m_T8Opps;

        std::vector<SpanDistribs> m_SpanDistFF; /**< the array of span distributions of the wings */
        std::vector<Vector3d> m_WingForce;   /**< The array of calculated resulting forces acting on the wings in wind axis (N/q) */

        Vector3d m_Force0;  /** The calculated equilibrium force  @todo check body or wind axis*/
        Vector3d m_Moment0; /** The calculated equilibrium moment @todo check body or wind axis */

        // temp variables used to create the operating point
        AeroForces m_AF;               /** the overall aero forces acting on the plane */
        std::vector<AeroForces> m_PartAF;  /** the array of Aero forces acting on each part, for each operating point */

        std::vector<double> m_gamma;    /**< the virtual twist angle for each span section; cf. Computationally Efficient Transonic and Viscous Potential Flow Aero-Structural Method for Rapid Multidisciplinary Design Optimization of Aeroelastic Wing Shaping Control, by Eric Ting and Daniel Chaparro,  Advanced Modeling and Simulation (AMS) Seminar Series, Advanced Advanced Air Air Vehicles Transport Program Technology Project NASA Ames Research Center, June 28, 2017 */

    private:
        static bool s_bViscInitTwist;
        static double s_ViscRelax;
        static double s_ViscAlphaPrecision;
        static int s_ViscMaxIter;

};




