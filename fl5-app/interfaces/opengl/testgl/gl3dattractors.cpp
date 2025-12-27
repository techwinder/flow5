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

#include <QRandomGenerator>
#include <QVBoxLayout>
#include <QLabel>

#include "gl3dattractors.h"
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/globals/wt_globals.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <api/utils.h>

#define NATTRACTORS 14

gl3dAttractors::enumAttractor gl3dAttractors::s_iAttractor(gl3dAttractors::LORENZ);
int gl3dAttractors::s_NTrace(11);
int gl3dAttractors::s_TailSize  = 729;
LineStyle gl3dAttractors::s_ls = {true, Line::SOLID, 2, fl5Color(205,92,92), Line::NOSYMBOL};
bool gl3dAttractors::s_bDynColor(true);


gl3dAttractors::gl3dAttractors(QWidget *pParent) : gl3dTestGLView (pParent)
{
    setWindowTitle("Strange attractors");
    m_bResetAttractor = true;
    m_iLead = 0;

    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Text, DisplayOptions::textColor());
    QColor clr = DisplayOptions::backgroundColor();
    clr.setAlpha(0);
    palette.setColor(QPalette::Window, clr);
    palette.setColor(QPalette::Base, clr);

    QFrame *pFrame = new QFrame(this);
    {
        pFrame->setCursor(Qt::ArrowCursor);

        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QGridLayout*pParamLayout = new QGridLayout;
            {
                m_prbAttractors.resize(NATTRACTORS);
                m_prbAttractors[0]  = new QRadioButton("Lorenz");
                m_prbAttractors[1]  = new QRadioButton("Newton-Leipnik");
                m_prbAttractors[2]  = new QRadioButton("Thomas");
                m_prbAttractors[3]  = new QRadioButton("Dadras");
                m_prbAttractors[4]  = new QRadioButton("Chen-Lee");
                m_prbAttractors[5]  = new QRadioButton("Aizawa");
                m_prbAttractors[6]  = new QRadioButton("Rössler");
                m_prbAttractors[7]  = new QRadioButton("Sprott");
                m_prbAttractors[8]  = new QRadioButton("Four wings");
                m_prbAttractors[9]  = new QRadioButton("Halvorsen");
                m_prbAttractors[10] = new QRadioButton("Rabinovich-Fabrikant");
                m_prbAttractors[11] = new QRadioButton("Nosé-Hoover");
                m_prbAttractors[12] = new QRadioButton("TSUCS 1");
                m_prbAttractors[13] = new QRadioButton("Arneodo");

                for(int i=0; i<m_prbAttractors.size(); i++)
                    connect(m_prbAttractors[i], SIGNAL(clicked()), SLOT(onAttractor()));
                if(s_iAttractor<m_prbAttractors.size())
                    m_prbAttractors[s_iAttractor]->setChecked(true);
                else
                {
                    s_iAttractor = LORENZ;
                    m_prbAttractors.front()->setChecked(true);
                }

                QLabel *plabNTrace = new QLabel("Nbr. traces=");
                m_pieNTrace = new IntEdit(s_NTrace);
                connect(m_pieNTrace, SIGNAL(intChanged(int)), SLOT(onRandomSeed()));

                QLabel *plabTailSize = new QLabel("Tail length=");
                m_pieTailSize = new IntEdit(s_TailSize);
                connect(m_pieTailSize, SIGNAL(intChanged(int)), SLOT(onRandomSeed()));

                QLabel *plabSpeed = new QLabel("Increment:");
                m_pslSpeed = new QSlider(Qt::Horizontal);
                m_pslSpeed->setMinimum(00);
                m_pslSpeed->setMaximum(300);
                m_pslSpeed->setTickInterval(25);
                m_pslSpeed->setValue(100);

                m_plbStyle  = new LineBtn(s_ls);
                m_plbStyle->setPalette(palette);
                connect(m_plbStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onLineStyle(LineStyle)));

                m_pchLeadingSphere = new QCheckBox("Leading spheres");
                m_pchLeadingSphere->setChecked(true);

                m_pchDynColor = new QCheckBox("Dynamic colour");
                m_pchDynColor->setChecked(s_bDynColor);

                QCheckBox *pchAxes = new QCheckBox("Axes");
                pchAxes->setChecked(true);
                connect(pchAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));

                for(int i=0; i<m_prbAttractors.size(); i++)
                    pParamLayout->addWidget(m_prbAttractors[i], i+1, 1, 1 , 2);

                pParamLayout->addWidget(plabNTrace,         NATTRACTORS+1, 1);
                pParamLayout->addWidget(m_pieNTrace,        NATTRACTORS+1, 2);
                pParamLayout->addWidget(plabTailSize,       NATTRACTORS+2, 1);
                pParamLayout->addWidget(m_pieTailSize,      NATTRACTORS+2, 2);
                pParamLayout->addWidget(plabSpeed,          NATTRACTORS+3, 1);
                pParamLayout->addWidget(m_pslSpeed,         NATTRACTORS+3, 2);
                pParamLayout->addWidget(m_plbStyle,         NATTRACTORS+4, 1, 1, 2);
                pParamLayout->addWidget(m_pchDynColor,      NATTRACTORS+5, 1, 1, 2);
                pParamLayout->addWidget(m_pchLeadingSphere, NATTRACTORS+6, 1, 1, 2);
                pParamLayout->addWidget(pchAxes,            NATTRACTORS+7, 1, 1, 2);
                pParamLayout->setColumnStretch(1,1);
                pParamLayout->setColumnStretch(2,2);
            }

            pFrameLayout->addLayout(pParamLayout);
        }
        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        wt::setWidgetStyle(pFrame, palette);
    }

    onRandomSeed();
    connect(&m_Timer, SIGNAL(timeout()), SLOT(moveThem()));
}


