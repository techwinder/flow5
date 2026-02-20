/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois techwinder@flow5.tech
    
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

#include <QtConcurrent/QtConcurrent>
#include <QRandomGenerator>
#include <QVBoxLayout>
#include <QSlider>
#include <QGroupBox>

#include "gl3dhydrogen.h"

#include <interfaces/controls/w3dprefs.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <core/displayoptions.h>
#include <api/utils.h>
#include <api/mathelem.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/globals/wt_globals.h>

#define PLANCK        6.62607015e-34      // reduced Planck constant, J/Hz
#define EPS0          8.8541878128e-12    // vacuum permittivity, F/m
#define q_e           1.602176634e-19     // proton charge, Coulomb >0
#define m_e           9.10938356e-31      // electron mass, kg
#define A0            5.29177210903e-11   // Bohr radius


int gl3dHydrogen::s_n(2); // principal quantum number: 1, 2, 3
int gl3dHydrogen::s_l(1); // azimuthal quantum number: 0, ..., n-1
int gl3dHydrogen::s_m(0); // magnetic  quantum number: -l, ...,l
int gl3dHydrogen::s_NObservations(25000); // Number of observationss
int gl3dHydrogen::s_ElectronSize = 15;
double gl3dHydrogen::s_ObsRadius(50.0); // in Bohr radius units
int gl3dHydrogen::s_iRenderer(0);

