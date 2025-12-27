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



#include <QApplication>
#include <QStandardPaths>
#include <QOpenGLFramebufferObject>
#include <QFileDialog>
#include <QMouseEvent>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QRandomGenerator>
#include <QVector4D>
#include <QQuaternion>

#include <interfaces/controls/w3dprefs.h>
#include <core/displayoptions.h>
#include <core/saveoptions.h>
#include <api/trace.h>
#include <api/units.h>
#include <core/xflcore.h>
#include <interfaces/opengl/controls/gllightdlg.h>
#include <interfaces/opengl/globals/gl_globals.h>
#include <interfaces/opengl/views/gl3dview.h>
#include <interfaces/widgets/customdlg/imagedlg.h>
#include <api/node.h>
#include <api/triangle3d.h>
#include <api/geom_global.h>

#define ZANIMINTERVAL 15


QSurfaceFormat gl3dView::s_GlSurfaceFormat;


bool gl3dView::s_bAnimateTransitions = true;
int gl3dView::s_AnimationTime = 500; //ms

double gl3dView::s_ZAnimAngle = 0.25;


Light gl3dView::s_Light;

gl3dView::gl3dView(QWidget *pParent) : QOpenGLWidget(pParent)
{
    setCursor(Qt::CrossCursor);
    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);

    reset();

    setFormat(s_GlSurfaceFormat);

    m_pglLightDlg = nullptr;

    m_bZAnimate = false;
    connect(&m_IdleTimer, SIGNAL(timeout()), SLOT(onIdleAnimate()));

    connect(&m_DynTimer, SIGNAL(timeout()), SLOT(onDynamicIncrement()));

    m_bIsImageLoaded = false;
    m_bScaleImageWithView = false;
    m_bFlipH = m_bFlipV = false;
    m_ImageScaleX = m_ImageScaleY = 1.0;

    m_ZoomFactor = 1.0;

    m_bLightVisible = false;

    m_nAnimationFrames = 50;
    m_bDynTranslation = false;
    m_bDynRotation    = false;
    m_bDynScaling     = false;

    m_uDepthLightViewMatrix = -1;
    m_uHasShadow = m_uShadowLightViewMatrix = -1;
    m_attrDepthPos = -1;
    m_fboDepthMap = m_texDepthMap = 0;
}


void gl3dView::reset()
{
    m_pOglLogger = nullptr;

    m_ClipPlanePos  = 500.0;
    m_locLine.m_ClipPlane = 0;
    m_LastPoint.setX(0);
    m_LastPoint.setY(0);
    m_PixOverlay = QPixmap(107, 97);
    m_PixOverlay.fill(Qt::transparent);
    m_PressedPoint.setX(0);
    m_PressedPoint.setY(0);

    m_RefLength = -1.0;

    m_bArcball            = false;
    m_bCrossPoint         = false;
    m_bAutoDeleteBuffers  = true;
    m_bAxes               = true;
    m_bHasMouseMoved      = false;
    m_bTrans              = false;
    m_bDynTranslation = m_bDynRotation = m_bDynScaling = false;

    m_glViewportTrans.reset();

    m_StartScale = 1.0;
    m_StartTranslation.reset();

    reset3dScale();

    m_iTimerInc = 0;

    memset(m_MatOut, 0, 16*sizeof(double));

    m_rectView = QRectF(-1.0, -1.0, 2.0, 2.0);
}


gl3dView::~gl3dView()
{
    if(m_bAutoDeleteBuffers)
    {
    }

    if(m_pglLightDlg)
    {
        m_pglLightDlg->close();
        delete m_pglLightDlg;
    }
}


void gl3dView::saveViewPoint(Quaternion &qt) const
{
    if(W3dPrefs::s_bSaveViewPoints)
        qt = m_ArcBall.m_Quat;
}


void gl3dView::restoreViewPoint(Quaternion const &qt)
{
    if(W3dPrefs::s_bSaveViewPoints)
        m_ArcBall.setQuat(qt);
}


void gl3dView::initializeGL()
{
    QOpenGLFunctions::initializeOpenGLFunctions();

    QString strange;

    QSurfaceFormat const &ctxtFormat = format();
//qDebug()<<"gl3dView::initializeGL"<<ctxtFormat;

    if(format().testOption(QSurfaceFormat::DeprecatedFunctions))
    {
    }

    if(g_bTrace)
    {
        QString log;
        log += "*************** initialized gl3dView Format ********************\n";
        if(!QOpenGLContext::currentContext())
        {
            log += ("No Current context\n");
            log += ("**************** end gl3dView Format********************\n");
            trace(log);
            return;
        }

        printFormat(ctxtFormat, strange);
        log += strange;

        if(format().testOption(QSurfaceFormat::DeprecatedFunctions))
        {
            log += "   Deprecated functions requested --> forcing use of v120 style shaders\n";
        }

        log += ("   Using GLSL v330 style shaders\n");

/*        QOpenGLContext *pOglCtx = QOpenGLContext::currentContext();
        m_pOglLogger = new QOpenGLDebugLogger(this);
        m_pOglLogger->initialize(); // initializes in the current context, i.e. ctx
        if(pOglCtx->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
           log += "   Context has debug extension GL_KHR_debug\n";
        connect(m_pOglLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), SLOT(onOglLogMsg(QOpenGLDebugMessage)));
        m_pOglLogger->startLogging();
        m_pOglLogger->enableMessages(); */
        log +="\n\n";
        trace(log);
    }    

    QString vsrc, gsrc, fsrc;

    makeStandardBuffers();


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
        strange = QString::asprintf("%s", QString("Line fragment shader log:"+m_shadLine.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadLine.link();
    m_shadLine.bind();
    {
        m_locLine.m_attrVertex   = m_shadLine.attributeLocation("vertexPosition_modelSpace");
        m_locLine.m_attrColor    = m_shadLine.attributeLocation("vertexColor");
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

    //--------- setup the shader to paint stippled large points -----------

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
        m_locPoint.m_State  = m_shadPoint.attributeLocation("PointState");
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
    vsrc = ":/shaders/point2/point2_VS.glsl";
    fsrc = ":/shaders/point2/point2_FS.glsl";

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
        m_locPt2.m_attrVertex  = m_shadPoint2.attributeLocation("vertexPosition_modelSpace");
        m_locPt2.m_attrColor   = m_shadPoint2.attributeLocation("vertexColor");
        m_locPt2.m_vmMatrix    = m_shadPoint2.uniformLocation("vmMatrix");
        m_locPt2.m_pvmMatrix   = m_shadPoint2.uniformLocation("pvmMatrix");
        m_locPt2.m_ClipPlane   = m_shadPoint2.uniformLocation("clipPlane0");
        m_locPt2.m_Shape       = m_shadPoint2.uniformLocation("pointsize");
        m_locPt2.m_HasUniColor = m_shadPoint2.uniformLocation("HasUniColor");
        m_locPt2.m_UniColor    = m_shadPoint2.uniformLocation("UniformColor");
    }
    m_shadPoint2.release();

    //setup the depth shader
    vsrc = ":/shaders/shadow/depth_VS.glsl";
    fsrc = ":/shaders/shadow/depth_FS.glsl";
    m_shadDepth.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadDepth.log().length())
    {
        QString strange = QString::asprintf("%s", QString("Depth vertex shader log:"+m_shadDepth.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadDepth.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadDepth.log().length())
    {
        QString strange = QString::asprintf("%s", QString("Depth fragment shader log:"+m_shadDepth.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadDepth.link();
    m_shadDepth.bind();
    {
        m_attrDepthPos = m_shadDepth.attributeLocation("vertexPosition_modelSpace");
        m_uDepthLightViewMatrix = m_shadDepth.uniformLocation("LightViewMatrix");
        m_shadDepth.setUniformValue(m_uDepthLightViewMatrix, m_LightViewMatrix);
    }
    m_shadDepth.release();

//    glEnable(GL_LINE_SMOOTH); // https://www.khronos.org/opengl/wiki/Multisampling -->Modern programs should not make use of these features.
//    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glSetupLight();
}


void gl3dView::initDepthMap()
{
    if(m_fboDepthMap != 0)
        return;

    // Create a texture to store the depth map
    glGenTextures(1, &m_texDepthMap);
    glBindTexture(GL_TEXTURE_2D, m_texDepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Create a frame-buffer and associate the texture with it.
    glGenFramebuffers(1, &m_fboDepthMap);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboDepthMap);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_texDepthMap, 0);

    // Let OpenGL know that we are not interested in colors for this buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Cleanup for now.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void gl3dView::onOglLogMsg(QOpenGLDebugMessage const & logmsg)
{
    trace(logmsg.message()+"\n");
}


void gl3dView::makeStandardBuffers()
{
    glMakeAxes();
    glMakeLightSource();
    glMakeArcBall(m_ArcBall);
    glMakeArcPoint(m_ArcBall);
    glMakeIcoSphere();
    glMakeIcosahedron();
    glMakeCylinder(1.0f, 0.05f, 10, 57);
    gl::makeCube(Vector3d(), 1.0,1.0,1.0, m_vboCube, m_vboCubeEdges);
    glMakeUnitArrow();
    glMakeCone(1.0f, 1.0f,  1, 57);
}


void gl3dView::glRenderView()
{
}


void gl3dView::glMake3dObjects()
{
}


void gl3dView::set3dRotationCenter(QPoint const &point)
{
    //adjusts the new rotation center after the user has picked a point on the screen
    //finds the closest panel under the point,
    //and changes the rotation vector and viewport translation
    Vector3d I, A, B, AA, BB, PP;

    screenToViewport(point, B);
    B.z = -100.0;
    A.set(B.x, B.y, +100.0);

    viewportToWorld(A, AA);
    viewportToWorld(B, BB);

    // apply the model matrix inverse rotation
    QVector4D AA4(AA.xf(), AA.yf(), AA.zf(), 1.0);
    QVector4D BB4(BB.xf(), BB.yf(), BB.zf(), 1.0);
/*    QMatrix4x4 minv = m_ModelMatrix.inverted();
    AA4 = minv * AA4;
    BB4 = minv * BB4;*/

    AA.set(double(AA4.x()), double(AA4.y()), double(AA4.z()));
    BB.set(double(BB4.x()), double(BB4.y()), double(BB4.z()));

    bool bIntersect = false;
    if(intersectTheObject(AA, BB, I))
    {
        bIntersect = true;
    }
    else
    {
        // try to intersect a plane normal to the view and passing through the origin
        // create a triangle normal to the view point
        Vector3d TL, TR, BL, BR;
        screenToWorld(QPoint(0,       0),        0, TL);
        screenToWorld(QPoint(width(), 0),        0, TR);
        screenToWorld(QPoint(0,       height()), 0, BL);
        screenToWorld(QPoint(width(), height()), 0, BR);
        Triangle3d ttl(TL, TR, BL);
        Triangle3d tbr(TR, BR, BL);
        if(ttl.intersectSegmentInside(AA, BB, I, true))
        {
            bIntersect = true;
        }
        else if(tbr.intersectSegmentInside(AA, BB, I, true))
        {
            bIntersect = true;
        }
    }

    if(bIntersect)
    {
        // apply the model matrix rotation
/*        QVector4D INear4d(I.xf(), I.yf(), I.zf(), 1.0);
        QVector4D I4d = m_ModelMatrix * INear4d;
        I.set(double(I4d.x()), double(I4d.y()), double(I4d.z()));
*/
        startTranslationTimer(I);
    }
}


void gl3dView::centerViewOn(Vector3d const &pt)
{
    // update the rotation center just in case
    m_ArcBall.getRotationMatrix(m_MatOut, true);

    setViewportTranslation();

    startTranslationTimer(pt);
}


void gl3dView::printFormat(QSurfaceFormat const & ctxtFormat, QString &log, QString prefix)
{
    QString strange;
    log.clear();

    strange = QString::asprintf("Context version: %d.%d\n", ctxtFormat.majorVersion(),ctxtFormat.minorVersion());
    log += prefix + strange;
    QString vendor, renderer, version, glslVersion;
    const GLubyte *p;
    if ((p = glGetString(GL_VENDOR)))
        vendor = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = glGetString(GL_RENDERER)))
        renderer = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = glGetString(GL_VERSION)))
        version = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = glGetString(GL_SHADING_LANGUAGE_VERSION)))
        glslVersion = QString::fromLatin1(reinterpret_cast<const char *>(p));

    log += prefix + "*** Context information ***\n";
    log += prefix + QString("   Vendor:         %1\n").arg(vendor).toStdString().c_str();
    log += prefix + QString("   Renderer:       %1\n").arg(renderer).toStdString().c_str();
    log += prefix + QString("   OpenGL version: %1\n").arg(version).toStdString().c_str();
    log += prefix + QString("   GLSL version:   %1\n").arg(glslVersion).toStdString().c_str();

    log += prefix + "Deprecated functions: "+xfl::boolToString(ctxtFormat.testOption(QSurfaceFormat::DeprecatedFunctions))+"\n";
    log += prefix + "Debug context:        "+xfl::boolToString(ctxtFormat.testOption(QSurfaceFormat::DebugContext))       +"\n";
    log += prefix + "Stereo buffers:       "+xfl::boolToString(ctxtFormat.testOption(QSurfaceFormat::StereoBuffers))      +"\n";

    strange = QString::asprintf("Sampling frames for antialiasing: %d\n", ctxtFormat.samples());
    log += prefix + strange;

    switch (ctxtFormat.profile()) {
        case QSurfaceFormat::NoProfile:
            log += prefix + "No Profile\n";
            break;
        case QSurfaceFormat::CoreProfile:
            log += prefix + "Core Profile\n";
            break;
        case QSurfaceFormat::CompatibilityProfile:
            log += prefix + "Compatibility Profile\n";
            break;
    }
    switch (ctxtFormat.renderableType())
    {
        case QSurfaceFormat::DefaultRenderableType:
            log += prefix + "DefaultRenderableType: The default, unspecified rendering method\n";
            break;
        case QSurfaceFormat::OpenGL:
            log += prefix + "OpenGL: Desktop OpenGL rendering\n";
            break;
        case QSurfaceFormat::OpenGLES:
            log += prefix + "OpenGLES: OpenGL ES 2.0 rendering\n";
            break;
        case QSurfaceFormat::OpenVG:
            log += prefix + "OpenVG: Open Vector Graphics rendering\n";
            break;
    }
}


