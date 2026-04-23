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

#include <QWidget>
#include <QMenu>
#include <QPixmap>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>

#include <interfaces/widgets/view/grid.h>


/**
* @file
* @brief This file contains the declaration of the class Section2dWidget, used for 2d drawing of object line-type sections.
*/

/**
* @class Section2dWidget
* @brief This class is used for 2d drawing of object line-type sections such as Foils, body Frames, and body axial lines.
*
* This class is abstract.
* The pure virtual methods highlightPoint(), selectPoint(), dragSelectedPoint(), onInsertPt(), and onRemovePt() are dependent on the object type
* and must therefore be implemented in the derived class;
*/

class MainFrame;
class Triangle2d;
class Segment2d;
class PSLG2d;

class Section2dWt : public QWidget
{
    Q_OBJECT

    friend class SailDlg;
    friend class MainFrame;
    friend class XDirectMenus;

    public:
        Section2dWt(QWidget *parent = nullptr);

        QSize sizeHint()        const override {return QSize(350,250);}
        QSize minimumSizeHint() const override {return QSize(300,100);}

        void contextMenuEvent (QContextMenuEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void keyReleaseEvent(QKeyEvent *pEvent) override;
        void mouseDoubleClickEvent (QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void paintEvent(QPaintEvent *pEvent) override;
        void wheelEvent (QWheelEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void setContextMenu(QMenu *pMenu) {m_pSection2dContextMenu = pMenu;}
        void setAutoUnits();
        void setAutoUnit(const QRect &, double scale, double offsetX, double unitfactor, double &unit) const;
        void setUnitFactor(double unitfactor) {m_UnitFactor=unitfactor;}

        void resetDefaultScale();
        virtual void showLegend(bool bShow) {m_bShowLegend=bShow;}
        virtual bool isLegendVisible() const {return m_bShowLegend;}

        Grid const &grid() const {return m_Grid;}
        Grid &grid() {return m_Grid;}
        void setGrid(Grid const&g) {m_Grid=g;}
        void showXAxis(bool bShow) {m_Grid.showXAxis(bShow);}
        void showYAxis(bool bShow) {m_Grid.showYAxis(bShow);}
        void showXMajGrid(bool bShow) {m_Grid.showXMajGrid(bShow);}
        void showXMinGrid(bool bShow) {m_Grid.showXMinGrid(bShow);}

        void showYMajGrid(int iy, bool bShow) {m_Grid.showYMajGrid(iy,bShow);}
        void showYMinGrid(int iy, bool bShow) {m_Grid.showYMinGrid(iy,bShow);}

        void setOutputInfo(QString const &info);
        void clearOutputInfo() {m_plabInfoOutput->setText(QString());}

        void paintTriangle(QPainter &painter, const Triangle2d &t2d, bool bVertices, bool bFill);
        void paintCircumCircle(QPainter &painter, const Triangle2d &t2d);


        virtual void paint(QPainter &painter) = 0;

        static void setNPixelSelection(int npix) {s_nPixelSelection=npix;}
        static int nPixelSelection() {return s_nPixelSelection;}

        static void setAnimateTransitions(bool bAnimate) {s_bAnimateTransitions=bAnimate;}
        static bool bAnimateTransitions() {return s_bAnimateTransitions;}

        static void setSpinDamping(double damp) {s_SpinDamping=damp;}
        static double spinDamping() {return s_SpinDamping;}

        static bool bAntiAliasing() {return s_bAntiAliasing;}
        static void setAntiAliasing(bool bAA) {s_bAntiAliasing=bAA;}

        static void setHighStyle(LineStyle const &ls) {s_HighStyle=ls;}
        static LineStyle &highStyle() {return s_HighStyle;}

        static void setSelectStyle(LineStyle const &ls) {s_SelectStyle=ls;}
        static LineStyle &selectStyle() {return s_SelectStyle;}

    protected:

        QPointF mousetoReal(const QPointF &point);

        void createBaseActions();
        void createBaseContextMenu();

        void updateScaleLabel();
        void drawBackImage(QPainter &painter);

        void drawXScale(QPainter &painter);
        void drawYScale(QPainter &painter);
        void drawXGrid(QPainter &painter, QPointF const &Offset);
        void drawYGrid(QPainter &painter, QPointF const &Offset);
        void drawXMinGrid(QPainter &painter);
        void drawYMinGrid(QPainter &painter);


        void drawTriangle(QPainter &painter, Triangle2d const &t2d, int linewidth, QColor lineclr, QColor fillclr);
        void drawSegment(QPainter &painter, Segment2d const &seg2d, int linewidth, QColor lineclr);
        void drawTriangles(QPainter &painter, QVector<Triangle2d> const &t2d, int linewidth, QColor lineclr, QColor fillclr);
        void drawPSLG(QPainter &painter, const PSLG2d &s2d, bool bVertices);

        void scaleView(const QPointF &pt, double zoomFactor);
        void releaseZoom();

        void drawGrids(QPainter &painter);


        void startDynamicTimer();
        void stopDynamicTimer();

        void addDebugPoint(double x, double y) {m_DebugPts.push_back({x,y});}
        void drawDebugPts(QPainter &painter);
        void clearDebugPts() {m_DebugPts.clear();}

        virtual int highlightPoint(QPointF const &pos) {(void)pos; return -1;}
        virtual int selectPoint(QPointF const &pos) {(void)pos; return -1;}
        virtual void dragSelectedPoint(double x, double y) {(void)x; (void)y;}
        virtual QPointF defaultOffset() const = 0;
        virtual double defaultScale() const = 0;

    public slots:
        void onClearBackImage();
        void onGridSettings();
        void onLoadBackImage();
        void onBackImageSettings();
        void onDynamicIncrement();
        void onResetScales();
        void onResetXScale();
        void onResetYScale();
        void onResetIncrement();
        void onTranslationIncrement();
        void onZoomIn();
        void onZoomLess();
        void onZoomYOnly();
        void onUpdateImageSettings(bool bScaleWithView, bool bFlipH, bool bFlipV, const QPointF &offset, double xscale, double yscale);


        virtual void onInsertPt() {}
        virtual void onRemovePt() {}
        virtual void onSaveToSvg();


    signals:
        void objectModified();
        void selectedChanged(int);
        void mouseDragReleased();


    protected:
        QMenu *m_pSection2dContextMenu;
        QList<QAction*> m_ActionList;

        bool m_bZoomPlus;           /**< true if the user is in the process of zooming in by drawing a rectangle */
        bool m_bZoomYOnly;          /**< true if only the y-axis should be scaled */
        bool m_bTrans;              /**< true if the view is being dragged by the user */
        bool m_bDrag;               /**< true if a point is being dragged by the user */
        bool m_bShowLegend;         /**< true if the legend should be shown */
        bool m_bXDown;              /**< true if the 'X' key is pressed */
        bool m_bYDown;              /**< true if the 'Y' key is pressed */

        Grid m_Grid;

        double m_fScale;            /**< the current scale of the display */
        double m_fScaleY;           /**< the ratio between the  y and x scales */
        double m_fRefScale;         /**< the reference scale of the display */

        QPointF m_ptOffset;          /**< the foil's leading edge position in screen coordinates */
        QPointF m_PointDown;         /**< the screen point where the last left-click occured */
        QPointF m_LastPoint;         /**< the screen point where the cursor last was */

        QRect m_ZoomRect;           /**< the user-defined rectangle for zooming in */

        QPointF m_CursorPos;         /**< the mouse position */


        double m_UnitFactor;

        QString m_ImagePath;
        bool m_bIsImageLoaded;      /**< true if a background image is loaded */
        QPixmap m_BackImage;        /**< the QPixmap object with the background image */

        QLabel *m_plabInfoOutput;
        QLabel *m_plabScaleOutput;

        // view actions
        QAction *m_pResetXScaleAct, *m_pResetYScaleAct, *m_pResetXYScaleAct;
        QAction *m_pZoomYAct, *m_pGridAct;
        QAction *m_pLoadImage, *m_pClearImage, *m_pImageSettings;
        QAction *m_pExportToSVG;

        QAction *m_pZoomInAct, *m_pZoomLessAct;

        double m_ScaleInc;
        double m_ScaleYInc;
        int m_iTimerInc;
        QPointF m_TransIncrement;

        QVector<QPointF> m_DebugPts;

        QString m_ViewName;        

        bool m_bScaleImageWithView;
        bool m_bFlipH, m_bFlipV;

        QPointF m_ImageOffset;
        double m_ImageScaleX, m_ImageScaleY;

        QTimer *m_pTransitionTimer;
        QTimer m_ResetTimer;
        QTimer m_DynTimer;
        QElapsedTimer m_MoveTime;
        bool m_bDynTranslation;
        bool m_bDynScaling;
        QPointF m_Trans;
        double m_ZoomFactor;

        static int s_nPixelSelection; /** the tolerance in pixels within which the nearest point is selected or highlighted */
        static bool s_bAnimateTransitions;
        static double s_SpinDamping;
        static bool s_bAntiAliasing;

        static LineStyle s_HighStyle;
        static LineStyle s_SelectStyle;



};





