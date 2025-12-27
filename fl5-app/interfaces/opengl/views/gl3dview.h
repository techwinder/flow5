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


#pragma once

#include <QElapsedTimer>
#include <QOpenGLBuffer>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>

#include <QTimer>

#include <api/linestyle.h>
#include <api/vector2d.h>
#include <api/node.h>
#include <core/fontstruct.h>
#include <interfaces/opengl/controls/arcball.h>
#include <interfaces/opengl/views/light.h>
#include <interfaces/opengl/views/shadloc.h>


#define DEPTHFACTOR 0.0f
#define DEPTHUNITS 1.0f

#define SHADOW_WIDTH 2048
#define SHADOW_HEIGHT 2048

#define GROUP_SIZE 64
#define TRACESEGS 32  // TODO: synchronize manually in the compute shader


class GLLightDlg;

class gl3dView : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

    public:
        gl3dView(QWidget *pParent = nullptr);
        virtual ~gl3dView() override;

        void saveViewPoint(Quaternion &qt) const;
        void restoreViewPoint(Quaternion const &qt);

        void setReferenceLength(double length) {m_RefLength = length;}
        double referenceLength() const {return m_RefLength;}

        float referenceScale() const {if(m_RefLength>0.0) return 1.0f/float(m_RefLength); else return 1.0f;}
        float scale() const {return m_glScalef;}
        void reset3dScale();

        void clearDebugPoints() {m_DebugPts.clear(); m_DebugVecs.clear();}
        void appendDebugPoint(Vector3d const &pt) {m_DebugPts.append(pt);}
        void appendDebugVec(Vector3d const &vec) {m_DebugVecs.append(vec);}

        void setDebugPoints(QVector<Vector3d> const &pts, QVector<Vector3d> const &vecs) {m_DebugPts=pts; m_DebugVecs=vecs;}
        void setDebugPoints(QVector<Vector3d> const &pts) {m_DebugPts=pts; m_DebugVecs.clear();}

        void setDebugPoints(std::vector<Vector3d> const &pts, std::vector<Vector3d> const &vecs);
        void setDebugPoints(std::vector<Vector3d> const &pts);
        void setDebugNodes(std::vector<Node> const &nodes);
        void setDebugNodes(std::vector<Node> const &nodes, std::vector<Vector3d> const &vecs);

        void showAxes(bool bShow) {m_bAxes=bShow;}
        bool bAxes() const {return m_bAxes;}

        ArcBall &arcBall() {return m_ArcBall;}
        void hideArcball() {m_bArcball=false; update();}

        bool bZAnimation() {return m_bZAnimate;}
        void setZAnimation(bool bZAnimate) {m_bZAnimate=bZAnimate;}

        void centerViewOn(Vector3d const &pt);
        void reset3dRotationCenter();

        void glSetupLight();

        void setAxisTitle(int iaxis, QString const &title) {m_AxisTitle[iaxis] = title;}

        void setLightVisible(bool bVisible) {m_bLightVisible=bVisible;}

        virtual bool intersectTheObject(Vector3d const &AA,  Vector3d const &BB, Vector3d &I) = 0;

        void printFormat(QSurfaceFormat const & fmt, QString &log, QString prefix="   " );

        static void setXflSurfaceFormat(QSurfaceFormat const &fmt) {s_GlSurfaceFormat = fmt;}
        static QSurfaceFormat const& defaultXflSurfaceFormat() {return s_GlSurfaceFormat;}

        static void setRenderableType(QSurfaceFormat::RenderableType type) {s_GlSurfaceFormat.setRenderableType(type);}
        static void setOGLVersion(int OGLMajor, int OGLMinor) {s_GlSurfaceFormat.setVersion(OGLMajor, OGLMinor);}
        static int oglMajor() {return s_GlSurfaceFormat.majorVersion();}
        static int oglMinor() {return s_GlSurfaceFormat.minorVersion();}

        static void setDefaultSamples(int nSamples) {s_GlSurfaceFormat.setSamples(nSamples);}
        static int defaultSamples() {return s_GlSurfaceFormat.samples();}

        static void setProfile(QSurfaceFormat::OpenGLContextProfile prof) {s_GlSurfaceFormat.setProfile(prof);}
        static QSurfaceFormat::OpenGLContextProfile defaultProfile() {return s_GlSurfaceFormat.profile();}

        static void setDeprecatedFuncs(bool bDeprecated) {s_GlSurfaceFormat.setOption(QSurfaceFormat::DeprecatedFunctions, bDeprecated);}
        static bool deprecatedFuncs() {return s_GlSurfaceFormat.testOption(QSurfaceFormat::DeprecatedFunctions);}

        static void setDebugContext(bool bDebug) {s_GlSurfaceFormat.setOption(QSurfaceFormat::DebugContext, bDebug);}
        static bool debugContext() {return s_GlSurfaceFormat.testOption(QSurfaceFormat::DebugContext);}


        static bool bAnimateTransitions() {return s_bAnimateTransitions;}
        static void setAnimationTransitions(bool bAnimate) {s_bAnimateTransitions = bAnimate;}

        static int transitionTime() {return s_AnimationTime;}
        static void setTransitionTime(int time_ms) {s_AnimationTime=time_ms;}

        static double zAnimAngle()    {return s_ZAnimAngle;}
        static void setZAnimAngle(double angle)       {s_ZAnimAngle=angle;}

        static void setLight(Light const l) {s_Light=l;}
        static Light const &light() {return s_Light;}
        static bool isLightOn() {return s_Light.m_bIsLightOn;}
        static void setLightOn(bool bLight) {s_Light.m_bIsLightOn = bLight;}
        static void setDefaultLength(double length) {s_Light.setDefaults(length);}
        static void setLightPos(double x, double y, double z) {s_Light.m_X=x; s_Light.m_Y=y; s_Light.m_Z=z;}
        static void setSpecular(double s) {s_Light.m_Specular=s;}


        static void saveSettings(QSettings &settings);
        static void loadSettings(QSettings &settings);

    signals:
        void viewModified();

    protected:
        //OVERLOADS
        virtual void initializeGL() override;
        virtual void paintGL() override;
        virtual void resizeGL(int width, int height) override;
        virtual void keyReleaseEvent(QKeyEvent *pEvent) override;
        virtual void keyPressEvent(QKeyEvent *pEvent) override;
        virtual void mouseDoubleClickEvent(QMouseEvent *pEvent) override;
        virtual void mouseMoveEvent(QMouseEvent *pEvent) override;
        virtual void mousePressEvent(QMouseEvent *pEvent) override;
        virtual void mouseReleaseEvent(QMouseEvent *pEvent) override;
        virtual void wheelEvent(QWheelEvent *pEvent) override;
        virtual QSize sizeHint() const override {return QSize(240, 280);}
        virtual QSize minimumSizeHint() const override {return QSize(150, 100);}
        virtual void glMake3dObjects();
        virtual void hideEvent(QHideEvent *pEvent) override;

    public slots:
        void onIdleAnimate();
        void on3dIso();
        void on3dFlipH();
        void on3dFlipV();
        virtual void on3dBot();
        virtual void on3dTop();
        void on3dLeft();
        void on3dRight();
        void on3dFront();
        void on3dRear();
        void on3dReset();
        void onAxes(bool bChecked) {m_bAxes = bChecked; update();}
        void onClipPlane(int pos);
        void onClipScreenPlane(bool bClip);
        void onZAnimate(bool bZAnimate);
        void onBackImageSettings();
        void onResetIncrement();
        void onRotationIncrement();
        void onTranslationIncrement();
        void onDynamicIncrement();

        void onSetupLight();
        void onLight(bool bOn);

        void onClearBackImage();
        void onLoadBackImage();
        void onUpdateImageSettings(bool bScaleWithView, bool bFlipH, bool bFlipV, QPointF const& offset, double xscale, double yscale);

        void onOglLogMsg(QOpenGLDebugMessage const & logmsg);

        void onSaveImage();

    protected:
        virtual void glRenderView();
        virtual void paintOverlay();

        void glMakeArcPoint(const ArcBall &arcball);
        void glMakeArcBall(ArcBall &arcball);
        void glMakeAxes();
        void glMakeLightSource();
        void glMakeCylinder(float h, float r, int nz, int nh);
        void glMakeIcosahedron();
        void glMakeIcoSphere(int nSplits=2);
        void glMakeUnitArrow();
        void glMakeCone(float h, float r, int nz, int nh);

        void getGLError();

        void paintPoints2(QOpenGLBuffer &vbo, float width, int stride);

        void paintDebugPts();
        double drawReferenceLength();

        void glRenderText(int x, int y, const QString & str, const QColor &textcolor=Qt::darkCyan, bool bBackground=false, bool bBold=false);
        void glRenderText(Vector3d const &pos, const QString & str, const QColor &textcolor, bool bBackground=false, bool bBold=false);
        void glRenderText(Vector3d const &pos, const QString & str, const fl5Color &textcolor, bool bBackground=false, bool bBold=false);
        void glRenderText(double x, double y, double z, const QString & str, const QColor &textcolor, bool bBackground=false, bool bBold=false);

        void paintGl3();
        void paintArcBall();
        void paintAxes();
        void paintXYCircle(QOpenGLBuffer &vbo, double xc, double yc, double radius, const QColor &circleColor);
        void paintXYCircle(QOpenGLBuffer &vbo, const Vector2d &place, double radius, const QColor &circleColor);
        void paintIcoSphere(const Vector3d &place, double radius, const fl5Color &color, bool bTriangles, bool bOutline);
        void paintIcoSphere(const Vector3d &place, double radius, const QColor &color, bool bTriangles, bool bOutline);
        void paintCube(double x, double y, double z, double side, QColor const &clr, bool bLight);
        void paintBox(double x, double y, double z, double dx, double dy, double dz, QColor const &clr, bool bLight);
        void paintSphere(float xs, float ys, float zs, float radius, const QColor &color, bool bLight=true);
        void paintSphere(const Vector3d &place, float radius, const QColor &sphereColor, bool bLight=true);
        void paintSphereInstances(QOpenGLBuffer &vboPosInstances, float radius, QColor const &clr, bool bTwoSided, bool bLight);

        void paintIcosahedron(const Vector3d &place, float radius, const QColor &color, LineStyle const &ls, bool bOutline, bool bLight);

        void paintTriangle(QOpenGLBuffer &vbo, bool bHighlight, bool bBackground, const QColor &clrBack);
        void paintQuad(const QColor &clrBack, bool bContour, float thickness, bool bHighlight, bool bBackground, bool bLight, QOpenGLBuffer &vbo);
        void paintLineStrip(QOpenGLBuffer &vbo, LineStyle const &ls);
        void paintLineStrip(QOpenGLBuffer &vbo, const QColor &clr, float width, Line::enumLineStipple stipple=Line::SOLID);
        void paintColorSegments(QOpenGLBuffer &vbo, float width, Line::enumLineStipple stipple=Line::SOLID);
        void paintColourSegments8(QOpenGLBuffer &vbo, LineStyle const &ls);
        void paintColourSegments8(QOpenGLBuffer &vbo, float width, Line::enumLineStipple stipple);

        void paintSegments(QOpenGLBuffer &vbo, LineStyle const &ls, bool bHigh = false);
        void paintSegments(QOpenGLBuffer &vbo, const QColor &clr, float thickness, Line::enumLineStipple stip=Line::SOLID, bool bHigh=false);
        void paintThickArrow(Vector3d const &origin, const Vector3d& arrow, const QColor &clr, const QMatrix4x4 &m_ModelMatrix=QMatrix4x4());
        void paintThinArrow(Vector3d const &origin, const Vector3d& arrow, LineStyle const &ls, QMatrix4x4 const ModelMatrix=QMatrix4x4());
        void paintThinArrow(Vector3d const &origin, const Vector3d& arrow, fl5Color const &clr, float w, Line::enumLineStipple stipple, QMatrix4x4 const ModelMatrix=QMatrix4x4());
        void paintThinArrow(Vector3d const &origin, const Vector3d& arrow, const QColor &clr, float w, Line::enumLineStipple stipple,   QMatrix4x4 const ModelMatrix=QMatrix4x4());
        void paintTriangles3Vtx(QOpenGLBuffer &vbo, const fl5Color &backclr, bool bTwoSided, bool bLight);
        void paintTriangles3Vtx(QOpenGLBuffer &vbo, const QColor &backclr, bool bTwoSided, bool bLight);

        void paintTriangleFan(QOpenGLBuffer &vbo, const QColor &clr, bool bLight, bool bCullFaces);

        void updateLightMatrix();
        void paintTrianglesToDepthMap(QOpenGLBuffer &vbo, const QMatrix4x4 &ModelMat, int stride);
        void paintTriangles3VtxShadow(QOpenGLBuffer &vbo, const QColor &backclr, bool bTwoSided, bool bLight, const QMatrix4x4 &modelmat, int stride);
        void paintTriangles3VtxTexture(QOpenGLBuffer &vbo, QOpenGLTexture *pTexture, bool bTwoSided, bool bLight);

        void paintTriangles3VtxOutline(QOpenGLBuffer &vbo, QColor clr, int thickness);

        void paintColourMap(QOpenGLBuffer &vbo, const QMatrix4x4 &m_ModelMatrix = QMatrix4x4());

        void paintPoints(QOpenGLBuffer &vbo, float width, int iShape, bool bLight, const fl5Color &clr, int stride);
        void paintPoints(QOpenGLBuffer &vbo, float width, int iShape, bool bLight, const QColor &clr, int stride);

        void set3dRotationCenter(const QPoint &point);

        void startResetTimer();
        void startRotationTimer();
        void startTranslationTimer(const Vector3d &PP);
        void startDynamicTimer();
        void stopDynamicTimer();

        void screenToViewport(QPoint const &point, int z, Vector3d &real) const;
        void screenToViewport(QPoint const &point, Vector3d &real) const;
        void screenToWorld(const QPoint &screenpt, int z, Vector3d &modelpt) const;
        void viewportToScreen(Vector3d const &real, QPoint &point) const;
        void viewportToWorld(Vector3d vp, Vector3d &w) const;

        QVector4D worldToViewport(const Vector3d &v) const;
        QPoint worldToScreen(const Vector3d &v, QVector4D &vScreen) const;
        QPoint worldToScreen(QVector4D v4, QVector4D &vScreen) const;


        void setAutoDeletePartBuffers(bool bAutoDelete) {m_bAutoDeleteBuffers=bAutoDelete;}

        void makeStandardBuffers();

        void reset();

        void setViewportTranslation();
        void initDepthMap();

    protected:
        QOpenGLShaderProgram m_shadSurf;
        QOpenGLShaderProgram m_shadLine;
        QOpenGLShaderProgram m_shadPoint;
        QOpenGLShaderProgram m_shadPoint2;

        ShaderLocations m_locSurf;
        ShaderLocations m_locLine;
        ShaderLocations m_locPoint;
        ShaderLocations m_locPt2;

        //shadow shader
        QOpenGLShaderProgram m_shadDepth;   /** the shader used to build the depth map */

        // class shader locations
        ShaderLocations m_locShadow;
        int m_uHasShadow;
        int m_uShadowLightViewMatrix;
        int m_uDepthLightViewMatrix;
        int m_attrDepthPos;
        uint m_fboDepthMap;
        uint m_texDepthMap;


        QMatrix4x4 m_LightViewMatrix;


        //Minimal buffers for the gl3dView
        QOpenGLBuffer m_vboArcBall, m_vboArcPoint;
        QOpenGLBuffer m_vboAxes;
        QOpenGLBuffer m_vboLightSource;
        QOpenGLBuffer m_vboCylinder, m_vboCylinderContour;
        QOpenGLBuffer m_vboCube, m_vboCubeEdges;
        QOpenGLBuffer m_vboIcosahedron, m_vboIcosahedronEdges;
        QOpenGLBuffer m_vboIcoSphere, m_vboIcoSphereEdges;
        QOpenGLBuffer m_vboCone, m_vboConeContour;
        QOpenGLBuffer m_vboThinArrow;

        bool m_bArcball;			//true if the arcball is to be displayed
        bool m_bCrossPoint;			//true if the control point on the arcball is to be displayed
        ArcBall m_ArcBall;
        float m_glScalef;
        bool m_bLightVisible;

        QRectF m_GLViewRect;    /**< The OpenGl viewport.*/

        QRectF m_rectView;

        bool m_bHasMouseMoved;
        bool m_bTrans;

        float m_ClipPlanePos;      /**< the z-position of the clip plane in viewport coordinates */
        double m_MatOut[16];

        QMatrix4x4 m_matProj, m_matView, m_matModel;

        QPoint m_LastPoint, m_PressedPoint;

        Vector3d m_TransIncrement, m_StartTranslation;
        float m_StartScale;

        Vector3d m_glViewportTrans;// the translation vector in gl viewport coordinates
        Vector3d m_glRotCenter;    // the center of rotation in object coordinates... is also the opposite of the translation vector

        QPixmap m_PixOverlay;

        QString m_ImagePath;
        bool m_bIsImageLoaded;      /**< true if a background image is loaded */
        bool m_bScaleImageWithView;
        bool m_bFlipH, m_bFlipV;
        QPixmap m_BackImage;        /**< the QPixmap object with the background image */
        QPointF m_ImageOffset;
        double m_ImageScaleX, m_ImageScaleY;

        Quaternion m_QuatStart, m_QuatEnd;

        int m_iTimerInc;

        double m_RefLength;

        bool m_bAxes;
        bool m_bAutoDeleteBuffers;

        QOpenGLVertexArrayObject m_vao; /** generic vao required for the core profile >3.x*/
        QOpenGLDebugLogger *m_pOglLogger;

        int m_nAnimationFrames;

        bool m_bZAnimate;
        QTimer m_IdleTimer; // used for idle rotation around the z-axis

        QTimer m_TransitionTimer; // used when the user has double-clicked on a location or has pressed a view icon

        QString m_AxisTitle[3]{"X", "Y", "Z"};

        QElapsedTimer m_MoveTime;
        QTimer m_DynTimer;
        Quaternion m_SpinInc;
        Vector3d m_Trans;
        bool m_bDynTranslation;
        bool m_bDynRotation;
        bool m_bDynScaling;
        float m_ZoomFactor;

        GLLightDlg *m_pglLightDlg;


        static int s_AnimationTime;
        static bool s_bAnimateTransitions;  // ms

        static double s_ZAnimAngle;

        static QSurfaceFormat s_GlSurfaceFormat;


        static Light s_Light;


    public:
        QVector<Vector3d> m_DebugPts;
        QVector<Vector3d> m_DebugVecs;

};


