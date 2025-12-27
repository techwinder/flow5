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


#define _MATH_DEFINES_DEFINED


#include <QVBoxLayout>
#include <QKeyEvent>

#include "gl3dflightview.h"

#include <interfaces/opengl/controls/gllightdlg.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/controls/w3dprefs.h>
#include <api/geom_global.h>
#include <core/displayoptions.h>
#include <core/stlreaderdlg.h>
#include <api/fuseocc.h>
#include <api/fusesections.h>
#include <api/fusestl.h>
#include <api/fusexfl.h>
#include <api/objects3d.h>
#include <api/planestl.h>
#include <api/planestl.h>
#include <api/planexfl.h>
#include <interfaces/widgets/globals/wt_globals.h>

double gl3dFlightView::s_PlaneScale = 50.0;
LineStyle gl3dFlightView::s_ls = {true, Line::SOLID, 2, fl5Color(200,150,0), Line::NOSYMBOL};

#define s_MaxPts 100
#define s_dt 0.02
#define s_RefreshInterval 20
#define SIDE 17
#define ZTRANS 5

gl3dFlightView::gl3dFlightView(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Flight view");
    m_pglLightDlg = new GLLightDlg;
    m_pglLightDlg->setgl3dView(this);

    m_bResetObject = m_bResetTrace = true;
    m_fboDepthMap=0;
    m_texDepthMap=0;

    m_uDepthLightViewMatrix = -1;
    m_uHasShadow = m_uShadowLightViewMatrix = -1;
    m_attrDepthPos = -1;

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

        QVBoxLayout *pMainLayout = new QVBoxLayout;
        {
            QHBoxLayout *pScaleLayout = new QHBoxLayout;
            {
                QLabel *plabRadius = new QLabel("Plane scale:");
                QSlider *pPlaneSize = new QSlider(Qt::Horizontal);
                pPlaneSize->setRange(0, 100);
                pPlaneSize->setTickInterval(0);
                pPlaneSize->setTickPosition(QSlider::TicksBelow);
                pPlaneSize->setValue(s_PlaneScale);
                connect(pPlaneSize,  SIGNAL(sliderMoved(int)),  SLOT(onObjectScale(int)));

                pScaleLayout->addWidget(plabRadius);
                pScaleLayout->addWidget(pPlaneSize);
                pScaleLayout->addStretch();
            }

            m_pchLightDlg = new QCheckBox("Light settings");
            connect(m_pchLightDlg, SIGNAL(clicked(bool)), m_pglLightDlg, SLOT(setVisible(bool)));

            pMainLayout->addLayout(pScaleLayout);
            pMainLayout->addWidget(m_pchLightDlg);
        }

        pFrame->setLayout(pMainLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        wt::setWidgetStyle(pFrame, palette);
    }

    StlReaderDlg dlg(this);
    if(dlg.importTrianglesFromStlFile(":/textfiles/stl_mesh.stl", 0.001))
        m_Triangles = dlg.triangleList();

    setReferenceLength(SIDE);
    reset3dScale();

    m_Trace.resize(s_MaxPts);
    for(int p=0; p<s_MaxPts; p++)
    {
        m_Trace[p] = Vector3d(5.25, 1.3, 0.1);
    }
    m_iLead = 0;

    connect(&m_Timer, SIGNAL(timeout()), SLOT(moveIt()));
    restartTimer();
//    moveIt(); // initialize position
}


gl3dFlightView::~gl3dFlightView()
{
}


void gl3dFlightView::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dFlightView");
    {
        s_PlaneScale    = settings.value("PlaneScale",  s_PlaneScale).toDouble();
    }
    settings.endGroup();
}


void gl3dFlightView::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dFlightView");
    {
        settings.setValue("PlaneScale",  s_PlaneScale);
    }
    settings.endGroup();
}


void gl3dFlightView::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Space:
        {
            if(m_Timer.isActive()) m_Timer.stop();
            else                   restartTimer();
            return;
        }
        case Qt::Key_F4:
            moveIt();
            break;
    }

    gl3dTestGLView::keyPressEvent(pEvent);
}


