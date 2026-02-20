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


#include <QTime>
#include <QGridLayout>
#include <QGuiApplication>
#include <QButtonGroup>
#include <QRandomGenerator>
#include <QFutureSynchronizer>
#include <QtConcurrent/qtconcurrentrun.h>

#define INFLUENCEDIST 3.0
#define MAXSPEED 3.0
#define MAXFORCE 0.03

#include "gl3dboids2.h"
#include <core/displayoptions.h>
#include <api/trace.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/globals/wt_globals.h>

int gl3dBoids2::s_NGroups = 32;
float gl3dBoids2::s_MaxSpeed   = 1.0f;
float gl3dBoids2::s_Cohesion   = 1.0f;
float gl3dBoids2::s_Separation = 1.0f;
float gl3dBoids2::s_Alignment  = 1.0f;
float gl3dBoids2::s_Predator   = 1.0f;
float gl3dBoids2::s_Ratio      = 0.15f;


gl3dBoids2::gl3dBoids2(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Boids (GPU)");

    m_bResetBox = m_bResetInstances = true;

    m_stackInterval.resize(50);
    m_stackInterval.fill(0);

    m_bAxes = false;

    m_BoxWidth = 200.0f;

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
        pFrame->setMinimumWidth(350);

        QGridLayout*pMainLayout = new QGridLayout;
        {
            QLabel *plabCS = new QLabel("<b>Compute shader settings:</b>");

            QLabel *plabGroupSize = new QLabel(QString::asprintf("Group size = %d", GROUP_SIZE));
            m_plabNMaxGroups = new QLabel("Max. number of groups");
            QLabel *plabNGroups = new QLabel("Number of groups =");
            m_pieNGroups = new IntEdit(s_NGroups);
            m_pieNGroups->setToolTip("<p>The number of groups should be less than the max. number of groups accepted by the GPU.<br>"
                                     "The GroupSize is hard-coded in the compute shader.<br>"
                                     "The number of particles is NGroups x GroupSize.</p>"
                                     "<p>Note that in the present case the main limitations to the number of groups are:"
                                     "<ul>"
                                     "<li>the processing time in the compute shader</li>"
                                     "<li>the number of particles that can be rendered without loss of frame rate</li>"
                                     "</ul></p>");
            connect(m_pieNGroups, SIGNAL(intChanged(int)), SLOT(onSwarmReset()));

            m_plabNParticles = new QLabel;


            QLabel *plabBoids = new QLabel("<b>Boid settings:</b>");

            QLabel *plabCohesion   = new QLabel("Cohesion:");
            QLabel *plabSeparation = new QLabel("Separation:");
            QLabel *plabAlignment  = new QLabel("Alignment:");

            m_pslCohesion   = new QSlider(Qt::Horizontal);
            m_pslCohesion->setToolTip("<b>Cohesion:</b> steer to move towards the average position of local flockmates");
            m_pslCohesion->setMinimum(0);
            m_pslCohesion->setMaximum(100);
            m_pslCohesion->setTickInterval(10);
            m_pslCohesion->setTickPosition(QSlider::TicksBelow);
            m_pslCohesion->setValue(int(s_Cohesion*30.0f));
            connect(m_pslCohesion, SIGNAL(sliderMoved(int)), SLOT(onSlider()));
            m_plabCohesion = new QLabel;
            m_plabCohesion->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

            m_pslSeparation = new QSlider(Qt::Horizontal);
            m_pslSeparation->setToolTip("<b>Separation:</b> Steer to avoid crowding local flockmates.");
            m_pslSeparation->setMinimum(0);
            m_pslSeparation->setMaximum(100);
            m_pslSeparation->setTickInterval(10);
            m_pslSeparation->setTickPosition(QSlider::TicksBelow);
            m_pslSeparation->setValue(int(s_Separation*30.0f));
            connect(m_pslSeparation, SIGNAL(sliderMoved(int)), SLOT(onSlider()));
            m_plabSeparation = new QLabel;
            m_plabSeparation->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));


            m_pslAlignment  = new QSlider(Qt::Horizontal);
            m_pslAlignment->setToolTip("<b>Alignment:</b> Steer towards the average heading of local flockmates.");
            m_pslAlignment->setMinimum(0);
            m_pslAlignment->setMaximum(100);
            m_pslAlignment->setTickInterval(10);
            m_pslAlignment->setTickPosition(QSlider::TicksBelow);
            m_pslAlignment->setValue(int(s_Alignment*30.0f));
            connect(m_pslAlignment, SIGNAL(sliderMoved(int)), SLOT(onSlider()));
            m_plabAlignment = new QLabel;
            m_plabAlignment->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));


            QLabel *plabMaxSpeed = new QLabel("Speed:");
            m_pslMaxSpeed  = new QSlider(Qt::Horizontal);
            m_pslMaxSpeed->setMinimum(0);
            m_pslMaxSpeed->setMaximum(100);
            m_pslMaxSpeed->setTickInterval(10);
            m_pslMaxSpeed->setTickPosition(QSlider::TicksBelow);
            m_pslMaxSpeed->setValue(int(s_MaxSpeed*30.0f));
            connect(m_pslMaxSpeed, SIGNAL(sliderMoved(int)), SLOT(onSlider()));
            m_plabMaxSpeed = new QLabel;
            m_plabMaxSpeed->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));


            m_pchPredator = new QCheckBox("Predator");
            m_pchPredator->setToolTip("The predator steers towards the average position of the closest boids.<br>"
                                      "It is 50% faster and can accelerate three times more than the boids can.");
            m_pslPredator  = new QSlider(Qt::Horizontal);
            m_pslPredator->setToolTip("Defines the boid's fear level of the predator");
            m_pslPredator->setMinimum(0);
            m_pslPredator->setMaximum(100);
            m_pslPredator->setTickInterval(10);
            m_pslPredator->setTickPosition(QSlider::TicksBelow);
            m_pslPredator->setValue(int(s_Predator*30.0f));
            connect(m_pslPredator, SIGNAL(sliderMoved(int)), SLOT(onSlider()));
            m_plabPredator = new QLabel;
            m_plabPredator->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

            QLabel *plabDisplay = new QLabel("<b>Display settings:</b>");

