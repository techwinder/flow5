/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QDialog>
#include <QGroupBox>
#include <QComboBox>
#include <QSlider>
#include <QPushButton>
#include <QCheckBox>
#include <QModelIndex>


#include <xflcore/linestyle.h>

class XflDelegate;
class ActionItemModel;
class CPTableView;
class DFoil;
class IntEdit;
class LineBtn;
class Spline;


class SplineCtrl : public QFrame
{
    Q_OBJECT

    public:
        SplineCtrl(QWidget *pParent=nullptr);

        void initSplineCtrls(Spline *pSpline);
        Spline *spline() {return m_pSpline;}

        void showPointTable(bool bShow);
        void showBunchBox(bool bShow) {m_pBunchBox->setVisible(bShow);}

        void setEnabledClosedTE(bool bEnable) {m_pchClosedTE->setEnabled(bEnable);}
        void setEnabledForceSym(bool bEnable) {m_pchSymmetric->setEnabled(bEnable);}

        bool bForcedsymmetric() const {return m_pchSymmetric->isChecked();}
        bool bClosed() const {return m_pchClosedTE->isChecked();}

    public slots:
        void fillPointModel();

    protected:
        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void connectSignals();

        void resizeColumns();

        void readData();
        void setupLayout();
        void updateSplines();

    protected slots:
        void onBunchSlide(int);
        void onClosedTE();
        void onCtrlPointTableClicked(QModelIndex index);
        void onCurrentRowChanged(QModelIndex, QModelIndex);
        void onDelete();
        void onForcesymmetric();
        void onInsertAfter();
        void onInsertBefore();
        void onSplineStyle(LineStyle);
        void onUpdate();

    signals:
        void splineChanged();
        void pointSelChanged();

    protected:

        QGroupBox *m_pBunchBox, *m_pSplineParamBox;
        QSlider *m_pslBunchAmp;

        QGroupBox *m_pOuputFrame;
        IntEdit	*m_pieOutputPoints;
        QComboBox *m_pcbSplineDegree;
        QCheckBox *m_pchSymmetric;
        QCheckBox *m_pchShow, *m_pchShowNormals;

        CPTableView *m_pcptPoint;
        ActionItemModel *m_pPointModel;
        XflDelegate *m_pPointFloatDelegate;

        LineBtn *m_plbSplineStyle;

        QCheckBox *m_pchClosedTE;

        Spline *m_pSpline;
};

