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

#include <QApplication>
#include <QRandomGenerator>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>

#include "gl2dnewton.h"

#include <api/utils.h>
#include <core/displayoptions.h>
#include <api/trace.h>
#include <core/xflcore.h>
#include <interfaces/opengl/views/gl3dview.h> // for the static variables
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/globals/wt_globals.h>


int gl2dNewton::s_MaxIter = 32;
float gl2dNewton::s_Tolerance = 1.e-6f;
QColor gl2dNewton::s_Colors[5] = {QColor(77,27,21), QColor(75,111,117), QColor(11,47,77), QColor(77,77,77), QColor(131,120,107)};


gl2dNewton::gl2dNewton(QWidget *pParent) : gl2dView(pParent)
{
    setWindowTitle("Newton fractal");
//    setMouseTracking(true);

    m_rectView = QRectF(-1.0, -1.0, 2.0, 2.0);

    m_bAxes = false;

    m_bResetRoots = true;

    m_iHoveredRoot = m_iSelectedRoot = -1;

    m_Time = 0;
    for(int i=0; i<2*MAXROOTS; i++)
    {
        m_omega[i] = 2.0*QRandomGenerator::global()->bounded(1.0)-1.0;
    }

    m_locIters = m_locTolerance = -1;

    m_locNRoots    = -1;
    for(int i=0; i<MAXROOTS; i++) m_locColor[i] = m_locRoot[i] = -1;

    QFrame *pCmdFrame = new QFrame(this);
    {
        pCmdFrame->setCursor(Qt::ArrowCursor);

        pCmdFrame->setFrameShape(QFrame::NoFrame);
        pCmdFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QLabel *pLabTitle = new QLabel("Using OpenGL's fragment shader to compute<br> and plot a polynomial Newton fractal.");

            QFormLayout*pParamLayout = new QFormLayout;
            {
                m_pieMaxIter = new IntEdit(s_MaxIter);
                m_pieMaxIter->setToolTip("<p>The number of iterations before bailing out.</p>");

                m_pfeTolerance = new FloatEdit(s_Tolerance);
                m_pfeTolerance->setToolTip("<p>The escape amplitude of z.</p>");
                connect(m_pieMaxIter,   SIGNAL(intChanged(int)), SLOT(update()));
                connect(m_pfeTolerance, SIGNAL(floatChanged(float)), SLOT(update()));

                pParamLayout->addRow("Max. iterations:", m_pieMaxIter);
                pParamLayout->addRow("Tolerance:",       m_pfeTolerance);
            }

            QHBoxLayout *pNRootsLayout = new QHBoxLayout;
            {
                m_prb3roots = new QRadioButton("3 roots");
                m_prb5roots = new QRadioButton("5 roots");
                m_prb3roots->setChecked(true);
                connect(m_prb3roots, SIGNAL(clicked(bool)), SLOT(onNRoots()));
                connect(m_prb5roots, SIGNAL(clicked(bool)), SLOT(onNRoots()));
                pNRootsLayout->addWidget(m_prb3roots);
                pNRootsLayout->addWidget(m_prb5roots);
                pNRootsLayout->addStretch();
            }

            m_pchShowRoots = new QCheckBox("Show roots");
            connect(m_pchShowRoots, SIGNAL(clicked(bool)), SLOT(update()));
            m_pchAnimateRoots = new QCheckBox("Animate roots");
            connect(m_pchAnimateRoots, SIGNAL(clicked(bool)), SLOT(onAnimate(bool)));
            m_plabScale = new QLabel();
            m_plabScale->setFont(DisplayOptions::textFont());

            QHBoxLayout *pWidthLayout = new QHBoxLayout;
            {
                QLabel *plabImgWidth = new QLabel("Image size=");
                QLabel *plabTimes = new QLabel(TIMESch);
                QLabel *plabPixel = new QLabel(tr("pixels"));
                pWidthLayout->addWidget(plabImgWidth);
                pWidthLayout->addWidget(m_pieWidth);
                pWidthLayout->addWidget(plabTimes);
                pWidthLayout->addWidget(m_pieHeight);
                pWidthLayout->addWidget(plabPixel);
                pWidthLayout->addStretch();
            }

            QLabel *pRefLink = new QLabel;
            pRefLink->setText(tr("Inspired by <a href=https://youtu.be/-RdOwhmqP5s>3Blue1Brown's YouTube video</a>"));
            pRefLink->setOpenExternalLinks(true);
            pRefLink->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
            pRefLink->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

            pFrameLayout->addWidget(pLabTitle);
            pFrameLayout->addLayout(pParamLayout);
            pFrameLayout->addLayout(pNRootsLayout);
            pFrameLayout->addWidget(m_pchShowRoots);
            pFrameLayout->addWidget(m_pchAnimateRoots);
            pFrameLayout->addWidget(m_plabScale);
            pFrameLayout->addWidget(m_ppbSaveImg);
            pFrameLayout->addLayout(pWidthLayout);
            pFrameLayout->addWidget(pRefLink);
        }

        pCmdFrame->setLayout(pFrameLayout);

        pCmdFrame->setStyleSheet("QFrame{background-color: transparent; color: white}"
                              "QRadioButton{background-color: transparent; color: white}"
                              "QCheckBox{background-color: transparent; color: white}");

//        wt::setWidgetStyle(pFrame, palette);
    }
    connect(&m_Timer, SIGNAL(timeout()), SLOT(onMoveRoots()));

    onNRoots();
}


