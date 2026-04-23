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

#include <QFile>
#include <QPoint>
#include <QRect>
#include <QColor>
#include <QSettings>
#include <QFont>


#include <interfaces/graphs/graph/curvemodel.h>
#include <interfaces/graphs/graph/axis.h>
#include <interfaces/widgets/view/grid.h>


namespace GRAPH
{
    typedef enum{BLGRAPH, OPPGRAPH, POLARGRAPH, POPPGRAPH, WPOLARGRAPH, STABPOLARGRAPH, STABTIMEGRAPH, CPGRAPH, INVERSEGRAPH, OTHERGRAPH} enumGraphType;
    typedef enum{RESETTING, EXPANDING} enumAutoScale;
}

class Curve;

class Graph
{
    friend class GraphDlg;
    friend class GraphOptions;
    public:

        Graph();
        virtual ~Graph() = default;

        void invalidate() {m_bInitialized = false;}


        Curve * getClosestPoint(double x, double y, double &xSel, double &ySel, int &nSel);
        void resetLimits();
        void resetCurves();
        void scaleAxes(double xm, double ym[], double zoom);
        void scaleXAxis(double xm, double zoom);
        void scaleYAxis(int iy, double ym, double zoom);

        void setAutoXMinUnit(bool bAutoCurveList);
        void setAutoYMinUnit(bool bAutoCurveList);
        void setAutoXUnit();
        void setAutoYUnit(int iy);
        void setBkColor(QColor cr) {m_BkColor = cr;}

        void setAllAxesStyle(LineStyle const &ls)  {m_XAxis.setTheStyle(ls); m_YAxis[0].setTheStyle(ls); m_YAxis[1].setTheStyle(ls);}
        void setAxisStyle(AXIS::enumAxis axis, Line::enumLineStipple s, int w, QColor clr);
        void setAxisStyle(AXIS::enumAxis axis, LineStyle const &ls);
        void setAxisColor(QColor crColor) {m_XAxis.setColor(crColor); m_YAxis[0].setColor(crColor); m_YAxis[1].setColor(crColor);}
        void setAxisStipple(Line::enumLineStipple nStyle) {m_XAxis.setStipple(nStyle); m_YAxis[0].setStipple(nStyle); m_YAxis[1].setStipple(nStyle);}
        void setAxisWidth(int Width)      {m_XAxis.setWidth(Width); m_YAxis[0].setWidth(Width);  m_YAxis[1].setWidth(Width);}


        void setBorder(bool bBorder)         {m_bBorder = bBorder;}
        void setTheBorderStyle(LineStyle const &ls) {m_theBorderStyle =ls;}
        void setBorderStyle(Line::enumLineStipple s) {m_theBorderStyle.m_Stipple = s;}
        void setBorderWidth(int w)           {m_theBorderStyle.m_Width = w;}
        void setBorderColor(QColor crBorder);
        void setBorderColor(fl5Color const &crBorder);

        void setMargins(int m) {m_margin[0]=m_margin[1]=m_margin[2]=m_margin[3]=m;}
        void setMargins(int *pm) {m_margin[0]=pm[0]; m_margin[1]=pm[1]; m_margin[2]=pm[2]; m_margin[3]=pm[3];}
        void setMargins(int l, int r, int t, int b) {m_margin[0]=l; m_margin[1]=r; m_margin[2]=t; m_margin[3]=b;}
        void setLeftMargin(int m)  {m_margin[0]=m;}
        void setRightMargin(int m) {m_margin[1]=m;}
        void setTopMargin(int m)   {m_margin[2]=m;}
        void setBotMargin(int m)   {m_margin[3]=m;}
        void setYInverted(int iy, bool bInverted) {m_YAxis[iy].setInverted(bInverted);}

        void setScaleType(GRAPH::enumAutoScale type) {m_AutoScaleType = type;}
        GRAPH::enumAutoScale scaleType() const {return m_AutoScaleType;}

        void setXRange(double xmin, double xmax) {m_XAxis.setMin(xmin); m_XAxis.setMax(xmax);}
        void setYRange(int iy, double ymin, double ymax) {m_YAxis[iy].setMin(ymin); m_YAxis[iy].setMax(ymax);}


        void setXo(double f)         {m_XAxis.set0(f);}
        void setXMax(double f)       {m_XAxis.setMax(f);}
        void setXMin(double f)       {m_XAxis.setMin(f);}
        void setXMinorUnit(double f) {m_XAxis.setMinorUnit(f);}
        void setXUnit(double f)      {m_XAxis.setUnit(f);}