void gl3dView::on3dFlipH()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;
    Quaternion qtflip(180.0, Vector3d(0.0,0.0,1.0));

    m_QuatEnd = m_QuatStart*qtflip;
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}


void gl3dView::on3dFlipV()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    Quaternion qtflip(180.0, Vector3d(1.0,0.0,0.0));
    float ab_flip[16];
    memset(ab_flip, 0, 16*sizeof(float));

    m_QuatEnd = m_QuatStart*qtflip;
    m_ArcBall.setQuat(m_QuatEnd);

//    memcpy(m_ArcBall.m_MatCurrent, ab_new, 16*sizeof(float));

    startRotationTimer();
    emit viewModified();
}


void gl3dView::on3dIso()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    Quaternion qti;
    qti.fromEulerAngles(ROLL, PITCH, YAW);
    Quaternion qtyaw(-30.0, Vector3d(0.0,0.0,1.0));
    m_QuatEnd = qti*qtyaw;
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}


void gl3dView::on3dBot()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    Quaternion qtTop(sqrt(2.0)/2.0, 0.0, 0.0, -sqrt(2.0)/2.0);
    Quaternion qtflip(180.0, Vector3d(0.0,1.0,0.0));
    m_QuatEnd = qtflip*qtTop;
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}


void gl3dView::on3dTop()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    m_QuatEnd.set(sqrt(2.0)/2.0, 0.0, 0.0, -sqrt(2.0)/2.0);
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}


//SHIFT + Y
void gl3dView::on3dLeft()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    m_QuatEnd.set(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0, 0.0);    // rotate by 90° around x
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}

//Y
void gl3dView::on3dRight()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;
    Quaternion qtLeft(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0, 0.0);
    Quaternion qtflip(180.0, Vector3d(0.0,1.0,0.0));
    m_QuatEnd.set(qtflip*qtLeft);
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}


void gl3dView::on3dRear()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    Quaternion Qt1(sqrt(2.0)/2.0, 0.0,           -sqrt(2.0)/2.0, 0.0);// rotate by 90° around y
    Quaternion Qt2(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0,           0.0);// rotate by 90° around x

    m_QuatEnd = Qt1*Qt2;
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}


void gl3dView::on3dFront()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    Quaternion Qt1(sqrt(2.0)/2.0, 0.0,           -sqrt(2.0)/2.0, 0.0);// rotate by 90° around y
    Quaternion Qt2(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0,           0.0);// rotate by 90° around x

    Quaternion qtflip(180.0, Vector3d(0.0,0.0,1.0));
    m_QuatEnd = Qt1*Qt2*qtflip;
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}


void gl3dView::wheelEvent(QWheelEvent *pEvent)
{
    int dy = pEvent->pixelDelta().y();
    if(dy==0) dy = pEvent->angleDelta().y(); // pixeldelta usabel on macOS and angleDelta on win/linux; depends also on driver and hardware

    if(W3dPrefs::bSpinAnimation() && abs(dy)>120)
    {
        m_bDynScaling = true;
        m_ZoomFactor = dy;

        startDynamicTimer();
    }
    else
    {
        if(m_bDynScaling && m_ZoomFactor*dy<=0)
        {
            //user has changed his mind
            m_bDynScaling=false;
            m_DynTimer.stop();
        }
        else
        {
            if(pEvent->angleDelta().y()>0) m_glScalef *= 1.0/(1.0+DisplayOptions::scaleFactor());
            else                           m_glScalef *= 1.0+DisplayOptions::scaleFactor();
        }
    }

    update();
}


void gl3dView::mouseDoubleClickEvent(QMouseEvent *pEvent)
{
    m_bHasMouseMoved = true;
    set3dRotationCenter(pEvent->pos());
}


void gl3dView::mouseMoveEvent(QMouseEvent *pEvent)
{
    QPoint point(pEvent->pos().x(), pEvent->pos().y());
    Vector3d Real;

    QPoint Delta(point.x()-m_LastPoint.x(), point.y()-m_LastPoint.y());
    screenToViewport(point, Real);

    if(std::abs(Delta.x())>10 || std::abs(Delta.y())>10)
        m_bHasMouseMoved = true;

    bool bCtrl = false;

    if (pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;
    if (pEvent->buttons()   & Qt::LeftButton)
    {
        if(bCtrl)
        {
            //rotate
            m_ArcBall.move(Real.x, Real.y);
            update();
        }
        else if(m_bTrans)
        {
            //translate
            int side = std::max(geometry().width(), geometry().height());

            m_glViewportTrans.x += Delta.x()*2.0/double(m_glScalef)/side;
            m_glViewportTrans.y += Delta.y()*2.0/double(m_glScalef)/side;

            m_glRotCenter.x = m_MatOut[0]*(m_glViewportTrans.x) + m_MatOut[1]*(-m_glViewportTrans.y) + m_MatOut[2] *m_glViewportTrans.z;
            m_glRotCenter.y = m_MatOut[4]*(m_glViewportTrans.x) + m_MatOut[5]*(-m_glViewportTrans.y) + m_MatOut[6] *m_glViewportTrans.z;
            m_glRotCenter.z = m_MatOut[8]*(m_glViewportTrans.x) + m_MatOut[9]*(-m_glViewportTrans.y) + m_MatOut[10]*m_glViewportTrans.z;

            update();
        }
    }
    else if (pEvent->buttons() & Qt::MiddleButton)
    {
        m_ArcBall.move(Real.x, Real.y);
        update();
    }
    else if(pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        float zoomFactor=1.0f;

        if(point.y()-m_LastPoint.y()<0) zoomFactor = 1.0f/1.025f;
        else                            zoomFactor = 1.025f;

        m_glScalef *= zoomFactor;
        update();
    }

    m_LastPoint = point;
}


void gl3dView::mousePressEvent(QMouseEvent *pEvent)
{
    QPoint point(pEvent->pos().x(), pEvent->pos().y());

    stopDynamicTimer();

    if(m_iTimerInc>0)
    {
        // interrupt animation and return
        m_TransitionTimer.stop();
        m_iTimerInc = 0;
    }

    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

    m_bHasMouseMoved = false;

    if (pEvent->buttons() & Qt::MiddleButton)
    {
        m_bArcball = true;
        Vector3d real;
        QPoint pt(pEvent->pos().x(), pEvent->pos().y());
        screenToViewport(pt, real);
        m_ArcBall.start(real.x, real.y);
        m_bCrossPoint = true;

        reset3dRotationCenter();
        update();
    }
    else if (pEvent->buttons() & Qt::LeftButton)
    {
        Vector3d real;
        QPoint pt(point.x(), point.y());
        screenToViewport(pt, real);
        m_ArcBall.start(real.x, real.y);
        reset3dRotationCenter();
        if(hasFocus())
        {
            if (!bCtrl)
            {
                m_bTrans = true;
                QApplication::setOverrideCursor(Qt::ClosedHandCursor);
            }
            else
            {
                m_bTrans=false;
                m_bArcball = true;
                m_bCrossPoint = true;
            }
        }
        else
            setFocus();
        update();
    }

    m_LastPoint = point;
    m_PressedPoint = point;

    m_MoveTime.restart();
}


void gl3dView::mouseReleaseEvent(QMouseEvent * pEvent)
{
    QApplication::restoreOverrideCursor();

    // reset all flags to default values
    m_bTrans         = false;
    m_bArcball       = false;
    m_bCrossPoint    = false;
    m_bHasMouseMoved = false;

    Vector3d Real;
    screenToViewport(pEvent->pos(), Real);


    //  inverse the rotation matrix and re-calculate the translation vector
    m_ArcBall.getRotationMatrix(m_MatOut, true);
    setViewportTranslation();


    if(W3dPrefs::bSpinAnimation())
    {
        int movetime = m_MoveTime.elapsed();
        if(movetime<DisplayOptions::moveTimeThreshold() && !m_PressedPoint.isNull())
        {
            bool bCtrl = false;
            if (pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

            if((pEvent->button()==Qt::LeftButton && bCtrl) || pEvent->button()==Qt::MiddleButton)
            {
                m_Trans.reset();
                Vector3d m_SpinEnd;
                m_ArcBall.getSpherePoint(Real.x, Real.y, m_SpinEnd);
                Quaternion qt;
                qt.from2UnitVectors(m_ArcBall.m_Start.normalized(), m_SpinEnd.normalized());
                m_SpinInc = Quaternion(qt.angle()/15.0, qt.axis());

                startDynamicTimer();
                m_bDynRotation = true;
            }
            else if(pEvent->button()==Qt::LeftButton)
            {
                Vector3d A, B;
                screenToWorld(m_PressedPoint, 0, A);
                screenToWorld(pEvent->pos(),  0, B);
                m_Trans = B-A;
                startDynamicTimer();
                m_bDynTranslation = true;
            }
        }
    }

    update();
    pEvent->accept();

    emit viewModified();
}


void gl3dView::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl  = (pEvent->modifiers() & Qt::ControlModifier);
    bool bShift = (pEvent->modifiers() & Qt::ShiftModifier);
//    bool bAlt   = (pEvent->modifiers() & Qt::AltModifier);

    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            if(m_DynTimer.isActive())
            {
                stopDynamicTimer();
                return;
            }
            break;
        }
        case Qt::Key_Control:
        {
            m_bArcball = true;
            update();
            pEvent->accept();
            return;
        }
        case Qt::Key_R:
        {
            on3dReset();
            pEvent->accept();
            return;
        }
        case Qt::Key_L:
        {
            if(bCtrl)
            {
                onSetupLight();
                pEvent->accept();
            }
            return;
        }
        case Qt::Key_H:
        {
            if(!bCtrl && !bShift)
            {
                on3dFlipH();
                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_V:
        {
            if(!bCtrl && !bShift)
            {
                on3dFlipV();
                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_I:
        {
            if (pEvent->modifiers().testFlag(Qt::ControlModifier) && pEvent->modifiers().testFlag(Qt::AltModifier))
            {
                if(!m_bIsImageLoaded)
                {
                    onLoadBackImage();
                }
                else
                {
                    onClearBackImage();
                }
            }
            else if(pEvent->modifiers().testFlag(Qt::AltModifier))
            {
                onSaveImage();
            }
            else
            {
                on3dIso();
                pEvent->accept();
                return;
            }
            break;
        }
        case Qt::Key_X:
        {
            if(bShift) on3dFront();
            else       on3dRear();
            pEvent->accept();
            return;
        }
        case Qt::Key_Y:
        {
            if(bShift) on3dLeft();
            else       on3dRight();
            pEvent->accept();
            return;
        }
        case Qt::Key_Z:
        {
            if(bShift) on3dBot();
            else       on3dTop();
            pEvent->accept();
            return;
        }
        default:
            break;
    }
//    pEvent->ignore();
    QOpenGLWidget::keyPressEvent(pEvent);
}


/**
*Overrides the keyReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dView::keyReleaseEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Control:
        {
            m_bArcball = false;
            update();
            break;
        }

        default:
            pEvent->ignore();
    }
}


void gl3dView::hideEvent(QHideEvent *pEvent)
{
    stopDynamicTimer();
    pEvent->ignore();
}


void gl3dView::resizeGL(int width, int height)
{
    QOpenGLWidget::resizeGL(width, height);

    double w = double(width);
    double h = double(height);
    double s = 1.0;

    if(w>h)	m_GLViewRect.setRect(-s, s*h/w, s, -s*h/w);
    else    m_GLViewRect.setRect(-s*w/h, s, s*w/h, -s);


    if(!m_PixOverlay.isNull())
    {
        QRect r(rect());
        m_PixOverlay = m_PixOverlay.scaled(r.size()*devicePixelRatio());
        m_PixOverlay.fill(Qt::transparent);
    }
}


void gl3dView::getGLError()
{
    switch(glGetError())
    {
        case GL_NO_ERROR:
            trace("No error has been recorded. The value of this symbolic constant is guaranteed to be 0.\n");
            break;

        case GL_INVALID_ENUM:
            trace("An unacceptable value is specified for an enumerated argument. "
                      "The offending command is ignored and has no other side effect than to set the error flag.\n");
            break;

        case GL_INVALID_VALUE:
            trace("A numeric argument is out of range. The offending command is ignored and has no other "
                      "side effect than to set the error flag.\n");
            break;

        case GL_INVALID_OPERATION:
            trace("The specified operation is not allowed in the current state. The offending command is "
                      "ignored and has no other side effect than to set the error flag.\n");
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            trace("The command is trying to render to or read from the framebuffer while the currently "
                      "bound framebuffer is not framebuffer complete (i.e. the return value from glCheckFramebufferStatus "
                      "is not GL_FRAMEBUFFER_COMPLETE). The offending command is ignored and has no other side effect than "
                      "to set the error flag.\n");
            break;

        case GL_OUT_OF_MEMORY:
            trace("There is not enough memory left to execute the command. The state of the GL is "
                      "undefined, except for the state of the error flags, after this error is recorded.\n");
            break;

        case GL_STACK_UNDERFLOW:
            trace("An attempt has been made to perform an operation that would cause an internal stack to underflow.\n");
            break;

        case GL_STACK_OVERFLOW:
            trace("An attempt has been made to perform an operation that would cause an internal stack to overflow.\n");
            break;
    }
}


void gl3dView::onLight(bool bOn)
{
    setLightOn(bOn);
    update();
}


void gl3dView::onSetupLight()
{
    if(!m_pglLightDlg)
    {
        m_pglLightDlg = new GLLightDlg(this);
        m_pglLightDlg->setgl3dView(this);
    }
    m_pglLightDlg->show();
}


void gl3dView::glSetupLight()
{
    QColor LightColor;
    LightColor.setRedF(  double(s_Light.m_Red));
    LightColor.setGreenF(double(s_Light.m_Green));
    LightColor.setBlueF( double(s_Light.m_Blue));
    GLfloat x = s_Light.m_X;
    GLfloat y = s_Light.m_Y;
    GLfloat z = s_Light.m_Z;

    m_shadSurf.bind();
    {
        if(isLightOn()) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
        else            m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);

        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("LightPosition_viewSpace"),  x,y,z);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("EyePosition_viewSpace"),    0,0,s_Light.m_EyeDist);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("LightColor"),               LightColor);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("LightAmbient"),             s_Light.m_Ambient);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("LightDiffuse"),             s_Light.m_Diffuse);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("LightSpecular"),            s_Light.m_Specular);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("MaterialShininess"),        float(s_Light.m_iShininess));
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("Kc"),                       s_Light.m_Attenuation.m_Constant);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("Kl"),                       s_Light.m_Attenuation.m_Linear);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("Kq"),                       s_Light.m_Attenuation.m_Quadratic);
    }
    m_shadSurf.release();

    m_shadPoint.bind();
    {
        if(isLightOn()) m_shadPoint.setUniformValue(m_locPoint.m_Light, 1);
        else            m_shadPoint.setUniformValue(m_locPoint.m_Light, 0);

        m_shadPoint.setUniformValue(m_shadPoint.uniformLocation("LightPosition_viewSpace"),  x,y,z);
        m_shadPoint.setUniformValue(m_shadPoint.uniformLocation("EyePosition_viewSpace"),    0,0,s_Light.m_EyeDist);
        m_shadPoint.setUniformValue(m_shadPoint.uniformLocation("LightColor"),               LightColor);
        m_shadPoint.setUniformValue(m_shadPoint.uniformLocation("LightAmbient"),             s_Light.m_Ambient);
        m_shadPoint.setUniformValue(m_shadPoint.uniformLocation("LightDiffuse"),             s_Light.m_Diffuse);
        m_shadPoint.setUniformValue(m_shadPoint.uniformLocation("LightSpecular"),            s_Light.m_Specular);
        m_shadPoint.setUniformValue(m_shadPoint.uniformLocation("MaterialShininess"),        float(s_Light.m_iShininess));
        m_shadPoint.setUniformValue(m_shadPoint.uniformLocation("Kc"),                       s_Light.m_Attenuation.m_Constant);
        m_shadPoint.setUniformValue(m_shadPoint.uniformLocation("Kl"),                       s_Light.m_Attenuation.m_Linear);
        m_shadPoint.setUniformValue(m_shadPoint.uniformLocation("Kq"),                       s_Light.m_Attenuation.m_Quadratic);
    }
    m_shadPoint.release();

}


void gl3dView::paintGL()
{
//    auto t0 = std::chrono::high_resolution_clock::now();

    glMake3dObjects();


//    QOpenGLPaintDevice device(size() * devicePixelRatio()); //"The context is captured upon construction."
//    QPainter painter(&device);
    QPainter painter(this);

    if(m_bIsImageLoaded && !m_BackImage.isNull())
    {
        painter.save();

        double xscale = m_ImageScaleX;
        double yscale = m_ImageScaleY;
        if(m_bScaleImageWithView)
        {
            xscale *= m_glScalef;
            yscale *= m_glScalef;
        }

        //scale from the center of the viewport
        QPoint VCenter = rect().center();

        int w = int(double(m_BackImage.width())* xscale);
        int h = int(double(m_BackImage.height())* yscale);
        //the coordinates of the top left corner are measured from the center of the viewport

        int xtop = VCenter.x() + int( - double(m_BackImage.width())  /2.*xscale);
        int ytop = VCenter.y() + int( - double(m_BackImage.height()) /2.*yscale);

        painter.drawPixmap(xtop+m_ImageOffset.x(), ytop+m_ImageOffset.y(), w, h, m_BackImage);
        painter.restore();
    }


    painter.beginNativePainting();
    paintGl3();
    painter.endNativePainting();


    paintOverlay();

/*    auto t1 = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    qDebug("gl3dView::paintGL: %7d µs", duration);*/
}


void gl3dView::paintOverlay()
{
    QOpenGLPaintDevice device(size() * devicePixelRatio());
    QPainter painter(&device);

    glDisable(GL_CULL_FACE);

    if(!m_PixOverlay.isNull())
    {
        painter.drawPixmap(0,0, m_PixOverlay);
        m_PixOverlay.fill(Qt::transparent);
    }
}


void gl3dView::paintGl3()
{
//    makeCurrent();
    if(W3dPrefs::s_bMultiSample) glEnable(GL_MULTISAMPLE);
    else                         glDisable(GL_MULTISAMPLE);

    if(!m_bIsImageLoaded)
    {
        glClearColor(float(DisplayOptions::backgroundColor().redF()), float(DisplayOptions::backgroundColor().greenF()), float(DisplayOptions::backgroundColor().blueF()), 1.0f);
        // clear the depth buffer before starting the rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    float s = 1.0;

    int width  = geometry().width();
    int height = geometry().height();

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // Enable face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_ClipPlane, m_ClipPlanePos);
    }
    m_shadSurf.release();

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_ClipPlane, m_ClipPlanePos);
        m_shadLine.setUniformValue(m_locLine.m_Viewport, QVector2D(float(m_GLViewRect.width()), float(m_GLViewRect.height())));
    }
    m_shadLine.release();

    if(m_shadPoint.isLinked())
    {
        m_shadPoint.bind();
        m_shadPoint.setUniformValue(m_locPoint.m_ClipPlane, m_ClipPlanePos);
        m_shadPoint.setUniformValue(m_locPoint.m_Viewport, QVector2D(float(m_GLViewRect.width()), float(m_GLViewRect.height())));
        m_shadPoint.release();
    }

    m_matProj.setToIdentity();
    m_matView.setToIdentity();
    m_matModel.setToIdentity();

    double m[16];
    m_ArcBall.getRotationMatrix(m, true);
    m_matView = QMatrix4x4(float(m[0]),  float(m[1]),  float(m[2]),  float(m[3]),
                           float(m[4]),  float(m[5]),  float(m[6]),  float(m[7]),
                           float(m[8]),  float(m[9]),  float(m[10]), float(m[11]),
                           float(m[12]), float(m[13]), float(m[14]), float(m[15]));


    if(GLLightDlg::isOrtho())
        m_matProj.ortho(-s,s,-(height*s)/width,(height*s)/width,-1.0e3*s,1.0e3*s);
    else
    {
        m_matProj.perspective(GLLightDlg::verticalAngle(), width/(height*s), 0.1f, 500.0f);
//        m_ProjectionMatrix.frustum(-1,1,-1,1, 1,10);
//        m_ModelMatrix.translate(0,0,5);
        QVector4D viewpos(0,0,-GLLightDlg::viewDistance(), 1.0);
        viewpos = m_matView.inverted() * viewpos;
        m_matView.translate({viewpos.x(), viewpos.y(), viewpos.z()});
    }


    if(m_bArcball)   paintArcBall();

    if(m_bAxes)
    {
        // fixed scale axis for the axis
        QMatrix4x4 vm(m_matView);
        m_matView.scale(m_glScalef, m_glScalef, m_glScalef);
        m_matView.translate(m_glRotCenter.xf(), m_glRotCenter.yf(), m_glRotCenter.zf());
        m_matView.scale(0.5f/m_glScalef, 0.5f/m_glScalef, 0.5f/m_glScalef);
        paintAxes();
        m_matView=vm; // leave things as they were
    }

    m_matView.scale(m_glScalef, m_glScalef, m_glScalef);
    m_matView.translate(m_glRotCenter.xf(), m_glRotCenter.yf(), m_glRotCenter.zf());


    glRenderView();

    paintDebugPts();

    // overlay the light
    if(m_bLightVisible)
    {
        double d = Vector3d(0,0,50).z-double(s_Light.m_Z);
        double radius = 500000.0/d/d/d;
        QColor lightColor;
        lightColor.setRedF(  double(s_Light.m_Red));
        lightColor.setGreenF(double(s_Light.m_Green));
        lightColor.setBlueF( double(s_Light.m_Blue));
        lightColor.setAlphaF(1.0);

        QMatrix4x4 vm(m_matView);
        m_matModel.setToIdentity();
        m_matModel.translate(s_Light.m_X, s_Light.m_Y, s_Light.m_Z);

        m_shadPoint.bind();
        {
            m_shadPoint.setUniformValue(m_locPoint.m_vmMatrix, m_matModel);
            m_shadPoint.setUniformValue(m_locPoint.m_pvmMatrix, m_matProj*m_matModel);
        }
        m_shadPoint.release();

        paintPoints(m_vboLightSource, radius, 0, false, lightColor, 4);

//        paintSphere(s_Light.m_X, s_Light.m_Y, s_Light.m_Z, radius, lightColor, false);
         // leave things as they were
        m_matModel.setToIdentity();
        m_matView=vm;

    }
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}


