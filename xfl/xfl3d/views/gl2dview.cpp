/****************************************************************************

    flow5
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#define _MATH_DEFINES_DEFINED

#include <QApplication>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMatrix4x4>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLPaintDevice>
#include <QStandardPaths>

#include "gl2dview.h"

#include <xfl3d/globals/gl_globals.h>
#include <xfl/controls/w3dprefs.h>
#include <xflcore/displayoptions.h>
#include <xflcore/trace.h>
#include <xflcore/xflcore.h>
#include <xflmath/constants.h>

#include <xflwidgets/wt_globals.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/view/grid.h>


QSize gl2dView::s_ImageSize(1920, 1080);


gl2dView::gl2dView(QWidget *pParent) : QOpenGLWidget(pParent)
{
    setFocusPolicy(Qt::WheelFocus);
    setCursor(Qt::CrossCursor);
//    setMouseTracking(true);


    m_locViewTrans = -1;
    m_locViewScale = -1;
    m_locViewRatio = -1;
    m_attrVertexPosition = -1;

    m_rectView = QRectF(-1.0, -1.0, 2.0, 2.0);

    m_bAxes = true;

    m_pCmdFrame = nullptr;

    m_ppbSaveImg = new QPushButton("Save 2d image");
    connect(m_ppbSaveImg, &QPushButton::clicked, this, &gl2dView::onSaveImage);

    m_pieWidth = new IntEdit(s_ImageSize.width());
//    m_pieWidth->setToolTip("<p>The target image's height will be adjusted to maintain the aspect ratio of this window.</p>");
    m_pieHeight = new IntEdit(s_ImageSize.height());

    m_plabInfoOutput = new QLabel(this);
    m_plabInfoOutput->setFont(DisplayOptions::tableFont());
    m_plabInfoOutput->setTextFormat(Qt::PlainText);
    m_plabInfoOutput->setAttribute(Qt::WA_NoSystemBackground);

    m_bDynTranslation = false;
    m_bDynScaling     = false;
    connect(&m_DynTimer, SIGNAL(timeout()), SLOT(onDynamicIncrement()));

    m_nRoots = 0;
    m_uHasShadow = m_uShadowLightViewMatrix = -1;

    m_PixOverlay = QPixmap(107, 97);
    m_PixOverlay.fill(Qt::transparent);

    m_fScale = 1.0f;
    m_RefLength = -1.0;

    ANIMATIONFRAMES = 30;
    m_iTimerInc = 0;
    m_glScaleIncrement = 0.0;
}


gl2dView::~gl2dView()
{    
}


void gl2dView::setOutputInfo(QString const &info)
{
    m_plabInfoOutput->setText(info);
    resizeLabels();
}


void gl2dView::resizeLabels()
{
    int w = rect().width();
    m_plabInfoOutput->adjustSize();
    QPoint pos1(w-m_plabInfoOutput->width()-5, 5);
    m_plabInfoOutput->move(pos1);
}


void gl2dView::onTranslationIncrement()
{
    if(m_iTimerInc>=ANIMATIONFRAMES)
    {
        m_TransitionTimer.stop();
        m_iTimerInc = 0;
        return;
    }

    m_ptOffset += m_TransIncrement;
    update();
    m_iTimerInc++;
}


void gl2dView::on2dReset()
{
    stopDynamicTimer();
    startResetTimer();

}


void gl2dView::onResetIncrement()
{
    if(m_iTimerInc>=ANIMATIONFRAMES)
    {
        m_TransitionTimer.stop();
        m_iTimerInc = 0;
        return;
    }

    m_fScale += m_glScaleIncrement;
    m_ptOffset += m_TransIncrement;
    update();
    m_iTimerInc++;
}


void gl2dView::onDynamicIncrement()
{
    if(m_bDynTranslation)
    {
        double dist = sqrt(m_Trans.x()*m_Trans.x()+m_Trans.y()*m_Trans.y())*m_fScale;
        if(dist<0.01)
        {
            stopDynamicTimer();
            update();
            return;
        }
        m_ptOffset += m_Trans/10.0;

        m_Trans *= (1.0-W3dPrefs::spinDamping());
    }

    if(m_bDynScaling)
    {
        if(abs(m_ZoomFactor)<10)
        {
            stopDynamicTimer();
            update();
            return;
        }

        double scalefactor(1.0-DisplayOptions::scaleFactor()/3.0 * m_ZoomFactor/120);

        m_fScale *= scalefactor;
        m_ZoomFactor *= (1.0-W3dPrefs::spinDamping());
    }

    setAutoUnits();

    update();
}


void gl2dView::startResetTimer()
{
    m_iTimerInc = 0;

    // calculate the number of animation frames for 60Hz refresh rate
    int period = 17; //60 Hz in ms

    ANIMATIONFRAMES = int(double(W3dPrefs::transitionTime())/double(period));

    m_glScaleIncrement = (referenceScale()-m_fScale)/ANIMATIONFRAMES;
    m_TransIncrement = (-m_ptOffset)/ANIMATIONFRAMES;

    disconnect(&m_TransitionTimer, nullptr, nullptr, nullptr);
    connect(&m_TransitionTimer, SIGNAL(timeout()), SLOT(onResetIncrement()));
    m_TransitionTimer.start(7);//7 ms x 50 times
}



void gl2dView::startDynamicTimer()
{
    m_DynTimer.start(17);
    setMouseTracking(false);
}


void gl2dView::startTranslationTimer(QPointF const &PP)
{
    int period = 17; //60 Hz in ms

    ANIMATIONFRAMES = int(double(W3dPrefs::transitionTime())/double(period));

    double inc = double(ANIMATIONFRAMES);

    QPointF screenCenter(float(width())/2.0f, float(height())/2.0f);
    m_TransIncrement.setX(( -PP.x() +screenCenter.x())/inc/m_fScale);
    m_TransIncrement.setY(( -PP.y() +screenCenter.y())/inc/m_fScale);

    m_iTimerInc = 0;

    disconnect(&m_TransitionTimer, nullptr, nullptr, nullptr);
    connect(&m_TransitionTimer, SIGNAL(timeout()), SLOT(onTranslationIncrement()));
    m_TransitionTimer.start(period);
}


void gl2dView::stopDynamicTimer()
{
    if(m_DynTimer.isActive())
    {
        m_TransitionTimer.stop();
        m_DynTimer.stop();
    }
    m_bDynTranslation  = m_bDynScaling = false;
    setMouseTracking(true);
}



void gl2dView::keyPressEvent(QKeyEvent *pEvent)
{
//    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    bool bAlt  = (pEvent->modifiers() & Qt::AltModifier);
    switch (pEvent->key())
    {
        case Qt::Key_R:
        {
            on2dReset();
            break;
        }
        case Qt::Key_I:
        {
            if(bAlt)
                onSaveImage();
            break;
        }
    }

    QOpenGLWidget::keyPressEvent(pEvent);
}


void gl2dView::reset2dScale()
{
    stopDynamicTimer();
    m_fScale = referenceScale();
    m_ptOffset = defaultOffset();
    update();
}


void gl2dView::showEvent(QShowEvent *pEvent)
{
    setFocus();
    resizeLabels();
    setAutoUnits();
    QOpenGLWidget::showEvent(pEvent);
}


void gl2dView::leaveEvent(QEvent *pEvent)
{
    clearOutputInfo();
    update();
    QOpenGLWidget::leaveEvent(pEvent);
}


void gl2dView::paintGL()
{
//    glClearColor(float(DisplayOptions::backgroundColor().redF()), float(DisplayOptions::backgroundColor().greenF()), float(DisplayOptions::backgroundColor().blueF()), 1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_MULTISAMPLE);

    glMake2dObjects();

    double w = m_rectView.width();
    QVector2D off(-m_ptOffset.x()/width()*w, m_ptOffset.y()/width()*w);

    float s = 1.0;
    int width  = geometry().width();
    int height = geometry().height();
    m_matModel.setToIdentity();
    m_matView.setToIdentity();
    m_matProj.setToIdentity();

    m_matProj.ortho(-s,s,-(height*s)/width,(height*s)/width,-1.0e3*s,1.0e3*s);

    m_matView.scale(m_fScale, m_fScale, m_fScale);
    m_matView.translate(-off.x(), -off.y(), 0.0f);

    glRenderView();

    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    paintOverlay();
}


void gl2dView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if(W3dPrefs::bSpinAnimation())
    {
        int movetime = m_MoveTime.elapsed();
        if(movetime<300 && !m_PressedPoint.isNull())
        {
            if(pEvent->button()==Qt::LeftButton)
            {
                m_Trans = (pEvent->pos() - m_PressedPoint)/m_fScale;
                startDynamicTimer();
                m_bDynTranslation = true;
            }
        }
    }
    QApplication::restoreOverrideCursor();
}


void gl2dView::mousePressEvent(QMouseEvent *pEvent)
{
    if(m_iTimerInc>0)
    {
        // interrupt animation
        m_TransitionTimer.stop();
        m_iTimerInc = 0;
    }

    m_LastPoint = pEvent->pos();
    m_PressedPoint = m_LastPoint;

    stopDynamicTimer();
    m_MoveTime.restart();
    if(hasFocus()) QApplication::setOverrideCursor(Qt::ClosedHandCursor);
}


void gl2dView::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(!hasFocus()) setFocus();

    QPoint point = pEvent->pos();

    if(pEvent->buttons() & Qt::LeftButton)
    {
        //translate the view

        QPoint delta = point - m_LastPoint;
        m_ptOffset.setX(m_ptOffset.x() + double(delta.x())/m_fScale);
        m_ptOffset.setY(m_ptOffset.y() + double(delta.y())/m_fScale);

        m_LastPoint=point;

        update();
        return;
    }
    else if(pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        float zoomFactor=1.0f;

        if(point.y()-m_LastPoint.y()<0)
        {
            zoomFactor = 1.0f/1.025f;
        }
        else
        {
            zoomFactor = 1.025f;
        }
        m_fScale *= zoomFactor;
        m_LastPoint=point;
        update();
        return;
    }

    pEvent->accept();
}


void gl2dView::mouseDoubleClickEvent(QMouseEvent *pEvent)
{
    QVector2D pt;
    screenToWorld(pEvent->pos(), pt);
    startTranslationTimer(pEvent->pos());
}


void gl2dView::paintDebugPts()
{
#ifdef QT_DEBUG
    for(int i=0; i<m_DebugPts.size(); i++)
         paintDisk(m_DebugPts.at(i), 0.1f, Qt::darkRed);

/*    for(int i=0; i<m_DebugVecs.size(); i++)
    {
        if(i<m_DebugPts.size())            paintThinArrow(m_DebugPts.at(i), m_DebugVecs.at(i), QColor(205,135,155).darker(), 2, Line::SOLID, QMatrix4x4());
    }*/