gl3dHydrogen::gl3dHydrogen(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Hydrogen atom");
    m_bCancel = false;
    m_bIsObserving = false;

    m_UpdateInterval = 25;

    m_StateMax = 0;
    m_bResetPositions = false;

    m_BlockSize = 50;

    setReferenceLength(s_ObsRadius);

    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Text,       DisplayOptions::textColor());

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
            QHBoxLayout *pTitleLayout = new QHBoxLayout;
            {
                QLabel *pLabTitle = new QLabel("<b>Hydrogen atom</b>");
                pTitleLayout->addStretch();
                pTitleLayout->addWidget(pLabTitle);
                pTitleLayout->addStretch();
            }
            QGroupBox *pQuantumBox = new QGroupBox("Quantum state");
            {
                QGridLayout*pQuantumLayout = new QGridLayout;
                {
                    QLabel *plab_n = new QLabel("n=");
                    m_pien = new IntEdit(s_n);
                    m_pien->setToolTip("Principal quantum number");

                    QLabel *plab_l = new QLabel("l=");
                    m_piel = new IntEdit(s_l);
                    m_piel->setToolTip("Azimuthal quantum number<br>"
                                       "l &lt; n");

                    QLabel *plab_m = new QLabel("m=");
                    m_piem = new IntEdit(s_m);
                    m_piem->setToolTip("Magnetic quantum number<br>"
                                       "-l &le; m &le; l");

                    pQuantumLayout->addWidget(plab_n,       1,1, Qt::AlignRight);
                    pQuantumLayout->addWidget(m_pien,       1,2);
                    pQuantumLayout->addWidget(plab_l,       2,1, Qt::AlignRight);
                    pQuantumLayout->addWidget(m_piel,       2,2);
                    pQuantumLayout->addWidget(plab_m,       3,1, Qt::AlignRight);
                    pQuantumLayout->addWidget(m_piem,       3,2);
                    pQuantumLayout->setColumnStretch(3,1);
                }
                pQuantumBox->setLayout(pQuantumLayout);
            }

            QGroupBox *pObsBox = new QGroupBox("Observations");
            {
                QGridLayout*pObsLayout = new QGridLayout;
                {
                    QLabel *plabNObs = new QLabel("Nbr. of observations=");
                    m_pieNObs = new IntEdit(s_NObservations);
                    m_pieNObs->setToolTip("Specifies the number of observations.<br>"
                                          "Each time that an observation is made, the wave function "
                                          "collapses to a single electron position. "
                                          "Each collapsed position is represented by a coloured point, "
                                          "with the most probable locations in red and the least probable in blue.<br>"
                                          "A standard GPU should be able to handle 10 thousand points "
                                          "without noticeable rendering speed decrease.");

                    QLabel *plabObsDist = new QLabel("Observation radius=");
                    m_pdeObsRad  = new FloatEdit(s_ObsRadius);
                    m_pdeObsRad->setToolTip("This is an arbitrary limit used to reduce the time to calculate the collapsed positions.<br>"
                                            "The radius should be as small as possible to reduce the computation times, "
                                            "but sufficiently large to include the most probable locations of the wave function.<br>"
                                            "Activate the checkbox to visualize the observation sphere.");
                    QLabel *plabBohrRad = new QLabel("x Bohr radius");

                    m_ppbMake = new QPushButton("Start observations");
                    m_ppbMake->setToolTip("Starts/stops the collapse of the electron's wave function for the specified quantum numbers.<br>"
                                          "Each dot is the result of a collapse operation.");
                    connect(m_ppbMake, SIGNAL(clicked()), SLOT(onCollapse()));

                    m_plabNObs = new QLabel(" ");
                    m_plabNObs->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

                    pObsLayout->addWidget(plabNObs,     4,1, Qt::AlignRight);
                    pObsLayout->addWidget(m_pieNObs,    4,2);
                    pObsLayout->addWidget(plabObsDist,  5,1, Qt::AlignRight);
                    pObsLayout->addWidget(m_pdeObsRad,  5,2);
                    pObsLayout->addWidget(plabBohrRad,  5,3, Qt::AlignLeft);
                    pObsLayout->addWidget(m_ppbMake,    6,1,1,3);
                    pObsLayout->addWidget(m_plabNObs,   7,1,1,3);
                }
                pObsBox->setLayout(pObsLayout);
            }

            QGroupBox *pDisplayBox = new QGroupBox("Display");
            {
                QVBoxLayout *pDisplayLayout = new QVBoxLayout;
                {
                    m_pchBohr = new QCheckBox("Ground state");
                    m_pchBohr->setToolTip("Activate this checkbox to display the electron's ground state "
                                          "which is its state of lowest energy corresponding to the quantum numbers (1,0,0).<br>"
                                          "This is also the radius of Bohr's atom.");
                    connect(m_pchBohr, SIGNAL(clicked(bool)), SLOT(update()));

                    m_pchObsRad = new QCheckBox("Observation sphere");
                    m_pchObsRad->setToolTip("This is an arbitrary limit used to reduce the time to make the observations.<br>"
                                            "The radius should be as small as possible to reduce the computation times, "
                                            "but sufficiently large to include the most probable locations of the wave function.<br>"
                                            "Activate the checkbox to visualize the observation sphere.");
                    connect(m_pchObsRad, SIGNAL(clicked(bool)), SLOT(update()));

                    QCheckBox *pchAxes = new QCheckBox("Axes");
                    pchAxes->setChecked(m_bAxes);
                    connect(pchAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));

                    QHBoxLayout *pWidthLayout = new QHBoxLayout;
                    {
                        QLabel *plabPtWidth = new QLabel("Electron size:");
                        QSlider *pPlanetSize = new QSlider(Qt::Horizontal);
                        pPlanetSize->setRange(0, 100);
                        pPlanetSize->setTickInterval(5);
                        pPlanetSize->setTickPosition(QSlider::TicksBelow);
                        pPlanetSize->setValue(int(s_ElectronSize));
                        connect(pPlanetSize,  SIGNAL(sliderMoved(int)),  SLOT(onObjectRadius(int)));

                        pWidthLayout->addWidget(plabPtWidth);
                        pWidthLayout->addWidget(pPlanetSize);
                    }

                    QHBoxLayout *pShaderLayout = new QHBoxLayout;
                    {
                        QString tip("The Geometry Shader (GS) renderer, the instancing renderer and the point renderer are three "
                                    "distinct OpenGL methods implemented in flow5 for comparative testing.<br>"
                                    "The first method generates a sphere on the fly in the geometry shader for each electron position. "
                                    "It is fast but displays coarse spheres. It requires OpenGL 3.2+.<br>"
                                    "The instancing method generates the sphere once and renders it at each electron position. "
                                    "It is slower but displays more regular spheres.<br>"
                                    "The point renderer uses the default OpenGL primitive to render a flat point. "
                                    "It is faster than the other two  methods and can handle larger numbers of instances "
                                    "but displays only flat unshaded points.<br>"
                                    "The instancing technique has been selected for the display of the VPW  since there isn't usually "
                                    "any noticeable speed difference when rendering only a few hundred vortons.");

                        QLabel *plabRend = new QLabel("Renderer:");
                        m_prbPtShader   = new QRadioButton("GS");
                        m_prbSurfShader = new QRadioButton("Instancing");
                        m_prbPt2Shader  = new QRadioButton("Points");
                        m_prbPtShader->setToolTip(tip);
                        m_prbSurfShader->setToolTip(tip);
                        m_prbPt2Shader->setToolTip(tip);
                        s_iRenderer = 0;
                        switch (s_iRenderer)
                        {
                            default:
                            case 0: m_prbPtShader->setChecked(true);    break;
                            case 1: m_prbSurfShader->setChecked(true);  break;
                            case 2: m_prbPt2Shader->setChecked(true);   break;
                        }

                        connect(m_prbPtShader,   SIGNAL(clicked(bool)), SLOT(onRenderer()));
                        connect(m_prbSurfShader, SIGNAL(clicked(bool)), SLOT(onRenderer()));
                        connect(m_prbPt2Shader,  SIGNAL(clicked(bool)), SLOT(onRenderer()));
                        pShaderLayout->addWidget(plabRend);
                        pShaderLayout->addWidget(m_prbPtShader);
                        pShaderLayout->addWidget(m_prbSurfShader);
                        pShaderLayout->addWidget(m_prbPt2Shader);
                        pShaderLayout->addStretch();
                    }
                    QCheckBox *pchClip = new QCheckBox("Clip screen plane");
                    pchClip->setToolTip("Activate this checkbox to hide all objects positioned forward of the screen/viewport.<br>"
                                        "Use the (SHIFT+) X, Y, and Z keys to view the scene in the direction of each axis.");
                    connect(pchClip, SIGNAL(clicked(bool)), SLOT(onClipScreenPlane(bool)));

                    pDisplayLayout->addLayout(pWidthLayout);
                    pDisplayLayout->addLayout(pShaderLayout);
                    pDisplayLayout->addWidget(m_pchBohr);
                    pDisplayLayout->addWidget(m_pchObsRad);
                    pDisplayLayout->addWidget(pchClip);
                    pDisplayLayout->addWidget(pchAxes);
                }
                pDisplayBox->setLayout(pDisplayLayout);
            }

            pFrameLayout->addLayout(pTitleLayout);
            pFrameLayout->addWidget(pQuantumBox);
            pFrameLayout->addWidget(pObsBox);
            pFrameLayout->addWidget(pDisplayBox);
        }
        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        wt::setWidgetStyle(pFrame, palette);
    }
}


