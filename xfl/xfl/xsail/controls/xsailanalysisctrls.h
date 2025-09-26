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

#include <xflcore/analysisrange.h>


class AnalysisRangeTable;

class XSail;
class BoatPolar;

class XSailAnalysisCtrls : public QWidget
{
    Q_OBJECT

    friend class XSail;

    public:
        XSailAnalysisCtrls(XSail *pXSail);

        void showEvent(QShowEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

        QVector<double> oppList() const;

        void setPolar3d(BoatPolar const *pBtPolar);

        static void setXPlane(XSail*pXSail) {s_pXSail=pXSail;}

    private:
        void setupLayout();
        void connectSignals();
        void setAnalysisRange();

    private slots:
        void onStoreBtOpp();
        void onSetControls();

    private:
        AnalysisRangeTable *m_pAnalysisRangeTable;


        QCheckBox *m_pchStoreBtOpps;
        QPushButton *m_ppbAnalyze;

        static XSail *s_pXSail;
};


