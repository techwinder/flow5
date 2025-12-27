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

// Lorenz attractor

#include <QFormLayout>
#include "gl3dlorenz.h"

#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/graphs/graph/graph.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/globals/wt_globals.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <api/utils.h>

Vector3d gl3dLorenz::s_P = Vector3d(13, 15, 30);
double gl3dLorenz::s_Sigma = 10.0;
double gl3dLorenz::s_Beta = 8.0/3.0;
double gl3dLorenz::s_Rho = 28.0;

int gl3dLorenz::s_RefreshInterval = 16; //ms = 1/60Hz = usual monitor refresh rate

int gl3dLorenz::s_MaxPts = 5000;
double gl3dLorenz::s_dt = 0.003;
LineStyle gl3dLorenz::s_ls = {true, Line::SOLID, 2, fl5Color(205,92,92), Line::NOSYMBOL};


gl3dLorenz::gl3dLorenz(QWidget *pParent) : gl3dTestGLView (pParent)
{
    setWindowTitle("Lorenz attractor");

    m_pTimer = nullptr;
    m_Counter = 0;
    m_bResetAttractor = true;
    m_iLead = 0;

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
            QLabel *pSystemLab = new QLabel;
            pSystemLab->setObjectName("Lorenz system");

            if(DisplayOptions::backgroundColor().value()>125)
                pSystemLab->setPixmap(QPixmap(QString::fromUtf8(":/images/Lorenz.png")));
            else
                pSystemLab->setPixmap(QPixmap(QString::fromUtf8(":/images/Lorenz_inv.png")));
            pSystemLab->setAlignment(Qt::AlignCenter);

            QFrame *pParamsForm = new QFrame;
            {
                QGridLayout*pParamsLayout = new QGridLayout;
                {
                    m_pdeSigma = new FloatEdit(s_Sigma);
                    m_pdeRho   = new FloatEdit(s_Rho);
                    m_pdeBeta  = new FloatEdit(s_Beta);
                    m_pdeX     = new FloatEdit(s_P.x);
                    m_pdeY     = new FloatEdit(s_P.y);
                    m_pdeZ     = new FloatEdit(s_P.z);
                    m_pdeDt    = new FloatEdit(s_dt);
                    m_pieIntervalms = new IntEdit(s_RefreshInterval);
                    m_pieMaxPts     = new IntEdit(s_MaxPts);


                    m_plbStyle  = new LineBtn(s_ls);
                    connect(m_plbStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onLineStyle(LineStyle)));

                    QLabel *pLabSigma     = new QLabel(SIGMAch+"=");
                    QLabel *pLabRho       = new QLabel(RHOch+"=");
                    QLabel *pLabBeta      = new QLabel(BETAch+"=");
                    QLabel *pLabX0        = new QLabel("X<sub>0</sub>=");
                    QLabel *pLabY0        = new QLabel("Y<sub>0</sub>=");
                    QLabel *pLabZ0        = new QLabel("Z<sub>0</sub>=");
                    QLabel *pLabdt        = new QLabel("dt=");
                    QLabel *pLabMaxPts    = new QLabel("Max. points=");
                    QLabel *pLabLineStyle = new QLabel("Line style:");
                    QLabel *pLabRefresh   = new QLabel("Refresh interval (ms)=");

                    pParamsLayout->addWidget(pLabSigma,      1, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabRho,        2, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabBeta,       3, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabX0,         4, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabY0,         5, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabZ0,         6, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabdt,         7, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabMaxPts,     8, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabLineStyle,  9, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pParamsLayout->addWidget(pLabRefresh,   10, 1, Qt::AlignRight | Qt::AlignVCenter);

                    pParamsLayout->addWidget(m_pdeSigma,       1, 2);
                    pParamsLayout->addWidget(m_pdeRho,         2, 2);
                    pParamsLayout->addWidget(m_pdeBeta,        3, 2);
                    pParamsLayout->addWidget(m_pdeX,           4, 2);
                    pParamsLayout->addWidget(m_pdeY,           5, 2);
                    pParamsLayout->addWidget(m_pdeZ,           6, 2);
                    pParamsLayout->addWidget(m_pdeDt,          7, 2);
                    pParamsLayout->addWidget(m_pieMaxPts,      8, 2);
                    pParamsLayout->addWidget(m_plbStyle,       9, 2);
                    pParamsLayout->addWidget(m_pieIntervalms, 10, 2);
                }
                pParamsForm->setLayout(pParamsLayout);
            }

            QPushButton *pRestartBtn = new QPushButton("Restart attractor");
            connect(pRestartBtn, SIGNAL(clicked()), SLOT(onRestart()));

            QPushButton *pResetDefaults = new QPushButton("Reset defaults");
            connect(pResetDefaults, SIGNAL(clicked()), SLOT(onResetDefaults()));

            m_plabFrameRate = new QLabel;
            m_pGraphWt = new GraphWt;
            {
                m_pGraphWt->showLegend(true);
                m_pGraphWt->setDefaultSize(QSize(100,150));
                m_pGraphWt->show();
                Graph *pGraph = new Graph;
                pGraph->setName("Lorenz");
                GraphOptions::resetGraphSettings(*pGraph);
                pGraph->setMargins(35,20,20,30);
                pGraph->setXVariableList(QStringList({"s"}));
                pGraph->setYVariableList(QStringList({"Updates/s"}));
                pGraph->setLegendVisible(false);
                pGraph->setAutoX(false);
                pGraph->setXMin(0);
                pGraph->setXMax(60);
                m_pGraphWt->setGraph(pGraph);
                pGraph->setCurveModel(new CurveModel);
                pGraph->setScaleType(GRAPH::EXPANDING);
                pGraph->setAuto(true);
                pGraph->addCurve("Frames/s");
                pGraph->curve(0)->appendPoint(0,0);
            }

            pFrameLayout->addWidget(pSystemLab);
            pFrameLayout->addWidget(pParamsForm);
            pFrameLayout->addWidget(pRestartBtn);
            pFrameLayout->addWidget(pResetDefaults);
            pFrameLayout->addWidget(m_plabFrameRate);
            pFrameLayout->addWidget(m_pGraphWt);
        }
        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        wt::setWidgetStyle(pFrame, palette);
    }

    onRestart();

    setReferenceLength(150);
    reset3dScale();
}


