/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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


#include <QTime>
#include <QVBoxLayout>
#include <QSurfaceFormat>
#include <QGroupBox>
#include <QButtonGroup>

#include "flowctrls.h"

#include <api/boatopp.h>
#include <api/planeopp.h>
#include <api/units.h>
#include <core/displayoptions.h>
#include <globals/mainframe.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/views/gl3dview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <modules/xplane/glview/gl3dxplaneview.h>
#include <modules/xplane/xplane.h>
#include <modules/xsail/view/gl3dxsailview.h>
#include <modules/xsail/xsail.h>


XPlane *FlowCtrls::s_pXPlane(nullptr);
XSail *FlowCtrls::s_pXSail(nullptr);


int FlowCtrls::s_FlowNGroups(16);
float FlowCtrls::s_Flowdt(0.01f);
FlowCtrls::flowODE FlowCtrls::s_ODE(FlowCtrls::RK4);
Vector3d FlowCtrls::s_FlowTopLeft{-1,-1,1};
Vector3d FlowCtrls::s_FlowBotRight{5,1,-1};


FlowCtrls::FlowCtrls(QWidget *pParent) : QWidget(pParent)
{
    m_pgl3dXSailView = nullptr;
    m_pgl3dXPlaneView = nullptr;

    setupLayout();
    connectSignals();

    m_stackInterval.resize(50);
    m_stackInterval.fill(0);
}


