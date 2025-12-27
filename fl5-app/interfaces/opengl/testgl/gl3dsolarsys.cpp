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
#include <QGuiApplication>
#include <QScreen>
#include <QCheckBox>

#include "gl3dsolarsys.h"
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/globals/wt_globals.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <core/displayoptions.h>

double gl3dSolarSys::s_dt = 1.0; //day
double gl3dSolarSys::s_PlanetSize = 1000.0;

#define SCALEFACTOR 1.0e9



gl3dSolarSys::gl3dSolarSys(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Solar system");

    m_bResetPlanets = true;

    m_bCeres        = false;
    m_bHalley       = false;

    m_Elapsed = QDate::currentDate();

    connect(&m_Timer, SIGNAL(timeout()), SLOT(onMovePlanets()));

    QFrame *pFrame = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
        palette.setColor(QPalette::Text, DisplayOptions::textColor());

        QColor clr = DisplayOptions::backgroundColor();
        clr.setAlpha(0);
        palette.setColor(QPalette::Window, clr);
        palette.setColor(QPalette::Base, clr);
        pFrame->setCursor(Qt::ArrowCursor);

        pFrame->setPalette(palette);
        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QVBoxLayout *pMainLayout = new QVBoxLayout;
        {
            QGridLayout*pParamsLayout = new QGridLayout;
            {
                QLabel *pLabInc = new QLabel(QString::asprintf("Increment (days) @%.0f Hz:", QGuiApplication::primaryScreen()->refreshRate()));
                pLabInc->setPalette(palette);
                m_pdeDt = new FloatEdit(s_dt);
                m_pdeDt->setPalette(palette);
                QLabel *pLabSize = new QLabel("Planet size factor:");
                pLabSize->setPalette(palette);
                m_pdePlanetSize = new FloatEdit;
                m_pdePlanetSize->setPalette(palette);
                m_pdePlanetSize->setValue(s_PlanetSize);

                pParamsLayout->addWidget(pLabInc,         1, 1);
                pParamsLayout->addWidget(m_pdeDt,         1, 2);
                pParamsLayout->addWidget(pLabSize,        2, 1);
                pParamsLayout->addWidget(m_pdePlanetSize, 2, 2);
            }

            QFont fnt = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            QFontMetrics fm(fnt);

            m_plabDate = new QLabel(" ");
            m_plabDate->setPalette(palette);
            m_plabDate->setFont(fnt);

            m_plabHalley = new QLabel("\n\n\n\n");
//            m_plabHalley->setPalette(palette);
            m_plabHalley->setFont(fnt);

            m_plabHalley->setMinimumWidth(fm.averageCharWidth()*30);
            m_plabHalley->setMinimumHeight(fm.height()*6);

            QCheckBox *pchCeres = new QCheckBox("Ceres");
            pchCeres->setChecked(m_bCeres);

            QCheckBox *pchHalleyPlane = new QCheckBox("Halley's comet");
            pchHalleyPlane->setChecked(m_bHalley);

            QCheckBox *pchAxes = new QCheckBox("Axes - ecliptic");
            pchAxes->setToolTip("The ecliptic is the plane of Earth's orbit around the Sun");
            pchAxes->setChecked(m_bAxes);
            connect(pchAxes,        SIGNAL(clicked(bool)), SLOT(onAxes(bool)));
            connect(pchHalleyPlane, SIGNAL(clicked(bool)), SLOT(onHalley(bool)));
            connect(pchCeres,       SIGNAL(clicked(bool)), SLOT(onCeres(bool)));

            pMainLayout->addLayout(pParamsLayout);
            pMainLayout->addWidget(m_plabDate);
            pMainLayout->addWidget(m_plabHalley);
            pMainLayout->addWidget(pchHalleyPlane);
            pMainLayout->addWidget(pchCeres);
            pMainLayout->addWidget(pchAxes);
        }

        pFrame->setLayout(pMainLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        wt::setWidgetStyle(pFrame, palette);
    }

    makePlanets();

    // save the current light
    m_RefLight = s_Light;
    // position the light at the Sun's center
    setLightPos(0,0,0);
    setSpecular(0);

    setReferenceLength(m_Planet.back().distance()/SCALEFACTOR);

    onRestart();

    reset3dScale();
}


gl3dSolarSys::~gl3dSolarSys()
{
    //restore the light's position
    setLight(m_RefLight);
}


void gl3dSolarSys::hideEvent(QHideEvent *pEvent)
{
    //restore the light's position
    setLight(m_RefLight);

    gl3dTestGLView::hideEvent(pEvent);
}


