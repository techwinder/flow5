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


#include "graphsvgwriter.h"
#include <core/displayoptions.h>
#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/graphs/containers/graphwt.h>
#include <interfaces/graphs/graph/graph.h>
#include <interfaces/view2d/paint2d.h>
#include <api/utils.h>


bool GraphSVGWriter::s_bFillBackground=true;

GraphSVGWriter::GraphSVGWriter(QFile &XFile): XflSvgWriter(XFile)
{
    m_pGraph   = nullptr;
    m_pGraphWt = nullptr;
}


void GraphSVGWriter::writeGraph(GraphWt const*pGraphWt, Graph const *pGraph)
{
    m_pGraphWt = pGraphWt;
    m_pGraph   = pGraph;
    QRect r=pGraphWt->rect();

    writeStartDocument();

    writeStartElement("svg");
    {
        writeAttribute("xmlns",   "http://www.w3.org/2000/svg");
        writeAttribute("width",   QString::asprintf("%dpx", pGraphWt->width()));
        writeAttribute("height",  QString::asprintf("%dpx", pGraphWt->height()));
        writeAttribute("viewbox", QString::asprintf("%d %d %d %d", r.x(), r.y(), r.width(), r.height()));
        writeAttribute("version", "1.1");
        writeTextElement("title", pGraph->name());

        if(s_bFillBackground)
        {
             writeStartElement("rect");
             {
                 writeAttribute("width","100%");
                 writeAttribute("height", "100%");
                 writeAttribute("fill", xfl::colorNameARGB(m_pGraph->backgroundColor()));
             }
             writeEndElement();
        }

        writeXAxis();
        writeYAxis(0);
        if(pGraph->hasRightAxis()) writeYAxis(1);

        writeXMajGrid();
        writeYMajGrid(0);
        if(m_pGraph->hasRightAxis()) writeYMajGrid(1);
        if(m_pGraph->bXMinGrid())  writeXMinGrid();
        if(m_pGraph->bYMinGrid(0)) writeYMinGrid();

        if(m_pGraph->curveModel())
        {
            for (int ic=0; ic<m_pGraph->curveCount(); ic++)
            {
                if(m_pGraph->curve(ic)->isVisible()) writeCurve(ic);
            }
        }
        writeTitles(pGraphWt->rect(), m_pGraph->titleColor());
        if(m_pGraph->isLegendVisible()) writeLegend(pGraphWt->legendOrigin(), m_pGraph->legendFont(), DisplayOptions::textColor());
    }
    writeEndElement(); //svg

    writeEndDocument();
}


void GraphSVGWriter::writeXAxis()
{
    // x-Axis

    double yp=0;
    double scaley = m_pGraph->yScale(0);
    if(!m_pGraph->bYLogScale(0))
    {
        if(m_pGraph->y0Origin(0)>=m_pGraph->yMin(0) && m_pGraph->y0Origin(0)<=m_pGraph->yMax(0))
            yp = m_pGraph->y0Origin(0);
        else if(m_pGraph->y0Origin(0)>m_pGraph->yMax(0))
            yp = m_pGraph->yMax(0);
        else
            yp = m_pGraph->yMin(0);
    }
    else
    {
        yp = m_pGraph->yMin(0);
    }
    QPointF from(m_pGraph->xMin()*m_pGraph->xScale() +m_pGraph->offset(0).x(), yp*scaley + m_pGraph->offset(0).y());
    QPointF to  (m_pGraph->xMax()*m_pGraph->xScale() +m_pGraph->offset(0).x(), yp*scaley + m_pGraph->offset(0).y());

    writeLine(from, to, m_pGraph->xAxis().theStyle());
}


void GraphSVGWriter::writeYAxis(int iy)
{
    double xp=m_pGraph->yAxisPos(iy);
    double scaley=m_pGraph->yScale(iy);

    QPointF from(xp*m_pGraph->xScale() + m_pGraph->offset(iy).x(), m_pGraph->yMin(iy)*scaley + m_pGraph->offset(iy).y());
    QPointF to  (xp*m_pGraph->xScale() + m_pGraph->offset(iy).x(), m_pGraph->yMax(iy)*scaley + m_pGraph->offset(iy).y());
    writeLine(from, to, m_pGraph->yAxis(iy).theStyle());
}