void gl3dFlightView::onObjectScale(int size)
{
    size = std::max(size, 1);
    s_PlaneScale = double(size); // [0..2]
    update();
}


void gl3dFlightView::restartTimer()
{
    m_Timer.start(17);
}

// AIZAWA
//double gl3dFlightView::f(double x, double y, double z) const {return x*(z-0.7) - 3.5*y;}
//double gl3dFlightView::g(double x, double y, double z) const {return 3.5*x + y*(z-0.7);}
//double gl3dFlightView::h(double x, double y, double z) const {return 0.6 + 0.95*z - z*z*z/3.0 -(x*x+y*y)*(1.0+0.25*z)+0.1*z*x*x*x;}

//RABINOVICH
//double gl3dFlightView::f(double x, double y, double z) const {return y*(z-1.0+x*x) + 0.1*x;}
//double gl3dFlightView::g(double x, double y, double z) const {return x*(3.0*z+1.0-x*x) + 0.1*y;}
//double gl3dFlightView::h(double x, double y, double z) const {return -2*z*(0.14+x*y);}

//ROSSLER
double gl3dFlightView::f(double ,  double y, double z) const {return -(y+z);}
double gl3dFlightView::g(double x, double y, double )  const {return x+0.2*y;}
double gl3dFlightView::h(double x, double ,  double z) const {return 0.2 + z*(x-5.7);}

void gl3dFlightView::moveIt()
{
    if(m_pchLightDlg->isChecked() && !m_pglLightDlg->isVisible())
        m_pchLightDlg->setChecked(false);

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
    pt.x += dt*(k1 +2*k2 +2*k3 +k4)/6; //double x = pt.x;
    pt.y += dt*(l1 +2*l2 +2*l3 +l4)/6; //double y = pt.y;
    pt.z += dt*(m1 +2*m2 +2*m3 +m4)/6; //double z = pt.z;

    double dx = f(pt.x, pt.y, pt.z);
    double dy = g(pt.x, pt.y, pt.z);
    double dz = h(pt.x, pt.y, pt.z);

    // ROSSLER
    double d2x = -(dy+dz);
    double d2y = dx+0.2*dy;
    double d2z = dz*(pt.x-5.7) + pt.z*(dx);

    /*RABINOVICH
    double d2x = dy*(z-1.0+x*x) + y*(dz+2*x*dx) +  0.1*dx;
    double d2y = dx*(3.0*z+1.0-x*x) + x*(3.0*dz+-2*x*dx) + 0.1*dy;
    double d2z = -2*dz*(0.14+x*y) -2*z*(x*dy+dx*y);*/

    /* AIZAWA
    double d2x = dx*(pt.z-0.7)+pt.x*dz-3.5*dy;
    double d2y = 3.5*dx + dy*(pt.z-0.7) + pt.y*dz;
    double d2z =   0.95*dz - 3*.0*pt.z*pt.z*dz
                - (pt.x*dx+pt.y*dy)*(1.0+0.25*pt.z) - (pt.x*pt.x+pt.y*pt.y)*(0.25*dz)
                + 0.1*dz*pt.x*pt.x*pt.x + 0.1*pt.z*3.0*pt.x*pt.x*dx;*/

    Vector3d tg(dx,dy, dz);      tg.normalize();
    Vector3d nm(d2x, d2y, d2z);  nm.normalize();

    Vector3d const &Pt = m_Trace.at(m_iLead);

    Vector3d k = tg*nm;
    QMatrix4x4 trans;
    trans.translate(Pt.xf(), Pt.yf(), Pt.zf()+ZTRANS);
    QMatrix4x4 r;
    float *f = r.data();
    f[0]=-tg.xf();     f[1]=-tg.yf();    f[ 2]=-tg.zf();
    f[4]=-k.xf();      f[5]=-k.yf();     f[ 6]=-k.zf();
    f[8]=nm.xf();      f[9]=nm.yf();     f[10]=nm.zf();

    m_matPlane = trans*r;
    m_matPlane.scale(s_PlaneScale/50.0);
    m_bResetTrace = true;
    update();
}


