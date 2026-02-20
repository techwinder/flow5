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

#include <QDebug>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QGuiApplication>
#include <QScreen>
#include <QImage>
#include <QRandomGenerator>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStandardPaths>


#include "gl3dflowvtx.h"
#include <interfaces/widgets/globals/wt_globals.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <core/displayoptions.h>
#include <api/trace.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>

#define GROUP_SIZE 64
#define REFLENGTH 1.0f

int gl3dFlowVtx::s_NGroups(64);
float gl3dFlowVtx::s_dt(0.001f);
float gl3dFlowVtx::s_VInf(10.0f);
float gl3dFlowVtx::s_Gamma(1.0f);

gl3dFlowVtx::gl3dFlowVtx(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Vortex flow");

    m_bAxes = false;

    m_stackInterval.resize(50);

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
//        pFrame->setMinimumWidth(550);

        QGridLayout*pMainLayout = new QGridLayout;
        {            
            QLabel *plabTitle = new QLabel("<p>Using OpenGL's compute shader to compute<br>the flow around a horseshoe vortex.<br>"
                                           "Requires OpenGL 4.3+.</p>");

            QLabel *plabGroupSize = new QLabel(QString::asprintf("Group size = %d", GROUP_SIZE));
            m_plabNMaxGroups = new QLabel("Max. number of groups");
            QLabel *plabNGroups = new QLabel("Number of groups =");
            m_pieNGroups = new IntEdit(s_NGroups);
            m_pieNGroups->setToolTip("The number of groups should be less than the max. number of groups accepted by the GPU.<br>"
                                     "The GroupSize is hard-coded in the compute shader.<br>"
                                     "The number of particles is NGroups x GroupSize.<br>"
                                     "Note that in the present case the main limitations to the number of groups are:"
                                     "<ul>"
                                     "<li>the processing time in the compute shader</li>"
                                     "<li>the number of particles that can be rendered without loss of frame rate</li>"
                                     "</ul>");

            m_plabNParticles = new QLabel;

            QLabel *plabGamma = new QLabel("<p>&Gamma;=</p>");
            m_pfeGamma = new FloatEdit(s_Gamma);
            m_pfeGamma->setToolTip("The vortex's circulation (i.e. strength)");

            QLabel *plabVInf = new QLabel("V<sub>&infin;</sub>=");
            m_pfeVInf = new FloatEdit(s_VInf);
            m_pfeVInf->setToolTip("The velocity of the incoming flow");

            QLabel *plabDt = new QLabel("dt=");
            m_pfeDt = new FloatEdit(s_dt);
            m_pfeDt->setToolTip("<p>The time step of the Runge-Kutta scheme.<br>"
                                "The boids (particles) are moved every 1/60 seconds by an increment of V.dt</p>");


            m_plabFrameRate = new QLabel;
            m_plabFrameRate->setFont(DisplayOptions::tableFont());

            QPushButton *ppbPause = new QPushButton("Pause/Resume");
            connect(ppbPause, SIGNAL(clicked()), SLOT(onPause()));

            pMainLayout->addWidget(plabTitle,           1, 1, 1, 2);
            pMainLayout->addWidget(plabGroupSize,       2, 1);
            pMainLayout->addWidget(plabNGroups,         3, 1);
            pMainLayout->addWidget(m_pieNGroups,        3, 2);
            pMainLayout->addWidget(m_plabNParticles,    4, 1, 1, 2);
            pMainLayout->addWidget(plabGamma,           5, 1);
            pMainLayout->addWidget(m_pfeGamma,          5, 2);
            pMainLayout->addWidget(plabVInf,            8, 1);
            pMainLayout->addWidget(m_pfeVInf,           8, 2);
            pMainLayout->addWidget(plabDt,              9, 1);
            pMainLayout->addWidget(m_pfeDt,             9, 2);
            pMainLayout->addWidget(m_plabFrameRate,    10, 1, 1, 2);

            pMainLayout->addWidget(ppbPause,           12, 1, 1, 2);

            pMainLayout->setColumnStretch(3,1);
            pMainLayout->setRowStretch(11,1);
        }
        pFrame->setLayout(pMainLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        wt::setWidgetStyle(pFrame, palette);
    }

    m_bResetBoids = true;
    m_bResetVortices = true;

    m_locGamma = -1;
    m_locVInf = -1;
    m_locNVortices = -1;
    m_locDt = -1;
    m_locRandSeed = -1;


    m_Period = 1;
//    m_Period = int(1000.0/QGuiApplication::primaryScreen()->refreshRate());
//    qDebug()<<"refreshrate="<<QGuiApplication::primaryScreen()->refreshRate()<<period;
    connect(&m_Timer, SIGNAL(timeout()), SLOT(moveThem()));


    connect(m_pieNGroups, SIGNAL(intChanged(int)), SLOT(onRestart()));

    onRestart();
}