/*            QButtonGroup *pGroup = new QButtonGroup;
            {
                m_prbBox    = new QRadioButton("Box");
                m_prbSphere = new QRadioButton("Sphere");
                m_prbBox->setChecked(true);
                pGroup->addButton(m_prbBox);
                pGroup->addButton(m_prbSphere);
            }*/

            QLabel *plabRatio = new QLabel("Volume size:");
            m_pslRatio = new QSlider(Qt::Horizontal);
            m_pslRatio->setMinimum(0);
            m_pslRatio->setMaximum(100);
            m_pslRatio->setTickInterval(10);
            m_pslRatio->setTickPosition(QSlider::TicksBelow);
            m_pslRatio->setValue(int(s_Ratio*100.0f));
            connect(m_pslRatio, SIGNAL(sliderMoved(int)), SLOT(onSlider()));

            QLabel *plabOpacity   = new QLabel("Opacity:");
            m_pslBoxOpacity  = new QSlider(Qt::Horizontal);
            m_pslBoxOpacity->setToolTip("<b>Alignment:</b> Steer towards the average heading of local flockmates.");
            m_pslBoxOpacity->setMinimum(0);
            m_pslBoxOpacity->setMaximum(100);
            m_pslBoxOpacity->setTickInterval(10);
            m_pslBoxOpacity->setTickPosition(QSlider::TicksBelow);
            m_pslBoxOpacity->setValue(10);

            m_pchTrace = new QCheckBox("Traces");
            m_pchTrace->setChecked(true);

            QCheckBox *pchAxes = new QCheckBox("Axes");
            pchAxes->setChecked(m_bAxes);
            connect(pchAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));

            m_plabFrameRate = new QLabel;
            m_plabFrameRate->setFont(DisplayOptions::tableFont());

            QLabel *pBoidsLink = new QLabel;
            pBoidsLink->setText("<a href=https://en.wikipedia.org/wiki/Boids>https://en.wikipedia.org/wiki/Boids</a>");
            pBoidsLink->setOpenExternalLinks(true);
            pBoidsLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

            pMainLayout->addWidget(plabCS,            1, 1, 1, 2);
            pMainLayout->addWidget(plabGroupSize,     2, 1, 1, 2);
            pMainLayout->addWidget(m_plabNMaxGroups,  3, 1, 1, 2);
            pMainLayout->addWidget(plabNGroups,       4, 1);
            pMainLayout->addWidget(m_pieNGroups,      4, 2);
            pMainLayout->addWidget(m_plabNParticles,  5, 1, 1, 2);

            pMainLayout->addWidget(plabBoids,         6, 1, 1, 2);

            pMainLayout->addWidget(plabCohesion,      7, 1);
            pMainLayout->addWidget(m_pslCohesion,     7, 2);
            pMainLayout->addWidget(m_plabCohesion,    7, 3);

            pMainLayout->addWidget(plabSeparation,    8, 1);
            pMainLayout->addWidget(m_pslSeparation,   8, 2);
            pMainLayout->addWidget(m_plabSeparation,  8, 3);

            pMainLayout->addWidget(plabAlignment,     9, 1);
            pMainLayout->addWidget(m_pslAlignment,    9, 2);
            pMainLayout->addWidget(m_plabAlignment,   9, 3);

            pMainLayout->addWidget(plabMaxSpeed,      10, 1);
            pMainLayout->addWidget(m_pslMaxSpeed,     10, 2);
            pMainLayout->addWidget(m_plabMaxSpeed,    10, 3);

            pMainLayout->addWidget(m_pchPredator,     11, 1);
            pMainLayout->addWidget(m_pslPredator,     11, 2);
            pMainLayout->addWidget(m_plabPredator,    11, 3);


            pMainLayout->addWidget(plabDisplay,       13, 1, 1, 2);

