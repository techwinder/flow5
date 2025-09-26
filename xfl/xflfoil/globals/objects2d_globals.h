/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QFile>
#include <QPainter>

class Foil;
class Polar;

void drawFoil(QPainter &painter, const Foil *pFoil, double alpha, double twist, double scalex, double scaley, QPointF const &Offset, bool bFill=false, QColor fillClr=Qt::gray);
void drawFoilNormals(QPainter &painter, Foil const *pFoil, double alpha, double scalex, double scaley, QPointF const &Offset);
void drawFoilMidLine(QPainter &painter, Foil const *pFoil, double scalex, double scaley, QPointF const &Offset);
void drawFoilPoints(QPainter &painter, Foil const *pFoil, double alpha, double scalex, double scaley, QPointF const &Offset, const QColor &backColor, const QRect &drawrect);