void gl3dFlowVtx::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dFlowVtx");
    {
        s_NGroups    = settings.value("NGroups",      s_NGroups).toInt();
        s_Gamma      = settings.value("Gamma",        s_Gamma).toFloat();
        s_VInf       = settings.value("VInf",         s_VInf).toFloat();
        s_dt         = settings.value("dt",           s_dt).toFloat();
    }
    settings.endGroup();
}


void gl3dFlowVtx::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dFlowVtx");
    {
        settings.setValue("NGroups",      s_NGroups);
        settings.setValue("Gamma",        s_Gamma);
        settings.setValue("VInf",         s_VInf);
        settings.setValue("dt",           s_dt);
    }
    settings.endGroup();
}


void gl3dFlowVtx::readParams()
{
    s_NGroups   = m_pieNGroups->value();
}


void gl3dFlowVtx::makeVortices()
{
    m_Vortex.vertex(0).x = 0.0;
    m_Vortex.vertex(0).y = -1.0;
    m_Vortex.vertex(0).z = 0.5;

    m_Vortex.vertex(1).x = 0.0;
    m_Vortex.vertex(1).y = 1.0;
    m_Vortex.vertex(1).z = 0.5;

    m_Vortex.setCirculation(s_Gamma);
}


void gl3dFlowVtx::makeBoids()
{
    int NBoids = s_NGroups * GROUP_SIZE;
     m_Boid.resize(NBoids);

     for(int inboid=0; inboid<NBoids; inboid++)
     {
         Boid & boid = m_Boid[inboid];
         boid.m_Position.x = -REFLENGTH/2.0f+QRandomGenerator::global()->generateDouble()*REFLENGTH*3.0;
         boid.m_Position.y = (QRandomGenerator::global()->generateDouble()*3.0-1.5)*REFLENGTH;
         boid.m_Position.z = (QRandomGenerator::global()->generateDouble()*2.0-1.0)*REFLENGTH;
         boid.Index = inboid;
         boid.m_Velocity.set(0.1,0,0);
     }
}


void gl3dFlowVtx::onPause()
{
    if(m_Timer.isActive())
    {
        m_Timer.stop();
    }
    else
    {
        m_Timer.start(m_Period); //  As a special case, a QTimer with a timeout of 0 will time out as soon as possible
    }
}


void gl3dFlowVtx::onRestart()
{
    m_Timer.stop();
    readParams();
    makeVortices();
    makeBoids();
    m_bResetBoids = true;
    m_bResetVortices = true;

    m_Timer.start(m_Period); //  As a special case, a QTimer with a timeout of 0 will time out as soon as possible
}


void gl3dFlowVtx::initializeGL()
{
    gl3dTestGLView::initializeGL();

#ifndef Q_OS_MAC
    QString csrc, strange;

    csrc = ":/shaders/flow/flowVtx_CS.glsl";
    m_shadCompute.addShaderFromSourceFile(QOpenGLShader::Compute, csrc);
    if(m_shadCompute.log().length())
    {
        strange = QString::asprintf("%s", QString("Compute shader log:"+m_shadCompute.log()).toStdString().c_str());
        xfl::trace(strange);
    }

    if(!m_shadCompute.link())
    {
        xfl::trace("Compute shader is not linked");
    }
    m_shadCompute.bind();
    {
        m_locGamma     = m_shadCompute.uniformLocation("gamma");
        m_locVInf      = m_shadCompute.uniformLocation("vinf");
        m_locNVortices = m_shadCompute.uniformLocation("nvortices");
        m_locDt        = m_shadCompute.uniformLocation("dt");
        m_locRandSeed  = m_shadCompute.uniformLocation("randseed");
        m_shadCompute.setUniformValue(m_locRandSeed, QRandomGenerator::global()->generate());
    }
    m_shadCompute.release();


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

    m_plabNMaxGroups->setText(tr("Max. number of groups = 2<sup>%1</sup>").arg(pow));
#endif
}


