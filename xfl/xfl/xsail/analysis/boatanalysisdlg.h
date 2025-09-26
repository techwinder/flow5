/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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
        BoatTask *analyze(Boat *pBoat, BoatPolar *pBtPolar, const QVector<double> &opplist);
        void deleteTask(BoatTask *pTask);
        bool isAnalysisRunning() const;
        bool hasErrors() const {return m_bHasErrors;}

        QVector<BoatOpp*> const &btOppList() const  {return m_BtOppList;}
        BoatOpp* lastBtOpp() const {if(m_BtOppList.size()) return m_BtOppList.last(); else return nullptr;}

        void clearBtOppList() {m_BtOppList.clear();}

        QSize sizeHint() const override {return QSize(800, 500);}


        static void setXSail(XSail*pXSail) {s_pXSail=pXSail;}

        void cancelTask(BoatTask *pTask);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    signals:
        void analysisFinished();

    protected slots:
        void onCancelAnalysis();
        void onTaskFinished();
        void onCancelClose();
        void onLiveVortons();
        void onKeepOpenErrors();
        void onStopIterations();
        void onOutputMessage(QString const &msg);

    protected:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;

        void setupLayout();
        void cleanUp();

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


        QVector<BoatOpp*> m_BtOppList;
        QDialogButtonBox *m_pButtonBox;


        static XSail *s_pXSail;
        static QByteArray s_WindowGeometry;
};

