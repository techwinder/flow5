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


#include <vorton.h>
#include <vortex.h>
#include <aeroforces.h>
#include <spandistribs.h>
#include <utils.h>

class Polar3d;
class Panel;
class StabDerivatives;
class Vortex;

class FL5LIB_EXPORT PanelAnalysis
{
    friend class  Task3d;
    friend class  PlaneTask;
    friend class  BoatTask;

    public:
        PanelAnalysis();
        virtual ~PanelAnalysis() = default;

        void setAnalysisStatus(xfl::enumAnalysisStatus status) {m_AnalysisStatus=status;}
        void cancelAnalysis() {m_AnalysisStatus=xfl::CANCELLED;}
        bool isCancelled() const {return m_AnalysisStatus==xfl::CANCELLED;}
        bool isRunning()   const {return m_AnalysisStatus==xfl::RUNNING;}
        bool isPending()   const {return m_AnalysisStatus==xfl::PENDING;}
        bool isFinished()  const {return m_AnalysisStatus==xfl::FINISHED || m_AnalysisStatus==xfl::CANCELLED;}

        bool allocateMatrix(int N);

        virtual Panel *panel(int p) = 0;
        virtual Panel const *panelAt(int p) const = 0;
        virtual bool initializeAnalysis(Polar3d const *pPolar3d, int nRHS);
        virtual int  matSize() const = 0;
        virtual int  makeWakePanels(Vector3d const &WindDirection, bool bVortonWake) = 0;
        virtual void scaleResultsToSpeed(double ratio) = 0;
        virtual void savePanels() = 0;
        virtual void restorePanels() = 0;
        virtual void makeInfluenceMatrix() = 0;
        virtual void makeUnitRHSBlock(int iBlock) = 0;
        virtual void makeRHSBlock(int iBlock, double *RHS, std::vector<Vector3d> const &VField, Vector3d const*normals) const = 0;
        virtual void makeUnitDoubletStrengths(double alpha, double beta) = 0;
        virtual void makeWakeMatrixBlock(int iBlock) = 0;

        virtual void makeMu(int qrhs) = 0;
        virtual void makeLocalVelocities(std::vector<double> const &uRHS, std::vector<double> const &vRHS, std::vector<double> const &wRHS,
                                         std::vector<Vector3d> &uVLocal, std::vector<Vector3d> &vVLocal, std::vector<Vector3d> &wVLocal,
                                         Vector3d const &WindDirection) const = 0;
        virtual void combineLocalVelocities(double alpha, double beta, std::vector<Vector3d> &VLocal) const = 0;
        virtual void computeOnBodyCp(std::vector<Vector3d> const &VInf, std::vector<Vector3d> const &VLocal, std::vector<double>&Cp) const = 0;
        virtual bool computeTrimmedConditions(double mass, Vector3d const &CoG, double &alphaeq, double &u0, bool bFuseMi) = 0;
        virtual void forces(double const *Mu, double const *Sigma, double alpha, double beta, Vector3d const &CoG, bool bFuseMi, std::vector<Vector3d> const &VInf, Vector3d &Force, Vector3d &Moment) = 0;
        virtual void inducedForce(int nPanel3, double QInf, double alpha, double beta, int pos, Vector3d &ForceBodyAxes, SpanDistribs &SpanResFF) const = 0;
        virtual void trefftzDrag(int nPanel3, double QInf, double alpha, double beta, int pos, Vector3d &Drag, SpanDistribs &SpanResFF) const = 0;
        virtual int  nPanels() const = 0;
        virtual void makeVertexDoubletDensities(std::vector<double> const &muPanel, std::vector<double> &muNode) const {(void)muPanel; (void)muNode;} //dummy virtual method to enable a call to P3analysis subclass...

        virtual void makeVortons(double dl, double const *mu3Vertex, int pos3, int nPanel3, int nStations, int nVtn0,
                                 std::vector<Vorton> &vortons, std::vector<Vortex> &vortexneg) const = 0;
        virtual void makeRHSVWVelocities(std::vector<Vector3d>& VPW, bool bVLM=false);
        virtual void makeRHSVWVelocitiesBlock(int iFirst, int iLast, bool bVLM, Vector3d *Vpanel);
        virtual void makeNegatingVortices(std::vector<Vortex> &negvortices) = 0;

        virtual void getVelocityVector(Vector3d const &C, double const *Mu, double const *Sigma, Vector3d &VT, double coreradius, bool bWakeOnly, bool bMultiThread) const = 0;

        void makeUnitRHSVectors();
        void makeWakeContribution();

        void makeSourceStrengths(Vector3d const &WindDirection);
        void makeSourceStrengths(std::vector<Vector3d> const &WindDirection);

        void vortonDrag(double alpha, double beta, double QInf, int n0, int nStations, Vector3d &Drag, SpanDistribs &SpanResFF) const;

        void setVortons(const std::vector<std::vector<Vorton> > &vortons);

        void combineUnitRHS(std::vector<double> &RHS, const Vector3d &VInf, const Vector3d &Omega);
        void makeRHS(const std::vector<Vector3d> &VField, std::vector<double> &RHS, const Vector3d *normals);

