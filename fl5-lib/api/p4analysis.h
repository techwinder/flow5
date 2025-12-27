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



#include <aeroforces.h>
#include <panel4.h>
#include <panelanalysis.h>
#include <quadmesh.h>
#include <quadmesh.h>
#include <spandistribs.h>
#include <trimesh.h>
#include <vector3d.h>



#include <vector>


class Panel;
class Panel4;
class Polar3d;
class Surface;
class Polar3d;


class FL5LIB_EXPORT P4Analysis : public PanelAnalysis
{
    friend class  Task3d;
    friend class  PlaneTask;
    friend class  BoatTask;

    public:
        P4Analysis();
        ~P4Analysis() = default;

        bool initializeAnalysis(const Polar3d *pPolar3d, int nRHS) override;

        bool isQuadMethod()     const {return true;}
        bool isTriangleMethod() const {return false;}
        bool isTriUniMethod()   const {return false;}
        bool isTriLinMethod()   const {return false;}

        int nPanels() const override {return int(m_Panel4.size());}
        int nWakePanels() const {return int(m_WakePanel4.size());}
        int matSize() const override {return int(m_Panel4.size());}


        void makeStripAreas();

        void makeInfluenceMatrix() override;
        void makeMatrixBlock(int iBlock);

        void makeUnitRHSBlock(int iBlock) override;
        void makeRHSBlock(int iBlock, double *RHS, std::vector<Vector3d> const &VField, const Vector3d *normals) const override;

        void makeWakeMatrixBlock(int iBlock) override;

        void getDoubletPotential(Vector3d const &C, bool bSelf, const Panel4 &p4, double &phi, double coreradius, bool bUseRFF, bool bIncludingBoundVortex) const;
        void getDoubletVelocity( Vector3d const &C, const Panel4 &p4, Vector3d &V, double coreradius, bool bUseRFF, bool bIncludingBoundVortex) const;
        void getSourcePotential( Vector3d const &C, const Panel4 &p4, double &phi) const;
        void getSourceVelocity(  Vector3d const &C, bool bSelf, const Panel4 &p4, Vector3d &V) const;
        void getFarFieldVelocity(Vector3d const &C, const std::vector<Panel4> &panel4, const double *Mu, Vector3d &VT, double coreradius) const;

        double getPotential(Vector3d const &C, const double *mu, const double *sigma) const;
        void getVelocityVector(Vector3d const &C,double const *Mu, double const *Sigma, Vector3d &VT, double coreradius,
                               bool bWakeOnly, bool bMultiThread) const override;

        void VLMGetVortexInfluence(const Panel4 &pPanel, Vector3d const &C, double *phi, Vector3d *V, bool bIncludingBound, double fardist) const;


        void forces(double const *Mu, double const *Sigma, double alpha, double beta, Vector3d const&CoG, bool bFuseMi, const std::vector<Vector3d> &VInf, Vector3d &Force, Vector3d &Moment) override;

        void makeMu(int qrhs) override;

        int makeWakePanels(Vector3d const &WindDirection, bool bVortonWake) override;

        void makeVortons(double dl, double const *Mu4,
                         int pos4, int nPanel4, int nStations, int nVtn0, std::vector<Vorton> &vortons, std::vector<Vortex> &vortexneg) const override;

        void makeNegatingVortices(std::vector<Vortex> &negvortices) override;

        void rebuildPanelsFromNodes(std::vector<Vector3d> const &node);

        void releasePanelArrays();

        void velocityVectorBlock(int iBlock, Vector3d const &C,
                                             Vector3d *VT) const;
        void getDoubletDerivative(int p, const double *Mu, double &Cp, Vector3d &VTotl, const Vector3d &VInf) const;

        Vector3d trailingWakePoint(const Panel4 *pWakePanel) const;
        Vector3d midWakePoint(const Panel4 *pWakePanel) const;
        const Panel4 *trailingWakePanel(Panel4 const *pWakePanel) const;
        int nextTopTrailingPanelIndex(Panel4 const &p4) const;

        void inducedForce(int nPanel3, double QInf, double alpha, double beta, int pos, Vector3d &ForceBodyAxes, SpanDistribs &SpanResFF) const override;
        void trefftzDrag(int nPanels, double QInf, double alpha, double beta, int pos, Vector3d &FFForce, SpanDistribs &SpanResFF) const override;

        Panel *panel(int p) override {if(p>=0 && p<nPanels()) return m_Panel4.data()+p; else return nullptr;}
        Panel const *panelAt(int p) const override {if(p>=0 && p<nPanels()) return m_Panel4.data()+p; else return nullptr;}
        Panel4 const *panel4(int p) const {if(p>=0 && p<nPanels()) return m_Panel4.data()+p; else return nullptr;}
        std::vector<Panel4> const &panels() const {return m_Panel4;}
        std::vector<Panel4> const &wakePanels() const {return m_WakePanel4;}


        void computeOnBodyCp(std::vector<Vector3d> const &VInf,
                             std::vector<Vector3d> const &VLocal, std::vector<double>&Cp) const override;

        double computeCm(const Vector3d &CoG, double Alpha, bool bFuseMi);
        bool computeTrimmedConditions(double mass, const Vector3d &CoG, double &alphaeq, double &u0, bool bFuseMi) override;
        bool getZeroMomentAngle(const Vector3d &CoG, double &alphaeq, bool bFuseMi);

        int allocateRHS4(int nRHS);
        void scaleResultsToSpeed(double ratio) override;
        void makeUnitDoubletStrengths(double alpha, double beta) override;
        void combineLocalVelocities(double alpha, double beta, std::vector<Vector3d> &VLocal) const override;
        void makeLocalVelocities(std::vector<double> const &uRHS, std::vector<double> const &vRHS, std::vector<double> const &wRHS,
                                 std::vector<Vector3d> &uVLocal, std::vector<Vector3d> &vVLocal, std::vector<Vector3d> &wVLocal,
                                 Vector3d const &WindDirection) const override;
//        void combineLocalVelocities(double alpha, double beta, std::vector<Vector3d> &VLocal) const override;

        void sumPanelForces(double *Cp, double Alpha, double &Lift, double &Drag);
        void getVortexCp(const int &p, double *Gamma, double *Cp, Vector3d &VInf);

        void restorePanels() override;
        void savePanels() override;

        void setQuadMesh(QuadMesh const &quadmesh);

        void testResults(double alpha, double beta, double QInf) const override;

    private:
        // Quad analysis mesh variables
        QuadMesh const *m_pRefQuadMesh;       /**< a constant pointer to the plane's reference QuadMesh */
        std::vector<Panel4> m_Panel4;             /**< the current working array of panels */
        std::vector<Panel4> m_WakePanel4;         /**< the current working array of wake panels */
        std::vector<Panel4> m_RefWakePanel4;      /**< a copy of the reference wake node array if wake needs to be reset */
        std::vector<Vector3d> m_WakeNode;	      /**< the current working wake node array */
        std::vector<Vector3d> m_RefWakeNode;      /**< a copy of the reference wake node array if the flat wake geometry needs to be restored */
};


