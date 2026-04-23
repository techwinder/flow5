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


#include "xflsvgwriter.h"

#include <core/saveoptions.h>
#include <core/xflcore.h>

int XflSvgWriter::s_RefFontSize = 10;

XflSvgWriter::XflSvgWriter(QFile &XFile): QXmlStreamWriter()
{
    setDevice(&XFile);
    setAutoFormatting(true);
}


void XflSvgWriter::writeText(int x, int y, QFont const &font, QColor const &clr, QString const &text)
{
    writeStartElement("text");
    {
        writeAttribute("x", QString::asprintf("%d", x));
        writeAttribute("y", QString::asprintf("%d", y));
        writeAttribute("font-family", fontFamilyExt(font));
        writeAttribute("font-size", QString::asprintf("%.2fem", double(font.pointSize())/double(s_RefFontSize)));
        if(font.italic())
            writeAttribute("font-style", "italic");
        if(font.bold())
            writeAttribute("font-weight", "bold");
        writeAttribute("fill", clr.name());
        writeCharacters(text);
    }
    writeEndElement();
}


QString XflSvgWriter::fontFamilyExt(QFont const &font)
{
    QString strange = font.family();
    switch(font.styleHint())
    {
        case QFont::AnyStyle:
        case QFont::SansSerif:        strange += ", Arial, Calibri, Tahoma, Segoe UI, sans-serif";                     break;
        case QFont::Monospace:        strange += ", Courier, Courier New, Lucida Console, Monaco, sans-serif";         break;
        case QFont::TypeWriter:       strange += ", Courier, Courier New, Lucida Console, Monaco, sans-serif";         break;
        case QFont::Serif:            strange += ", Times New Roman, Courier New, Lucida Console, Monaco, sans-serif"; break;
        case QFont::OldEnglish:
        case QFont::Fantasy:
        case QFont::Cursive:
        case QFont::System:           strange += ", Courier, Courier New, Lucida Console, Monaco, sans-serif";         break;
    }

    return strange;
}

/*
void XflSvgWriter::writeLine(QPoint const& from, QPoint const &to, LineStyle const &ls)
{
    writeLine(from.x(), from.y(), to.x(), to.y(), ls.m_Color, ls.m_Width, ls.m_Stipple);
}*/


void XflSvgWriter::writeLine(QPointF const& from, QPointF const &to, LineStyle const &ls)
{
    writeLine(from.x(), from.y(), to.x(), to.y(), xfl::fromfl5Clr(ls.m_Color), ls.m_Width, ls.m_Stipple);
}


void XflSvgWriter::writeLine(QPoint const& from, QPoint const &to, QColor const &clr, int w, Line::enumLineStipple stip)
{
    writeLine(from.x(), from.y(), to.x(), to.y(), clr, w, stip);
}


void XflSvgWriter::writeLine(double x0, double y0, double x1, double y1, LineStyle const &ls)
{
    writeLine(x0, y0, x1, y1, xfl::fromfl5Clr(ls.m_Color), ls.m_Width, ls.m_Stipple);
}


void XflSvgWriter::writeLine(double x0, double y0, double x1, double y1, QColor const &clr, int w, Line::enumLineStipple stip)
{
    writeStartElement("line");
    {
        writeAttribute("x1", QString::asprintf("%g",x0));
        writeAttribute("y1", QString::asprintf("%g",y0));
        writeAttribute("x2", QString::asprintf("%g",x1));
        writeAttribute("y2", QString::asprintf("%g",y1));
        writeLineAttributes(clr, w, stip);
    }
    writeEndElement(); //line
}

void XflSvgWriter::writeLineAttributes(LineStyle const &ls, bool bFill, QColor fillclr)
{
    writeLineAttributes(xfl::fromfl5Clr(ls.m_Color), ls.m_Width, ls.m_Stipple, bFill, fillclr);
}

void XflSvgWriter::writeLineAttributes(QColor const &clr, int w, Line::enumLineStipple stip, bool bFill, QColor fillclr)
{
    QString strange;
    if(!bFill) strange = "fill:none; ";
    else       strange = "fill:"+fillclr.name()+"; ";
    writeAttribute("style", strange + "stroke:"+clr.name()+"; stroke-width:"+QString::asprintf("%d",w));

    switch (stip)
    {
        case Line::DASH:
            writeAttribute("stroke-dasharray", "5 2");
            break;
        case Line::DOT:
            writeAttribute("stroke-dasharray", "2 2");
            break;
        case Line::DASHDOT:
            writeAttribute("stroke-dasharray", "5 2 2 2");
            break;
        case Line::DASHDOTDOT:
            writeAttribute("stroke-dasharray", "5 2 2 2 2 2");
            break;
        default:
        case Line::SOLID:
            break;
    }
}