void gl2dNewton::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl2dNewton");
    {
        s_MaxIter   = settings.value("MaxIters", s_MaxIter).toInt();
        s_Tolerance = settings.value("MaxLength", s_Tolerance).toFloat();
    }
    settings.endGroup();
}


void gl2dNewton::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl2dNewton");
    {
        settings.setValue("MaxIters", s_MaxIter);
        settings.setValue("MaxLength", s_Tolerance);
    }
    settings.endGroup();
}


void gl2dNewton::initializeGL()
{
    gl2dView::initializeGL();

    QString strange, vsrc, gsrc, fsrc;
    vsrc = ":/shaders/shaders2d/fractal_VS.glsl";
    m_shadNewton.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadNewton.log().length())
    {
        strange = QString::asprintf("%s", QString("Newton vertex shader log:"+m_shadNewton.log()).toStdString().c_str());
        xfl::trace(strange);
    }

    fsrc = ":/shaders/shaders2d/newton_FS.glsl";
    m_shadNewton.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadNewton.log().length())
    {
        strange = QString::asprintf("%s", QString("Newton fragment shader log:"+m_shadNewton.log()).toStdString().c_str());
        xfl::trace(strange);
    }

    m_shadNewton.link();

    m_shadNewton.bind();
    {
        m_attrVertexPosition = m_shadNewton.attributeLocation("VertexPosition");
        m_locViewTrans = m_shadNewton.uniformLocation("ViewTrans");
        m_locViewScale = m_shadNewton.uniformLocation("ViewScale");
        m_locViewRatio = m_shadNewton.uniformLocation("ViewRatio");
        m_locNRoots    = m_shadNewton.uniformLocation("nroots");
        m_locTolerance = m_shadNewton.uniformLocation("tolerance");
        m_locIters     = m_shadNewton.uniformLocation("maxiters");
        for(int i=0; i<MAXROOTS; i++)
            m_locColor[i] = m_shadNewton.uniformLocation(QString::asprintf("color%d",i));
        for(int i=0; i<MAXROOTS; i++)
            m_locRoot[i]  = m_shadNewton.uniformLocation(QString::asprintf("root%d",i));
    }
    m_shadNewton.release();

}


