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


#include <QGridLayout>
#include <QRandomGenerator>
#include <QFutureSynchronizer>
#include <QtConcurrent/qtconcurrentrun.h>

#define INFLUENCEDIST 3.0
#define MAXSPEED 3.0
#define MAXFORCE 0.03

#include "gl3dboids.h"

#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/globals/wt_globals.h>
#include <core/displayoptions.h>

int gl3dBoids::s_FlockSize = 137;
double gl3dBoids::s_Cohesion   = 1.0;
double gl3dBoids::s_Separation = 1.0;
double gl3dBoids::s_Alignment  = 1.0;


gl3dBoids::gl3dBoids(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Boids");

    m_bResetBox = m_bResetInstances = true;

    m_nBlocks = QThread::idealThreadCount();

    m_bAxes = true;
    m_Radius = 3.0;
    m_X = m_Y = 200.0;
    m_Z = 100.0;

    setReferenceLength(1.5*m_X);

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
            QLabel *plabSwarmSize = new QLabel("Flock size:");
            m_pieFlockSize = new IntEdit(s_FlockSize);
            connect(m_pieFlockSize, SIGNAL(intChanged(int)), SLOT(onSwarmReset()));

            QLabel *plabCohesion   = new QLabel("Cohesion:");
            QLabel *plabSeparation = new QLabel("Separation:");
            QLabel *plabAlignment  = new QLabel("Alignment:");

            m_pslCohesion   = new QSlider(Qt::Horizontal);
            m_pslCohesion->setToolTip("<b>Cohesion:</b> steer to move towards the average position of local flockmates");
            m_pslCohesion->setMinimum(0);
            m_pslCohesion->setMaximum(100);
            m_pslCohesion->setTickInterval(10);
            m_pslCohesion->setTickPosition(QSlider::TicksBelow);
            m_pslCohesion->setValue(int(s_Cohesion*30.0));
            connect(m_pslCohesion, SIGNAL(sliderMoved(int)), SLOT(onSlider()));
            m_plabCohesion = new QLabel;
            m_plabCohesion->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

            m_pslSeparation = new QSlider(Qt::Horizontal);
            m_pslSeparation->setToolTip("<b>Separation:</b> Steer to avoid crowding local flockmates.");
            m_pslSeparation->setMinimum(0);
            m_pslSeparation->setMaximum(100);
            m_pslSeparation->setTickInterval(10);
            m_pslSeparation->setTickPosition(QSlider::TicksBelow);
            m_pslSeparation->setValue(int(s_Separation*30.0));
            connect(m_pslSeparation, SIGNAL(sliderMoved(int)), SLOT(onSlider()));
            m_plabSeparation = new QLabel;
            m_plabSeparation->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));


            m_pslAlignment  = new QSlider(Qt::Horizontal);
            m_pslAlignment->setToolTip("<b>Alignment:</b> Steer towards the average heading of local flockmates.");
            m_pslAlignment->setMinimum(0);
            m_pslAlignment->setMaximum(100);
            m_pslAlignment->setTickInterval(10);
            m_pslAlignment->setTickPosition(QSlider::TicksBelow);
            m_pslAlignment->setValue(int(s_Alignment*30.0));
            connect(m_pslAlignment, SIGNAL(sliderMoved(int)), SLOT(onSlider()));
            m_plabAlignment = new QLabel;
            m_plabAlignment->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

            QLabel *pBoidsLink = new QLabel;
            pBoidsLink->setText("<a href=https://en.wikipedia.org/wiki/Boids>https://en.wikipedia.org/wiki/Boids</a>");
            pBoidsLink->setOpenExternalLinks(true);
            pBoidsLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

            QLabel *plabOpacity   = new QLabel("Box opacity:");
            m_pslBoxOpacity  = new QSlider(Qt::Horizontal);
            m_pslBoxOpacity->setMinimum(0);
            m_pslBoxOpacity->setMaximum(100);
            m_pslBoxOpacity->setTickInterval(10);
            m_pslBoxOpacity->setTickPosition(QSlider::TicksBelow);
            m_pslBoxOpacity->setValue(10);

            QCheckBox *pchAxes = new QCheckBox("Axes");
            pchAxes->setChecked(true);
            connect(pchAxes, SIGNAL(clicked(bool)), SLOT(onAxes(bool)));

            pMainLayout->addWidget(plabSwarmSize,    1, 1);
            pMainLayout->addWidget(m_pieFlockSize,   1, 2);

            pMainLayout->addWidget(plabCohesion,     2, 1);
            pMainLayout->addWidget(m_pslCohesion,    2, 2);
            pMainLayout->addWidget(m_plabCohesion,   2, 3);

            pMainLayout->addWidget(plabSeparation,   3, 1);
            pMainLayout->addWidget(m_pslSeparation,  3, 2);
            pMainLayout->addWidget(m_plabSeparation, 3, 3);

            pMainLayout->addWidget(plabAlignment,    4, 1);
            pMainLayout->addWidget(m_pslAlignment,   4, 2);
            pMainLayout->addWidget(m_plabAlignment,  4, 3);

            pMainLayout->addWidget(pBoidsLink,       5, 1, 1, 3);

            pMainLayout->addWidget(plabOpacity,      6, 1);
            pMainLayout->addWidget(m_pslBoxOpacity,  6, 2);

            pMainLayout->addWidget(pchAxes,          7, 1, 1, 3);
            pMainLayout->setColumnStretch(2,1);
        }
        pFrame->setLayout(pMainLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        wt::setWidgetStyle(pFrame, palette);
    }

    makeBoids(s_FlockSize);

    connect(&m_Timer, SIGNAL(timeout()), SLOT(onMoveBoids()));
    m_Timer.start(16);  // 16.667 ms = 60Hz

    onSlider();
}


