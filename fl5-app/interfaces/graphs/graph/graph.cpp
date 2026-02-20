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

#include <cmath>

#include <QApplication>
#include <QClipboard>

#include <QPainter>



#include <interfaces/graphs/graph/graph.h>
#include <interfaces/graphs/graph/curve.h>
#include <core/xflcore.h>
#include <core/displayoptions.h>
#include <core/saveoptions.h>
#include <interfaces/view2d/paint2d.h>

bool Graph::s_bAntiAliasing = true;

bool Graph::s_bHighlightObject = true;
bool Graph::s_bShowMousePos = true;

Graph::Graph()
{
    m_pCurveModel = nullptr;

    //Type is used to determine automatic scales
    m_AutoScaleType = GRAPH::EXPANDING;
    m_GraphType = GRAPH::OTHERGRAPH;

    m_X = 0;
    m_Y[0] = 0;
    m_Y[1] = 0;

    m_bInitialized  = false;

    resetXLimits();
    resetYLimits();

    m_ptOffset[0] = m_ptOffset[1] = QPointF(0,0);

    m_XVarList = QStringList({"x"});
    m_YVarList = QStringList({"y"});

    m_bBorder       = true;
    m_theBorderStyle.m_Stipple = Line::SOLID;
    m_theBorderStyle.m_Width = 2;

    m_bRightAxis = false;
    m_bRightAxisEnabled = false;

    m_margin[0] = 51;
    m_margin[1] = m_margin[2] = m_margin[3] = 43;

    m_bShowLegend   = false;
    m_LegendPosition = Qt::AlignTop | Qt::AlignHCenter;

    m_BkColor = QColor(255,255,255);
    m_theBorderStyle.m_Color = fl5Color(55,55,55);

    setAxisColor(QColor(200,200,200));
    m_TitleColor  = QColor(0,0,0);
    m_LabelColor  = QColor(0,0,0);
    m_LegendColor = QColor(0,0,0);

    m_Grid.setXMajColor(QColor(165,165,165));
    m_Grid.setXMinColor(QColor(205,205,205));

    m_Grid.setYMajColor(0, QColor(165,165,165));
    m_Grid.setYMinColor(0, QColor(205,205,205));
    m_Grid.setYMajColor(1, QColor(165,165,165));
    m_Grid.setYMinColor(1, QColor(205,205,205));


    m_theBorderStyle.m_Stipple = Line::SOLID;
    m_theBorderStyle.m_Width = 3;

    m_XAxis.setAxis(AXIS::XAXIS);
    m_XAxis.setStipple(Line::SOLID);
    m_XAxis.setWidth(1);

    m_YAxis[0].setAxis(AXIS::LEFTYAXIS);
    m_YAxis[0].setStipple(Line::SOLID);
    m_YAxis[0].setWidth(1);

    m_YAxis[1].setAxis(AXIS::RIGHTYAXIS);
    m_YAxis[1].setStipple(Line::SOLID);
    m_YAxis[1].setWidth(1);
}


void Graph::deleteCurveModel()
{
    if(m_pCurveModel) delete m_pCurveModel;
    m_pCurveModel = nullptr;
}


void Graph::copySettings(Graph const &graph)
{
    copySettings(&graph);
}


void Graph::copySettings(Graph const *pGraph)
{
    // avoid overwriting graph type which is permanent state
    //    m_GraphType     = pGraph->m_GraphType;

    m_AutoScaleType  = pGraph->m_AutoScaleType;

    m_XAxis.copySettings(pGraph->m_XAxis); // do not change title --> don't use copy constructor
    m_YAxis[0].copySettings(pGraph->m_YAxis[0]); // do not change title --> don't use copy constructor
    m_YAxis[1].copySettings(pGraph->m_YAxis[1]); // do not change title --> don't use copy constructor

    m_Grid           = pGraph->m_Grid;
    m_BkColor        = pGraph->m_BkColor;
    m_bBorder        = pGraph->m_bBorder;
    m_theBorderStyle = pGraph->m_theBorderStyle;

    m_TitleColor     = pGraph->m_TitleColor;
    m_LabelColor     = pGraph->m_LabelColor;
    m_LegendColor    = pGraph->m_LegendColor;

    m_bBorder        = pGraph->m_bBorder;
    for(int i=0; i<4; i++) m_margin[i] = pGraph->m_margin[i];

    m_bShowLegend    = pGraph->m_bShowLegend;
    m_LegendPosition = pGraph->m_LegendPosition;

    m_TitleFont      = pGraph->m_TitleFont;
    m_LabelFont      = pGraph->m_LabelFont;
    m_LegendFont     = pGraph->m_LegendFont;

    invalidate(); // mark for scale reset
}


void Graph::xMajGrid(bool &bstate, LineStyle &ls)
{
    bstate = m_Grid.bXMajGrid();
    ls     = m_Grid.xMajStyle();
}


void Graph::bXMinGrid(bool &state, bool &bAuto, LineStyle &ls, double &unit)
{
    state  = m_Grid.bXMinGrid();
    bAuto  = m_Grid.bXAutoMinGrid();
    ls     = m_Grid.xMinStyle();
    unit   = m_XAxis.minUnit();
}


void Graph::yMajGrid(int iy, bool &state, LineStyle &ls)
{
    state = m_Grid.bYMajGrid(iy);
    ls    = m_Grid.yMajStyle(iy);
}


void Graph::bYMinGrid(int iy, bool &state, bool &bAuto, LineStyle &ls, double &unit)
{
    state  = m_Grid.bYMinGrid(iy);
    bAuto  = m_Grid.bYAutoMinGrid(iy);
    ls     = m_Grid.yMinStyle(iy);
    unit   = m_YAxis[iy].minUnit();
}


void Graph::setAxisStyle(AXIS::enumAxis axis, LineStyle const &ls)
{
    switch(axis)
    {
        case AXIS::XAXIS:       m_XAxis.setTheStyle(ls);       break;
        case AXIS::LEFTYAXIS:   m_YAxis[0].setTheStyle(ls);    break;
        case AXIS::RIGHTYAXIS:  m_YAxis[1].setTheStyle(ls);    break;
    }
}


void Graph::setAxisStyle(AXIS::enumAxis axis, Line::enumLineStipple s, int w, QColor clr)
{
    switch(axis)
    {
        case AXIS::XAXIS:
            m_XAxis.setStipple(s);
            m_XAxis.setWidth(w);
            m_XAxis.setColor(clr);
            break;
        case AXIS::LEFTYAXIS:
            m_YAxis[0].setStipple(s);
            m_YAxis[0].setWidth(w);
            m_YAxis[0].setColor(clr);
            break;
        case AXIS::RIGHTYAXIS:
            m_YAxis[1].setStipple(s);
            m_YAxis[1].setWidth(w);
            m_YAxis[1].setColor(clr);
            break;
    }
}


void Graph::resetLimits()
{
    resetXLimits();
    resetYLimits();
}


void Graph::resetXLimits()
{
    if(!m_pCurveModel || !m_pCurveModel->hasVisibleCurve())
    {
        m_XAxis.setMin(0.0);
        m_XAxis.setMax(1.0);
    }
    else
    {
        if(m_XAxis.bAuto())
        {
            if(!m_XAxis.bLogScale())
            {
                m_XAxis.setMin(0.0);
                m_XAxis.setMax(0.0);
            }
            else
            {
                m_XAxis.setMin(0.0);
                m_XAxis.setMax(1.0);
            }
            m_XAxis.set0(0.0);
        }
    }
}


void Graph::resetYLimits()
{
    resetYLimits(0);
    resetYLimits(1);
}


void Graph::resetYLimits(int iy)
{
    if(!m_pCurveModel || !m_pCurveModel->hasVisibleCurve())
    {
        m_YAxis[iy].setMin(0.0);
        m_YAxis[iy].setMax(1.0);
    }
    else
    {
        if(m_YAxis[iy].bAuto())
        {
            if(!m_YAxis[iy].bLogScale())
            {
                m_YAxis[iy].setMin(0.0);
                m_YAxis[iy].setMax(0.0);
            }
            else
            {
                m_YAxis[iy].setMin(0.0);
                m_YAxis[iy].setMax(1.0);
             }
            m_YAxis[iy].set0(0.0);
        }
    }
}


void Graph::clientToGraph(QPointF const& pt, double *val) const
{
    val[0] = clientTox(pt.x());
    val[1] = clientToy(0, pt.y());
    val[2] = clientToy(1, pt.y());
}


void Graph::scaleAxes(double xm, double ym[], double zoom)
{
    if (zoom<0.01) zoom = 0.01;
    m_XAxis.setAutoUnit(false);

    m_XAxis.setMin(xm+(m_XAxis.axmin()-xm)*zoom);
    m_XAxis.setMax(xm+(m_XAxis.axmax()-xm)*zoom);

    for(int iy=0; iy<2; iy++)
    {
        Axis & yaxis = m_YAxis[iy];  // outsmart the debugger
        yaxis.setAutoUnit(false);

        yaxis.setMin(ym[iy]+(yaxis.axmin()-ym[iy])*zoom);
        yaxis.setMax(ym[iy]+(yaxis.axmax()-ym[iy])*zoom);
    }
}


void Graph::scaleXAxis(double xm, double zoom)
{
    if (zoom<0.01) zoom = 0.01;
    m_XAxis.setAutoUnit(false);

    m_XAxis.setMin(xm+(m_XAxis.axmin()-xm)*zoom);
    m_XAxis.setMax(xm+(m_XAxis.axmax()-xm)*zoom);
}


void Graph::scaleYAxis(int iy, double ym, double zoom)
{
    if (zoom<0.01) zoom = 0.01;
    m_YAxis[iy].setAutoUnit(false);

    m_YAxis[iy].setMin(ym+(m_YAxis[iy].axmin()-ym)*zoom);
    m_YAxis[iy].setMax(ym+(m_YAxis[iy].axmax()-ym)*zoom);
}


void Graph::setAuto(bool bAuto)
{
    m_XAxis.setAutoUnit(bAuto);
    m_YAxis[0].setAutoUnit(bAuto);
    m_YAxis[1].setAutoUnit(bAuto);
}


void Graph::setAutoX(bool bAuto)
{
    m_XAxis.setAutoUnit(bAuto);
    resetXLimits();
}


void Graph::setAutoY(int iy, bool bAuto)
{
    m_YAxis[iy].setAutoUnit(bAuto);
    resetYLimits(iy);
}


void Graph::setAutoXMinUnit(bool bAuto)
{
    m_Grid.setAutoXMinGrid(bAuto);
    if(bAuto) m_XAxis.setMinorUnit(m_XAxis.unit()/double(Axis::unitRatio()));
}


void Graph::setAutoXUnit()
{
    m_XAxis.setAutoUnit();
}


