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

#include <QGuiApplication>
#include <QGridLayout>
#include <QRandomGenerator>
#include <QOpenGLVertexArrayObject>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QTime>

#include "gl3dlorenz2.h"
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <core/displayoptions.h>
#include <api/trace.h>
#include <api/utils.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/globals/wt_globals.h>

#define LORENZ_GROUP_SIZE 512

int gl3dLorenz2::s_NGroups(128);


double gl3dLorenz2::s_X0(7.0);
double gl3dLorenz2::s_Y0(13.0);
double gl3dLorenz2::s_Z0(17.0);
double gl3dLorenz2::s_Scatter(0.25);
double gl3dLorenz2::s_dt(0.002);


float gl3dLorenz2::s_Size = 5.0f;

gl3dLorenz2::gl3dLorenz2(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Lorenz");

    m_bResetParticles = true;

    m_locRadius = -1;
    m_locPosition = m_locFillColor = -1;

    m_stackInterval.resize(50);

    connect(&m_Timer, SIGNAL(timeout()), SLOT(update()));


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
            QLabel *plabTitle = new QLabel("Using OpenGL's compute shader<br>to compute the Lorenz attractor");
            QGridLayout*pMainLayout = new QGridLayout;
            {
                QLabel *plabCS = new QLabel("<b>Compute shader settings:</b>");

                QLabel *plabGroupSize = new QLabel(QString::asprintf("Group size = %d", LORENZ_GROUP_SIZE));
                m_plabNMaxGroups = new QLabel("Max. number of groups");
                QLabel *plabNGroups = new QLabel("Number of groups =");
                m_pieNGroups = new IntEdit(s_NGroups);
                m_pieNGroups->setToolTip("<p>The number of groups should be less than the max. number of groups accepted by the GPU.<br>"
                                         "The Group Size is hard-coded in the compute shader.<br>"
                                         "The number of particles is NGroups x GroupSize.</p>"
                                         "<p>Note that in the present case the main limitation to the number of groups is "
                                         "the number of particles that can be rendered without loss of frame rate.<.p>");

                m_plabNParticles = new QLabel;

                QLabel *pLabAttractor = new QLabel("<b>Attractor settings:</b>");
                QLabel *plabX = new QLabel("x<sub>0</sub> =");
                QLabel *plabY = new QLabel("y<sub>0</sub> =");
                QLabel *plabZ = new QLabel("z<sub>0</sub> =");
                m_pdeX = new FloatEdit(s_X0);
                m_pdeY = new FloatEdit(s_Y0);
                m_pdeZ = new FloatEdit(s_Z0);
                m_pdeX->setToolTip("This defines the starting average position of the particles");
                m_pdeY->setToolTip("This defines the starting average position of the particles");
                m_pdeZ->setToolTip("This defines the starting average position of the particles");

                QLabel *plabScatt = new QLabel("Initial scatter=");
                m_pdeScatter = new FloatEdit(s_Scatter);
                m_pdeScatter->setToolTip("This defines the max. deviation of the particles around the average starting position");


                QLabel *plabDt = new QLabel("dt=");
                m_pdeDt = new FloatEdit(s_dt);
                m_pdeDt->setToolTip("This defines the time step of the RK4 method");

                QPushButton *ppbRestart = new QPushButton("Restart attractor");
                connect(ppbRestart, SIGNAL(clicked(bool)), SLOT(onRestart()));

                m_plabFrameRate = new QLabel;
                m_plabFrameRate->setFont(DisplayOptions::tableFont());

                QLabel *plabSize = new QLabel("Particle size =");
                m_pdeParticleSize = new FloatEdit(s_Size);
                connect(m_pdeParticleSize, SIGNAL(floatChanged(float)), SLOT(onParticleSize()));

                QCheckBox *pchAxes = new QCheckBox("Axes");
                pchAxes->setChecked(true);
                connect(pchAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));

                pMainLayout->addWidget(plabCS,            1,  1, 1, 2);
                pMainLayout->addWidget(plabGroupSize,     2,  1, 1, 2);
                pMainLayout->addWidget(m_plabNMaxGroups,  3,  1, 1, 2);
                pMainLayout->addWidget(plabNGroups,       4,  1);
                pMainLayout->addWidget(m_pieNGroups,      4,  2);
                pMainLayout->addWidget(m_plabNParticles,  5,  1, 1, 2);

                pMainLayout->addWidget(pLabAttractor,     7,  1, 1, 2);
                pMainLayout->addWidget(plabX,             8,  1);
                pMainLayout->addWidget(m_pdeX,            8,  2);
                pMainLayout->addWidget(plabY,             9,  1);
                pMainLayout->addWidget(m_pdeY,            9,  2);
                pMainLayout->addWidget(plabZ,             10,  1);
                pMainLayout->addWidget(m_pdeZ,            10,  2);

                pMainLayout->addWidget(plabScatt,         11,  1);
                pMainLayout->addWidget(m_pdeScatter,      11,  2);

                pMainLayout->addWidget(plabDt,            12,  1);
                pMainLayout->addWidget(m_pdeDt,           12,  2);

                pMainLayout->addWidget(ppbRestart,        13, 1, 1, 2);

                pMainLayout->addWidget(plabSize,          14, 1);
                pMainLayout->addWidget(m_pdeParticleSize, 14, 2);

                pMainLayout->addWidget(pchAxes,           15, 1, 1, 2);
                pMainLayout->addWidget(m_plabFrameRate,   16, 1, 1, 2);

                pMainLayout->setColumnStretch(2,1);
            }
            pFrameLayout->addWidget(plabTitle);
            pFrameLayout->addLayout(pMainLayout);
            pFrameLayout->addStretch();
        }
        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        wt::setWidgetStyle(pFrame, palette);
    }


    setReferenceLength(100.0);

