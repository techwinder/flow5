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

#include <QXmlStreamReader>
#include <QFile>
#include <QColor>

#include <api/linestyle.h>

class XflSvgWriter : public QXmlStreamWriter
{
    public:
        XflSvgWriter(QFile &XFile);

        static void setRefFontSize(int size) {s_RefFontSize=size;}
        static int refFontSize() {return s_RefFontSize;}

    protected:
        void writeText(int x, int y, QFont const &font, const QColor &clr, QString const &text);
//        void writeLine(QPoint const& from, QPoint const &to, LineStyle const &ls);
        void writeLine(QPointF const& from, QPointF const &to, LineStyle const &ls);
        void writeLine(QPoint const& from, QPoint const &to, QColor const &clr, int w, Line::enumLineStipple stip);
        void writeLine(double x0, double y0, double x1, double y1,       QColor const &clr, int w, Line::enumLineStipple stip);
        void writeLine(double x0, double y0, double x1, double y1,       LineStyle const &ls);
        void writeLineAttributes(LineStyle const &ls, bool bFill=false, QColor fillclr=Qt::white);
        void writeLineAttributes(QColor const &clr, int w, Line::enumLineStipple stip, bool bFill=false, QColor fillclr=Qt::white);
        void writePoint(double x, double y, const LineStyle &ls, const QColor &backcolor);
        void writePolyLine(QPolygonF const &line, LineStyle const &ls);


    private:
        QString fontFamilyExt( QFont const &font);

        static int s_RefFontSize;
};

