/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#include <QFileInfo>

#include "objects2d_globals.h"


#include <xflcore/xflcore.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/polar.h>


void drawFoil(QPainter &painter, Foil const*pFoil, double alpha, double twist, double scalex, double scaley,
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


void drawFoilNormals(QPainter &painter, Foil const*pFoil, double alpha, double scalex, double scaley, QPointF const &Offset)
{
    QPointF From, To;

    QPen NormalPen;

    NormalPen.setColor(pFoil->lineColor().darker());
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


void drawFoilMidLine(QPainter &painter, Foil const*pFoil, double scalex, double scaley, QPointF const &Offset)
{
    painter.save();
    painter.translate(Offset);
    painter.scale(scalex, scaley);

    if(pFoil->m_CbLine.size()>0)
    {
        QPolygonF camberline;
        for (int k=0; k<pFoil->m_CbLine.size(); k++)
        {
            camberline.append({  pFoil->m_CbLine.at(k).x, -pFoil->m_CbLine.at(k).y});
        }
        painter.drawPolyline(camberline);
    }
    painter.restore();
}


void drawFoilPoints(QPainter &painter, Foil const *pFoil, double alpha, double scalex, double scaley,
                    QPointF const &Offset, QColor const &backColor, QRect const &drawrect)
{
    QPen FoilPen;
    FoilPen.setColor(pFoil->lineColor());
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

        xfl::drawSymbol(painter, pFoil->pointStyle(), backColor, QColor(255,0,0), pt);
    }
}