void GraphSVGWriter::writeCurve(int nIndex)
{
    Curve const* pCurve = nullptr;
    pCurve = m_pGraph->curve(nIndex);

    if(!pCurve) return;

    int iy = pCurve->isLeftAxis() ? 0 : 1;

    Axis const &m_XAxis = m_pGraph->xAxis();
    Axis const &m_YAxis = m_pGraph->yAxis(iy);
    QPointF Offset = m_pGraph->offset(iy);

    double scaley = m_YAxis.scale();

    QPointF from, to, ptMin, ptMax;
    QRectF rViewRect;

    ptMin.setX(m_XAxis.axmin()*m_XAxis.scale() + Offset.x());
    ptMin.setY(m_YAxis.axmin()*scaley          + Offset.y());

    ptMax.setX(m_XAxis.axmax()*m_XAxis.scale() + Offset.x());
    ptMax.setY(m_YAxis.axmax()*scaley          + Offset.y());

    rViewRect.setTopLeft(ptMin);
    rViewRect.setBottomRight(ptMax);

    QPolygonF line;
    if(pCurve->size()>=1 && pCurve->isVisible())
    {
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

        if(m_YAxis.bLogScale())
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

        if(pCurve->stipple()!=Line::NOLINE)
        {
            line.clear();

            if(!m_XAxis.bLogScale()) from.setX(pCurve->x(i0)*m_XAxis.scale() + Offset.x());
            else                     from.setX(log10(pCurve->x(i0))*m_XAxis.scale() + Offset.x());
            if(!m_YAxis.bLogScale()) from.setY(pCurve->y(i0)*scaley + Offset.y());
            else                     from.setY(log10(pCurve->y(i0))*scaley + Offset.y());

            line.push_back(from);

            for (int i=i0+1; i<pCurve->size();i++)
            {
                if(i<pCurve->size())
                {
                    bool bLogPoint = true;
                    if(!m_XAxis.bLogScale())
                    {
                        to.setX(pCurve->x(i)*m_XAxis.scale() + Offset.x());
                    }
                    else
                    {
                        if(pCurve->x(i)>0.0)
                        {
                            to.setX(log10(pCurve->x(i))*m_XAxis.scale() + Offset.x());
                        }
                        else
                        {
                            to = from;
                            bLogPoint = false;
                        }
                    }

                    if(!m_YAxis.bLogScale())
                    {
                        to.setY(pCurve->y(i)*scaley + Offset.y());
                    }
                    else
                    {
                        if(pCurve->y(i)>0.0)
                        {
                            to.setY(log10(pCurve->y(i))*scaley + Offset.y());
                        }
                        else
                        {
                            to = from;
                            bLogPoint = false;
                        }
                    }
                    if(bLogPoint)
                        line.push_back(to);

                }
            }

            writePolyLine(line, pCurve->theStyle());
        }

        if(pCurve->pointsVisible())
        {
            for (int i=0; i<pCurve->size();i++)
            {
                bool bPoint = true;
                double x=pCurve->x(i);
                double y=pCurve->y(i);
                if(m_XAxis.bLogScale())
                {
                    if(x>0.0) x= log10(x);
                    else      bPoint = false;
                }

                if(m_YAxis.bLogScale())
                {
                    if(y>0.0) y= log10(y);
                    else      bPoint = false;
                }
                if(bPoint)
                {
                    writePoint(x*m_XAxis.scale()+Offset.x(),
                               y*m_YAxis.scale()+Offset.y(),
                               pCurve->theStyle(),
                               m_pGraph->backgroundColor());
                }
            }
        }
    }
}


void GraphSVGWriter::writeTitles(QRectF const &graphrect, QColor const &clr)
{
    Axis const &m_XAxis = m_pGraph->xAxis();
    Axis const &m_YAxis = m_pGraph->yAxis(0);
    QPointF Offset = m_pGraph->offset(0);
    QFontMetrics fm(m_pGraph->labelFont());

    //draws the x & y axis titles
    int XPosXTitle=0, YPosXTitle=0;
    double yp=0;

    XPosXTitle = -fm.horizontalAdvance(m_pGraph->xVariableName())/2;
    YPosXTitle = -10;

    if(!m_YAxis.bLogScale())
    {
        if(m_YAxis.origin()>=m_YAxis.axmin() && m_YAxis.origin()<=m_YAxis.axmax()) yp = m_YAxis.origin();
        else if(m_YAxis.origin()>m_YAxis.axmax())                                  yp = m_YAxis.axmax();
        else                                                                       yp = m_YAxis.axmin();
    }
    else
    {
        yp = m_YAxis.axmin();
    }

    QPointF pos(m_XAxis.axmax()*m_XAxis.scale() + Offset.x() + XPosXTitle,
                yp  *m_YAxis.scale() + Offset.y() + YPosXTitle);
    writeText(pos.x(), pos.y(), m_pGraph->titleFont(), clr, m_pGraph->xVariableName());

    for(int iy=0; iy<2; iy++)
    {
        if(iy==0 || (iy==1 && m_pGraph->hasRightAxis()))
        {
            //draws the x & y axis name

            int XPosYTitle = -5;
            int YPosYTitle =  5;

            double xp = m_pGraph->yAxisPos(iy);

            QPointF pos0(Offset.x() + xp*m_XAxis.scale() + XPosYTitle, graphrect.top() + m_pGraph->topMargin() - YPosYTitle);
            writeText(pos0.rx(), pos0.y(), m_pGraph->titleFont(), clr, m_pGraph->yVariableName(iy));
        }
    }
}