void gl3dFlowVtx::glMake3dObjects()
{
    if(m_bResetVortices)
    {
        // Create a VBO and a SSBO for the vortices
        // VBO is used for display and SSBO is used in the compute shader

        // create the minimal SSBO
        //need to use 4 components for positions due to std140/430 padding constraints for vec3
        int buffersize =  2*(3+1); // (2 vortices * (3 components+ 1 float padding) )
        QVector<float>BufferArray(buffersize);
        int iv=0;


        BufferArray[iv++] = m_Vortex.vertexAt(0).xf();
        BufferArray[iv++] = m_Vortex.vertexAt(0).yf();
        BufferArray[iv++] = m_Vortex.vertexAt(0).zf();
        BufferArray[iv++] = 1.0f;

        BufferArray[iv++] = m_Vortex.vertexAt(1).xf();
        BufferArray[iv++] = m_Vortex.vertexAt(1).yf();
        BufferArray[iv++] = m_Vortex.vertexAt(1).zf();
        BufferArray[iv++] = 1.0f;


        Q_ASSERT(iv==buffersize);

        if(m_ssboVortices.isCreated()) m_ssboVortices.destroy();
        m_ssboVortices.create();
        m_ssboVortices.bind();
        {
            m_ssboVortices.allocate(BufferArray.data(), buffersize * sizeof(GLfloat));
        }
        m_ssboVortices.release();


        //Create the VBO
        buffersize = 1*3*2*3; //3 segments * 2 vortices * 3 components

        BufferArray.resize(buffersize);
        iv=0;

        Vortex const &vortex = m_Vortex;

        BufferArray[iv++] = vortex.vertexAt(0).xf()+5.0f;
        BufferArray[iv++] = vortex.vertexAt(0).yf();
        BufferArray[iv++] = vortex.vertexAt(0).zf();

        BufferArray[iv++] = vortex.vertexAt(0).xf();
        BufferArray[iv++] = vortex.vertexAt(0).yf();
        BufferArray[iv++] = vortex.vertexAt(0).zf();

        BufferArray[iv++] = vortex.vertexAt(0).xf();
        BufferArray[iv++] = vortex.vertexAt(0).yf();
        BufferArray[iv++] = vortex.vertexAt(0).zf();

        BufferArray[iv++] = vortex.vertexAt(1).xf();
        BufferArray[iv++] = vortex.vertexAt(1).yf();
        BufferArray[iv++] = vortex.vertexAt(1).zf();

        BufferArray[iv++] = vortex.vertexAt(1).xf();
        BufferArray[iv++] = vortex.vertexAt(1).yf();
        BufferArray[iv++] = vortex.vertexAt(1).zf();

        BufferArray[iv++] = vortex.vertexAt(1).xf()+5.0f;
        BufferArray[iv++] = vortex.vertexAt(1).yf();
        BufferArray[iv++] = vortex.vertexAt(1).zf();

        Q_ASSERT(iv==buffersize);

        if(m_vboVortices.isCreated()) m_vboVortices.destroy();
        m_vboVortices.create();
        m_vboVortices.bind();
        {
            m_vboVortices.allocate(BufferArray.data(), buffersize * sizeof(GLfloat));
        }
        m_vboVortices.release();

        m_bResetVortices = false;
    }

    if(m_bResetBoids)
    {
        // use only one buffer object used both as VBO and SSBO
        int NBoids = s_NGroups * GROUP_SIZE;
        m_plabNParticles->setText(tr("Number of particles = %1").arg(NBoids));
        //need to use v4 vertices for velocity due to std140/430 padding constraints for vec3:
        int buffersize = NBoids*(4+4+4); //4 vertices + 4 velocity + 4 color components for each boid

        QColor clr(Qt::darkCyan);
        QVector<float>BufferArray(buffersize);
        int iv=0;
        for(int i=0; i<NBoids; i++)
        {
            Boid const &boid = m_Boid.at(i);
            BufferArray[iv++] = boid.m_Position.xf();
            BufferArray[iv++] = boid.m_Position.yf();
            BufferArray[iv++] = boid.m_Position.zf();
            BufferArray[iv++] = 1.0f;

            BufferArray[iv++] = boid.m_Velocity.xf();
            BufferArray[iv++] = boid.m_Velocity.xf();
            BufferArray[iv++] = boid.m_Velocity.xf();
            BufferArray[iv++] = 0.0f;

            BufferArray[iv++] = clr.redF();
            BufferArray[iv++] = clr.greenF();
            BufferArray[iv++] = clr.blueF();
            BufferArray[iv++] = 1.0f;
        }
        Q_ASSERT(iv==buffersize);

        if(m_ssboBoids.isCreated()) m_ssboBoids.destroy();
        m_ssboBoids.create();
        m_ssboBoids.bind();
        {
            m_ssboBoids.allocate(BufferArray.data(), buffersize * sizeof(GLfloat));
        }
        m_ssboBoids.release();



        buffersize = NBoids*TRACESEGS*2*(4+4); //TRACESEGS segments x 2 pts x (4 vertices + 4 color components)
        BufferArray.resize(buffersize);
        iv=0;
        for(int i=0; i<NBoids; i++)
        {
            Boid const &boid = m_Boid.at(i);
            //  write TRACESEGS segments x 2 vertices, with all vertices set to the boid's position as a starting point
            for(int j=0; j<TRACESEGS; j++)
            {
                for(int k=0; k<2; k++)
                {
                    BufferArray[iv++] = boid.m_Position.xf();
                    BufferArray[iv++] = boid.m_Position.yf();
                    BufferArray[iv++] = boid.m_Position.zf();
                    BufferArray[iv++] = 1.0f;
                    BufferArray[iv++] = clr.redF();
                    BufferArray[iv++] = clr.greenF();
                    BufferArray[iv++] = clr.blueF();
                    BufferArray[iv++] = 1.0f;
                }
            }
        }
        Q_ASSERT(iv==buffersize);
        if(m_vboTraces.isCreated()) m_vboTraces.destroy();
        m_vboTraces.create();
        m_vboTraces.bind();
        {
            m_vboTraces.allocate(BufferArray.data(), buffersize * sizeof(GLfloat));
//            qDebug("Boids trace size = %.2f MB", float(m_vboTraces.size())/1024.0f/1024.0f);
        }
        m_vboTraces.release();

        m_bResetBoids = false;
    }
}


