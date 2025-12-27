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


#include <enums_objects.h>
#include <task3d.h>
#include <vector3d.h>
#include <spandistribs.h>



struct LLTOppReport
{
    public:
        LLTOppReport(double alpha, std::vector<double>const &max_a, std::string const &msg)

        {
            m_alpha = alpha;
            m_max_a = max_a;
            m_Msg = msg;
        }

        double alpha() const {return m_alpha;}
        std::vector<double> const &max_a() const {return m_max_a;}
        std::string const & message() const {return m_Msg;}

        void setAlpha(double aoa) {m_alpha=aoa;}
        void setMaxa(std::vector<double> maxa) {m_max_a=maxa;}
        void setMsg(std::string const &msg) {m_Msg=msg;}

    private:
        std::string m_Msg;
        double m_alpha;
        std::vector<double> m_max_a;
};



class PlaneXfl;
class PlanePolar;
class PlaneOpp;
class WingXfl;
class Polar;
class Foil;

#define MAXSPANSTATIONS   1000     /**< The max number of stations for LLT. For a VLM analysis, this is the max number of panels in the spanwise direction. */


class FL5LIB_EXPORT LLTTask : public Task3d
{
    public:
        LLTTask();
        void clearPOppList();
        void initializeAnalysis();
        void initializeGeom();

        void setLLTRange(const std::vector<double> &opplist) {m_AoAList = opplist;}
        void setObjects(PlaneXfl *pPlane, PlanePolar *pWPolar);

        void run() override;
        void traceStdLog(std::string const &str) override;
        void traceLog(QString const &str);
        void traceOpp(double alpha, std::vector<double>const &max_a, std::string const &msg);

        bool hasErrors() const override {return m_bError || m_bWarning;}

        PlaneXfl *plane() {return m_pPlane;}
        PlanePolar *wPolar() {return m_pPlPolar;}

        std::vector<PlaneOpp*> const & planeOppList() const {return m_PlaneOppList;}
        void clearPlaneOppList();

        void makeVortonRow(int ) override {}
        void loop() override {}

        static void setMaxIter(int maxIter){s_IterLim = maxIter;}
        static void setConvergencePrecision(double precision) {s_CvPrec = precision;}
        static void setNSpanStations(int nStations){s_NLLTStations=nStations;}
        static void setRelaxationFactor(double relax){s_RelaxMax = relax;}

        static int maxIter() {return s_IterLim;}
        static double convergencePrecision() {return s_CvPrec;}
        static int nSpanStations() {return s_NLLTStations;}
        static double relaxationFactor() {return s_RelaxMax;}

    private:
        double alphaInduced(int k) const;
        double Beta(int m, int k) const;
        double Eta(int m) const;
        void computeWing(double QInf, double Alpha, std::string &ErrMessage);
        void initializeVelocity(double alpha, double &QInf);
        int iterate(double &QInf, double const Alpha);
        void setBending(double QInf);
        bool setLinearSolution(double Alpha);
        void resetVariables();
        double Sigma(int m) const;
        void computeLLTChords(int NStation, double *lltchord, double *lltoffset, double *llttwist);

        PlaneOpp *createPlaneOpp(double QInf, double Alpha, bool bWingOut);
        bool alphaLoop();


    private:

        PlaneXfl *m_pPlane;                            /**< A pointer to the Plane object for which the main wing calculation shall be performed >*/
        WingXfl *m_pWing;                              /**< A pointer to the Wing object for which the calculation shall be performed >*/
        PlanePolar *m_pPlPolar;                          /**< A pointer to the WPolar object for which the calculation shall be performed >*/

        std::vector<double> m_AoAList;   /**< The list of operating points to analyze */

        bool m_bConverged;                          /**< true if the analysis has converged  */
        bool m_bWingOut;                            /**< true if the interpolation of viscous properties falls outside the polar mesh */

        double m_CDi;                               /**< The wing's induced drag coefficient */
        double m_CDv;                               /**< The wing's viscous drag coefficient */
        double m_CL;                                /**< The wing's lift coefficient */
        double m_GCm;                               /**< The wing's total pitching moment */
        double m_GRm;                               /**< The wing's total rolling moment */
        double m_GYm;                               /**< The wing's total yawing moment */
        double m_ICm;                               /**< The wing's induced pitching moment */
        double m_IYm;                               /**< The wing's induced yawing moment */
        double m_QInf0;                             /**< The freestream velocity */
        double m_VCm;                               /**< The wing's viscous pitching moment */
        double m_VYm;                               /**< The wing's viscous yawing moment */

        std::vector<double> m_SpanPos;                  /**< Span position of the span stations */
        std::vector<double> m_Chord;                    /**< chord at the span stations */
        std::vector<double> m_StripArea;                /** <Local strip area at the span stations */
        std::vector<double> m_Twist;                    /**< twist at the span stations */
        std::vector<double> m_Offset;                   /**< offset at  the span stations */

        std::vector<double> m_Re;                       /**< Reynolds number at the span stations */
        std::vector<double> m_Ai;                       /**< Induced Angle coefficient at the span stations */
        std::vector<double> m_BendingMoment;            /**< bending moment at the span stations */
        std::vector<double> m_Cl;                       /**< Local lift coefficient at the span stations */
        std::vector<double> m_Cm;                       /**< Total pitching moment coefficient at the span stations */
        std::vector<double> m_CmAirf;                   /**< Airfoil part of the pitching moment coefficient at the span stations */
        std::vector<double> m_ICd;                      /**< Induced Drag coefficient at the span stations */
        std::vector<double> m_PCd;                      /**< Viscous Drag coefficient at the span stations */
        std::vector<double> m_XCPSpanAbs;               /**< Center of Pressure pos at the span stations */
        std::vector<double> m_XCPSpanRel;               /**< Center of Pressure pos at the span stations */
        std::vector<double> m_XTrTop;                   /**< Upper transition location at the span stations */
        std::vector<double> m_XTrBot;                   /**< Lower transition location at the span stations */

        Vector3d m_CP;                               /**< The position of the center of pressure */

        SpanDistribs m_SpanDistribs;


        //    Output Data
        std::vector<int> m_iter;
        std::vector<double> m_Max_a;


        static int s_IterLim;                       /**< The maximum number of iterations in the calculation */
        static int s_NLLTStations;                  /**< The number of LLT stations in the spanwise direction */
        static double s_RelaxMax;                   /**< The relaxation factor for the iterations */
        static double s_CvPrec;                     /**< Precision criterion to stop the iterations. The difference in induced angle at any span point between two iterations should be less than the criterion */

        std::vector<PlaneOpp*> m_PlaneOppList;



    public:
        // thread related variables to share the message queue with the calling thread
        std::mutex m_mtx;
        std::condition_variable m_cv;
        std::queue<LLTOppReport> m_theOppQueue;

};