void Graph::setAutoYMinUnit(bool bAuto)
{
    m_Grid.setAutoYMinGrid(0, bAuto);
    if(bAuto) m_YAxis[0].setMinorUnit(m_YAxis[0].unit()/double(Axis::unitRatio()));
    m_Grid.setAutoYMinGrid(1, bAuto);
    if(bAuto) m_YAxis[1].setMinorUnit(m_YAxis[1].unit()/double(Axis::unitRatio()));
}


void Graph::setAutoYUnit(int iy)
{
    m_YAxis[iy].setAutoUnit();
}


LineStyle Graph::axisStyle(AXIS::enumAxis axis) const
{
    switch (axis)
    {
        case AXIS::XAXIS:         return m_XAxis.theStyle();
        case AXIS::LEFTYAXIS:     return m_YAxis[0].theStyle();
        case AXIS::RIGHTYAXIS:    return m_YAxis[1].theStyle();
    }
    return LineStyle();
}


Line::enumLineStipple Graph::axisStipple(AXIS::enumAxis axis) const
{
    switch (axis)
    {
        case AXIS::XAXIS:         return m_XAxis.stipple();
        case AXIS::LEFTYAXIS:     return m_YAxis[0].stipple();
        case AXIS::RIGHTYAXIS:    return m_YAxis[1].stipple();
    }
    return Line::SOLID;
}


int Graph::axisWidth(AXIS::enumAxis axis) const
{
    switch (axis)
    {
        case AXIS::XAXIS:         return m_XAxis.width();
        case AXIS::LEFTYAXIS:     return m_YAxis[0].width();
        case AXIS::RIGHTYAXIS:    return m_YAxis[1].width();
    }
    return 1;
}


QColor Graph::axisColor(AXIS::enumAxis const &axis) const
{
    switch (axis)
    {
        case AXIS::XAXIS:         return m_XAxis.qColor();
        case AXIS::LEFTYAXIS:     return m_YAxis[0].qColor();
        case AXIS::RIGHTYAXIS:    return m_YAxis[1].qColor();
    }
    return Qt::gray;
}


void Graph::setXWindow(double x1, double x2)
{
    m_XAxis.setAutoUnit(false);
    m_XAxis.setMin(x1);
    m_XAxis.setMax(x2);
}


void Graph::setYWindow(int iy, double y1, double y2)
{
    m_YAxis[iy].setAutoUnit(false);
    m_YAxis[iy].setMin(y1);
    m_YAxis[iy].setMax(y2);
}


void Graph::setXMajGrid(bool state, LineStyle const &ls)
{
    m_Grid.showXMajGrid(state);
    m_Grid.setXMajStyle(ls);
}


void Graph::setXMinGrid(bool state, bool bAuto, LineStyle const &ls, double unit)
{
    m_Grid.showXMinGrid(state);
    m_Grid.setAutoXMinGrid(bAuto);
    m_Grid.setXMinStyle(ls);

    if(unit>0.0) m_XAxis.setMinorUnit(unit);
}


void Graph::setYMajGrid(int iy, bool state, LineStyle const &ls)
{
    m_Grid.showYMajGrid(iy, state);
    m_Grid.setYMajStyle(iy, ls);
}


void Graph::setYMinGrid(int iy, bool state, bool bAuto, LineStyle const &ls, double unit)
{
    m_Grid.showYMinGrid(iy, state);
    m_Grid.setAutoYMinGrid(iy, bAuto);
    m_Grid.setYMinStyle(iy, ls);
    if(unit>0.0) m_YAxis[iy].setMinorUnit(unit);
}


Curve* Graph::getClosestPoint(double x, double y, double &xSel, double &ySel, int &nSel)
{
    return m_pCurveModel->getClosestPoint(x,y,xSel, ySel, nSel);
}


void Graph::drawGraph(QPainter &painter, QRectF const &graphrect)
{
    QColor color;
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing,          s_bAntiAliasing);
    painter.setRenderHint(QPainter::TextAntialiasing,      s_bAntiAliasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, s_bAntiAliasing);

    //    Draw Border
    if(m_bBorder) color = xfl::fromfl5Clr(m_theBorderStyle.m_Color);
    else          color = m_BkColor;
    QPen BorderPen(color);
    BorderPen.setStyle(xfl::getStyle(m_theBorderStyle.m_Stipple));
    BorderPen.setWidth(m_theBorderStyle.m_Width);

    painter.setPen(BorderPen);
    painter.fillRect(graphrect, m_BkColor);
    painter.drawRect(graphrect);
    if(!m_bInitialized)
    {
        setGraphScales(graphrect);
        m_bInitialized = true;
    }

    painter.setClipRect(graphrect);
    painter.setBackgroundMode(Qt::TransparentMode);

    drawGrids(painter);

    drawAxes(painter);

    if(m_pCurveModel)
    {
        for (int ic=0; ic<m_pCurveModel->curveCount(); ic++)
            drawCurve(ic, painter);

    }

    drawTitles(painter, graphrect);

    painter.setClipping(false);
    painter.restore();   
}


void Graph::drawGrids(QPainter &painter)
{

    if(m_Grid.bXMinGrid())
    {
        if(!m_XAxis.bLogScale()) drawXMinGrid(painter);
        else                     drawXLogMinGrid(painter);
    }

    if(!m_XAxis.bLogScale()) drawXGrid(painter);
    else                     drawXLogGrid(painter);

    if(!m_YAxis[0].bLogScale()) drawYMinGrid(0, painter);
    else                        drawYLogMinGrid(0, painter);

    if(!m_YAxis[0].bLogScale()) drawYGrid(0, painter);
    else                        drawYLogGrid(0, painter);

    if(m_bRightAxis)
    {
        if(!m_YAxis[1].bLogScale()) drawYMinGrid(1, painter);
        else                        drawYLogMinGrid(1, painter);

        if(!m_YAxis[1].bLogScale()) drawYGrid(1, painter);
        else                        drawYLogGrid(1, painter);
    }
}


void Graph::drawCurve(int nIndex, QPainter &painter) const
{
    Curve *pCurve = curve(nIndex);
    if(!pCurve) return;

    int iy = pCurve->isLeftAxis() ? 0 : 1;

    double scaley = m_YAxis[iy].scale();

    QPointF ptMin, ptMax;
    QRectF rViewRect;

    painter.save();

    QPen curvepen;
    curvepen.setCosmetic(true);
    curvepen.setStyle(xfl::getStyle(pCurve->stipple()));

    QPen textpen(m_LabelColor);
    textpen.setCosmetic(true);

    QBrush FillBrush(m_BkColor);
    painter.setBrush(FillBrush);

    ptMin.setX(m_XAxis.axmin()*m_XAxis.scale() + m_ptOffset[iy].x());
    ptMin.setY(m_YAxis[iy].axmin()*scaley      + m_ptOffset[iy].y());

    ptMax.setX(m_XAxis.axmax()*m_XAxis.scale() + m_ptOffset[iy].x());
    ptMax.setY(m_YAxis[iy].axmax()*scaley      + m_ptOffset[iy].y());

    rViewRect.setTopLeft(ptMin);
    rViewRect.setBottomRight(ptMax);

    QColor linecolor;
    if(pCurve->size()>=1 && pCurve->isVisible())
    {
        if(m_pCurveModel->isCurveSelected(pCurve) && isHighLighting())
        {
            linecolor = pCurve->qColor();
            curvepen.setWidth(pCurve->width()+2);
        }
        else
        {
            linecolor = pCurve->qColor();
            curvepen.setWidth(pCurve->width());
        }
        curvepen.setColor(linecolor);
        painter.setPen(curvepen);

        int i0 = 0;
        if(m_XAxis.bLogScale())
        {
            while(true)
            {
                if(i0>=pCurve->count()-1) break;
                if(pCurve->x(i0)>0.0)
                {
                    break;
                }
                i0++;
            }
        }

        if(m_YAxis[iy].bLogScale())
        {
            while(true)
            {
                if(i0>=pCurve->count()-1) break;
                if(pCurve->y(i0)>0.0)
                {
                    break;
                }
                i0++;
            }
        }

        painter.resetTransform();

        painter.translate(m_ptOffset[iy]);
        painter.scale(m_XAxis.scale(), scaley);

        if(pCurve->stipple()!=Line::NOLINE)
        {
            painter.drawPolyline(pCurve->points());
        }

        painter.resetTransform();


        if(pCurve->pointsVisible())
        {
            curvepen.setStyle(Qt::SolidLine); // no stipple for the points

            for (int ipt=0; ipt<pCurve->size();ipt++)
            {
                bool bPoint = true;
                double x=pCurve->x(ipt);
                double y=pCurve->y(ipt);
                if(m_XAxis.bLogScale())
                {
                    if(x>0.0) x= log10(x);
                    else      bPoint = false;
                }

                if(m_YAxis[iy].bLogScale())
                {
                    if(y>0.0) y= log10(y);
                    else      bPoint = false;
                }
                if(bPoint)
                {
                    painter.setPen(curvepen);
                    xfl::drawSymbol(painter, pCurve->symbol(), backgroundColor(), linecolor,
                                    x*m_XAxis.scale()    +m_ptOffset[iy].x(),
                                    y*m_YAxis[iy].scale()+m_ptOffset[iy].y());
                    if(pCurve->tagSize()>ipt && !pCurve->tag(ipt).isEmpty())
                    {
                        painter.setPen(textpen);
                        QPointF ptf(x*m_XAxis.scale()    +m_ptOffset[iy].x() + 5.0f,
                                    y*m_YAxis[iy].scale()+m_ptOffset[iy].y() - 5.0f);
                        painter.drawText(ptf, pCurve->tag(ipt));
                    }
                }
            }
        }
    }

//    if(s_bHighlightObject && m_pCurveModel->isCurveSelected(pCurve))

    if(s_bHighlightObject)
    {
        int point = pCurve->selectedPoint();
        if(point>=0)
        {
            highlightPoint(painter, pCurve, point);
        }
    }
    painter.restore();;
}


void Graph::highlightPoint(QPainter &painter, Curve const*pCurve, int ref) const
{
    if(!pCurve) return;
    if(ref<0 || ref>=pCurve->size()) return;

    int iy=0;

    painter.save();

    QPen HighlightPen(Qt::red);
    HighlightPen.setWidth(pCurve->width()+2);
    painter.setPen(HighlightPen);
    QPointF to;
    double ptside=5;
    to.setX(pCurve->x(ref)*m_XAxis.scale()    + m_ptOffset[iy].x());
    to.setY(pCurve->y(ref)*m_YAxis[0].scale() + m_ptOffset[iy].y());
    painter.drawRect(to.x()-ptside, to.y()-ptside, 2.0*ptside, 2.0*ptside);

    painter.restore();
}