void gl3dAttractors::onLineStyle(LineStyle)
{
    LineMenu *pLineMenu = new LineMenu(nullptr, false);
    pLineMenu->initMenu(s_ls);
    pLineMenu->exec(QCursor::pos());
    s_ls = pLineMenu->theStyle();
    m_plbStyle->setTheStyle(s_ls);
}


void gl3dAttractors::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dAttractors");
    {
        s_iAttractor   = static_cast<enumAttractor>(settings.value("Attractor", s_iAttractor).toInt());
        s_NTrace       = settings.value("NTrace",    s_NTrace).toInt();
        s_TailSize     = settings.value("TailSize",  s_TailSize).toInt();
        s_bDynColor    = settings.value("DynColor",  s_bDynColor).toBool();
        xfl::loadLineSettings(settings, s_ls, "LineStyle");
    }
    settings.endGroup();
}


void gl3dAttractors::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dAttractors");
    {
        settings.setValue("Attractor", s_iAttractor);
        settings.setValue("NTrace",    s_NTrace);
        settings.setValue("TailSize",  s_TailSize);
        settings.setValue("DynColor",  s_bDynColor);
        xfl::saveLineSettings(settings, s_ls, "LineStyle");
    }
    settings.endGroup();
}


void gl3dAttractors::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Space:
            if(m_Timer.isActive()) m_Timer.stop();
            else                   m_Timer.start(16);
            break;
        case Qt::Key_Escape:
            showNormal();
            break;

        case Qt::Key_F10:
            moveThem();
            break;
    }

    gl3dTestGLView::keyPressEvent(pEvent);
}