/**
* The user has modified the position of the clip plane in the 3D view
*@param pos the new z position in viewport coordinates of the clipping plane
*/
void gl3dView::onClipPlane(int pos)
{
    float coef = 5.0;
    float planepos =  float(pos)/100.0;
    if(pos>=99)       m_ClipPlanePos =  100.0f;
    else if(pos<=-99) m_ClipPlanePos = -100.0f;
    else              m_ClipPlanePos = 5.0f*sinhf(planepos*coef)/sinhf(coef);

    update();
}


void gl3dView::onClipScreenPlane(bool bClip)
{
    m_ClipPlanePos = bClip ? 0 : 1000;
    update();
}


void gl3dView::paintDebugPts()
{
#ifdef QT_DEBUG
    for(int i=0; i<m_DebugPts.size(); i++)
         paintIcosahedron(m_DebugPts.at(i), 0.0075/m_glScalef, Qt::darkRed, W3dPrefs::s_OutlineStyle, true, true);

    for(int i=0; i<m_DebugVecs.size(); i++)
    {
        if(i<m_DebugPts.size())
            paintThinArrow(m_DebugPts.at(i), m_DebugVecs.at(i), QColor(205,135,155).darker(), 2, Line::SOLID, QMatrix4x4());
    }
#endif
}


void gl3dView::on3dReset()
{
    stopDynamicTimer();
    if(s_bAnimateTransitions) startResetTimer();
    else                      reset3dScale();
}


void gl3dView::reset3dScale()
{
    m_glScalef = referenceScale();
    m_glViewportTrans.set(0.0, 0.0, 0.0);
    reset3dRotationCenter();
    update();
}


void gl3dView::paintArcBall()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_vboArcBall.bind();
        {
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 0);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
            if(DisplayOptions::isLightTheme())
                m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(54,54,54,75));
            else
                m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(43,43,43,175));
            m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(Line::SOLID));
            m_shadLine.setUniformValue(m_locLine.m_Thickness, 2.0f);

            int nSegs = m_vboArcBall.size()/2/3/int(sizeof(float)); // 2 vertices and (3 position components)
            glDrawArrays(GL_LINES, 0, nSegs*2);
        }

        m_vboArcBall.release();

        if(m_bCrossPoint)
        {
            QMatrix4x4 pvmCP(m_matProj);
            float angle, xf, yf, zf;
            m_ArcBall.rotateCrossPoint(angle, xf, yf, zf);
            pvmCP.rotate(angle, xf, yf, zf);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmCP);

            m_vboArcPoint.bind();
            {
                m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 0);
                m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(70, 25, 40));
                m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(Line::SOLID));
                m_shadLine.setUniformValue(m_locLine.m_Thickness, 3.0f);

                int nSegs = m_vboArcPoint.size()/2/3/int(sizeof(float)); // 2 vertices and (3 position components)
                glDrawArrays(GL_LINES, 0, nSegs*2);
            }
            m_vboArcPoint.release();
        }
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}


void gl3dView::paintTriangle(QOpenGLBuffer &vbo, bool bHighlight, bool bBackground, QColor const &clrBack)
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
        m_shadLine.setUniformValue(m_locLine.m_UniColor, xfl::fromfl5Clr(W3dPrefs::s_OutlineStyle.m_Color));
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


void gl3dView::paintXYCircle(QOpenGLBuffer &vbo, Vector2d const &place, double radius, QColor const &circleColor)
{
    paintXYCircle(vbo, place.x, place.y, radius, circleColor);
}


void gl3dView::paintXYCircle(QOpenGLBuffer &vbo, double xc, double yc, double radius, QColor const &circleColor)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QMatrix4x4 mCircle; //is identity
    mCircle.translate(float(xc), float(yc), 0.0f);
    mCircle.scale(float(radius));

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel * mCircle);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel * mCircle);

        m_shadLine.setUniformValue(m_locLine.m_UniColor, circleColor);
        m_shadLine.setUniformValue(m_locLine.m_Thickness, 1.0f);
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);

        vbo.bind();
        {
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
            int nPts = vbo.size() /3 /int(sizeof(float)); // Note: unchecked
            glDrawArrays(GL_LINE_STRIP, 0, nPts);
        }
        vbo.release();

        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}


void gl3dView::onRotationIncrement()
{
    if(m_iTimerInc>m_nAnimationFrames)
    {
        m_TransitionTimer.stop();
        return;
    }
    Quaternion qtrot;
    double t = double(m_iTimerInc)/double(m_nAnimationFrames);
    qtrot.slerp(m_QuatStart, m_QuatEnd, t);
    m_ArcBall.setQuat(qtrot);

    reset3dRotationCenter();
    update();
    m_iTimerInc++;
}


void gl3dView::startResetTimer()
{
    m_iTimerInc = 0;

    // calculate the number of animation frames for 60Hz refresh rate
    int period = 17; //60 Hz in ms
    m_nAnimationFrames = int(double(s_AnimationTime)/double(period));


    m_StartScale = m_glScalef;
    m_StartTranslation = m_glViewportTrans;
    m_TransIncrement = (Vector3d(0.0,0.0,0.0)-m_glViewportTrans)/float(m_nAnimationFrames);

    disconnect(&m_TransitionTimer, nullptr, nullptr, nullptr);
    connect(&m_TransitionTimer, SIGNAL(timeout()), SLOT(onResetIncrement()));
    m_TransitionTimer.start(period);//7 ms x 50 times
}


void gl3dView::startRotationTimer()
{
    if(s_bAnimateTransitions)
    {
        m_iTimerInc = 0;

        // calculate the number of animation frames for 60Hz refresh rate
        int period = 17; //60 Hz in ms
        m_nAnimationFrames = int(double(s_AnimationTime)/double(period));

        disconnect(&m_TransitionTimer, nullptr, nullptr, nullptr);
        connect(&m_TransitionTimer, SIGNAL(timeout()), SLOT(onRotationIncrement()));
        m_TransitionTimer.start(period);
    }
    else
    {
        reset3dRotationCenter();
        update();
    }
}


void gl3dView::startTranslationTimer(Vector3d const &PP)
{
    int period = 17; //60 Hz in ms
    m_nAnimationFrames = int(double(s_AnimationTime)/double(period));

    double inc = double(m_nAnimationFrames);
    if(s_bAnimateTransitions)
    {
        m_TransIncrement.x = (-PP.x -m_glRotCenter.x)/inc;
        m_TransIncrement.y = (-PP.y -m_glRotCenter.y)/inc;
        m_TransIncrement.z = (-PP.z -m_glRotCenter.z)/inc;

        m_iTimerInc = 0;

        disconnect(&m_TransitionTimer, nullptr, nullptr, nullptr);
        connect(&m_TransitionTimer, SIGNAL(timeout()), SLOT(onTranslationIncrement()));
        m_TransitionTimer.start(period);
    }
    else
    {
        m_glRotCenter.set(-PP.x, -PP.y, -PP.z);
        setViewportTranslation();

        update();
    }
}


