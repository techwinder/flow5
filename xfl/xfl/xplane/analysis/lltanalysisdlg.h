/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QDialog>
#include <QTimer>
#include <QPushButton>
#include <QCheckBox>
#include <QString>
#include <QFile>
#include <QTextEdit>
#include <QSplitter>
#include <QSettings>

#include <xflcore/flow5events.h>

class XPlane;
class Graph;
class GraphWt;
class PlaneXfl;
class WPolar;
class PlaneOpp;
class WingXfl;
class LLTTask;
class PlaneTask;
class PlainTextOutput;

/**
 *@class LLTAnalysisDlg
 *@brief The class is used to launch the LLT and to manage the progress of the analysis.

 It successively :
  - creates a single instance of the LLTAnalysis object,
  - initializes the data,
  - launches the analysis
  - displays the progress,
  - stores the results in the OpPoint and Polar objects
  - updates the display in the Miarex view.

 The LLTAnalysis class performs the calculation of a signle OpPoint. The loop over a sequence of aoa, Cl, or Re values
 are performed in the LLAnalysisDlg Class.
*/
class LLTAnalysisDlg : public QDialog
{
    Q_OBJECT

    public:
        LLTAnalysisDlg(QWidget *pParent);
        ~LLTAnalysisDlg() override;

        void initDialog(PlaneXfl *pPlane, WPolar *pWPolar, const QVector<double> &opplist);

        void analyze();
        void cleanUp();

        bool hasErrors() const {return m_bHasErrors;}

        void outputMessage(QString const &msg);

        PlaneOpp *lastPOpp() const {return m_pLastPOpp;}

        QSize sizeHint() const override {return QSize(900,700);}

        static void setXPlane(XPlane* pXPlane) {s_pXPlane=pXPlane;}
        static bool loadSettings(QSettings &settings);
        static bool saveSettings(QSettings &settings);

    signals:
        void analysisFinished(WPolar*);


    private slots:
        void onCancelAnalysis();
        void onClearCurves();
        void onTaskFinished();
        void onOutputMessage(QString const&msg);

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;

        bool alphaLoop();
        bool QInfLoop();
        void setupLayout();
        void storePOpps();
        void updateView();


    private:

        static XPlane *s_pXPlane;

        LLTTask *m_pTheLLTTask; /**< a pointer to the one and only instance of the PlaneAnalysisTask class */

        PlaneOpp *m_pLastPOpp;

        bool m_bHasErrors;
        bool m_bFinished;           /**< true if the analysis is completed, false if it is running */
        Graph *m_pIterGraph;         /**< A pointer to the QGraph object where the progress of the iterations are displayed */

        //GUI widget variables
        QPushButton *m_ppbCancel, *m_ppbClearCurves;
        GraphWt * m_pGraphWt;
        PlainTextOutput *m_ppto;
        QCheckBox * m_pchKeepOpenOnErrors;

        QSplitter *m_pHSplitter;

        static QByteArray s_Geometry;
        static QByteArray s_HSplitterSizes;

};