        void setYo(int iy, double f)         {m_YAxis[iy].set0(f);}
        void setYMax(int iy, double f)       {m_YAxis[iy].setMax(f);}
        void setYMin(int iy, double f)       {m_YAxis[iy].setMin(f);}
        void setYMinorUnit(int iy, double f) {m_YAxis[iy].setMinorUnit(f);}
        void setYUnit(int iy, double f)      {m_YAxis[iy].setUnit(f);}


        void setXMajGrid(bool state, LineStyle const &ls);
        void setYMajGrid(int iy, bool state, LineStyle const &ls);
        void setXMinGrid(bool state, bool bAutoCurveList, LineStyle const &ls, double unit = -1.0);
        void setYMinGrid(int iy, bool state, bool bAutoCurveList, LineStyle const &ls, double unit = -1.0);

        void setAuto(bool bAuto);
        void setAutoX(bool bAuto);
        void setAutoY(int iy, bool bAuto);

        void setXWindow(double x1, double x2);
        void setYWindow(int iy, double y1, double y2);

        void setTitleColor(QColor color)  {m_TitleColor  = color;}
        void setLabelColor(QColor color)  {m_LabelColor  = color;}
        void setLegendColor(QColor color) {m_LegendColor = color;}
        QColor titleColor()  const {return m_TitleColor;}
        QColor labelColor()  const {return m_LabelColor;}
        QColor legendColor() const {return m_LegendColor;}

        int leftMargin()   const {return m_margin[0];}
        int rightMargin()  const {return m_margin[1];}
        int topMargin()    const {return m_margin[2];}
        int bottomMargin() const {return m_margin[3];}

        bool hasCurve() const {return m_pCurveModel && m_pCurveModel->curveCount()>0;}

        LineStyle axisStyle(AXIS::enumAxis axis) const;
        Line::enumLineStipple axisStipple(AXIS::enumAxis axis) const;
        int axisWidth(AXIS::enumAxis axis) const;
        QColor axisColor(const AXIS::enumAxis &axis) const ;

        int xVariable() const {return m_X;}
        int yVariable(int iy) const {return m_Y[iy];}

        QString xVariableName()       const {return (m_X>=0 && m_X<m_XVarList.size()) ? m_XVarList.at(m_X) : QString();}
        QString yVariableName(int iy) const {return (m_Y[iy]>=0 && m_Y[iy]<m_YVarList.size()) ? m_YVarList.at(m_Y[iy]) : QString();}

        void setVariables(int X, int Y0, int Y1=-1);
        void setXVariable(int XVar) {m_X = XVar;}
        void setYVariable(int iy, int YVar) {m_Y[iy] = YVar;}

        double xOrigin()        const {return m_XAxis.origin();}
        double xMin()           const {return m_XAxis.axmin();}
        double xMax()           const {return m_XAxis.axmax();}
        double xUnit()          const {return m_XAxis.unit();}
        double xScale()         const {return m_XAxis.scale();}

        double y0Origin(int iy) const {return m_YAxis[iy].origin();}
        double yMin(int iy)     const {return m_YAxis[iy].axmin();}
        double yMax(int iy)     const {return m_YAxis[iy].axmax();}
        double y0Unit(int iy)   const {return m_YAxis[iy].unit();}
        double yScale(int iy)   const {return m_YAxis[iy].scale();}

        Grid const &grid()      const {return m_Grid;}

        bool xMajGrid()         const {return m_Grid.bXMajGrid();}
        bool bXMinGrid()        const {return m_Grid.bXMinGrid();}
        bool yMajGrid(int iy)   const {return m_Grid.bYMajGrid(iy);}
        bool bYMinGrid(int iy)  const {return m_Grid.bYMinGrid(iy);}

        bool bAutoX()           const {return m_XAxis.bAuto();}
        bool bAutoY(int iy)     const {return m_YAxis[iy].bAuto();}
        bool bAutoXMin()        const {return m_Grid.bXAutoMinGrid();}
        bool bAutoYMin(int iy)  const {return m_Grid.bYAutoMinGrid(iy);}
        bool hasBorder()        const {return m_bBorder;}
        bool bYInverted(int iy) const {return m_YAxis[iy].bInverted();}
        QPointF offset(int iy)  const {return m_ptOffset[iy];}

        void clientToGraph(QPointF const& pt, double *val) const;
        double clientTox(double x) const {return (x-m_ptOffset[0].x())/m_XAxis.scale();}
        double clientToy(int iy, double y) const {return (y-m_ptOffset[iy].y())/m_YAxis[iy].scale();}

        double xToClient(double x) const {return (x*m_XAxis.scale() + m_ptOffset[0].x());}
        double yToClient(int iy, double y) const {return (y*m_YAxis[iy].scale() + m_ptOffset[iy].y());}

