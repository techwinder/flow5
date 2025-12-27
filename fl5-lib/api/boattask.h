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

#include <spandistribs.h>
#include <aeroforces.h>
#include <panel3.h>
#include <task3d.h>


class Boat;
class BoatPolar;
class BoatOpp;

class Vector3d;


class FL5LIB_EXPORT BoatTask : public Task3d
{
    public:
        BoatTask();

        void setObjects(Boat *pBoat, BoatPolar *pBtPolar);
        bool initializeTask(QObject *pParent);


        void setAnalysisRange(const std::vector<double> &opplist);

        bool initializeTriangleAnalysis();

        void loop() override;

        double ctrlParam() const {return m_Ctrl;}

        void scaleResultsToSpeed(int qrhs);
        BoatOpp *computeBoat(int qrhs);
        BoatOpp *createBtOpp(const double *Cp, const double *Mu, const double *Sigma);

        void setAngles(std::vector<Panel3> &panels, double phi);

        int allocateSailResultsArrays();

        Boat *boat() const {return m_pBoat;}
        BoatPolar *btPolar() const {return m_pBtPolar;}
        std::vector<BoatOpp*> const &BtOppList() const {return m_BtOppList;}


    private:
        void makeVortonRow(int qrhs) override;
        void computeInducedForces(double alpha, double beta, double QInf);
        void computeInducedDrag(double alpha, double beta, double QInf, int qrhs,
                                std::vector<Vector3d> &WingForce, std::vector<SpanDistribs> &SpanDist) const;

    private:
        Boat *m_pBoat;
        BoatPolar *m_pBtPolar;
        BoatOpp *m_pLiveBtOpp;             /**< to update the 3d display during the analysis */

        std::vector<double> m_OppList;
        double m_Ctrl;

        std::vector<Vector3d> m_SailForceFF;   /**< The array of calculated resulting forces acting on the sails in body axis (N/q) */
        std::vector<Vector3d> m_SailForceSum;   /**< The array of calculated resulting forces acting on the sails in body axis (N/q) */
        std::vector<Vector3d> m_HullForce;   /**< The array of calculated resulting forces acting on the fuses in body axis (N/q) */
        std::vector<SpanDistribs> m_SpanDist; /**< the array of span distributions of the sails */

        //only one set of aerocoefs is needed, since unlike planes, points are calculated one at a time and stored in a BoatOpp
        AeroForces m_AF;

        std::vector<BoatOpp*> m_BtOppList;

};