void gl3dFlowVtx::moveThem()
{
#ifndef Q_OS_MAC
    s_dt        = m_pfeDt->valuef();
    s_Gamma     = m_pfeGamma->valuef();
    s_VInf      = m_pfeVInf->valuef();

    m_shadCompute.bind();
    {
        m_shadCompute.setUniformValue(m_locRandSeed,  QRandomGenerator::global()->bounded(1024));
        m_shadCompute.setUniformValue(m_locNVortices, 3);
        m_shadCompute.setUniformValue(m_locGamma,     s_Gamma);
        m_shadCompute.setUniformValue(m_locVInf,      s_VInf);
        m_shadCompute.setUniformValue(m_locDt,        s_dt);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboBoids.bufferId());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssboVortices.bufferId());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_vboTraces.bufferId());

        glDispatchCompute(s_NGroups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glFinish();
        getGLError();
    }
    m_shadCompute.release();
    update();
#endif
}


void gl3dFlowVtx::glRenderView()
{
    m_matModel.setToIdentity();
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix,  vmMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat);
    }

    m_vao.bind();
    {
        paintColourSegments8(m_vboTraces, float(W3dPrefs::s_FlowStyle.m_Width), W3dPrefs::s_FlowStyle.m_Stipple);
    }
    m_vao.release();


    paintSegments(m_vboVortices, Qt::darkYellow, 3.0f, Line::SOLID);

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
}


