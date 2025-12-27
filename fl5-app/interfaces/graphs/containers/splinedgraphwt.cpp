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
#include <QPainter>
#include <QMouseEvent>



#include "splinedgraphwt.h"
#include <core/xflcore.h>
#include <core/displayoptions.h>
#include <interfaces/graphs/graph/graph.h>
#include <interfaces/view2d/paint2d.h>
#include <interfaces/widgets/view/section2dwt.h>

SplinedGraphWt::SplinedGraphWt() : GraphWt()
{
    m_bAutoConvert = false;
    m_bConstrainEndPoints = false;
    m_bOnCurve = false;

    m_CurveIdxMin = m_CurveIdxMax = -1;

    m_XMin = m_YMin = -1e10;
    m_XMax = m_YMax = +1e10;

    m_Spline.setColor(fl5Color(225, 55, 35));
    m_Spline.setWidth(2);
    m_Spline.appendControlPoint(0.00, 0.00);
    m_Spline.appendControlPoint(0.25, 0.25);
    m_Spline.appendControlPoint(0.50, 0.50);
    m_Spline.appendControlPoint(0.75, 0.25);
    m_Spline.appendControlPoint(1.00, 0.00);
    m_Spline.updateSpline();
    m_Spline.makeCurve();
    convertSpline();
}


void SplinedGraphWt::setGraph(Graph *pGraph)
{
    m_pGraph = pGraph;
    if(pGraph)
    {
        m_Spline.updateSpline();
        m_Spline.makeCurve();
        convertSpline();

        pGraph->setXMin(m_Spline.xMin());
        pGraph->setXMax(m_Spline.xMax());
        pGraph->setYMin(0, m_Spline.yMin());
        pGraph->setYMax(0, m_Spline.yMax());
        setWindowTitle(pGraph->name());
    }
}

void SplinedGraphWt::initialize()
{
    m_Spline.updateSpline();
    m_Spline.makeCurve();
    convertSpline();

    resetGraphScales();
}


/**
 * The user has requested the reset of the active graph's scales to their default value
 */
void SplinedGraphWt::onResetGraphScales()
{
    if(!m_pGraph) return;

    m_pGraph->setAuto(true);
    resetGraphScales();

    update();
}


void SplinedGraphWt::resetGraphScales()
{
    if(!m_pGraph) return;
    for(int ic=0; ic<m_Spline.ctrlPointCount(); ic++)
    {
        Vector2d pt = m_Spline.controlPoint(int(ic));
        m_pGraph->setXMin(std::min(m_pGraph->xMin(), pt.x));
        m_pGraph->setXMax(std::max(m_pGraph->xMax(), pt.x));
        for(int iy=0; iy<2; iy++)
        {
            m_pGraph->setYMin(iy, std::min(m_pGraph->yMin(iy), pt.y));
            m_pGraph->setYMax(iy, std::max(m_pGraph->yMax(iy), pt.y));
        }
    }
    m_pGraph->invalidate();
}


void SplinedGraphWt::paintEvent(QPaintEvent* pEvent)
{
    if(!m_pGraph)return;
    QPainter painter(this);
    painter.save();

    QBrush BackBrush(m_pGraph->backgroundColor());
    painter.setBrush(BackBrush);

    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setBackground(BackBrush);

    if(!isEnabled())
    {
        painter.fillRect(pEvent->rect(), BackBrush);
        painter.restore();
        return;
    }

    m_pGraph->drawGraph(painter, rect());

    QPen splinepen(xfl::fromfl5Clr(m_Spline.color()));
    splinepen.setWidth(m_Spline.width());
    splinepen.setStyle(xfl::getStyle(m_Spline.stipple()));
    painter.setPen(splinepen);

    if(m_Spline.isVisible())
    {
        xfl::drawSpline(    &m_Spline, painter, m_pGraph->xScale(), -m_pGraph->yScale(0), m_pGraph->offset(0));
        xfl::drawCtrlPoints(&m_Spline, painter, m_pGraph->xScale(), -m_pGraph->yScale(0), m_pGraph->offset(0), DisplayOptions::backgroundColor());
        if(m_Spline.bShowNormals())
            xfl::drawNormals(&m_Spline, painter, m_pGraph->xScale(), -m_pGraph->yScale(0)/m_pGraph->xScale(), m_pGraph->offset(0));
    }

    //two options for the legend: in graph, or in a separate widget - or both

    if(m_pGraph->isLegendVisible())
    {
        repositionLegend();
        m_pGraph->drawLegend(painter, m_LegendOrigin);
    }

    if(hasFocus() && Graph::bMousePos())
    {
        QPen textPen(DisplayOptions::textColor());
        QFontMetrics fm(DisplayOptions::textFontStruct().font());

        int fmheight  = fm.height();

        painter.setFont(DisplayOptions::textFontStruct().font());
        painter.setPen(textPen);
        painter.drawText(width()-14*fm.averageCharWidth(),fmheight,
                         QString("x = %1").arg(m_pGraph->clientTox(m_LastPoint.x()),9,'f',3));
        painter.drawText(width()-14*fm.averageCharWidth(),2*fmheight,
                         QString("y = %1").arg(m_pGraph->clientToy(0, m_LastPoint.y()),9,'f',3));
    }
    painter.restore();
}