void GraphSVGWriter::writeLegend(QPointF const &Place, QFont const &LegendFont, QColor const &LegendColor)
{
    QString strong;

    double LegendSize = 30.0;

    QFontMetrics fm(LegendFont);
    double fmh = double(fm.height());
    fmh *= 11.0/10.0;

    QColor linecolor;

    QPen LegendPen(Qt::gray);

    int npos = 0;
    for (int nc=0; nc<m_pGraph->curveCount(); nc++)
    {
        Curve*pCurve = m_pGraph->curve(nc);
        if(pCurve->isVisible())
        {
            strong = pCurve->name();
            if(pCurve->size()>0 && strong.length())//is there anything to draw?
            {
                LegendPen.setStyle(xfl::getStyle(pCurve->stipple()));

                linecolor = pCurve->qColor();
                LegendPen.setWidth(pCurve->width());

                LegendPen.setColor(linecolor);
                writeLine(Place.x(),              Place.y() + fmh*npos + fmh/3,
                          Place.x() + LegendSize, Place.y() + fmh*npos + fmh/3,
                          pCurve->theStyle());

                if(pCurve->symbol())
                {
                    double x1 = Place.x() + 0.5*LegendSize;
                    double y1 = Place.y() + 1.*fmh*npos+ fmh/3;

                    writePoint(x1, y1, pCurve->theStyle(), m_pGraph->backgroundColor());
                }

                writeText(Place.x() + 1.5*LegendSize,
                          Place.y() + fmh*double(npos)+fmh/2,
                          LegendFont,
                          LegendColor,
                          strong);

                npos++;
            }
        }
    }
}