#endif
}


void gl2dView::screenToViewport(QPoint const &point, QVector2D &real) const
{
    double h2 = double(geometry().height()) /2.0;
    double w2 = double(geometry().width())  /2.0;

    real.setX( (double(point.x()) - w2) / w2);
    real.setY(-(double(point.y()) - h2) / w2);
}


QVector2D gl2dView::screenToViewport(QPoint const &point) const
{
    double h2 = double(geometry().height()) /2.0;
    double w2 = double(geometry().width())  /2.0;

    QVector2D real;
    real.setX( (double(point.x()) - w2) / w2);
    real.setY(-(double(point.y()) - h2) / w2);
    return real;
}


Vector2d gl2dView::screenToWorld(QPoint const &screenpt) const
{
    QVector2D Pt2D;
    screenToWorld(screenpt, Pt2D);
    Vector2d pt(Pt2D.x(), Pt2D.y());
    return pt;
}


void gl2dView::screenToWorld(QPoint const &screenpt, Vector2d &pt) const
{
    QVector2D Pt2D;
    screenToWorld(screenpt, Pt2D);
    pt.x = Pt2D.x();
    pt.y = Pt2D.y();
}


void gl2dView::screenToWorld(QPoint const &screenpt, QVector2D &pt)  const
{
    QMatrix4x4 m_matView, m;
    QVector4D in, out;
    QVector2D real;
    double w = m_rectView.width();
    QVector2D off((-m_ptOffset.x())/width()*w, m_ptOffset.y()/width()*w);

    screenToViewport(screenpt, real);
    in.setX(float(real.x()));
    in.setY(float(real.y()));
    in.setZ(0.0f);
    in.setW(1.0f);

    m_matView.scale(m_fScale, m_fScale, m_fScale);
    m_matView.translate(-off.x(), -off.y(), 0.0f);

    bool bInverted=false;
    QMatrix4x4 vmMatrix = m_matView;
    m = vmMatrix.inverted(&bInverted);
    out = m * in;

    if(fabs(double(out[3]))>PRECISION)
    {
        pt.setX(double(out[0]/out[3]));
        pt.setY(double(out[1]/out[3]));
    }
    else
    {
        pt.setX(double(out[0]));
        pt.setY(double(out[0]));
    }
}


