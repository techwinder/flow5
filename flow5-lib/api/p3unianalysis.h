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

#include "p3analysis.h"

class FL5LIB_EXPORT P3UniAnalysis : public P3Analysis
{
    public:
        P3UniAnalysis();
        ~P3UniAnalysis() = default;

        bool isTriUniMethod() const override {return true;}
        bool isTriLinMethod() const override {return false;}

    protected:
        void makeMatrixBlock(int iBlock) override;

        void makeRHSBlock(int iBlock, double *RHS, std::vector<Vector3d> const &VField, Vector3d const*normals) const override;

        void makeLocalVelocities(std::vector<double> const &uRHS, std::vector<double> const &vRHS, const std::vector<double> &wRHS,
                                 std::vector<Vector3d> &uLocal, std::vector<Vector3d> &vLocal, std::vector<Vector3d> &wLocal,
                                 Vector3d const &WindDirection) const override;
        void computeOnBodyCp(std::vector<Vector3d> const &VInf, const std::vector<Vector3d> &VLocal, std::vector<double> &Cp) const override;

        void makeWakeMatrixBlock(int iBlock) override;

        void makeUnitRHSBlock(int iBlock) override;

        void makeUnitDoubletStrengths(double alpha, double beta) override;
        void makeVertexDoubletDensities(std::vector<double> const &muSrc, std::vector<double> &muNode) const override;

        void makeVortons(double dl, double const *mu3Vertex, int pos3, int nPanel3, int nStations, int nVtn0,
                         std::vector<Vorton> &vortons, std::vector<Vortex> &vortexneg) const override;

        void checkThinSurfaceSolution();

        void testResults(double alpha, double beta, double QInf) const override;
};

