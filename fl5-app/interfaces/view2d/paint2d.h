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

#pragma once

#include <QFile>
#include <QPainter>

#include <api/linestyle.h>

class Foil;
class Polar;
class Spline;

namespace xfl
{
    void drawFoil(QPainter &painter, const Foil *pFoil, double alpha, double twist, double scalex, double scaley, QPointF const &Offset, bool bFill=false, QColor fillClr=Qt::gray);
    void drawFoilNormals(QPainter &painter, Foil const *pFoil, double alpha, double scalex, double scaley, QPointF const &Offset);
    void drawFoilMidLine(QPainter &painter, Foil const *pFoil, double scalex, double scaley, QPointF const &Offset);
    void drawFoilPoints(QPainter &painter, Foil const *pFoil, double alpha, double scalex, double scaley, QPointF const &Offset, const QColor &backColor, const QRect &drawrect);



    void drawSpline(const Spline *pSpline, QPainter & painter, double scalex, double scaley, QPointF const &Offset);
    void drawCtrlPoints(Spline const *pSpline, QPainter & painter, double scalex, double scaley, QPointF const &Offset, const QColor &backclr);
    void drawOutputPoints(Spline const *spline, QPainter & painter, double scalex, double scaley, QPointF const &Offset, const QColor &backColor, const QRect &drawrect);
    void drawNormals(Spline const *pSpline, QPainter &painter, double scalex, double scaley, QPointF const &Offset);


    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, QPoint const &pt);
    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, QPointF const &pt);
    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, QColor const &linecolor, double x, double y);


    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, fl5Color const &linecolor, QPoint const &pt);
    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, fl5Color const &linecolor, QPointF const &pt);
    void drawSymbol(QPainter &painter, Line::enumPointStyle pointStyle, QColor const &bkColor, fl5Color const &linecolor, double x, double y);
    void drawLabel(QPainter &painter, int xu, int yu, double value, Qt::Alignment align);


    inline Qt::PenStyle getStyle(Line::enumLineStipple s)
    {
         switch(s)
         {
             default:
             case Line::SOLID:      return Qt::SolidLine;
             case Line::DASH:       return Qt::DashLine;
             case Line::DOT:        return Qt::DotLine;
             case Line::DASHDOT:    return Qt::DashDotLine;
             case Line::DASHDOTDOT: return Qt::DashDotDotLine;
             case Line::NOLINE:     return Qt::NoPen;
         }
    }

}