QPoint gl2dView::worldToScreen(Vector2d const&v) const
{
    return worldToScreen(v.xf(), v.yf());
}


QPoint gl2dView::worldToScreen(float xf, float yf) const
{
    QVector4D v4(xf, yf, 0.0f, 1.0f);
    QVector4D vScreen = m_matProj*m_matView*m_matModel * v4;
    return QPoint(int((vScreen.x()+1.0f)*width()/2), int((1.0f-vScreen.y())*height()/2));
}


void gl2dView::paintPoints(QOpenGLBuffer &vbo, float width, int iShape, bool bLight, QColor const &clr, int stride)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_shadPoint.bind();
    {
        // iShape 0: Pentagon, 1: Icosahedron, 2: Cube
        m_shadPoint.setUniformValue(m_locPoint.m_Shape, iShape);
        m_shadPoint.setUniformValue(m_locPoint.m_UniColor, clr); // only used if vertex state is invalid 0< or >1
        m_shadPoint.setUniformValue(m_locPoint.m_Thickness, width);
        if(bLight)m_shadPoint.setUniformValue(m_locPoint.m_Light, 1);
        else      m_shadPoint.setUniformValue(m_locPoint.m_Light, 0);

        if(vbo.bind())
        {
            m_shadPoint.enableAttributeArray(m_locPoint.m_attrVertex);
            m_shadPoint.setAttributeBuffer(m_locPoint.m_attrVertex, GL_FLOAT, 0, 3, stride*sizeof(float));
            m_shadPoint.enableAttributeArray(m_locPoint.m_State);
            m_shadPoint.setAttributeBuffer(m_locPoint.m_State, GL_FLOAT, 3*sizeof(float), 1, stride*sizeof(float));
            int npts = vbo.size()/stride/int(sizeof(float));
            glDrawArrays(GL_POINTS, 0, npts);// 4 vertices defined but only 3 are used
            m_shadPoint.disableAttributeArray(m_locPoint.m_attrVertex);
            m_shadPoint.disableAttributeArray(m_locPoint.m_State);
        }
        vbo.release();
    }
    m_shadPoint.release();
}


void gl2dView::paintPoints2(QOpenGLBuffer &vbo, float w)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_shadPoint2.bind();
    {
        int stride = 8;

        m_shadPoint2.setUniformValue(m_locPt2.m_Shape, w);
        m_shadPoint2.setUniformValue(m_locPt2.m_ClipPlane, 500.0f);

        vbo.bind();
        {
            m_shadPoint2.enableAttributeArray(m_locPt2.m_attrVertex);
            m_shadPoint2.enableAttributeArray(m_locPt2.m_attrColor);

            m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrVertex, GL_FLOAT, 0,                  4, stride * sizeof(GLfloat));
            m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrColor,  GL_FLOAT, 4* sizeof(GLfloat), 4, stride * sizeof(GLfloat));

//               glPointSize(5.0f);

            glEnable(GL_POINT_SPRITE);
            glEnable(GL_PROGRAM_POINT_SIZE);


            int npts = vbo.size()/stride/sizeof(GLfloat);

            glDrawArrays(GL_POINTS, 0, npts);

            m_shadPoint2.disableAttributeArray(m_locPt2.m_attrVertex);
            m_shadPoint2.disableAttributeArray(m_locPt2.m_attrColor);
        }
        vbo.release();
    }
    m_shadPoint2.release();
}


