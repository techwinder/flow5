/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <QFileInfo>

#include "paint2d.h"


#include <core/xflcore.h>
#include <core/displayoptions.h>
#include <api/foil.h>
#include <api/polar.h>
#include <api/constants.h>
#include <api/spline.h>


void xfl::drawFoil(QPainter &painter, Foil const*pFoil, double alpha, double twist, double scalex, double scaley,
              QPointF const &Offset, bool bFill, QColor fillClr)
{
    if(pFoil->nNodes()<=0) return;

    painter.save();
    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);
    double cost = cos(twist*PI/180.0);
    double sint = sin(twist*PI/180.0);

    QPolygonF polyline(pFoil->nNodes());

    double xa(0), ya(0);
    double xr(0), yr(0);
    for(int k=0; k<pFoil->nNodes(); k++)
    {
        xa = (pFoil->x(k)-0.5)*cosa - pFoil->y(k)*sina+ 0.5;
        ya = (pFoil->x(k)-0.5)*sina + pFoil->y(k)*cosa;
        xr = xa*cost - ya*sint;
        yr = xa*sint + ya*cost;
        polyline[k].rx() =  xr;
        polyline[k].ry() = -yr;
    }

    painter.translate(Offset);
    painter.scale(scalex, scaley);

    if(bFill)
    {
        painter.setBrush(fillClr);
        painter.drawPolygon(polyline);
    }
    else
        painter.drawPolyline(polyline);
    painter.restore();
}


void xfl::drawFoilNormals(QPainter &painter, Foil const*pFoil, double alpha, double scalex, double scaley, QPointF const &Offset)
{
    QPointF From, To;

    QPen NormalPen;

    NormalPen.setColor(xfl::fromfl5Clr(pFoil->lineColor()).darker());
    NormalPen.setWidth(1);
    NormalPen.setStyle(Qt::SolidLine);
    painter.setPen(NormalPen);

    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);

    for (int k=0; k<pFoil->nNodes(); k++)
    {
        double xa = (pFoil->x(k)-0.5)*cosa - pFoil->y(k)*sina+ 0.5;
        double ya = (pFoil->x(k)-0.5)*sina + pFoil->y(k)*cosa;
        From.rx() =  xa*scalex+Offset.x();
        From.ry() = -ya*scaley+Offset.y();

        double nx = pFoil->normal(k).x*cosa - pFoil->normal(k).y*sina;
        double ny = pFoil->normal(k).x*sina + pFoil->normal(k).y*cosa;

        xa += nx/10.0;
        ya += ny/10.0;

        To.rx() =  xa*scalex+Offset.x();
        To.ry() = -ya*scaley+Offset.y();

        painter.drawLine(From,To);
    }
}


void xfl::drawFoilMidLine(QPainter &painter, Foil const*pFoil, double scalex, double scaley, QPointF const &Offset)
{
    painter.save();
    painter.translate(Offset);
    painter.scale(scalex, scaley);

    if(pFoil->CbLine().size()>0)
    {
        QPolygonF camberline;
        for (uint k=0; k<pFoil->CbLine().size(); k++)
        {
            camberline.append({  pFoil->CbLine().at(k).x, -pFoil->CbLine().at(k).y});
        }
        painter.drawPolyline(camberline);
    }
    painter.restore();
}