void Graph::drawAxes(QPainter &painter) const
{
    double scaley = m_YAxis[0].scale();
    painter.save();

    QPen AxesPen;
    AxesPen.setColor(m_XAxis.qColor());
    AxesPen.setStyle(xfl::getStyle(m_XAxis.stipple()));
    AxesPen.setWidth(m_XAxis.width());
    painter.setPen(AxesPen);

    //horizontal axis
    double yp=0;
    if(!m_YAxis[0].bLogScale())
    {
        if(m_YAxis[0].origin()>=m_YAxis[0].axmin() && m_YAxis[0].origin()<=m_YAxis[0].axmax()) yp = m_YAxis[0].origin();
        else if(m_YAxis[0].origin()>m_YAxis[0].axmax())                                        yp = m_YAxis[0].axmax();
        else                                                                                   yp = m_YAxis[0].axmin();
    }
    else
    {
        yp = m_YAxis[0].axmin();
    }
    QPointF from(m_XAxis.axmin()*m_XAxis.scale() +m_ptOffset[0].x(), yp*scaley + m_ptOffset[0].y());
    QPointF to  (m_XAxis.axmax()*m_XAxis.scale() +m_ptOffset[0].x(), yp*scaley + m_ptOffset[0].y());
    painter.drawLine(from, to);

    drawYAxis(0, painter);
    if(m_bRightAxis) drawYAxis(1, painter);

    painter.restore();
}


void Graph::drawYAxis(int iy, QPainter &painter) const
{
    painter.save();

    QPen AxesPen;
    AxesPen.setColor(m_YAxis[iy].qColor());
    AxesPen.setStyle(xfl::getStyle(m_YAxis[iy].stipple()));
    AxesPen.setWidth(m_YAxis[iy].width());
    painter.setPen(AxesPen);

    double scaley= m_YAxis[iy].scale();
    double xp = yAxisPos(iy);

    QPointF from = QPointF(xp*m_XAxis.scale() + m_ptOffset[iy].x(), m_YAxis[iy].axmin()*scaley + m_ptOffset[iy].y());
    QPointF to   = QPointF(xp*m_XAxis.scale() + m_ptOffset[iy].x(), m_YAxis[iy].axmax()*scaley + m_ptOffset[iy].y());

    painter.drawLine(from, to);

    painter.restore();
}


void Graph::drawTitles(QPainter &painter, QRectF const &graphrect) const
{
    int XPosXTitle=0, YPosXTitle=0;
    double yp=0;
    painter.save();

    QFontMetrics fm(m_TitleFont);
    XPosXTitle = -fm.horizontalAdvance(xVariableName())/2;
    YPosXTitle = -10;

    if(!m_YAxis[0].bLogScale())
    {
        if(m_YAxis[0].origin()>=m_YAxis[0].axmin() && m_YAxis[0].origin()<=m_YAxis[0].axmax()) yp = m_YAxis[0].origin();
        else if(m_YAxis[0].origin()>m_YAxis[0].axmax())                                   yp = m_YAxis[0].axmax();
        else                                                                         yp = m_YAxis[0].axmin();
    }
    else
    {
        yp = m_YAxis[0].axmin();
    }

    QPen TitlePen(m_TitleColor);
    painter.setPen(TitlePen);

    painter.setFont(m_TitleFont);

    QPointF pos(m_XAxis.axmax()*m_XAxis.scale() + m_ptOffset[0].x() + XPosXTitle,
                yp  *m_YAxis[0].scale() + m_ptOffset[0].y() + YPosXTitle);
    painter.drawText(pos, xVariableName());

    drawYTitle(0, painter, graphrect);
    if(m_bRightAxis) drawYTitle(1, painter, graphrect);

    painter.restore();
}


void Graph::drawYTitle(int iy, QPainter &painter, QRectF const &graphRect) const
{
    //draws the x & y axis name
    int XPosYTitle=0, YPosYTitle=0;
    painter.save();

    XPosYTitle = -5;
    YPosYTitle =  5;

    QPen TitlePen(m_TitleColor);
    painter.setPen(TitlePen);
    painter.setFont(m_TitleFont);

    double xp = yAxisPos(iy);

    QPointF pos0( m_ptOffset[iy].x() + xp*m_XAxis.scale() + XPosYTitle, graphRect.top() + m_margin[2] - YPosYTitle);
    //    painter.drawText(pos0, m_YAxis[iy].m_Title);
    painter.drawText(pos0, yVariableName(iy));

    painter.restore();
}


void Graph::drawXGrid(QPainter &painter) const
{
    double main(0), xt(0), yp(0);
    int exp(0), nx(0);

    int iy(0); // use left axis

    painter.save();
    QString strLabel, strLabelExp;
    QString format = xfl::isLocalized() ? "%L1" : "%1";
    QFontMetrics fm(m_LabelFont);

    painter.setFont(m_LabelFont);
    QFont expfont(m_LabelFont);
    expfont.setPointSize(expfont.pointSize()-2);

    double ticksize(5);
    double height(double(fm.height())/2);
    double yExpOff(height/2);

    QPen gridpen(m_Grid.xMajColor());
    gridpen.setStyle(xfl::getStyle(m_Grid.xMajStipple()));
    gridpen.setWidth(m_Grid.xMajWidth());
    painter.setPen(gridpen);

    QPen labelpen(m_XAxis.qColor());
    labelpen.setStyle(xfl::getStyle(m_XAxis.stipple()));
    labelpen.setWidth(m_XAxis.width());
    painter.setPen(labelpen);

    nx = int((m_XAxis.origin()-m_XAxis.axmin())/m_XAxis.unit());
    xt = m_XAxis.origin() - nx*m_XAxis.unit();

    if(!m_YAxis[0].bLogScale())
    {
        if(m_YAxis[0].origin()>=m_YAxis[0].axmin() && m_YAxis[0].origin()<=m_YAxis[0].axmax()) yp = m_YAxis[0].origin();
        else if(m_YAxis[0].origin()>m_YAxis[0].axmax())                                        yp = m_YAxis[0].axmax();
        else                                                                                   yp = m_YAxis[0].axmin();
    }
    else yp = m_YAxis[0].axmin();

    double Y_min = m_YAxis[0].axmin()*m_YAxis[0].scale() + m_ptOffset[iy].y();
    double Y_max = m_YAxis[0].axmax()*m_YAxis[0].scale() + m_ptOffset[iy].y();

    int iTick(0);
    double xtp(0), ytp(0);
    while(xt<=m_XAxis.axmax()*1.0001 && iTick<100)
    {
        if(fabs(xt-m_YAxis[0].origin())<1.e-6)
        {
            xt += m_XAxis.unit();
            iTick++;
            continue; //don't overwrite the Y-axis
        }

        xtp = xt*m_XAxis.scale();
        ytp = yp*m_YAxis[0].scale();

        painter.setPen(labelpen);
        painter.drawLine(int(xtp + m_ptOffset[iy].x()), int(ytp +ticksize + m_ptOffset[iy].y()),
                         int(xtp + m_ptOffset[iy].x()), int(ytp           + m_ptOffset[iy].y()));

        if(m_Grid.bXMajGrid() && (m_bRightAxis || iTick!=nx))
        {
            painter.setPen(gridpen);
            painter.drawLine(int(xtp + m_ptOffset[iy].x()), int(ytp +ticksize + m_ptOffset[iy].y()),
                             int(xtp + m_ptOffset[iy].x()), int(ytp           + m_ptOffset[iy].y()));
            painter.drawLine(int(xtp + m_ptOffset[iy].x()), Y_min, int(xtp + m_ptOffset[iy].x()), Y_max);
        }

        painter.setPen(m_LabelColor);
        painter.setFont(m_LabelFont);
        if(iTick==nx)
        {
            // plain 0 at the origin
            painter.drawText(int(xtp - double(fm.averageCharWidth())/2.0 + m_ptOffset[iy].x()),
                             int(ytp + ticksize*2 +height                + m_ptOffset[iy].y()),
                             "0");
        }
        else if(m_XAxis.exponent()>=5 || m_XAxis.exponent()<=-5)
        {
            main = xt;
            xfl::expFormat(main, exp);

            strLabel = QString(format+" 10").arg(main,5,'f',2);
            double w=0;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
            w = double(fm.horizontalAdvance(strLabel));
#else
            w = fm.width(strLabel);
#endif
            painter.drawText(int(xtp - w/2                + m_ptOffset[iy].x()),
                             int(ytp + ticksize*2 +height + m_ptOffset[iy].y()),
                             strLabel);
            strLabelExp = QString(format).arg(exp);
            painter.setFont(expfont);

            painter.drawText(int(xtp + w/2                        + m_ptOffset[iy].x()),
                             int(ytp + ticksize*2 +height-yExpOff + m_ptOffset[iy].y()),
                             strLabelExp);
        }
        else
        {
            if      (m_XAxis.exponent()>0)   strLabel = QString(format).arg(xt, 6, 'f', 0);
            else if (m_XAxis.exponent()>=-1) strLabel = QString(format).arg(xt, 6, 'f', 1);
            else if (m_XAxis.exponent()>=-2) strLabel = QString(format).arg(xt, 6, 'f', 2);
            else if (m_XAxis.exponent()>=-3) strLabel = QString(format).arg(xt, 6, 'f', 3);
            else if (m_XAxis.exponent()>=-4) strLabel = QString(format).arg(xt, 7, 'f', 4);

            double w=0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
            w = double(fm.horizontalAdvance(strLabel));
#else
            w = fm.width(strLabel);
#endif
            painter.drawText(int(xtp - w/2                + m_ptOffset[iy].x()),
                             int(ytp + ticksize*2 +height + m_ptOffset[iy].y()),
                             strLabel);
        }

        xt += m_XAxis.unit();
        iTick++;
    }

    painter.restore();
}


