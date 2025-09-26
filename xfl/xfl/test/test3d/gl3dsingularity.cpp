/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#include <QVBoxLayout>

#include "gl3dsingularity.h"


#include <xfl/controls/w3dprefs.h>
#include <xfl3d/globals/gl_globals.h>
#include <xflcore/trace.h>
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflpanels/panels/vortex.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/wt_globals.h>

float gl3dSingularity::s_Progression(1.5f);
float gl3dSingularity::s_Radius(0.1f);
float gl3dSingularity::s_ArrowLength(0.1f);
int gl3dSingularity::s_NCircles(3);
int gl3dSingularity::s_NArrows(10);

gl3dSingularity::gl3dSingularity(QWidget *pParent) : gl3dXflView(pParent)
{
    m_bResetArrows = true;

    m_pVortex = new Vortex(Node(0.0, 0.0, -1.0), Node(0.0, 0.0, 1.0));
    m_pVortex->setCirculation(1.0f);

    QFrame *pFrame = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
        palette.setColor(QPalette::Text,       DisplayOptions::textColor());
        QColor clr = DisplayOptions::backgroundColor();
        clr.setAlpha(0);
        palette.setColor(QPalette::Window,     clr);
        palette.setColor(QPalette::Base,       clr);

        pFrame->setCursor(Qt::ArrowCursor);
        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QHBoxLayout *pSingLayout = new QHBoxLayout;
            {
                m_prbSource  = new QRadioButton("Source");
                m_prbDoublet = new QRadioButton("Doublet");
                m_prbVortex  = new QRadioButton("Vortex");


                connect(m_prbSource,  SIGNAL(clicked(bool)), this, SLOT(onUpdateData()));
                connect(m_prbDoublet, SIGNAL(clicked(bool)), this, SLOT(onUpdateData()));
                connect(m_prbVortex,  SIGNAL(clicked(bool)), this, SLOT(onUpdateData()));

                m_prbVortex->setChecked(true);

                pSingLayout->addStretch();
                pSingLayout->addWidget(m_prbSource);
                pSingLayout->addWidget(m_prbDoublet);
                pSingLayout->addWidget(m_prbVortex);
                pSingLayout->addStretch();
            }

            QGroupBox *pgbDisplay = new QGroupBox("Display");
            {
                QGridLayout *pDispLayout = new QGridLayout;
                {
                    QLabel *plabNCircles = new QLabel("Nbr. of circles=");
                    QLabel *plabNArrows  = new QLabel("Nbr. of arrows=");
                    QLabel *plabRadius   = new QLabel("Inner radius=");
                    QLabel *plabProg     = new QLabel("Progression factor=");
                    QLabel *plabScale    = new QLabel("Arrow scale=");

                    m_pieNCircles   = new IntEdit(s_NCircles);
                    m_pieNArrows    = new IntEdit(s_NArrows);

                    m_pfeInnerRadius = new FloatEdit(s_Radius);
                    m_pfeProgression = new FloatEdit(s_Progression);
                    m_pfeArrowScale = new FloatEdit(s_ArrowLength);
                    connect(m_pieNCircles,    SIGNAL(intChanged(int)),     SLOT(onUpdateData()));
                    connect(m_pieNArrows,     SIGNAL(intChanged(int)),     SLOT(onUpdateData()));
                    connect(m_pfeInnerRadius, SIGNAL(floatChanged(float)), SLOT(onUpdateData()));
                    connect(m_pfeProgression, SIGNAL(floatChanged(float)), SLOT(onUpdateData()));
                    connect(m_pfeArrowScale,  SIGNAL(floatChanged(float)), SLOT(onUpdateData()));

                    pDispLayout->addWidget(plabNCircles,     3, 1);
                    pDispLayout->addWidget(m_pieNCircles,    3, 2);
                    pDispLayout->addWidget(plabNArrows,      4, 1);
                    pDispLayout->addWidget(m_pieNArrows,     4, 2);
                    pDispLayout->addWidget(plabRadius,       10, 1);
                    pDispLayout->addWidget(m_pfeInnerRadius, 10, 2);
                    pDispLayout->addWidget(plabProg,         11, 1);
                    pDispLayout->addWidget(m_pfeProgression, 11, 2);
                    pDispLayout->addWidget(plabScale,        12, 1);
                    pDispLayout->addWidget(m_pfeArrowScale,  12, 2);
                }
                pgbDisplay->setLayout(pDispLayout);
            }

            pFrameLayout->addLayout(pSingLayout);
            pFrameLayout->addWidget(pgbDisplay);
        }


        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }


    setReferenceLength(2.0);
    reset3dScale();
}