void gl3dSolarSys::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Space:
            if(m_Timer.isActive()) m_Timer.stop();
            else                   m_Timer.start(16);
            break;
    }

    gl3dTestGLView::keyPressEvent(pEvent);
}


void gl3dSolarSys::initializeGL()
{
    gl3dTestGLView::initializeGL();
    glMakeIcoSphere(3);
}


void gl3dSolarSys::glRenderView()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }
    m_shadLine.release();
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();

    Vector3d pos;

    for(int i=0; i<m_Planet.size(); i++)
    {
        Planet const &planet = m_Planet.at(i);

        m_matModel = planet.orbitMat();

        vmMat = m_matView*m_matModel;
        pvmMat = m_matProj*vmMat;

        m_shadSurf.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
            m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
        }
        m_shadSurf.release();
        m_shadLine.bind();
        {
            m_shadLine.setUniformValue(m_locLine.m_vmMatrix, vmMat);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
        }
        m_shadLine.release();

        pos = planet.position()/SCALEFACTOR; // million km
        paintLineStrip(m_vboCircle[i], planet.m_Color, 1.0f, Line::SOLID);
        paintSphere(pos, planet.m_Radius/SCALEFACTOR*s_PlanetSize, planet.m_Color, true);
        glRenderText(pos.x, pos.y, pos.z+planet.m_Radius/SCALEFACTOR*s_PlanetSize*1.1,
                     planet.m_Name, planet.m_Color);

        if(i==5)
        {
            //paint Saturn's disk
            Planet const &Saturn = m_Planet.at(5);
//            QMatrix4x4 scale, rotate, translate;
            m_matModel.translate(Saturn.m_var[0]/SCALEFACTOR, Saturn.m_var[1]/SCALEFACTOR, Saturn.m_var[2]/SCALEFACTOR);
            m_matModel.rotate(-27.0f, sqrtf(2)/2.0f, sqrtf(2)/2.0f,0.0f);
            m_matModel.scale(s_PlanetSize);
//            m_pvmMatrix = m_pvmMatrix*translate*rotate*scale;

            vmMat = m_matView*m_matModel;
            pvmMat = m_matProj*vmMat;

            m_shadSurf.bind();
            {
                m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
                m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
            }
            m_shadSurf.release();

            QColor clr = Saturn.m_Color;
            clr.setAlpha(175);
            paintTriangleFan(m_vboSaturnDisk, clr, true, false);

            m_matModel.setToIdentity();
            vmMat = m_matView*m_matModel;
            pvmMat = m_matProj*vmMat;
            m_shadSurf.bind();
            {
                m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
                m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
            }
            m_shadSurf.release();
        }
    }

    if(m_bHalley)
    {
        m_matModel = m_Halley.orbitMat();

        QMatrix4x4 vmMat(m_matView*m_matModel);
        QMatrix4x4 pvmMat(m_matProj*vmMat);

        m_shadSurf.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
            m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
        }
        m_shadSurf.release();

        QColor clr = m_Halley.m_Color;
        clr.setAlpha(75);
        paintTriangleFan(m_vboHalleyEllipse, clr, false, false);

        pos = m_Halley.position()/SCALEFACTOR; // million km
        paintSphere(pos, 0.005/m_glScalef, m_Halley.m_Color, true);
        glRenderText(pos.x, pos.y, pos.z*s_PlanetSize*1.1, m_Halley.m_Name, m_Halley.m_Color);
    }

    if(m_bCeres)
    {
        m_matModel = m_Ceres.orbitMat();

        QMatrix4x4 vmMat(m_matView*m_matModel);
        QMatrix4x4 pvmMat(m_matProj*vmMat);

        m_shadLine.bind();
        {
            m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
        }
        m_shadLine.release();

        paintLineStrip(m_vboCeresEllipse, m_Ceres.m_Color, 0.3f, Line::SOLID);

        pos = m_Ceres.position()/SCALEFACTOR; // million km
        paintSphere(pos, m_Ceres.m_Radius/SCALEFACTOR*s_PlanetSize, m_Ceres.m_Color, true);
        glRenderText(pos.x, pos.y, pos.z*s_PlanetSize*1.1, m_Ceres.m_Name, m_Ceres.m_Color);
    }

    // paint the sun
    m_matModel.setToIdentity();
    vmMat = m_matView*m_matModel;
    pvmMat = m_matProj*vmMat;

    float radius = float(1.3927e9/SCALEFACTOR);

    m_shadPoint.bind();
    {
        m_shadPoint.setUniformValue(m_locPoint.m_vmMatrix, vmMat);
        m_shadPoint.setUniformValue(m_locPoint.m_pvmMatrix, pvmMat);
    }
    m_shadPoint.release();
    paintPoints(m_vboLightSource, radius*m_glScalef*500.0, 0, false, Qt::yellow, 4);


    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dSolarSys::glMake3dObjects()
{
    if(m_bResetPlanets)
    {
        for(int i=0; i<m_vboCircle.size(); i++)
        {
            if(m_vboCircle[i].isCreated()) m_vboCircle[i].destroy();
        }
        m_vboCircle.resize(m_Planet.size());
        for(int i=0; i<m_Planet.size(); i++)
        {
            Planet const &planet = m_Planet.at(i);
            gl::makeEllipseLineStrip(planet.m_a/SCALEFACTOR, planet.m_e, Vector3d(), m_vboCircle[i]);
        }
        gl::makeDisk(270.0e6/2.0/SCALEFACTOR, Vector3d(), m_vboSaturnDisk); // 270 000 km diameter

        // make Ceres's ellipse
        gl::makeEllipseLineStrip(m_Ceres.m_a/SCALEFACTOR, m_Ceres.m_e, Vector3d(), m_vboCeresEllipse);

        // make Halley's ellipse
        gl::makeEllipseFan(m_Halley.m_a/SCALEFACTOR, m_Halley.m_e, Vector3d(), m_vboHalleyEllipse);

       m_bResetPlanets = false;
    }

}


