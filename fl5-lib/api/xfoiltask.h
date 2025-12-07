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


#include <condition_variable>
#include <mutex>
#include <queue>



#include <fl5lib_global.h>

#include <foil.h>
#include <xfoil.h>
#include <analysisrange.h>
#include <utils.h>

class Polar;
class OpPoint;


struct FoilAnalysis
{
    Foil m_Foil;                       /**< the Foil object to be analyzed by the thread; each task must have
                                            its own foil because the flap angle is variable */
    Polar *m_pPolar{nullptr};          /**< a pointer to the Polar object to be analyzed by the thread */
    std::vector<AnalysisRange> range = std::vector<AnalysisRange>();
};


/**
* @class  XFoilTask
* This file implements the management task of an XFoil calculation.
*/
class FL5LIB_EXPORT XFoilTask
{
    public:
        XFoilTask();


        void run();

        void setAnalysisStatus(xfl::enumAnalysisStatus status) {m_AnalysisStatus = status;}
        bool isCancelled() const {return m_AnalysisStatus==xfl::CANCELLED;}
        bool isRunning()   const {return m_AnalysisStatus==xfl::RUNNING;}
        bool isPending()   const {return m_AnalysisStatus==xfl::PENDING;}
        bool isFinished()  const {return m_AnalysisStatus==xfl::FINISHED || m_AnalysisStatus==xfl::CANCELLED;}

        bool hasErrors()   const {return m_bErrors;}

        Foil const *foil() const {return m_pFoil;}
        Polar const *polar() const {return m_pPolar;}

        std::vector<OpPoint*> const &operatingPoints() const {return m_OpPoints;}

        bool processCl(int k);
        bool processClList();

        void clearRanges() {m_AnalysisRange.clear();}
        void setAnalysisRanges(std::vector<AnalysisRange> const &ranges) {m_AnalysisRange=ranges;}
        void appendRange(AnalysisRange const &range) {m_AnalysisRange.push_back(range);}
        void appendRange(bool b, double vmin, double vmax, double vinc) {m_AnalysisRange.push_back({b, vmin, vmax, vinc});}

        bool initialize(FoilAnalysis *pFoilAnalysis, bool bKeepOpps);
        bool initialize(Foil &foil, Polar *pPolar, bool bKeepOpps);

        void initializeBL();

        XFoil const &XFoilInstance() const {return m_XFoilInstance;}

        void clearLog();
        std::string const &log() const {return m_Log;}

        void setAlphaRange(double vMin, double vMax, double vDelta);
        void setClRange(double vMin, double vMax, double vDelta);

        void setKeepOpps(bool b) {m_bKeepOpps = b;}

        bool bAlpha()   const   {return m_bAlpha;}
        void setAoAAnalysis(bool b) {m_bAlpha=b;}

        void clearOpps() {m_OpPoints.clear();}

        void traceLog(const QString &str);
        void traceStdLog(const std::string &str);

        static void setCancelled(bool b);

        static int maxIterations() {return s_IterLim;}
        static void setMaxIterations(int maxiter) {s_IterLim=maxiter;}

        static double CdError() {return s_CdError;}
        static double setCdError(double cderr) {return s_CdError=cderr;}

    private:
        int loop();
        bool alphaSequence(bool bAlpha);
        bool thetaSequence();
        bool ReSequence();
        void addXFoilData(OpPoint *pOpp, XFoil &xfoil, const Foil *pFoil);

        bool processClRange(Polar *pPolar, const AnalysisRange &range);


    private:
        bool m_bErrors;


        XFoil m_XFoilInstance;     /**< An instance of the XFoil class specific to this task */

        std::vector<OpPoint*> m_OpPoints;

        Foil *m_pFoil;                 /**< A pointer to the instance of the Foil object for which the calculation is performed */
        Polar *m_pPolar;                /**< A pointer to the instance of the Polar object for which the calculation is performed */

        bool m_bViscous;           /**< true if performing a viscous calculation - ALWAYS TRUE */
        bool m_bAlpha;             /**< true if performing an analysis based on aoa, false if based on Cl */

        bool m_bKeepOpps;

        std::string m_Log;

        xfl::enumAnalysisStatus m_AnalysisStatus;

        std::vector<AnalysisRange> m_AnalysisRange;

        static bool s_bCancel;            /**< True if the user has asked to cancel the analysis */

        static int  s_IterLim;
        static bool s_bAutoInitBL;        /**< true if the BL initialization is left to the code's decision */
        static double s_CdError;          /**< discard points with |Cd| less than this value: these operating points are likely erroneous (spurious?) */


    public:
        // thread related variables to share the message queue with the calling thread
        std::mutex m_mtx;
        std::condition_variable m_cv;
        std::queue<std::string> m_theMsgQueue;
};




