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


#include <QDialog>
#include <QTimer>
#include <QPushButton>
#include <QCheckBox>
#include <QString>
#include <QFile>
#include <QTextEdit>
#include <QSplitter>
#include <QSettings>

#include <api/flow5events.h>

class XPlane;
class Graph;
class GraphWt;
class PlaneXfl;
class PlanePolar;
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

        void initDialog(PlaneXfl *pPlane, PlanePolar *pWPolar, const std::vector<double> &opplist);

        void analyze();
        void cleanUp();

        bool hasErrors() const {return m_bHasErrors;}

        void outputMessage(QString const &msg);
        void outputStdMessage(std::string const &msg);

        PlaneOpp *lastPOpp() const {return m_pLastPOpp;}

        QSize sizeHint() const override {return QSize(900,700);}

        static void setXPlane(XPlane* pXPlane) {s_pXPlane=pXPlane;}
        static bool loadSettings(QSettings &settings);
        static bool saveSettings(QSettings &settings);

    signals:
        void analysisFinished(PlanePolar*);


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
        void updateView();

        void runAsync();

    private:

        static XPlane *s_pXPlane;

        LLTTask *m_pLLTTask; /**< a pointer to the one and only instance of the PlaneAnalysisTask class */

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

