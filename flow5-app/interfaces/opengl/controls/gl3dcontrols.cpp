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

#include "gl3dcontrols.h"
#include <interfaces/opengl/views/gl3dview.h>
#include <interfaces/opengl/controls/fine3dcontrols.h>
#include <core/displayoptions.h>


gl3dControls::gl3dControls(gl3dView *pgl3dview, QWidget *pParent) : QFrame(pParent)
{
    m_pgl3dView = pgl3dview;

    makeCommonWts();
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
}


gl3dControls::~gl3dControls()
{
    if(m_pFineCtrls) delete m_pFineCtrls;
    m_pFineCtrls = nullptr;
}


void gl3dControls::makeCommonWts()
{
    m_ptbX         = new QToolButton(this);
    m_ptbY         = new QToolButton(this);
    m_ptbZ         = new QToolButton(this);
    m_ptbIso       = new QToolButton(this);
    m_ptbHFlip     = new QToolButton(this);
    m_ptbVFlip     = new QToolButton(this);
    m_ptbReset     = new QToolButton(this);
    m_ptbFineCtrls = new QToolButton(this);
    m_ptbDistance  = new QToolButton(this);

    QAction *pXView      = new QAction(QIcon(":/icons/OnXView.png"),      tr("X view\t(X)\n\t(Shift+X)"),    this);
    QAction *pYView      = new QAction(QIcon(":/icons/OnYView.png"),      tr("Y view\t(Y)\n\t(Shift+Y)"),    this);
    QAction *pZView      = new QAction(QIcon(":/icons/OnZView.png"),      tr("Z view\t(Z)\n\t(Shift+Z)"),    this);
    QAction *pIsoView    = new QAction(QIcon(":/icons/OnIsoView.png"),    tr("Pseudo iso view\t(I)"),        this);
    QAction *pFlipHView  = new QAction(QIcon(":/icons/OnFlipHView.png"),  tr("Flip view horizontally\t(H)"), this);
    QAction *pFlipVView  = new QAction(QIcon(":/icons/OnFlipVView.png"),  tr("Flip view vertically\t(V)"),   this);
    QAction *pResetView  = new QAction(QIcon(":/icons/reset3dscale.png"), tr("Reset 3d scale \t(R)"),        this);
    QAction *pLightView  = new QAction(QIcon(":/icons/light.png"),        tr("Toggle the light on/off"),     this);
    QAction *pFineCtrls  = new QAction(QIcon(":/icons/OnFineCtrls.png"),  tr("Show/hide the fine controls"), this);
    QAction *pDistance   = new QAction(QIcon(":/icons/OnDistance.png"),   tr("Node distance"),               this);

    pXView->setCheckable(false);
    pYView->setCheckable(false);
    pZView->setCheckable(false);
    pIsoView->setCheckable(false);
    pFlipHView->setCheckable(false);
    pResetView->setCheckable(false);
    pLightView->setCheckable(true);
    pFineCtrls->setCheckable(true);
    pDistance->setCheckable(true);

    m_ptbX->setDefaultAction(pXView);
    m_ptbY->setDefaultAction(pYView);
    m_ptbZ->setDefaultAction(pZView);
    m_ptbIso->setDefaultAction(pIsoView);
    m_ptbHFlip->setDefaultAction(pFlipHView);
    m_ptbVFlip->setDefaultAction(pFlipVView);
    m_ptbReset->setDefaultAction(pResetView);
    m_ptbFineCtrls->setDefaultAction(pFineCtrls);
    m_ptbDistance->setDefaultAction(pDistance);
    m_ptbDistance->setToolTip(tr("<p>Click on two mesh nodes to display their distance</p>"));


    int iconsize = DisplayOptions::iconSize();
    m_ptbX->setIconSize(        QSize(iconsize, iconsize));
    m_ptbY->setIconSize(        QSize(iconsize, iconsize));
    m_ptbZ->setIconSize(        QSize(iconsize, iconsize));
    m_ptbIso->setIconSize(      QSize(iconsize, iconsize));
    m_ptbHFlip->setIconSize(    QSize(iconsize, iconsize));
    m_ptbVFlip->setIconSize(    QSize(iconsize, iconsize));
    m_ptbReset->setIconSize(    QSize(iconsize, iconsize));
    m_ptbFineCtrls->setIconSize(QSize(iconsize, iconsize));
    m_ptbDistance->setIconSize( QSize(iconsize, iconsize));


    m_pFineCtrls = new Fine3dControls(m_pgl3dView);
    m_pFineCtrls->setVisible(false);
}


void gl3dControls::connectBaseSignals()
{
    connect(m_ptbIso,        SIGNAL(clicked(bool)), m_pgl3dView, SLOT(on3dIso()));
    connect(m_ptbX,          SIGNAL(clicked(bool)), m_pgl3dView, SLOT(on3dRear()));
    connect(m_ptbY,          SIGNAL(clicked(bool)), m_pgl3dView, SLOT(on3dLeft()));
    connect(m_ptbZ,          SIGNAL(clicked(bool)), m_pgl3dView, SLOT(on3dTop()));
    connect(m_ptbHFlip,      SIGNAL(clicked(bool)), m_pgl3dView, SLOT(on3dFlipH()));
    connect(m_ptbVFlip,      SIGNAL(clicked(bool)), m_pgl3dView, SLOT(on3dFlipV()));
    connect(m_ptbReset,      SIGNAL(clicked(bool)), m_pgl3dView, SLOT(on3dReset()));

    connect(m_ptbFineCtrls,  SIGNAL(clicked(bool)),  SLOT(onFineCtrls(bool)));

    connect(m_pgl3dView,     SIGNAL(viewModified()), m_pFineCtrls, SLOT(onResetCtrls()));
}


void gl3dControls::showEvent(QShowEvent *)
{
    setControls();
    QSize minsize(DisplayOptions::iconSize(), DisplayOptions::iconSize());

    if(m_ptbX->height()<32)
    {
        m_ptbX->setIconSize(minsize);
        m_ptbY->setIconSize(minsize);
        m_ptbZ->setIconSize(minsize);
        m_ptbIso->setIconSize(minsize);
        m_ptbHFlip->setIconSize(minsize);
        m_ptbVFlip->setIconSize(minsize);
        m_ptbReset->setIconSize(minsize);
        m_ptbFineCtrls->setIconSize(minsize);
        m_ptbDistance->setIconSize(minsize);
    }
}


void gl3dControls::setControls()
{
    m_ptbFineCtrls->setChecked(m_pFineCtrls->isVisible());
    m_ptbFineCtrls->defaultAction()->setChecked(m_pFineCtrls->isVisible());
}


void gl3dControls::onFineCtrls(bool bChecked)
{
    m_pgl3dView->update();
    m_pFineCtrls->setVisible(bChecked);
}



void gl3dControls::resetClipPlane()
{
    m_pFineCtrls->m_pslClipPlanePos->setValue(1000);
}