void XflSvgWriter::writePolyLine(QPolygonF const &line, LineStyle const &ls)
{
    writeStartElement("polyline");
    {
        QString strange;
        for(int i=0; i<line.size(); i++)
        {
            strange += QString::asprintf("%g,%g ", line.at(i).x(), line.at(i).y());
        }
        writeAttribute("points", strange);
        writeLineAttributes(ls);
    }
    writeEndElement();
}



void XflSvgWriter::writePoint(double x, double y, LineStyle const &ls, QColor const &backcolor)
{
    switch(ls.m_Symbol)
    {
        case Line::NOSYMBOL: break;
        case Line::LITTLECIRCLE:
        {
            double ptSide = 3.0;
            writeStartElement("circle");
            {
                writeAttribute("cx", QString::asprintf("%d",int(x)));
                writeAttribute("cy", QString::asprintf("%d",int(y)));
                writeAttribute("r", QString::asprintf("%d", int(ptSide)));
                writeAttribute("fill", backcolor.name());
                writeLineAttributes(ls, true, backcolor);
            }
            writeEndElement();
            break;
        }
        case Line::LITTLECIRCLE_F:
        {
            double ptSide = 3.0;
            writeStartElement("circle");
            {
                writeAttribute("cx",  QString::asprintf("%d",int(x)));
                writeAttribute("cy",  QString::asprintf("%d",int(y)));
                writeAttribute("r",   QString::asprintf("%d", int(ptSide)));
                writeLineAttributes(ls, true, xfl::fromfl5Clr(ls.m_Color));
            }
            writeEndElement();
            break;
        }
        case Line::BIGCIRCLE:
        {
            double ptSide = 5.0;
            writeStartElement("circle");
            {
                writeAttribute("cx", QString::asprintf("%d",int(x)));
                writeAttribute("cy", QString::asprintf("%d",int(y)));
                writeAttribute("r", QString::asprintf("%d", int(ptSide)));
                writeAttribute("fill", backcolor.name());
                writeLineAttributes(ls, true, backcolor);
            }
            writeEndElement();
            break;
        }
        case Line::BIGCIRCLE_F:
        {
            double ptSide = 5.0;
            writeStartElement("circle");
            {
                writeAttribute("cx", QString::asprintf("%d",int(x)));
                writeAttribute("cy", QString::asprintf("%d",int(y)));
                writeAttribute("r", QString::asprintf("%d", int(ptSide)));
                writeLineAttributes(ls, true, xfl::fromfl5Clr(ls.m_Color));
            }
            writeEndElement();
            break;
        }
        case Line::LITTLESQUARE:
        {
            double ptSide = 4.0;
            writeStartElement("rect");
            {
                writeAttribute("x", QString::asprintf("%d",int(x-ptSide/2)));
                writeAttribute("y", QString::asprintf("%d",int(y-ptSide/2)));
                writeAttribute("width", QString::asprintf("%d", int(ptSide)));
                writeAttribute("height", QString::asprintf("%d", int(ptSide)));
                writeAttribute("fill", backcolor.name());
                writeLineAttributes(ls, true, backcolor);
            }
            writeEndElement();
            break;
        }
        case Line::LITTLESQUARE_F:
        {
            double ptSide = 4.0;
            writeStartElement("rect");
            {
                writeAttribute("x", QString::asprintf("%d",int(x-ptSide/2)));
                writeAttribute("y", QString::asprintf("%d",int(y-ptSide/2)));
                writeAttribute("width", QString::asprintf("%d", int(ptSide)));
                writeAttribute("height", QString::asprintf("%d", int(ptSide)));
                writeLineAttributes(ls, true, xfl::fromfl5Clr(ls.m_Color));
            }
            writeEndElement();
            break;
        }
        case Line::BIGSQUARE:
        {
            double ptSide = 8.0;
            writeStartElement("rect");
            {
                writeAttribute("x", QString::asprintf("%d",int(x-ptSide/2)));
                writeAttribute("y", QString::asprintf("%d",int(y-ptSide/2)));
                writeAttribute("width", QString::asprintf("%d", int(ptSide)));
                writeAttribute("height", QString::asprintf("%d", int(ptSide)));
                writeAttribute("fill", backcolor.name());
                writeLineAttributes(ls, true, backcolor);
            }
            writeEndElement();
            break;
        }
        case Line::BIGSQUARE_F:
        {
            double ptSide = 8.0;
            writeStartElement("rect");
            {
                writeAttribute("x", QString::asprintf("%d",int(x-ptSide/2)));
                writeAttribute("y", QString::asprintf("%d",int(y-ptSide/2)));
                writeAttribute("width", QString::asprintf("%d", int(ptSide)));
                writeAttribute("height", QString::asprintf("%d", int(ptSide)));
                writeLineAttributes(ls, true, xfl::fromfl5Clr(ls.m_Color));
            }
            writeEndElement();
            break;
        }
        case Line::TRIANGLE:
        {
            double ptSide = 5;

            const QPointF points[3] = {
                QPointF(x-ptSide, y-ptSide),
                QPointF(x,  y+ptSide),
                QPointF(x+ptSide, y-ptSide),
            };

            writeStartElement("polygon");
            {
                QString strange;
                for(int i=0; i<3; i++) strange += QString::asprintf("%d,%d ", int(points[i].x()), int(points[i].y()));
                writeAttribute("points", strange);
                writeAttribute("fill", backcolor.name());
                writeLineAttributes(ls, true, backcolor);
            }
            writeEndElement();
            break;
        }
        case Line::TRIANGLE_INV:
        {
            double ptSide = 5;

            const QPointF points[3] = {
                QPointF(x-ptSide, y+ptSide),
                QPointF(x,  y-ptSide),
                QPointF(x+ptSide, y+ptSide),
            };

            writeStartElement("polygon");
            {
                QString strange;
                for(int i=0; i<3; i++) strange += QString::asprintf("%d,%d ", int(points[i].x()), int(points[i].y()));
                writeAttribute("points", strange);
                writeAttribute("fill", backcolor.name());
                writeLineAttributes(ls, true, backcolor);
            }
            writeEndElement();
            break;
        }
        case Line::TRIANGLE_F:
        {
            double ptSide = 5;

            const QPointF points[3] = {
                QPointF(x-ptSide, y-ptSide),
                QPointF(x,  y+ptSide),
                QPointF(x+ptSide, y-ptSide),
            };

            writeStartElement("polygon");
            {
                QString strange;
                for(int i=0; i<3; i++) strange += QString::asprintf("%d,%d ", int(points[i].x()), int(points[i].y()));
                writeAttribute("points", strange);
                writeLineAttributes(ls, true, xfl::fromfl5Clr(ls.m_Color));
            }
            writeEndElement();
            break;
        }
        case Line::TRIANGLE_INV_F:
        {
            double ptSide = 5;

            const QPointF points[3] = {
                QPointF(x-ptSide, y+ptSide),
                QPointF(x,  y-ptSide),
                QPointF(x+ptSide, y+ptSide),
            };

            writeStartElement("polygon");
            {
                QString strange;
                for(int i=0; i<3; i++) strange += QString::asprintf("%d,%d ", int(points[i].x()), int(points[i].y()));
                writeAttribute("points", strange);
                writeLineAttributes(ls, true, xfl::fromfl5Clr(ls.m_Color));
            }
            writeEndElement();
            break;
        }
        case Line::LITTLECROSS:
        {
            int ptSide = 3;
            writeLine(int(x)-ptSide, int(y)-ptSide, int(x)+ptSide, int(y)+ptSide, ls);
            writeLine(int(x)-ptSide, int(y)+ptSide, int(x)+ptSide, int(y)-ptSide, ls);
            break;
        }
        case Line::BIGCROSS:
        {
            int ptSide = 4;
            writeLine(int(x)-ptSide, int(y)-ptSide, int(x)+ptSide, int(y)+ptSide, ls);
            writeLine(int(x)-ptSide, int(y)+ptSide, int(x)+ptSide, int(y)-ptSide, ls);
            break;
        }
//        default: break;
    }
}
