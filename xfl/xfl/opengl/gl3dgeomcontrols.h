/****************************************************************************

	flow5 application
	Copyright (C) Andre Deperrois 
	All rights reserved.

*****************************************************************************/

#pragma once

#include <QObject>
#include <QWidget>
#include <QCheckBox>
#include <QToolButton>
#include <QPushButton>
#include <QSlider>
#include <QFrame>
#include <QSettings>

#include <xfl3d/controls/gl3dcontrols.h>

enum TypeLayout {HLayout, VLayout, XPlaneLayout, PlaneLayout, BoatLayout, FuseLayout, SailLayout, WingLayout, InertiaLayout, OptimCpLayout};

class gl3dXflView;

class gl3dGeomControls : public gl3dControls
{
    friend class OptimCp3d;

    Q_OBJECT
    public:
        gl3dGeomControls(gl3dXflView *pgl3dview, Qt::Orientation orientation, bool bRowLayout, QWidget *parent = nullptr);
        gl3dGeomControls(gl3dXflView *pgl3dview, TypeLayout typelayout, bool bRowLayout, QWidget *parent = nullptr);

    public:
        void connectSignals();
        void setupLayout(Qt::Orientation orientation, bool bInColums);

        void setupPlaneLayout();
        void setupWingLayout();
        void setupFuseLayout();
        void setupSailLayout();
        void setupInertiaLayout();
        void setupOptimCpLayout();

        bool getDistance() const {return m_ptbDistance->isChecked();}
        void stopDistance();

        bool bShowCornerPts() const {return m_pchCornerPts->isChecked();}

        void setControls() override;
        void showMasses(bool bShow);

        void checkPanels(bool b) {m_pchPanels->setChecked(b);}
        void checkSurfaces(bool b) {m_pchSurfaces->setChecked(b);}


        void disableFoilCtrl() {m_pchFoilNames->setEnabled(false);}
        void showTessCtrl(bool bShow)     {m_pchTessellation->setVisible(bShow);}
        void showNormalCtrl(bool bShow)     {m_pchNormals->setVisible(bShow);}
        void showHighlightCtrl(bool bShow)  {m_pchHighlight->setVisible(bShow);}

        void enableCtrlPts(bool bEnabled) {m_pchCtrlPts->setEnabled(bEnabled);}

        void checkHighlight(bool b) {m_pchHighlight->setChecked(b);}

    private:
        void hideControls();
        void makeControls();


    protected:
        gl3dXflView *m_pgl3dXflView;


    private:
        QCheckBox *m_pchAxes, *m_pchSurfaces, *m_pchOutline, *m_pchPanels;
        QCheckBox *m_pchFoilNames, *m_pchMasses, *m_pchTessellation;
        QCheckBox *m_pchNormals;
        QCheckBox *m_pchHighlight;
        QCheckBox *m_pchCtrlPts;
        QCheckBox *m_pchCornerPts;

        QCheckBox *m_pchCpSections, *m_pchCp, *m_pchCpIsobars;
};