void FlowCtrls::setupLayout()
{
    QVBoxLayout*pFlowLayout = new QVBoxLayout;
    {
        QLabel *plabOGLVersion = new QLabel;
        int oglmajor = gl3dView::oglMajor();
        int oglminor = gl3dView::oglMinor();
        if(oglmajor*10+oglminor<43)
        {
            QString strange = "flow animations require OpenGL 4.3 or greater.\n";
            strange += QString::asprintf("Active version: OpenGL %d.%d",
                                         QSurfaceFormat::defaultFormat().majorVersion(),
                                         QSurfaceFormat::defaultFormat().minorVersion());

            plabOGLVersion->setText(strange);
            QString stylesheet = QString("color: red");
            plabOGLVersion->setStyleSheet(stylesheet);
        }
        else
            plabOGLVersion->setVisible(false);

        QGroupBox *pgbBox = new QGroupBox("Box limits");
        {
            QGridLayout *pBoxLayout = new QGridLayout;
            {
                QString str = Units::lengthUnitQLabel();
                for(int i=0; i<3; i++) m_plabFlowLength[i] = new QLabel(str);

                QLabel *plabMin = new QLabel("Min.");
                QLabel *plabMax = new QLabel("Max.");

                QLabel *plabX = new QLabel("x:");
                QLabel *plabY = new QLabel("y:");
                QLabel *plabZ = new QLabel("z:");

                m_pfeStart = new FloatEdit;
                m_pfeEnd   = new FloatEdit;

                m_pfeTop   = new FloatEdit;
                m_pfeBot   = new FloatEdit;
                m_pfeLeft  = new FloatEdit;
                m_pfeRight = new FloatEdit;

                pBoxLayout->addWidget(plabMin,                 1, 2, Qt::AlignCenter);
                pBoxLayout->addWidget(plabMax,                 1, 3, Qt::AlignCenter);
                pBoxLayout->addWidget(plabX,                   2, 1, Qt::AlignRight);
                pBoxLayout->addWidget(m_pfeStart,              2, 2);
                pBoxLayout->addWidget(m_pfeEnd,                2, 3);
                pBoxLayout->addWidget(m_plabFlowLength[0],     2, 4);
                pBoxLayout->addWidget(plabY,                   3, 1, Qt::AlignRight);
                pBoxLayout->addWidget(m_pfeLeft,               3, 2);
                pBoxLayout->addWidget(m_pfeRight,              3, 3);
                pBoxLayout->addWidget(m_plabFlowLength[1],     3, 4);
                pBoxLayout->addWidget(plabZ,                   4, 1, Qt::AlignRight);
                pBoxLayout->addWidget(m_pfeBot,                4, 2);
                pBoxLayout->addWidget(m_pfeTop,                4, 3);
                pBoxLayout->addWidget(m_plabFlowLength[2],     4, 4);
                pBoxLayout->setColumnStretch(1,1);
                pBoxLayout->setColumnStretch(4,1);
            }
            pgbBox->setLayout(pBoxLayout);
        }

        QGroupBox *pgbODE = new QGroupBox("ODE");
        {
            QVBoxLayout *pODELayout = new QVBoxLayout;
            {
                QHBoxLayout *pRKLayout = new QHBoxLayout;
                {
                    QLabel *plabODE = new QLabel("Method:");
                    QButtonGroup *pGroup = new QButtonGroup(this);
                    {
                        m_prbRK1 = new QRadioButton("Euler");
                        m_prbRK2 = new QRadioButton("RK2");
                        m_prbRK4 = new QRadioButton("RK4");
                        QString tip("<p>The forward Euler method is the simplest, fastest and least precise method to calculate the flow lines.<br>"
                                    "The Runge-Kutta methods at order 2 and 4 are more accurate and more computationally expensive.<br>"
                                    "<b>Recommendation:</b> select the most accurate method which does not slow down the frame rate.</p>");
                        m_prbRK1->setToolTip(tip);
                        m_prbRK2->setToolTip(tip);
                        m_prbRK4->setToolTip(tip);
                        pGroup->addButton(m_prbRK1);
                        pGroup->addButton(m_prbRK2);
                        pGroup->addButton(m_prbRK4);
                    }
                    pRKLayout->addWidget(plabODE);
                    pRKLayout->addWidget(m_prbRK1);
                    pRKLayout->addWidget(m_prbRK2);
                    pRKLayout->addWidget(m_prbRK4);
                    pRKLayout->addStretch();
                }

                QHBoxLayout *pDTLayout = new QHBoxLayout;
                {
                    QLabel *plabDt = new QLabel("dt=");
                    m_pfeDt = new FloatEdit(s_Flowdt);
                    m_pfeDt->setToolTip("<p>Defines the time increment used to move the particles:<br>"
                                        "At each frame update, the particles move in the x-direction "
                                        "a distance equal to V<sub>&infin;</sub>.dt</p>");
                    QLabel *plabSec = new QLabel("s");
                    pDTLayout->addWidget(plabDt);
                    pDTLayout->addWidget(m_pfeDt);
                    pDTLayout->addWidget(plabSec);
                    pDTLayout->addStretch();
                }

                pODELayout->addLayout(pRKLayout);
                pODELayout->addLayout(pDTLayout);
            }
            pgbODE->setLayout(pODELayout);
        }

        QGroupBox *pgbParticles = new QGroupBox("Particles");
        {
            QVBoxLayout *pParticlesLayout = new QVBoxLayout;
            {
                QHBoxLayout *pGroupsLayout = new QHBoxLayout;
                {
                    QLabel *plabNGroups = new QLabel("Nbr. of groups=");
                    m_pieNGroups = new IntEdit(s_FlowNGroups);
                    QString str = QString::asprintf("The calculation of the flow is dispatched to the GPU where it is broken down into groups of %d particles.", GROUP_SIZE);
                    str =   "<p>" + str + "<br>" +
                            "The number of groups should be less than the max. number of groups "
                            "accepted by the GPU.<br>"
                            "The GroupSize is hard-coded in the compute shader.<br>"
                            "The number of particles is NGroups x GroupSize.<br>"
                            "In the present case the main limitation to the number of groups is "
                            "the number of particles that can be computed "
                            "and rendered without loss of frame rate.</p>";

                    m_pieNGroups->setToolTip(str);

                    pGroupsLayout->addWidget(plabNGroups);
                    pGroupsLayout->addWidget(m_pieNGroups);
                    pGroupsLayout->addStretch();
                }

                m_plabNParticles = new QLabel;
                m_plabNParticles->setFont(DisplayOptions::tableFont());

                QHBoxLayout *pStyleLayout = new QHBoxLayout;
                {
                    QLabel *plabStyle = new QLabel("Flow lines style:");
                    m_plbFlowLines = new LineBtn(this);
                    m_plbFlowLines->setBackground(true);

                    pStyleLayout->addWidget(plabStyle);
                    pStyleLayout->addWidget(m_plbFlowLines);
                    pStyleLayout->addStretch();
                }

                pParticlesLayout->addLayout(pGroupsLayout);
                pParticlesLayout->addWidget(m_plabNParticles);
                pParticlesLayout->addLayout(pStyleLayout);
            }
            pgbParticles->setLayout(pParticlesLayout);
        }

        m_plabFPS = new QLabel("FPS");
        m_plabFPS->setFont(DisplayOptions::tableFont());

        QLabel *pFlow5Link = new QLabel;
        pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Releases/v722.html>https://flow5.tech/docs/flow5_doc/Releases/v722.html</a>");
        pFlow5Link->setOpenExternalLinks(true);
        pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
        pFlow5Link->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);


        pFlowLayout->addWidget(plabOGLVersion);
        pFlowLayout->addWidget(pgbBox);
        pFlowLayout->addWidget(pgbODE);

        pFlowLayout->addWidget(pgbParticles);

        pFlowLayout->addStretch();
        pFlowLayout->addWidget(m_plabFPS);
        pFlowLayout->addWidget(pFlow5Link);

    }
    setLayout(pFlowLayout);
}


