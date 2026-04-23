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
#include <QElapsedTimer>
#include <QLabel>
#include <QSettings>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>

class PlainTextOutput;
class Panel3;
class Boat;
class BoatPolar;
class BoatOpp;
class BoatTask;
class XSail;


class BoatAnalysisDlg : public QDialog
{
    Q_OBJECT

    public:
        BoatAnalysisDlg();
        virtual ~BoatAnalysisDlg();

        void setTask(BoatTask *pTask) {m_pActiveTask = pTask;}
        BoatTask *analyze(Boat *pBoat, BoatPolar *pBtPolar, const std::vector<double> &opplist);
        void deleteTask(BoatTask *pTask);
        bool isAnalysisRunning() const;
        bool hasErrors() const {return m_bHasErrors;}

        std::vector<BoatOpp*> const &btOppList() const  {return m_BtOppList;}
        BoatOpp* lastBtOpp() const {if(m_BtOppList.size()) return m_BtOppList.back(); else return nullptr;}

        void clearBtOppList() {m_BtOppList.clear();}

        QSize sizeHint() const override {return QSize(800, 500);}


        static void setXSail(XSail*pXSail) {s_pXSail=pXSail;}

        void cancelTask(BoatTask *pTask);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    signals:
        void analysisFinished();

    protected slots:
        void onTaskFinished();
        void onCancelClose();
        void onLiveVortons();
        void onKeepOpenErrors();
        void onStopIterations();
        void onOutputStdMessage(std::string const &msg) {onOutputMessage(QString::fromStdString(msg));}
        void onOutputMessage(QString const &msg);

    private:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;

        void setupLayout();

        void cleanUp();
        void runAsync();

    private:
        BoatTask *m_pActiveTask; /**< a pointer to the one and only instance of the PlaneAnalysisTask class */

        QElapsedTimer m_Clock;

        QCheckBox *m_pchLiveVortons;
        QCheckBox *m_pchKeepOpenOnErrors;
        PlainTextOutput *m_ppto;
        QPushButton *m_ppbStopIter;
        QPushButton *m_ppbCloseDialog;
        QLabel *m_plabTaskInfo;

        bool m_bHasErrors;

        QVector<Panel3> m_WakePanel3; //debug only


        std::vector<BoatOpp*> m_BtOppList;
        QDialogButtonBox *m_pButtonBox;


        static XSail *s_pXSail;
        static QByteArray s_WindowGeometry;
};