void GraphSVGWriter::writeXMajGrid()
{
//    double main=0, xt=0, yp=0;
//    int exp=0, nx=0;

//    int iy=0; // use left axis

    Grid const &grid = m_pGraph->grid();
    Axis const &XAxis = m_pGraph->xAxis();
    Axis const &YAxis0 = m_pGraph->yAxis(0);
//    Axis const &m_YAxis1 = m_pGraph->yAxis(1);
    QString strLabel;

    QFontMetrics fm(m_pGraph->labelFont());

    QFont expfont(m_pGraph->labelFont());
    expfont.setPointSize(expfont.pointSize()-2);

    double ticksize = 5;
    double height = fm.height()/2;
//    int yExpOff = height/2;

    int nx = int((XAxis.origin()-XAxis.axmin())/XAxis.unit());
    double xt = XAxis.origin() - nx*XAxis.unit();

    double yp=0.0;
    if(!YAxis0.bLogScale())
    {
        if     (YAxis0.origin()>=YAxis0.axmin() && YAxis0.origin()<=YAxis0.axmax()) yp = YAxis0.origin();
        else if(YAxis0.origin()>YAxis0.axmax())                                     yp = YAxis0.axmax();
        else                                                                        yp = YAxis0.axmin();
    }
    else yp = YAxis0.axmin();

    double Y_min = YAxis0.axmin()*YAxis0.scale() + m_pGraph->offset(0).y();
    double Y_max = YAxis0.axmax()*YAxis0.scale() + m_pGraph->offset(0).y();

    int iTick = 0;
    while(xt<=XAxis.axmax()*1.0001 && iTick<100)
    {
        double xtp = xt*XAxis.scale();
        double ytp = yp*YAxis0.scale();

        QPointF from(xtp + m_pGraph->offset(0).x(), ytp+ticksize + m_pGraph->offset(0).y());
        QPointF to(  xtp + m_pGraph->offset(0).x(), ytp          + m_pGraph->offset(0).y());
        writeLine(from, to, XAxis.theStyle());

        if(grid.bXMajGrid() && (m_pGraph->hasRightAxis() || iTick!=nx))
        {
            from.setX(xtp + m_pGraph->offset(0).x());
            from.setY(ytp+ticksize + m_pGraph->offset(0).y());
            to.setX(xtp + m_pGraph->offset(0).x());
            to.setY(ytp           + m_pGraph->offset(0).y());
            writeLine(from, to, XAxis.theStyle());

            from.setX(xtp + m_pGraph->offset(0).x());
            from.setY(Y_min);
            to.setX(xtp + m_pGraph->offset(0).x());
            to.setY(Y_max);
            writeLine(from, to, grid.xMajStyle());
        }

        if(iTick==nx)
        {
            // plain 0 at the origin
            writeText(xtp - fm.averageCharWidth()/2 + m_pGraph->offset(0).x(),
                      ytp + ticksize*2 +height  + m_pGraph->offset(0).y(),
                      m_pGraph->labelFont(),
                      m_pGraph->labelColor(),
                      "0");
        }
        else
        {
            if      (XAxis.exponent()>0)   strLabel = QString::asprintf("%6.0f",xt);
            else if (XAxis.exponent()>=-1) strLabel = QString::asprintf("%6.1f",xt);
            else if (XAxis.exponent()>=-2) strLabel = QString::asprintf("%6.2f",xt);
            else if (XAxis.exponent()>=-3) strLabel = QString::asprintf("%6.3f",xt);
            else                           strLabel = QString::asprintf("%7g",xt);
            double w = fm.horizontalAdvance(strLabel);

            writeText(xtp - w/3.0 + m_pGraph->offset(0).x(),
                      ytp + ticksize*2 +height   + m_pGraph->offset(0).y(),
                      m_pGraph->labelFont(),
                      m_pGraph->labelColor(),
                      strLabel);
        }

        xt += XAxis.unit();
        iTick++;
    }
}


void GraphSVGWriter::writeYMajGrid(int iy)
{
    Axis const &m_XAxis = m_pGraph->xAxis();
    Axis const &yAxis = m_pGraph->yAxis(iy);
    Grid const &m_Grid = m_pGraph->grid();
    QPointF Offset = m_pGraph->offset(iy);

    if(fabs(yAxis.axmax()-yAxis.axmin() )/yAxis.unit()>30.0) return;
    double scaley = yAxis.scale();

    QString strLabel;

    QFontMetrics fm(m_pGraph->labelFont());

    QFont expfont(m_pGraph->labelFont());
    expfont.setPointSize(expfont.pointSize()-2);

    int fmheight  = fm.height();

    double ticksize = 5;

    QPen labelpen(yAxis.qColor());
    labelpen.setStyle(xfl::getStyle(yAxis.stipple()));
    labelpen.setWidth(yAxis.width());

    double X_min = m_XAxis.axmin()*m_XAxis.scale() + Offset.x();
    double X_max = m_XAxis.axmax()*m_XAxis.scale() + Offset.x();

    double xp = m_pGraph->yAxisPos(iy);

    int ny = int((yAxis.origin()-yAxis.axmin())/yAxis.unit());
    double yt = yAxis.origin() - ny*yAxis.unit();

    int iTick=0;

    double yd = -1; // slight offset for y-labels to avoid having minus sign on grid lign

    double xtp = xp*m_XAxis.scale(); // the reference x-coordinate
    int sign= (iy==0) ? 1 : -1;

    if(iy==0)
        xtp -= ticksize;

    while(yt<=yAxis.axmax()*1.0001 && iTick<100)
    {
        double ytp = yt*scaley;
        writeLine(xtp          + Offset.x(), ytp + Offset.y(),
                  xtp+ticksize + Offset.x(), ytp + Offset.y(),
                  yAxis.theStyle());

        if(iTick==ny)
        {
            writeText(xtp - (fm.averageCharWidth()+ticksize*1)*sign +Offset.x(),
                      ytp + fmheight/4 +Offset.y(),
                      m_pGraph->labelFont(),
                      m_pGraph->labelColor(),
                      "0");
        }
        else
        {
            if      (yAxis.exponent()>= 0) strLabel = QString::asprintf("%.0f",yt);
            else if (yAxis.exponent()>=-1) strLabel = QString::asprintf("%.1f",yt);
            else if (yAxis.exponent()>=-2) strLabel = QString::asprintf("%.2f",yt);
            else if (yAxis.exponent()>=-3) strLabel = QString::asprintf("%.3f",yt);
            else                           strLabel = QString::asprintf("%7g", yt);
            double w = fm.horizontalAdvance(strLabel);

            if(iy==0)
            {

                writeText(xtp - w - 2*ticksize   + Offset.x(),
                          ytp + fmheight/4 + yd  + Offset.y(),
                          m_pGraph->labelFont(),
                          m_pGraph->labelColor(),
                          strLabel);
            }
            else
            {
                writeText(xtp + 1*ticksize + 1*fm.averageCharWidth() + Offset.x(),
                          ytp + fmheight/4 + yd  + Offset.y(),
                          m_pGraph->labelFont(),
                          m_pGraph->labelColor(),
                          strLabel);
            }
        }

        // draw grid
        if(m_Grid.bYMajGrid(iy) && fabs(yt-yAxis.origin())>1.0e-10)
        {
            writeLine(X_min, ytp+Offset.y(), X_max, ytp+Offset.y(), m_Grid.yMajStyle(0));
        }


        yt += yAxis.unit();
        iTick++;
    }
}


