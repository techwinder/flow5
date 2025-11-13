/****************************************************************************

    flow5
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
#include <QPainter>

class Foil;
class Polar;

void drawFoil(QPainter &painter, const Foil *pFoil, double alpha, double twist, double scalex, double scaley, QPointF const &Offset, bool bFill=false, QColor fillClr=Qt::gray);
void drawFoilNormals(QPainter &painter, Foil const *pFoil, double alpha, double scalex, double scaley, QPointF const &Offset);
void drawFoilMidLine(QPainter &painter, Foil const *pFoil, double scalex, double scaley, QPointF const &Offset);
void drawFoilPoints(QPainter &painter, Foil const *pFoil, double alpha, double scalex, double scaley, QPointF const &Offset, const QColor &backColor, const QRect &drawrect);


bool readFoilFile(QFile &FoilFile, Foil *pFoil);
bool readPolarFile(QFile &plrFile, QVector<Foil*> &foilList, QVector<Polar*> &polarList);
Polar *importXFoilPolar(QFile & txtFile, QString &logmsg);

bool serializeFoil(Foil*pFoil, QDataStream &ar, bool bIsStoring);
bool serializePolarv6(Polar *pPolar, QDataStream &ar, bool bIsStoring);