void gl2dView::paintSegments(QOpenGLBuffer &vbo, LineStyle const &ls, bool bHigh)
{
    paintSegments(vbo, ls.m_Color, ls.m_Width, ls.m_Stipple, bHigh);
}


void gl2dView::paintSegments(QOpenGLBuffer &vbo, QColor const &clr, float thickness, Line::enumLineStipple stip, bool bHigh)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    int stride = 3; // 3 position components
    m_shadLine.bind();
    {
        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, stride*sizeof(GLfloat));

            int nSegs = vbo.size()/2/stride/int(sizeof(float));

            if(bHigh)
            {
                m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(Line::SOLID));
                m_shadLine.setUniformValue(m_locLine.m_UniColor, Qt::red);

                m_shadLine.setUniformValue(m_locLine.m_Thickness, thickness+2);
            }
            else
            {
                m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(stip));
                m_shadLine.setUniformValue(m_locLine.m_UniColor, clr);
                m_shadLine.setUniformValue(m_locLine.m_Thickness, thickness);
            }

            glDrawArrays(GL_LINES, 0, nSegs*2);// 4 vertices defined but only 3 are used
            glDisable(GL_LINE_STIPPLE);
        }
        vbo.release();

        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}


void gl2dView::paintLineStrip(QOpenGLBuffer &vbo, LineStyle const &ls)
{
    paintLineStrip(vbo, ls.m_Color, ls.m_Width, ls.m_Stipple);
}


void gl2dView::paintLineStrip(QOpenGLBuffer &vbo, QColor const &clr, float width, Line::enumLineStipple stipple)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_UniColor, clr);
        m_shadLine.setUniformValue(m_locLine.m_Thickness, width);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(stipple));

        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
            int nPoints = vbo.size()/3/int(sizeof(float));
            glDrawArrays(GL_LINE_STRIP, 0, nPoints);
            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        }
        vbo.release();
    }
    m_shadLine.release();
}


void gl2dView::paintColourMap(QOpenGLBuffer &vbo)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadSurf.bind();
    {
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrColor);

        vbo.bind();
        {
            // 3 vertices x(3 coords + 3clr components)
            int nTriangles = vbo.size()/3/6/int(sizeof(float));

            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                  3, 6 * sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrColor,  GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

            m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 0);
            m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

            glDisable(GL_CULL_FACE); // most colour maps can be viewed from both sides
            glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);

            glDisable(GL_POLYGON_OFFSET_FILL);

            m_shadSurf.disableAttributeArray(m_locSurf.m_attrColor);
            m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        }
        vbo.release();
    }
    m_shadSurf.release();

    glEnable(GL_CULL_FACE);
}


void gl2dView::paintDisk(Vector2d const &pos, float rad, const QColor &clr)
{
    paintDisk(pos.xf(), pos.yf(), rad, clr);
}


void gl2dView::paintDisk(float xf, float yf, float rad, const QColor &clr)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    int stride = 6;
    m_shadSurf.bind();
    {
        QMatrix4x4 matmodel;
        matmodel.translate(xf, yf, 0.0f);
        matmodel.scale(rad);
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  m_matView*matmodel);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView*matmodel);

        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, clr);
        m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);


        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0);
        glEnable(GL_CULL_FACE);

        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);

        m_vboDisk.bind();
        {
            int nTriangles = m_vboDisk.size()/3/stride/int(sizeof(float)); // three vertices and (3 position components+3 normal components)

            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                 3, stride*sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3*sizeof(GLfloat), 3, stride*sizeof(GLfloat));
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

            glDrawArrays(GL_TRIANGLES, 0, nTriangles*3); // 4 vertices defined but only 3 are used
        }
        m_vboDisk.release();
        glDisable(GL_POLYGON_OFFSET_FILL);

        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0); // leave things as they were
        glEnable(GL_CULL_FACE);
    }
    m_shadSurf.release();
}


void gl2dView::paintTriangles3Vtx(QOpenGLBuffer &vbo, const QColor &backclr)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int stride = 6;
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, backclr);
        m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);


        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0);
        glEnable(GL_CULL_FACE);

        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);

        vbo.bind();
        {
            int nTriangles = vbo.size()/3/stride/int(sizeof(float)); // three vertices and (3 position components+3 normal components)

            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                 3, stride*sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3*sizeof(GLfloat), 3, stride*sizeof(GLfloat));
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

            glDrawArrays(GL_TRIANGLES, 0, nTriangles*3); // 4 vertices defined but only 3 are used
        }
        vbo.release();
        glDisable(GL_POLYGON_OFFSET_FILL);

        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0); // leave things as they were
        glEnable(GL_CULL_FACE);
    }
    m_shadSurf.release();
}


void gl2dView::glRenderText(float x, float y, QString const& str, const QColor &textcolor, bool bBackground, bool bBold)
{
    QPoint point;
    point = worldToScreen(x, y);
    point *= devicePixelRatio();

    QPainter painter(&m_PixOverlay);
    painter.save();

    painter.setPen(QPen(textcolor));
    QFont font(DisplayOptions::textFont());
    font.setBold(bBold);
    font.setPointSize(DisplayOptions::textFont().pointSize()*devicePixelRatio());
    painter.setFont(font);
    if(bBackground)
    {
        QBrush backbrush(DisplayOptions::backgroundColor());
//        paint.setBrush(backbrush);
        painter.setBackground(backbrush);
        painter.setBackgroundMode(Qt::OpaqueMode);
    }
    painter.drawText(point, str);
    painter.restore();
}


