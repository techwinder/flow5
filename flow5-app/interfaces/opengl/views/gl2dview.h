/****************************************************************************

    flow5
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/


#pragma once

#include <QMainWindow>
#include <QFrame>
#include <QLabel>
#include <QElapsedTimer>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include <QTimer>
#include <QPushButton>

#include <api/linestyle.h>
#include <interfaces/opengl/views/shadloc.h>
#include <api/vector2d.h>
#include <interfaces/widgets/view/grid.h>

#define MAXROOTS 5


#define DEPTHFACTOR 0.0f
#define DEPTHUNITS 1.0f


class IntEdit;

class gl2dView : public QOpenGLWidget
{
    Q_OBJECT
    public:
        gl2dView(QWidget *pParent = nullptr);
        ~gl2dView();

        void setOutputInfo(QString const &info);
        void clearOutputInfo()  {m_plabInfoOutput->clear();}

        void setReferenceLength(double length) {m_RefLength = length;}
        double referenceLength() const {return m_RefLength;}

        float referenceScale() const {if(m_RefLength>0.0) return 1.0f/float(m_RefLength); else return 1.0f;}
        void setScale(float scale)  {m_fScale = scale;}
        float scale() const {return m_fScale;}
        void reset2dScale();

        void clearDebugPoints() {m_DebugPts.clear(); m_DebugVecs.clear();}
        void appendDebugPoint(Vector2d const &pt) {m_DebugPts.append(pt);}
        void appendDebugVec(Vector2d const &vec) {m_DebugVecs.append(vec);}
        void setDebugPoints(QVector<Vector2d> const &pts, QVector<Vector2d> const &vecs) {m_DebugPts=pts; m_DebugVecs=vecs;}
        void setDebugPoints(QVector<Vector2d> const &pts) {m_DebugPts=pts; m_DebugVecs.clear();}

        void paintDebugPts();

        static void setImageSize(QSize sz) {s_ImageSize=sz;}

    protected:
        virtual QSize sizeHint() const override {return QSize(1500, 1100);}
        void showEvent(QShowEvent *pEvent) override;
        void wheelEvent(QWheelEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mouseDoubleClickEvent(QMouseEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void leaveEvent(QEvent *pEvent) override;
        void resizeGL(int width, int height)  override;
        void initializeGL() override;
        void paintGL() override;
        virtual void glMake2dObjects() {}
        virtual void glRenderView() = 0;
        virtual QPointF defaultOffset() = 0;

        virtual void paintOverlay();

        void startDynamicTimer();
        void stopDynamicTimer();
        void startResetTimer();
        void startTranslationTimer(const QPointF &PP);

        void screenToViewport(QPoint const &point, QVector2D &real) const;
        QVector2D screenToViewport(QPoint const &point) const;
        void screenToWorld(QPoint const &screenpt, QVector2D &pt) const;
        void screenToWorld(QPoint const &screenpt, Vector2d &pt) const;
        Vector2d screenToWorld(QPoint const &screenpt) const;
        QPoint worldToScreen(Vector2d const&v) const;
        QPoint worldToScreen(float xf, float yf) const;

        void paintPoints(QOpenGLBuffer &vbo, float width, int iShape, bool bLight, QColor const &clr, int stride);
        void paintPoints2(QOpenGLBuffer &vbo, float w);
        void paintSegments(QOpenGLBuffer &vbo, LineStyle const &ls, bool bHigh=false);
        void paintSegments(QOpenGLBuffer &vbo, QColor const &clr, float thickness, Line::enumLineStipple stip, bool bHigh);

        void paintLineStrip(QOpenGLBuffer &vbo, LineStyle const &ls);
        void paintLineStrip(QOpenGLBuffer &vbo, QColor const &clr, float width, Line::enumLineStipple stipple);

        void paintDisk(float xf, float yf, float rad, const QColor &clr);
        void paintDisk(Vector2d const &pos, float rad, const QColor &clr);
        void paintColourMap(QOpenGLBuffer &vbo);
        void paintTriangles3Vtx(QOpenGLBuffer &vbo, const QColor &backclr);

        void paintTriangle(QOpenGLBuffer &vbo, bool bHighlight, bool bBackground, QColor const &clrBack);

        void glRenderText(Vector2d const &pos, const QString & str, const QColor &textcolor, bool bBackground=false, bool bBold=false) {glRenderText(pos.xf(), pos.yf(), str, textcolor, bBackground, bBold);}
        void glRenderText(int x, int y, const QString & str, const QColor &backclr, const QColor &textcolor = QColor(Qt::white), bool bBold=false);
        void glRenderText(float x, float y, const QString & str, const QColor &textcolor, bool bBackground=false, bool bBold=false);

        void setAutoUnits();
        double setAutoUnit(int nTicks, float width) const;

        void drawXScale(QPainter &painter, float scale, const QPoint &origin);
        void drawYScale(QPainter &painter, float scale, const QPoint &origin);

        virtual void resizeLabels();

        void saveImage(QString const &filename, QString const &description);

    protected slots:
        void onDynamicIncrement();
        void onTranslationIncrement();
        void onResetIncrement();
        void on2dReset();
        virtual void onSaveImage() = 0;

    signals:
        void ready2d();

    protected:
        QOpenGLVertexArrayObject m_vao; /** generic vao required for the core profile >3.x*/
        QOpenGLBuffer m_vboQuad;
        QOpenGLBuffer m_vboDisk;

        QOpenGLShaderProgram m_shadPoint;
        ShaderLocations m_locPoint;

        QOpenGLShaderProgram m_shadPoint2;
        ShaderLocations m_locPt2;

        QOpenGLShaderProgram m_shadLine;
        ShaderLocations m_locLine;

        QOpenGLShaderProgram m_shadSurf;
        ShaderLocations m_locSurf;

        QMatrix4x4 m_matProj, m_matView, m_matModel;

        // shader uniforms
        int m_locViewTrans;
        int m_locViewScale;
        int m_locViewRatio;

        //shader attributes
        int m_attrVertexPosition;

        bool m_bAxes;
        Grid m_Grid;

        QPoint m_LastPoint, m_PressedPoint;
        float m_fScale;
        QPointF m_ptOffset;
        QPointF m_TransIncrement;
        float m_glScaleIncrement;


        QElapsedTimer m_MoveTime;
        QTimer m_DynTimer;

        QTimer m_TransitionTimer; // used when the user has double-clicked on a location or has pressed a view icon
        QPointF m_Trans;
        bool m_bDynTranslation;

        bool m_bDynScaling;
        float m_ZoomFactor;

        QRectF m_rectView;
        QRectF m_GLViewRect;    /**< The OpenGl viewport.*/
        double m_RefLength;


        bool m_bInitialized;

        int m_nRoots;

        int m_uHasShadow, m_uShadowLightViewMatrix;


        QPixmap m_PixOverlay;

        QFrame *m_pCmdFrame;
        QLabel *m_plabInfoOutput;

        QPushButton *m_ppbSaveImg;
        IntEdit *m_pieWidth;
        IntEdit *m_pieHeight;


        int ANIMATIONFRAMES;
        int m_iTimerInc;



        static QSize s_ImageSize;

    public:
        QVector<Vector2d> m_DebugPts;
        QVector<Vector2d> m_DebugVecs;
};


