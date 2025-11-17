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

#include <QFormLayout>

#include "fine3dcontrols.h"
#include <interfaces/opengl/views/gl3dview.h>
#include <interfaces/opengl/controls/gllightdlg.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <core/xflcore.h>
#include <api/utils.h>

Fine3dControls::Fine3dControls(gl3dView *pgl3dview) : QFrame()
{
    setWindowFlag(Qt::WindowStaysOnTopHint);

    m_XLast = 0;
    m_YLast = 0;
    m_ZLast = 0;

    m_XSet = m_YSet = m_ZSet = 0;//sanity check

    m_pgl3dView = pgl3dview;
    setupLayout();
    connectSignals();
}


#define RANGE 1000

void Fine3dControls::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGridLayout *pSliderLayout = new QGridLayout;
        {
            m_pslClipPlanePos = new QSlider(Qt::Horizontal);
            m_pslClipPlanePos->setMinimum(-100);
            m_pslClipPlanePos->setMaximum(100);
            m_pslClipPlanePos->setSliderPosition(0);
            m_pslClipPlanePos->setTickInterval(10);
            m_pslClipPlanePos->setTickPosition(QSlider::TicksBelow);
            m_pslClipPlanePos->setValue(100);

            //------------------------------------------------------
            m_pslXView = new QSlider(Qt::Horizontal);
            m_pslXView->setMinimum(-RANGE);
            m_pslXView->setMaximum( RANGE);
            m_pslXView->setSliderPosition(0);
            m_pslXView->setTickInterval(int(RANGE/10));
            m_pslXView->setTickPosition(QSlider::TicksBelow);
            m_pslXView->setValue(0);

            m_pslYView = new QSlider(Qt::Horizontal);
            m_pslYView->setMinimum(-RANGE);
            m_pslYView->setMaximum( RANGE);
            m_pslYView->setSliderPosition(0);
            m_pslYView->setTickInterval(int(RANGE/10));
            m_pslYView->setTickPosition(QSlider::TicksBelow);
            m_pslYView->setValue(0);

            m_pslZView = new QSlider(Qt::Horizontal);
            m_pslZView->setMinimum(-RANGE);
            m_pslZView->setMaximum( RANGE);
            m_pslZView->setSliderPosition(0);
            m_pslZView->setTickInterval(int(RANGE/10));
            m_pslZView->setTickPosition(QSlider::TicksBelow);
            m_pslZView->setValue(0);

            pSliderLayout->addWidget(new QLabel("roll"), 1, 1, Qt::AlignRight | Qt::AlignVCenter);
            pSliderLayout->addWidget(m_pslXView,         1, 2);
            pSliderLayout->addWidget(new QLabel("pitch"),2, 1, Qt::AlignRight | Qt::AlignVCenter);
            pSliderLayout->addWidget(m_pslYView,         2, 2);
            pSliderLayout->addWidget(new QLabel("yaw"),  3, 1, Qt::AlignRight | Qt::AlignVCenter);
            pSliderLayout->addWidget(m_pslZView,         3, 2);
            pSliderLayout->addWidget(new QLabel("clip"), 4, 1, Qt::AlignRight | Qt::AlignVCenter);
            pSliderLayout->addWidget(m_pslClipPlanePos,  4, 2);
        }

        QHBoxLayout *pZAnimLayout = new QHBoxLayout;
        {
            m_pchZAnimate   = new QCheckBox("Auto z-rotation");
            m_pdeZAnimAngle = new FloatEdit(gl3dView::zAnimAngle());
            QLabel *pLabDeg = new QLabel(DEGch + "@60Hz");

            pZAnimLayout->addWidget(m_pchZAnimate);
            pZAnimLayout->addWidget(m_pdeZAnimAngle);
            pZAnimLayout->addWidget(pLabDeg);
            pZAnimLayout->addStretch();
        }

        pMainLayout->addLayout(pSliderLayout);
        pMainLayout->addLayout(pZAnimLayout);
    }
    setLayout(pMainLayout);
}


void Fine3dControls::connectSignals()
{
    connect(m_pslClipPlanePos, SIGNAL(valueChanged(int)), m_pgl3dView, SLOT(onClipPlane(int)));
    connect(m_pslXView,        SIGNAL(valueChanged(int)), SLOT(onXView(int)));
    connect(m_pslYView,        SIGNAL(valueChanged(int)), SLOT(onYView(int)));
    connect(m_pslZView,        SIGNAL(valueChanged(int)), SLOT(onZView(int)));
    connect(m_pchZAnimate,     SIGNAL(clicked(bool)), m_pgl3dView, SLOT(onZAnimate(bool)));
    connect(m_pdeZAnimAngle,   SIGNAL(floatChanged(float)), this, SLOT(onZAnimAngle()));
}


void Fine3dControls::setControls()
{
    m_XSet = m_YSet = m_ZSet = 0;//sanity check

    m_XLast = m_YLast = m_ZLast = 0.0;

    m_pslXView->setValue(m_XLast);
    m_pslYView->setValue(m_YLast);
    m_pslZView->setValue(m_ZLast);

    m_pchZAnimate->setChecked(m_pgl3dView->bZAnimation());
}


void Fine3dControls::showEvent(QShowEvent *)
{
    setControls();
}


void Fine3dControls::onZAnimAngle()
{
    gl3dView::setZAnimAngle(m_pdeZAnimAngle->value());
}


void Fine3dControls::onResetCtrls()
{
    m_XSet = m_YSet = m_ZSet = 0;//sanity check

    m_XLast = m_YLast = m_ZLast = 0.0;

    m_pslXView->setValue(m_XLast);
    m_pslYView->setValue(m_YLast);
    m_pslZView->setValue(m_ZLast);
}


void Fine3dControls::onXView(int)
{
    int xrot  = m_pslXView->value();

    m_XSet += xrot-m_XLast;

    double angle = double(xrot-m_XLast)*90.0/double(RANGE);
    Quaternion qtrot(angle, Vector3d(1,0,0));
    m_pgl3dView->arcBall().applyRotation(qtrot, true);

    m_pgl3dView->reset3dRotationCenter();
    m_pgl3dView->update();

    m_XLast = xrot;
}


void Fine3dControls::onYView(int)
{
    int yrot  = m_pslYView->value();

    m_YSet += yrot-m_YLast;

    double angle = double(yrot-m_YLast)*90.0/double(RANGE);
    Quaternion qtrot(angle, Vector3d(0,1,0));
    m_pgl3dView->arcBall().applyRotation(qtrot, true);

    m_pgl3dView->reset3dRotationCenter();
    m_pgl3dView->update();

    m_YLast = yrot;
}


void Fine3dControls::onZView(int)
{
    int zrot  = m_pslZView->value();

    m_ZSet += zrot-m_ZLast;

    double angle = double(zrot-m_ZLast)*90.0/double(RANGE);
    Quaternion qtrot(angle, Vector3d(0,0,1));
    m_pgl3dView->arcBall().applyRotation(qtrot, true);

    m_pgl3dView->reset3dRotationCenter();
    m_pgl3dView->update();

    m_ZLast = zrot;
}


