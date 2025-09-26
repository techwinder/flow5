/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QVector>
#include <QString>
#include <QColor>

#include <xflobjects/objects3d/analysis/opp3d.h>

class Boat;
class BoatPolar;
class Panel4;

class BoatOpp : public Opp3d
{
    public:
        BoatOpp();
        BoatOpp(Boat *pBoat, BoatPolar *pBtPolar, int nPanel3, int nPanel4);
        bool serializeBoatOppFl5(QDataStream &ar, bool bIsStoring);
        void getProperties(const Boat *pBoat, double density, QString &BOppProperties, bool bLongOutput=false) const;

        QString const &boatName() const {return m_BoatName;}
        void setBoatName(QString const &name) {m_BoatName=name;}

        QString const &polarName() const override {return m_BtPolarName;}
        void setPolarName(QString const &name) override {m_BtPolarName=name;}

        void setTWInf(double tws, double twa)  {m_TWS_inf=tws; m_TWA_inf=twa;}

        void setSailAngle(int iSail, double theta) {if(iSail>=0 && iSail<m_SailAngle.size()) m_SailAngle[iSail]=theta;}
        double sailAngle(int iSail) const {if(iSail>=0 && iSail<m_SailAngle.size()) return m_SailAngle.at(iSail); else return 0.0;}

        void resizeResultsArrays(int N);

        QString name() const override {return title(false);}
        QString title(bool bLong=false) const override;

        Vector3d windDir() const;

        Vector3d sailForceFF(int iSail) const {return m_SailForceFF[iSail];}
        Vector3d sailForceSum(int iSail) const {return m_SailForceSum[iSail];}

        void setSailForceFF(QVector<Vector3d>const &forcesFF)   {m_SailForceFF  = forcesFF;}
        void setSailForceSum(QVector<Vector3d>const &forcesSum) {m_SailForceSum = forcesSum;}

        void exportMainDataToString(const Boat *pBoat, QString &poppdata, xfl::enumTextFileType filetype, const QString &textsep) const;
        void exportPanel3DataToString(const Boat *pBoat, xfl::enumTextFileType exporttype, QString &paneldata) const;


    private:
        QString m_BoatName;              /**< the boat name to which the BoatOpp belongs */
        QString m_BtPolarName;           /**< the polar name to which the BoatOpp belongs */

        bool m_bIgnoreBodyPanels;  /**< true if the body panels should be ignored in the analysis */
        bool m_bThinSurfaces;      /**< true if VLM, false if 3D-panels */
        bool m_bTrefftz;           /**< true if the lift and drag are evaluated in the Trefftz plane, false if evaluation is made by summation of panel forces */

        double m_TWS_inf;          /**< the true wind speed at high altitude */
        double m_TWA_inf;          /**< the true wind angle */

        QVector<double> m_SailAngle;    /**< Sail angles around the mast, degrees */


        QVector<Vector3d> m_SailForceFF;    /**< The array of sail forces calculated in the FF plane, in (N/q) */
        QVector<Vector3d> m_SailForceSum;   /**< The array of sail forces calculated by force summation acting on the panels, in (N/q) */


};