void gl3dView::onDynamicIncrement()
{
    if(m_bDynRotation)
    {
        if(fabs(m_SpinInc.angle())<0.01)
        {
            stopDynamicTimer();
            update();
            return;
        }
        m_SpinInc = Quaternion(m_SpinInc.angle()*(1.0-W3dPrefs::spinDamping()), m_SpinInc.axis());
        m_ArcBall.applyRotation(m_SpinInc, false);
    }

    if(m_bDynTranslation)
    {
        double dist = m_Trans.norm()*m_glScalef;
        if(dist<0.01)
        {
            stopDynamicTimer();
            update();
            return;
        }
        m_glRotCenter += m_Trans/10.0;
        setViewportTranslation();

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

        m_glScalef *= scalefactor;
        m_ZoomFactor *= (1.0-W3dPrefs::spinDamping());
    }

    update();
}


void gl3dView::startDynamicTimer()
{
    m_DynTimer.start(17);
}


void gl3dView::stopDynamicTimer()
{
    if(m_DynTimer.isActive())
    {
        m_DynTimer.stop();
//        reset3dRotationCenter();
        //  inverse the rotation matrix and re-calculate the translation vector
        m_ArcBall.getRotationMatrix(m_MatOut, true);
        setViewportTranslation();
    }
    m_bDynTranslation = m_bDynRotation = m_bDynScaling = false;
//    setMouseTracking(true);
}


void gl3dView::onTranslationIncrement()
{
    if(m_iTimerInc>=m_nAnimationFrames)
    {
        m_TransitionTimer.stop();
        m_iTimerInc = 0;
        return;
    }

    m_glRotCenter += m_TransIncrement;
    setViewportTranslation();

    update();
    m_iTimerInc++;
}


void gl3dView::setViewportTranslation()
{
    m_glViewportTrans.x =  (m_MatOut[0]*m_glRotCenter.x + m_MatOut[1]*m_glRotCenter.y + m_MatOut[2] *m_glRotCenter.z);
    m_glViewportTrans.y = -(m_MatOut[4]*m_glRotCenter.x + m_MatOut[5]*m_glRotCenter.y + m_MatOut[6] *m_glRotCenter.z);
    m_glViewportTrans.z =  (m_MatOut[8]*m_glRotCenter.x + m_MatOut[9]*m_glRotCenter.y + m_MatOut[10]*m_glRotCenter.z);
}


void gl3dView::onResetIncrement()
{
    if(m_iTimerInc>m_nAnimationFrames)
    {
        m_TransitionTimer.stop();
        m_iTimerInc = 0;
        return;
    }

    double frac = double(m_iTimerInc)/double(m_nAnimationFrames);
    double tau = sin(frac * PI/2.0); //start fast, end slowly

    m_glScalef = m_StartScale *(1.0-frac) + referenceScale()*frac;
    m_glViewportTrans = m_StartTranslation * (1.0-tau) + Vector3d(0.0,0.0,0.0)*tau;

    reset3dRotationCenter();
    update();
    m_iTimerInc++;
}


void gl3dView::reset3dRotationCenter()
{
    m_ArcBall.getRotationMatrix(m_MatOut, false);

    m_glRotCenter.x = m_MatOut[0]*(m_glViewportTrans.x) + m_MatOut[1]*(-m_glViewportTrans.y) + m_MatOut[2] *m_glViewportTrans.z;
    m_glRotCenter.y = m_MatOut[4]*(m_glViewportTrans.x) + m_MatOut[5]*(-m_glViewportTrans.y) + m_MatOut[6] *m_glViewportTrans.z;
    m_glRotCenter.z = m_MatOut[8]*(m_glViewportTrans.x) + m_MatOut[9]*(-m_glViewportTrans.y) + m_MatOut[10]*m_glViewportTrans.z;
}


void gl3dView::paintQuad(QColor const &clrBack, bool bContour, float thickness, bool bHighlight, bool bBackground, bool bLight, QOpenGLBuffer &vbo)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int stride = 6;

    vbo.bind();
    {
        if(vbo.size()==0)
        {
            vbo.release();
            return;
        }
        if(bContour)
        {
            m_shadLine.bind();
            {
                m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
                m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, stride*sizeof(GLfloat));
                m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(Line::SOLID));

                if(bHighlight)
                {
                    m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::highlightColor());
                    m_shadLine.setUniformValue(m_locLine.m_Thickness, float(W3dPrefs::highlightWidth()));
                    glDrawArrays(GL_LINE_STRIP, 0, 5);
                }
                else
                {
                    m_shadLine.setUniformValue(m_locLine.m_Thickness, float(W3dPrefs::s_PanelStyle.m_Width));
                    m_shadLine.setUniformValue(m_locLine.m_Thickness, thickness);
                    glDrawArrays(GL_LINE_STRIP, 0, 5);
                }

                m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
            }
            m_shadLine.release();
        }

        if(bBackground)
        {
            m_shadSurf.bind();
            {
                m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                 3, stride*sizeof(GLfloat));
                m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);
                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3*sizeof(GLfloat), 3, stride*sizeof(GLfloat));

                m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);
                m_shadSurf.setUniformValue(m_locSurf.m_UniColor, clrBack);

                if(bLight) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
                else       m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);

                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);
                glDisable(GL_CULL_FACE);

                glDrawArrays(GL_TRIANGLES, 0, 3); // nodes 0,1,2
                glDrawArrays(GL_TRIANGLES, 2, 3); // nodes 2,3,4
                glDisable(GL_POLYGON_OFFSET_FILL);
                glEnable(GL_CULL_FACE);

                m_shadSurf.disableAttributeArray(m_locSurf.m_UniColor);
                m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
            }
            m_shadSurf.release();
        }
    }
    vbo.release();
}


double gl3dView::drawReferenceLength()
{
    int nPixels = (this->width()/4);

    int xc = int(rect().center().x()*devicePixelRatioF());
    QPoint screenPt0(xc-nPixels, DisplayOptions::textFontStruct().height()*devicePixelRatioF());
    QPoint screenPt1(xc+nPixels, DisplayOptions::textFontStruct().height()*devicePixelRatioF());
    Vector3d v0, v1;
    screenToWorld(screenPt0, 0, v0);
    screenToWorld(screenPt1, 0, v1);
    double length = (v1-v0).norm()/devicePixelRatioF();

    QPainter painter(&m_PixOverlay);
    painter.save();
    QFont fnt = DisplayOptions::textFontStruct().font();
    fnt.setPointSize(fnt.pointSize()*devicePixelRatio());
    painter.setFont(fnt);
    painter.setPen(QPen());
    QPen pen(DisplayOptions::textColor().darker());
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawLine(screenPt0, screenPt1);
    QString strange;
    strange = QString::asprintf("%g ", length*Units::mtoUnit());
    strange += Units::lengthUnitQLabel();

//    strange = QString::asprintf("devicePixelRatioF %g", devicePixelRatioF());
    painter.drawText(xc-DisplayOptions::textFontStruct().width(strange)*devicePixelRatio()/2, DisplayOptions::textFontStruct().height()*devicePixelRatioF()*2, strange);

    painter.restore();
    return length;
}


void gl3dView::paintAxes()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_HasUniColor, 1);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, xfl::fromfl5Clr(W3dPrefs::s_AxisStyle.m_Color));
        m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(W3dPrefs::s_AxisStyle.m_Stipple));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, float(W3dPrefs::s_AxisStyle.m_Width));
        m_shadLine.setUniformValue(m_locLine.m_Viewport, QVector2D(float(m_GLViewRect.width()), float(m_GLViewRect.height())));
        m_vboAxes.bind();
        {
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);

            int nvertices = m_vboAxes.size()/3/int(sizeof(float)); // three components
            glDrawArrays(GL_LINES, 0, nvertices);
        }
        m_vboAxes.release();

        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();

    const float delta = 0.015f;
    glRenderText(1.0+3.0*delta, 0.0+delta,     0.0+delta,     m_AxisTitle[0], DisplayOptions::textColor());
    glRenderText(0.0+delta,     1.0+3.0*delta, 0.0+delta,     m_AxisTitle[1], DisplayOptions::textColor());
    glRenderText(0.0+delta,     0.0+delta,     1.0+3.0*delta, m_AxisTitle[2], DisplayOptions::textColor());
}


void gl3dView::paintLineStrip(QOpenGLBuffer &vbo, LineStyle const &ls)
{
    paintLineStrip(vbo, xfl::fromfl5Clr(ls.m_Color), ls.m_Width, ls.m_Stipple);
}


void gl3dView::paintLineStrip(QOpenGLBuffer &vbo, QColor const &clr, float width, Line::enumLineStipple stipple)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int stride = 3;

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_UniColor, clr);

        m_shadLine.setUniformValue(m_locLine.m_Thickness, width);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(stipple));

        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, stride * sizeof(GLfloat));
            int nPoints = vbo.size()/stride/int(sizeof(float));
            glDrawArrays(GL_LINE_STRIP, 0, nPoints);
            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        }
        vbo.release();
    }
    m_shadLine.release();
}


void gl3dView::paintColorSegments(QOpenGLBuffer &vbo, float width, Line::enumLineStipple stipple)
{
    int stride = 6;
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.enableAttributeArray(m_locLine.m_attrColor);
        m_shadLine.setUniformValue(m_locLine.m_HasUniColor, 0);
        m_shadLine.setUniformValue(m_locLine.m_Thickness, width);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(stipple));

        vbo.bind();
        {
            // 2 vertices x (3 coords + 3 color components)
            int nSegs = vbo.size() /2 /stride /int(sizeof(float));

            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex,  GL_FLOAT, 0,                  3, stride * sizeof(GLfloat));
            m_shadLine.setAttributeBuffer(m_locLine.m_attrColor,   GL_FLOAT, 3* sizeof(GLfloat), 3, stride * sizeof(GLfloat));

            glDrawArrays(GL_LINES, 0, nSegs*2);
        }
        vbo.release();
        m_shadLine.disableAttributeArray(m_locLine.m_attrColor);
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setUniformValue(m_locLine.m_HasUniColor, 1); // leave things as they were
    }
    m_shadLine.release();
}


void gl3dView::paintColourSegments8(QOpenGLBuffer &vbo, LineStyle const &ls)
{
    paintColourSegments8(vbo, float(ls.m_Width), ls.m_Stipple);
}


void gl3dView::paintColourSegments8(QOpenGLBuffer &vbo, float width, Line::enumLineStipple stipple)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    int stride = 8;
    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.enableAttributeArray(m_locLine.m_attrColor);

        m_shadLine.setUniformValue(m_locLine.m_HasUniColor, 0);
        m_shadLine.setUniformValue(m_locLine.m_Thickness, width);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(stipple));

        vbo.bind();
        {
            // 2 vertices x (4 coords + 4 color components)
            int nSegs = vbo.size() /2 /stride /int(sizeof(float));

            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0,                  4, stride * sizeof(GLfloat));
            m_shadLine.setAttributeBuffer(m_locLine.m_attrColor,  GL_FLOAT, 4* sizeof(GLfloat), 4, stride * sizeof(GLfloat));

            glDrawArrays(GL_LINES, 0, nSegs*2);
        }
        vbo.release();
        m_shadLine.disableAttributeArray(m_locLine.m_attrColor);
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setUniformValue(m_locLine.m_HasUniColor, 1); // leave things as they were
    }
    m_shadLine.release();
}


void gl3dView::paintSegments(QOpenGLBuffer &vbo, LineStyle const &ls, bool bHigh)
{
    paintSegments(vbo, xfl::fromfl5Clr(ls.m_Color), float(ls.m_Width), ls.m_Stipple, bHigh);
}


void gl3dView::paintSegments(QOpenGLBuffer &vbo, QColor const &clr, float thickness, Line::enumLineStipple stip, bool bHigh)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3*sizeof(GLfloat));

            int nSegs = vbo.size()/2/3/int(sizeof(float)); // 2 vertices and (3 position components)

            if(bHigh)
            {
                m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(W3dPrefs::highlightPattern()));
                m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::highlightColor());

                m_shadLine.setUniformValue(m_locLine.m_Thickness, float(W3dPrefs::highlightWidth()));
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


void gl3dView::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl3dView");
    {
        int OGLMajor = s_GlSurfaceFormat.majorVersion();
        int OGLMinor = s_GlSurfaceFormat.minorVersion();
        settings.setValue("OpenGL_major", OGLMajor);
        settings.setValue("OpenGL_minor", OGLMinor);

        switch(s_GlSurfaceFormat.profile())
        {
            case QSurfaceFormat::NoProfile:            settings.setValue("Profile", 0);  break;
            case QSurfaceFormat::CoreProfile:          settings.setValue("Profile", 1);  break;
            case QSurfaceFormat::CompatibilityProfile: settings.setValue("Profile", 2);  break;
        }
        settings.setValue("MultiSamples", s_GlSurfaceFormat.samples());
        settings.setValue("DeprecatedFuncs", s_GlSurfaceFormat.testOption(QSurfaceFormat::DeprecatedFunctions));

        settings.setValue("Ambient",      s_Light.m_Ambient);
        settings.setValue("Diffuse",      s_Light.m_Diffuse);
        settings.setValue("Specular",     s_Light.m_Specular);

        settings.setValue("XLight",       s_Light.m_X);
        settings.setValue("YLight",       s_Light.m_Y);
        settings.setValue("ZLight",       s_Light.m_Z);

        settings.setValue("EyeDist",      s_Light.m_EyeDist);

        settings.setValue("RedLight",     s_Light.m_Red);
        settings.setValue("GreenLight",   s_Light.m_Green);
        settings.setValue("BlueLight",    s_Light.m_Blue);
        settings.setValue("bLight",       s_Light.m_bIsLightOn);

        settings.setValue("MatShininess", s_Light.m_iShininess);

        settings.setValue("ConstantAtt",  s_Light.m_Attenuation.m_Constant);
        settings.setValue("LinearAtt",    s_Light.m_Attenuation.m_Linear);
        settings.setValue("QuadraticAtt", s_Light.m_Attenuation.m_Quadratic);
    }
    settings.endGroup();
}


