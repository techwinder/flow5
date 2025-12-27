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



#include <vector3d.h>
#include <polar3d.h>
#include <utils.h>
#include <aeroforces.h>

class Boat;
class BoatOpp;

class FL5LIB_EXPORT BoatPolar : public Polar3d
{
    friend class  BoatPolarDlg;
    friend class  BoatTask;
    friend class  BtPolarAutoNameDlg;
    friend class  BtPolarDlg;
    friend class  BtPolarNameMaker;
    friend class  XSail;
    friend class  XmlBtPolarReader;

    public:
        BoatPolar();

        void setDefaultSpec(Boat const *pBoat);
        void duplicateSpec(Polar3d const *pPolar3d) override;
        double wakeLength() const override {return m_TotalWakeLengthFactor;} // in meters
        double TrefftzDistance() const override {return m_TotalWakeLengthFactor;}


        bool serializeFl5v726(QDataStream &ar, bool bIsStoring) override;
        bool serializeFl5v750(QDataStream &ar, bool bIsStoring) override;
        void clearData();

        void addPoint(const BoatOpp *pBtOpp);
        double getVariable(int iVar, int iPoint) const;
        void getProperties(std::string &props, xfl::enumTextFileType filetype=xfl::CSV, bool bData=false) const;
        void exportBtPlr(std::string &outstring, xfl::enumTextFileType filetype=xfl::CSV, bool bDataOnly=false) const;

        void copy(BoatPolar const *pBoatPolar);

        std::string const &boatName() const {return m_BoatName;}
        void setBoatName(std::string const &name) {m_BoatName=name;}

        int dataSize() const {return int(m_Ctrl.size());}
        bool hasPoints() const {return m_Ctrl.size()>0;}

        bool hasBtOpp(const BoatOpp *pBOpp) const;

        void makeDefaultArrays(); //debug only

        double twaMin() const {return m_TWAMin;}
        double twaMax() const {return m_TWAMax;}
        void setTwaMin(double bmin) {m_TWAMin=bmin;}
        void setTwaMax(double bmax) {m_TWAMax=bmax;}

        double phiMin() const {return m_PhiMin;}
        double phiMax() const {return m_PhiMax;}
        void setPhiMin(double fmin) {m_PhiMin=fmin;}
        void setPhiMax(double fmax) {m_PhiMax=fmax;}

        double RyMin() const {return m_RyMin;}
        double RyMax() const {return m_RyMax;}
        void setRyMin(double fmin) {m_RyMin=fmin;}
        void setRyMax(double fmax) {m_RyMax=fmax;}

        double qInfMin() const {return m_TWSMin;}
        double qInfMax() const {return m_TWSMax;}
        void setQInfMin(double vmin) {m_TWSMin=vmin;}
        void setQInfMax(double vmax) {m_TWSMax=vmax;}

        double VBtMin() const {return m_VBtMin;}
        double VBtMax() const {return m_VBtMax;}
        void setVBtMin(double vmin) {m_VBtMin=vmin;}
        void setVBtMax(double vmax) {m_VBtMax=vmax;}


        /**< returns the dynamic pressure for a given ctrl parameter */
        double qDyn(double ctrl) const {return 0.5*density()*TWSInf(ctrl)*TWSInf(ctrl);}

        double boatSpeed(double ctrl) const {return m_VBtMin+ctrl*(m_VBtMax-m_VBtMin);}
        double TWSInf(double ctrl)  const {return m_TWSMin*(1.0-ctrl) + m_TWSMax*ctrl;}
        double TWAInf(double ctrl)  const {return m_TWAMin*(1.0-ctrl) + m_TWAMax*ctrl;}
        double AWSInf(double ctrl)  const;
        double AWAInf(double ctrl)  const;
        double phi(double ctrl)     const {return m_PhiMin  + ctrl*(m_PhiMax-m_PhiMin);}
        double Ry(double ctrl)      const {return m_RyMin   + ctrl*(m_RyMax-m_RyMin);}
        double sailAngle(int iSail, double ctrl) const;

        int sailAngleSize() const {return int(m_SailAngleMin.size());}
        double sailAngleMin(int isail) const {return m_SailAngleMin.at(isail);}
        double sailAngleMax(int isail) const {return m_SailAngleMax.at(isail);}
        void setSailAngleRange(int isail, double thetamin, double thetamax);

        void resizeSailAngles(int newsize);

        double windFactor(double x, double y, double z) const override {(void)x; (void)y; return windForce(z);}
        double windForce(double z) const;

        void trueWindSpeed(double ctrl, double z, Vector3d &VT) const;
        void apparentWind(double ctrl, double z, Vector3d &AWS) const;

        void getBtPolarData(std::string &data, const std::string &separator) const;
        double variable(int iVariable, int index) const;

        double bufferWakeLength() const override {return m_BufferWakeFactor * referenceChordLength();}

        bool bAutoRefDims() const {return m_ReferenceDim==xfl::AUTODIMS;}
        double referenceChordLength() const override {return m_ReferenceChord;}
        double referenceArea() const {return m_ReferenceArea;}

        void setAutoRefDims(bool b) {m_ReferenceDim = b ? xfl::AUTODIMS : xfl::CUSTOM;}
        void setReferenceChordLength(double ch) override {m_ReferenceChord=ch;}
        void setReferenceArea(double a) {m_ReferenceArea=a;}

        void remove(int i);
        void insertDataPointAt(int index, bool bAfter);

        BSpline &windSpline() {return m_WindSpline;}
        BSpline const &windSpline() const {return m_WindSpline;}
        void setWindSpline(BSpline const &spline) {m_WindSpline=spline;}

        static std::vector<std::string> const &variableNames() {return s_BtPolarVariableNames;}
        static std::string variableName(int iVar) {return s_BtPolarVariableNames.at(iVar);}
        static int variableCount() {return int(s_BtPolarVariableNames.size());}
        static void setVariableNames();

    private:

        std::string m_BoatName;

        double m_ReferenceArea, m_ReferenceChord;

        //Variables
        double m_VBtMin; /** Boat speed - minimum value */
        double m_VBtMax; /** Boat speed - minimum value */

        double m_TWSMin; /** Apparent Wind Speed AWS at high altitude - minimum value */
        double m_TWSMax; /** Apparent Wind Speed AWS at high altitude - maximum value */
        double m_TWAMin; /** Apparent Wind Angle AWA at high altitude - minimum value */
        double m_TWAMax; /** Apparent Wind Angle AWA at high altitude - maximum value */


        double m_PhiMin, m_PhiMax;   //bank angle (°)
        double m_RyMin, m_RyMax;     //Rotation around Y (°), special for windsurfs
        std::vector<double> m_SailAngleMin;
        std::vector<double> m_SailAngleMax;

        std::vector<double>  m_Ctrl; // The main parameter for the polar
        std::vector<double>  m_VInf;
        std::vector<double>  m_Beta;
        std::vector<double>  m_Phi;
        std::vector<AeroForces> m_AC;

        BSpline m_WindSpline;

        static std::vector<std::string> s_BtPolarVariableNames;
};

