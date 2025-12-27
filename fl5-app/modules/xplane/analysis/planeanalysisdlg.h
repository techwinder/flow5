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

#include <QLabel>
#include <QElapsedTimer>
#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QSettings>

#include <api/planeopp.h>
#include <api/t8opp.h>

class PlaneTask;
class XPlane;
class PlaneXfl;
class PlanePolar;
//class PlaneOpp;
class PlainTextOutput;
class Panel3;
class Panel4;

class PlaneAnalysisDlg : public QDialog
{
    Q_OBJECT

    public:
        PlaneAnalysisDlg(QWidget *pParent=nullptr);

        void setTask(PlaneTask *pTask){m_pActiveTask = pTask;}
        PlaneTask *analyze(Plane *pPlane, PlanePolar *pPlPolar, const std::vector<double> &opplist, const std::vector<T8Opp> &ranges);
        bool isAnalysisRunning() const;
        bool hasErrors() const {return m_bHasErrors;}

        QSize sizeHint() const override {return QSize(750,375);}
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;

        PlaneOpp *lastPOpp() const {return m_pLastPOpp;}

        PlaneTask const*activeTask() const {return m_pActiveTask;}
        bool bIsRunning() const;

        static bool loadSettings(QSettings &settings);
        static bool saveSettings(QSettings &settings);
        static void setXPlane(XPlane*pXPlane) {s_pXPlane=pXPlane;}


    signals:
        void analysisFinished(PlanePolar*);

    protected slots:
        void onCancelClose();
        void onKeepOpenErrors();
        void onLiveVortons();
        void onOutputMessage(const QString &msg);
        void onStopIterations();
        void onTaskFinished();

    private:
        void setupLayout();
        void cleanUp();
        void cancelTask(PlaneTask *pTask);

        void runAsync();

    private:
        PlaneTask *m_pActiveTask; /**< a pointer to the one and only instance of the PlaneAnalysisTask class */

        QElapsedTimer m_Clock;

        QCheckBox *m_pchKeepOpenOnErrors;
        QCheckBox *m_pchLiveVortons;

        QLabel *m_plabTaskInfo;
        PlainTextOutput *m_ppto;
        QPushButton *m_ppbStopIter;
        QPushButton *m_ppbCloseDialog;

        QDialogButtonBox *m_pButtonBox;

        bool m_bHasErrors;

        /** @todo replace with planeopplist.back() */
        PlaneOpp *m_pLastPOpp;

        static XPlane *s_pXPlane;
        static QByteArray s_Geometry;
};