gl3dLorenz::~gl3dLorenz()
{
    if(m_pTimer)
    {
        m_pTimer->stop();
        delete m_pTimer;
        m_pTimer = nullptr;
    }

    if(m_pGraphWt && m_pGraphWt->graph())
    {
        m_pGraphWt->graph()->deleteCurveModel();
        delete m_pGraphWt->graph();
    }
}


void gl3dLorenz::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Space:
            if(m_pTimer)
            {
                if(m_pTimer->isActive()) m_pTimer->stop();
                else                     m_pTimer->start(17);
            }
            break;
    }

    gl3dTestGLView::keyPressEvent(pEvent);
}


void gl3dLorenz::hideGraphWt()
{
    m_pGraphWt->setVisible(false);
}


void gl3dLorenz::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dLorenz");
    {
        s_P.x             = settings.value("X0", s_P.x).toDouble();
        s_P.y             = settings.value("Y0", s_P.y).toDouble();
        s_P.z             = settings.value("Z0", s_P.z).toDouble();
        s_dt              = settings.value("dt", s_dt).toDouble();
        s_MaxPts          = settings.value("MaxPoints",       s_MaxPts).toInt();
        s_RefreshInterval = settings.value("RefreshInterval", s_RefreshInterval).toInt();  // ms = 1/60Hz
        xfl::loadLineSettings(settings, s_ls, "LineStyle");
    }
    settings.endGroup();
}