void gl3dHydrogen::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dHydrogen");
    {
        s_n             = settings.value("n", s_n).toInt();
        s_l             = settings.value("l", s_l).toInt();
        s_m             = settings.value("m", s_m).toInt();
        s_ObsRadius     = settings.value("ObsDistance",   s_ObsRadius).toDouble();
        s_NObservations = settings.value("NObservations", s_NObservations).toInt();
        s_ElectronSize  = settings.value("ElectronSize", s_ElectronSize).toDouble();
        s_iRenderer     = settings.value("Renderer",      s_iRenderer).toInt();
    }
    settings.endGroup();
}


void gl3dHydrogen::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dHydrogen");
    {
        settings.setValue("n", s_n);
        settings.setValue("l", s_l);
        settings.setValue("m", s_m);
        settings.setValue("NObservations", s_NObservations);
        settings.setValue("ObsDistance",   s_ObsRadius);
        settings.setValue("ElectronSize",  s_ElectronSize);
        settings.setValue("Renderer",      s_iRenderer);
    }
    settings.endGroup();
}


void gl3dHydrogen::onRenderer()
{
    if     (m_prbPtShader->isChecked())   s_iRenderer = 0;
    else if(m_prbSurfShader->isChecked()) s_iRenderer = 1;
    else if(m_prbPt2Shader->isChecked())  s_iRenderer = 2;
    update();
}


void gl3dHydrogen::onObjectRadius(int size)
{
    s_ElectronSize = size;
    update();
}


void gl3dHydrogen::showEvent(QShowEvent *pEvent)
{
    // make sure the window is displayed before starting the observations
    QTimer::singleShot(10, this, SLOT(onCollapse()));

    gl3dTestGLView::showEvent(pEvent);
}


void gl3dHydrogen::hideEvent(QHideEvent *pEvent)
{
    m_bCancel = true;
    gl3dTestGLView::hideEvent(pEvent);
}