void gl3dBoids::onSlider()
{
    s_Cohesion   = m_pslCohesion->value()  /30.0;
    s_Separation = m_pslSeparation->value()/30.0;
    s_Alignment  = m_pslAlignment->value() /30.0;

    m_plabCohesion->setText(  QString::asprintf("%7.3f", s_Cohesion));
    m_plabSeparation->setText(QString::asprintf("%7.3f", s_Separation));
    m_plabAlignment->setText( QString::asprintf("%7.3f", s_Alignment));
}


void gl3dBoids::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dBoids");
    {
        s_FlockSize    = settings.value("FlockSize",  s_FlockSize).toInt();
        s_Cohesion     = settings.value("Cohesion",   s_Cohesion).toDouble();
        s_Separation   = settings.value("Separation", s_Separation).toDouble();
        s_Alignment    = settings.value("Alignment",  s_Alignment).toDouble();
    }
    settings.endGroup();
}


void gl3dBoids::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dBoids");
    {
        settings.setValue("FlockSize",  s_FlockSize);
        settings.setValue("Cohesion",   s_Cohesion);
        settings.setValue("Separation", s_Separation);
        settings.setValue("Alignment",  s_Alignment);
    }
    settings.endGroup();
}


void gl3dBoids::keyPressEvent(QKeyEvent *pEvent)
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
            onMoveBoids();
            break;
    }

    gl3dTestGLView::keyPressEvent(pEvent);
}


void gl3dBoids::glMake3dObjects()
{
    if(m_bResetBox)
    {
//        glMakeCube(Vector3d(), 2.0*m_X, 2.0*m_Y, 2.0*m_Z, m_vboBox, m_vboBoxEdges);
        m_bResetBox = false;
    }
    if(m_bResetInstances)
    {
        QVector<float>PositionArray(m_Boids.size() *3);//3 vertices for each galaxy
        int iv=0;
        for(int i=0; i<m_Boids.size(); i++)
        {
            PositionArray[iv++] = m_Boids.at(i).m_Position.xf();
            PositionArray[iv++] = m_Boids.at(i).m_Position.yf();
            PositionArray[iv++] = m_Boids.at(i).m_Position.zf();
        }

        if(m_vboInstPositions.isCreated()) m_vboInstPositions.destroy();
        m_vboInstPositions.create();
        m_vboInstPositions.bind();
        {
            m_vboInstPositions.allocate(PositionArray.data(), int(PositionArray.size()) * sizeof(GLfloat));
        }
        m_vboInstPositions.release();

        m_bResetInstances = false;
    }
}