void gl3dAttractors::glRenderView()
{
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView);
    }
    m_shadLine.release();

    paintColourSegments8(m_vboTrace, s_ls);

    if(m_pchLeadingSphere->isChecked())
    {
        m_shadPoint.bind();
        {
            m_shadPoint.setUniformValue(m_locPoint.m_vmMatrix, m_matView*m_matModel);
            m_shadPoint.setUniformValue(m_locPoint.m_pvmMatrix, m_matProj*m_matView);
        }
        m_shadPoint.release();
        paintPoints(m_vboPoints, 1.0f, 0, false, s_ls.m_Color, 4);
    }
    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dAttractors::glMake3dObjects()
{
    if(m_bResetAttractor)
    {
        s_bDynColor = m_pchDynColor->isChecked();

        int buffersize =  s_NTrace
                         *(s_TailSize-1)  // NSegments
                         *2*(4+4);     // 2 vertices * (4 coordinates+ 4 color components)
        QVector<float> buffer(buffersize);

        int ip0(0), ip1(0);

        int iv = 0;
        for(int i=0; i<m_Trace.size(); i++)
        {
            QVector<Vector3d> const &trace = m_Trace.at(i);
            QVector<double> &velocity = m_Velocity[i];
            for(int j=1; j<trace.size(); j++)
            {
                ip0 = (m_iLead+j-1)%s_TailSize;
                ip1 = (m_iLead+j  )%s_TailSize;
                buffer[iv++] = trace[ip0].xf();
                buffer[iv++] = trace[ip0].yf();
                buffer[iv++] = trace[ip0].zf();
                buffer[iv++] = 1.0f;
                if(s_bDynColor)
                {
                    buffer[iv++] = xfl::getRed(  velocity.at(ip0)/m_MaxVelocity);
                    buffer[iv++] = xfl::getGreen(velocity.at(ip0)/m_MaxVelocity);
                    buffer[iv++] = xfl::getBlue( velocity.at(ip0)/m_MaxVelocity);
                }
                else
                {
                    buffer[iv++] = s_ls.m_Color.redF();
                    buffer[iv++] = s_ls.m_Color.greenF();
                    buffer[iv++] = s_ls.m_Color.blueF();
                }

                buffer[iv++] = double(trace.size()-j+1)/double(trace.size()-1);

                buffer[iv++] = trace[ip1].xf();
                buffer[iv++] = trace[ip1].yf();
                buffer[iv++] = trace[ip1].zf();
                buffer[iv++] = 1.0f;
                if(s_bDynColor)
                {
                    buffer[iv++] = xfl::getRed(  velocity.at(ip1)/m_MaxVelocity);
                    buffer[iv++] = xfl::getGreen(velocity.at(ip1)/m_MaxVelocity);
                    buffer[iv++] = xfl::getBlue( velocity.at(ip1)/m_MaxVelocity);
                }
                else
                {
                    buffer[iv++] = s_ls.m_Color.redF();
                    buffer[iv++] = s_ls.m_Color.greenF();
                    buffer[iv++] = s_ls.m_Color.blueF();
                }


                buffer[iv++] = double(trace.size()-j)/double(trace.size()-1);
            }
        }

        Q_ASSERT(iv==buffersize);

        if(m_vboTrace.isCreated()) m_vboTrace.destroy();
        m_vboTrace.create();
        m_vboTrace.bind();
        m_vboTrace.allocate(buffer.data(), buffersize * int(sizeof(GLfloat)));
        m_vboTrace.release();

        // leading points
        buffersize =  s_NTrace * 4;
        buffer.resize(buffersize);
        iv = 0;
        for(int i=0; i<m_Trace.size(); i++)
        {
            QVector<double> const &velocity = m_Velocity.at(i);
            buffer[iv++] = m_Trace.at(i).at(m_iLead).xf();
            buffer[iv++] = m_Trace.at(i).at(m_iLead).yf();
            buffer[iv++] = m_Trace.at(i).at(m_iLead).zf();

            if(s_bDynColor)      buffer[iv++] = velocity.at(m_iLead)/m_MaxVelocity;
            else                 buffer[iv++] = -1.0f;
        }
        if(m_vboPoints.isCreated()) m_vboPoints.destroy();
        m_vboPoints.create();
        m_vboPoints.bind();
        m_vboPoints.allocate(buffer.data(), buffersize * int(sizeof(GLfloat)));
        m_vboPoints.release();

        m_bResetAttractor = false;
    }
}


void gl3dAttractors::onAttractor()
{
    s_iAttractor = LORENZ;
    for(int i=0; i<m_prbAttractors.size(); i++)
    {
        if(m_prbAttractors[i]->isChecked())
        {
            s_iAttractor = static_cast<enumAttractor>(i);
            break;
        }
    }
    onRandomSeed();
    on3dReset();
}