//    int period = int(1000.0/QGuiApplication::primaryScreen()->refreshRate());

    m_Timer.start(0);
}


void gl3dLorenz2::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dLorenz2");
    {
        s_NGroups = settings.value("NGroups",      s_NGroups).toInt();
        s_Size    = settings.value("ParticleSize", s_Size   ).toDouble();
        s_X0      = settings.value("X0",           s_X0     ).toDouble();
        s_Y0      = settings.value("Y0",           s_Y0     ).toDouble();
        s_Z0      = settings.value("Z0",           s_Z0     ).toDouble();
        s_Scatter = settings.value("Scatter",      s_Scatter).toDouble();
        s_dt      = settings.value("dt",           s_dt     ).toDouble();
    }
    settings.endGroup();
}


void gl3dLorenz2::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dLorenz2");
    {
        settings.setValue("NGroups",      s_NGroups);
        settings.setValue("ParticleSize", s_Size);
        settings.setValue("X0",           s_X0     );
        settings.setValue("Y0",           s_Y0     );
        settings.setValue("Z0",           s_Z0     );
        settings.setValue("Scatter",      s_Scatter);
        settings.setValue("dt",           s_dt     );
    }
    settings.endGroup();
}


void gl3dLorenz2::initializeGL()
{
    gl3dTestGLView::initializeGL();

#ifndef Q_OS_MAC
    QString csrc, strange;

    csrc = ":/shaders/lorenz2/lorenz2_CS.glsl";
    m_shadLorenz2.addShaderFromSourceFile(QOpenGLShader::Compute, csrc);
    if(m_shadLorenz2.log().length())
    {
        strange = QString::asprintf("%s", QString("Compute shader log:"+m_shadLorenz2.log()).toStdString().c_str());
        xfl::trace(strange);
    }

    if(!m_shadLorenz2.link())
    {
        xfl::trace("Compute shader is not linked");
    }
    m_shadLorenz2.bind();
    {
        m_locRadius = m_shadLorenz2.uniformLocation("radius");
        m_locDt = m_shadLorenz2.uniformLocation("dt");
    }
    m_shadLorenz2.release();


    int MaxInvocations = 0;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &MaxInvocations);

    int workGroupCounts[3] = { 0 };
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupCounts[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupCounts[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGroupCounts[2]);

    int workGroupSizes[3] = { 0 };
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSizes[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSizes[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSizes[2]);

/*    qDebug("Max invocations = %5d", MaxInvocations);
    qDebug("Work Group Count= %5d %5d %5d", workGroupCounts[0], workGroupCounts[1], workGroupCounts[2]);
    qDebug("Work Group Size = %5d %5d %5d", workGroupSizes[0], workGroupSizes[1], workGroupSizes[2]);*/

    int n = workGroupCounts[0];
    int pow = 1;
    do
    {
        pow++;
        n=n/2;
    }while(n>1);

    m_plabNMaxGroups->setText(QString("Max. number of groups = 2<sup>")+QString::asprintf("%d", pow)+QString("</sup>"));
#endif
}


void gl3dLorenz2::onRestart()
{
    s_X0 = m_pdeX->value();
    s_Y0 = m_pdeY->value();
    s_Z0 = m_pdeZ->value();
    s_NGroups = m_pieNGroups->value();
    s_Scatter = m_pdeScatter->value();
    s_dt = m_pdeDt->value();
    s_Size = m_pdeParticleSize->value();

    m_bResetParticles = true;
}


void gl3dLorenz2::onParticleSize()
{
    s_Size = m_pdeParticleSize->value();
}


void gl3dLorenz2::glMake3dObjects()
{
    if(m_bResetParticles)
    {
        int nParticles = LORENZ_GROUP_SIZE * s_NGroups;
        m_plabNParticles->setText(QString::asprintf("Number of particles =%d", nParticles));

        QVector<LorenzBO> data(nParticles);

        float state(0.0f);

        double X = m_pdeX->value();
        double Y = m_pdeY->value();
        double Z = m_pdeZ->value();
        Vector3d start(X,Y,Z);

        for(int i=0; i<nParticles; i++)
        {
            LorenzBO & BO = data[i];

            double phi   = QRandomGenerator::global()->generateDouble()*2.0*PI;
            double theta = QRandomGenerator::global()->generateDouble()*PI - PI/2.0;
            double r     = QRandomGenerator::global()->generateDouble()*s_Scatter;

            BO.position.setX(start.x + r* cos(phi) * cos(theta)); // non uniform density!
            BO.position.setY(start.y + r* sin(phi) * cos(theta));
            BO.position.setZ(start.z + r* sin(theta));
            BO.position.setW(state);

            // for the surface shader
            BO.color.setX(xfl::getRed(state));
            BO.color.setY(xfl::getGreen(state));
            BO.color.setZ(xfl::getBlue(state));
            BO.color.setW(1.0f); // alpha
        }

        if(!m_ssbParticle.create())
        {
            qDebug("Failed to create the SSBO");
            return;
        }
        m_ssbParticle.bind();
        {
//            m_ssbParticle.setUsagePattern(QOpenGLBuffer::DynamicCopy);
            m_ssbParticle.allocate(data.data(), nParticles*sizeof(LorenzBO));
        }
        m_ssbParticle.release();

//        printBuffer(m_ssbParticle, stride);

        m_bResetParticles = false;
    }
}


void gl3dLorenz2::glRenderView()
{
#ifndef Q_OS_MAC
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLorenz2.bind();
    {
        s_dt = m_pdeDt->value();
        m_shadLorenz2.setUniformValue(m_locRadius, 10.0f);
        m_shadLorenz2.setUniformValue(m_locDt, float(s_dt));

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbParticle.bufferId());
        glDispatchCompute(s_NGroups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glFinish();
        getGLError();
    }
    m_shadLorenz2.release();

    m_shadPoint2.bind();
    {
        // Bind the VBO
//        glBindBuffer(GL_ARRAY_BUFFER, m_ssbParticle.bufferId());
        QMatrix4x4 vmMat(m_matView*m_matModel);
        QMatrix4x4 pvmMat(m_matProj*vmMat);


        m_shadPoint2.setUniformValue(m_locPt2.m_vmMatrix,  vmMat);
        m_shadPoint2.setUniformValue(m_locPt2.m_pvmMatrix, pvmMat);
//        m_shadPoint2.setUniformValue(m_locPt2.m_Shape, 3.0f);
        m_shadPoint2.setUniformValue(m_locPt2.m_ClipPlane, m_ClipPlanePos);

        m_ssbParticle.bind();
        {
            m_shadPoint2.enableAttributeArray(m_locPt2.m_attrVertex);
            m_shadPoint2.enableAttributeArray(m_locPt2.m_attrColor);

            m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrVertex, GL_FLOAT, 0,                  4, 8 * sizeof(GLfloat));
            m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrColor,  GL_FLOAT, 4* sizeof(GLfloat), 4, 8 * sizeof(GLfloat));

    //        glEnable(GL_POINT_SPRITE);
    //        getGLError();
            glPointSize(s_Size);
            getGLError();
    //        glEnable(GL_PROGRAM_POINT_SIZE);
    //        getGLError();

            glDrawArrays(GL_POINTS, 0, s_NGroups * LORENZ_GROUP_SIZE);

            getGLError();

            m_shadPoint2.disableAttributeArray(m_locPt2.m_attrVertex);
            m_shadPoint2.disableAttributeArray(m_locPt2.m_attrColor);
        }
        m_ssbParticle.release();
    }
    m_shadPoint2.release();

    m_stackInterval.push_back(QTime::currentTime().msecsSinceStartOfDay());
    double average = 0.0;
    for(int i=0; i<m_stackInterval.size()-1; i++)
        average += m_stackInterval.at(i+1)-m_stackInterval.at(i);
    average /= double(m_stackInterval.size()-1);
    m_plabFrameRate->setText(QString::asprintf("FPS = %4.1f Hz", 1000.0/average));
    m_stackInterval.pop_front();

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
#endif
}




