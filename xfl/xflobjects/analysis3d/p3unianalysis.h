/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include "p3analysis.h"

class P3UniAnalysis : public P3Analysis
{
    public:
        P3UniAnalysis();
        ~P3UniAnalysis() = default;

        bool isTriUniMethod() const override {return true;}
        bool isTriLinMethod() const override {return false;}

    protected:
        void makeMatrixBlock(int iBlock) override;

        void makeRHSBlock(int iBlock, double *RHS, QVector<Vector3d> const &VField, Vector3d const*normals) const override;

        void makeLocalVelocities(QVector<double> const &uRHS, QVector<double> const &vRHS, const QVector<double> &wRHS,
                                 QVector<Vector3d> &uLocal, QVector<Vector3d> &vLocal, QVector<Vector3d> &wLocal,
                                 Vector3d const &WindDirection) const override;
        void computeOnBodyCp(QVector<Vector3d> const &VInf, const QVector<Vector3d> &VLocal, QVector<double> &Cp) const override;

        void makeWakeMatrixBlock(int iBlock) override;

        void makeUnitRHSBlock(int iBlock) override;

        void makeUnitDoubletStrengths(double alpha, double beta) override;
        void makeVertexDoubletDensities(QVector<double> const &muSrc, QVector<double> &muNode) const override;

        void makeVortons(double dl, double const *mu3Vertex, int pos3, int nPanel3, int nStations, int nVtn0,
                         QVector<Vorton> &vortons, QVector<Vortex> &vortexneg) const override;

        void checkThinSurfaceSolution();

        void testResults(double alpha, double beta, double QInf) const override;
};