gl3dSingularity::~gl3dSingularity()
{
    delete m_pVortex;
}


void gl3dSingularity::onUpdateData()
{
    s_NCircles = m_pieNCircles->value();
    s_NArrows  = m_pieNArrows->value();
    s_ArrowLength = m_pfeArrowScale->valuef();

    s_Radius = m_pfeInnerRadius->valuef();
    s_Progression = m_pfeProgression->valuef();

    m_bResetArrows = true;

    update();
}



void gl3dSingularity::glRenderView()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    if(m_prbSource->isChecked())
    {
        paintIcoSphere(Vector3d(), s_Radius/2.0f, xfl::CornFlowerBlue, true, false);
    }
    else if(m_prbDoublet->isChecked())
    {
        paintThinArrow(Vector3d(), Vector3d(0.0,0.0,1.0), xfl::PhugoidGreen, 2.0f, Line::SOLID);
    }
    else if(m_prbVortex->isChecked())
    {
        paintThickArrow(m_pVortex->vertexAt(0), m_pVortex->segment(), xfl::FireBrick);
    }

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj * m_matView*m_matModel);
    }
    m_shadLine.release();
    paintSegments(m_vboArrows, xfl::GreenYellow, 1.0);
}


void gl3dSingularity::glMake3dObjects()
{
    if(m_bResetArrows)
    {
        QVector<Vector3d> points, arrows;

        float radius = s_Radius;

        // doublet direction
        float alpha=0.0;
        float cosa = cos(alpha*PI/180.0);
        float sina = sin(alpha*PI/180.0);

        for(int i=0; i<s_NCircles; i++)
        {
            for(int j=0; j<s_NArrows; j++)
            {
                float theta = float(j) * 2.0 * PI / float(s_NArrows);
                float cost = cos(theta);
                float sint = sin(theta);

                Vector3d pt(radius*cost, radius*sint, 0.0);
                Vector3d vel;
                if(m_prbSource->isChecked())
                {
                    vel = pt.normalized() * 1.0/pt.norm() * s_ArrowLength;
                }
                else if(m_prbDoublet->isChecked())
                {
                    double r2 = pt.x*pt.x + pt.y*pt.y;
                    vel.x = (pt.x*pt.x * cosa - pt.y*pt.y * cosa + 2.0*pt.x*pt.y*sina)/r2/r2;
                    vel.y = (pt.y*pt.y * sina - pt.x*pt.x * sina + 2.0*pt.x*pt.y*cosa)/r2/r2;
                }
                else if(m_prbVortex->isChecked())
                {
                    m_pVortex->getInducedVelocity(pt, vel, 1.0e-6);
                }
                points.append(pt);
                arrows.append(vel);
            }
            radius *= s_Progression;
        }

        gl::makeArrows(points, arrows, s_ArrowLength, m_vboArrows);

        m_bResetArrows = false;
    }
}


void gl3dSingularity::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dSingularity");
    {
        s_NCircles    = settings.value("NCircles",    s_NCircles).toInt();
        s_NArrows     = settings.value("NArrows",     s_NArrows).toInt();
        s_ArrowLength = settings.value("ArrowLength", s_ArrowLength).toFloat();
        s_Radius      = settings.value("Radius",      s_Radius).toFloat();
        s_Progression = settings.value("Progression", s_Progression).toFloat();
    }
    settings.endGroup();
}


void gl3dSingularity::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dSingularity");
    {
        settings.setValue("NCircles",    s_NCircles);
        settings.setValue("NArrows",     s_NArrows);
        settings.setValue("ArrowLength", s_ArrowLength);
        settings.setValue("Radius",      s_Radius);
        settings.setValue("Progression", s_Progression);
    }
    settings.endGroup();
}


void gl3dSingularity::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            if(windowFlags()&Qt::FramelessWindowHint)
            {
                setWindowFlags(Qt::Window);
                show(); //Note: This function calls setParent() when changing the flags for a window, causing the widget to be hidden. You must call show() to make the widget visible again..
//                update();
                return;
            }
            break;
        }
        case Qt::Key_W:
        {
            if(bCtrl)
            {
                close();
                return;
            }
        }
    }

    gl3dXflView::keyPressEvent(pEvent);
}


void gl3dSingularity::showEvent(QShowEvent *pEvent)
{
    reset3dScale();
    setFocus();
    gl3dXflView::showEvent(pEvent);
}