void SplinedGraphWt::mousePressEvent(QMouseEvent *pEvent)
{
    bool bCtrl=false, bShift=false;

    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl  = true;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift = true;

    int CtrlPt=-1;
    QPoint pttmp;
    QPoint point = pEvent->pos();
    if((pEvent->buttons() & Qt::LeftButton))
    {
        m_PointDown.rx() = point.x();
        m_PointDown.ry() = point.y();
        pttmp = QPoint(point.x(), point.y());

        m_bTransGraph = true;
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);

        double xd = m_pGraph->clientTox(point.x());
        double yd = m_pGraph->clientToy(0, point.y());
        double dtolx = fabs(double(Section2dWt::nPixelSelection())/m_pGraph->xScale());
        double dtoly = fabs(double(Section2dWt::nPixelSelection())/m_pGraph->yScale(0));
        CtrlPt = m_Spline.isCtrlPoint(xd, yd, dtolx, dtoly);
        if(CtrlPt<0)
        {
            m_Spline.setSelectedPoint(-1);
            GraphWt::mousePressEvent(pEvent);
        }
        else
        {
            m_Spline.setSelectedPoint(CtrlPt);
        }

        if (bCtrl)
        {
            if(CtrlPt>=0)
            {
                if (m_Spline.selectedPoint()>=0)
                {
                    if(!m_Spline.removeCtrlPoint(m_Spline.selectedPoint()))
                    {
                        // The minimum number of control points has been reached for this spline degree
                        return;
                    }
                    m_Spline.updateSpline();
                    m_Spline.makeCurve();
                    convertSpline();
                }
            }
        }
        else if (bShift)
        {
            m_Spline.insertCtrlPoint(xd,yd);
            m_Spline.updateSpline();
            m_Spline.makeCurve();
            convertSpline();
        }
    }
    update();
}


void SplinedGraphWt::mouseMoveEvent(QMouseEvent *pEvent)
{
    QPoint point;
    point = pEvent->pos();

    if ((pEvent->buttons() & Qt::LeftButton)  && m_Spline.selectedPoint()>=0)
    {
        // user is dragging the point
        double x1 =  m_pGraph->clientTox(point.x());
        double y1 =  m_pGraph->clientToy(0, point.y());
        int ns = m_Spline.selectedPoint();

        if(ns>=0)
        {
            x1 = std::max(x1, m_XMin);
            x1 = std::min(x1, m_XMax);
            y1 = std::max(y1, m_YMin);
            y1 = std::min(y1, m_YMax);
            m_Spline.setCtrlPoint(ns, x1, y1);
        }

        if(ns==0 || ns==m_Spline.ctrlPointCount()-1)
        {
            if(m_bOnCurve && m_pGraph->hasCurve())
            {
                Curve const *pCurve = m_pGraph->firstCurve();
                if(ns==0)
                {
                    m_CurveIdxMin = pCurve->closestPoint(x1, y1, m_pGraph->xScale(), m_pGraph->yScale(0));
                    m_Spline.setCtrlPoint(ns, pCurve->x(m_CurveIdxMin), pCurve->y(m_CurveIdxMin));
                }
                else if(ns==m_Spline.ctrlPointCount()-1)// n=last
                {
                    m_CurveIdxMax = pCurve->closestPoint(x1, y1, m_pGraph->xScale(), m_pGraph->yScale(0));
                    m_Spline.setCtrlPoint(ns, pCurve->x(m_CurveIdxMax), pCurve->y(m_CurveIdxMax));
                }
            }
            else if(m_bConstrainEndPoints)
            {
                if(ns==0)
                {
                    // do not move first point, clamped at (0,0)
                    m_Spline.setCtrlPoint(ns, 0.0, 0.0);
                }
                else if(ns==m_Spline.ctrlPointCount()-1)// n=last
                {
                    // finish at somewhere with t>0 and zero amplitude
                    if(x1<0.0) x1=0.0;
                    y1 = 0.0;
                    m_Spline.setCtrlPoint(ns, x1, y1);
                }
            }
        }

        m_Spline.makeCurve();
        convertSpline();

        update();
    }
    else
    {
        // highlight if mouse passes over a point
        double x1 =  m_pGraph->clientTox(point.x());
        double y1 =  m_pGraph->clientToy(0, point.y());

        double dtolx = fabs(double(Section2dWt::nPixelSelection())/m_pGraph->xScale());
        double dtoly = fabs(double(Section2dWt::nPixelSelection())/m_pGraph->yScale(0));

        int n = m_Spline.isCtrlPoint(x1, y1, dtolx, dtoly);
        if (n>=0 && n<int(m_Spline.ctrlPointCount()))
        {
            m_Spline.setHighlighted(n);
            update();
        }
        else
        {
            m_Spline.setHighlighted(-1);
        }
        GraphWt::mouseMoveEvent(pEvent);
    }
}


void SplinedGraphWt::convertSpline()
{
    if(m_bAutoConvert)
    {
        if(m_pGraph->hasCurve())
        {
            Curve*pCurve0 = m_pGraph->curve(0);
            QVector<double> x(m_Spline.outputSize()), y(m_Spline.outputSize());
            for(int i=0; i<m_Spline.outputSize(); i++)
            {
                x[i] = m_Spline.outputPt(i).x;
                y[i] = m_Spline.outputPt(i).y;
            }
            pCurve0->setPoints(x, y);
        }
    }
}


void SplinedGraphWt::mouseReleaseEvent(QMouseEvent *pEvent)
{
    update();
    QApplication::restoreOverrideCursor();

    emit splineModified(m_CurveIdxMin,m_CurveIdxMax);

    GraphWt::mouseReleaseEvent(pEvent);
}

