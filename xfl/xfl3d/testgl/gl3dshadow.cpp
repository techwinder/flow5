/****************************************************************************

    flow5
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

#include <QVBoxLayout>

#include "gl3dshadow.h"

#include <xfl3d/controls/gllightdlg.h>
#include <xfl3d/globals/gl_globals.h>
#include <xfl/controls/w3dprefs.h>
#include <xflcore/trace.h>
#include <xflcore/displayoptions.h>
#include <xflgeom/geom_globals/geom_global.h>
#include <xflwidgets/customwts/exponentialslider.h>
#include <xflwidgets/wt_globals.h>


gl3dShadow::gl3dShadow(QWidget *pParent) : gl3dTestGLView(pParent)
{
    setWindowTitle("Shadows");
    m_pglLightDlg = new GLLightDlg;
    m_pglLightDlg->setgl3dView(this);

    m_bResetObjects = true;
    m_fboDepthMap=0;
    m_texDepthMap=0;

    m_uDepthLightViewMatrix = -1;
    m_uHasShadow = m_uShadowLightViewMatrix = -1;
    m_attrDepthPos = -1;

    m_ptexQuad = nullptr;

    QPalette palette;
    palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    palette.setColor(QPalette::Text,  DisplayOptions::textColor());

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
        QVBoxLayout *pMainLayout = new QVBoxLayout;
        {
            QGroupBox *pObjBox = new QGroupBox("Object position, model space");
            {
                QGridLayout*pObjectLayout = new QGridLayout;
                {
                    QLabel *plabX = new QLabel("X:");
                    QLabel *plabY = new QLabel("Y:");
                    QLabel *plabZ = new QLabel("Z:");

                    m_peslXObj   = new ExponentialSlider(Qt::Horizontal);
                    m_peslXObj->setMinimum(0);
                    m_peslXObj->setMaximum(100);
                    m_peslXObj->setTickInterval(10);
                    m_peslXObj->setTickPosition(QSlider::TicksBelow);
                    connect(m_peslXObj, SIGNAL(sliderMoved(int)), SLOT(onObjectPos()));

                    m_peslYObj = new ExponentialSlider(Qt::Horizontal);
                    m_peslYObj->setMinimum(0);
                    m_peslYObj->setMaximum(100);
                    m_peslYObj->setTickInterval(10);
                    m_peslYObj->setTickPosition(QSlider::TicksBelow);
                    connect(m_peslYObj, SIGNAL(sliderMoved(int)), SLOT(onObjectPos()));

                    m_peslZObj  = new ExponentialSlider(false, 1, Qt::Horizontal);
                    m_peslZObj->setMinimum(0);
                    m_peslZObj->setMaximum(100);
                    m_peslZObj->setTickInterval(10);
                    m_peslZObj->setTickPosition(QSlider::TicksBelow);
                    connect(m_peslZObj, SIGNAL(sliderMoved(int)), SLOT(onObjectPos()));

                    pObjectLayout->addWidget(plabX ,     2, 1);
                    pObjectLayout->addWidget(m_peslXObj, 2, 2);

                    pObjectLayout->addWidget(plabY,      3, 1);
                    pObjectLayout->addWidget(m_peslYObj, 3, 2);

                    pObjectLayout->addWidget(plabZ,      4, 1);
                    pObjectLayout->addWidget(m_peslZObj, 4, 2);

                    pObjectLayout->setColumnStretch(2,1);
                }
                pObjBox->setLayout(pObjectLayout);
            }

            QCheckBox *pchTex = new QCheckBox("Use texture");
            connect(pchTex, SIGNAL(clicked(bool)), SLOT(onTexture(bool)));

            m_pchLightDlg = new QCheckBox("Light settings");
            connect(m_pchLightDlg, SIGNAL(clicked(bool)), this, SLOT(onLightSettings(bool)));

            pMainLayout->addWidget(pObjBox);
            pMainLayout->addWidget(m_pchLightDlg);
            pMainLayout->addWidget(pchTex);
        }
        pFrame->setLayout(pMainLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }

    m_Object.set(0,0,0.25);

    m_peslXObj->setRange(-100, 100);
    m_peslYObj->setRange(-100, 100);
    m_peslZObj->setRange(   0, 200);

    m_peslXObj->setExpValuef(m_Object.x*100.0f);
    m_peslYObj->setExpValuef(m_Object.y*100.0f);
    m_peslZObj->setExpValuef(m_Object.z*100.0f);

    connect(&m_CtrlTimer, SIGNAL(timeout()), SLOT(onControls()));
    m_CtrlTimer.start(250);
}


gl3dShadow::~gl3dShadow()
{
    if(m_ptexQuad) delete m_ptexQuad;
}


void gl3dShadow::onLightSettings(bool bShow)
{
    m_bLightVisible = bShow;
    m_pglLightDlg->setVisible(bShow);
    update();
}


void gl3dShadow::onControls()
{
    if(m_pchLightDlg->isChecked() && !m_pglLightDlg->isVisible())
    {
        m_pchLightDlg->setChecked(false);
        m_bLightVisible = false;
        update();
    }
}


void gl3dShadow::onTexture(bool bTex)
{
    // load the texture
    if(m_ptexQuad) delete m_ptexQuad;
    m_ptexQuad = nullptr;
    if(bTex)
    {
        m_ptexQuad = new QOpenGLTexture(QImage(QString(":/images/quadtex.png")));
        m_bResetObjects = true;
    }

    update();
}


void gl3dShadow::onObjectPos()
{
    m_Object.x = m_peslXObj->expValuef()/100.0f;
    m_Object.y = m_peslYObj->expValuef()/100.0f;
    m_Object.z = m_peslZObj->expValuef()/100.0f;

//    glSetupLight();

    m_bResetObjects = true;
    update();
}


void gl3dShadow::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dShadow");
    {
    }
    settings.endGroup();
}


void gl3dShadow::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dShadow");
    {
    }
    settings.endGroup();
}


void gl3dShadow::glMake3dObjects()
{
    if(m_bResetObjects)
    {
        double side = 1;

        QVector<Triangle3d> triangles;
        makeSphere(side/8.0, 2, triangles);
        for(int i=0; i<triangles.size(); i++)
            triangles[i].translate(m_Object);

        gl::makeTriangles3Vtx(triangles, true, m_vboSphere);
        gl::makeTrianglesOutline(triangles, Vector3d(), m_vboSphereEdges);

        gl::makeQuadTex(side, side, m_vboBackgroundQuad);

//        setReferenceLength(2*side);
        setReferenceLength(1.0);
        m_bResetObjects = false;
    }
}


void gl3dShadow::glRenderView()
{
    updateLightMatrix();

    // 1. first render to depth map
    renderToShadowMap();

    // 2. then render scene as normal with shadow mapping (using depth map)
    renderToScreen();
}


void gl3dShadow::renderToShadowMap()
{
    initDepthMap();

    // Render into the depth framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboDepthMap);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    paintTrianglesToDepthMap(m_vboSphere, QMatrix4x4(), 6);
    paintTrianglesToDepthMap(m_vboBackgroundQuad, QMatrix4x4(), 8); //vbo has texture

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void gl3dShadow::renderToScreen()
{
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,   vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix,  pvmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_TexSampler, 0); //TEXTURE0  is the default anyway
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);
        m_shadSurf.setUniformValue(m_locSurf.m_ClipPlane, m_ClipPlanePos);
        m_shadSurf.setUniformValue(m_locSurf.m_Viewport, QVector2D(float(m_GLViewRect.width()), float(m_GLViewRect.height())));
    }
    m_shadSurf.release();

    glViewport(0,0,width()*devicePixelRatio(), height()*devicePixelRatio());
    glCullFace(GL_BACK);

    glActiveTexture(GL_TEXTURE0); // to be consistent with the default sampler2d
    glBindTexture(GL_TEXTURE_2D, m_texDepthMap);

    paintTriangles3VtxShadow(m_vboSphere, Qt::yellow,  false, s_Light.m_bIsLightOn, QMatrix4x4(), 6);
    if(m_ptexQuad)
        paintTriangles3VtxTexture(m_vboBackgroundQuad,  m_ptexQuad, true, s_Light.m_bIsLightOn);//vbo has texture
    else
        paintTriangles3VtxShadow(m_vboBackgroundQuad,  Qt::darkRed, true, s_Light.m_bIsLightOn, QMatrix4x4(), 8);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();
    paintSegments(m_vboSphereEdges, W3dPrefs::s_OutlineStyle);
}