void gl3dSolarSys::onRestart()
{
    s_dt = m_pdeDt->value();
    s_PlanetSize = m_pdePlanetSize->value();

    int period = int(1000.0/QGuiApplication::primaryScreen()->refreshRate());
    m_Timer.start(period);
}


void gl3dSolarSys::onPlanetSize(int size)
{
    size = std::max(size, 1);
    s_PlanetSize = double(size);
    update();
}


void gl3dSolarSys::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dSolarSystem");
    {
        s_dt = settings.value("deltat", s_dt).toDouble();
        s_PlanetSize = settings.value("PlanetSizeCoef", s_PlanetSize).toDouble();
    }
    settings.endGroup();
}


void gl3dSolarSys::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dSolarSystem");
    {
         settings.setValue("deltat", s_dt);
         settings.setValue("PlanetSizeCoef", s_PlanetSize);
    }
    settings.endGroup();
}


void gl3dSolarSys::onCeres(bool bShow)
{
    m_bCeres = bShow;
    update();
}


void gl3dSolarSys::onHalley(bool bShow)
{
    m_bHalley = bShow;
    update();
}


void gl3dSolarSys::makePlanets()
{
    Planet::setCentralMass(1.98847e30); //kg

    m_Planet.resize(9);

    int p=0;
    m_Planet[p].m_Name     = "Mercury";
    m_Planet[p].m_Color    = QColor(125, 125, 125);
    m_Planet[p].m_Radius   = 2.4397 * 1.e6; // meters
    m_Planet[p].m_i = 7.005; // degrees
    m_Planet[p].m_a = 57.909e9; // meters
    m_Planet[p].m_e = 0.205630;
    m_Planet[p].m_Omega= 48.331; // degrees
    m_Planet[p].m_omega= 29.124; // degrees

    p++;
    m_Planet[p].m_Name     = "Venus";
    m_Planet[p].m_Color    = QColor(255, 150, 50);
    m_Planet[p].m_Radius   = 6.0518 * 1.e6;
    m_Planet[p].m_i = 3.39458; // degrees
    m_Planet[p].m_a = 108.208e9; // meters
    m_Planet[p].m_e = 0.006772;
    m_Planet[p].m_Omega= 76.680;
    m_Planet[p].m_omega= 54.884;

    p++;
    m_Planet[p].m_Name     = "Earth";
    m_Planet[p].m_Color    = QColor(100, 100, 255);
    m_Planet[p].m_Radius   = 6.371 * 1.e6;
    m_Planet[p].m_i = 0.0; // The ecliptic is the plane of Earth's orbit around the Sun
    m_Planet[p].m_a = 149.256e9; // meters
    m_Planet[p].m_e = 0.0167086;
    m_Planet[p].m_Omega= -11.260; // degrees
    m_Planet[p].m_omega= 114.20783;

    p++;
    m_Planet[p].m_Name     = "Mars";
    m_Planet[p].m_Color    = QColor(205, 100, 100);
    m_Planet[p].m_Radius   = 3.3895 * 1.e6;
    m_Planet[p].m_i = 1.850; // degrees
    m_Planet[p].m_a = 227.9392e9; // meters
    m_Planet[p].m_e = 0.0934;
    m_Planet[p].m_Omega= 49.558; // degrees
    m_Planet[p].m_omega= 286.502; // degrees

    p++;
    m_Planet[p].m_Name     = "Jupiter";
    m_Planet[p].m_Color    = QColor(150, 95, 75);
    m_Planet[p].m_Radius   = 69.911 * 1.e6;
    m_Planet[p].m_i = 1.303; // degrees
    m_Planet[p].m_a = 778.57e9; // meters
    m_Planet[p].m_e = 0.0489;
    m_Planet[p].m_Omega= 100.464; // degrees
    m_Planet[p].m_omega= 273.867; // degrees

    p++;
    m_Planet[p].m_Name     = "Saturn";
    m_Planet[p].m_Color    = QColor(150, 95, 150);
    m_Planet[p].m_Radius   = 58.232 * 1.e6;
    m_Planet[p].m_i = 2.485; // degrees
    m_Planet[p].m_a = 1433.53e9; // meters
    m_Planet[p].m_e = 0.0565;
    m_Planet[p].m_Omega= 113.665; // degrees
    m_Planet[p].m_omega= 339.392; // degrees

    p++;
    m_Planet[p].m_Name     = "Uranus";
    m_Planet[p].m_Color    = QColor(100, 35, 55);
    m_Planet[p].m_Radius   = 25.362 * 1.e6;
    m_Planet[p].m_i = 0.773; // degrees
    m_Planet[p].m_a = 19.191 * AU; // meters
    m_Planet[p].m_e = 0.046381;
    m_Planet[p].m_Omega= 74.006; // degrees
    m_Planet[p].m_omega= 96.998857; // degrees

    p++;
    m_Planet[p].m_Name     = "Neptune";
    m_Planet[p].m_Color    = QColor(50, 50, 175);
    m_Planet[p].m_Radius   = 24.622 * 1.e6;
    m_Planet[p].m_i = 1.767975; // degrees
    m_Planet[p].m_a = 30.07 * AU; // meters
    m_Planet[p].m_e = 0.008678;
    m_Planet[p].m_Omega= 131.783; // degrees
    m_Planet[p].m_omega= 273.187; // degrees


    p++;
    m_Planet[p].m_Name     = "Pluto";
    m_Planet[p].m_Color    = QColor(137,137,137);
    m_Planet[p].m_Radius   = 1.1883 * 1.e6;
    m_Planet[p].m_i = 17.16; // degrees
    m_Planet[p].m_a = 39.482 * AU; // meters
    m_Planet[p].m_e = 0.2488;
    m_Planet[p].m_Omega= 110.299; // degrees
    m_Planet[p].m_omega= 113.834; // degrees

    for(int i=0; i<m_Planet.size(); i++)    m_Planet[i].initializeOrbit();

    m_Ceres.m_Name = "Ceres";
    m_Ceres.m_Color = Qt::gray;

    m_Ceres.m_Radius = 469.73e3;
    m_Ceres.m_a = 2.77 * AU;
    m_Ceres.m_e = 0.0785;
    m_Ceres.m_i = 10.6;
    m_Ceres.m_omega = 73.6;
    m_Ceres.m_Omega = 80.3;
    m_Ceres.initializeOrbit();

    m_Halley.m_Name = "Halley";
    m_Halley.m_Color = Qt::lightGray;
    m_Halley.m_Radius = 1.0; // whatever
    m_Halley.m_a = 17.834 * AU; // semi-major axis length
    m_Halley.m_e = 0.96714; //excentricity
    m_Halley.m_i = 162.26;
    m_Halley.m_omega = 111.33;
    m_Halley.m_Omega = 58.42;
    // Halley's orbit is retrograde; it orbits the Sun in the opposite direction to the planets,
    // or clockwise from above the Sun's north pole
    m_Halley.initializeOrbit();
    m_Halley.setRefEnergy();
}


void gl3dSolarSys::onMovePlanets()
{
    s_PlanetSize = m_pdePlanetSize->value();

    s_dt = m_pdeDt->value(); // days
    m_Elapsed = m_Elapsed.addDays(s_dt);
    double dt = s_dt*24*3600;//seconds
    int nSteps = 20;
    dt /= nSteps;

    for(int p=0; p<m_Planet.size(); p++)
    {
        m_Planet[p].rk4_step(dt, nSteps);
    }

    m_Halley.rk4_step(dt, nSteps);
    m_Ceres.rk4_step(dt, nSteps);


    m_plabDate->setText(QString::asprintf("%2d years %2d months %3d days", m_Elapsed.year()-QDate::currentDate().year(), m_Elapsed.month(), m_Elapsed.day()));

    QString strange;
    m_Halley.list(strange);
    m_plabHalley->setText(strange);

    update();
}