//            pMainLayout->addWidget(m_prbBox,          14, 1);
//            pMainLayout->addWidget(m_prbSphere,       14, 2);

            pMainLayout->addWidget(plabRatio,         15, 1);
            pMainLayout->addWidget(m_pslRatio,        15, 2);

            pMainLayout->addWidget(plabOpacity,       16, 1);
            pMainLayout->addWidget(m_pslBoxOpacity,   16, 2);

            pMainLayout->addWidget(m_pchTrace,        18, 1, 1, 3);
            pMainLayout->addWidget(pchAxes,           19, 1, 1, 3);
            pMainLayout->addWidget(m_plabFrameRate,   20, 1, 1, 3);
            pMainLayout->addWidget(pBoidsLink,        21, 1, 1, 3);

            pMainLayout->setColumnStretch(2,1);
        }
        pFrame->setLayout(pMainLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        wt::setWidgetStyle(pFrame, palette);
    }

    onSlider();

    setReferenceLength(1.5*m_BoxWidth);
//    setLightOn(false);

//    int period = int(1000.0/QGuiApplication::primaryScreen()->refreshRate());
//    qDebug()<<"refreshrate="<<QGuiApplication::primaryScreen()->refreshRate()<<period;
    connect(&m_Timer, SIGNAL(timeout()), SLOT(update()));
    m_Timer.start(0); //  As a special case, a QTimer with a timeout of 0 will time out as soon as possible
}


void gl3dBoids2::onSlider()
{
    s_Cohesion   = float(m_pslCohesion->value())  /30.0f;
    s_Separation = float(m_pslSeparation->value())/30.0f;
    s_Alignment  = float(m_pslAlignment->value()) /30.0f;
    s_Predator   = float(m_pslPredator->value())  /30.0f;
    s_MaxSpeed   = float(m_pslMaxSpeed->value())  /30.0f;
    s_MaxSpeed = std::max(0.05f, s_MaxSpeed);

    m_plabCohesion->setText(  QString::asprintf("%7.3f", s_Cohesion));
    m_plabSeparation->setText(QString::asprintf("%7.3f", s_Separation));
    m_plabAlignment->setText( QString::asprintf("%7.3f", s_Alignment));
    m_plabPredator->setText(  QString::asprintf("%7.3f", s_Predator));
    m_plabMaxSpeed->setText(  QString::asprintf("%7.3f", s_MaxSpeed));

    s_Ratio = std::max(m_pslRatio->value()/100.0f,0.01f);
}


void gl3dBoids2::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dBoids2");
    {
        s_NGroups      = settings.value("NGroups",    s_NGroups).toInt();
        s_Cohesion     = settings.value("Cohesion",   s_Cohesion).toFloat();
        s_Separation   = settings.value("Separation", s_Separation).toFloat();
        s_Alignment    = settings.value("Alignment",  s_Alignment).toFloat();
        s_Predator     = settings.value("Predator",   s_Predator).toFloat();
        s_MaxSpeed     = settings.value("MaxSpeed",   s_MaxSpeed).toFloat();
        s_Ratio        = settings.value("Ratio",      s_Ratio).toFloat();
    }
    settings.endGroup();
}


void gl3dBoids2::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dBoids2");
    {
        settings.setValue("NGroups",    s_NGroups);
        settings.setValue("Cohesion",   s_Cohesion);
        settings.setValue("Separation", s_Separation);
        settings.setValue("Alignment",  s_Alignment);
        settings.setValue("Predator",   s_Predator);
        settings.setValue("MaxSpee",    s_MaxSpeed);
        settings.setValue("Ratio",      s_Ratio);
    }
    settings.endGroup();
}


void gl3dBoids2::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Space:
            if(m_Timer.isActive()) m_Timer.stop();
            else                   m_Timer.start(0);
            break;
        case Qt::Key_Escape:
            showNormal();
            break;
    }

    gl3dTestGLView::keyPressEvent(pEvent);
}


