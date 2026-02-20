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

#define _MATH_DEFINES_DEFINED


#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "xplanewt.h"
#include <modules/xplane/xplane.h>
#include <modules/xplane/glview/gl3dxplaneview.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <modules/xplane/controls/popp3dctrls.h>
#include <modules/xplane/menus/xplaneactions.h>
#include <api/planexfl.h>
#include <api/planepolar.h>

QByteArray XPlaneWt::s_Geometry;
QByteArray XPlaneWt::s_HSplitterSizes;

XPlaneWt::XPlaneWt(XPlane *pXPlane, gl3dXPlaneView *pgl3dXPlaneView) : QWidget()
{
    setWindowTitle(tr("3d floating view"));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::WindowStaysOnTopHint);

    m_pXPlane = pXPlane;

    setupLayout();
    m_pgl3dXPlaneFloatView->setSharedBuffers(pgl3dXPlaneView);
    m_pgl3dXPlaneFloatView->setReferenceLength(pgl3dXPlaneView->objectReferenceLength());
    m_pgl3dXPlaneFloatView->reset3dScale();
}


void XPlaneWt::updateView()
{
    m_pgl3dXPlaneFloatView->update();
}


void XPlaneWt::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);

    switch (pEvent->key())
    {
        case Qt::Key_F4:
        case Qt::Key_W:
        {
            if(bCtrl) hide();
            pEvent->accept();
            return;
        }
        default:
            break;
    }
    pEvent->ignore();
}


void XPlaneWt::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    setControls();
}


void XPlaneWt::hideEvent(QHideEvent *pEvent)
{
    QWidget::hideEvent(pEvent);
    m_pXPlane->m_pActions->m_pOpen3dViewInNewWindow->setChecked(false);
    s_HSplitterSizes  = m_pHSplitter->saveState();
    s_Geometry = saveGeometry();
}


void XPlaneWt::setupLayout()
{
    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        m_pHSplitter = new QSplitter(Qt::Horizontal, this);
        {
            m_pHSplitter->setChildrenCollapsible(false);
            m_pgl3dXPlaneFloatView = new gl3dXPlaneView; // all buffers are shared with the main 3dview

            QFrame *pCtrlFrame = new QFrame;
            {
                QVBoxLayout *pControlLayout = new QVBoxLayout;
                {
                    m_pPOpp3dControls = new POpp3dCtrls(m_pgl3dXPlaneFloatView, this);

                    pControlLayout->addStretch();
                    pControlLayout->addWidget(m_pPOpp3dControls);
                }
                pCtrlFrame->setLayout(pControlLayout);
            }
            m_pHSplitter->addWidget(m_pgl3dXPlaneFloatView);
            m_pHSplitter->addWidget(pCtrlFrame);
            m_pHSplitter->setStretchFactor(0,1);
        }

        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);

    //exchange pointers
    m_pgl3dXPlaneFloatView->setResultControls(m_pPOpp3dControls);
}


void XPlaneWt::setControls()
{
    m_pPOpp3dControls->setControls();
}


void XPlaneWt::updateObjectData()
{
    if(m_pXPlane->m_pCurPlane)
    {
        if(m_pXPlane->curPlPolar())
            m_pgl3dXPlaneFloatView->setBotLeftOutput(m_pXPlane->m_pCurPlane->planeData(m_pXPlane->curPlPolar()->bIncludeOtherWingAreas()));
        else
            m_pgl3dXPlaneFloatView->setBotLeftOutput(m_pXPlane->m_pCurPlane->planeData(true));
    }
    else
        m_pgl3dXPlaneFloatView->setBotLeftOutput(QString());

    if(m_pXPlane->m_pCurPOpp)
        m_pgl3dXPlaneFloatView->setBotRightOutput(m_pXPlane->planeOppData());
    else
        m_pgl3dXPlaneFloatView->setBotRightOutput(QString());
}


void XPlaneWt::loadSettings(QSettings &settings)
{
    settings.beginGroup("XPlaneWidget");
    {
        s_HSplitterSizes = settings.value("HSplitterSizes").toByteArray();
        s_Geometry       = settings.value("WindowGeometry").toByteArray();
    }
    settings.endGroup();
}


void XPlaneWt::saveSettings(QSettings &settings)
{
    settings.beginGroup("XPlaneWidget");
    {        
        settings.setValue("HSplitterSizes",  s_HSplitterSizes);
        settings.setValue("WindowGeometry", s_Geometry);
    }
    settings.endGroup();
}