void Graph::drawXLogGrid(QPainter &painter) const
{
    double main=0, yp=0;
    int exp=0;

    int iy=0; // use left axis

    painter.save();
    QString strLabel, strLabelExp;

    QFontMetrics fm(m_LabelFont);

    painter.setFont(m_LabelFont);
    QFont expfont(m_LabelFont);
    expfont.setPointSize(expfont.pointSize()-2);

    int ticksize = 5;
    int height  = fm.height()/2;
    int yExpOff = height/2;

    QPen gridpen(m_Grid.xMajColor());
    gridpen.setStyle(xfl::getStyle(m_Grid.xMajStipple()));
    gridpen.setWidth(m_Grid.xMajWidth());
    painter.setPen(gridpen);

    QPen labelpen(m_XAxis.qColor());
    labelpen.setStyle(xfl::getStyle(m_XAxis.stipple()));
    labelpen.setWidth(m_XAxis.width());
    painter.setPen(labelpen);

    //    xt = m_XAxis.origin()-(m_XAxis.origin()-m_XAxis.axmin());//one tick at the origin

    int nx = int((m_XAxis.origin()-m_XAxis.axmin())/m_XAxis.unit());
    double xt = m_XAxis.origin() - nx*m_XAxis.unit();


    if(!m_YAxis[0].bLogScale())
    {
        if(m_YAxis[0].origin()>=m_YAxis[0].axmin() && m_YAxis[0].origin()<=m_YAxis[0].axmax()) yp = m_YAxis[0].origin();
        else if(m_YAxis[0].origin()>m_YAxis[0].axmax())                                 yp = m_YAxis[0].axmax();
        else                                                                     yp = m_YAxis[0].axmin();
    }
    else
    {
        yp = m_YAxis[0].axmin();
    }

    int Y_min = int(m_YAxis[0].axmin()*m_YAxis[0].scale() + m_ptOffset[iy].y());
    int Y_max = int(m_YAxis[0].axmax()*m_YAxis[0].scale() + m_ptOffset[iy].y());

    int nTicks = int(round(m_XAxis.axmax()-m_XAxis.axmin())+1.0);
    int iTick = 0;
    while(iTick<nTicks)
    {
        int xtp = int(xt*m_XAxis.scale());
        int ytp = int(yp*m_YAxis[0].scale());

        if(xt<=m_XAxis.axmax()+1.e-10)
        {
            // draw the ticks
            painter.setPen(labelpen);
            painter.drawLine(xtp + int(m_ptOffset[iy].x()), ytp +ticksize + int(m_ptOffset[iy].y()),
                             xtp + int(m_ptOffset[iy].x()), ytp           + int(m_ptOffset[iy].y()));

            // draw the labels
            painter.setPen(m_LabelColor);
            painter.setFont(m_LabelFont);

            main = pow(10, int(xt));
            xfl::expFormat(main, exp);

            strLabel = "10";
            int w=0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
            w= fm.horizontalAdvance(strLabel);
#else
            w = fm.width(strLabel);
#endif
            painter.drawText(xtp - w/2  +int(m_ptOffset[iy].x()),
                             ytp + ticksize*2 +height    +int(m_ptOffset[iy].y()),
                             strLabel);
            strLabelExp = QString::asprintf("%d",exp);
            painter.setFont(expfont);

            painter.drawText(xtp + w/2       +int(m_ptOffset[iy].x()),
                             ytp + ticksize*2 +height-yExpOff +int(m_ptOffset[iy].y()),
                             strLabelExp);


            // draw the grid
            if(m_Grid.bXMajGrid() && xt>m_XAxis.axmin()+1.e-10)
            {
                painter.setPen(gridpen);
                painter.drawLine(xtp + int(m_ptOffset[iy].x()), ytp +ticksize + int(m_ptOffset[iy].y()),
                                 xtp + int(m_ptOffset[iy].x()), ytp           + int(m_ptOffset[iy].y()));
                painter.drawLine(xtp + int(m_ptOffset[iy].x()), Y_min, xtp + int(m_ptOffset[iy].x()), Y_max);
            }

        }

        xt+=1.0;
        iTick++;
    }

    painter.restore();
}


double Graph::yAxisPos(int iy) const
{
    double xp=0;
    if(iy==0)
    {
        //left vertical axis
        if(m_XAxis.bLogScale() || m_bRightAxis)
        {
            xp = m_XAxis.axmin();
        }
        else
        {
            if(m_XAxis.origin()>=m_XAxis.axmin() && m_XAxis.origin()<=m_XAxis.axmax()) xp = m_XAxis.origin();
            else if(m_XAxis.origin()>m_XAxis.axmax())                             xp = m_XAxis.axmax();
            else                                                             xp = m_XAxis.axmin();
        }
    }
    else if (iy==1)
    {
        xp = m_XAxis.axmax();
    }
    return xp;
}


void Graph::drawYGrid(int iy, QPainter &painter) const
{
    double main=0;
    int exp=0;

    if(fabs(m_YAxis[iy].axmax()-m_YAxis[iy].axmin() )/m_YAxis[iy].unit()>30.0) return;
    double scaley = m_YAxis[iy].scale();
    painter.save();

    QString strLabel, strLabelExp;
    QString format = xfl::isLocalized() ? "%L1" : "%1";

    QFontMetrics fm(m_LabelFont);
    painter.setFont(m_LabelFont);
    QFont expfont(m_LabelFont);
    expfont.setPointSize(expfont.pointSize()-2);

    double fmheight  = double(fm.height());
    double fmheight4 = fmheight/4;
    double yExpOff = fmheight/2;

    double ticksize = 5;

    QPen labelpen(m_YAxis[iy].qColor());
    labelpen.setStyle(xfl::getStyle(m_YAxis[iy].stipple()));
    labelpen.setWidth(m_YAxis[iy].width());

    double X_min = m_XAxis.axmin()*m_XAxis.scale() + m_ptOffset[iy].x();
    double X_max = m_XAxis.axmax()*m_XAxis.scale() + m_ptOffset[iy].x();

    QPen gridpen(m_Grid.yMajColor(iy));
    gridpen.setStyle(xfl::getStyle(m_Grid.yMajStipple(iy)));
    gridpen.setWidth(m_Grid.yMajWidth(iy));
    painter.setPen(gridpen);

    double xp = yAxisPos(iy);

    int ny = int((m_YAxis[iy].origin()-m_YAxis[iy].axmin())/m_YAxis[iy].unit());
    double yt = m_YAxis[iy].origin() - ny*m_YAxis[iy].unit();

    int iTick=0;

    int yd = -1; // slight offset for y-labels to avoid having minus sign on grid lign

    double xtp = int(xp*m_XAxis.scale()); // the reference x-coordinate
    double sign= (iy==0) ? 1.0 : -1.0;

    if(iy==0)
        xtp -= ticksize;

    while(yt<=m_YAxis[iy].axmax()*1.0001 && iTick<100)
    {
        if(fabs(yt-m_XAxis.origin())<1.e-6)
        {
            yt += m_YAxis[iy].unit();
            iTick++;
            continue; //don't overwrite the X-axis
        }

        painter.setFont(m_LabelFont);
        painter.setPen(labelpen);
        double ytp = yt*scaley;
        painter.drawLine(int(xtp          + m_ptOffset[iy].x()), int(ytp + m_ptOffset[iy].y()),
                         int(xtp+ticksize + m_ptOffset[iy].x()), int(ytp + m_ptOffset[iy].y()));

        painter.setPen(m_LabelColor);

        if(iTick==ny)
        {
            painter.drawText(int(xtp - (double(fm.averageCharWidth())+ticksize*1)*sign +m_ptOffset[iy].x()),
                             int(ytp + fmheight4 +m_ptOffset[iy].y()),
                             "0");
        }
        else if(abs(m_YAxis[iy].exponent())>=4)
        {
            main = yt;
            xfl::expFormat(main, exp);

            strLabel = QString(format+" 10").arg(main, 5, 'f', 2);
//            strLabel = QString::asprintf("%5.2f 10",main);
            strLabelExp = QString::asprintf("%d",exp);
            int l0=0,l1=0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
            l0 = fm.horizontalAdvance(strLabel);
            l1 = fm.horizontalAdvance(strLabelExp);
#else
            l0 = fm.width(strLabel);
            l1 = fm.width(strLabelExp);
#endif

            if(iy==0)
            {
                painter.drawText(int(xtp +(-l0-l1 -ticksize*0) + m_ptOffset[iy].x()),
                                 int(ytp + fmheight4 +yd       + m_ptOffset[iy].y()),
                                 strLabel);
            }
            else
            {
                painter.drawText(int(xtp + double(fm.averageCharWidth())/3.0    + m_ptOffset[iy].x()),
                                 int(ytp + fmheight4 + yd - fm.height()*2.0/3.0 + m_ptOffset[iy].y()),
                                 l0, fm.height(),
                                 Qt::AlignLeft,
                                 strLabel);
            }

            painter.setFont(expfont);

            if(iy==0)
            {
                painter.drawText(int(xtp +(-l1 -ticksize*0)        + m_ptOffset[iy].x()),
                                 int(ytp + fmheight4 +yd  -yExpOff + m_ptOffset[iy].y()),
                                 strLabelExp);
            }
            else
            {
                if(m_YAxis[iy].exponent()>=4)
                {
                    painter.drawText(int(xtp + fm.averageCharWidth()/3 +l0 + m_ptOffset[iy].x()),
                                     int(ytp + yd  - yExpOff               + m_ptOffset[iy].y()),
                                     l1, fm.height(),
                                     Qt::AlignLeft,
                                     strLabelExp);
                }
                else
                {
                    painter.drawText(int(xtp + fm.averageCharWidth()/3 +l0 + m_ptOffset[iy].x()),
                                     int(ytp + yd  - yExpOff               + m_ptOffset[iy].y()),
                                     l1, fm.height(),
                                     Qt::AlignLeft,
                                     strLabelExp);
                }
            }
        }
        else
        {
/*            if      (m_YAxis[iy].exponent()>= 0) strLabel = QString::asprintf("%.0f",yt);
            else if (m_YAxis[iy].exponent()>=-1) strLabel = QString::asprintf("%.1f",yt);
            else if (m_YAxis[iy].exponent()>=-2) strLabel = QString::asprintf("%.2f",yt);
            else if (m_YAxis[iy].exponent()>=-3) strLabel = QString::asprintf("%.3f",yt);*/
            if      (m_YAxis[iy].exponent()>= 0) strLabel = QString(format).arg(yt,0,'f',0);
            else if (m_YAxis[iy].exponent()>=-1) strLabel = QString(format).arg(yt,0,'f',1);
            else if (m_YAxis[iy].exponent()>=-2) strLabel = QString(format).arg(yt,0,'f',2);
            else if (m_YAxis[iy].exponent()>=-3) strLabel = QString(format).arg(yt,0,'f',3);
            int w=0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
            w = fm.horizontalAdvance(strLabel);
#else
            w = fm.width(strLabel);
#endif
            if(iy==0)
            {

                painter.drawText(int(xtp - w-ticksize*1                      + m_ptOffset[iy].x()),
                                 int(ytp + fmheight4 + yd  - fm.height()*2/3 + m_ptOffset[iy].y()),
                                 w, fm.height(),
                                 Qt::AlignRight,
                                 strLabel);
            }
            else
            {
                painter.drawText(int(xtp + ticksize*1 + fm.averageCharWidth() + m_ptOffset[iy].x()),
                                 int(ytp + fmheight4 + yd - fm.height()*2/3   + m_ptOffset[iy].y()),
                                 w, fm.height(),
                                 Qt::AlignLeft,
                                 strLabel);
            }
        }

        // draw grid
        if(m_Grid.bYMajGrid(iy) && fabs(yt-m_YAxis[iy].origin())>1.0e-10)
        {
            painter.setPen(gridpen);
            painter.drawLine(X_min, int(ytp+m_ptOffset[iy].y()),
                             X_max, int(ytp+m_ptOffset[iy].y()));
        }

        yt += m_YAxis[iy].unit();
        iTick++;
    }
    painter.restore();
}


