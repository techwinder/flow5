/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QSettings>

#include <xflcore/analysisrange.h>


class XDirect;
class FloatEdit;
class AnalysisRangeTable;

class Analysis2dCtrls : public QWidget
{
	Q_OBJECT

    public:
        Analysis2dCtrls(QWidget *pParent = nullptr);

        void enableAnalyze(bool b);

        QVector<AnalysisRange> ranges() const;

        static void setXDirect(XDirect *pXDirect) {s_pXDirect = pXDirect;}

    private:
        void setupLayout();
        void connectSignals();
        void fillAnalysisTable();

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(50,500);}

    public slots:
        void onReadAnalysisData();
        void onSetControls();

    private slots:
        void onStoreOpp();
        void onSpec();
        void onViscous();
        void onInputChanged();

    private:
        QRadioButton *m_prbSpec1, *m_prbSpec2, *m_prbSpec3, *m_prbSpec4;
        AnalysisRangeTable *m_pRangeTable;
        QPushButton *m_ppbAnalyze;

        QCheckBox *m_pchViscous;
        QCheckBox *m_pchInitBL;
        QCheckBox *m_pchStoreOpp;

        static XDirect* s_pXDirect;
};