        void addWakeContribution();
        void computeStabilityDerivatives(  double alphaeq, double u0, Vector3d const &CoG, bool bFuseMi, StabDerivatives &SD, Vector3d &Force0, Vector3d &Moment0);
        void computeTranslationDerivatives(double alphaeq, double u0, Vector3d const &CoG, bool bFuseMi, StabDerivatives &SD, Vector3d &Force0, Vector3d &Moment0);
        void computeAngularDerivatives(    double alphaeq, double u0, Vector3d const &CoG, bool bFuseMi, StabDerivatives &SD);

        void traceLog(const QString &str) const;
        void traceStdLog(const std::string &str) const;
        inline double sourceStrength(const Vector3d &normal, Vector3d const &Velocity) const {return -1.0/4.0/PI * Velocity.dot(normal);}
        void releasePanelArrays();

        Polar3d const *polar3d() const {return m_pPolar3d;}

        int nVortonRows() const {return int(m_Vorton.size());}
        void clearVortons() {m_Vorton.clear();}
        void getVortonVelocity(Vector3d const &C, double vtncorelength, Vector3d &VelVtn, bool bMultiThread=false) const;
        void getVortonRowVelocity(int iRow, Vector3d const &C, double vtncorelength, Vector3d *VelVtn) const;
        void getVortonVelocityGradient(Vector3d const &C, double *G) const;


        virtual void testResults(double alpha, double beta, double QInf) const = 0;

        static void setMultiThread(bool bMulti) {s_bMultiThread=bMulti;}
        static void setMaxThreadCount(int maxthreads) {s_MaxThreads=maxthreads;}
        static void setDoublePrecision(bool bDouble) {s_bDoublePrecision=bDouble;}
        static bool bDoublePrecision() {return s_bDoublePrecision;}

        static void clearDebugPts() {s_DebugPts.clear(); s_DebugVecs.clear();}


    protected:
        bool LUfactorize();
        virtual void backSubUnitRHS(double *uRHS, double *vRHS, double*wRHS, double *pRHS, double *qRHS, double*rRHS);
        bool backSubRHS(std::vector<double> &RHS);

    protected:

        mutable std::string m_ErrorLog;

        Polar3d const *m_pPolar3d;

        std::vector<double> m_aijd;  /**< the matrix of panel influences - double precision; std::vector is limited to 2 GB and is unusable*/
        std::vector<float>  m_aijf;  /**< the matrix of panel influences - single precision; std::vector is limited to 2 GB and is unusable*/
        std::vector<int>    m_ipiv;  /** the array of pivot indices for the LAPACK LU solver */


        // unit RHS for the 6 motion d.o.f
        std::vector<double> m_uRHS, m_vRHS, m_wRHS;
        std::vector<double> m_pRHS, m_qRHS, m_rRHS;

        // for control derivatives

        std::vector<double> m_cRHS;

        std::vector<Vector3d> m_uVLocal;              /**< the array of unit velocity vectors for aoa=0,  in local coordinates */
        std::vector<Vector3d> m_vVLocal;              /**< the array of unit velocity vectors for beta=PI/2, in local coordinates */
        std::vector<Vector3d> m_wVLocal;              /**< the array of unit velocity vectors for aoa=PI/2, in local coordinates */

        std::vector<double> m_Cp;                /**< The array of pressure coefficients on the panels. 1 value/panel in the case of the quad methods, 3 values/panel in the case of the triangular methods */
        std::vector<double> m_Mu;                /**< The array of doublet strengths, or vortex circulations, associated to the panels. 1 value/panel in the case of the quad methods, 3 values/panel in the case of the triangular methods */
        std::vector<double> m_Sigma;             /**< The array of resulting source strengths of the analysis. 1 value/panel for all methods. */

        bool m_bCancel; /** to interrupt the matrix solver only; */

        bool m_bSequence;           /**< true if the calculation is should be performed for a range of aoa */
        bool m_bWarning;     /**< true if one the OpPoints could not be properly interpolated */
        bool m_bMatrixError;

        xfl::enumAnalysisStatus m_AnalysisStatus;
        int m_nBlocks;              /** the number of row blocks for multithreading */

        int m_nStations;          /**< the number of chordwise strips,
                                   which is also the number of panels or stations in the spanwise direction,
                                   which is also the number of wake columns*/


        std::vector<std::vector<Vorton>> m_Vorton; /** The array of vorton rows. Vortons are organized in rows. Each row is located in a crossflow plane. The number of vortons is variable for each row, due to vortex stretching and vorton redistribution. */
        std::vector<Vortex> m_VortexNeg;    /** The array of negating vortices at the trailing edge of the trailing wake panel of each wake column. cf. Willis 2005 fig. 3*/


        static bool s_bDoublePrecision;
        static bool s_bMultiThread;
        static int s_MaxThreads;

        mutable double const *tmp_Mu;
        mutable double const *tmp_Sigma;
        mutable double tmp_coreradius;
        mutable bool tmp_bWakeOnly;

    public:
        static std::vector<Vector3d> s_DebugPts;
        static std::vector<Vector3d> s_DebugVecs;
};