void gl3dBoids::glRenderView()
{
    m_matModel.setToIdentity();
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_IsInstanced, 0);
    }
    m_shadSurf.release();

    paintSphereInstances(m_vboInstPositions, m_Radius, QColor(201,194,153), false, true);

    paintBox(0, 0, 0, 2.0*m_X, 2.0*m_Y, 2.0*m_Z, QColor(91,91,91, double(m_pslBoxOpacity->value())/100.0*255.0), true);

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dBoids::makeBoids(int size)
{
    m_Boids.resize(size);

    double velamp = m_Z/2.0;
    velamp = std::min(velamp, m_X/2.0);
    velamp = std::min(velamp, m_Y/2.0);
    for(int i=0; i<m_Boids.size(); i++)
    {
        Boid &boid = m_Boids[i];
        boid.Index = i;

        boid.m_Position.x = -velamp/2.0 + QRandomGenerator::global()->bounded(velamp);
        boid.m_Position.y = -velamp/2.0 + QRandomGenerator::global()->bounded(velamp);
        boid.m_Position.z = -velamp/2.0 + QRandomGenerator::global()->bounded(velamp);

        boid.m_Velocity.x = -velamp/6.0 + QRandomGenerator::global()->bounded(velamp/3.0);
        boid.m_Velocity.y = -velamp/6.0 + QRandomGenerator::global()->bounded(velamp/3.0);
        boid.m_Velocity.z = -velamp/6.0 + QRandomGenerator::global()->bounded(velamp/3.0);
    }
}


Vector3d gl3dBoids::cohesionForce(Boid const &boid)
{
    double neighbordist = m_X/INFLUENCEDIST;
    Vector3d sum;
    int nNeigh = 0;
    for(int iboid=0; iboid<m_Boids.size(); iboid++)
    {
        Boid const &b = m_Boids.at(iboid);
        if(b.Index!=boid.Index && boid.m_Position.distanceTo(b.m_Position)<neighbordist)
        {
            sum += b.m_Position;
            nNeigh++;
        }
    }

    if(nNeigh==0) return Vector3d();

    sum *= 1.0/double(nNeigh);

    Vector3d desired = sum - boid.m_Position;
    desired.normalize();
    desired *= MAXSPEED;

    Vector3d steer = desired - boid.m_Velocity;
    if(steer.norm()>MAXFORCE) steer.set(steer.normalized()*MAXFORCE);
    return steer;
}


Vector3d gl3dBoids::separationForce(Boid const &boid)
{
    double targetseparation = m_X/INFLUENCEDIST;
    Vector3d steer;
    int count = 0;
    int nBoids=0;
    for(int ib=0; ib<m_Boids.size(); ib++)
    {
        Boid const &b = m_Boids.at(ib);
        if(b.Index != boid.Index)
        {
            double dist = boid.m_Position.distanceTo(b.m_Position);

            if(dist>0.0 && dist<targetseparation)
            {
                Vector3d diff = boid.m_Position - b.m_Position; // points away
                diff.normalize();
                diff *= 1.0/dist;
                steer += diff;
                count++;
            }
            nBoids++;
        }
    }
    (void)nBoids;

    if (count>0)  steer *= 1.0/double(count);

    if (steer.norm()>0.0)
    {
        steer.normalize();
        steer *= MAXSPEED;
        steer -= boid.m_Velocity;
        if(steer.norm()>MAXFORCE) steer.set(steer.normalized()*MAXFORCE);
    }
    return steer;
}


Vector3d gl3dBoids::alignmentForce(Boid const &boid)
{
    double neighbordist = m_X/INFLUENCEDIST;
    Vector3d sum;
    int count = 0;
    for(int ib=0; ib<m_Boids.size(); ib++)
    {
        Boid const &b = m_Boids.at(ib);
        if(b.Index != boid.Index)
        {
            double dist = boid.m_Position.distanceTo(b.m_Position);
            if(dist<neighbordist)
            {
                sum += b.m_Velocity;
                count++;
            }
        }
    }

    if(count>0)
    {
        sum *= 1.0/double(count);
        sum.normalize();
        sum *= MAXSPEED;
        Vector3d steer = sum-boid.m_Velocity;
        if(steer.norm()>MAXFORCE) steer = steer.normalized()*MAXFORCE;
        return steer;
    }

    return Vector3d();
}


void gl3dBoids::onSwarmReset()
{
    m_Timer.stop();
    s_FlockSize =  m_pieFlockSize->value();
    makeBoids(s_FlockSize);
    m_Timer.start(16);
}