void gl3dLorenz::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dLorenz");
    {
        settings.setValue("X0", s_P.x);
        settings.setValue("Y0", s_P.y);
        settings.setValue("Z0", s_P.z);
        settings.setValue("dt", s_dt);
        settings.setValue("MaxPoints",       s_MaxPts);
        settings.setValue("RefreshInterval", s_RefreshInterval);
        xfl::saveLineSettings(settings, s_ls, "LineStyle");
    }
    settings.endGroup();
}


/**
 * The user has changed the color of the current curve
 */
void gl3dLorenz::onLineStyle(LineStyle)
{
    LineMenu *pLineMenu = new LineMenu(nullptr, false);
    pLineMenu->initMenu(s_ls);
    pLineMenu->exec(QCursor::pos());
    s_ls = pLineMenu->theStyle();
    m_plbStyle->setTheStyle(s_ls);
}


void gl3dLorenz::onResetDefaults()
{
    s_P = Vector3d(13, 15, 30);
    m_pdeX->setValue(s_P.x);
    m_pdeY->setValue(s_P.y);
    m_pdeZ->setValue(s_P.z);
    s_Sigma = 10.0;
    s_Beta  = 8.0/3.0;
    s_Rho   = 28.0;
    m_pdeSigma->setValue(s_Sigma);
    m_pdeRho->setValue(s_Rho);
    m_pdeBeta->setValue(s_Beta);

    s_RefreshInterval = 16; //ms --> 60Hz
    m_pieIntervalms->setValue(s_RefreshInterval);

    s_MaxPts = 5000;
    m_pieMaxPts->setValue(s_MaxPts);

    s_dt = 0.003;
    m_pdeDt->setValue(s_dt);
    onRestart();
}