void gl3dAttractors::onRandomSeed()
{
    if(m_Timer.isActive())
        m_Timer.stop();

    s_NTrace = m_pieNTrace->value();
    s_TailSize = m_pieTailSize->value();

    m_Trace.resize(s_NTrace);
    for(int jt=0; jt<m_Trace.size(); jt++) m_Trace[jt].resize(s_TailSize);
    m_Velocity.resize(s_NTrace);
    for(int jt=0; jt<m_Velocity.size(); jt++)
    {
        m_Velocity[jt].resize(s_TailSize);
        m_Velocity[jt].fill(0);
    }
    m_MaxVelocity = 0.0001;

    m_iLead = 0;

    double xmin(0), ymin(0), zmin(0), amp(1);
    switch(s_iAttractor)
    {
        case LORENZ:     xmin = ymin=-10; zmin=3.0;       amp = 20.0;  break;
        case NEWTON:     xmin = ymin=-15;                 amp = 30.0;  break;
        case THOMAS:     xmin = ymin = zmin = 0;          amp = 3.0;   break;
        case DADRAS:     xmin = ymin = zmin = -5;         amp = 10.0;  break;
        case CHENLEE:    xmin = ymin=-5; zmin=3.0;        amp = 10.0;  break;
        case AIZAWA:     xmin = ymin=-0.5; zmin=0.0;      amp = 1.0;   break;
        case ROSSLER:    xmin = ymin = -5;                amp = 10.0;  break;
        case SPROTT:     xmin = 0; ymin = zmin = -0.5;    amp = 1.0;   break;
        case FOURWINGS:  xmin = ymin = zmin = -0.5;       amp = 1.0;   break;
        case HALVORSEN:  xmin = ymin = zmin = -2.5;       amp = 5.0;   break;
        case RABINOVICH: xmin = ymin=-1; zmin=0;          amp = 2.0;   break;
        case NOSE:       xmin = ymin = -2; zmin = 0;      amp = 5.0;   break;
        case TCUCS1:     xmin = ymin = -30; zmin = 0;     amp = 60.0;  break;
        case ARNEODO:    xmin = ymin = zmin = -1.0;       amp = 2.0;   break;
    }

    double rmax(0);
    Vector3d pos;
    for(int i=0; i<s_NTrace; i++)
    {
        pos.x = xmin + QRandomGenerator::global()->bounded(amp);
        pos.y = ymin + QRandomGenerator::global()->bounded(amp);
        pos.z = zmin + QRandomGenerator::global()->bounded(amp);
        rmax = std::max(rmax, pos.norm());
        QVector<Vector3d> &trace = m_Trace[i];
        for(int jt=0; jt<s_TailSize; jt++)
            trace[jt] = pos;
        m_Velocity[i].fill(0);
    }
    m_MaxVelocity = 0.0001;

    setReferenceLength(rmax*3.0);
//    on3dReset();
    m_bResetAttractor = true;
    setFocus();
    m_Timer.start(16);
}


double gl3dAttractors::f(double x, double y, double z)  const
{
    switch(s_iAttractor)
    {
        default:
        case LORENZ:     return 10.0*(y-x);
        case NEWTON:     return -0.4*x + y + 10.0*y*z;
        case THOMAS:     return sin(y) -0.208186*x;
        case DADRAS:     return y-3.0*x+2.7*y*z;
        case CHENLEE:    return 5.0*x-y*z;
        case AIZAWA:     return x*(z-0.7) - 3.5*y;
        case ROSSLER:    return -(y+z);
        case SPROTT:     return y + 2.07*x*y + x*z;
        case FOURWINGS:  return 0.2*x + y*z;
        case HALVORSEN:  return -1.89*x - 4*y - 4*z - y*y;
        case RABINOVICH: return y*(z-1.0+x*x) + 0.1*x;
        case NOSE:       return y;
        case TCUCS1:     return 40.0*(y-x)+0.5*x*z;
        case ARNEODO:    return y;
    }
}


double gl3dAttractors::g(double x, double y, double z) const
{
    switch(s_iAttractor)
    {
        default:
        case LORENZ:     return x*(28.0-z)-y;
        case NEWTON:     return -x - 0.4*y + 5.0*x*z;
        case THOMAS:     return sin(z) -0.208186*y;
        case DADRAS:     return 1.7*y -x*z+z;
        case CHENLEE:    return -10.0*y+x*z;
        case AIZAWA:     return 3.5*x + y*(z-0.7);
        case ROSSLER:    return x+0.2*y;
        case SPROTT:     return 1.0 - 1.79*x*x + y*z;
        case FOURWINGS:  return 0.01*x - 0.4*y -x*z;
        case HALVORSEN:  return -1.89*y - 4*z - 4*x - z*z;
        case RABINOVICH: return x*(3.0*z+1.0-x*x) + 0.1*y;
        case NOSE:       return -x+y*z;
        case TCUCS1:     return 20.0*y-x*z;
        case ARNEODO:    return z;
    }
}