void Graph::drawYLogGrid(int iy, QPainter &painter) const
{
    double main=0, xp=0;
    int exp=0;

    double scaley = m_YAxis[iy].scale();

    QFontMetrics fm(m_LabelFont);
    int fmheight  = fm.height();
    int fmheight4 = fmheight/4;
    //    if(fabs(m_Y0Axis.axmax()-m_Y0Axis.axmin())/m_Y0Axis.unit()>30.0) return;

    painter.save();
    QString strLabel, strLabelExp;


    QFont expfont(m_LabelFont);
    expfont.setPointSize(expfont.pointSize()-2);

    int ticksize = 5;

    QPen gridpen(m_Grid.xMajColor());
    gridpen.setStyle(xfl::getStyle(m_Grid.xMajStipple()));
    gridpen.setWidth(m_Grid.xMajWidth());
    painter.setPen(gridpen);

    QPen labelpen(xfl::fromfl5Clr(m_YAxis[iy].theStyle().m_Color));
    labelpen.setStyle(xfl::getStyle(m_YAxis[iy].stipple()));
    labelpen.setWidth(m_YAxis[iy].width());
    painter.setPen(labelpen);

    //    xt = m_Y0Axis.origin()-(m_Y0Axis.origin()-m_Y0Axis.axmin());//one tick at the origin

    int nx = int((m_YAxis[iy].origin()-m_YAxis[iy].axmin())/m_YAxis[iy].unit());
    double yt = m_YAxis[iy].origin() - nx*m_YAxis[iy].unit();

    if(!m_XAxis.bLogScale())
    {
        if(m_XAxis.origin()>=m_XAxis.axmin() && m_XAxis.origin()<=m_XAxis.axmax()) xp = m_XAxis.origin();
        else if(m_XAxis.origin()>m_XAxis.axmax())                             xp = m_XAxis.axmax();
        else                                                             xp = m_XAxis.axmin();
    }
    else xp = m_XAxis.axmin();

    int X_min = int(m_XAxis.axmin()*m_XAxis.scale() + m_ptOffset[iy].x());
    int X_max = int(m_XAxis.axmax()*m_XAxis.scale() + m_ptOffset[iy].x());

    int nTicks = int(round(m_YAxis[iy].axmax()-m_YAxis[iy].axmin())+1);
    int iTick = 0;
    while(iTick<nTicks)
    {
        int xtp = int(xp*m_XAxis.scale());
        int ytp = int(yt*scaley);

        if(yt<=m_YAxis[iy].axmax()+1.e-10)
        {
            // draw tick
            painter.drawLine(xtp          + int(m_ptOffset[iy].x()), ytp + int(m_ptOffset[iy].y()),
                             xtp-ticksize + int(m_ptOffset[iy].x()), ytp + int(m_ptOffset[iy].y()));

            // draw label
            painter.setPen(labelpen);
            main = pow(10, int(yt));
            xfl::expFormat(main, exp);

            strLabel = "10";
            strLabelExp = QString::asprintf("%d",exp);

            int l0=0, l1=0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
            l0 = fm.horizontalAdvance(strLabel);
            l1 = fm.horizontalAdvance(strLabelExp);
#else
            l0 = fm.width(strLabel);
            l1 = fm.width(strLabelExp);
#endif
            painter.setFont(m_LabelFont);

            painter.drawText(xtp  -l0-l1 -ticksize*1 + int(m_ptOffset[iy].x()),
                             ytp + fmheight4         + int(m_ptOffset[iy].y()),
                             strLabel);

            painter.setFont(expfont);
            if(m_YAxis[iy].exponent()>=4)
            {
                painter.drawText(xtp -l1  - ticksize*1 + int(m_ptOffset[iy].x()),
                                 ytp                   + int(m_ptOffset[iy].y()),
                                 strLabelExp);
            }
            else
            {
                painter.drawText(xtp -l1  - ticksize*1 + 2 + int(m_ptOffset[iy].x()),
                                 ytp                   + int(m_ptOffset[iy].y()),
                                 strLabelExp);
            }

            // draw grid
            if(m_Grid.bYMajGrid(iy) && yt>m_YAxis[iy].axmin()+1.e-10)
            {
                painter.setPen(gridpen);
                painter.drawLine(X_min, ytp+int(m_ptOffset[iy].y()), X_max, ytp+int(m_ptOffset[iy].y()));
            }

        }
        yt+=1.0;
        iTick++;
    }

    painter.restore();
}


void Graph::drawXMinGrid(QPainter &painter) const
{
    int iy=0; // use left axis

    double scaley = m_YAxis[0].scale();
    if(fabs(m_XAxis.unit())<0.00000001) return;
    if(fabs(m_XAxis.minUnit())<0.00000001) return;
    if(fabs(m_XAxis.axmax()-m_XAxis.axmin())/m_XAxis.unit()>30.0) return;
    if(fabs(m_XAxis.axmax()-m_XAxis.axmin())/m_XAxis.minUnit()>100.0) return;

    painter.save();
    QPen GridPen(m_Grid.xMinColor());
    GridPen.setStyle(xfl::getStyle(m_Grid.xMinStipple()));
    GridPen.setWidth(m_Grid.xMinWidth());
    painter.setPen(GridPen);

    int Y_min = int(m_YAxis[0].axmin()*scaley+m_ptOffset[iy].y());
    int Y_max = int(m_YAxis[0].axmax()*scaley+m_ptOffset[iy].y());

    double xDelta = m_XAxis.minUnit();
    int nx = int((m_XAxis.origin()-m_XAxis.axmin())/m_XAxis.unit()) +1;
    double xt = m_XAxis.origin() - nx*m_XAxis.unit();
    //    double xt = m_XAxis.origin()-int((m_XAxis.origin()-m_XAxis.axmin())*1.0001/xDelta)*xDelta;//one tick at the origin
    int iter=0;

    while(xt<=m_XAxis.axmax()*1.001 && iter<500)
    {
        if(xt>=m_XAxis.axmin())
        {
            if(iter%Axis::unitRatio()!=0)  // don't draw over major grid
                painter.drawLine(int(xt*m_XAxis.scale()+m_ptOffset[iy].x()), Y_min, int(xt*m_XAxis.scale()+m_ptOffset[iy].x()), Y_max);
        }
        xt += xDelta;
        iter++;
    }
    painter.restore();
}


void Graph::drawYMinGrid(int iy, QPainter &painter) const
{
    if(!m_Grid.bYMinGrid(iy)) return;

    double scaley = m_YAxis[0].scale();
    if(fabs(m_YAxis[0].unit())<0.00000001) return;
    if(fabs(m_YAxis[0].minUnit())<0.00000001) return;
    if(fabs(m_YAxis[0].axmax()-m_YAxis[0].axmin() )/m_YAxis[0].unit()>30.0) return;
    if(fabs(m_YAxis[0].axmax()-m_YAxis[0].axmin() )/m_YAxis[0].minUnit()>100.0) return;

    painter.save();
    QPen GridPen(m_Grid.yMinColor(0));
    GridPen.setStyle(xfl::getStyle(m_Grid.yMinStipple(0)));
    GridPen.setWidth(m_Grid.yMinWidth(0));
    painter.setPen(GridPen);

    double yDelta = m_YAxis[0].minUnit();

    int ny = int((m_YAxis[iy].origin()-m_YAxis[iy].axmin())/m_YAxis[iy].unit()) + 1;
    double yt = m_YAxis[iy].origin() - ny*m_YAxis[iy].unit();

    int X_min = int(m_XAxis.axmin()*m_XAxis.scale() + m_ptOffset[iy].x());
    int X_max = int(m_XAxis.axmax()*m_XAxis.scale() + m_ptOffset[iy].x());

    int iter = 0;
    while(yt<=m_YAxis[0].axmax()*1.0001 && iter<500)
    {
        if(yt>=m_YAxis[0].axmin())
        {
            if(iter%Axis::unitRatio()!=0)  // don't draw over major grid
                painter.drawLine(X_min, int(yt*scaley+int(m_ptOffset[iy].y())), X_max, int(yt*scaley+m_ptOffset[iy].y()));
        }
        yt += yDelta;
        iter++;
    }
    painter.restore();
}


void Graph::drawXLogMinGrid(QPainter &painter) const
{
    painter.save();

    int iy=0; // use left axis

    QPen GridPen(m_Grid.xMinColor());
    GridPen.setStyle(xfl::getStyle(m_Grid.xMinStipple()));
    GridPen.setWidth(m_Grid.xMinWidth());
    painter.setPen(GridPen);

    double scaley = m_YAxis[0].scale();
    int Y_min = int(m_YAxis[0].axmin()*scaley+m_ptOffset[iy].y());
    int Y_max = int(m_YAxis[0].axmax()*scaley+m_ptOffset[iy].y());

    for(int iu=int(floor(m_XAxis.axmin())); iu<m_XAxis.axmax(); iu++)
    {
        double xt = double(iu);
        for(double dj=1.0; dj<9.5; dj+=1.0)
        {
            double xm = log10(dj);
            if(xt+xm>m_XAxis.axmin() && xt+xm<m_XAxis.axmax())
            {
                painter.drawLine(int((xt+xm)*m_XAxis.scale()+m_ptOffset[iy].x()), Y_min,
                                 int((xt+xm)*m_XAxis.scale()+m_ptOffset[iy].x()), Y_max);
            }
        }
        if(iu>500) break; // error somewhere
    }
    painter.restore();
}


void Graph::drawYLogMinGrid(int iy, QPainter &painter) const
{
    if(!m_Grid.bYMinGrid(iy)) return;

    painter.save();

    QPen GridPen(m_Grid.yMinColor(0));
    GridPen.setStyle(xfl::getStyle(m_Grid.yMinStipple(0)));
    GridPen.setWidth(m_Grid.yMinWidth(0));
    painter.setPen(GridPen);

    double scalex = m_XAxis.scale();
    int X_min = int(m_XAxis.axmin()/scalex+m_ptOffset[iy].x());
    int X_max = int(m_XAxis.axmax()/scalex+m_ptOffset[iy].x());

    for(int iu=int(floor(m_YAxis[0].axmin())); iu<m_YAxis[0].axmax(); iu++)
    {
        double yt = double(iu);
        for(double dj=1.0; dj<9.5; dj+=1.0)
        {
            double ym = log10(dj);
            if(yt+ym>m_XAxis.axmin() && yt+ym<m_XAxis.axmax())
            {
                painter.drawLine(X_min, int((yt+ym)*m_YAxis[0].scale()+m_ptOffset[iy].y()),
                                 X_max, int((yt+ym)*m_YAxis[0].scale()+m_ptOffset[iy].y()));
            }
        }
        if(iu>500) break; // error somewhere
    }
    painter.restore();
}


