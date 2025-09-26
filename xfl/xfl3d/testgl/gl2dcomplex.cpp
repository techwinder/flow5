/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#include <QApplication>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

#include "gl2dcomplex.h"

#include <xflcore/xflcore.h>
#include <xflcore/displayoptions.h>
#include <xflcore/trace.h>
#include <xfl3d/views/gl3dview.h> // for the static variables
#include <xflmath/mathelem.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>
#include <xflwidgets/wt_globals.h>


QByteArray gl2dComplex::s_Geometry;

gl2dComplex::gl2dComplex(QWidget *pParent) : gl2dView(pParent)
{
    setWindowTitle("Complex analysis");
    setMouseTracking(false);

    m_fScale = 0.5f;

    m_bAxes = true;

    m_nRoots = 1;

    m_pCmdFrame = new QFrame(this);
    {
        m_pCmdFrame->setCursor(Qt::ArrowCursor);

        m_pCmdFrame->setFrameShape(QFrame::NoFrame);
        m_pCmdFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QVBoxLayout *pFrameLayout = new QVBoxLayout;
        {
            QLabel *plabTitle = new QLabel("<p>Using OpenGL's fragment shader to compute<br>"
                                           "and plot complex functions</p>");

            m_plabScale = new QLabel();
            m_plabScale->setFont(DisplayOptions::textFont());
            m_plabScale->setWordWrap(true);
            m_plabScale->setMinimumHeight(DisplayOptions::tableFontStruct().height()*3);

            m_ppto = new PlainTextOutput;

            pFrameLayout->addWidget(plabTitle);
            pFrameLayout->addWidget(m_plabScale);
            pFrameLayout->addWidget(m_ppto);
        }

        m_pCmdFrame->setLayout(pFrameLayout);

        m_pCmdFrame->setStyleSheet("QFrame{background-color: transparent; color: white}"
                                   "QRadioButton{background-color: transparent; color: white}"
                                   "QCheckBox{background-color: transparent; color: white}");
    }
}


void gl2dComplex::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl2dComplex");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();
    }
    settings.endGroup();
}


void gl2dComplex::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl2dComplex");
    {
        settings.setValue("WindowGeometry", s_Geometry);
    }
    settings.endGroup();
}


void gl2dComplex::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void gl2dComplex::hideEvent(QHideEvent *pEvent)
{
    QWidget::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void gl2dComplex::initializeGL()
{
    QString strange, vsrc, fsrc;
    vsrc = ":/shaders/shaders2d/fractal_VS.glsl";
    m_shadComplex.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadComplex.log().length())
    {
        strange = QString::asprintf("%s", QString("Frac. vertex shader log:"+m_shadComplex.log()).toStdString().c_str());
        trace(strange);
    }

    fsrc = ":/shaders/shaders2d/complex_FS.glsl";
    m_shadComplex.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadComplex.log().length())
    {
        strange = QString::asprintf("%s", QString("Complex fragment shader log:"+m_shadComplex.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadComplex.link();
    m_shadComplex.bind();
    {
        m_locViewTrans  = m_shadComplex.uniformLocation("ViewTrans");
        m_locViewScale  = m_shadComplex.uniformLocation("ViewScale");
        m_locViewRatio  = m_shadComplex.uniformLocation("ViewRatio");
        m_attrVertexPosition = m_shadComplex.attributeLocation("VertexPosition");
    }
    m_shadComplex.release();

    gl2dView::initializeGL();
}


void gl2dComplex::glRenderView()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    double w = m_rectView.width();
    QVector2D off(-m_ptOffset.x()/width()*w, m_ptOffset.y()/width()*w);

    if(m_shadComplex.bind())
    {
        int stride = 2;

        m_shadComplex.setUniformValue(m_locViewRatio, float(width())/float(height()));
        m_shadComplex.setUniformValue(m_locViewTrans,  off);
        m_shadComplex.setUniformValue(m_locViewScale,  m_fScale);

        m_vboQuad.bind();
        {
            m_shadComplex.enableAttributeArray(m_attrVertexPosition);
            m_shadComplex.setAttributeBuffer(m_attrVertexPosition, GL_FLOAT, 0, stride, stride*sizeof(GLfloat));

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_CULL_FACE);

            int nvtx = m_vboQuad.size()/stride/int(sizeof(float));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, nvtx);

            m_shadComplex.disableAttributeArray(m_attrVertexPosition);
        }
        m_vboQuad.release();
        m_shadComplex.release();
    }


    QString strange = QString::asprintf("Scale = %g\n", m_fScale);
    m_plabScale->setText(strange);

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready2d();
    }
}




void gl2dComplex::onSaveImage()
{
    QString filename = "Complex.png";
    QString description = QString::asprintf("Made with flow5");
    saveImage(filename, description);
}