void gl3dView::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl3dView");
    {
        int OGLMajor = settings.value("OpenGL_major", 3).toInt();
        int OGLMinor = settings.value("OpenGL_minor", 3).toInt();
        s_GlSurfaceFormat.setMajorVersion(OGLMajor);
        s_GlSurfaceFormat.setMinorVersion(OGLMinor);
        switch(settings.value("Profile",1).toInt())
        {
            case 0: s_GlSurfaceFormat.setProfile(QSurfaceFormat::NoProfile);            break;
            default:
            case 1: s_GlSurfaceFormat.setProfile(QSurfaceFormat::CoreProfile);          break;
            case 2: s_GlSurfaceFormat.setProfile(QSurfaceFormat::CompatibilityProfile); break;
        }
        s_GlSurfaceFormat.setSamples(settings.value("MultiSamples", 4).toInt());

        s_GlSurfaceFormat.setOption(QSurfaceFormat::DeprecatedFunctions, settings.value("DeprecatedFuncs", false).toBool());

        s_Light.m_Ambient           = settings.value("Ambient",  s_Light.m_Ambient).toFloat();
        s_Light.m_Diffuse           = settings.value("Diffuse",  s_Light.m_Diffuse).toFloat();
        s_Light.m_Specular          = settings.value("Specular", s_Light.m_Specular).toFloat();

        s_Light.m_X                 = settings.value("XLight",  s_Light.m_X).toFloat();
        s_Light.m_Y                 = settings.value("YLight",  s_Light.m_Y).toFloat();
        s_Light.m_Z                 = settings.value("ZLight",  s_Light.m_Z).toFloat();
        s_Light.m_EyeDist           = settings.value("EyeDist", s_Light.m_EyeDist).toFloat();

        s_Light.m_Red               = settings.value("RedLight",   s_Light.m_Red).toFloat();
        s_Light.m_Green             = settings.value("GreenLight", s_Light.m_Green).toFloat();
        s_Light.m_Blue              = settings.value("BlueLight",  s_Light.m_Blue).toFloat();

        s_Light.m_iShininess                = settings.value("MatShininess", s_Light.m_iShininess).toInt();
        s_Light.m_Attenuation.m_Constant    = settings.value("ConstantAtt",  s_Light.m_Attenuation.m_Constant).toFloat();
        s_Light.m_Attenuation.m_Linear      = settings.value("LinearAtt",    s_Light.m_Attenuation.m_Linear).toFloat();
        s_Light.m_Attenuation.m_Quadratic   = settings.value("QuadraticAtt", s_Light.m_Attenuation.m_Quadratic).toFloat();

        s_Light.m_bIsLightOn        = settings.value("bLight", true).toBool();
    }
    settings.endGroup();
}


/**
 * Renders an arrow composed of a cylinder and a cone, starting at origin, in the direction arrow and with length |arrow|
 */
void gl3dView::paintThickArrow(Vector3d const &origin, const Vector3d& arrow, QColor const &clr, QMatrix4x4 const &ModelMatrix)
{
    float length = arrow.normf();
    if(fabsf(length)<LENGTHPRECISION) return; // zero length arrow to draw

    QMatrix4x4 translation;
    translation.translate(origin.xf(), origin.yf(), origin.zf());

    QVector3D N(0,0,1);// this is the vector used to define m_vboArrow
    QVector3D A(arrow.xf(), arrow.yf(), arrow.zf());
    A.normalize();
    QQuaternion qqt = QQuaternion::rotationTo(N, A);
    QMatrix4x4 ArrowDirectionMat;
    ArrowDirectionMat.rotate(qqt);
//    ArrowDirectionMat.translate(origin.xf(), origin.yf(), origin.zf());
    ArrowDirectionMat.scale(length);

    QMatrix4x4 ArrowMat; // identity
//    modelMat.translate(origin.xf(), origin.yf(), origin.zf());
    ArrowMat.scale(0.3f,  0.3f,  1.0f); // squeeze the cylinder radially
    ArrowMat.scale(0.8f, 0.8f, 0.8f); // make it 80% of the arrow's length
    QMatrix4x4 pvmMatrix = m_matProj * m_matView * ModelMatrix *translation* ArrowDirectionMat * ArrowMat;
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, m_matView*ModelMatrix * ArrowMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMatrix);
    }
    m_shadSurf.release();
    paintTriangles3Vtx(m_vboCylinder, clr, false, true);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*ModelMatrix * ArrowMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMatrix);
    }
    m_shadLine.release();
    paintSegments(m_vboCylinderContour, clr.darker(), 1);

    ArrowMat.setToIdentity();
    ArrowMat.translate(0.0f, 0.0f, 0.8f);
    ArrowMat.scale(0.2f, 0.2f, 0.2f); // make it 20% of the arrow's length
    ArrowMat.scale(0.17f, 0.17f, 1.0f); // reduce the base diameter

    pvmMatrix = m_matProj * m_matView * ModelMatrix*translation* ArrowDirectionMat * ArrowMat ;
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, m_matView*ModelMatrix * ArrowMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMatrix);
    }
    m_shadSurf.release();
    paintTriangles3Vtx(m_vboCone, clr, false, true);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*ModelMatrix * ArrowMat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMatrix);
    }
    m_shadLine.release();
    paintLineStrip(m_vboConeContour, clr.darker(), 1);

    // leave things as they were
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj * m_matView*m_matModel);
    }
    m_shadSurf.release();

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj * m_matView*m_matModel);
    }
    m_shadLine.release();
}


void gl3dView::paintColourMap(QOpenGLBuffer &vbo, QMatrix4x4 const& m_ModelMatrix)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int stride = 6;

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix,  m_matView*m_ModelMatrix);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView*m_ModelMatrix);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrColor);

        vbo.bind();
        {
            // 3 vertices x(3 coords + 3 clr components)
            int nTriangles = vbo.size()/3/stride/int(sizeof(float));

            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                  3, stride * sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrColor,  GL_FLOAT, 3* sizeof(GLfloat), 3, stride * sizeof(GLfloat));

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


void gl3dView::paintThinArrow(Vector3d const &origin, const Vector3d& arrow, LineStyle const &ls, QMatrix4x4 const ModelMatrix)
{
    paintThinArrow(origin, arrow, xfl::fromfl5Clr(ls.m_Color), float(ls.m_Width), ls.m_Stipple, ModelMatrix);
}

void gl3dView::paintThinArrow(Vector3d const &origin, const Vector3d& arrow,
                              fl5Color const &clr, float w, Line::enumLineStipple stipple, QMatrix4x4 const ModelMatrix)
{
    paintThinArrow(origin, arrow, xfl::fromfl5Clr(clr), w, stipple, ModelMatrix);
}
/**
 * Renders an arrow composed of three lines, starting at origin, in the direction arrow and with length |arrow|
 */
void gl3dView::paintThinArrow(Vector3d const &origin, const Vector3d& arrow,
                              QColor const &clr, float w, Line::enumLineStipple stipple, QMatrix4x4 const ModelMatrix)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    float length = arrow.normf();
    if(fabsf(length)<LENGTHPRECISION) return; // zero length arrow to draw

    QVector3D N(0,0,1);// this is the vector used to define m_vboArrow
    QVector3D A(arrow.xf(), arrow.yf(), arrow.zf());
    A.normalize();
    QQuaternion qqt = QQuaternion::rotationTo(N, A);
    QMatrix4x4 modelRotationMat;
    modelRotationMat.rotate(qqt);


    QMatrix4x4 modelmat; // identity
    modelmat.translate(origin.xf(), origin.yf(), origin.zf());
    modelmat *= modelRotationMat;
    modelmat.scale(arrow.normf(), arrow.normf(), arrow.normf());

    QMatrix4x4 pvmMatrix = m_matProj * m_matView * ModelMatrix * modelmat;
    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*ModelMatrix*modelmat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMatrix);

        m_shadLine.setUniformValue(m_locLine.m_UniColor, clr);
        m_shadLine.setUniformValue(m_locLine.m_Thickness, w);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(stipple));

        m_vboThinArrow.bind();
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
        int nPoints = m_vboThinArrow.size()/3/int(sizeof(float));
        glDrawArrays(GL_LINES, 0, nPoints);
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        m_vboThinArrow.release();

        // leave things as they were

        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView * ModelMatrix);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();
}


void gl3dView::paintTriangles3Vtx(QOpenGLBuffer &vbo, const fl5Color &backclr, bool bTwoSided, bool bLight)
{
    paintTriangles3Vtx(vbo, xfl::fromfl5Clr(backclr), bTwoSided, bLight);
}


void gl3dView::paintTriangles3Vtx(QOpenGLBuffer &vbo, const QColor &backclr, bool bTwoSided, bool bLight)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int stride = 6;

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, backclr);
        if(bLight) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
        else       m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);

        if(bTwoSided)
        {
            m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 1);
            glDisable(GL_CULL_FACE);
        }
        else
        {
            m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0);
            glEnable(GL_CULL_FACE);
        }

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


void gl3dView::paintSphere(Vector3d const &place, float radius, QColor const &sphereColor, bool bLight)
{
    paintSphere(place.xf(), place.yf(), place.zf(), radius, sphereColor, bLight);
}


/** @todo make paintTriangles3Vtx */
void gl3dView::paintSphere(float xs, float ys, float zs, float radius, const QColor &color, bool bLight)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    QMatrix4x4 mSphere; //is identity
    mSphere.translate(xs, ys, zs);
    mSphere.scale(radius);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat*mSphere);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat*mSphere);
        if(bLight) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
        else       m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, color);
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);


        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);

        m_vboIcoSphere.bind();
        {
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                  3, 6 * sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

            glEnable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

            int nTriangles = m_vboIcoSphere.size()/3/6/int(sizeof(float)); // 3 vertices and (3 position components+3 normal components)
            glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);
            glDisable(GL_CULL_FACE);
        }

        m_vboIcoSphere.release();

        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);

        // leave things as they were
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();
}


void gl3dView::paintSphereInstances(QOpenGLBuffer &vboPosInstances, float radius, QColor const &clr, bool bTwoSided, bool bLight)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int nTriangles(0);
    int nObjects(0);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_Scale, radius);
        m_shadSurf.setUniformValue(m_locSurf.m_HasTexture,  0);
        m_shadSurf.setUniformValue(m_locSurf.m_IsInstanced, 1);
        if(bTwoSided) m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 1);
        else          m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0);
        if(bLight) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
        else       m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);

        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);
        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, clr);

        glEnable(GL_CULL_FACE);

        m_vboIcoSphere.bind();
        {
            m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
            m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);

            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                 3, 6*sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat));

            nTriangles = m_vboIcoSphere.size()/3/6/int(sizeof(float));
//            glDrawArrays(GL_TRIANGLES, 0, nTriangles*3); // 4 vertices defined but only 3 are used
        }
        m_vboIcoSphere.release();

        vboPosInstances.bind();
        {
            m_shadSurf.enableAttributeArray(m_locSurf.m_attrOffset);
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrOffset, GL_FLOAT, 0, 3, 3*sizeof(float));
            glVertexAttribDivisor(m_locSurf.m_attrOffset, 1);

            nObjects = vboPosInstances.size()/3/int(sizeof(float));

            glDrawArraysInstanced(GL_TRIANGLES, 0, nTriangles*3, nObjects);
            glVertexAttribDivisor(m_locSurf.m_attrOffset, 0);
            m_shadSurf.disableAttributeArray(m_locSurf.m_attrOffset);
        }
        vboPosInstances.release();
        glDisable(GL_CULL_FACE);

        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
        m_shadSurf.setUniformValue(m_locSurf.m_IsInstanced, 0);
    }
    m_shadSurf.release();
}


void gl3dView::onZAnimate(bool bZAnimate)
{
    m_bZAnimate = bZAnimate;
    if(m_bZAnimate)
    {
        m_IdleTimer.start(ZANIMINTERVAL);
    }
    else
        m_IdleTimer.stop();
}


void gl3dView::onIdleAnimate()
{
    if(!isVisible()) return;
    if(!m_bZAnimate) return;
    if(m_DynTimer.isActive()) return;

    m_bArcball = false;
    m_bCrossPoint = false;

    // apply camera rotation
    Quaternion qtrot(s_ZAnimAngle, Vector3d(0,0,1));
    m_ArcBall.applyRotation(qtrot, true);
    reset3dRotationCenter();

    update();
}


void gl3dView::paintTriangleFan(QOpenGLBuffer &vbo, QColor const &clr, bool bLight, bool bCullFaces)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    if(!bCullFaces) glDisable(GL_CULL_FACE);

    m_shadSurf.bind();
    {
        vbo.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_UniColor, clr);

            m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
            m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                  3, 6 * sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

            if(bLight) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
            else       m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);

            int nVertices = vbo.size()/6/int(sizeof(float)); //(3 position components + 3 normal components)

            glPolygonMode(GL_FRONT, GL_FILL);
            glDrawArrays(GL_TRIANGLE_FAN, 0, nVertices);
        }
        vbo.release();
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
    }
    m_shadSurf.release();

    glEnable(GL_CULL_FACE);
}


void gl3dView::onLoadBackImage()
{
    m_ImagePath = QFileDialog::getOpenFileName(this, "Open Image File",
                                            SaveOptions::lastDirName(),
                                            "Image files (*.png *.jpg *.bmp)");

    QFileInfo fi(m_ImagePath);
    if(fi.exists())
        SaveOptions::setLastDirName(fi.canonicalPath());
    else return;

    QImage img(m_ImagePath);


#if (QT_VERSION < QT_VERSION_CHECK(6, 9, 0))
    if(m_bFlipH||m_bFlipV)
    img = img.mirrored(m_bFlipH, m_bFlipV);
#else
    if(m_bFlipV)
        img = img.flipped(Qt::Vertical);

    if(m_bFlipH)
        img = img.flipped(Qt::Horizontal);
#endif
    m_bIsImageLoaded = m_BackImage.convertFromImage(img);
    update();

    onBackImageSettings();
}


void gl3dView::onClearBackImage()
{
    m_bIsImageLoaded = false;
    update();
}


void gl3dView::onBackImageSettings()
{    
    QVector<double> values;
    values << m_ImageOffset.x() << m_ImageOffset.y() << m_ImageScaleX << m_ImageScaleY;
    ImageDlg dlg(this, values, m_bScaleImageWithView, m_bFlipH, m_bFlipV);
    connect(&dlg, SIGNAL(imageChanged(bool,bool,bool,QPointF,double,double)), SLOT(onUpdateImageSettings(bool,bool,bool,QPointF,double,double)));

    dlg.exec();
    update();
}