void gl3dHydrogen::closeEvent(QCloseEvent *pEvent)
{
    m_bCancel = true;
    QOpenGLWidget::closeEvent(pEvent);
}


void gl3dHydrogen::initializeGL()
{
    gl3dTestGLView::initializeGL();
    glMakeIcoSphere(3);
}


void gl3dHydrogen::glMake3dObjects()
{
    if(m_bResetPositions)
    {
        float state(0);
        int stride = 8;
        int buffersize = m_Pts.size()*stride;
        QVector<float> pts(buffersize);
        int iv =0;
        for(int i=0; i<m_Pts.size(); i++)
        {
            state = m_State.at(i)/m_StateMax;// state is converted to colour in the Point shader
            pts[iv++] = m_Pts.at(i).xf();
            pts[iv++] = m_Pts.at(i).yf();
            pts[iv++] = m_Pts.at(i).zf();
            pts[iv++] = state;

            pts[iv++] = xfl::getRed(  state); // for the surface shader
            pts[iv++] = xfl::getGreen(state);
            pts[iv++] = xfl::getBlue( state);
            pts[iv++] = 1.0f; //alpha
        }

        Q_ASSERT(iv==buffersize);

        if(m_vboObservations.isCreated()) m_vboObservations.destroy();
        m_vboObservations.create();
        m_vboObservations.bind();
        m_vboObservations.allocate(pts.data(), buffersize * int(sizeof(GLfloat)));
        m_vboObservations.release();

        m_bResetPositions = false;
    }
}


void gl3dHydrogen::glRenderView()
{
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);


    if(s_iRenderer==0)
    {
        m_shadPoint.bind();
        {
            m_shadPoint.setUniformValue(m_locPoint.m_vmMatrix,  vmMat);
            m_shadPoint.setUniformValue(m_locPoint.m_pvmMatrix, pvmMat);
        }
        m_shadPoint.release();
        paintPoints(m_vboObservations, float(s_ElectronSize)/50.0f, 0, true, Qt::black, 8);
    }
    else if(s_iRenderer==1)
    {
        m_shadSurf.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  vmMat);
            m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
        }
        m_shadSurf.release();
        paintElectronInstances(m_vboObservations, float(s_ElectronSize)/5000.f/m_glScalef, Qt::cyan, false, true);
    }
    else
    {
        QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

        m_shadPoint2.bind();
        {
            int stride = 8;
            m_shadPoint2.setUniformValue(m_locPt2.m_vmMatrix,  vmMat);
            m_shadPoint2.setUniformValue(m_locPt2.m_pvmMatrix, pvmMat);
            m_shadPoint2.setUniformValue(m_locPt2.m_Shape, float(s_ElectronSize)/2.0f);
            m_shadPoint2.setUniformValue(m_locPt2.m_ClipPlane, m_ClipPlanePos);

            m_shadPoint2.enableAttributeArray(m_locPt2.m_attrVertex);
            m_shadPoint2.enableAttributeArray(m_locPt2.m_attrColor);

            m_vboObservations.bind();
            {
                m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrVertex, GL_FLOAT, 0,                  4, stride * sizeof(GLfloat));
                m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrColor,  GL_FLOAT, 4* sizeof(GLfloat), 4, stride * sizeof(GLfloat));

                int nPoints = m_vboObservations.size()/stride/int(sizeof(float));

                glEnable (GL_POINT_SPRITE);
                glEnable(GL_PROGRAM_POINT_SIZE); // To set the point size from a shader, enable the glEnable with argument (GL_PROGRAM_POINT_SIZE)
                glDrawArrays(GL_POINTS, 0, nPoints);
            }
            m_vboObservations.release();

            m_shadPoint2.disableAttributeArray(m_locPt2.m_attrVertex);
            m_shadPoint2.disableAttributeArray(m_locPt2.m_attrColor);
        }
        m_shadPoint2.release();
    }

    if(m_pchBohr->isChecked())   paintSphere(0,0,0, 1,           QColor(201,201,201,171));
    if(m_pchObsRad->isChecked()) paintSphere(0,0,0, s_ObsRadius, QColor(71,71,71,71));

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dHydrogen::paintElectronInstances(QOpenGLBuffer &vboPosInstances, float radius, QColor const &clr, bool bTwoSided, bool bLight)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int stride(8);
    int nTriangles(0);
    int nObjects(0);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_Scale, radius);
        m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_IsInstanced, 1);
        if(bTwoSided) m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 1);
        else          m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0);
        if(bLight) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
        else       m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, clr);
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1); // using color attribute instead

        m_vboIcoSphere.bind();
        {
            m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
            m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);

            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                 3, 6*sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat));

            nTriangles = m_vboIcoSphere.size()/3/6/int(sizeof(float));
            glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);
        }
        m_vboIcoSphere.release();

        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 0); // using color attribute instead
        vboPosInstances.bind();
        {
            m_shadSurf.enableAttributeArray(m_locSurf.m_attrOffset);
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrOffset, GL_FLOAT, 0*sizeof(GLfloat), 3, stride*sizeof(GLfloat));
            glVertexAttribDivisor(m_locSurf.m_attrOffset, 1);

            m_shadSurf.enableAttributeArray(m_locSurf.m_attrColor);
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrColor,  GL_FLOAT, 4*sizeof(GLfloat), 4, stride*sizeof(GLfloat));
            glVertexAttribDivisor(m_locSurf.m_attrColor, 1);

            nObjects = vboPosInstances.size()/stride/int(sizeof(float));
            glDrawArraysInstanced(GL_TRIANGLES, 0, nTriangles*3, nObjects);

            glVertexAttribDivisor(m_locSurf.m_attrOffset, 0);
            glVertexAttribDivisor(m_locSurf.m_attrColor, 0);
            m_shadSurf.disableAttributeArray(m_locSurf.m_attrOffset);
            m_shadSurf.setUniformValue(m_locSurf.m_IsInstanced, 0);
        }
        vboPosInstances.release();
    }
    m_shadSurf.release();
}