void gl3dBoids2::initializeGL()
{
    gl3dTestGLView::initializeGL();
#ifndef Q_OS_MAC
    QString csrc, strange;

    csrc = ":/shaders/boids2/boids2_CS.glsl";
    m_shadBoids.addShaderFromSourceFile(QOpenGLShader::Compute, csrc);
    if(m_shadBoids.log().length())
    {
        strange = QString::asprintf("%s", QString("Compute shader log:"+m_shadBoids.log()).toStdString().c_str());
        xfl::trace(strange);
    }

    if(!m_shadBoids.link())
    {
        xfl::trace("Compute shader is not linked");
    }
    m_shadBoids.bind();
    {
        m_locCube        = m_shadBoids.uniformLocation("cube");
        m_locMaxSpeed    = m_shadBoids.uniformLocation("maxspeed");
        m_locWidth       = m_shadBoids.uniformLocation("width");
        m_locHeight      = m_shadBoids.uniformLocation("height");
        m_locCohesion    = m_shadBoids.uniformLocation("cohesion");
        m_locSeparation  = m_shadBoids.uniformLocation("separation");
        m_locAlignment   = m_shadBoids.uniformLocation("alignment");
        m_locPredator    = m_shadBoids.uniformLocation("predatorf");
        m_locHasPredator = m_shadBoids.uniformLocation("haspredator");
    }
    m_shadBoids.release();

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


void gl3dBoids2::glMake3dObjects()
{
    if(m_bResetBox)
    {
        m_bResetBox = false;
    }

    if(m_bResetInstances)
    {
        int nParticles = GROUP_SIZE * s_NGroups;
        m_plabNParticles->setText(tr("Number of particles =%1").arg(nParticles));

        QVector<Vector3d> boids(nParticles);

        double amp = m_BoxWidth/4.0;

        //need to use v4 vertices for velocity due to std140/430 padding constraints for vec3:
        int buffersize = nParticles *(4+4+4); //4 vertices + 4 velocity + 4 color components for each boid
        // double the size to create a virtual double buffer
        // the second half of the buffer is populated in the compute shader
        buffersize *=2;

        QVector<float>BufferArray(buffersize);
        int iv=0;
        for(int i=0; i<nParticles; i++)
        {
            boids[i].x = -amp/2.0f + QRandomGenerator::global()->bounded(amp);
            boids[i].y = -amp/2.0f + QRandomGenerator::global()->bounded(amp);
            boids[i].z = -amp/2.0f + QRandomGenerator::global()->bounded(amp);
            BufferArray[iv++] = boids.at(i).x;
            BufferArray[iv++] = boids.at(i).y;
            BufferArray[iv++] = boids.at(i).z;
            BufferArray[iv++] = 1.0f;

            BufferArray[iv++] = -amp/6.0  + QRandomGenerator::global()->bounded(amp/3.0);
            BufferArray[iv++] = -amp/6.0  + QRandomGenerator::global()->bounded(amp/3.0);
            BufferArray[iv++] = (-amp/6.0  + QRandomGenerator::global()->bounded(amp/3.0))*s_Ratio;
            BufferArray[iv++] = 0.0f;

            BufferArray[iv++] = 0.0f; // overwritten at 1st iteration in the CS
            BufferArray[iv++] = 0.0f;
            BufferArray[iv++] = 0.0f;
            BufferArray[iv++] = 0.0f;
        }
        // rest is zero
//        Q_ASSERT(iv==buffersize/2);
//        QElapsedTimer t;  t.start();

        if(m_vboBoids.isCreated()) m_vboBoids.destroy();
        m_vboBoids.create();
        m_vboBoids.bind();
        {
            m_vboBoids.allocate(BufferArray.data(), buffersize * sizeof(GLfloat));
        }
        m_vboBoids.release();
//qDebug("         Time to Transfer %g MB: %g s\n", double(buffersize * sizeof(GLfloat)/1024/1024), double(t.elapsed())/1000.0);



        QColor clr(Qt::darkCyan);
        buffersize = nParticles*TRACESEGS*2*(4+4); //TRACESEGS segments x 2 pts x (4 vertices + 4 color components)
        BufferArray.resize(buffersize);
        iv=0;
        for(int i=0; i<nParticles; i++)
        {
            Vector3d boid = boids.at(i);
            //  write TRACESEGS segments x 2 vertices, with all vertices set to the boid's position as a starting point
            for(int j=0; j<TRACESEGS; j++)
            {
                for(int k=0; k<2; k++)
                {
                    BufferArray[iv++] = boid.xf();
                    BufferArray[iv++] = boid.yf();
                    BufferArray[iv++] = boid.zf();
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

        m_bResetInstances = false;
    }
}


void gl3dBoids2::glRenderView()
{
#ifndef Q_OS_MAC
    m_matModel.setToIdentity();
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    // move the flock at each frame update
    int stride = 12;
    int buffersize = GROUP_SIZE * s_NGroups * stride;
    m_shadBoids.bind();
    {
//        if(m_prbBox->isChecked())
            m_shadBoids.setUniformValue(m_locCube, 1);
//        else          m_shadBoids.setUniformValue(m_locCube, 0);

        m_shadBoids.setUniformValue(m_locWidth,      m_BoxWidth);
        m_shadBoids.setUniformValue(m_locHeight,     m_BoxWidth*s_Ratio);
        m_shadBoids.setUniformValue(m_locMaxSpeed,   s_MaxSpeed);
        m_shadBoids.setUniformValue(m_locCohesion,   s_Cohesion);
        m_shadBoids.setUniformValue(m_locSeparation, s_Separation);
        m_shadBoids.setUniformValue(m_locAlignment,  s_Alignment);
        m_shadBoids.setUniformValue(m_locPredator,   s_Predator);
        if(m_pchPredator->isChecked()) m_shadBoids.setUniformValue(m_locHasPredator,  1);
        else                           m_shadBoids.setUniformValue(m_locHasPredator,  0);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_vboBoids.bufferId());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_vboTraces.bufferId());

        glDispatchCompute(s_NGroups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glFinish();
        getGLError();

        // virtual double buffering
        glBindBuffer(GL_COPY_READ_BUFFER, m_vboBoids.bufferId());
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_READ_BUFFER, buffersize*sizeof(float), 0, buffersize*sizeof(float));
        getGLError();
    }
    m_shadBoids.release();

    m_vao.bind();
    {
        QMatrix4x4 vmMat(m_matView*m_matModel);
        QMatrix4x4 pvmMat(m_matProj*vmMat);

        m_shadPoint2.bind();
        {

            m_shadPoint2.setUniformValue(m_locPt2.m_vmMatrix,  vmMat);
            m_shadPoint2.setUniformValue(m_locPt2.m_pvmMatrix, pvmMat);

            m_shadPoint2.setUniformValue(m_locPt2.m_ClipPlane, m_ClipPlanePos);

            m_vboBoids.bind();
            {
                int nPts = m_vboBoids.size()/stride/int(sizeof(float)) /2; // first half of the buffer only

                m_shadPoint2.enableAttributeArray(m_locPt2.m_attrVertex);
                m_shadPoint2.enableAttributeArray(m_locPt2.m_attrColor);

                m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrVertex, GL_FLOAT, 0,                  4, stride * sizeof(GLfloat));
                m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrColor,  GL_FLOAT, 8* sizeof(GLfloat), 4, stride * sizeof(GLfloat));

                glPointSize(7.0f);

                if(!m_pchPredator->isChecked())
                    glDrawArrays(GL_POINTS, 0, nPts);
                else
                {
                    glDrawArrays(GL_POINTS, 0, nPts-1);
                    glPointSize(31.0f);
                    glDrawArrays(GL_POINTS, nPts-1, 1);
                }

                m_shadPoint2.disableAttributeArray(m_locPt2.m_attrVertex);
                m_shadPoint2.disableAttributeArray(m_locPt2.m_attrColor);
            }
            m_vboBoids.release();
        }
        m_shadPoint2.release();

        if(m_pchTrace->isChecked())
        {
            m_shadLine.setUniformValue(m_locPt2.m_vmMatrix,  vmMat);
            m_shadLine.setUniformValue(m_locPt2.m_pvmMatrix, pvmMat);
            paintColourSegments8(m_vboTraces, float(W3dPrefs::s_FlowStyle.m_Width), W3dPrefs::s_FlowStyle.m_Stipple);
        }
    }
    m_vao.release();

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_IsInstanced, 0);
    }
    m_shadSurf.release();
//    if(m_prbBox->isChecked())
        paintBox(0, 0, 0, 2.0*m_BoxWidth, 2.0*m_BoxWidth, 2.0*m_BoxWidth*s_Ratio, QColor(91,91,91, double(m_pslBoxOpacity->value())/100.0*255.0), false);
//    else        paintSphere(Vector3d(), m_BoxWidth, QColor(91,91,91, double(m_pslBoxOpacity->value())/100.0*255.0), true);

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


void gl3dBoids2::onSwarmReset()
{
    s_NGroups = m_pieNGroups->value();
    m_bResetInstances = true;
}