void gl2dView::glRenderText(int x, int y, const QString & str, QColor const &backclr, QColor const &textcolor, bool bBold)
{
    QPainter painter(&m_PixOverlay);
    painter.save();
    painter.setPen(QPen(textcolor));
    QFont font(painter.font());
    font.setPointSize(DisplayOptions::textFont().pointSize()*devicePixelRatio());
    font.setBold(bBold);
    painter.setFont(font);
    QBrush backbrush(backclr);
    painter.setBrush(backbrush);
    painter.drawText(x*devicePixelRatio(),y*devicePixelRatio(), str);
    painter.restore();
}




void gl2dView::paintTriangle(QOpenGLBuffer &vbo, bool bHighlight, bool bBackground, QColor const &clrBack)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    if(!vbo.isCreated()) return;
    vbo.bind();
    {
        if(vbo.size()<=0)
        {
            vbo.release();
            return;
        }
    }

    m_shadLine.bind();
    m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);

    m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

    if(bHighlight)
    {
        m_shadLine.setUniformValue(m_locLine.m_Thickness, float(W3dPrefs::highlightWidth()));

        m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::highlightColor());
    }
    else
    {
        m_shadLine.setUniformValue(m_locLine.m_Thickness, float(W3dPrefs::s_PanelStyle.m_Width));

        m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_OutlineStyle.m_Color);
    }
    glDrawArrays(GL_LINE_STRIP, 0, 4);

    m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    m_shadLine.release();

    if(bBackground)
    {
        m_shadSurf.bind();
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0, 3, 3*sizeof(GLfloat));

        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, clrBack);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);
        glDrawArrays(GL_TRIANGLES, 0, 3); // nodes 0,1,2
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    vbo.release();
}



void gl2dView::resizeGL(int width, int height)
{
    QOpenGLWidget::resizeGL(width, height);

//    int side = std::min(width, height);
//    glViewport((width - side) / 2, (height - side) / 2, side, side);

    double w = double(width);
    double h = double(height);
    double s = 1.0;

    if(w>h) m_GLViewRect.setRect(-s, s*h/w, s, -s*h/w);
    else    m_GLViewRect.setRect(-s*w/h, s, s*w/h, -s);

    m_ptOffset = defaultOffset();

    if(!m_PixOverlay.isNull())
    {
        m_PixOverlay = m_PixOverlay.scaled(rect().size()*devicePixelRatio());
        m_PixOverlay.fill(Qt::transparent);
    }

    resizeLabels();
    setAutoUnits();

}


void gl2dView::wheelEvent(QWheelEvent *pEvent)
{
    int dy = pEvent->pixelDelta().y();
    if(dy==0) dy = pEvent->angleDelta().y(); // pixeldelta usable on macOS and angleDelta on win/linux; depends also on driver and hardware

    if(W3dPrefs::bSpinAnimation() && abs(dy)>120)
    {
        m_bDynScaling = true;
        m_ZoomFactor = dy;

        startDynamicTimer();
    }
    else
    {
        float zoomfactor(1.0f);
        if(pEvent->angleDelta().y()>0) zoomfactor = 1.0/(1.0+DisplayOptions::scaleFactor());
        else                           zoomfactor = 1.0+DisplayOptions::scaleFactor();

        m_fScale *= zoomfactor;

        int a = rect().center().x();
        int b = rect().center().y();
        m_ptOffset.rx() = a + (m_ptOffset.x()-a);
        m_ptOffset.ry() = b + (m_ptOffset.y()-b);
        setAutoUnits();
        update();
    }

    pEvent->accept();
}


