/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/



#pragma once

#include <xflobjects/analysis3d/panelanalysis.h>
#include <xflpanels/panels/panel4.h>
#include <xflpanels/mesh/quadmesh.h>


#include <xflgeom/geom3d/vector3d.h>

#include <xflobjects/objects3d/analysis/aeroforces.h>
#include <xflobjects/objects3d/analysis/spandistribs.h>
#include <xflpanels/mesh/quadmesh.h>
#include <xflpanels/mesh/trimesh.h>



#include <QVector>
#include <QTime>


class Panel;
class Panel4;
class Polar3d;
class Surface;
class Polar3d;


class P4Analysis : public PanelAnalysis
{
    friend class Task3d;
    friend class PlaneTask;
    friend class BoatTask;

    public:
        P4Analysis();
        ~P4Analysis() = default;

        bool initializeAnalysis(const Polar3d *pPolar3d, int nRHS) override;

        bool isQuadMethod()     const {return true;}
        bool isTriangleMethod() const {return false;}
        bool isTriUniMethod()   const {return false;}
        bool isTriLinMethod()   const {return false;}

        void makeStripAreas();

        void makeInfluenceMatrix() override;
        void makeMatrixBlock(int iBlock);

        void makeUnitRHSBlock(int iBlock) override;
        void makeRHSBlock(int iBlock, double *RHS, QVector<Vector3d> const &VField, const Vector3d *normals) const override;

        void makeWakeMatrixBlock(int iBlock) override;

        void getDoubletPotential(Vector3d const &C, bool bSelf, const Panel4 &p4, double &phi, double coreradius, bool bUseRFF, bool bIncludingBoundVortex) const;
        void getDoubletVelocity( Vector3d const &C, const Panel4 &p4, Vector3d &V, double coreradius, bool bUseRFF, bool bIncludingBoundVortex) const;
        void getSourcePotential( Vector3d const &C, const Panel4 &p4, double &phi) const;
        void getSourceVelocity(  Vector3d const &C, bool bSelf, const Panel4 &p4, Vector3d &V) const;
        void getFarFieldVelocity(Vector3d const &C, const QVector<Panel4> &panel4, const double *Mu, Vector3d &VT, double coreradius) const;

        double getPotential(Vector3d const &C, const double *mu, const double *sigma) const;
        void getVelocityVector(Vector3d const &C,double const *Mu, double const *Sigma, Vector3d &VT, double coreradius,
                               bool bWakeOnly, bool bMultiThread) const override;

        void VLMGetVortexInfluence(const Panel4 &pPanel, Vector3d const &C, double *phi, Vector3d *V, bool bIncludingBound, double fardist) const;


        void forces(double const *Mu, double const *Sigma, double alpha, double beta, Vector3d const&CoG, bool bFuseMi, const QVector<Vector3d> &VInf, Vector3d &Force, Vector3d &Moment) override;

        void makeMu(int qrhs) override;

        int makeWakePanels(Vector3d const &WindDirection, bool bVortonWake) override;

        void makeVortons(double dl, double const *Mu4,
                         int pos4, int nPanel4, int nStations, int nVtn0, QVector<Vorton> &vortons, QVector<Vortex> &vortexneg) const override;

        void makeNegatingVortices(QVector<Vortex> &negvortices) override;

        void rebuildPanelsFromNodes(QVector<Vector3d> const &node);

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

        Panel *panel(int p) override {if(p>=0 && p<m_Panel4.size()) return m_Panel4.data()+p; else return nullptr;}
        Panel const *panelAt(int p) const override {if(p>=0 && p<m_Panel4.size()) return m_Panel4.data()+p; else return nullptr;}
        Panel4 const *panel4(int p) const {if(p>=0 && p<m_Panel4.size()) return m_Panel4.data()+p; else return nullptr;}
        QVector<Panel4> const &panels() {return m_Panel4;}
        QVector<Panel4> const &wakePanels() {return m_WakePanel4;}

        int nPanels() const override {return m_Panel4.size();}
        int nWakePanels() const {return m_WakePanel4.size();}
        int matSize() const override {return m_Panel4.size();}

        void computeOnBodyCp(QVector<Vector3d> const &VInf,
                             QVector<Vector3d> const &VLocal, QVector<double>&Cp) const override;

        double computeCm(const Vector3d &CoG, double Alpha, bool bFuseMi);
        bool computeTrimmedConditions(double mass, const Vector3d &CoG, double &alphaeq, double &u0, bool bFuseMi) override;
        bool getZeroMomentAngle(const Vector3d &CoG, double &alphaeq, bool bFuseMi);

        int allocateRHS4(int nRHS);
        void scaleResultsToSpeed(double ratio) override;
        void makeUnitDoubletStrengths(double alpha, double beta) override;
        void combineLocalVelocities(double alpha, double beta, QVector<Vector3d> &VLocal) const override;
        void makeLocalVelocities(QVector<double> const &uRHS, QVector<double> const &vRHS, QVector<double> const &wRHS,
                                 QVector<Vector3d> &uVLocal, QVector<Vector3d> &vVLocal, QVector<Vector3d> &wVLocal,
                                 Vector3d const &WindDirection) const override;
//        void combineLocalVelocities(double alpha, double beta, QVector<Vector3d> &VLocal) const override;

        void sumPanelForces(double *Cp, double Alpha, double &Lift, double &Drag);
        void getVortexCp(const int &p, double *Gamma, double *Cp, Vector3d &VInf);

        void restorePanels() override;
        void savePanels() override;

        void setQuadMesh(QuadMesh const &quadmesh);

        void testResults(double alpha, double beta, double QInf) const override;

    private:
        // Quad analysis mesh variables
        QuadMesh const *m_pRefQuadMesh;       /**< a constant pointer to the plane's reference QuadMesh */
        QVector<Panel4> m_Panel4;             /**< the current working array of panels */
        QVector<Panel4> m_WakePanel4;         /**< the current working array of wake panels */
        QVector<Panel4> m_RefWakePanel4;      /**< a copy of the reference wake node array if wake needs to be reset */
        QVector<Vector3d> m_WakeNode;	      /**< the current working wake node array */
        QVector<Vector3d> m_RefWakeNode;      /**< a copy of the reference wake node array if the flat wake geometry needs to be restored */
};


