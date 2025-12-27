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




#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QLabel>

#include "gl3dgeomcontrols.h"
#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <interfaces/opengl/controls/fine3dcontrols.h>


gl3dGeomControls::gl3dGeomControls(gl3dXflView *pgl3dview, Qt::Orientation orientation, bool bRowLayout, QWidget *)
    : gl3dControls(pgl3dview)
{
    m_pgl3dXflView = pgl3dview;

    makeControls();

    setupLayout(orientation, bRowLayout);
    connectSignals();
}



gl3dGeomControls::gl3dGeomControls(gl3dXflView *pgl3dview, TypeLayout layout, bool bRowLayout, QWidget *)
    : gl3dControls(pgl3dview)
{
    m_pgl3dXflView = pgl3dview;

    makeControls();

    switch(layout)
    {
        case PlaneLayout:           setupPlaneLayout();           break;
        case WingLayout:            setupWingLayout();            break;
        case FuseLayout:            setupFuseLayout();            break;
        case BoatLayout:
        case SailLayout:            setupSailLayout();            break;
        case InertiaLayout:         setupInertiaLayout();         break;
        case OptimCpLayout:         setupOptimCpLayout();         break;
        default:
            setupLayout(Qt::Horizontal, bRowLayout); break;
    }

    connectSignals();
}


void gl3dGeomControls::connectSignals()
{
    connectBaseSignals();

    connect(m_ptbReset,        SIGNAL(clicked()),      m_pgl3dView, SLOT(on3dReset()));
    connect(m_pchAxes,         SIGNAL(clicked(bool)),  m_pgl3dView, SLOT(onAxes(bool)));
    connect(m_pchPanels,       SIGNAL(clicked(bool)),  m_pgl3dView, SLOT(onPanels(bool)));
    connect(m_pchSurfaces,     SIGNAL(clicked(bool)),  m_pgl3dView, SLOT(onSurfaces(bool)));
    connect(m_pchOutline,      SIGNAL(clicked(bool)),  m_pgl3dView, SLOT(onOutline(bool)));
    connect(m_pchFoilNames,    SIGNAL(clicked(bool)),  m_pgl3dView, SLOT(onFoilNames(bool)));
    connect(m_pchMasses,       SIGNAL(clicked(bool)),  m_pgl3dView, SLOT(onShowMasses(bool)));
    connect(m_pchTessellation, SIGNAL(clicked(bool)),  m_pgl3dView, SLOT(onTessellation(bool)));
    connect(m_pchCtrlPts,      SIGNAL(clicked(bool)),  m_pgl3dView, SLOT(onCtrlPoints(bool)));
    connect(m_pchCornerPts,    SIGNAL(clicked(bool)),  m_pgl3dView, SLOT(onCornerPts(bool)));
    connect(m_pchNormals,      SIGNAL(clicked(bool)),  m_pgl3dView, SLOT(onNormals(bool)));
    connect(m_pchHighlight,    SIGNAL(clicked(bool)),  m_pgl3dView, SLOT(onHighlightPanel(bool)));
}