void gl3dView::onUpdateImageSettings(bool bScaleWithView, bool bFlipH, bool bFlipV, QPointF const& offset, double xscale, double yscale)
{
    m_bScaleImageWithView = bScaleWithView;
    m_bFlipH              = bFlipH;
    m_bFlipV              = bFlipV;
    m_ImageOffset         = offset;
    m_ImageScaleX         = xscale;
    m_ImageScaleY         = yscale;

    QImage img(m_ImagePath);

#if (QT_VERSION < QT_VERSION_CHECK(6, 9, 0))
    if(m_bFlipH||m_bFlipV)
    img = img.mirrored(m_bFlipH, m_bFlipV);
#else
    if(m_bFlipV)
        img = img.flipped(Qt::Vertical);

    if(m_bFlipH)
        img = img.flipped(Qt::Horizontal);
#endif

    m_bIsImageLoaded = m_BackImage.convertFromImage(img);

    update();
}


void gl3dView::paintCube(double x, double y, double z, double side, QColor const &clr, bool bLight)
{
    paintBox(x, y, z, side, side, side, clr, bLight);
}


void gl3dView::paintBox(double x, double y, double z, double dx, double dy, double dz, QColor const &clr, bool bLight)
{
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    QMatrix4x4 modelmat;
    modelmat.translate(x, y, z);
    modelmat.scale(dx, dy, dz);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat*modelmat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat*modelmat);
    }
    m_shadSurf.release();

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, vmMat*modelmat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat*modelmat);
    }
    m_shadLine.release();

    paintTriangles3Vtx(m_vboCube, clr, false, bLight);
    paintSegments(m_vboCubeEdges, W3dPrefs::s_OutlineStyle);

    //leave things as they were
    modelmat.setToIdentity();
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();
}


void gl3dView::paintIcosahedron(const Vector3d &place, float radius, const QColor &color, LineStyle const &ls,
                                bool bOutline, bool bLight)
{
    QMatrix4x4 modelmat;
    modelmat.translate(place.xf(), place.yf(), place.zf());
    modelmat.scale(radius, radius, radius);
    QMatrix4x4 vmmat = m_matView*m_matModel*modelmat;
    QMatrix4x4 pvmmat = m_matProj*vmmat;

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmmat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmmat);
    }
    m_shadSurf.release();

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, vmmat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmmat);
    }
    m_shadLine.release();

    paintTriangles3Vtx(m_vboIcosahedron, color, false, bLight);
    if(bOutline) paintSegments(m_vboIcosahedronEdges, ls);

    //leave things as they were
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadSurf.release();

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();
}


void gl3dView::paintIcoSphere(Vector3d const&place, double radius, fl5Color const &color, bool bTriangles, bool bOutline)
{
    paintIcoSphere(place, radius, xfl::fromfl5Clr(color), bTriangles, bOutline);
}


void gl3dView::paintIcoSphere(Vector3d const&place, double radius, QColor const &color, bool bTriangles, bool bOutline)
{
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    QMatrix4x4 modelmat;
    modelmat.translate(place.xf(), place.yf(), place.zf());
    modelmat.scale(radius, radius, radius);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat*modelmat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat*modelmat);
    }
    m_shadSurf.release();

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*modelmat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat*modelmat);
    }
    m_shadLine.release();

    if(bTriangles) paintTriangles3Vtx(m_vboIcoSphere, color, false, true);
    if(bOutline) paintSegments(m_vboIcoSphereEdges, W3dPrefs::s_OutlineStyle);

    //leave things as they were
    modelmat.setToIdentity();
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();
}


void gl3dView::paintPoints(QOpenGLBuffer &vbo, float width, int iShape, bool bLight, fl5Color const &clr, int stride)
{
    paintPoints(vbo, width, iShape, bLight, xfl::fromfl5Clr(clr), stride);
}


void gl3dView::paintPoints(QOpenGLBuffer &vbo, float width, int iShape, bool bLight, QColor const &clr, int stride)
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


void gl3dView::glMakeLightSource()
{
    int nPts = 1;
    int buffersize = nPts*4;
    QVector<float> pts(buffersize);

    pts[0] = 0.0;
    pts[1] = 0.0;
    pts[2] = 0.0;
    pts[3] = -1.0; // invalid state so that the shader uses the uniform colour instead

    if(m_vboLightSource.isCreated()) m_vboLightSource.destroy();
    m_vboLightSource.create();
    m_vboLightSource.bind();
    m_vboLightSource.allocate(pts.data(), buffersize * int(sizeof(GLfloat)));
    m_vboLightSource.release();
}


void gl3dView::glMakeAxes()
{
    QVector<GLfloat>axisVertexArray(54);

    GLfloat const x_axis[] = {
        -1.0f,   0.0f,    0.0f,
         1.0f,   0.0f,    0.0f,
         1.0f,   0.0f,    0.0f,
         0.95f,  0.015f,  0.015f,
         1.0f,   0.0f,    0.0f,
         0.95f, -0.015f, -0.015f
    };

    GLfloat const y_axis[] = {
          0.0f,    -1.0f,    0.0f,
          0.0f,     1.0f,    0.0f,
          0.f,      1.0f,    0.0f,
          0.015f,   0.95f,   0.015f,
          0.f,      1.0f,    0.0f,
         -0.015f,   0.95f,  -0.015f
    };

    GLfloat const z_axis[] = {
         0.0f,    0.0f,   -1.0f,
         0.0f,    0.0f,    1.0f,
         0.0f,    0.0f,    1.0f,
         0.015f,  0.015f,  0.95f,
         0.0f,    0.0f,    1.0f,
        -0.015f, -0.015f,  0.95f
    };

    int iv=0;
    for(int i=0; i<18; i++) axisVertexArray[iv++] = x_axis[i]*1.0f;
    for(int i=0; i<18; i++) axisVertexArray[iv++] = y_axis[i]*1.0f;
    for(int i=0; i<18; i++) axisVertexArray[iv++] = z_axis[i]*1.0f;

    Q_ASSERT(iv==54);

    m_vboAxes.destroy();
    m_vboAxes.create();
    m_vboAxes.bind();
    m_vboAxes.allocate(axisVertexArray.constData(), axisVertexArray.size() * int(sizeof(GLfloat)));
    m_vboAxes.release();
}


//ArcBall parameters
#define NUMANGLES     57
#define NUMCIRCLES     6
#define NUMPERIM     131
#define NUMARCPOINTS  11


void gl3dView::glMakeArcBall(ArcBall & arcball)
{
    float GLScale = 1.0f;
    int row(0), col(0);
    float lat_incr(0), lon_incr(0), phi(0), phi1(0), theta(0),  theta1(0);

    float Radius = float(arcball.s_sphereRadius);
    lat_incr =  90.0f / NUMANGLES;
    lon_incr = 360.0f / NUMCIRCLES;

    int iv=0;

//    int bufferSize = ((NUMCIRCLES*2)*(NUMANGLES-2) + (NUMPERIM-1)*2)*3;
    int nSegs= NUMCIRCLES * (NUMANGLES-3) *2;
    nSegs += (NUMPERIM-2)*2;

    int buffersize = nSegs;
    buffersize *= 2; // 2 vertices/segment
    buffersize *= 3; // 3 components/vertex

    QVector<float> ArcBallVertexArray(buffersize, 0);

    //ARCBALL
    for (col=0; col<NUMCIRCLES; col++)
    {
        //first
        phi = (col * lon_incr) * PIf/180.0f;
        for (row=1; row<NUMANGLES-2; row++)
        {
            theta  = ( row    * lat_incr) * PIf/180.0f;
            theta1 = ((row+1) * lat_incr) * PIf/180.0f;
            ArcBallVertexArray[iv++] = Radius*cosf(phi)*cosf(theta)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(theta)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(phi)*cosf(theta)*GLScale;
            ArcBallVertexArray[iv++] = Radius*cosf(phi)*cosf(theta1)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(theta1)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(phi)*cosf(theta1)*GLScale;
        }
    }

    for (col=0; col<NUMCIRCLES; col++)
    {
        //Second
        phi = (col * lon_incr ) * PIf/180.0f;
        for (row=1; row<NUMANGLES-2; row++)
        {
            theta  = -( row    * lat_incr) * PIf/180.0f;
            theta1 = -((row+1) * lat_incr) * PIf/180.0f;
            ArcBallVertexArray[iv++] = Radius*cosf(phi)*cosf(theta)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(theta)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(phi)*cosf(theta)*GLScale;
            ArcBallVertexArray[iv++] = Radius*cosf(phi)*cosf(theta1)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(theta1)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(phi)*cosf(theta1)*GLScale;
        }
    }

    theta = 0.;
    for(col=1; col<NUMPERIM-1; col++)
    {
        phi  = (0.0f +  col   *360.0f/72.0f) * PIf/180.0f;
        phi1 = (0.0f + (col+1)*360.0f/72.0f) * PIf/180.0f;
        ArcBallVertexArray[iv++] = Radius * cosf(phi)  * cosf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(phi)  * cosf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * cosf(phi1) * cosf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(phi1) * cosf(theta)*GLScale;
    }

    theta = 0.;
    for(col=1; col<NUMPERIM-1; col++)
    {
        phi =  (0.0f +  col   *360.0f/72.0f) * PIf/180.0f;
        phi1 = (0.0f + (col+1)*360.0f/72.0f) * PIf/180.0f;
        ArcBallVertexArray[iv++] = Radius * cosf(-phi)  * cosf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(-phi)  * cosf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * cosf(-phi1) * cosf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(-phi1) * cosf(theta)*GLScale;
    }
    Q_ASSERT(iv==buffersize);

    m_vboArcBall.destroy();
    m_vboArcBall.create();
    m_vboArcBall.bind();
    m_vboArcBall.allocate(ArcBallVertexArray.data(), buffersize * int(sizeof(GLfloat)));

    m_vboArcBall.release();
}


void gl3dView::glMakeArcPoint(ArcBall const&arcball)
{
    float theta(0), theta1(0), phi(0), phi1(0);
    float Radius = float(arcball.s_sphereRadius);

    float Angle(10.0);

    int iv=0;

    int nsegs = (2*NUMARCPOINTS-1) * 2;
    int buffersize = nsegs * 2 * 3; // 2 vertices and 3 components

    QVector<float> ArcPointVertexArray(buffersize, 0);

    //ARCPOINT
    float lat_incr = Angle / NUMARCPOINTS;

    phi = 0.0;
    for (int row=-NUMARCPOINTS; row<NUMARCPOINTS-1; row++)
    {
        theta  = (0.0f +  row    * lat_incr) * PIf/180.0f;
        theta1 = (0.0f + (row+1) * lat_incr) * PIf/180.0f;
        ArcPointVertexArray[iv++] = Radius*cosf(phi)*cosf(theta);
        ArcPointVertexArray[iv++] = Radius*sinf(theta);
        ArcPointVertexArray[iv++] = Radius*sinf(phi)*cosf(theta);
        ArcPointVertexArray[iv++] = Radius*cosf(phi)*cosf(theta1);
        ArcPointVertexArray[iv++] = Radius*sinf(theta1);
        ArcPointVertexArray[iv++] = Radius*sinf(phi)*cosf(theta1);
    }

    theta = 0.0;
    for(int col=-NUMARCPOINTS; col<NUMARCPOINTS-1; col++)
    {
        phi  = (0.0f +  col   *Angle/NUMARCPOINTS) * PIf/180.0f;
        phi1 = (0.0f + (col+1)*Angle/NUMARCPOINTS) * PIf/180.0f;
        ArcPointVertexArray[iv++] = Radius * cosf(phi) * cosf(theta);
        ArcPointVertexArray[iv++] = Radius * sinf(theta);
        ArcPointVertexArray[iv++] = Radius * sinf(phi) * cosf(theta);
        ArcPointVertexArray[iv++] = Radius * cosf(phi1) * cosf(theta);
        ArcPointVertexArray[iv++] = Radius * sinf(theta);
        ArcPointVertexArray[iv++] = Radius * sinf(phi1) * cosf(theta);
    }

    Q_ASSERT(iv==buffersize);

    m_vboArcPoint.destroy();
    m_vboArcPoint.create();
    m_vboArcPoint.bind();
    m_vboArcPoint.allocate(ArcPointVertexArray.data(), buffersize * int(sizeof(GLfloat)));
    m_vboArcPoint.release();
}


/**
 Creates a vbo for a cylinder with height h and radius r, with axis Z
 */
