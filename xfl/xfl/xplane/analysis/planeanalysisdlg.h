/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QLabel>
#include <QElapsedTimer>
#include <QDialog>
#include <QProgressBar>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>

#include <xflobjects/objects3d/analysis/planeopp.h>
#include <xflcore/t8opp.h>

class PlaneTask;
class XPlane;
class PlaneXfl;
class WPolar;
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
        PlaneTask *analyze(Plane *pPlane, WPolar *pWPolar, QVector<double> const &opplist, const QVector<T8Opp> &ranges);
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
        void analysisFinished(WPolar*);

    protected slots:
        void onCancelClose();
        void onKeepOpenErrors();
        void onLiveVortons();
        void onOutputMessage(const QString &msg);
        void onStopIterations();
        void onTaskFinished();

    protected:
        void setupLayout();
        void cleanUp();
        void storePOpps();
        void cancelTask(PlaneTask *pTask);

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