void gl2dNewton::glRenderView()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    float w_ = float(width());
    float h_ = float(height());
    float ratio = w_/h_;
    double w = m_rectView.width();
    QVector2D off((-m_ptOffset.x())/width()*w, m_ptOffset.y()/width()*w);

    m_shadNewton.bind();
    {
        int stride = 2;
        s_MaxIter   = m_pieMaxIter->value();
        s_Tolerance = m_pfeTolerance->value();
        m_shadNewton.setUniformValue(m_locIters,     s_MaxIter);
        m_shadNewton.setUniformValue(m_locTolerance, s_Tolerance);
        m_shadNewton.setUniformValue(m_locViewRatio, ratio);

        int nroots = m_prb3roots->isChecked() ? 3 : 5;
        m_shadNewton.setUniformValue(m_locNRoots, nroots);
        for(int i=0; i<nroots; i++)
        {
            m_shadNewton.setUniformValue(m_locColor[i], s_Colors[i]);
            m_shadNewton.setUniformValue(m_locRoot[i], m_Root[i]);
        }

        m_shadNewton.setUniformValue(m_locViewTrans, off);
        m_shadNewton.setUniformValue(m_locViewScale, m_fScale);

        m_vboQuad.bind();
        {
            m_shadNewton.enableAttributeArray(m_attrVertexPosition);
            m_shadNewton.setAttributeBuffer(m_attrVertexPosition, GL_FLOAT, 0, 2, stride*sizeof(GLfloat));

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_CULL_FACE);

            int nvtx = m_vboQuad.size()/stride/int(sizeof(float));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, nvtx);

            m_shadNewton.disableAttributeArray(m_attrVertexPosition);
        }
        m_vboQuad.release();
    }
    m_shadNewton.release();

    if(m_pchShowRoots->isChecked())
    {

        if(m_bResetRoots)
        {
            int nroots = m_prb3roots->isChecked() ? 3 : 5;
            int buffersize = nroots*4;
            QVector<float> pts(buffersize);
            int iv = 0;
            for(int is=0; is<nroots; is++)
            {
                pts[iv++] = m_Root[is].x();
                pts[iv++] = m_Root[is].y();
                pts[iv++] = 1.0f;
                if (is==m_iSelectedRoot || is==m_iHoveredRoot)
                    pts[iv++] = 1.0f;
                else
                    pts[iv++] = -1.0f; // invalid state, use uniform
            }

            if(m_vboRoots.isCreated()) m_vboRoots.destroy();
            m_vboRoots.create();
            m_vboRoots.bind();
            m_vboRoots.allocate(pts.data(), buffersize * int(sizeof(GLfloat)));
            m_vboRoots.release();

            m_bResetRoots = false;
        }

        QMatrix4x4 m_matModel;
        QMatrix4x4 m_matView;
        QMatrix4x4 m_matProj;
        float s = 1.0;
        int width  = geometry().width();
        int height = geometry().height();
        m_matProj.ortho(-s,s,-(height*s)/width,(height*s)/width,-1.0e3*s,1.0e3*s);

        m_matView.scale(m_fScale, m_fScale, m_fScale);
        m_matView.translate(-off.x(), -off.y(), 0.0f);

        QMatrix4x4 vmMat(m_matView*m_matModel);
        QMatrix4x4 pvmMat(m_matProj*vmMat);


        m_shadPoint.bind();
        {
            float m_ClipPlanePos(500.0);
            m_shadPoint.setUniformValue(m_locPoint.m_ClipPlane, m_ClipPlanePos);
            m_shadPoint.setUniformValue(m_locPoint.m_Viewport, QVector2D(float(m_GLViewRect.width()), float(m_GLViewRect.height())));
    //        m_shadPoint.setUniformValue(m_locPoint.m_Viewport, QVector2D(float(m_rectView.width()), float(m_rectView.height())));
            m_shadPoint.setUniformValue(m_locPoint.m_vmMatrix,  vmMat);
            m_shadPoint.setUniformValue(m_locPoint.m_pvmMatrix, pvmMat);
        }
        m_shadPoint.release();

        paintPoints(m_vboRoots, 1.0, 0, false, Qt::white, 4);
    }

    m_plabScale->setText(QString::asprintf("Scale = %g", m_fScale));

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready2d();
    }
}