#define NLAT  30
#define NLONG 50

void gl3dHydrogen::onHarmonic()
{
    s_l = m_piel->value();
    s_m = m_piem->value();
    s_n = m_pien->value();

    m_Pts.resize(NLONG*NLAT);
    m_State.resize(NLONG*NLAT);
    int iv =0;
    for(int i=0; i<NLONG; i++)
    {
        double phi = double(i)/double(NLONG)*2.0*PI; //longitude
        for(int j=0; j<NLAT; j++)
        {
            double theta = double(j)/double(NLAT-1)*PI; //colatitude
            std::complex<double> amp = LaplaceHarmonic(s_m, s_l, theta, phi);
            double r = amp.real();

            m_Pts[iv].x = fabs(r) * sin(theta)*cos(phi);
            m_Pts[iv].y = fabs(r) * sin(theta)*sin(phi);
            m_Pts[iv].z = fabs(r) * cos(theta);

            m_State[iv] = r>0 ? 0.8f : 0.2f;
            iv++;
        }
    }

    m_bResetPositions = true;
    update();
}


/**
 *  Ground state wave function
 *  r is in Bohr radius units*/
double gl3dHydrogen::psi_1s(double r, double, double)
{
    return 1.0/sqrt(PI*A0*A0*A0)*exp(-r);
}


/** r is in Bohr radius units
*  theta is the colatitude in [0, PI]
*  phi is the longitude in [0, 2.PI]
*/
double gl3dHydrogen::psi(double r, double theta, double phi) const
{
//    double hb = PLANCK/2.0/PI;  // Planck reduced
//    double a0 = 4.0*PI * EPS0 * hb*hb / m_e / q_e / q_e; //  Bohr radius = 5.29177210903e-11
//    double a0star = 5.2946541e-11; // reduced Bohr radius
    double rho = 2.0*r/double(s_n);
    double coef = 1.0;
    coef *= sqrt((2.0/double(s_n)/A0)*(2.0/double(s_n)/A0)*(2.0/double(s_n)/A0));
//    double fact1 = dfactorial(s_n-s_l-1);
//    double fact2 = dfactorial(s_n+s_l);
    double fact1 = double(factorial(s_n-s_l-1));
    double fact2 = double(factorial(s_n+s_l));
    coef *= sqrt(fact1/2.0/double(s_n)/fact2);
    coef *= exp(-rho/2);
    coef *= pow(rho, s_l);
    coef *= Laguerre(2*s_l+1, s_n-s_l-1, rho);
    std::complex<double> wavefunc =   coef * LaplaceHarmonic(s_m, s_l, theta, phi);
    return wavefunc.real();
}