void GraphSVGWriter::writeXMinGrid()
{
    Grid const &grid = m_pGraph->grid();
    Axis const &XAxis = m_pGraph->xAxis();
    Axis const &YAxis0 = m_pGraph->yAxis(0);
    QPointF const &offset = m_pGraph->offset(0);

    double scaley = YAxis0.scale();
    if(fabs(XAxis.unit())<0.00000001) return;
    if(fabs(XAxis.minUnit())<0.00000001) return;
    if(fabs(XAxis.axmax()-XAxis.axmin())/XAxis.unit()>30.0) return;
    if(fabs(XAxis.axmax()-XAxis.axmin())/XAxis.minUnit()>100.0) return;


    double Y_min = YAxis0.axmin()*scaley+offset.y();
    double Y_max = YAxis0.axmax()*scaley+offset.y();

    double xDelta = XAxis.minUnit();
    int nx = int((XAxis.origin()-XAxis.axmin())/XAxis.unit()) +1;
    double xt = XAxis.origin() - nx*XAxis.unit();
    //    double xt = m_XAxis.origin()-int((m_XAxis.origin()-m_XAxis.axmin())*1.0001/xDelta)*xDelta;//one tick at the origin
    int iter=0;

    while(xt<=XAxis.axmax()*1.001 && iter<500)
    {
        if(xt>=XAxis.axmin())
        {
            if(iter%Axis::unitRatio()!=0)  // don't draw over major grid
                writeLine(xt*XAxis.scale()+offset.x(), Y_min,
                          xt*XAxis.scale()+offset.x(), Y_max,
                          grid.xMinStyle());
        }
        xt += xDelta;
        iter++;
    }
}


void GraphSVGWriter::writeYMinGrid()
{
    Grid const &grid = m_pGraph->grid();
    Axis const &XAxis = m_pGraph->xAxis();
    Axis const &YAxis0 = m_pGraph->yAxis(0);
    QPointF const &offset = m_pGraph->offset(0);

    double scaley = YAxis0.scale();
    if(fabs(YAxis0.unit())<0.00000001) return;
    if(fabs(YAxis0.minUnit())<0.00000001) return;
    if(fabs(YAxis0.axmax()-YAxis0.axmin() )/YAxis0.unit()>30.0) return;
    if(fabs(YAxis0.axmax()-YAxis0.axmin() )/YAxis0.minUnit()>100.0) return;

    double yDelta = YAxis0.minUnit();

    int ny = int((YAxis0.origin()-YAxis0.axmin())/YAxis0.unit()) + 1;
    double yt = YAxis0.origin() - ny*YAxis0.unit();

    double X_min = XAxis.axmin()*XAxis.scale() + offset.x();
    double X_max = XAxis.axmax()*XAxis.scale() + offset.x();

    int iter = 0;
    while(yt<=YAxis0.axmax()*1.0001 && iter<500)
    {
        if(yt>=YAxis0.axmin())
        {
            if(iter%Axis::unitRatio()!=0)  // don't draw over major grid
                writeLine(X_min, yt*scaley+offset.y(),
                          X_max, yt*scaley+offset.y(),
                          grid.yMinStyle(0));
        }
        yt += yDelta;
        iter++;
    }
}



