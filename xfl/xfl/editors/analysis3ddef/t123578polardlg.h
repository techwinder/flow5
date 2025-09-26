/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

/**
 *@file This file contains the definition of the class WPolarDlg which is used to define the data for a WPolar object.
 */

#pragma once


#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QStackedWidget>
#include <QRadioButton>
#include <QStandardItemModel>
#include <QLineEdit>


#include <xfl/editors/analysis3ddef/planepolardlg.h>


class CPTableView;
class CtrlTableModel;


class T123578PolarDlg : public PlanePolarDlg
{
    Q_OBJECT

    public:
        T123578PolarDlg(QWidget *pParent);
        void initPolar3dDlg(Plane const *pPlane, WPolar const *pWPolar=nullptr) override;
        void setType7Polar();

    private:

        void connectSignals();

        void readData() override;
        void enableControls() override;

        void setReynolds();
        void setupLayout();
        void setWingLoad();

        void fillAVLCtrlList();
        void fillAVLGains();
        void readAVLCtrls();

    private slots:

        void onAVLContextMenu(QPoint pt);
        void onAVLRowChanged(QModelIndex index);
        void onAppendAVLCtrl();
        void onDuplicateAVLCtrl();
        void onDeleteAVLCtrl();
        void onAVLCtrlChanged();
        void onAVLGainChanged();
        void onMoveAVLCtrl();

        void onOK() override;
        void onPolarType();
        void onEditingFinished() override;


    private:

        FloatEdit *m_pfeQInf;
        FloatEdit *m_pdeAlphaSpec, *m_pdePhiSpec;

        QRadioButton *m_prbType1, *m_prbType2, *m_prbType3, *m_prbType4, *m_prbType5, *m_prbType7, *m_prbType8;

        QFrame *m_pfrAlpha, *m_pfrPhi, *m_pfrQInf, *m_frFlightInfo;
        QLabel *m_plabGlide, *m_plabVh;

        QLabel *m_plabReInfo;
        QLabel *m_plabWingLoad;

        CPTableView *m_pcptAVLCtrls;
        QStandardItemModel *m_pAVLCtrlModel;
        CPTableView *m_pcptAVLGains;
        CtrlTableModel *m_pAVLGainModel;
        CtrlTableDelegate *m_pAVLGainDelegate;

};








