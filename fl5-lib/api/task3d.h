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
#include <condition_variable>
#include <mutex>
#include <queue>

#include <api/fl5lib_global.h>
#include <api/vorton.h>
#include <api/utils.h>


struct VPWReport
{
    public:
        VPWReport()
        {
        }

        std::string const & message() const {return m_Msg;}
        void setMsg(std::string const &msg) {m_Msg=msg;}
    public:
        std::vector<std::vector<Vorton>> m_Vortons;
        double m_Ctrl;

        std::string m_Msg;
};


class Polar3d;
class PanelAnalysis;
class P4Analysis;
class P3Analysis;

class FL5LIB_EXPORT Task3d
{
    public:
        Task3d();
        virtual ~Task3d();

        virtual bool hasErrors() const {return m_bError;}
        virtual bool hasWarning() const {return m_bWarning;}

        virtual void run();
        virtual void onCancel();

        void setAnalysisStatus(xfl::enumAnalysisStatus status);

        bool isCancelled() const {return m_AnalysisStatus==xfl::CANCELLED;}
        bool isRunning()   const {return m_AnalysisStatus==xfl::RUNNING;}
        bool isPending()   const {return m_AnalysisStatus==xfl::PENDING;}
        bool isFinished()  const {return m_AnalysisStatus==xfl::FINISHED || m_AnalysisStatus==xfl::CANCELLED;}

        PanelAnalysis * panelAnalysis() {return m_pPA;}

        int qRHS() const {return m_qRHS;}
        int nRHS() const {return m_nRHS;}

        void advectVortons(double alpha, double beta, double QInf, int qrhs);
        void advectVortonRow(std::vector<Vorton> *thisrow);


        void stopVPWIterations() {m_bStopVPWIterations = true;}

        void traceVPWLog(double ctrl);
        void traceLog(const QString &str);
        virtual void traceStdLog(const std::string &str);

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



    protected:

        Polar3d *m_pPolar3d;

        PanelAnalysis *m_pPA;
        P4Analysis *m_pP4A;
        P3Analysis *m_pP3A;

        bool m_bError;       /**< true if one of the operating points wasn't successfully computed */
        bool m_bWarning;     /**< true if some partial results, e.g. eigenvalues wasn't successfully computed */
        bool m_bStopVPWIterations;

        int m_qRHS;                 /**< the index of the operating point being currently calculated */
        int m_nRHS;                 /**< the number of RHS to calculate; cannot be greater than VLMMAXRHS */

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



    public:
        // thread related variables to share the message queue with the calling threa
        std::mutex m_mtx;
        std::condition_variable m_cv;
        std::queue<VPWReport> m_theMsgQueue;
};