double gl3dAttractors::h(double x, double y, double z) const
{
    switch(s_iAttractor)
    {
        default:
        case LORENZ:     return x*y-8.0/3.0*z;
        case NEWTON:     return 0.175*z - 5*x*y;
        case THOMAS:     return sin(x) -0.208186*z;
        case DADRAS:     return 2*x*y-9.0*z;
        case CHENLEE:    return -0.38*z+x*y/3.0;
        case AIZAWA:     return 0.6 + 0.95*z - z*z*z/3.0 -(x*x+y*y)*(1.0+0.25*z)+0.1*z*x*x*x;
        case ROSSLER:    return 0.2 + z*(x-5.7);
        case SPROTT:     return x -x*x -y*y;
        case FOURWINGS:  return -z - x*y;
        case HALVORSEN:  return -1.89*z - 4*x - 4*y - x*x;
        case RABINOVICH: return -2*z*(0.14+x*y);
        case NOSE:       return (1.5-y*y);
        case TCUCS1:     return 0.833*z+x*y-0.65*x*x;
        case ARNEODO:    return 5.5*x-3.5*y-z-x*x*x;
    }
}


void gl3dAttractors::moveThem()
{
    double k1(0), l1(0), m1(0);
    double k2(0), l2(0), m2(0);
    double k3(0), l3(0), m3(0);
    double k4(0), l4(0), m4(0);

    double dt = 0.003;

    switch(s_iAttractor)
    {
        case LORENZ:     dt=0.003;        break;
        case NEWTON:     dt=2.e-4;        break;
        case THOMAS:     dt=0.07;         break;
        case DADRAS:     dt=0.004;        break;
        case CHENLEE:    dt=0.0025;       break;
        case AIZAWA:     dt=0.005;        break;
        case ROSSLER:    dt=0.01;         break;
        case SPROTT:     dt=0.01;         break;
        case FOURWINGS:  dt=0.05;         break;
        case HALVORSEN:  dt=0.003;        break;
        case RABINOVICH: dt=0.015;        break;
        case NOSE:       dt=0.01;         break;
        case TCUCS1:     dt=0.0005;       break;
        case ARNEODO:    dt=0.01;         break;
    }

    double coef = double(m_pslSpeed->value())/100.0;
    coef = std::max(0.1, coef);
    dt *= coef;

    m_iLead--;
    if(m_iLead<0) m_iLead = s_TailSize-1;

    double rmax = 0.0;

    m_MaxVelocity *=0.995; // partial reset to prevent the colors from getting squashed

    for(int i=0; i<m_Trace.size(); i++)
    {
        QVector<Vector3d> &trace = m_Trace[i];
        Vector3d &pt = trace[m_iLead];
        pt = trace[(m_iLead+1)%s_TailSize];

        //predictor
        k1 = f(pt.x,           pt.y,             pt.z);
        l1 = g(pt.x,           pt.y,             pt.z);
        m1 = h(pt.x,           pt.y,             pt.z);

        k2 = f(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));
        l2 = g(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));
        m2 = h(pt.x+0.5*k1*dt, pt.y+(0.5*l1*dt), pt.z+(0.5*m1*dt));

        k3 = f(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));
        l3 = g(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));
        m3 = h(pt.x+0.5*k2*dt, pt.y+(0.5*l2*dt), pt.z+(0.5*m2*dt));

        k4 = f(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);
        l4 = g(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);
        m4 = h(pt.x+k3*dt,     pt.y+l3*dt,       pt.z+m3*dt);

        //corrector
        pt.x += dt*(k1 +2*k2 +2*k3 +k4)/6;
        pt.y += dt*(l1 +2*l2 +2*l3 +l4)/6;
        pt.z += dt*(m1 +2*m2 +2*m3 +m4)/6;

        double dx = f(pt.x, pt.y, pt.z);
        double dy = g(pt.x, pt.y, pt.z);
        double dz = h(pt.x, pt.y, pt.z);
        QVector<double> &velocity = m_Velocity[i];
        velocity[m_iLead] = sqrt(dx*dx+dy*dy+dz*dz)/5.0;
        m_MaxVelocity = std::max(m_MaxVelocity, velocity.at(m_iLead));
        rmax = std::max(rmax, pt.norm());
    }
    m_bResetAttractor = true;
    setReferenceLength(rmax*3.0);
    update();
}




