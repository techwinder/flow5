/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


/** @file
 * This file implements the interface for the analysis of the active Foil and Polar objects in QXDirect.
*/

#pragma once

#include <QDialog>
#include <QSettings>
#include <QFile>
#include <QTextEdit>

#include <QCheckBox>


class XFoilTask;
class Foil;
class Polar;
class OpPoint;
class PlainTextOutput;
struct AnalysisRange;

/**
* @class XFoilAnalysisDlg
* This class provides an interface to manage the analysis of the active Foil and Polar pair.
*/
class XFoilAnalysisDlg : public QDialog
{
	Q_OBJECT

    public:
        XFoilAnalysisDlg(QWidget *pParent=nullptr);
        ~XFoilAnalysisDlg();

        void initializeAnalysis(Foil *pFoil, Polar *pPolar, const QVector<AnalysisRange> &ranges);
        void analyze();

        void customEvent(QEvent * pEvent) override;

        OpPoint *lastOpp() {return m_pLastOpp;}
        bool hasErrors() const {return m_bErrors;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onCancelClose();
        void onKeepOpen(bool bChecked);
        void onSkipPoint();
        void onProgress();
        void onOutputMessage(const QString &msg);
        void onTaskFinished();



    signals:
        void analysisFinished();

    private:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        QSize sizeHint() const override;

        void setFileHeader();
        void setupLayout();

   private:

        //variables
        PlainTextOutput *m_ppto;
        QPushButton* m_ppbCancel, *m_ppbSkip; /** @todo ButtonBox */
        QCheckBox* m_pchKeepOpen;

        bool m_bErrors;                /**< true if some points are unconverged. Used by the calling class to know if the window should be kept visible at the end of the analysis.>*/

        QFile *m_pXFile;               /**< a pointer to the log file>*/

        XFoilTask *m_pXFoilTask;       /**< A pointer to the instance of the XFoilTask associated to this analysis. >*/

        Foil *m_pFoil;
        Polar *m_pPolar;

        OpPoint *m_pLastOpp;


        static QByteArray s_WindowGeometry;
};









