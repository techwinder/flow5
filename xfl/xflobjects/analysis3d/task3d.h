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

#include <QObject>
#include <QTime>
#include <QVector>

#include <xflcore/analysisrange.h>
#include <xflcore/enums_core.h>
#include <xflcore/flow5events.h>
#include <xflpanels/panels/vorton.h>

class Polar3d;
class PanelAnalysis;
class P4Analysis;
class P3Analysis;

class Task3d : public QObject
{
    Q_OBJECT

    public:
        Task3d();
        virtual ~Task3d();

        void setEventDestination(QObject *pDestination) {m_pEventDestination=pDestination;}

        virtual bool hasErrors() const {return m_bError;}
        virtual bool hasWarning() const {return m_bWarning;}

        void setAnalysisStatus(xfl::enumAnalysisStatus status);

        bool isCancelled() const {return m_AnalysisStatus==xfl::CANCELLED;}
        bool isRunning()   const {return m_AnalysisStatus==xfl::RUNNING;}
        bool isPending()   const {return m_AnalysisStatus==xfl::PENDING;}
        bool isFinished()  const {return m_AnalysisStatus==xfl::FINISHED || m_AnalysisStatus==xfl::CANCELLED;}

        PanelAnalysis * panelAnalysis() {return m_pPA;}

        int qRHS() const {return m_qRHS;}
        int nRHS() const {return m_nRHS;}

        void advectVortons(double alpha, double beta, double QInf, int qrhs);
        void advectVortonRow(QVector<Vorton> *thisrow);


        void stopVPWIterations() {m_bStopVPWIterations = true;}


        static void setCancelled(bool bCancel) {s_bCancel=bCancel;}

        static void setVortonStretch(bool bStretch) {s_bVortonStretch=bStretch;}
        static void setVortonRedist(bool bRedist) {s_bVortonRedist=bRedist;}
        static bool bVortonStretch() {return s_bVortonStretch;}
        static bool bVortonRedist() {return s_bVortonRedist;}

        static void setMaxNRHS(int nmax) {s_MaxNRHS=nmax;}
        static int maxNRHS() {return s_MaxNRHS;}



        static void setLiveUpdate(bool bLive) {s_bLiveUpdate=bLive;}
        static bool bLiveUpdate() {return s_bLiveUpdate;}

    protected:
        virtual void makeVortonRow(int qrhs) = 0;
        virtual void loop() = 0;

    public slots:
        virtual void onCancel();
        virtual void run();
        void onTraceLog(const QString &str) const;

    signals:
        void updatePanelDisplay();
        void taskFinished();
        void outputMessage(QString const &msg) const;

    protected:

        QObject *m_pEventDestination;

        Polar3d *m_pPolar3d;

        PanelAnalysis *m_pPA;
        P4Analysis *m_pP4A;
        P3Analysis *m_pP3A;

        bool m_bError;       /**< true if one of the operating points wasn't successfully computed */
        bool m_bWarning;     /**< true if some partial results, e.g. eigenvalues wasn't successfully computed */
        bool m_bStopVPWIterations;

        int m_qRHS;                 /**< the index of the operating point being currently calculated */
        int m_nRHS;                 /**< the number of RHS to calculate; cannot be greater than VLMMAXRHS */
        QVector<double> m_gamma;    /**< the virtual twist angle for each span section; cf. Computationally Efficient Transonic and Viscous Potential Flow Aero-Structural Method for Rapid Multidisciplinary Design Optimization of Aeroelastic Wing Shaping Control, by Eric Ting and Daniel Chaparro,  Advanced Modeling and Simulation (AMS) Seminar Series, Advanced Advanced Air Air Vehicles Transport Program Technology Project NASA Ames Research Center, June 28, 2017 */

        xfl::enumAnalysisStatus m_AnalysisStatus;

        // temp variables used in the parallelization of vorton row advects
        double const *tmp_Mu;
        double const *tmp_Sigma;
        double tmp_dt;
        double tmp_vortonwakelength;
        Vector3d tmp_VInf;

        static bool s_bCancel;

        static int s_MaxNRHS;

        static bool s_bVortonRedist;  /** option for vorton redistribution */
        static bool s_bVortonStretch;      /** option for vorton strength exchange */
        static bool s_bLiveUpdate;
};



class VPWUpdateEvent : public QEvent
{
    public:
        VPWUpdateEvent(): QEvent(VPW_UPDATE_EVENT) {}
        QVector<QVector<Vorton>> m_Vortons;
        double m_Ctrl;
};