void gl3dBoids::onMoveBoids()
{
    QVector<Vector3d> accel(m_Boids.size());

    if(m_Boids.size()>100)
    {
        m_nBlocks = QThread::idealThreadCount();
        QFutureSynchronizer<void> futureSync;
        for(int iBlock=0; iBlock<m_nBlocks; iBlock++)
        {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            futureSync.addFuture(QtConcurrent::run(this, &gl3dBoids::moveBoidBlock, iBlock, accel.data()));
#else
            futureSync.addFuture(QtConcurrent::run(&gl3dBoids::moveBoidBlock, this, iBlock, accel.data()));
#endif
        }
        futureSync.waitForFinished();
    }
    else
    {
        m_nBlocks = 1;
        moveBoidBlock(0, accel.data());
    }

    // move all the boids simultaneously
    int iboid = 0;
    for(Boid &boid : m_Boids)
    {
        boid.m_Velocity += accel.at(iboid);
        if(boid.m_Velocity.norm()>MAXSPEED) boid.m_Velocity.set(boid.m_Velocity.normalized()*MAXSPEED);

        boid.m_Position += boid.m_Velocity;

        // bounce off border
        if (boid.m_Position.x<-m_X+m_Radius) {boid.m_Velocity.x *= -1.0;  boid.m_Position.x = (-m_X+m_Radius) * 0.99;}
        if (boid.m_Position.x> m_X-m_Radius) {boid.m_Velocity.x *= -1.0;  boid.m_Position.x = ( m_X-m_Radius) * 0.99;}

        if (boid.m_Position.y<-m_Y+m_Radius) {boid.m_Velocity.y *= -1.0;  boid.m_Position.y = (-m_Y+m_Radius) * 0.99;}
        if (boid.m_Position.y> m_Y-m_Radius) {boid.m_Velocity.y *= -1.0;  boid.m_Position.y = ( m_Y-m_Radius) * 0.99;}

        if (boid.m_Position.z<-m_Z+m_Radius) {boid.m_Velocity.z *= -1.0;  boid.m_Position.z = (-m_Z+m_Radius) * 0.99;}
        if (boid.m_Position.z> m_Z-m_Radius) {boid.m_Velocity.z *= -1.0;  boid.m_Position.z = ( m_Z-m_Radius) * 0.99;}

        iboid++;
    }

    m_bResetInstances = true;
    update();
}


void gl3dBoids::moveBoidBlock(int iBlock, Vector3d *accel)
{
    int blockSize = int(m_Boids.size()/m_nBlocks) +1;
    int iStart = iBlock*blockSize;
    int maxboids = m_Boids.size();
    int iMax = std::min(iStart+blockSize, maxboids);

    Vector3d fc, fs, fa; // cohesion, separation and alignment forces

    for (int iboid=iStart; iboid<iMax; iboid++)
    {
        Boid const &b = m_Boids.at(iboid);

        fc = cohesionForce(b);
        fs = separationForce(b);
        fa = alignmentForce(b);

        // additional tweaks
//        fb = borderForce(b);

        fc *= s_Cohesion;
        fs *= s_Separation;
        fa *= s_Alignment;

        accel[iboid] = fc + fs + fa;
    }
}


Vector3d gl3dBoids::borderForce(Boid const &boid)
{
    Vector3d fborder;
    double dist(0); //distance to origin
    dist = fabs(boid.m_Position.x-m_X);    fborder.x -= 1.0/(dist*dist);
    dist = fabs(boid.m_Position.x+m_X);    fborder.x += 1.0/(dist*dist);

    dist = fabs(boid.m_Position.y-m_Y);    fborder.y -= 1.0/(dist*dist);
    dist = fabs(boid.m_Position.y+m_Y);    fborder.y += 1.0/(dist*dist);

    dist = fabs(boid.m_Position.z-m_Z);    fborder.z -= 1.0/(dist*dist);
    dist = fabs(boid.m_Position.z+m_Z);    fborder.z += 1.0/(dist*dist);

/*    if(dist>m_SideLength)        fborder =  boid.Position *(-sqrt(dist-m_SideLength));
    if(fborder.norm()>MAXFORCE) fborder.set(fborder.normalized()*MAXFORCE);*/

    return fborder;
}