void gl3dFlightView::glMake3dObjects()
{
    if(m_bResetTrace)
    {
        int buffersize = (s_MaxPts-1)  // NSegments
                         *2*(4+4);     // 2 vertices * (4 coordinates+ 4 color components)
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

        m_bResetTrace = false;
    }


    if(m_bResetObject)
    {
        gl::makeTriangles3Vtx(m_Triangles, true, m_vboStlTriangulation);
        gl::makeTrianglesOutline(m_Triangles, Vector3d(), m_vboStlOutline);

        gl::makeQuadTex(SIDE, SIDE, m_vboBackgroundQuad);

/*        QVector<Triangle3d> triangles;
        makeSphere(side/8.0, 2, triangles);
        for(int i=0; i<triangles.size(); i++)
            triangles[i].translate(15,0,25);
        glMakeTriangles3Vtx(triangles, true, m_vboSphere);
        glMakeTrianglesOutline(triangles, Vector3d(), m_vboSpherEdges);*/

/*        double m_X(15), m_Y(10), m_Z(13);
        glMakeCube({m_X/2, m_Y/2, m_Z*3/2}, m_X, m_Y, m_Z, m_vboCube, m_vboCubeEdges);*/

        m_bResetObject = false;
    }
}


void gl3dFlightView::glRenderView()
{
    updateLightMatrix();

    // 1. first render to depth map
    if(s_Light.m_bIsLightOn)
        renderToShadowMap();

    // 2. then render scene as normal with shadow mapping (using depth map)
    renderToScreen();

    // paint segments, no shadow
    QMatrix4x4 ModelMatrix;
    ModelMatrix.translate({0,0,ZTRANS});
//    m_pvmMatrix = m_matProj*m_matView*m_matModel*ModelMatrix;

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*ModelMatrix);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel*ModelMatrix);
    }
    m_shadLine.release();

    paintColourSegments8(m_vboTrace, s_ls);
//    paintSegments(m_vboCubeEdges, W3dPrefs::s_OutlineStyle);


//    m_pvmMatrix = m_matProj*m_matView*m_matModel*m_matPlane;

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel*m_matPlane);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel*m_matPlane);
    }
    m_shadLine.release();
    paintSegments(m_vboStlOutline, W3dPrefs::s_OutlineStyle);

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready();
    }
}


void gl3dFlightView::renderToShadowMap()
{
    initDepthMap();

    // Render into the depth framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboDepthMap);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    QMatrix4x4 identity;
    paintTrianglesToDepthMap(m_vboBackgroundQuad,   identity, 8);
    paintTrianglesToDepthMap(m_vboStlTriangulation, m_matPlane, 6);
//    paintTrianglesToDepthMap(m_vboCube, identity, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void gl3dFlightView::renderToScreen()
{
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,   m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix,  m_matProj*m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);
        m_shadSurf.setUniformValue(m_locSurf.m_TexSampler, 0); //TEXTURE0  is the default anyway
        m_shadSurf.setUniformValue(m_locSurf.m_ClipPlane, m_ClipPlanePos);
        m_shadSurf.setUniformValue(m_locSurf.m_Viewport, QVector2D(float(m_GLViewRect.width()), float(m_GLViewRect.height())));
    }
    m_shadSurf.release();

    glViewport(0,0,width()*devicePixelRatio(), height()*devicePixelRatio());
    glCullFace(GL_BACK);

    glActiveTexture(GL_TEXTURE0); // to be consistent with the default sampler2d
    glBindTexture(GL_TEXTURE_2D, m_texDepthMap);

//    m_pvmMatrix = m_matProj*m_matView;
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadSurf.release();
    QMatrix4x4 identity;
    paintTriangles3VtxShadow(m_vboBackgroundQuad, Qt::lightGray,  true, s_Light.m_bIsLightOn, identity, 8);

//    m_pvmMatrix = m_matProj*m_matView*m_matPlane;
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  m_matView*m_matModel*m_matPlane);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView*m_matModel*m_matPlane);
    }
    m_shadSurf.release();
//    paintTriangles3VtxShadow(m_vboCube, Qt::cyan,  false, true, identity, 6);
    paintTriangles3VtxShadow(m_vboStlTriangulation, Qt::darkCyan, false, s_Light.m_bIsLightOn, m_matPlane, 6);
}