void FlowCtrls::connectSignals()
{
    connect(m_pfeDt,               SIGNAL(floatChanged(float)),    SLOT(onFlowUpdate()));
    connect(m_pieNGroups,          SIGNAL(intChanged(int)),        SLOT(onFlowRestart()));
    connect(m_pfeStart,            SIGNAL(floatChanged(float)),    SLOT(onFlowUpdate()));
    connect(m_pfeEnd,              SIGNAL(floatChanged(float)),    SLOT(onFlowUpdate()));
    connect(m_pfeLeft,             SIGNAL(floatChanged(float)),    SLOT(onFlowUpdate()));
    connect(m_pfeRight,            SIGNAL(floatChanged(float)),    SLOT(onFlowUpdate()));
    connect(m_pfeBot,              SIGNAL(floatChanged(float)),    SLOT(onFlowUpdate()));
    connect(m_pfeTop,              SIGNAL(floatChanged(float)),    SLOT(onFlowUpdate()));
    connect(m_prbRK1,              SIGNAL(clicked()),              SLOT(onFlowUpdate()));
    connect(m_prbRK2,              SIGNAL(clicked()),              SLOT(onFlowUpdate()));
    connect(m_prbRK4,              SIGNAL(clicked()),              SLOT(onFlowUpdate()));
    connect(m_plbFlowLines,        SIGNAL(clickedLB(LineStyle)),   SLOT(onFlowLineStyle()));
}


void FlowCtrls::initWidget()
{
    updateUnits();

    //flow
    m_pieNGroups->setValue(s_FlowNGroups);
    m_pfeDt->setValue(s_Flowdt);

    updateFlowInfo();
    m_plbFlowLines->setTheStyle(W3dPrefs::s_FlowStyle);
    m_prbRK1->setChecked(s_ODE==EULER);
    m_prbRK2->setChecked(s_ODE==RK2);
    m_prbRK4->setChecked(s_ODE==RK4);
}


void FlowCtrls::loadSettings(QSettings &settings)
{
    settings.beginGroup("FlowCtrls");
    {
        s_FlowNGroups    = settings.value("NGroups",        s_FlowNGroups).toInt();
        s_Flowdt         = settings.value("dt",             s_Flowdt).toFloat();
        int iODE         = settings.value("ODE",             2).toInt();
        if     (iODE==0) s_ODE = EULER;
        else if(iODE==1) s_ODE = RK2;
        else             s_ODE = RK4;

        s_FlowTopLeft.x = settings.value("FlowTopLeft_x",   s_FlowTopLeft.x).toDouble();
        s_FlowTopLeft.y = settings.value("FlowTopLeft_y",   s_FlowTopLeft.y).toDouble();
        s_FlowTopLeft.z = settings.value("FlowTopLeft_z",   s_FlowTopLeft.z).toDouble();

        s_FlowBotRight.x = settings.value("FlowBotRight_x", s_FlowBotRight.x).toDouble();
        s_FlowBotRight.y = settings.value("FlowBotRight_y", s_FlowBotRight.y).toDouble();
        s_FlowBotRight.z = settings.value("FlowBotRight_z", s_FlowBotRight.z).toDouble();

    }
    settings.endGroup();
}


void FlowCtrls::saveSettings(QSettings &settings)
{
    settings.beginGroup("FlowCtrls");
    {
        settings.setValue("NGroups",        s_FlowNGroups);
        settings.setValue("dt",             s_Flowdt);

        switch(s_ODE)
        {
            case EULER: settings.setValue("ODE",  0);   break;
            case RK2:   settings.setValue("ODE",  1);   break;
            case RK4:   settings.setValue("ODE",  2);   break;
        }

        settings.setValue("FlowTopLeft_x",  s_FlowTopLeft.x);
        settings.setValue("FlowTopLeft_y",  s_FlowTopLeft.y);
        settings.setValue("FlowTopLeft_z",  s_FlowTopLeft.z);

        settings.setValue("FlowBotRight_x", s_FlowBotRight.x);
        settings.setValue("FlowBotRight_y", s_FlowBotRight.y);
        settings.setValue("FlowBotRight_z", s_FlowBotRight.z);

    }
    settings.endGroup();
}


