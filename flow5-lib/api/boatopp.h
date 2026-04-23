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


#include <opp3d.h>

class Boat;
class BoatPolar;
class Panel4;

class FL5LIB_EXPORT BoatOpp : public Opp3d
{
    public:
        BoatOpp();
        BoatOpp(Boat *pBoat, BoatPolar *pBtPolar, int nPanel3, int nPanel4);
        bool serializeBoatOppFl5(QDataStream &ar, bool bIsStoring);
        void getProperties(const Boat *pBoat, double density, std::string &props, bool bLongOutput=false) const;

        std::string const &boatName() const {return m_BoatName;}
        void setBoatName(std::string const &name) {m_BoatName=name;}

        std::string const &polarName() const override {return m_BtPolarName;}
        void setPolarName(std::string const &name) override {m_BtPolarName=name;}

        void setTWInf(double tws, double twa)  {m_TWS_inf=tws; m_TWA_inf=twa;}

        void setSailAngle(int iSail, double theta) {if(iSail>=0 && iSail<int(m_SailAngle.size())) m_SailAngle[iSail]=theta;}
        double sailAngle(int iSail) const {if(iSail>=0 && iSail<int(m_SailAngle.size())) return m_SailAngle.at(iSail); else return 0.0;}

        void resizeResultsArrays(int N);

        std::string name() const override {return title(false);}
        std::string title(bool bLong=false) const override;

        Vector3d windDir() const;

        Vector3d sailForceFF(int iSail) const {return m_SailForceFF[iSail];}
        Vector3d sailForceSum(int iSail) const {return m_SailForceSum[iSail];}

        void setSailForceFF(std::vector<Vector3d>const &forcesFF)   {m_SailForceFF  = forcesFF;}
        void setSailForceSum(std::vector<Vector3d>const &forcesSum) {m_SailForceSum = forcesSum;}

        void exportMainDataToString(const Boat *pBoat, std::string &data, xfl::enumTextFileType filetype, const std::string &textsep) const;
        void exportPanel3DataToString(const Boat *pBoat, xfl::enumTextFileType exporttype, std::string &data) const;


    private:
        std::string m_BoatName;              /**< the boat name to which the BoatOpp belongs */
        std::string m_BtPolarName;           /**< the polar name to which the BoatOpp belongs */

        bool m_bIgnoreBodyPanels;  /**< true if the body panels should be ignored in the analysis */
        bool m_bThinSurfaces;      /**< true if VLM, false if 3D-panels */
        bool m_bTrefftz;           /**< true if the lift and drag are evaluated in the Trefftz plane, false if evaluation is made by summation of panel forces */

        double m_TWS_inf;          /**< the true wind speed at high altitude */
        double m_TWA_inf;          /**< the true wind angle */

        std::vector<double> m_SailAngle;    /**< Sail angles around the mast, degrees */


        std::vector<Vector3d> m_SailForceFF;    /**< The array of sail forces calculated in the FF plane, in (N/q) */
        std::vector<Vector3d> m_SailForceSum;   /**< The array of sail forces calculated by force summation acting on the panels, in (N/q) */


};