        QPointF toClient(double x, double y) const {return QPointF(xToClient(x), yToClient(0,y));}

        void showXMajGrid(bool bShow) {m_Grid.showXMajGrid(bShow);}
        void showXMinGrid(bool bShow) {m_Grid.showXMinGrid(bShow);}

        void showYMajGrid(int iy, bool bShow) {m_Grid.showYMajGrid(iy, bShow);}
        void showYMinGrid(int iy, bool bShow) {m_Grid.showYMinGrid(iy, bShow);}

        void xMajGrid(bool &bstate, LineStyle &ls);
        void bXMinGrid(bool &bstate, bool &bAuto, LineStyle &ls, double &unit);
        void yMajGrid(int iy, bool &bstate, LineStyle &ls);
        void bYMinGrid(int iy, bool &bstate, bool &bAuto, LineStyle &ls, double &unit);

        LineStyle const &xMajGridStyle()       const {return m_Grid.xMajStyle();}
        LineStyle const &yMajGridStyle(int iy) const {return m_Grid.yMajStyle(iy);}

        LineStyle const &xMinGridStyle()       const {return m_Grid.xMinStyle();}
        LineStyle const &yMinGridStyle(int iy) const {return m_Grid.yMinStyle(iy);}

        QString xTitle()       const {return xVariableName();}
        QString yTitle(int iy) const {return yVariableName(iy);}

        QStringList const &XVariableList() {return m_XVarList;}
        QStringList const &YVariableList() {return m_YVarList;}
        void setXVarStdList(std::vector<std::string> const &XVarList);
        void setYVarStdList(std::vector<std::string> const &YVarList);
        void setXVariableList(QStringList const &XVarList) {m_XVarList=XVarList;}
        void setYVariableList(QStringList const &YVarList) {m_YVarList=YVarList;}

        void setName(QString const &GraphName) {m_Name = GraphName;}
        QString const&name() const {return m_Name;}

        void resetXLimits();
        void resetYLimits();
        void resetYLimits(int iy);

        int curveCount() const;


        QColor backgroundColor()            const {return m_BkColor;}
        QColor borderColor()                const;
        Line::enumLineStipple borderStyle() const {return m_theBorderStyle.m_Stipple;}
        int borderWidth()                   const {return m_theBorderStyle.m_Width;}
        LineStyle const &theBorderStyle()   const {return m_theBorderStyle;}


    public:
        void toClipboard();
        virtual void drawGraph(QPainter &painter, const QRectF &graphRect);
        void drawAxes(QPainter &painter) const;
        void drawYAxis(int iy, QPainter &painter) const;
        void drawCurve(int nIndex, QPainter &painter) const;
        void drawLegend(QPainter &painter, const QPointF &Place) const;
        void drawTitles(QPainter &painter, const QRectF &graphRect) const;
        void drawYTitle(int iy, QPainter &painter, const QRectF &graphRect) const;
        void drawGrids(QPainter &painter);
        void drawXMinGrid(QPainter &painter) const;
        void drawYMinGrid(int iy, QPainter &painter) const;
        void drawXGrid(QPainter &painter) const;
        void drawYGrid(int iy, QPainter &painter) const;
        void drawXLogGrid(QPainter &painter) const;
        void drawXLogMinGrid(QPainter &painter) const;
        void drawYLogGrid(int iy, QPainter &painter) const;
        void drawYLogMinGrid(int iy, QPainter &painter) const;

        void toFile(QFile &XFile, bool bCSV) const;
        void highlightPoint(QPainter &painter, const Curve *pCurve, int ref) const;

        void loadSettings(QSettings &settings);
        void saveSettings(QSettings &settings);

        void copySettings(const Graph &graph);
        void copySettings(Graph const *pGraph);
        QFont const & titleFont() const {return m_TitleFont;}
        QFont const & labelFont() const {return m_LabelFont;}
        QFont const & legendFont() const {return m_LegendFont;}

        void setTitleFont(QFont const &font) {m_TitleFont  = font;}
        void setLabelFont(QFont const &font) {m_LabelFont  = font;}
        void setLegendFont(QFont const &font) {m_LegendFont = font;}

        void clearSelection(){m_pCurveModel->clearSelection();}
        bool selectCurve(int ic)                    {return m_pCurveModel->selectCurve(ic);}
        bool selectCurve(Curve *pCurve)             {return m_pCurveModel->selectCurve(pCurve);}
        bool selectCurve(QString const & curvename) {return m_pCurveModel->selectCurve(curvename);}
        bool isCurveSelected(Curve const*pCurve) const {return m_pCurveModel->isCurveSelected(pCurve);}

        Curve * getCurve(const QPointF &point, int &ipt);