void xfl::drawFoilPoints(QPainter &painter, Foil const *pFoil, double alpha, double scalex, double scaley,
                    QPointF const &Offset, QColor const &backColor, QRect const &drawrect)
{
    QPen FoilPen;
    FoilPen.setColor(xfl::fromfl5Clr(pFoil->lineColor()));
    FoilPen.setWidth(pFoil->lineWidth());
    FoilPen.setStyle(Qt::SolidLine);
    FoilPen.setCosmetic(true);
    painter.setPen(FoilPen);

    double cosa = cos(alpha*PI/180.0);
    double sina = sin(alpha*PI/180.0);

    for (int i=0; i<pFoil->nNodes();i++)
    {
        double xa = (pFoil->x(i)-0.5)*cosa - pFoil->y(i)*sina + 0.5;
        double ya = (pFoil->x(i)-0.5)*sina + pFoil->y(i)*cosa;

        QPointF pt( xa*scalex + Offset.x(), -ya*scaley + Offset.y());

        if(drawrect.contains(int(pt.x()), int(pt.y())))
            xfl::drawSymbol(painter, pFoil->pointStyle(), backColor, pFoil->lineColor(), pt);
    }

    int ih = pFoil->iSelectedPt();


    if(ih>=0 && ih<int(pFoil->nNodes()))
    {
        QPen HighPen;
        HighPen.setCosmetic(true);
        HighPen.setColor(QColor(255,0,0));
        HighPen.setWidth(2);
        painter.setPen(HighPen);

        double xa = (pFoil->x(ih)-0.5)*cosa - pFoil->y(ih)*sina + 0.5;
        double ya = (pFoil->x(ih)-0.5)*sina + pFoil->y(ih)*cosa;

        QPointF pt( xa*scalex + Offset.x(), -ya*scaley + Offset.y());

        xfl::drawSymbol(painter, pFoil->pointStyle(), backColor, fl5Color(255,0,0), pt);
    }
}


void xfl::drawNormals(Spline const *pSpline, QPainter &painter, double scalex, double scaley, QPointF const &Offset)
{
    if(!pSpline->isSingular() && pSpline->ctrlPointCount()>0)
    {
        QPointF From, To;
        Vector2d pt;
        double dx=0,dy=0;
        for(int it=0; it<=10; it++)
        {
            double t = double(it)/10.0;
            double tt = t;
            if(pSpline->isBSpline())
            {
                if(t<=0.0) tt= 0.00001;
                if(t>=1.0) tt= 0.99999;
            }

            pt = pSpline->splinePoint(t);
            pSpline->splineDerivative(tt, dx, dy);
            double norm = sqrt(dx*dx+dy*dy)*10.0;
            From.rx() =  pt.x * scalex + Offset.x();
            From.ry() = -pt.y * scalex*scaley + Offset.y();

            To.rx() =  (pt.x+dy/norm) * scalex         + Offset.x();
            To.ry() = -(pt.y-dx/norm) * scalex* scaley + Offset.y();

            painter.drawLine(From, To);
        }
    }
}


void xfl::drawCtrlPoints(Spline const *pSpline, QPainter &painter, double scalex, double scaley, QPointF const &Offset, QColor const& backclr)
{
    if(!pSpline) return;
    if(!pSpline->isVisible()) return;
    if(!pSpline->bShowCtrlPts()) return;

    painter.save();

    QPointF pt;
    QColor linecolor;
    for (int i=0; i<int(pSpline->ctrlPointCount()); i++)
    {
        pt.rx() =  pSpline->controlPoint(i).x*scalex + Offset.x();
        pt.ry() = -pSpline->controlPoint(i).y*scaley + Offset.y();

        if (pSpline->selectedPoint()==int(i))
        {
            painter.save();
            QPen PointPen;
            PointPen.setWidth(3);
            linecolor = QColor(100,100,255);
            PointPen.setColor(linecolor);
            painter.setPen(PointPen);
            xfl::drawSymbol(painter, Line::BIGSQUARE, backclr, linecolor, QPoint(int(pt.x()), int(pt.y())));
            painter.restore();
        }
        else if(pSpline->highlightedPoint()==int(i))
        {
            painter.save();
            QPen PointPen;
            PointPen.setWidth(3);
            linecolor = QColor(255,0,0);
            PointPen.setColor(linecolor);
            painter.setPen(PointPen);
            xfl::drawSymbol(painter, Line::BIGSQUARE, backclr, linecolor, QPoint(int(pt.x()), int(pt.y())));
            painter.restore();
        }
        else
        {
            painter.save();
            QPen PointPen;
            PointPen.setWidth(2);
            PointPen.setColor(xfl::fromfl5Clr(pSpline->color()));
            painter.setPen(PointPen);
            xfl::drawSymbol(painter, Line::BIGSQUARE, backclr, pSpline->color(), QPoint(int(pt.x()), int(pt.y())));
            painter.restore();
        }
    }
    painter.restore();
}