void gl2dNewton::onNRoots()
{
    if(m_prb3roots->isChecked())
    {
        for(int i=0; i<3; i++)
        {
            m_amp0[i] = 1.0f;
            m_phi0[i] = 2.0f*PIf/3.0f * float(i);
            m_Root[i].setX(m_amp0[i]*cos(m_phi0[i]));
            m_Root[i].setY(m_amp0[i]*sin(m_phi0[i]));
        }
    }
    else if(m_prb5roots->isChecked())
    {
        for(int i=0; i<5; i++)
        {
            m_amp0[i] = 1.0f;
            m_phi0[i] = 2.0f*PIf/5.0f * float(i);
            m_Root[i].setX(m_amp0[i]*cos(m_phi0[i]));
            m_Root[i].setY(m_amp0[i]*sin(m_phi0[i]));
        }
    }

    m_Time = 0;
    m_bResetRoots = true;
    update();
}


void gl2dNewton::onAnimate(bool bAnimate)
{
    if(bAnimate)
    {
        if(!m_Timer.isActive())
        {
            m_Timer.start(17);
        }
    }
    else
        m_Timer.stop();
}


void gl2dNewton::mousePressEvent(QMouseEvent *pEvent)
{
    QVector2D pt;
    screenToWorld(pEvent->pos(), pt);

    int nroots = m_prb3roots->isChecked() ? 3 : 5;
    for(int i=0; i<nroots; i++)
    {
        if(pt.distanceToPoint(m_Root[i])<0.025/m_fScale)
        {
            m_Timer.stop();
//            m_pchAnimateRoots->setChecked(false);
            m_iSelectedRoot  = i;
            return;
        }
    }

    gl2dView::mousePressEvent(pEvent);
}


void gl2dNewton::mouseMoveEvent(QMouseEvent *pEvent)
{
    QVector2D pt;
    screenToWorld(pEvent->pos(), pt);
    int nroots = m_prb3roots->isChecked() ? 3 : 5;

    if(m_iSelectedRoot>=0 && m_iSelectedRoot<nroots)
    {
        m_Root[m_iSelectedRoot] = pt;
        for(int i=0; i<nroots; i++)
        {
            m_amp0[i] = sqrt(m_Root[i].x()*m_Root[i].x()+m_Root[i].y()*m_Root[i].y());
            m_phi0[i] = atan2f(m_Root[i].y(), m_Root[i].x());
        }
        m_Time = 0;
        m_bResetRoots = true;
        update();
        return;
    }
    else
    {
        for(int i=0; i<nroots; i++)
        {
            if(pt.distanceToPoint(m_Root[i])<0.025/m_fScale)
            {
                m_iHoveredRoot = i;
                m_bResetRoots = true;
                update();
                return;
            }
        }
        m_iHoveredRoot = -1;
        m_bResetRoots = true;
        if(!m_pchAnimateRoots->isChecked()) update();
    }
    gl2dView::mouseMoveEvent(pEvent);
}


void gl2dNewton::mouseReleaseEvent(QMouseEvent *pEvent)
{
    m_iSelectedRoot = -1;
    QApplication::restoreOverrideCursor();

    if(m_iSelectedRoot>=0 || m_iHoveredRoot>=0)
    {
        m_iSelectedRoot = m_iHoveredRoot = -1;
        m_bResetRoots = true;
        m_Time = 0;
        update();

        if(m_pchAnimateRoots->isChecked()) m_Timer.start(17);
        return;
    }
    gl2dView::mouseReleaseEvent(pEvent);
}


void gl2dNewton::onMoveRoots()
{
    float t = float(m_Time)/1000.0f;

    int nroots = m_prb3roots->isChecked() ? 3 : 5;
    for(int i=0; i<nroots; i++)
    {
        float amp = m_amp0[i] - (1.0f - cosf(m_omega[2*i]*t*6.0*PIf))/2.0f;
        float phi = m_phi0[i] + 2.0f*PIf*sinf(m_omega[2*i+1]*t);

        m_Root[i].setX(amp*cos(phi));
        m_Root[i].setY(amp*sin(phi));
    }

    m_Time++;

    m_bResetRoots = true;
    update();
}


void gl2dNewton::onSaveImage()
{
    QString filename = "Newton fractal.png";
    QString description = QString::asprintf("Made with flow5");
    saveImage(filename, description);
}