void Graph::drawLegend(QPainter &painter, QPointF const &Place) const
{
    if(!m_bShowLegend) return;
    painter.save();

    QString strong;

    QFontMetrics fm(m_LegendFont);
    int fmh = int ((fm.height()*11)/10);
    int LegendSize = fm.averageCharWidth()*7;

    painter.setFont(m_LegendFont);

    QColor linecolor;
    QPen TextPen(m_LegendColor);
    QPen LegendPen(Qt::gray);
    QBrush LegendBrush(m_BkColor);
    painter.setBrush(LegendBrush);

    int npos = 0;
    for (int nc=0; nc<m_pCurveModel->curveCount(); nc++)
    {
        Curve const*pCurve = curve(nc);
        if(pCurve->isVisible())
        {
            strong = pCurve->name();

            if(pCurve->size()>0 && strong.length())//is there anything to draw?
            {
                LegendPen.setStyle(xfl::getStyle(pCurve->stipple()));

                if(m_pCurveModel->isCurveSelected(pCurve) && isHighLighting())
                {
                    if(DisplayOptions::isLightTheme())
                        linecolor = pCurve->qColor().darker();
                    else
                        linecolor = pCurve->qColor().lighter();
                    LegendPen.setWidth(pCurve->width()+3);
                }
                else
                {
                    linecolor = pCurve->qColor();
                    LegendPen.setWidth(pCurve->width());
                }
                LegendPen.setColor(linecolor);
                painter.setPen(LegendPen);

                painter.drawLine(int(Place.x()),              int(Place.y() + fmh*npos + fmh/3),
                                 int(Place.x() + LegendSize), int(Place.y() + fmh*npos + fmh/3));

                if(pCurve->symbol())
                {
                    int x1 = int(Place.x() + 0.5*LegendSize);
                    int y1 = int(Place.y() + 1.*fmh*npos+ fmh/3);

                    LegendPen.setStyle(Qt::SolidLine);
                    painter.setPen(LegendPen);
                    xfl::drawSymbol(painter, pCurve->symbol(), m_BkColor, linecolor, QPoint(x1, y1));
                }

                painter.setPen(TextPen);
                painter.drawText(int(Place.x() + LegendSize + fm.averageCharWidth()),
                                 int(Place.y() + fmh*npos+fmh/2),
                                 strong);

                npos++;
            }
        }
    }

    painter.restore();
}



void Graph::toFile(QFile &XFile, bool bCSV) const
{
    int maxpoints=0;
    QString strong;
    QTextStream out(&XFile);

    maxpoints = 0;
    for(int i=0; i<m_pCurveModel->curveCount(); i++)
    {
        Curve *pCurve = curve(i);
        if(pCurve)
        {
            maxpoints = qMax(maxpoints,pCurve->size());

            strong = pCurve->name();
            if(!bCSV) out << "     "<<xVariableName()<<"       "<< strong <<"    ";
            else      out << xVariableName()<<", "<< strong<<", ";
        }
    }
    out<<"\n"; //end of title line

    for(int j=0; j<maxpoints; j++)
    {
        for(int i=0; i<m_pCurveModel->curveCount(); i++)
        {
            Curve *pCurve = curve(i);
            if(pCurve && j<pCurve->size())
            {
                if(!bCSV) strong = QString::asprintf("%13.7g  %13.7g  ", pCurve->x(j), pCurve->y(j));
                else      strong = QString::asprintf("%13.7g, %13.7g,",  pCurve->x(j), pCurve->y(j));
            }
            else
            {
                if(!bCSV) strong= "                                 ";
                else      strong= ", , ";
            }
            out << strong;
        }
        out<<"\n"; //end of data line
    }
    out<<"\n"; //end of file
    XFile.close();
}


void Graph::saveSettings(QSettings &settings)
{
    settings.beginGroup(m_Name);
    {
        //save variables
        settings.setValue("TitleColor",  m_TitleColor);
        settings.setValue("LabelColor",  m_LabelColor);
        settings.setValue("LegendColor", m_LegendColor);

        settings.setValue("TitleFontName",   m_TitleFont.family());
        settings.setValue("TitleFontSize",   m_TitleFont.pointSize());
        settings.setValue("TitleFontItalic", m_TitleFont.italic());
        settings.setValue("TitleFontBold",   m_TitleFont.bold());

        settings.setValue("LabelFontName",   m_LabelFont.family());
        settings.setValue("LabelFontSize",   m_LabelFont.pointSize());
        settings.setValue("LabelFontItalic", m_LabelFont.italic());
        settings.setValue("LabelFontBold",   m_LabelFont.bold());

        settings.setValue("LegendFontName",   m_LegendFont.family());
        settings.setValue("LegendFontSize",   m_LegendFont.pointSize());
        settings.setValue("LegendFontItalic", m_LegendFont.italic());
        settings.setValue("LegendFontBold",   m_LegendFont.bold());


        xfl::saveLineSettings(settings, m_theBorderStyle, "BorderStyle");
        settings.setValue("BorderShow", m_bBorder);

        settings.setValue("BackgroundColor", m_BkColor);

        settings.setValue("lmargin", m_margin[0]);
        settings.setValue("rmargin", m_margin[1]);
        settings.setValue("tmargin", m_margin[2]);
        settings.setValue("bmargin", m_margin[3]);

        settings.setValue("XVariable", m_X);
        settings.setValue("Y0Variable", m_Y[0]);
        settings.setValue("Y1Variable", m_Y[1]);

        settings.setValue("ShowLegend", m_bShowLegend);


        if     (legendPosition() == (Qt::AlignRight  | Qt::AlignVCenter))
            settings.setValue("LegendPosition", 0);
        else if(legendPosition() == (Qt::AlignLeft   | Qt::AlignVCenter))
            settings.setValue("LegendPosition", 1);
        else if(legendPosition() == (Qt::AlignTop    | Qt::AlignHCenter))
            settings.setValue("LegendPosition", 2);
        else if(legendPosition() == (Qt::AlignTop    | Qt::AlignLeft))
            settings.setValue("LegendPosition", 3);
        else if(legendPosition() == (Qt::AlignTop    | Qt::AlignRight))
            settings.setValue("LegendPosition", 4);
        else if(legendPosition() == (Qt::AlignBottom | Qt::AlignHCenter))
            settings.setValue("LegendPosition", 5);
        else if(legendPosition() == (Qt::AlignBottom | Qt::AlignLeft))
            settings.setValue("LegendPosition", 6);
        else if(legendPosition() == (Qt::AlignBottom | Qt::AlignRight))
            settings.setValue("LegendPosition", 7);

        m_Grid.saveSettings(settings);
        m_XAxis.saveSettings(settings);
        m_YAxis[0].saveSettings(settings);
        m_YAxis[1].saveSettings(settings);

        settings.setValue("bRightAxis", m_bRightAxis);
    }
    settings.endGroup();
}


void Graph::loadSettings(QSettings &settings)
{
    QColor clr=Qt::gray;

    settings.beginGroup(m_Name);
    {
        //read variables
        m_TitleColor  = settings.value("TitleColor", QColor(0,0,0)).value<QColor>();
        m_LabelColor  = settings.value("LabelColor", QColor(0,0,0)).value<QColor>();
        m_LegendColor = settings.value("LegendColor", QColor(0,0,0)).value<QColor>();

        m_TitleFont = QFont(settings.value("TitleFontName","Courier").toString());
        int size = settings.value("TitleFontSize",8).toInt();
        if(size>0) m_TitleFont.setPointSize(size);
        m_TitleFont.setItalic(settings.value("TitleFontItalic", false).toBool());
        m_TitleFont.setBold(settings.value("TitleFontBold", false).toBool());

        m_LabelFont = QFont(settings.value("LabelFontName","Courier").toString());
        size = settings.value("LabelFontSize",8).toInt();
        if(size>0) m_LabelFont.setPointSize(size);
        m_LabelFont.setItalic(settings.value("LabelFontItalic", false).toBool());
        m_LabelFont.setBold(settings.value("LabelFontBold", false).toBool());

        m_LegendFont = QFont(settings.value("LegendFontName","Courier").toString());
        size = settings.value("LegendFontSize",8).toInt();
        if(size>0) m_LegendFont.setPointSize(size);
        m_LegendFont.setItalic(settings.value("LegendFontItalic", false).toBool());
        m_LegendFont.setBold(settings.value("LegendFontBold", false).toBool());

        xfl::loadLineSettings(settings, m_theBorderStyle, "BorderStyle");

        m_bBorder = settings.value("BorderShow", true).toBool();

        clr  = settings.value("BackgroundColor", QColor(255,255,255)).value<QColor>();
        setBkColor(clr);

        m_margin[0] = settings.value("lmargin", m_margin[0]).toInt();
        m_margin[1] = settings.value("rmargin", m_margin[1]).toInt();
        m_margin[2] = settings.value("tmargin", m_margin[2]).toInt();
        m_margin[3] = settings.value("bmargin", m_margin[3]).toInt();

        m_X     = settings.value("XVariable",0).toInt();
        m_Y[0]  = settings.value("Y0Variable", 1).toInt();
        m_Y[1]  = settings.value("Y1Variable", 2).toInt();

        m_bShowLegend = settings.value("ShowLegend", false).toBool();

        int pos = settings.value("LegendPosition",4).toInt();
        switch(pos)
        {
            case 0:
                m_LegendPosition = Qt::AlignRight | Qt::AlignVCenter;
                break;
            case 1:
                m_LegendPosition = Qt::AlignLeft | Qt::AlignVCenter;
                break;
            case 2:
                m_LegendPosition = Qt::AlignTop | Qt::AlignHCenter;
                break;
            case 3:
                m_LegendPosition = Qt::AlignTop | Qt::AlignLeft;
                break;
            default:
            case 4:
                m_LegendPosition = Qt::AlignTop | Qt::AlignRight;
                break;
            case 5:
                m_LegendPosition = Qt::AlignBottom | Qt::AlignHCenter;
                break;
            case 6:
                m_LegendPosition = Qt::AlignBottom | Qt::AlignLeft;
                break;
            case 7:
                m_LegendPosition = Qt::AlignBottom | Qt::AlignRight;
                break;
        }

        m_Grid.loadSettings(settings);
        m_XAxis.loadSettings(settings);
        m_YAxis[0].loadSettings(settings);
        m_YAxis[1].loadSettings(settings);

        m_bRightAxis = settings.value("bRightAxis", false).toBool();

    }
    settings.endGroup();
}


/**
 * Copies the graph's data to the clipboard
 */
