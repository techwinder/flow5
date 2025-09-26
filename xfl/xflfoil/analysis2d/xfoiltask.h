/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

/** @file This file implements the management task of an XFoil calculation. Used in multithreaded analysis. */

#pragma once

#include <QSettings>
#include <QRunnable>
#include <QFile>


#include <xfoil.h>
#include <xflcore/analysisrange.h>
#include <xflcore/enums_core.h>


class Foil;
class Polar;
class OpPoint;

/**
 * @struct Analysis a pair of foil and polar used by a thread to perform one foil polar analysis.
 */
struct FoilAnalysis
{
        Foil *pFoil{nullptr};            /**< a pointer to the Foil object to be analyzed by the thread */
        Polar *pPolar{nullptr};          /**< a pointer to the Polar object to be analyzed by the thread */
        double vMin{0}, vMax{0}, vInc{0};
};


/**
* @class XFoilTask
* This file implements the management task of an XFoil calculation.
*/
class XFoilTask : public QObject,  public QRunnable
{
    Q_OBJECT

    public:
        XFoilTask(QObject *pParent);

        void setAnalysisStatus(xfl::enumAnalysisStatus status) {m_AnalysisStatus = status;}
        bool isCancelled() const {return m_AnalysisStatus==xfl::CANCELLED;}
        bool isRunning()   const {return m_AnalysisStatus==xfl::RUNNING;}
        bool isPending()   const {return m_AnalysisStatus==xfl::PENDING;}
        bool isFinished()  const {return m_AnalysisStatus==xfl::FINISHED || m_AnalysisStatus==xfl::CANCELLED;}

        bool hasErrors()   const {return m_bErrors;}

        bool isXFoilTask() const {return true;}
        bool isFoilTask()  const {return false;}

        void setEventDestination(QObject *pDestination) {m_pParent=pDestination;}


        bool processCl(double Cl, double Re, double &Cd, double &XTrTop, double &XTrBot, bool &bCv);
        bool processClList(const QVector<double> &ClList, const QVector<double> &ReList, QVector<double> &CdList, QVector<double> &XTrTopList, QVector<double> &XTrBotList, QVector<bool> &CvList);

        void setAnalysisRanges(QVector<AnalysisRange> const &ranges) {m_AnalysisRange=ranges;}

        bool initializeTask(FoilAnalysis *pFoilAnalysis, bool bStoreOpp, bool bViscous=true, bool bInitBL=true);
        bool initializeTask(Foil *pFoil, Polar *pPolar, bool bStoreOpp, bool bViscous=true, bool bInitBL=true);

        XFoil const &XFoilInstance() const {return m_XFoilInstance;}


        void setAlphaRange(double vMin, double vMax, double vDelta);
        void setClRange(double vMin, double vMax, double vDelta);

        void initializeBL();

        void setStreamText(QString *stream) {m_OutStream.setString(stream);}
        void setStreamFile(QFile *stream) {m_OutStream.setDevice(stream);}
        void flushOutStream() {m_OutStream.flush();}

        void traceLog(const QString &str);

        QString const &logMessage() const {return m_OutMessage;}
        void clearLog() {m_OutMessage.clear();}

        static void cancelAnalyses() {s_bCancel=true;}
        static void setCancelled(bool b) {s_bCancel=b;}

        static void setSkipOpp(bool b) {s_bSkipOpp=b;}
        static bool bSkipOpp() {return s_bSkipOpp;}

        static void setSkipPolar(bool b) {s_bSkipPolar=b;}
        static bool bSkipPolar() {return s_bSkipPolar;}

        static int maxIterations() {return s_IterLim;}
        static void setMaxIterations(int maxiter) {s_IterLim=maxiter;}

        static bool bAutoInitBL() {return s_bAutoInitBL;}
        static void setAutoInitBL(bool b) {s_bAutoInitBL=b;}

        static bool bStoreOpps() {return s_bStoreOpp;}
        static void setStoreOpps(bool b) {s_bStoreOpp=b;}

        static bool bAlpha()     {return s_bAlpha;}
        static void setAoAAnalysis(bool b) {s_bAlpha=b;}

        static bool bViscous()   {return s_bViscous;}
        static void setViscous(bool b) {s_bViscous=b;}

        static bool bInitBL()    {return s_bInitBL;}
        static void setInitBL(bool b) {s_bInitBL=b;}

        static double CdError() {return s_CdError;}
        static double setCdError(double cderr) {return s_CdError=cderr;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        int loop();
        bool alphaSequence(bool bAlpha);
        bool thetaSequence();
        bool ReSequence();
        void addXFoilData(OpPoint *pOpp, XFoil &xfoil, const Foil *pFoil);

    public slots:
        void run();


    signals:
        void outputMessage(QString const &msg) const;


    public:
        QString m_XFoilLog;
        QTextStream m_XFoilStream;

    private:
        XFoil m_XFoilInstance;     /**< An instance of the XFoil class specific for this object */

    public:
        bool m_bErrors;

        QObject *m_pParent;


    protected:
        Foil *m_pFoil;                 /**< A pointer to the instance of the Foil object for which the calculation is performed */
        Polar *m_pPolar;                /**< A pointer to the instance of the Polar object for which the calculation is performed */

        xfl::enumAnalysisStatus m_AnalysisStatus;

        QVector<AnalysisRange> m_AnalysisRange;

        QTextStream m_OutStream;
        QString m_OutMessage;           /**< Alternate feedback method to signal logmsg() */


        static bool s_bSkipPolar;
        static bool s_bCancel;          /**< True if the user has asked to cancel the analysis */
        static bool s_bSkipOpp;

        static int  s_IterLim;
        static bool s_bAutoInitBL;      /**< true if the BL initialization is left to the code's decision */
        static bool s_bViscous;           /**< true if performing a viscous calculation, false if inviscid */
        static bool s_bAlpha;             /**< true if performing an analysis based on aoa, false if based on Cl */
        static bool s_bInitBL;            /**< true if the boundary layer should be initialized for the next xfoil calculation */
        static bool s_bStoreOpp;          /**< true if the operating points should be stored */
        static double s_CdError;          /**< discard points with |Cd| less than this value: these operating points are likely erroneous (spurious?) */


};