void gl2dView::initializeGL()
{
    QString strange, vsrc, gsrc, fsrc;

    //--------- setup the shader to paint stippled thick lines -----------
    vsrc = ":/shaders/line/line_VS.glsl";
    m_shadLine.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadLine.log().length())
    {
        strange = QString::asprintf("%s", QString("Line vertex shader log:"+m_shadLine.log()).toStdString().c_str());
        trace(strange);
    }

    gsrc = ":/shaders/line/line_GS.glsl";
    m_shadLine.addShaderFromSourceFile(QOpenGLShader::Geometry, gsrc);
    if(m_shadLine.log().length())
    {
        strange = QString::asprintf("%s", QString("Line geometry shader log:"+m_shadLine.log()).toStdString().c_str());
        trace(strange);
    }

    fsrc = ":/shaders/line/line_FS.glsl";
    m_shadLine.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadLine.log().length())
    {
        strange = QString::asprintf("%s", QString("Stipple fragment shader log:"+m_shadLine.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadLine.link();
    m_shadLine.bind();
    {
        m_locLine.m_attrVertex    = m_shadLine.attributeLocation("vertexPosition_modelSpace");
        m_locLine.m_attrColor = m_shadLine.attributeLocation("vertexColor");
        m_locLine.m_vmMatrix     = m_shadLine.uniformLocation("vmMatrix");
        m_locLine.m_pvmMatrix    = m_shadLine.uniformLocation("pvmMatrix");
        m_locLine.m_HasUniColor  = m_shadLine.uniformLocation("HasUniColor");
        m_locLine.m_UniColor     = m_shadLine.uniformLocation("UniformColor");
        m_locLine.m_ClipPlane    = m_shadLine.uniformLocation("clipPlane0");
        m_locLine.m_Thickness    = m_shadLine.uniformLocation("Thickness");
        m_locLine.m_Viewport     = m_shadLine.uniformLocation("Viewport");
        m_locLine.m_Pattern      = m_shadLine.uniformLocation("pattern");
        m_locLine.m_nPatterns    = m_shadLine.uniformLocation("nPatterns");
        GLint nPatterns = 300; // number of patterns per unit projected length (viewport half width = 1)
        m_shadLine.setUniformValue(m_locLine.m_nPatterns, nPatterns);
    }
    m_shadLine.release();

    vsrc = ":/shaders/point/point_VS.glsl";
    m_shadPoint.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadPoint.log().length())
    {
        strange = QString::asprintf("%s", QString("Point vertex shader log:"+m_shadPoint.log()).toStdString().c_str());
        trace(strange);
    }

    gsrc = ":/shaders/point/point_GS.glsl";
    m_shadPoint.addShaderFromSourceFile(QOpenGLShader::Geometry, gsrc);
    if(m_shadPoint.log().length())
    {
        strange = QString::asprintf("%s", QString("Point geometry shader log:"+m_shadPoint.log()).toStdString().c_str());
        trace(strange);
    }

    fsrc = ":/shaders/point/point_FS.glsl";
    m_shadPoint.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadPoint.log().length())
    {
        strange = QString::asprintf("%s", QString("Point fragment shader log:"+m_shadPoint.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadPoint.link();
    m_shadPoint.bind();
    {
        m_locPoint.m_attrVertex = m_shadPoint.attributeLocation("vertexPosition_modelSpace");
        m_locPoint.m_State      = m_shadPoint.attributeLocation("PointState");
        m_locPoint.m_vmMatrix   = m_shadPoint.uniformLocation("vmMatrix");
        m_locPoint.m_pvmMatrix  = m_shadPoint.uniformLocation("pvmMatrix");
        m_locPoint.m_ClipPlane  = m_shadPoint.uniformLocation("clipPlane0");
        m_locPoint.m_UniColor   = m_shadPoint.uniformLocation("Color");
        m_locPoint.m_Thickness  = m_shadPoint.uniformLocation("Thickness");
        m_locPoint.m_Shape      = m_shadPoint.uniformLocation("Shape");
        m_locPoint.m_Viewport   = m_shadPoint.uniformLocation("Viewport");
        m_locPoint.m_Light      = m_shadPoint.uniformLocation("LightOn");
        m_locPoint.m_TwoSided   = m_shadPoint.uniformLocation("TwoSided");
    }
    m_shadPoint.release();


    // setup the flat point shader
    vsrc =  ":/shaders/point2/point2_VS.glsl";
    fsrc =  ":/shaders/point2/point2_FS.glsl";

    m_shadPoint2.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadPoint2.log().length())
    {
        strange = QString::asprintf("%s", QString("point2 vertex shader log:"+m_shadPoint2.log()).toStdString().c_str());
        trace(strange);
    }


    m_shadPoint2.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadPoint2.log().length())
    {
        strange = QString::asprintf("%s", QString("point2 fragment shader log:"+m_shadPoint2.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadPoint2.link();
    m_shadPoint2.bind();
    {
        m_locPt2.m_attrVertex = m_shadPoint2.attributeLocation("vertexPosition_modelSpace");
        m_locPt2.m_attrColor  = m_shadPoint2.attributeLocation("vertexColor");
        m_locPt2.m_vmMatrix   = m_shadPoint2.uniformLocation("vmMatrix");
        m_locPt2.m_pvmMatrix  = m_shadPoint2.uniformLocation("pvmMatrix");
        m_locPt2.m_ClipPlane  = m_shadPoint2.uniformLocation("clipPlane0");
        m_locPt2.m_Shape      = m_shadPoint2.uniformLocation("pointsize");
    }
    m_shadPoint2.release();


    //setup the shader to paint coloured surfaces
    vsrc = ":/shaders/surface/surface_VS.glsl";
    fsrc = ":/shaders/surface/surface_FS.glsl";
    m_shadSurf.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadSurf.log().length())
    {
        strange = QString::asprintf("%s", QString("Surface vertex shader log:"+m_shadSurf.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadSurf.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadSurf.log().length())
    {
        strange = QString::asprintf("%s", QString("Surface fragment shader log:"+m_shadSurf.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadSurf.link();
    m_shadSurf.bind();
    {
        m_locSurf.m_attrVertex = m_shadSurf.attributeLocation("vertexPosition_modelSpace");
        m_locSurf.m_attrNormal = m_shadSurf.attributeLocation("vertexNormal_modelSpace");
        m_locSurf.m_attrUV     = m_shadSurf.attributeLocation("vertexUV");
        m_locSurf.m_attrColor  = m_shadSurf.attributeLocation("vertexColor");
        m_locSurf.m_attrOffset = m_shadSurf.attributeLocation("vertexOffset");

        m_locSurf.m_ClipPlane    = m_shadSurf.uniformLocation("clipPlane0");
        m_locSurf.m_pvmMatrix    = m_shadSurf.uniformLocation("pvmMatrix");
        m_locSurf.m_vmMatrix     = m_shadSurf.uniformLocation("vmMatrix");
        m_locSurf.m_HasUniColor  = m_shadSurf.uniformLocation("HasUniColor");
        m_locSurf.m_UniColor     = m_shadSurf.uniformLocation("UniformColor");
        m_locSurf.m_Light        = m_shadSurf.uniformLocation("LightOn");
        m_locSurf.m_TwoSided     = m_shadSurf.uniformLocation("TwoSided");
        m_locSurf.m_HasTexture   = m_shadSurf.uniformLocation("HasTexture");
        m_locSurf.m_TexSampler   = m_shadSurf.uniformLocation("TheSampler");
        m_locSurf.m_IsInstanced  = m_shadSurf.uniformLocation("Instanced");
        m_locSurf.m_Scale      = m_shadSurf.uniformLocation("uScale");

        m_uHasShadow             = m_shadSurf.uniformLocation("HasShadow");
        m_uShadowLightViewMatrix = m_shadSurf.uniformLocation("LightViewMatrix");
    }
    m_shadSurf.release();

    gl::makeQuad2d(m_rectView, m_vboQuad);
    gl::makeUnitDisk(51, m_vboDisk);
}


void gl2dView::saveImage(QString const &filename, QString const &description)
{
    QString Filter;
    QStringList filters;
    filters << "Portable Network Graphics (*.png)";
    Filter = filters.at(0);

    QStringList loc = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    QString FileName;
    if(!loc.isEmpty()) FileName = loc.first() + QDir::separator() + filename;

    FileName = QFileDialog::getSaveFileName(this, "Save image",
                                            FileName,
                                            filters.at(0),
                                            &Filter);

    if(FileName.right(4)!=".png") FileName+= ".png";


    QOpenGLFramebufferObjectFormat fboFormat;
    fboFormat.setInternalTextureFormat(GL_RGB16);
    fboFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);

    int iw = m_pieWidth->value();
    int ih = m_pieHeight->value();

    // keep same viewwport ratio
    float wtratio = float(width())/float(height());
    float imgratio = float(iw)/float(ih);
    bool bAdjustWidth = wtratio>imgratio;
    int w(iw), h(ih);
    QRect srcRect;
    if(bAdjustWidth)
    {
        w = int(float(width()) * float(h)/float(height()));
        int middle = w/2;
        srcRect = QRect(middle-iw/2,0, iw, ih);
    }
    else // adjust height
    {
        h = int(float(height()) * float(w)/float(width()));
        int middle = h/2;
        srcRect = QRect(0, middle-ih/2, iw, ih);
    }
    s_ImageSize = QSize(w, h);


    QOpenGLFramebufferObject *pfboOff = new QOpenGLFramebufferObject(s_ImageSize, fboFormat);


    pfboOff->bind();
    {
        QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
        glViewport(0,0, s_ImageSize.width()*devicePixelRatio(), s_ImageSize.height()*devicePixelRatio());

        glRenderView();

        QOpenGLContext::currentContext()->functions()->glFlush();

        QImage fb = pfboOff->toImage();//.convertToFormat(QImage::Format_RGB32);

        fb = fb.copy(srcRect);

        fb.setText(QString("Description"), description);
        fb.save(FileName, "PNG");

    }
    pfboOff->release();
    delete pfboOff;
}


#define MININTERVAL  0.000000001


void gl2dView::drawXScale(QPainter &painter, float scale, QPoint const &origin)
{
    if(qIsNaN(scale) || qIsInf(scale)) return;

    QString format = xfl::isLocalized() ? "%L1" : "%1";

    double scalex = scale;
    painter.save();

    painter.setFont(DisplayOptions::textFontStruct().font());
    int dD = DisplayOptions::textFontStruct().height();
    QString strLabel = QString(format).arg(0.25,0,'f', m_Grid.nDecimals());

    int dW = int((DisplayOptions::textFontStruct().width(strLabel)*4)/16);
    int TickSize = int(dD*3/4);
    int offy = int(origin.y());
    offy = std::max(offy,0); //pixels
    offy = std::min(offy, rect().height()-(dD*7/4)-2); //pixels

//    painter.translate(origin.x(), offy);

    QPen AxisPen(m_Grid.xAxisColor());
    AxisPen.setStyle(xfl::getStyle(m_Grid.xAxisStipple()));
    AxisPen.setWidth(m_Grid.xAxisWidth());
    painter.setPen(AxisPen);

    QPen LabelPen(DisplayOptions::textColor());

    int XMin = rect().left();
    int XMax = rect().right();

    double xunit = m_Grid.xMajUnit();
    double xt = 0;

    painter.drawLine(XMin, offy, XMax, offy);

    // ticks and labels for x<0
    int iter = 0;
    int xu = int(xt*scalex + origin.x());
    while(xu>XMin && iter++<50)
    {
        xu = int(xt*scalex + origin.x());
        if(xu<XMax)
        {
            painter.setPen(AxisPen);
            painter.drawLine(xu, offy, xu, offy+TickSize);
            painter.setPen(LabelPen);
            xfl::drawLabel(painter, xu-dW, offy+(dD*7)/4, xt, Qt::AlignHCenter);
        }
        xt -= xunit;
    }

    // ticks and labels for x>0
    if(origin.x()<XMin)
    {
        // skip tickmarks left of the view rectangle
        int nt = int((double(XMin)-origin.x())/scalex/xunit);
        xt = nt * xunit;
    }
    else xt = xunit; // start at origin +1 unit
    xu = int(xt*scalex + origin.x());
    iter = 0;
    while(xu<XMax && iter++<50)
    {
        xu = int(xt*scalex + origin.x());
        if(xu>XMin)
        {
            painter.setPen(AxisPen);
            painter.drawLine(xu, offy, xu, offy+TickSize);
            painter.setPen(LabelPen);
            xfl::drawLabel(painter, xu-dW, offy+(dD*7)/4, xt, Qt::AlignHCenter);
        }
        xt += xunit;
    }

    painter.restore();
}


void gl2dView::drawYScale(QPainter &painter, float scale, const QPoint &origin)
{
    if(qIsNaN(scale) || qIsInf(scale)) return;
    double scaley = scale;;
    painter.save();
    painter.setFont(DisplayOptions::textFontStruct().font());

    int dD = DisplayOptions::textFontStruct().height();

    int height3  = int(dD/3);

    int TickSize = int(dD*3/4);
    int offx = int(origin.x());
    offx = std::max(offx, 0);
    offx = std::min(offx, rect().width()-(TickSize*12)/8-DisplayOptions::textFontStruct().width("-1.0 10-4"));

    QPen AxisPen(m_Grid.yAxisColor());
    AxisPen.setStyle(xfl::getStyle(m_Grid.yAxisStipple()));
    AxisPen.setWidth(m_Grid.yAxisWidth());
    QPen LabelPen(DisplayOptions::textColor());

    int YMin = rect().top();
    int YMax = rect().bottom();


    double yunit = m_Grid.yMajUnit(0);

    double yt = -yunit;

    painter.setPen(AxisPen);
    painter.drawLine(offx, YMin, offx, YMax);

    int iter = 0;

    while(int(yt*scaley) + origin.y()>YMin && iter<500)
    {
        int yu = int(yt*scaley + origin.y());
        if(yu<YMax)
        {
            painter.setPen(AxisPen);
            painter.drawLine(offx, yu, offx+TickSize, yu);
            yu += height3;
            painter.setPen(LabelPen);
            xfl::drawLabel(painter, offx+(TickSize*12)/8, yu, -yt, Qt::AlignLeft);
        }
        yt -= yunit;
        iter++;
    }

    yt = yunit;
    iter = 0;
    while(int(yt*scaley) + origin.y()<YMax  && iter<500)
    {
        int yu = int(yt*scaley + origin.y());
        if(yu>YMin)
        {
            painter.setPen(AxisPen);
            painter.drawLine(offx, yu, offx+TickSize, yu);
            yu += height3;

            painter.setPen(LabelPen);
            xfl::drawLabel(painter, offx+(TickSize*12)/8, yu, -yt, Qt::AlignLeft);
        }
        yt += yunit;
        iter++;
    }

    painter.restore();
}


void gl2dView::setAutoUnits()
{
    double unit=0;

    QRect r = rect();

    Vector2d tl = screenToWorld(r.topLeft());
    Vector2d br = screenToWorld(r.bottomRight());
//    QVector2D pt1 = screenToViewport(r.topLeft());

    float width =  br.x-tl.x;

    int nTicks = r.width()/DisplayOptions::textFontStruct().averageCharWidth()/10;
    if     (nTicks>=5)  nTicks = 5;
    else if(nTicks>=2)  nTicks = 2;
    else                nTicks = 1;

    unit = setAutoUnit(nTicks, width);
    m_Grid.setXMajUnit(unit);
    m_Grid.setYMajUnit(0, unit);

    m_Grid.setXMinUnit(unit/5.0);
    m_Grid.setYMinUnit(0, unit/5.0);
}


double gl2dView::setAutoUnit(int nTicks, float width) const
{
    double unit = width/double(nTicks);

    int exponent = 0;

    if (unit <1.0)  exponent = int(log10(unit *1.00001)-1);
    else            exponent = int(log10(unit *1.00001));
    int main = int(unit /pow(10.0, exponent)*1.000001);

    if(main<2)
        unit  = pow(10.0, exponent);
    else if (main<5)
        unit  = 2.0*pow(10.0, exponent);
    else
        unit  = 5.0*pow(10.0, exponent);

    return unit;
}


void gl2dView::paintOverlay()
{
    QOpenGLPaintDevice device(size() * devicePixelRatio());
    QPainter painter(&device);
    painter.save();
    if(m_bAxes)
    {
/*        QPoint point;
        point = worldToScreen(0.0f, 0.0f);
        point *= devicePixelRatio();

        QPen axespen(W3dPrefs::s_AxisStyle.m_Color);
        axespen.setWidth(W3dPrefs::s_AxisStyle.m_Width);
        axespen.setStyle(W3dPrefs::s_AxisStyle.getStipple());
        painter.setPen(axespen);

        painter.drawLine(point.x(),     point.y(), point.x()+300, point.y());
        painter.drawLine(point.x()+300, point.y(), point.x()+275, point.y()-10);
        painter.drawLine(point.x()+300, point.y(), point.x()+275, point.y()+10);

        painter.drawLine(point.x(), point.y(),     point.x(),    point.y()-300);
        painter.drawLine(point.x(), point.y()-300, point.x()-10, point.y()-275);
        painter.drawLine(point.x(), point.y()-300, point.x()+10, point.y()-275);*/


        QPoint Origin = worldToScreen(0.0, 0.0);
        QPoint Unit =   worldToScreen(1.0, 0.0);

        Origin *= devicePixelRatio();
        Unit   *= devicePixelRatio();
        float scale = (Unit.x()-Origin.x());
        drawXScale(painter, scale, Origin);
        drawYScale(painter, scale, Origin);
    }


    if(!m_PixOverlay.isNull())
    {
        painter.drawPixmap(0,0, m_PixOverlay);
        m_PixOverlay.fill(Qt::transparent);
    }
    painter.restore();
}