void gl3dHydrogen::onCollapse()
{
    if(m_bIsObserving)
    {
        m_ppbMake->setText(tr("Start observations"));
        m_bCancel = true;
        m_bIsObserving = false;
        update();
        return;
    }
    m_ppbMake->setText(tr("Stop observations"));

    m_StateMax = 0;
    m_Pts.clear();
    m_State.clear();
    m_plabNObs->clear();

    m_bResetPositions = true;
    update();

    s_n = m_pien->value();
    s_l = m_piel->value();
    s_m = m_piem->value();

    if(s_l>=s_n)
    {
        s_l = s_n-1;
        m_piel->setValue(s_l);
    }
    if(abs(s_m)>s_l)
    {
        if(s_m>0) s_m =  s_l;
        else      s_m = -s_l;
        m_piem->setValue(s_m);
    }

    s_NObservations = m_pieNObs->value();
    s_ObsRadius = m_pdeObsRad->value();

    setReferenceLength(s_ObsRadius);

    m_StateMax = 0.0;

    m_bCancel  = false;
    m_bIsObserving = true;

//    int nThreads = QThread::idealThreadCount();
    int nThreads = 1;
    m_BlockSize = s_NObservations/nThreads+1;
    m_UpdateInterval = std::min(m_BlockSize, 50);

    for(int i=0; i<nThreads; i++)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        QFuture<void> future = QtConcurrent::run(&gl3dHydrogen::collapseBlock, this, this);
#else
        QtConcurrent::run(this, &gl3dHydrogen::collapseBlock, this);
#endif
//        collapseBlock(this);
    }
}


void gl3dHydrogen::collapseBlock(QWidget *pParent) const
{
    double wavefunc(0), probdens(0);
    double r(0), rho(0), theta(0), phi(0), p(0);
    QVector<float>tmpstate(m_UpdateInterval); // temporary working array
    QVector<Vector3d>tmppos(m_UpdateInterval); //  temporary working array

    int counter = 0;
    int iv = 0;
    do
    {
        rho   = pow(QRandomGenerator::global()->generateDouble(),0.33333)*s_ObsRadius;
        r = rho*A0;// in Bohr radius units

        phi   = QRandomGenerator::global()->bounded(2.0*PI);
        theta = acos(1.0-2.0*QRandomGenerator::global()->generateDouble());

        p     = QRandomGenerator::global()->generateDouble()/A0; // set max to 1/A0

        wavefunc = psi(rho, theta, phi);  // r.dr.dtheta.dphi
        probdens = r*r * wavefunc*wavefunc;

        if(p<probdens)
        {
            tmppos[iv].x = r/A0 * sin(theta)*cos(phi);
            tmppos[iv].y = r/A0 * sin(theta)*sin(phi);
            tmppos[iv].z = r/A0 * cos(theta);
            tmpstate[iv] = probdens;

            iv++;
            counter++;
            if(iv==m_UpdateInterval || counter==m_BlockSize)
            {
                HydrogenEvent *pHEvent = new HydrogenEvent;
                pHEvent->setNewPoints(tmppos);
                pHEvent->setNewStates(tmpstate);
                if(!m_bCancel) qApp->postEvent(pParent, pHEvent);
                iv=0;
            }
        }
    }
    while(counter<m_BlockSize && !m_bCancel);
}


void gl3dHydrogen::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == HYDROGEN_EVENT)
    {
        HydrogenEvent const *pHEvent = dynamic_cast<HydrogenEvent*>(pEvent);

        m_Pts.append(pHEvent->newPoints());
        m_State.append(pHEvent->newStates());

        if(m_Pts.size()>=s_NObservations)
        {
            m_Pts.resize(s_NObservations);
            m_State.resize(s_NObservations);

            m_bIsObserving = false;
            m_ppbMake->setText(tr("Start observations"));
        }

        for(int i=0; i<pHEvent->newStates().size(); i++)
            m_StateMax = std::max(m_StateMax, pHEvent->newStates().at(i));


        m_plabNObs->setText(tr("%1/%2").arg(int(m_Pts.size()), 5).arg(s_NObservations, 5));

        m_bResetPositions = true;

        update();
    }
    else
        gl3dTestGLView::customEvent(pEvent);
}

