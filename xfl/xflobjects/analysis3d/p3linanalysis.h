/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include "p3analysis.h"

class Panel3;
class P3LinAnalysis : public P3Analysis
{
    public:
        P3LinAnalysis();
        ~P3LinAnalysis() = default;

        bool isTriUniMethod() const override {return false;}
        bool isTriLinMethod() const override {return true;}

        void makeSourceInfluenceMatrix();
        void sourceToRHS(const QVector<double> &sigma, QVector<double> &RHS);

    protected:
        void makeMatrixBlock(int iBlock) override;
        void makeLocalVelocities(QVector<double> const &uRHS, QVector<double> const &vRHS, const QVector<double> &wRHS,
                                 QVector<Vector3d> &uVLocal, QVector<Vector3d> &vVLocal, QVector<Vector3d> &wVLocal,
                                 Vector3d const &WindDirection) const override;
        void computeOnBodyCp(QVector<Vector3d> const &VInf, const QVector<Vector3d> &VGLOBAL, QVector<double> &Cp) const override;
        void makeNodeDoubletSurfaceVelocity(int iNode, const QVector<double> &uRHS, const QVector<double> &vRHS, const QVector<double> &wRHS, Vector3d &uNode, Vector3d &vNode, Vector3d &wNode) const;
        void makeRHSBlock(int iBlock, double *RHS, QVector<Vector3d> const &VField, Vector3d const*normals) const override;
        void makeWakeMatrixBlock(int iBlock) override;
        void makeSourceMatrixBlock(int iBlock);

        void backSubUnitRHS(double *uRHS, double *vRHS, double*wRHS, double *pRHS, double *qRHS, double*rRHS) override;

        void makeUnitRHSBlock(int iBlock) override;

        void makeUnitDoubletStrengths(double alpha, double beta) override;

        void makeVortons(double dl, double const *mu3Vertex, int pos3, int nPanel3, int nStations, int nVtn0,
                         QVector<Vorton> &vortons, QVector<Vortex> &vortexneg) const override;

        bool scalarProductWake(const Panel3 &panel0, int iWake, double *scalarLeft, double *scalarRight) const;

        void testResults(double alpha, double beta, double QInf) const override;

    protected:
        std::vector<float> m_bijf;    /**< the source influence matrix in single precision */
};