void xfl::drawOutputPoints(Spline const *pSpline, QPainter & painter, double scalex, double scaley, QPointF const &Offset,
                      const QColor &backColor, const QRect &drawrect)
{
    painter.save();

    QPointF pt;
    QPen OutPen;
    OutPen.setColor(xfl::fromfl5Clr(pSpline->color()));
    OutPen.setStyle(Qt::SolidLine);
    OutPen.setWidth(1);
    painter.setPen(OutPen);

    for (int i=0; i<pSpline->outputSize(); i++)
    {
        pt.rx() =  pSpline->outputPt(i).x*scalex + Offset.x();
        pt.ry() = -pSpline->outputPt(i).y*scaley + Offset.y();
        if(drawrect.contains(int(pt.x()),int(pt.y())))
            xfl::drawSymbol(painter, pSpline->pointStyle(), backColor, pSpline->color(), pt);
    }

    painter.restore();
}


void xfl::drawSpline(Spline const *pSpline, QPainter & painter, double scalex, double scaley, QPointF const &Offset)
{
    if(!pSpline) return;
    if(!pSpline->isVisible()) return;

    painter.save();

    if(pSpline->outputSize()>=3)
    {
        QPolygonF polyline;
        polyline.clear();

        for(int k=0; k<pSpline->outputSize();k++)
        {
            double x =  pSpline->outputPt(k).x * scalex + Offset.x();
            double y = -pSpline->outputPt(k).y * scaley + Offset.y();

            polyline.append(QPointF(x,y));
        }

        painter.drawPolyline(polyline);

    }
    painter.restore();
}


void xfl::drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, QPoint const &pt)
{
    xfl::drawSymbol(painter, pointStyle, bkColor, xfl::tofl5Clr(linecolor), pt.x(), pt.y());
}

void xfl::drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, QPointF const &ptf)
{
    xfl::drawSymbol(painter, pointStyle, bkColor, xfl::tofl5Clr(linecolor), ptf.x(), ptf.y());
}


void xfl::drawSymbol(QPainter &painter, Line::enumPointStyle ptstyle, QColor const &backcolor, fl5Color const &linecolor, QPoint const &pt)
{
    xfl::drawSymbol(painter, ptstyle, backcolor, linecolor, double(pt.x()), double(pt.y()));
}


void xfl::drawSymbol(QPainter &painter, Line::enumPointStyle ptstyle, QColor const &backcolor, fl5Color const &linecolor,  QPointF const & ptf)
{
    xfl::drawSymbol(painter, ptstyle, backcolor, linecolor, ptf.x(), ptf.y());
}


void xfl::drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, double x, double y)
{
    xfl::drawSymbol(painter, pointStyle, bkColor, xfl::tofl5Clr(linecolor), x, y);
}