void gl3dGeomControls::makeControls()
{
    m_pchAxes         = new QCheckBox("Axes",            this);
    m_pchSurfaces     = new QCheckBox("Surfaces",        this);
    m_pchOutline      = new QCheckBox("Outline",         this);
    m_pchPanels       = new QCheckBox("Panels",          this);
    m_pchFoilNames    = new QCheckBox("Foils",           this);
    m_pchMasses       = new QCheckBox("Masses",          this);
    m_pchTessellation = new QCheckBox("Tessellation",    this);
    m_pchNormals      = new QCheckBox("Normals",         this);
    m_pchHighlight    = new QCheckBox("Highlight panel", this);
    m_pchCtrlPts      = new QCheckBox("Ctrtl points",    this);
    m_pchCornerPts    = new QCheckBox("Corner points",   this);

    m_pchCpSections = new QCheckBox("Sections");
    m_pchCp         = new QCheckBox("Cp");
    m_pchCpIsobars  = new QCheckBox("Isobars");

    m_pchAxes->setSizePolicy(        QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchSurfaces->setSizePolicy(    QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchOutline->setSizePolicy(     QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchPanels->setSizePolicy(      QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchFoilNames->setSizePolicy(   QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchMasses->setSizePolicy(      QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchTessellation->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchNormals->setSizePolicy(     QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchHighlight->setSizePolicy(   QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchCtrlPts->setSizePolicy(     QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchCornerPts->setSizePolicy(   QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchCpSections->setSizePolicy(  QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchCp->setSizePolicy(          QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pchCpIsobars->setSizePolicy(   QSizePolicy::Maximum, QSizePolicy::Maximum);
}


void gl3dGeomControls::hideControls()
{
    m_pchAxes->setVisible(false);
    m_pchSurfaces->setVisible(false);
    m_pchOutline->setVisible(false);
    m_pchPanels->setVisible(false);

    m_pchFoilNames->setVisible(false);
    m_pchMasses->setVisible(false);
    m_pchTessellation->setVisible(false);
    m_pchNormals->setVisible(false);
    m_pchHighlight->setVisible(false);
    m_pchCtrlPts->setVisible(false);
    m_pchCornerPts->setVisible(false);

    m_pchCpSections->setVisible(false);
    m_pchCp->setVisible(false);
    m_pchCpIsobars->setVisible(false);
}


void gl3dGeomControls::setControls()
{
    gl3dControls::setControls();

    m_pchAxes->setChecked(m_pgl3dXflView->bAxes());
    m_pchSurfaces->setChecked(m_pgl3dXflView->bSurfaces());
    m_pchOutline->setChecked(m_pgl3dXflView->bOutline());
    m_pchPanels->setChecked(m_pgl3dXflView->bVLMPanels());
    m_pchFoilNames->setChecked(m_pgl3dXflView->bFoilNames());
    m_pchMasses->setChecked(m_pgl3dXflView->bMasses());
    m_pchTessellation->setChecked(m_pgl3dXflView->bTriangleOutline());
    m_pchCtrlPts->setChecked(m_pgl3dXflView->bCtrlPts());
    m_pchCornerPts->setChecked(m_pgl3dXflView->bCornerPts());

    m_pchHighlight->setChecked(m_pgl3dXflView->bHigh());
    m_pchNormals->setChecked(m_pgl3dXflView->bNormals());
}


void gl3dGeomControls::showMasses(bool bShow)
{
    m_pgl3dXflView->showMasses(bShow);
}


void gl3dGeomControls::stopDistance()
{
    m_ptbDistance->setChecked(false);
    m_ptbDistance->defaultAction()->setChecked(false);
}


void gl3dGeomControls::setupLayout(Qt::Orientation orientation, bool bRowLayout)
{
    hideControls();
    m_pchAxes->setVisible(true);
    m_pchSurfaces->setVisible(true);
    m_pchFoilNames->setVisible(true);
    m_pchHighlight->setVisible(true);
    m_pchNormals->setVisible(true);
    m_pchPanels->setVisible(true);
    m_pchOutline->setVisible(true);
    m_pchMasses->setVisible(true);
    m_pchTessellation->setVisible(true);

    QGridLayout *p3dParamsLayout = new QGridLayout;
    {
        if(orientation==Qt::Horizontal)
        {
            p3dParamsLayout->addWidget(m_pchAxes,           2,1);
            p3dParamsLayout->addWidget(m_pchSurfaces,       2,2);
            p3dParamsLayout->addWidget(m_pchFoilNames,      2,3);
            p3dParamsLayout->addWidget(m_pchHighlight,      2,4);
            p3dParamsLayout->addWidget(m_pchNormals,        2,5);
            p3dParamsLayout->addWidget(m_pchPanels,         3,1);
            p3dParamsLayout->addWidget(m_pchOutline,        3,2);
            p3dParamsLayout->addWidget(m_pchMasses,         3,3);
            p3dParamsLayout->addWidget(m_pchTessellation,   3,4);
            p3dParamsLayout->setRowStretch(1,1);
            p3dParamsLayout->setRowStretch(5,1);
        }
        else
        {
            p3dParamsLayout->addWidget(m_pchAxes,           1,1);
            p3dParamsLayout->addWidget(m_pchPanels,         1,2);
            p3dParamsLayout->addWidget(m_pchSurfaces,       2,1);
            p3dParamsLayout->addWidget(m_pchOutline,        2,2);
            p3dParamsLayout->addWidget(m_pchFoilNames,      3,1);
            p3dParamsLayout->addWidget(m_pchMasses,         3,2);
            p3dParamsLayout->addWidget(m_pchHighlight,      4,1);
            p3dParamsLayout->addWidget(m_pchTessellation,   4,2);
            p3dParamsLayout->addWidget(m_pchNormals,        5,1);
        }
        p3dParamsLayout->setSpacing(0);
    }

    QVBoxLayout *pAxisViewLayout = new QVBoxLayout;
    {
        QHBoxLayout *pTopRowLayout = new QHBoxLayout;
        {
            pTopRowLayout->addStretch();
            pTopRowLayout->addWidget(m_ptbX);
            pTopRowLayout->addWidget(m_ptbY);
            pTopRowLayout->addWidget(m_ptbZ);
            pTopRowLayout->addWidget(m_ptbIso);
            pTopRowLayout->addStretch();
        }
        QHBoxLayout *pBotRowLayout = new QHBoxLayout;
        {
            pBotRowLayout->addStretch();
            pBotRowLayout->addWidget(m_ptbHFlip);
            pBotRowLayout->addWidget(m_ptbVFlip);
            pBotRowLayout->addWidget(m_ptbReset);
            pBotRowLayout->addWidget(m_ptbDistance);
            pBotRowLayout->addWidget(m_ptbFineCtrls);
            pBotRowLayout->addStretch();
        }
        pAxisViewLayout->addStretch();
        pAxisViewLayout->addLayout(pTopRowLayout);
        pAxisViewLayout->addLayout(pBotRowLayout);
        pAxisViewLayout->addStretch();
    }

    if(orientation==Qt::Horizontal || bRowLayout)
    {
        QHBoxLayout *p3dViewControlsLayout = new QHBoxLayout;
        {
            p3dViewControlsLayout->addLayout(p3dParamsLayout);
            p3dViewControlsLayout->addLayout(pAxisViewLayout);
            p3dViewControlsLayout->addWidget(m_pFineCtrls);
        }
        setLayout(p3dViewControlsLayout);
    }
    else
    {
        QVBoxLayout *p3dViewControlsLayout = new QVBoxLayout;
        {
            p3dViewControlsLayout->addLayout(p3dParamsLayout);
            p3dViewControlsLayout->addLayout(pAxisViewLayout);
            p3dViewControlsLayout->addWidget(m_pFineCtrls);
        }
        setLayout(p3dViewControlsLayout);
    }
}


void gl3dGeomControls::setupPlaneLayout()
{
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Maximum);

    hideControls();
    m_pchAxes->setVisible(true);
    m_pchPanels->setVisible(true);
    m_pchSurfaces->setVisible(true);
    m_pchFoilNames->setVisible(true);
    m_pchOutline->setVisible(true);
    m_pchMasses->setVisible(true);
    m_pchHighlight->setVisible(true);

    QHBoxLayout *p3dViewControlsLayout = new QHBoxLayout;
    {
        QGridLayout *p3dParamsLayout = new QGridLayout;
        {
            p3dParamsLayout->addWidget(m_pchAxes,           2,1);
            p3dParamsLayout->addWidget(m_pchSurfaces,       2,2);
            p3dParamsLayout->addWidget(m_pchFoilNames,      2,3);
            p3dParamsLayout->addWidget(m_pchHighlight,      2,4);
            p3dParamsLayout->addWidget(m_pchPanels,         3,1);
            p3dParamsLayout->addWidget(m_pchOutline,        3,2);
            p3dParamsLayout->addWidget(m_pchMasses,         3,3);
            p3dParamsLayout->setRowStretch(1,1);
            p3dParamsLayout->setRowStretch(5,1);

    //        pThreeDParamsLayout->setContentsMargins(0,0,0,0);
            p3dParamsLayout->setSpacing(0);
        }

        QVBoxLayout *pAxisViewLayout = new QVBoxLayout;
        {
            QHBoxLayout *pTopRowLayout = new QHBoxLayout;
            {
                pTopRowLayout->addStretch();
                pTopRowLayout->addWidget(m_ptbX);
                pTopRowLayout->addWidget(m_ptbY);
                pTopRowLayout->addWidget(m_ptbZ);
                pTopRowLayout->addWidget(m_ptbIso);
                pTopRowLayout->addStretch();
            }
            QHBoxLayout *pBotRowLayout = new QHBoxLayout;
            {
                pBotRowLayout->addStretch();
                pBotRowLayout->addWidget(m_ptbHFlip);
                pBotRowLayout->addWidget(m_ptbVFlip);
                pBotRowLayout->addWidget(m_ptbReset);
                pBotRowLayout->addWidget(m_ptbDistance);
                pBotRowLayout->addWidget(m_ptbFineCtrls);
                pBotRowLayout->addStretch();
            }
            pAxisViewLayout->addStretch();
            pAxisViewLayout->addLayout(pTopRowLayout);
            pAxisViewLayout->addLayout(pBotRowLayout);
            pAxisViewLayout->addStretch();
        }


        p3dViewControlsLayout->addLayout(p3dParamsLayout);
        p3dViewControlsLayout->addLayout(pAxisViewLayout);
        p3dViewControlsLayout->addWidget(m_pFineCtrls);
    }
    setLayout(p3dViewControlsLayout);
}


void gl3dGeomControls::setupWingLayout()
{
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Maximum);

    hideControls();
    m_pchAxes->setVisible(true);
    m_pchPanels->setVisible(true);
    m_pchSurfaces->setVisible(true);
    m_pchFoilNames->setVisible(true);
    m_pchOutline->setVisible(true);
    m_pchMasses->setVisible(true);
    m_pchHighlight->setVisible(true);
    m_pchTessellation->setVisible(true);

    QVBoxLayout *p3dViewControlsLayout = new QVBoxLayout;
    {
        QGridLayout *p3dParamsLayout = new QGridLayout;
        {
            p3dParamsLayout->addWidget(m_pchAxes,           1,1);
            p3dParamsLayout->addWidget(m_pchPanels,         1,2);
            p3dParamsLayout->addWidget(m_pchSurfaces,       2,1);
            p3dParamsLayout->addWidget(m_pchOutline,        2,2);
            p3dParamsLayout->addWidget(m_pchFoilNames,      3,1);
            p3dParamsLayout->addWidget(m_pchMasses,         3,2);
            p3dParamsLayout->addWidget(m_pchHighlight,      4,1);
            p3dParamsLayout->addWidget(m_pchTessellation,   4,2);
            p3dParamsLayout->setRowStretch(1,1);
            p3dParamsLayout->setRowStretch(5,1);

    //        pThreeDParamsLayout->setContentsMargins(0,0,0,0);
            p3dParamsLayout->setSpacing(0);
        }

        QVBoxLayout *pAxisViewLayout = new QVBoxLayout;
        {
            QHBoxLayout *pTopRowLayout = new QHBoxLayout;
            {
                pTopRowLayout->addStretch();
                pTopRowLayout->addWidget(m_ptbX);
                pTopRowLayout->addWidget(m_ptbY);
                pTopRowLayout->addWidget(m_ptbZ);
                pTopRowLayout->addWidget(m_ptbIso);
                pTopRowLayout->addStretch();
            }
            QHBoxLayout *pBotRowLayout = new QHBoxLayout;
            {
                pBotRowLayout->addStretch();
                pBotRowLayout->addWidget(m_ptbHFlip);
                pBotRowLayout->addWidget(m_ptbVFlip);
                pBotRowLayout->addWidget(m_ptbReset);
                pBotRowLayout->addWidget(m_ptbDistance);
                pBotRowLayout->addWidget(m_ptbFineCtrls);
                pBotRowLayout->addStretch();
            }
            pAxisViewLayout->addStretch();
            pAxisViewLayout->addLayout(pTopRowLayout);
            pAxisViewLayout->addLayout(pBotRowLayout);
            pAxisViewLayout->addStretch();
        }


        p3dViewControlsLayout->addLayout(p3dParamsLayout);
        p3dViewControlsLayout->addLayout(pAxisViewLayout);
        p3dViewControlsLayout->addWidget(m_pFineCtrls);
    }
    setLayout(p3dViewControlsLayout);
}


void gl3dGeomControls::setupFuseLayout()
{
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Maximum);

    hideControls();
    m_pchAxes->setVisible(true);
    m_pchPanels->setVisible(true);
    m_pchSurfaces->setVisible(true);
    m_pchOutline->setVisible(true);
    m_pchMasses->setVisible(true);
    m_pchNormals->setVisible(true);
    m_pchTessellation->setVisible(true);
    m_pchHighlight->setVisible(true);

    QHBoxLayout *p3dViewControlsLayout = new QHBoxLayout;
    {
        QGridLayout *p3dParamsLayout = new QGridLayout;
        {
            p3dParamsLayout->addWidget(m_pchAxes,           2,1);
            p3dParamsLayout->addWidget(m_pchSurfaces,       2,2);
            p3dParamsLayout->addWidget(m_pchNormals,        2,3);
            p3dParamsLayout->addWidget(m_pchHighlight,      2,4);
            p3dParamsLayout->addWidget(m_pchPanels,         3,1);
            p3dParamsLayout->addWidget(m_pchOutline,        3,2);
            p3dParamsLayout->addWidget(m_pchMasses,         3,3);
            p3dParamsLayout->addWidget(m_pchTessellation,   3,4);
            p3dParamsLayout->setRowStretch(1,1);
            p3dParamsLayout->setRowStretch(5,1);
            p3dParamsLayout->setSpacing(0);
        }

        QVBoxLayout *pAxisViewLayout = new QVBoxLayout;
        {
            QHBoxLayout *pTopRowLayout = new QHBoxLayout;
            {
                pTopRowLayout->addStretch();
                pTopRowLayout->addWidget(m_ptbX);
                pTopRowLayout->addWidget(m_ptbY);
                pTopRowLayout->addWidget(m_ptbZ);
                pTopRowLayout->addWidget(m_ptbIso);
                pTopRowLayout->addStretch();
            }
            QHBoxLayout *pBotRowLayout = new QHBoxLayout;
            {
                pBotRowLayout->addStretch();
                pBotRowLayout->addWidget(m_ptbHFlip);
                pBotRowLayout->addWidget(m_ptbVFlip);
                pBotRowLayout->addWidget(m_ptbReset);
                pBotRowLayout->addWidget(m_ptbDistance);
                pBotRowLayout->addWidget(m_ptbFineCtrls);
                pBotRowLayout->addStretch();
            }
            pAxisViewLayout->addStretch();
            pAxisViewLayout->addLayout(pTopRowLayout);
            pAxisViewLayout->addLayout(pBotRowLayout);
            pAxisViewLayout->addStretch();
        }


        p3dViewControlsLayout->addLayout(p3dParamsLayout);
        p3dViewControlsLayout->addLayout(pAxisViewLayout);
        p3dViewControlsLayout->addWidget(m_pFineCtrls);
    }
    setLayout(p3dViewControlsLayout);
}


void gl3dGeomControls::setupSailLayout()
{
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Maximum);

    hideControls();
    m_pchAxes->setVisible(true);
    m_pchSurfaces->setVisible(true);
    m_pchPanels->setVisible(true);
    m_pchHighlight->setVisible(true);
    m_pchNormals->setVisible(true);
    m_pchOutline->setVisible(true);
    m_pchCtrlPts->setVisible(true);
    m_pchCornerPts->setVisible(true);

    QHBoxLayout *p3dViewControlsLayout = new QHBoxLayout;
    {
        QGridLayout *p3dParamsLayout = new QGridLayout;
        {
            p3dParamsLayout->addWidget(m_pchAxes,           2,1);
            p3dParamsLayout->addWidget(m_pchSurfaces,       2,2);
            p3dParamsLayout->addWidget(m_pchPanels,         2,3);
            p3dParamsLayout->addWidget(m_pchHighlight,      2,4);
            p3dParamsLayout->addWidget(m_pchNormals,        3,1);
            p3dParamsLayout->addWidget(m_pchOutline,        3,2);
            p3dParamsLayout->addWidget(m_pchCtrlPts,        3,3);
            p3dParamsLayout->addWidget(m_pchCornerPts,      3,4);
            p3dParamsLayout->setRowStretch(1,1);
            p3dParamsLayout->setRowStretch(5,1);

    //        pThreeDParamsLayout->setContentsMargins(0,0,0,0);
            p3dParamsLayout->setSpacing(0);
        }

        QVBoxLayout *pAxisViewLayout = new QVBoxLayout;
        {
            QHBoxLayout *pTopRowLayout = new QHBoxLayout;
            {
                pTopRowLayout->addStretch();
                pTopRowLayout->addWidget(m_ptbX);
                pTopRowLayout->addWidget(m_ptbY);
                pTopRowLayout->addWidget(m_ptbZ);
                pTopRowLayout->addWidget(m_ptbIso);
                pTopRowLayout->addStretch();
            }
            QHBoxLayout *pBotRowLayout = new QHBoxLayout;
            {
                pBotRowLayout->addStretch();
                pBotRowLayout->addWidget(m_ptbHFlip);
                pBotRowLayout->addWidget(m_ptbVFlip);
                pBotRowLayout->addWidget(m_ptbReset);
                pBotRowLayout->addWidget(m_ptbDistance);
                pBotRowLayout->addWidget(m_ptbFineCtrls);
                pBotRowLayout->addStretch();
            }
            pAxisViewLayout->addStretch();
            pAxisViewLayout->addLayout(pTopRowLayout);
            pAxisViewLayout->addLayout(pBotRowLayout);
            pAxisViewLayout->addStretch();
        }


        p3dViewControlsLayout->addLayout(p3dParamsLayout);
        p3dViewControlsLayout->addLayout(pAxisViewLayout);
        p3dViewControlsLayout->addWidget(m_pFineCtrls);
    }
    setLayout(p3dViewControlsLayout);
}


void gl3dGeomControls::setupOptimCpLayout()
{
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Maximum);

    hideControls();
    m_pchAxes->setVisible(true);
    m_pchSurfaces->setVisible(true);
    m_pchFoilNames->setVisible(true);
    m_pchPanels->setVisible(true);
    m_pchOutline->setVisible(true);
    m_pchCpSections->setVisible(true);
    m_pchCp->setVisible(true);
    m_pchCpIsobars->setVisible(true);

    QHBoxLayout *p3dViewControlsLayout = new QHBoxLayout;
    {
        QGridLayout *p3dParamsLayout = new QGridLayout;
        {
            p3dParamsLayout->addWidget(m_pchAxes,           2,1);
            p3dParamsLayout->addWidget(m_pchSurfaces,       2,2);
            p3dParamsLayout->addWidget(m_pchFoilNames,      2,3);
            p3dParamsLayout->addWidget(m_pchPanels,         2,4);
            p3dParamsLayout->addWidget(m_pchOutline,        3,1);
            p3dParamsLayout->addWidget(m_pchCpSections,     3,2);
            p3dParamsLayout->addWidget(m_pchCp,             3,3);
            p3dParamsLayout->addWidget(m_pchCpIsobars,      3,4);
            p3dParamsLayout->setRowStretch(1,1);
            p3dParamsLayout->setRowStretch(5,1);

    //        pThreeDParamsLayout->setContentsMargins(0,0,0,0);
            p3dParamsLayout->setSpacing(0);
        }

        QVBoxLayout *pAxisViewLayout = new QVBoxLayout;
        {
            QHBoxLayout *pTopRowLayout = new QHBoxLayout;
            {
                pTopRowLayout->addStretch();
                pTopRowLayout->addWidget(m_ptbX);
                pTopRowLayout->addWidget(m_ptbY);
                pTopRowLayout->addWidget(m_ptbZ);
                pTopRowLayout->addWidget(m_ptbIso);
                pTopRowLayout->addStretch();
            }
            QHBoxLayout *pBotRowLayout = new QHBoxLayout;
            {
                pBotRowLayout->addStretch();
                pBotRowLayout->addWidget(m_ptbHFlip);
                pBotRowLayout->addWidget(m_ptbVFlip);
                pBotRowLayout->addWidget(m_ptbReset);
                pBotRowLayout->addWidget(m_ptbDistance);
                pBotRowLayout->addWidget(m_ptbFineCtrls);
                pBotRowLayout->addStretch();
            }
            pAxisViewLayout->addStretch();
            pAxisViewLayout->addLayout(pTopRowLayout);
            pAxisViewLayout->addLayout(pBotRowLayout);
            pAxisViewLayout->addStretch();
        }


        p3dViewControlsLayout->addLayout(p3dParamsLayout);
        p3dViewControlsLayout->addLayout(pAxisViewLayout);
        p3dViewControlsLayout->addWidget(m_pFineCtrls);
    }
    setLayout(p3dViewControlsLayout);
}


void gl3dGeomControls::setupInertiaLayout()
{
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Maximum);

    hideControls();
    m_pchAxes->setVisible(true);
    m_pchPanels->setVisible(true);
    m_pchSurfaces->setVisible(true);
    m_pchFoilNames->setVisible(true);
    m_pchOutline->setVisible(true);
    m_pchMasses->setVisible(true);

    QHBoxLayout *p3dViewControlsLayout = new QHBoxLayout;
    {
        QGridLayout *p3dParamsLayout = new QGridLayout;
        {
            p3dParamsLayout->addWidget(m_pchAxes,           2,1);
            p3dParamsLayout->addWidget(m_pchSurfaces,       2,2);
            p3dParamsLayout->addWidget(m_pchFoilNames,      2,3);
            p3dParamsLayout->addWidget(m_pchPanels,         3,1);
            p3dParamsLayout->addWidget(m_pchOutline,        3,2);
            p3dParamsLayout->addWidget(m_pchMasses,         3,3);
            p3dParamsLayout->setRowStretch(1,1);
            p3dParamsLayout->setRowStretch(4,1);

    //        pThreeDParamsLayout->setContentsMargins(0,0,0,0);
            p3dParamsLayout->setSpacing(0);
        }

        QVBoxLayout *pAxisViewLayout = new QVBoxLayout;
        {
            QHBoxLayout *pTopRowLayout = new QHBoxLayout;
            {
                pTopRowLayout->addStretch();
                pTopRowLayout->addWidget(m_ptbX);
                pTopRowLayout->addWidget(m_ptbY);
                pTopRowLayout->addWidget(m_ptbZ);
                pTopRowLayout->addWidget(m_ptbIso);
                pTopRowLayout->addStretch();
            }
            QHBoxLayout *pBotRowLayout = new QHBoxLayout;
            {
                pBotRowLayout->addStretch();
                pBotRowLayout->addWidget(m_ptbHFlip);
                pBotRowLayout->addWidget(m_ptbVFlip);
                pBotRowLayout->addWidget(m_ptbReset);
                pBotRowLayout->addWidget(m_ptbDistance);
                pBotRowLayout->addWidget(m_ptbFineCtrls);
                pBotRowLayout->addStretch();
            }
            pAxisViewLayout->addStretch();
            pAxisViewLayout->addLayout(pTopRowLayout);
            pAxisViewLayout->addLayout(pBotRowLayout);
            pAxisViewLayout->addStretch();
        }


        p3dViewControlsLayout->addLayout(p3dParamsLayout);
        p3dViewControlsLayout->addLayout(pAxisViewLayout);
        p3dViewControlsLayout->addWidget(m_pFineCtrls);
    }
    setLayout(p3dViewControlsLayout);
}




