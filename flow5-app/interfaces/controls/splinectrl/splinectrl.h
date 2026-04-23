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
#include <QGroupBox>
#include <QComboBox>
#include <QSlider>
#include <QPushButton>
#include <QCheckBox>
#include <QModelIndex>


#include <api/linestyle.h>

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