void xfl::drawSymbol(QPainter &painter, Line::enumPointStyle ptstyle, QColor const &backcolor, const fl5Color &linecolor, double x, double y)
{
    painter.save();
    painter.setBackgroundMode(Qt::TransparentMode);

    QPen Pointer(painter.pen());

    painter.setPen(Pointer);

    QColor bck(backcolor);
    bck.setAlpha(255);

    switch(ptstyle)
    {
        case Line::NOSYMBOL: break;
        case Line::LITTLECIRCLE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize);
            painter.drawEllipse(QPointF(x,y), ptSide*1.2, ptSide*1.2);
            break;
        }
        case Line::LITTLECIRCLE_F:
        {
            QBrush backBrush(xfl::fromfl5Clr(linecolor));
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize);
            painter.drawEllipse(QPointF(x,y), ptSide*1.2, ptSide*1.2);
            break;
        }
        case Line::BIGCIRCLE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize)*1.75;
            painter.drawEllipse(QPointF(x,y), ptSide, ptSide);
            break;
        }
        case Line::BIGCIRCLE_F:
        {
            QBrush backBrush(xfl::fromfl5Clr(linecolor));
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize)*1.75;
            painter.drawEllipse(QPointF(x,y), ptSide, ptSide);
            break;
        }
        case Line::LITTLESQUARE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize)*1.1;
            QRectF rf(x-ptSide, y-ptSide, 2*ptSide, 2*ptSide);
            painter.drawRect(rf);
            break;
        }
        case Line::LITTLESQUARE_F:
        {
            QBrush backBrush(xfl::fromfl5Clr(linecolor));
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize)*1.1;
            QRectF rf(x-ptSide, y-ptSide, 2*ptSide, 2*ptSide);
            painter.drawRect(rf);
            break;
        }
        case Line::BIGSQUARE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize)*1.7;
            QRectF rf(x-ptSide, y-ptSide, 2*ptSide, 2*ptSide);
            painter.drawRect(rf);
            break;
        }
        case Line::BIGSQUARE_F:
        {
            QBrush backBrush(xfl::fromfl5Clr(linecolor));
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize)*1.7;
            QRectF rf(x-ptSide, y-ptSide, 2*ptSide, 2*ptSide);
            painter.drawRect(rf);
            break;
        }
        case Line::TRIANGLE:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(g_SymbolSize)*0.8;

            const QPointF points[3] = {
                QPointF(x-ptSide, y+ptSide),
                QPointF(x,        y-ptSide),
                QPointF(x+ptSide, y+ptSide),
            };

            painter.drawPolygon(points, 3);
            break;
        }
        case Line::TRIANGLE_F:
        {
            QBrush backBrush(xfl::fromfl5Clr(linecolor));
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(g_SymbolSize)*0.8;

            const QPointF points[3] = {
                QPointF(x-ptSide, y+ptSide),
                QPointF(x,        y-ptSide),
                QPointF(x+ptSide, y+ptSide),
            };

            painter.drawPolygon(points, 3);
            break;
        }
        case Line::TRIANGLE_INV:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(g_SymbolSize)*0.8;

            const QPointF points[3] = {
                QPointF(x-ptSide, y-ptSide),
                QPointF(x,        y+ptSide),
                QPointF(x+ptSide, y-ptSide),
            };

            painter.drawPolygon(points, 3);
            break;
        }
        case Line::TRIANGLE_INV_F:
        {
            QBrush backBrush(xfl::fromfl5Clr(linecolor));
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(g_SymbolSize)*0.8;

            const QPointF points[3] = {
                QPointF(x-ptSide, y-ptSide),
                QPointF(x,        y+ptSide),
                QPointF(x+ptSide, y-ptSide),
            };

            painter.drawPolygon(points, 3);
            break;
        }
        case Line::LITTLECROSS:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = double(g_SymbolSize);
            QPointF p0(x-ptSide, y-ptSide);
            QPointF p1(x+ptSide, y+ptSide);
            QPointF p2(x-ptSide, y+ptSide);
            QPointF p3(x+ptSide, y-ptSide);

            painter.drawLine(p0,p1);
            painter.drawLine(p2,p3);
            break;
        }
        case Line::BIGCROSS:
        {
            QBrush backBrush(bck);
            painter.setBrush(backBrush);
            double ptSide = 2.0*double(g_SymbolSize)*0.85;
            QPointF p0(x-ptSide, y-ptSide);
            QPointF p1(x+ptSide, y+ptSide);
            QPointF p2(x-ptSide, y+ptSide);
            QPointF p3(x+ptSide, y-ptSide);

            painter.drawLine(p0,p1);
            painter.drawLine(p2,p3);
            break;
        }
//        default: break;
    }
    painter.restore();
}


#define MININTERVAL  0.000000001

void xfl::drawLabel(QPainter &painter, int xu, int yu, double value, Qt::Alignment align)
{
    if(fabs(value)<MININTERVAL)
    {
        QString strLabel = "0";
        painter.drawText(xu, yu, strLabel);
        return;
    }

    QString strLabel;

    strLabel = QString(xfl::isLocalized() ? "%L1" : "%1").arg(value);
    if(align & Qt::AlignHCenter)
    {
        int px = DisplayOptions::textFontStruct().width(strLabel);
        painter.drawText(xu-px/2, yu, strLabel);
    }
    else if(align & Qt::AlignLeft)
    {
        painter.drawText(xu, yu, strLabel);
    }
}