void Graph::toClipboard()
{
    QString curveData;
    QString strange;

    //    const char *separator = SaveOptions::csvSeparator().toStdString().c_str();
    bool bAtEnd = false;
    int i=0;
    for(int ic=0; ic<curveCount(); ic++)
    {
        Curve*pCurve = curve(ic);
        strange = pCurve->name();
        if(!strange.length()) strange = QString::asprintf("Curve_%d",ic+1);
        curveData += " " + xTitle() + SaveOptions::csvSeparator() + " " + strange + SaveOptions::csvSeparator() + " ";
    }
    curveData += "\n";

    do
    {
        bAtEnd = true;
        for(int ic=0; ic<curveCount(); ic++)
        {
            Curve*pCurve = curve(ic);
            if(i<pCurve->count())
            {
                strange = QString::asprintf(" %11.5g", pCurve->x(i));
                curveData += strange + SaveOptions::csvSeparator();
                strange = QString::asprintf(" %11.5g", pCurve->y(i));
                curveData += strange + SaveOptions::csvSeparator() + " ";
                bAtEnd = false;
            }
            else
            {
                curveData += "             " + SaveOptions::csvSeparator() +"             "+SaveOptions::csvSeparator()+" ";
            }
        }
        curveData +="\n";
        i++;
    } while(!bAtEnd);

    QClipboard *pClipBoard = QApplication::clipboard();
    pClipBoard->setText(curveData);
}


Curve *Graph::getCurve(QPointF const &pt, int &ipt)
{
    double x1(0),x2(0),y1(0),y2(0),d2(0);
    double pixelDist = 10.0;
    for(int ic=0; ic<curveCount(); ic++)
    {
        Curve *pCurve = curve(ic);
        if(!pCurve->isVisible()) continue;

        int iy= pCurve->isLeftAxis() ? 0 : 1;

        if(pCurve->isEmpty())
        {
        }
        else if(pCurve->size()==1)
        {
            if(!m_XAxis.bLogScale()) x1 = xToClient(pCurve->x(0));
            else                     x1 = xToClient(log10(pCurve->x(0)));
            if(!m_YAxis[iy].bLogScale()) y1 = yToClient(iy, pCurve->y(0));
            else                         y1 = yToClient(iy, log10(pCurve->y(0)));
            if(sqrt((pt.x()-x1)*(pt.x()-x1)+(pt.y()-y1)*(pt.y()-y1))<pixelDist)
                return pCurve;
        }
        else
        {
            for(int ip=0; ip<pCurve->size()-1; ip++)
            {
                if(!m_XAxis.bLogScale()) x1 = xToClient(pCurve->x(ip));
                else                     x1 = xToClient(log10(pCurve->x(ip)));
                if(!m_YAxis[iy].bLogScale()) y1 = yToClient(iy, pCurve->y(ip));
                else                         y1 = yToClient(iy, log10(pCurve->y(ip)));

                if(!m_XAxis.bLogScale()) x2 = xToClient(pCurve->x(ip+1));
                else                     x2 = xToClient(log10(pCurve->x(ip+1)));
                if(!m_YAxis[iy].bLogScale()) y2 = yToClient(iy, pCurve->y(ip+1));
                else                         y2 = yToClient(iy, log10(pCurve->y(ip+1)));

                if(sqrt((pt.x()-x1)*(pt.x()-x1)+(pt.y()-y1)*(pt.y()-y1))<pixelDist)
                {
                    ipt = ip;
                    return pCurve;
                }
                if(sqrt((pt.x()-x2)*(pt.x()-x2)+(pt.y()-y2)*(pt.y()-y2))<pixelDist)
                {
                    ipt = ip+1;
                    return pCurve;
                }

                // test between the two points
                d2  = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
                if(d2>0.0 && (pCurve->stipple()!=Line::NOLINE))
                {
                    double x1p = pt.x()-x1;
                    double y1p = pt.y()-y1;
                    double x2p = pt.x()-x2;
                    double y2p = pt.y()-y2;
                    double crossZ = x1p*y2p - y1p*x2p;

                    if(fabs(crossZ)/d2<pixelDist)
                    {
                        // the point is close to the line
                        // test if the projection of the point is between pt1 and pt2
                        double x12 = x2-x1;
                        double y12 = y2-y1;
                        double dotproduct = x1p*x12 + y1p*y12;
                        if(dotproduct>=0)
                        {
                            // point is "right" of point 1
                            // check if it "left" of point 2
                            double x12 = x2-x1;
                            double y12 = y2-y1;
                            double dotproduct = x2p*x12 + y2p*y12;
                            if(dotproduct<=0)
                            {
                                ipt = -1;
                                return pCurve;
                            }
                        }
                    }
                }
            }
        }
    }

    ipt = -1;
    return nullptr;
}


bool Graph::setXScale(QRectF const &graphrect)
{
    if(!m_pCurveModel) return false;

    double mw =  double(graphrect.width() - m_margin[0] - m_margin[1]);
    mw = std::max(mw, 10.0);
    Axis &xax = m_XAxis; // be smarter than the debugger
    if(xax.bAuto())
    {
        if (m_pCurveModel->hasVisibleCurve())
        {
            double Cxmin =  1.e10;
            double Cxmax = -1.e10;
            m_pCurveModel->getXBounds(Cxmin, Cxmax);

            if(Cxmax<=Cxmin)
                Cxmax = Cxmin+1.0;

            if(m_AutoScaleType == GRAPH::EXPANDING)
            {
                xax.setMin(std::min(xax.axmin(), Cxmin));
                xax.setMax(std::max(xax.axmax(), Cxmax));
            }
            else // GRAPH::RESETTING
            {
                xax.setMin(Cxmin);
                xax.setMax(Cxmax);
            }


            if(Cxmin>=0.0) xax.setMin(0.0);
            if(Cxmax<=0.0) xax.setMax(0.0);
        }
        else
        {
            // until things are made clear
            if(xax.axmin()>xax.axmax())
            {
                double tmp = xax.axmax();
                xax.setMax(xax.axmin());
                xax.setMin(tmp);
                xax.setUnit((xax.axmax()-xax.axmin())/10.0);
            }
        }
        if(xax.axmin()<0.0 && xax.axmax()<0.0) xax.setMax(0.0);
        if(xax.axmin()>0.0 && xax.axmax()>0.0) xax.setMin(0.0);
        xax.set0(0.0);

        if(fabs((xax.axmin()-xax.axmax())/xax.axmin())<0.001)
        {
            if(fabs(xax.axmin())<0.00001) xax.setMax(1.0);
            else
            {
                xax.setMax(2.0 * xax.axmin());
                if(xax.axmax() < xax.axmin())
                {
                    double tmp = xax.axmax();
                    xax.setMax(xax.axmin());
                    xax.setMin(tmp);
                }
            }
        }


        if(fabs(xax.axmax()-xax.axmin())<1.e-15) return false;
        xax.setScale(mw/(xax.axmax()-xax.axmin()));

        //try to set an automatic scale for X Axis
        setAutoXUnit();
    }
    else
    {
        //scales are set manually
        if(mw<=0.0) return false;

        setAutoXUnit();

        if (xax.unit()<1.0)
        {
            xax.setExponent(int(log10(xax.unit()*1.00001)-1));
            xax.setExponent(std::max(-4, xax.exponent()));
        }
        else xax.setExponent(int(log10(xax.unit()*1.00001)));

    }
    if(fabs(xax.axmax()-xax.axmin())<1.e-15) return false;
    xax.setScale(mw/(xax.axmax()-xax.axmin()));

    //graph center position
    double Xg = double((graphrect.right()-m_margin[1]) + (graphrect.left()-m_margin[0]))/2;
    // curves center position
    double Xc = (xax.axmin()+xax.axmax())/2.0*xax.scale();
    // center graph in drawing rectangle
    m_ptOffset[0].rx() = double(m_margin[0])+(Xg-Xc);
    m_ptOffset[1].rx() = double(m_margin[0])+(Xg-Xc);

    return true;
}


bool Graph::setXLogScale(QRectF const &graphrect)
{
    if(!m_pCurveModel) return false;

    double mw =  double(graphrect.width() - m_margin[0] - m_margin[1]);
    mw = std::max(mw, 10.0);

    int iy=0; // use left axis

    if(m_XAxis.bAuto())
    {
        if (m_pCurveModel->hasVisibleCurve())
        {
            double Cxmin =  1.e10;
            double Cxmax = -1.e10;
            m_pCurveModel->getXPositiveBounds(Cxmin, Cxmax);

            if(Cxmin<0.0) Cxmin=0.0;
            if(Cxmax<=0.0)
                Cxmax=1.0;
            /*            if(m_AutoScaleType == GRAPH::EXPANDING)
            {
                m_XAxis.axmin() = std::min(m_XAxis.axmin(), Cxmin);
                m_XAxis.axmax() = std::max(m_XAxis.axmax(), Cxmax);
            }
            else // GRAPH::RESETTING */
            {
                m_XAxis.setMax(ceil(log10(Cxmax)));
                if(Cxmin>0.0)
                    m_XAxis.setMin(floor(log10(Cxmin)));
                else
                    m_XAxis.setMin(m_XAxis.axmax()-2);
            }
        }
        else
        {
            // until things are made clear
            if(m_XAxis.axmin()>m_XAxis.axmax())
            {
                double tmp = m_XAxis.axmax();
                m_XAxis.setMax(m_XAxis.axmin());
                m_XAxis.setMin(tmp);
                m_XAxis.setUnit((m_XAxis.axmax()-m_XAxis.axmin())/10.0);
            }
        }

        //try to set an automatic scale for X Axis
        m_XAxis.setUnit(1.0);
    }
    else
    {
        //scales are set manually
        if (m_XAxis.unit()<1.0)
        {
            m_XAxis.setExponent(int(log10(m_XAxis.unit()*1.00001)-1));
            m_XAxis.setExponent(std::max(-4, m_XAxis.exponent()));
        }
        else m_XAxis.setExponent(int(log10(m_XAxis.unit()*1.00001)));

    }
    m_XAxis.setUnit(1.0);
    m_XAxis.set0(0.0);

    if(fabs(m_XAxis.axmax()-m_XAxis.axmin())<1.e-15) return false;
    m_XAxis.setScale(mw/(m_XAxis.axmax()-m_XAxis.axmin()));

    //graph center position
    double Xg = ((graphrect.right()-m_margin[1]) + (graphrect.left()-m_margin[0]))/2.0;
    // curves center position
    double Xc = (m_XAxis.axmin()+m_XAxis.axmax())/2.0*m_XAxis.scale();
    // center graph in drawing rectangle
    m_ptOffset[iy].rx() = m_margin[0]+(Xg-Xc);

    return true;
}