void gl3dView::glMakeCylinder(float h, float r, int nz, int nh)
{
    int buffersize = 0;
    buffersize += // lateral triangles
              nz     // nz quads in the z direction
            * nh     // nh hoop quads
            * 2;     // 2 triangles/quad

    // top and bottom triangles
    buffersize +=
            nh       // nh hoop triangles
            *2;      // top and bottom faces

    // GLfloat count
    buffersize *=
              3      // 3 vertices/triangle to close the triangle
            * 6;     // (3 coords+3normal components)/ vertex

    QVector<GLfloat> CylVertexArray(buffersize);

    int iv=0;
    //side triangles
    for (int iz=0; iz<nz; iz++)
    {
        GLfloat z  = h* float(iz)  /float(nz);
        GLfloat z1 = h* float(iz+1)/float(nz);

        for (int iLat=0; iLat<nh; iLat++)
        {
            float theta  = float(iLat)  *2.0f*PIf/float(nh);
            float theta1 = float(iLat+1)*2.0f*PIf/float(nh);
            GLfloat x  = r* cosf(theta);
            GLfloat x1 = r* cosf(theta1);
            GLfloat y  = r* sinf(theta);
            GLfloat y1 = r* sinf(theta1);

            // normal components
            float nx = -cosf(theta);    float nx1 = -cosf(theta1);
            float ny = -sinf(theta);    float ny1 = -sinf(theta1);

            // first triangle
            CylVertexArray[iv++] = x;
            CylVertexArray[iv++] = y;
            CylVertexArray[iv++] = z;
            CylVertexArray[iv++] = nx;
            CylVertexArray[iv++] = ny;
            CylVertexArray[iv++] = 0.0;

            CylVertexArray[iv++] = x1;
            CylVertexArray[iv++] = y1;
            CylVertexArray[iv++] = z;
            CylVertexArray[iv++] = nx1;
            CylVertexArray[iv++] = ny1;
            CylVertexArray[iv++] = 0.0;

            CylVertexArray[iv++] = x1;
            CylVertexArray[iv++] = y1;
            CylVertexArray[iv++] = z1;
            CylVertexArray[iv++] = nx1;
            CylVertexArray[iv++] = ny1;
            CylVertexArray[iv++] = 0.0;

            //second triangle
            CylVertexArray[iv++] = x;
            CylVertexArray[iv++] = y;
            CylVertexArray[iv++] = z;
            CylVertexArray[iv++] = nx;
            CylVertexArray[iv++] = ny;
            CylVertexArray[iv++] = 0.0;

            CylVertexArray[iv++] = x1;
            CylVertexArray[iv++] = y1;
            CylVertexArray[iv++] = z1;
            CylVertexArray[iv++] = nx1;
            CylVertexArray[iv++] = ny1;
            CylVertexArray[iv++] = 0.0;

            CylVertexArray[iv++] = x;
            CylVertexArray[iv++] = y;
            CylVertexArray[iv++] = z1;
            CylVertexArray[iv++] = nx;
            CylVertexArray[iv++] = ny;
            CylVertexArray[iv++] = 0.0;
        }
    }

    // top and bottom triangles
    for(int iz=0; iz<2; iz++)
    {
        float z = float(iz) * h;
        float sign = iz==0 ? -1.0f : 1.0f;
        for (int iLat=0; iLat<nh; iLat++)
        {
            float theta  = float(iLat)  *2.0f*PIf/float(nh);
            float theta1 = float(iLat+1)*2.0f*PIf/float(nh);
            GLfloat x  = r* cosf(theta);
            GLfloat x1 = r* cosf(theta1);
            GLfloat y  = r* sinf(theta);
            GLfloat y1 = r* sinf(theta1);

            // bot triangle
            CylVertexArray[iv++] = 0.0f;
            CylVertexArray[iv++] = 0.0f;
            CylVertexArray[iv++] = z;
            CylVertexArray[iv++] = 0.0f;
            CylVertexArray[iv++] = 0.0f;
            CylVertexArray[iv++] = sign;

            CylVertexArray[iv++] = x;
            CylVertexArray[iv++] = y;
            CylVertexArray[iv++] = z;
            CylVertexArray[iv++] = 0.0f;
            CylVertexArray[iv++] = 0.0f;
            CylVertexArray[iv++] = sign;

            CylVertexArray[iv++] = x1;
            CylVertexArray[iv++] = y1;
            CylVertexArray[iv++] = z;
            CylVertexArray[iv++] = 0.0f;
            CylVertexArray[iv++] = 0.0f;
            CylVertexArray[iv++] = sign;
        }
    }

    Q_ASSERT(iv==buffersize);

    m_vboCylinder.create();
    m_vboCylinder.bind();
    m_vboCylinder.allocate(CylVertexArray.constData(), CylVertexArray.size() * int(sizeof(GLfloat)));
    m_vboCylinder.release();

    // make the contour vbo, which are just the base circle
    // using GL_LINES since the circles are disjoint
    buffersize = 2   // 2 circles
                 *nh // segments
                 *2  // vertices
                 *3; // 3 coordinates
    CylVertexArray.resize(buffersize);
    iv=0;

    // top and bottom circles
    for(int iz=0; iz<2; iz++)
    {
        float z = float(iz) * h;
        for (int iLat=0; iLat<nh; iLat++)
        {
            float theta  = float(iLat)  *2.0f*PIf/float(nh);
            float theta1 = float(iLat+1)*2.0f*PIf/float(nh);
            GLfloat x  = r* cosf(theta);
            GLfloat x1 = r* cosf(theta1);
            GLfloat y  = r* sinf(theta);
            GLfloat y1 = r* sinf(theta1);

            CylVertexArray[iv++] = x;
            CylVertexArray[iv++] = y;
            CylVertexArray[iv++] = z;

            CylVertexArray[iv++] = x1;
            CylVertexArray[iv++] = y1;
            CylVertexArray[iv++] = z;
        }
    }
    Q_ASSERT(iv==buffersize);

    m_vboCylinderContour.create();
    m_vboCylinderContour.bind();
    m_vboCylinderContour.allocate(CylVertexArray.constData(), CylVertexArray.size() * int(sizeof(GLfloat)));
    m_vboCylinderContour.release();
}


/**
 * Creates a vbo for an icosahedron with unit radius
*/
void gl3dView::glMakeIcosahedron()
{
    double radius = 1.0;
    // make vertices
    QVector<Node> vtx(12);
    vtx[10].set(0,0, radius);   // North pole
    vtx[11].set(0,0,-radius);   // South pole
    double x=0,y=0,z=0;
    double atn= atan(0.5);
    double di=0.0;
    for(int i=0; i<5; i++)
    {
        di = double(i);
        x = radius * cos(atn)*cos(72.0*di*PI/180.0);
        y = radius * cos(atn)*sin(72.0*di*PI/180.0);
        z = radius * sin(atn);
        vtx[i].set(x,y,z);
        vtx[i].setNormal(x,y,z);

        x =  radius * cos(atn)*cos((36+72.0*di)*PI/180.0);
        y =  radius * cos(atn)*sin((36+72.0*di)*PI/180.0);
        z = -radius * sin(atn);
        vtx[i+5].set(x,y,z);
        vtx[i+5].setNormal(x,y,z);
    }

    std::vector<Triangle3d> triangle(20);

    // 20 triangles
    // x3vertices/triangle
    // 3 coordinates/vertex

    //make the top five triangles from the North pole to the northern hemisphere latitude

    for(int i=0; i<5; i++)
    {
        int i1 = i;
        int i2 = (i+1)%5;
        triangle[i].setTriangle(vtx[10], vtx[i1], vtx[i2]);
    }
    //make the bottom five triangles from the South pole to the northern hemisphere latitude
    for(int i=0; i<5; i++)
    {
        int i1 = 5+i;
        int i2 = 5+(i+1)%5;
        triangle[5+i].setTriangle(vtx[11], vtx[i2], vtx[i1]);
    }

    // make the equatorial belt
    for(int i=0; i<5; i++)
    {
        int i1 = i;
        int i2 = (i+1)%5;
        triangle[10+i].setTriangle(vtx[i1], vtx[i1+5], vtx[i2]);
    }
    for(int i=0; i<5; i++)
    {
        int i1 = 5+i;
        int i2 = 5+(i+1)%5;
        triangle[15+i].setTriangle(vtx[i1], vtx[i2], vtx[(i1+1)%5]);
    }

    gl::makeTriangles3Vtx(triangle, true, m_vboIcosahedron);
    gl::makeTrianglesOutline(triangle, Vector3d(), m_vboIcosahedronEdges);
}


void gl3dView::glMakeIcoSphere(int nSplits)
{
    double radius = 1.0;
    // make vertices
    std::vector<Triangle3d> icotriangles;
    geom::makeSphere(radius, nSplits, icotriangles);
    gl::makeTriangles3Vtx(icotriangles, false, m_vboIcoSphere);
    gl::makeTrianglesOutline(icotriangles, Vector3d(), m_vboIcoSphereEdges);
}


void gl3dView::glMakeUnitArrow()
{
    QVector<GLfloat>axisVertexArray(18);

    GLfloat const z_axis[] = {
         0.0f,    0.0f,    0.0f,
         0.0f,    0.0f,    1.0f,
         0.0f,    0.0f,    1.0f,
         0.025f,  0.025f,  0.89f,
         0.0f,    0.0f,    1.0f,
        -0.025f, -0.025f,  0.89f
    };

    int iv=0;

    for(int i=0; i<18; i++) axisVertexArray[iv++] = z_axis[i]*1.0f;

    Q_ASSERT(iv==18);

    m_vboThinArrow.destroy();
    m_vboThinArrow.create();
    m_vboThinArrow.bind();
    m_vboThinArrow.allocate(axisVertexArray.constData(), axisVertexArray.size() * int(sizeof(GLfloat)));
    m_vboThinArrow.release();
}



/**
 Creates a vbo for a cylinder with height h and radius r, with axis Z
 */
void gl3dView::glMakeCone(float h, float rad, int nz, int nh)
{
//    int nz = 1;   // number of steps in the Z direction
//    int nh = 10;  // number of hoop steps

    int buffersize = 0;
    buffersize += // lateral triangles
              nz     // nz quads in the z direction
            * nh     // nh hoop quads
//            * 2;     // 2 triangles/quad
            *1; // 1 triangle
    // top and bottom triangles
    buffersize +=
            nh       // nh hoop triangles
            *1;      // bottom faces

    // GLfloat count
    buffersize *=
              3      // 3 vertices/triangle
            * 6;     // (3 coords+3normal components)/ vertex

    QVector<GLfloat> VertexArray(buffersize);

    int iv=0;
    //side triangles
    for (int iz=0; iz<nz; iz++)
    {
        GLfloat z  = h* float(iz)  /float(nz);
        GLfloat z1 = h* float(iz+1)/float(nz);

        for (int iLat=0; iLat<nh; iLat++)
        {
            float theta  = float(iLat)  *2.0f*PIf/float(nh);
            float theta1 = float(iLat+1)*2.0f*PIf/float(nh);
            GLfloat x  = rad* cosf(theta);
            GLfloat x1 = rad* cosf(theta1);
            GLfloat y  = rad* sinf(theta);
            GLfloat y1 = rad* sinf(theta1);

            // normal components
            float nx = -cosf(theta);    float nx1 = -cosf(theta1);
            float ny = -sinf(theta);    float ny1 = -sinf(theta1);

            // Only one triangle, the second is null
            VertexArray[iv++] = x * (1.0f-z/h);
            VertexArray[iv++] = y * (1.0f-z/h);
            VertexArray[iv++] = z;
            VertexArray[iv++] = nx;
            VertexArray[iv++] = ny;
            VertexArray[iv++] = 0.0;

            VertexArray[iv++] = x1 * (1.0f-z/h);
            VertexArray[iv++] = y1 * (1.0f-z/h);
            VertexArray[iv++] = z;
            VertexArray[iv++] = nx1;
            VertexArray[iv++] = ny1;
            VertexArray[iv++] = 0.0;

            VertexArray[iv++] = x1 * (1.0f-z1/h);
            VertexArray[iv++] = y1 * (1.0f-z1/h);
            VertexArray[iv++] = z1;
            VertexArray[iv++] = nx1;
            VertexArray[iv++] = ny1;
            VertexArray[iv++] = 0.0;
        }
    }

    // bottom triangles
    float z = 0.0;
    float sign = -1.0f;
    for (int iLat=0; iLat<nh; iLat++)
    {
        float theta  = float(iLat)  *2.0f*PIf/float(nh);
        float theta1 = float(iLat+1)*2.0f*PIf/float(nh);
        GLfloat x  = rad* cosf(theta);
        GLfloat x1 = rad* cosf(theta1);
        GLfloat y  = rad* sinf(theta);
        GLfloat y1 = rad* sinf(theta1);

        // bot triangle
        VertexArray[iv++] = 0.0f;
        VertexArray[iv++] = 0.0f;
        VertexArray[iv++] = z;
        VertexArray[iv++] = 0.0f;
        VertexArray[iv++] = 0.0f;
        VertexArray[iv++] = sign;

        VertexArray[iv++] = x;
        VertexArray[iv++] = y;
        VertexArray[iv++] = z;
        VertexArray[iv++] = 0.0f;
        VertexArray[iv++] = 0.0f;
        VertexArray[iv++] = sign;

        VertexArray[iv++] = x1;
        VertexArray[iv++] = y1;
        VertexArray[iv++] = z;
        VertexArray[iv++] = 0.0f;
        VertexArray[iv++] = 0.0f;
        VertexArray[iv++] = sign;
    }

    Q_ASSERT(iv==buffersize);

    m_vboCone.create();
    m_vboCone.bind();
    m_vboCone.allocate(VertexArray.constData(), VertexArray.size() * int(sizeof(GLfloat)));
    m_vboCone.release();

    // make the contour vbo, which is just the base circle
    buffersize = (nh+1)*3; // nh+1 vertices * 3 for a line strip
    VertexArray.resize(buffersize);
    z = 0.0;
//    sign = -1.0f;
    iv=0;
    for (int iLat=0; iLat<=nh; iLat++)
    {
        float theta  = float(iLat)  *2.0f*PIf/float(nh);
        GLfloat x = rad* cosf(theta);
        GLfloat y = rad* sinf(theta);
        VertexArray[iv++] = x;
        VertexArray[iv++] = y;
        VertexArray[iv++] = z;
    }
    Q_ASSERT(iv==buffersize);

    m_vboConeContour.create();
    m_vboConeContour.bind();
    m_vboConeContour.allocate(VertexArray.constData(), VertexArray.size() * int(sizeof(GLfloat)));
    m_vboConeContour.release();
}


/**
 * Convert the light position defined in viewport coordinates to world coordinates
 * and setup the light's perspective matrix.
 */
void gl3dView::updateLightMatrix()
{
    QVector3D center(-m_glRotCenter.xf(), -m_glRotCenter.yf(), -m_glRotCenter.zf());
    QVector3D lightpos(s_Light.m_X, s_Light.m_Y, s_Light.m_Z);
    QVector3D up(0,1,0);

    double m[16];
    m_ArcBall.getRotationMatrix(m, true);
    QMatrix4x4 RotMatrix = QMatrix4x4(float(m[0]),  float(m[1]),  float(m[2]),  float(m[3]),                              float(m[4]),  float(m[5]),  float(m[6]),  float(m[7]),
                                      float(m[8]),  float(m[9]),  float(m[10]), float(m[11]),
                                      float(m[12]), float(m[13]), float(m[14]), float(m[15]));

    m_LightViewMatrix.setToIdentity();

    lightpos *= 1.0/m_glScalef;
    m_LightViewMatrix.lookAt(lightpos, center, up);
    m_LightViewMatrix = m_LightViewMatrix * RotMatrix;
    m_LightViewMatrix.translate(m_glRotCenter.xf(), m_glRotCenter.yf(), m_glRotCenter.zf());

    int width  = geometry().width();
    int height = geometry().height();
    QMatrix4x4 projectionmatrix;
    projectionmatrix.perspective(75.0, float(width)/float(height), 0.1f, m_RefLength*10.0);

    m_LightViewMatrix = projectionmatrix*m_LightViewMatrix;
}


