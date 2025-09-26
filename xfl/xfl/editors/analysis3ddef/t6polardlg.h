/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QSettings>

#include <xfl/editors/analysis3ddef/planepolardlg.h>
#include <xfl/editors/analysis3ddef/ctrltablemodel.h>


class Plane;
class WingXfl;
class WPolar;
class DoubleEdit;
class IntEdit;
class CPTableView;
class CtrlTableDelegate;



class T6PolarDlg : public PlanePolarDlg
{
    Q_OBJECT

    public:
        T6PolarDlg(QWidget *pParent);
        ~T6PolarDlg();

        void initPolar3dDlg(const Plane *pPlane, const WPolar *pWPolar=nullptr) override;

    private:
        void setupLayout();
        void connectSignals();
        void resizeColumns() override;


    private slots:
        void onOK() override;
        void onPlaneInertia() override;
        void onEditingFinished() override;
        void onAdjustedVelocity();
        void onRangeCellChanged();
        void onInertiaCellChanged();
        void onAngleCellChanged();

    private:
        void enableControls() override;
        //	void setAutoWPolarName(WPolar * pWPolar, Plane *pPlane);

        void fillOppRangePage();
        void fillAngleControlList();
        void fillInertiaPage() override;
        void readAngleRangeData();
        void readOperatingData();
        void readInertiaData();
        void readData() override;
        void setViscous();


    private:
        CPTableView *m_pcptOppRange;
        CtrlTableModel *m_pOppRangeControlModel;
        CtrlTableDelegate *m_pOppRangeCtrlDelegate;

        CPTableView *m_pcptInertia;
        CtrlTableModel *m_pInertiaControlModel;
        CtrlTableDelegate *m_pInertiaCtrlDelegate;

        CPTableView *m_pcptAngles;
        CtrlTableModel *m_pAngleControlModel;
        CtrlTableDelegate *m_pAngleCtrlDelegate;

        QCheckBox *m_pchAdjustedVelocity;

};