bool Graph::setYScale(int iYAxis, QRectF const &graphrect)
{
    if(!m_pCurveModel) return false;

    double mh =  graphrect.height() - m_margin[2] - m_margin[3];
    mh = std::max(mh, 10.0);

    Axis *pYax = &m_YAxis[iYAxis];

    if(pYax->bAuto())
    {
        if(m_pCurveModel->hasVisibleCurve())
        {
            double Cymin =  1.e10;
            double Cymax = -1.e10;
            m_pCurveModel->getYBounds(Cymin, Cymax, pYax->axis());

            if(Cymax<=Cymin)
            {
                if(fabs(Cymin)<1.e-10) Cymax = 1.0; // everything is zero
                else {
                    if(Cymin>0.0) Cymax = Cymin*1.1;
                    else          Cymax = 0.0;

                }
             }

            if(m_AutoScaleType == GRAPH::EXPANDING)
            {
                pYax->setMin(std::min(pYax->axmin(), Cymin));
                pYax->setMax(std::max(pYax->axmax(), Cymax));
            }
            else // GRAPH::RESETTING
            {
                pYax->setMin(Cymin);
                pYax->setMax(Cymax);
                if(Cymin>=0.0) pYax->setMin(0.0);
                if(Cymax<=0.0) pYax->setMax(0.0);
            }
        }
        else
        {
            // until things are made clear
            if(pYax->axmin()>pYax->axmax())
            {
                double tmp = pYax->axmax();
                pYax->setMax(pYax->axmin());
                pYax->setMin(tmp);
                pYax->setUnit((pYax->axmax()-pYax->axmin())/10.0);
            }
        }

        if(pYax->axmin()<0.0 && pYax->axmax()<0.0) pYax->setMax(0.0);
        if(pYax->axmin()>0.0 && pYax->axmax()>0.0) pYax->setMin(0.0);

        pYax->set0(0.0);

        if (fabs((pYax->axmin() -pYax->axmax())/pYax->axmin() )<0.001)
        {
            if(fabs(pYax->axmin() )<0.00001) pYax->setMax(1.0);
            else
            {
                pYax->setMax(2.0 * pYax->axmin());
                if(pYax->axmax() < pYax->axmin() )
                {
                    double tmp = pYax->axmax();
                    pYax->setMax(pYax->axmin());
                    pYax->setMin(tmp);
                }
            }
        }

        double range = pYax->axmax()-pYax->axmin();
        if (!pYax->bInverted()) pYax->setScale(-mh/range);
        else                    pYax->setScale( mh/range);

        //try to set an automatic scale for Y Axis
        setAutoYUnit(iYAxis);
    }
    else
    {
        //scales are set manually

        if (!pYax->bInverted()) pYax->setScale(-mh/(pYax->axmax()-pYax->axmin() ));
        else                    pYax->setScale( mh/(pYax->axmax()-pYax->axmin() ));

        setAutoYUnit(iYAxis);

        if (pYax->unit()<1.0)
        {
            pYax->setExponent(int(log10(pYax->unit()*1.00001)-1));
            pYax->setExponent(std::max(-4,  pYax->exponent()));
        }
        else   pYax->setExponent(int(log10(pYax->unit()*1.00001)));
    }

    //graph center position
    double Yg = ((graphrect.top()-m_margin[2]) + (graphrect.bottom()-m_margin[3]))/2.0;
    // curves center position
    double Yc = (pYax->axmin() +pYax->axmax())/2.0*pYax->scale();
    // center graph in drawing rectangle
    m_ptOffset[iYAxis].ry() = m_margin[2]+(Yg-Yc);

    return true;
}


bool Graph::setYLogScale(int iYAxis, QRectF const &graphrect)
{
    if(!m_pCurveModel) return false;

    double mh =  graphrect.height() - m_margin[2] - m_margin[3];
    mh = std::max(mh, 10.0);

    Axis *pYax = &m_YAxis[iYAxis];

    if(pYax->bAuto())
    {
        if (m_pCurveModel->hasVisibleCurve())
        {
            double Cymin =  1.e10;
            double Cymax = -1.e10;
            m_pCurveModel->getYPositiveBounds(Cymin, Cymax);

            if(Cymin<0.0) Cymin=0.0;
            if(Cymax<=0.0)
                Cymax=1.0;
            /*            if(m_AutoScaleType == GRAPH::EXPANDING)
            {
                pYax->axmin() = std::min(pYax->axmin(), Cxmin);
                pYax->axmax() = std::max(pYax->axmax(), Cxmax);
            }
            else // GRAPH::RESETTING */
            {
                pYax->setMax(ceil(log10(Cymax)));
                if(Cymin>0.0) pYax->setMin(floor(log10(Cymin)));
                else          pYax->setMin(pYax->axmax()-2.0);
            }
        }
        else
        {
            // until things are made clear
            if(pYax->axmin()>pYax->axmax())
            {
                double tmp = pYax->axmax();
                pYax->setMax(pYax->axmin());
                pYax->setMin(tmp);
                pYax->setUnit((pYax->axmax()-pYax->axmin())/10.0);
            }
        }
    }
    else
    {
        //scales are set manually
        if (pYax->unit()<1.0)
        {
            pYax->setExponent(int(log10(pYax->unit()*1.00001)-1));
            pYax->setExponent(std::max(-4, pYax->exponent()));
        }
        else pYax->setExponent(int(log10(pYax->unit()*1.00001)));

    }
    pYax->setUnit(1.0);
    pYax->set0(0.0);
    if(!pYax->bInverted()) pYax->setScale(-mh/(pYax->axmax()-pYax->axmin()));
    else                   pYax->setScale( mh/(pYax->axmax()-pYax->axmin()));

    //graph center position
    double Yg = ((graphrect.top()-m_margin[2]) + (graphrect.bottom()-m_margin[3]))/2.0;
    // curves center position
    double Yc = (pYax->axmin() +pYax->axmax())/2.0/pYax->scale();
    // center graph in drawing rectangle
    m_ptOffset[iYAxis].ry() = m_margin[2]+(Yg-Yc);

    return true;
}


void Graph::setGraphScales(QRectF const &graphrect)
{
    if(!m_XAxis.bLogScale()) setXScale(graphrect);
    else                     setXLogScale(graphrect);

    if(!m_YAxis[0].bLogScale()) setYScale(0, graphrect);
    else                        setYLogScale(0, graphrect);

    if(!m_YAxis[1].bLogScale()) setYScale(1, graphrect);
    else                        setYLogScale(1, graphrect);

    if(m_Grid.bXAutoMinGrid())  m_XAxis.setMinorUnit(m_XAxis.unit()/double(Axis::unitRatio()));
    if(m_Grid.bYAutoMinGrid(0)) m_YAxis[0].setMinorUnit(m_YAxis[0].unit()/double(Axis::unitRatio()));
}


/**
 * Adds a curve object to the Graph's curve model
 * @param name the curve's name, used to plot the legend
 * @param axis defines whether the curve should be plotted against the left y-axis (the default) or the right y-axis
 * @param bDarkTheme used to select the curve's color
*/
Curve* Graph::addCurve(const QString &name, AXIS::enumAxis axis, bool bDarkTheme)
{
    if(m_pCurveModel)
    {
        Curve *pCurve = m_pCurveModel->addCurve(name, axis, bDarkTheme);
        pCurve->setWidth(Curve::defaultLineWidth());
        return pCurve;
    }
    else return nullptr;
}


/**
 * Adds a curve object to the Graph's curve model
 * @param axis defines whether the curve should be plotted against the left y-axis (the default) or the right y-axis
 * @param bDarkTheme used to select the curve's color
*/
Curve* Graph::addCurve(AXIS::enumAxis axis, bool bDarkTheme)
{
    if(m_pCurveModel)
    {
        Curve *pCurve = m_pCurveModel->addCurve(axis, bDarkTheme);
        pCurve->setWidth(Curve::defaultLineWidth());
        return pCurve;
    }
    else return nullptr;
}


void Graph::deleteCurve(int index)
{
    if(m_pCurveModel) m_pCurveModel->deleteCurve(index);

}


void Graph::deleteCurve(Curve *pCurve)
{
    if(m_pCurveModel) m_pCurveModel->deleteCurve(pCurve);
}


void Graph::deleteCurve(QString const&CurveTitle)
{
    if(m_pCurveModel) m_pCurveModel->deleteCurve(CurveTitle);
}


void Graph::deleteCurves()
{
    if(m_pCurveModel) m_pCurveModel->deleteCurves();
    if (m_XAxis.bAuto() && m_AutoScaleType==GRAPH::RESETTING)
    {
        m_XAxis.setMin(0.0);
        m_XAxis.setMax(1.0);
    }

    for(int iy=0; iy<2; iy++)
    {
        if (m_YAxis[1].bAuto() && m_AutoScaleType==GRAPH::RESETTING)
        {
            m_YAxis[1].setMin(0.0);
            m_YAxis[1].setMax(1.0);
        }
    }
}


int Graph::curveCount() const
{
    if(m_pCurveModel)
    {
        return m_pCurveModel->curveCount();
    }
    else return 0;
}


void Graph::resetCurves()
{
    if(m_pCurveModel) m_pCurveModel->resetCurves();
}


Curve* Graph::curve(int nIndex) const
{
    if(m_pCurveModel) return m_pCurveModel->curve(nIndex);
    return nullptr;
}


Curve* Graph::firstCurve() const
{
    if(m_pCurveModel) return m_pCurveModel->firstCurve();
    return nullptr;
}


Curve* Graph::lastCurve() const
{
    if(m_pCurveModel) return m_pCurveModel->lastCurve();
    return nullptr;
}


Curve* Graph::curve(QString const &curveTitle, bool bFromLast) const
{
    if(m_pCurveModel) return m_pCurveModel->curve(curveTitle, bFromLast);
    return nullptr;
}


void Graph::setVariables(int X, int Y0, int Y1)
{
    m_X=X;
    m_Y[0]=Y0;
    m_Y[1]=Y1;
}

void Graph::setBorderColor(fl5Color const &crBorder) {m_theBorderStyle.m_Color = crBorder;}
void Graph::setBorderColor(QColor crBorder) {m_theBorderStyle.m_Color = xfl::tofl5Clr(crBorder);}
QColor Graph::borderColor() const {return xfl::fromfl5Clr(m_theBorderStyle.m_Color);}


void Graph::setXVarStdList(std::vector<std::string> const &XVarList)
{
    m_XVarList.clear();
    m_XVarList.reserve(XVarList.size());
    for(uint i=0; i<XVarList.size(); i++)
        m_XVarList.append(QString::fromStdString(XVarList.at(i)));
}


void Graph::setYVarStdList(std::vector<std::string> const &YVarList)
{
    m_YVarList.clear();
    m_YVarList.reserve(YVarList.size());
    for(uint i=0; i<YVarList.size(); i++)
        m_YVarList.append(QString::fromStdString(YVarList.at(i)));
}