void gl3dView::paintTrianglesToDepthMap(QOpenGLBuffer &vbo, QMatrix4x4 const &ModelMat, int stride)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadDepth.bind();
    {
//        m_Shader_Depth.setUniformValue(m_uDepthLightViewMatrix, ModelMat*m_LightViewMatrix);
        m_shadDepth.setUniformValue(m_uDepthLightViewMatrix, m_LightViewMatrix*ModelMat);
        m_shadDepth.enableAttributeArray(m_attrDepthPos);

        vbo.bind();
        {
            int nTriangles = vbo.size()/3/stride/int(sizeof(float)); // three vertices and (3 position components+3 normal components)
            m_shadDepth.setAttributeBuffer(m_attrDepthPos, GL_FLOAT, 0, 3, stride*sizeof(GLfloat));
            glDrawArrays(GL_TRIANGLES, 0, nTriangles*3); // 4 vertices defined but only 3 are used
            m_shadDepth.disableAttributeArray(m_attrDepthPos);
        }
        vbo.release();
    }
    m_shadDepth.release();
}



void gl3dView::paintPoints2(QOpenGLBuffer &vbo, float width, int stride)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadPoint2.bind();
    {
        m_shadPoint2.enableAttributeArray(m_locPt2.m_attrVertex);
        m_shadPoint2.enableAttributeArray(m_locPt2.m_attrColor);

        vbo.bind();
        {
            m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrVertex, GL_FLOAT, 0,                  4, stride * sizeof(GLfloat));
            m_shadPoint2.setAttributeBuffer(m_locPt2.m_attrColor,  GL_FLOAT, 4* sizeof(GLfloat), 4, stride * sizeof(GLfloat));

            int nPoints = vbo.size()/stride/int(sizeof(float));
            glPointSize(width);

            m_shadPoint2.setUniformValue(m_locPt2.m_Shape, width);
            glEnable (GL_POINT_SPRITE);
            glEnable(GL_PROGRAM_POINT_SIZE); // To set the point size in the shader, glEnable with argument (GL_PROGRAM_POINT_SIZE)
            glDrawArrays(GL_POINTS, 0, nPoints);
        }
        vbo.release();

        m_shadPoint2.disableAttributeArray(m_locPt2.m_attrVertex);
        m_shadPoint2.disableAttributeArray(m_locPt2.m_attrColor);
    }
    m_shadPoint2.release();
}


void gl3dView::paintTriangles3VtxShadow(QOpenGLBuffer &vbo, const QColor &backclr, bool bTwoSided, bool bLight,
                                        QMatrix4x4 const &modelmat, int stride)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, backclr);

//        m_Shader_Surf.setUniformValue(m_uShadowLightViewMatrix, modelmat*m_LightViewMatrix);
        m_shadSurf.setUniformValue(m_uShadowLightViewMatrix, m_LightViewMatrix*modelmat);

        if(bLight)
        {
            m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
            m_shadSurf.setUniformValue(m_uHasShadow, 1);
        }
        else
        {
            m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);
            m_shadSurf.setUniformValue(m_uHasShadow, 0);
        }

        if(bTwoSided)
        {
            m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 1);
            glDisable(GL_CULL_FACE);
        }
        else
        {
            m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0);
            glEnable(GL_CULL_FACE);
        }

        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);

        vbo.bind();
        int nTriangles = vbo.size()/3/stride/int(sizeof(float)); // three vertices and (3 position components+3 normal components)

        m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                 3, stride*sizeof(GLfloat));
        m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3*sizeof(GLfloat), 3, stride*sizeof(GLfloat));
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

        glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);

        glDisable(GL_POLYGON_OFFSET_FILL);

        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0); // leave things as they were
        glEnable(GL_CULL_FACE);
    }
    m_shadSurf.release();

    vbo.release();
}


void gl3dView::paintTriangles3VtxTexture(QOpenGLBuffer &vbo, QOpenGLTexture *pTexture, bool bTwoSided, bool bLight)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int stride  = 8; // three vertices and (3 position components+3 normal components+2UV components)
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 1);

        m_shadSurf.setUniformValue(m_uShadowLightViewMatrix, m_LightViewMatrix);
        m_shadSurf.setUniformValue(m_uHasShadow, 0);

        if(bLight) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
        else       m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);


        if(bTwoSided)
        {
            m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 1);
            glDisable(GL_CULL_FACE);
        }
        else
        {
            m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0);
            glEnable(GL_CULL_FACE);
        }

        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrUV);

        pTexture->bind();
        {
            vbo.bind();
            {
                int nTriangles = vbo.size()/3/stride/int(sizeof(float));

                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                 3, stride*sizeof(GLfloat));
                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3*sizeof(GLfloat), 3, stride*sizeof(GLfloat));
                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrUV,     GL_FLOAT, 6*sizeof(GLfloat), 2, stride*sizeof(GLfloat));
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

                glDrawArrays(GL_TRIANGLES, 0, nTriangles*3); // 4 vertices defined but only 3 are used

                glDisable(GL_POLYGON_OFFSET_FILL);
            }
            vbo.release();
        }
        pTexture->release();

        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrUV);
        // leave things as they were
        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided,   0);
        m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 0);


        glEnable(GL_CULL_FACE);
    }
    m_shadSurf.release();
}



void gl3dView::paintTriangles3VtxOutline(QOpenGLBuffer &vbo, QColor clr, int thickness)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int stride = 6; // n vertices x (3 position components+3 normal components)

    int nVertices = 3;

    if(!vbo.isCreated()) return;

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_UniColor, clr);
        m_shadLine.setUniformValue(float(m_locLine.m_Thickness), thickness);

        m_shadLine.setUniformValue(m_locLine.m_Pattern, gl::stipple(Line::SOLID));
        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, stride*sizeof(GLfloat));

            int nTriangles = vbo.size()/nVertices/stride/int(sizeof(float));

            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

            glDrawArrays(GL_LINE_STRIP, 0, nTriangles*3);// 4 vertices defined
        }
        vbo.release();

        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}


/**
* Converts screen coordinates to OpenGL Viewport coordinates.
* @param point the screen coordinates.
* @param real the viewport coordinates.
*/
void gl3dView::screenToViewport(QPoint const &point, Vector3d &real) const
{
    double h2 = double(geometry().height()) /2.0;
    double w2 = double(geometry().width())  /2.0;

    real.x =  (double(point.x()) - w2) / w2;
    real.y = -(double(point.y()) - h2) / w2;
}


/**
*Converts screen coordinates to OpenGL Viewport coordinates.
*@param point the screen coordinates.
*@param real the viewport coordinates.
*/
void gl3dView::screenToViewport(QPoint const &point, int z, Vector3d &real) const
{
    double h2 = double(geometry().height()) /2.0;
    double w2 = double(geometry().width())  /2.0;

    real.x =  (double(point.x()) - w2) / w2;
    real.y = -(double(point.y()) - h2) / w2;

    real.z = double(z);
}


/**
*Converts OpenGL Viewport coordinates to screen coordinates
*@param real the viewport coordinates.
*@param point the screen coordinates.
*/
void gl3dView::viewportToScreen(Vector3d const &real, QPoint &point) const
{
    double h2 = m_GLViewRect.height() /2.0;
    double w2 = m_GLViewRect.width()  /2.0;

    double dx = ( real.x + w2)/2.0;
    double dy = (-real.y + h2)/2.0;

    point.setX(int(dx * double(geometry().width())));
    point.setY(int(dy * double(geometry().width())));
}


QVector4D gl3dView::worldToViewport(Vector3d const&v) const
{
    QVector4D v4(float(v.x), float(v.y), float(v.z), 1.0f);
    return m_matProj*m_matView*m_matModel * v4;
}


QPoint gl3dView::worldToScreen(Vector3d const&v, QVector4D &vScreen) const
{
    QVector4D v4(float(v.x), float(v.y), float(v.z), 1.0f);
    vScreen = m_matProj*m_matView*m_matModel * v4;
    return QPoint(int((vScreen.x()+1.0f)*width()/2), int((1.0f-vScreen.y())*height()/2));
}


QPoint gl3dView::worldToScreen(QVector4D v4, QVector4D &vScreen) const
{
    vScreen = m_matProj*m_matView*m_matModel * v4;
    return QPoint(int((vScreen.x()+1.0f)*width()/2), int((1.0f-vScreen.y())*height()/2));
}


void gl3dView::screenToWorld(QPoint const &screenpt, int z, Vector3d &modelpt) const
{
    QMatrix4x4 m;
    QVector4D in, out;
    Vector3d real;

    screenToViewport(screenpt, z, real);
    in.setX(float(real.x));
    in.setY(float(real.y));
    in.setZ(float(real.z));
    in.setW(1.0);

    bool bInverted=false;
    QMatrix4x4 vmMatrix = m_matView*m_matModel;
    m = vmMatrix.inverted(&bInverted);
    out = m * in;

    if(fabs(double(out[3]))>PRECISION)
    {
        modelpt.x = double(out[0]/out[3]);
        modelpt.y = double(out[1]/out[3]);
        modelpt.z = double(out[2]/out[3]);
    }
    else
    {
        modelpt.set(double(out[0]), double(out[1]), double(out[2]));
    }
}


void gl3dView::viewportToWorld(Vector3d vp, Vector3d &w) const
{
    //un-translate
    vp.x += - m_glViewportTrans.x*double(m_glScalef);
    vp.y += + m_glViewportTrans.y*double(m_glScalef);

    //un-scale
    vp.x *= 1.0/double(m_glScalef);
    vp.y *= 1.0/double(m_glScalef);
    vp.z *= 1.0/double(m_glScalef);

    //un-rotate
    double m[16];
    m_ArcBall.getRotationMatrix(m, false);
    w.x = m[0]*vp.x + m[1]*vp.y + m[2] *vp.z;
    w.y = m[4]*vp.x + m[5]*vp.y + m[6] *vp.z;
    w.z = m[8]*vp.x + m[9]*vp.y + m[10]*vp.z;
}


void gl3dView::glRenderText(double x, double y, double z, const QString & str, const QColor &textcolor, bool bBackground, bool bBold)
{
    glRenderText(Vector3d(x,y,z), str, textcolor, bBackground, bBold);
}


void gl3dView::glRenderText(Vector3d const &pos, QString const &str, fl5Color const &textcolor, bool bBackground, bool bBold)
{
    glRenderText(pos, str, xfl::fromfl5Clr(textcolor), bBackground, bBold);
}


void gl3dView::glRenderText(Vector3d const &pos, QString const &str, QColor const &textcolor, bool bBackground, bool bBold)
{
    QPoint point;
    if(pos.z>double(m_ClipPlanePos)) return;
    QVector4D v4d;
    point = worldToScreen(pos, v4d);
    point *= devicePixelRatio();

    glRenderText(point.x(), point.y(), str, textcolor, bBackground, bBold);
}


void gl3dView::glRenderText(int x, int y, QString const &str, QColor const &textcolor, bool bBackground, bool bBold)
{
    QPainter painter(&m_PixOverlay);
    painter.save();
    {
        double ratio = devicePixelRatio();

        QFont font(DisplayOptions::textFont());
        font.setPointSize(DisplayOptions::textFontStruct().pointSize()*ratio);
        font.setBold(bBold);
        painter.setFont(font);

        QRect rect;
        QRect boundingrect = painter.boundingRect(rect, Qt::AlignLeft, str);
        boundingrect.translate(x, y);

        if(bBackground)
        {
            QBrush backbrush(DisplayOptions::backgroundColor());
            painter.setBackground(backbrush);
            painter.setBackgroundMode(Qt::OpaqueMode);
        }

        painter.setPen(QPen(textcolor));
        painter.drawText(boundingrect, str);
    }
    painter.restore();
}


void gl3dView::onSaveImage()
{
    QString Filter;
    QStringList filters;
    filters << "Portable Network Graphics (*.png)";
    Filter = filters.at(0);

    QStringList loc = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    QString FileName;
    if(!loc.isEmpty()) FileName = loc.front() + QDir::separator() + "gl3dView.png";

/*    FileName = QFileDialog::getSaveFileName(this, "Save image",
                                            FileName,
                                            filters.at(0),
                                            &Filter);

    if(FileName.right(4)!=".png") FileName+= ".png"; */

//    QImage m_Img = grabFramebuffer();
//    m_Img.save(FileName);

    QOpenGLFramebufferObjectFormat fboFormat;
//        fboFormat.setInternalTextureFormat(GL_RGBA);
    fboFormat.setAttachment(QOpenGLFramebufferObject::Depth);

    QSize m_OffSize = QSize(1920, 1080);

    QOpenGLFramebufferObject *m_pfboOff = new QOpenGLFramebufferObject(m_OffSize, fboFormat);

    m_pfboOff->bind();
    {
        qDebug()<<"fbo3d:"<<m_pfboOff->isValid()<<m_pfboOff->isBound();

        // Enable blending
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);

        // Enable face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
//        glViewport(0,0,m_OffSize.width()*devicePixelRatio(), m_OffSize.height()*devicePixelRatio());


        glRenderView();

        QOpenGLContext::currentContext()->functions()->glFlush();

        QImage fb = m_pfboOff->toImage();//.convertToFormat(QImage::Format_RGB32);
        fb.save(FileName, "PNG");

    }
    m_pfboOff->release();

    delete m_pfboOff;

//    makeCurrent();
}


void gl3dView::setDebugPoints(std::vector<Vector3d> const &pts, std::vector<Vector3d> const &vecs)
{
    m_DebugPts.clear();
    m_DebugVecs.clear();

    for(Vector3d const &pt  : pts)  m_DebugPts.push_back(pt);
    for(Vector3d const &vec : vecs) m_DebugVecs.push_back(vec);
}


void gl3dView::setDebugPoints(std::vector<Vector3d> const &pts)
{
    m_DebugPts.clear();
    for(Vector3d const &pt : pts)  m_DebugPts.push_back(pt);

    m_DebugVecs.clear();
}


void gl3dView::setDebugNodes(std::vector<Node> const &nodes)
{
    m_DebugPts.clear();
    for(Vector3d const &nd : nodes)  m_DebugPts.push_back(nd);

    m_DebugVecs.clear();
}


void gl3dView::setDebugNodes(std::vector<Node> const &nodes, std::vector<Vector3d> const &vecs)
{
    m_DebugPts.clear();
    m_DebugVecs.clear();
    for(Vector3d const &nd : nodes) m_DebugPts.push_back(nd);
    for(Vector3d const &vec : vecs) m_DebugVecs.push_back(vec);
}







