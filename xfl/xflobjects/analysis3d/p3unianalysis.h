/****************************************************************************

    flow5
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