        GRAPH::enumGraphType const &graphType() {return m_GraphType;}
        void setGraphType(GRAPH::enumGraphType type) {m_GraphType = type;}

        void setGraphScales(const QRectF &graphrect);

        void setGrid(Grid const &grid) {m_Grid= grid;}

        bool setXScale(const QRectF &graphrect);
        bool setXLogScale(QRectF const &graphrect);
        void setXLnScale(bool bLog) {m_XAxis.setLogScale(bLog);}
        bool bXLogScale() const {return m_XAxis.bLogScale();}

        bool setYScale(int iYAxis, QRectF const &graphrect);
        bool setYLogScale(int iYAxis, QRectF const &graphrect);
        void setYLnScale(int iy, bool bLog) {m_YAxis[iy].setLogScale(bLog);}
        bool bYLogScale(int iy) const {return m_YAxis[iy].bLogScale();}

        double yAxisPos(int iy) const;

        // curve model

        CurveModel *curveModel() {return m_pCurveModel;}
        CurveModel const *curveModel() const {return m_pCurveModel;}
        void setCurveModel(CurveModel *pCurveModel) {m_pCurveModel = pCurveModel;}
        void deleteCurveModel();

        Curve* curve(int nIndex) const;
        Curve* curve(const QString &curveTitle, bool bFromLast=false) const;
        Curve* firstCurve() const;
        Curve* lastCurve() const;
        Curve* addCurve(AXIS::enumAxis axis=AXIS::LEFTYAXIS, bool bDarkTheme=true);
        Curve* addCurve(QString const &name, AXIS::enumAxis axis=AXIS::LEFTYAXIS, bool bDarkTheme=true);

        void deleteCurve(int index);
        void deleteCurve(Curve *pCurve);
        void deleteCurve(QString const &CurveTitle);
        void deleteCurves();

        bool isLegendVisible() const {return m_bShowLegend;}
        void setLegendVisible(bool bVisible) {m_bShowLegend=bVisible;}

        Qt::Alignment legendPosition() const {return m_LegendPosition;}
        void setLegendPosition(Qt::Alignment alignment) {m_LegendPosition = alignment;}

        Axis const &xAxis() const  {return m_XAxis;}
        Axis const &yAxis(int iy) const {return m_YAxis[iy];}


        bool hasYAxis(int iy) {if (iy==0) return true; else return hasRightAxis();}
        bool hasRightAxis() const {return m_bRightAxis && m_bRightAxisEnabled;}
        void showRightAxis(bool bShow) {m_bRightAxis=bShow;}

        void enableRightAxis(bool bEnable) {m_bRightAxisEnabled=bEnable;}
        bool isRightAxisEnabled() const {return m_bRightAxisEnabled;}

        static void setHighLighting(bool bHighLight) {s_bHighlightObject = bHighLight;}
        static bool isHighLighting() {return s_bHighlightObject;}

        static bool bMousePos() {return s_bShowMousePos;}
        static void showMousePos(bool bShow) {s_bShowMousePos=bShow;}

        static bool antiAliasing() {return s_bAntiAliasing;}
        static void setAntiAliasing(bool bAA) {s_bAntiAliasing=bAA;}

    protected:

        QString m_Name;        /** The graph's name, used for little else than to identify it in the settings file */

        GRAPH::enumGraphType m_GraphType;
        GRAPH::enumAutoScale m_AutoScaleType;

        bool m_bInitialized;
        bool m_bBorder;
        bool m_bRightAxis;         /**< if true, and if m_bRightAxisEnabled is true, the right axis is enabled */
        bool m_bRightAxisEnabled;  /**< if false, the right axis is disabled altogether for this graph; the default is false */
        bool m_bShowLegend;

        Qt::Alignment m_LegendPosition;

        QPointF m_ptOffset[2]; //in screen coordinates, w.r.t. the client area

        LineStyle m_theBorderStyle;
        int m_margin[4]; // left, right, top, bottom

        Axis m_XAxis;
        Axis m_YAxis[2]; /**< the two Y axes */

        Grid m_Grid;

        QColor m_BkColor;

        int m_X;  /** index of the currently selected left X variable */
        int m_Y[2]; /** index of the currently selected Y variables */

        QStringList m_XVarList, m_YVarList; /** the list of variable names available for this graph */

        QFont m_TitleFont;
        QFont m_LabelFont;
        QFont m_LegendFont;

        QColor m_TitleColor;
        QColor m_LabelColor;
        QColor m_LegendColor;

        CurveModel *m_pCurveModel;

        static bool s_bAntiAliasing;

        static bool s_bHighlightObject;       /**< true if the active OpPoint should be highlighted on the polar curve. */
        static bool s_bShowMousePos;

};