void FlowCtrls::setFPS()
{
    m_stackInterval.push_back(QTime::currentTime().msecsSinceStartOfDay());
    double average = 0.0;
    for(int i=0; i<m_stackInterval.size()-1; i++)
        average += m_stackInterval.at(i+1)-m_stackInterval.at(i);
    average /= double(m_stackInterval.size()-1);
    m_plabFPS->setText(QString::asprintf("Frames/s = %4.1f Hz", 1000.0/average));
    m_stackInterval.pop_front();
}


void FlowCtrls::updateUnits()
{
    QString str = Units::lengthUnitQLabel();

    for(int i=0; i<3; i++)
        m_plabFlowLength[i]->setText(str);

    m_pfeStart->setValue(s_FlowTopLeft.x*Units::mtoUnit());
    m_pfeLeft->setValue(s_FlowTopLeft.y*Units::mtoUnit());
    m_pfeTop->setValue(s_FlowTopLeft.z*Units::mtoUnit());

    m_pfeEnd->setValue(s_FlowBotRight.x*Units::mtoUnit());
    m_pfeRight->setValue(s_FlowBotRight.y*Units::mtoUnit());
    m_pfeBot->setValue(s_FlowBotRight.z*Units::mtoUnit());

}


void FlowCtrls::onFlowRestart()
{
    onFlowUpdate();
    if      (MainFrame::xflApp()==xfl::XPLANE) m_pgl3dXPlaneView->restartFlow();
    else if (MainFrame::xflApp()==xfl::XSAIL)  m_pgl3dXSailView->restartFlow();
}


void FlowCtrls::onFlowLineStyle()
{
    LineMenu lm(nullptr, false);
    lm.initMenu(W3dPrefs::s_FlowStyle);
    lm.exec(QCursor::pos());

    W3dPrefs::s_FlowStyle = lm.theStyle();
    m_plbFlowLines->setTheStyle(W3dPrefs::s_FlowStyle);
}


void FlowCtrls::onFlowUpdate()
{
    s_Flowdt        = m_pfeDt->valuef();
    s_FlowNGroups   = m_pieNGroups->value();

    s_FlowTopLeft.x  = m_pfeStart->value()/Units::mtoUnit();
    s_FlowTopLeft.y  = m_pfeLeft->value()/Units::mtoUnit();
    s_FlowTopLeft.z  = m_pfeTop->value()/Units::mtoUnit();

    s_FlowBotRight.x = m_pfeEnd->value()/Units::mtoUnit();
    s_FlowBotRight.y = m_pfeRight->value()/Units::mtoUnit();
    s_FlowBotRight.z = m_pfeBot->value()/Units::mtoUnit();

    if     (m_prbRK1->isChecked()) s_ODE = EULER;
    else if(m_prbRK2->isChecked()) s_ODE = RK2;
    else if(m_prbRK4->isChecked()) s_ODE = RK4;

    updateFlowInfo();
}


void FlowCtrls::updateFlowInfo()
{
    int NBoids = s_FlowNGroups * GROUP_SIZE;
    QString strange = QString::asprintf("%d groups x %d = %d particles", s_FlowNGroups, GROUP_SIZE, NBoids);
    m_plabNParticles->setText(strange);
}


void FlowCtrls::enableFlowControls()
{
#ifdef Q_OS_MAC
    return;
#endif

    if      (MainFrame::xflApp()==xfl::XPLANE && m_pgl3dXPlaneView)
    {
        bool bEnable = s_pXPlane->curPOpp() && s_pXPlane->curPOpp()->isTriUniformMethod();

        if(!bEnable)
        {
            m_pgl3dXPlaneView->startFlow(false);
        }
    }
    else if (MainFrame::xflApp()==xfl::XSAIL && m_pgl3dXSailView)
    {
        bool bEnable = s_pXSail->curBtOpp() && s_pXSail->curBtOpp()->isTriUniformMethod();

        if(!bEnable)
        {
            m_pgl3dXSailView->startFlow(false);
        }
    }
}

