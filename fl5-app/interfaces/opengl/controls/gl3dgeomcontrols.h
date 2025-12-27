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

#include <QObject>
#include <QWidget>
#include <QCheckBox>
#include <QToolButton>
#include <QPushButton>
#include <QSlider>
#include <QFrame>
#include <QSettings>

#include <interfaces/opengl/controls/gl3dcontrols.h>

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

        void setHighlighting(bool b) {m_pchHighlight->setChecked(b);}
        bool isHighlighting() {return m_pchHighlight->isChecked();}

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



