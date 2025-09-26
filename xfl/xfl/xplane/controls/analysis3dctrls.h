/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSettings>
#include <QStandardItemModel>
#include <QStackedWidget>



#include <xflcore/analysisrange.h>
#include <xflcore/t8opp.h>

class IntEdit;
class FloatEdit;
class XPlane;


class AnalysisRangeTable;
class T8RangeTable;

class Analysis3dCtrls : public QWidget
{
    Q_OBJECT
//    friend class XPlane;

    public:
        Analysis3dCtrls(QWidget *pParent=nullptr);

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

        void enableAnalyze(bool b) {m_ppbAnalyze->setEnabled(b);}

        void setParameterLabels();
        double minValue() const;
        double maxValue() const;
        double incValue() const;
        bool isSequentialAnalysis() const;
        void setAnalysisRange();

        QVector<double> oppList() const;
        void getXRanges(QVector<T8Opp> &ranges) const;

        static void setXPlane(XPlane*pXPlane) {s_pXPlane=pXPlane;}

    private:
        void setupLayout();
        void connectSignals();

    public slots:
        void onSetControls();

    private slots:
        void onStorePOpps();


    private:
        AnalysisRangeTable *m_pAnalysisRangeTable;
        T8RangeTable *m_pXRangeTable;

        QStackedWidget *m_pswTables;

        QCheckBox *m_pchStorePOpps;


        QLabel *m_plabParamName;


        static XPlane *s_pXPlane;

    public:
        QPushButton *m_ppbAnalyze;
};