void gl3dLorenz::glRenderView()
{
    QColor clr(xfl::fromfl5Clr(s_ls.m_Color));
    if(DisplayOptions::isLightTheme()) clr = clr.darker();
    else                              clr = clr.lighter();
    paintSphere(s_P, 0.5f, xfl::fromfl5Clr(s_ls.m_Color), true);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();


    paintColourSegments8(m_vboTrace, s_ls);

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dLorenz::glMake3dObjects()
{
    if(m_bResetAttractor)
    {
        int buffersize = (s_MaxPts-1)  // NSegments
                         *2*(4+4);     // 2 vertices * (3 coordinates+ 4 color components)
        QVector<float> buffer(buffersize);

        int ip0(0), ip1(0);

        int iv = 0;

        for(int j=1; j<m_Trace.size(); j++)
        {
            ip0 = (m_iLead+j-1)%s_MaxPts;
            ip1 = (m_iLead+j  )%s_MaxPts;
            buffer[iv++] = m_Trace[ip0].xf();
            buffer[iv++] = m_Trace[ip0].yf();
            buffer[iv++] = m_Trace[ip0].zf();
            buffer[iv++] = 1.0f;

            buffer[iv++] = s_ls.m_Color.redF();
            buffer[iv++] = s_ls.m_Color.greenF();
            buffer[iv++] = s_ls.m_Color.blueF();
            buffer[iv++] = double(m_Trace.size()-j+1)/double(m_Trace.size()-1);

            buffer[iv++] = m_Trace[ip1].xf();
            buffer[iv++] = m_Trace[ip1].yf();
            buffer[iv++] = m_Trace[ip1].zf();
            buffer[iv++] = 1.0f;

            buffer[iv++] = s_ls.m_Color.redF();
            buffer[iv++] = s_ls.m_Color.greenF();
            buffer[iv++] = s_ls.m_Color.blueF();
            buffer[iv++] = double(m_Trace.size()-j)/double(m_Trace.size()-1);
        }

        Q_ASSERT(iv==buffersize);

        if(m_vboTrace.isCreated()) m_vboTrace.destroy();
        m_vboTrace.create();
        m_vboTrace.bind();
        m_vboTrace.allocate(buffer.data(), buffersize * int(sizeof(GLfloat)));

        m_vboTrace.release();

        m_bResetAttractor = false;
    }
}

double gl3dLorenz::f(double x, double y, double )  const {return s_Sigma*(y-x);}
double gl3dLorenz::g(double x, double y, double z) const {return x*(s_Rho-z)-y;}
double gl3dLorenz::h(double x, double y, double z) const {return x*y-s_Beta*z;}


void gl3dLorenz::moveIt()
{
    s_dt = m_pdeDt->value();
    s_RefreshInterval = m_pieIntervalms->value();

    if(m_pTimer)
        m_pTimer->setInterval(s_RefreshInterval);

    m_iLead--;
    if(m_iLead<0) m_iLead = s_MaxPts-1;

    Vector3d &pt = m_Trace[m_iLead];
    pt = m_Trace[(m_iLead+1)%s_MaxPts];
    // RK4
    double dt = s_dt;

    //predictor
    double k1 = f(pt.x,           pt.y,             pt.z);
    double l1 = g(pt.x,           pt.y,             pt.z);
    double m1 = h(pt.x,           pt.y,             pt.z);

    double k2 = f(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));
    double l2 = g(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));
    double m2 = h(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));

    double k3 = f(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));
    double l3 = g(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));
    double m3 = h(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));

    double k4 = f(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);
    double l4 = g(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);
    double m4 = h(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);

    //corrector
    pt.x += dt*(k1 +2*k2 +2*k3 +k4)/6;
    pt.y += dt*(l1 +2*l2 +2*l3 +l4)/6;
    pt.z += dt*(m1 +2*m2 +2*m3 +m4)/6;

    m_bResetAttractor = true;

    s_P = pt; // save it

    if(m_pGraphWt->isVisible())
    {
        m_Counter++;
        if(m_Counter>=30)
        {
            int elapsed = m_LastTime.elapsed();
            double framerate = double(m_Counter)/double(elapsed)*1000.0;
            QString str;
            str = QString::asprintf("Updates/s = %7.1f", framerate);
            m_plabFrameRate->setText(str);
            Graph *pGraph = m_pGraphWt->graph();
            Curve *pCurve = m_pGraphWt->graph()->curve(0);
            if(pCurve && pCurve->size())
            {
                double lasttime = pCurve->points().back().x();
                double time = lasttime + double(elapsed)/1000.0;
                pCurve->appendPoint(time, framerate);
                if(pCurve->size()>1000)
                {
                    pCurve->popFront();
                    pGraph->setXMin(pCurve->points().front().x());
                    pGraph->setXMax(pCurve->points().back().x());
                }
            }
            pGraph->invalidate();
            m_pGraphWt->update();

            m_LastTime.restart();
            m_Counter=0;
        }
    }

    update();
}


void gl3dLorenz::onRestart()
{
    s_P = Vector3d(m_pdeX->value(), m_pdeY->value(), m_pdeZ->value());

    s_Sigma = m_pdeSigma->value();
    s_Beta  = m_pdeBeta->value();
    s_Rho   = m_pdeRho->value();

    // initialize the array
    s_MaxPts = m_pieMaxPts->value();
    s_dt = m_pdeDt->value();
    s_RefreshInterval = m_pieIntervalms->value();

    m_Trace.resize(s_MaxPts);
    for(int p=0; p<s_MaxPts; p++)
    {
        m_Trace[p] = s_P;
    }
    m_iLead = 0;

    moveIt(); // initialize the vbo

    if(m_pTimer)
    {
        m_pTimer->stop();
        delete m_pTimer;
    }

    m_LastTime.start();
    m_pGraphWt->graph()->curve(0)->clear();
    m_pGraphWt->graph()->curve(0)->appendPoint(0,0);
    m_Counter = 0;

    m_pTimer = new QTimer;
    connect(m_pTimer, SIGNAL(timeout()), SLOT(moveIt()));
    m_pTimer->start(s_RefreshInterval);
}
