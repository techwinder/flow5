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

#pragma once

#include<string>

#include <fl5color.h>

#define NPOINTSTYLES   15
#define NLINESTYLES    6
#define NLINEWIDTHS    10

namespace Line
{
    enum enumLineStipple {SOLID, DASH, DOT, DASHDOT, DASHDOTDOT, NOLINE};

    enum enumPointStyle{NOSYMBOL,
                        SMALLCIRCLE,   BIGCIRCLE, SMALLSQUARE, BIGSQUARE, TRIANGLE, TRIANGLE_INV,
                        SMALLCIRCLE_FILLED, BIGCIRCLE_FILLED, SMALLSQUARE_FILLED, BIGSQUARE_FILLED, TRIANGLE_FILLED, TRIANGLE_INV_FILLED,
                        SMALLCROSS, BIGCROSS};
}


struct LineStyle
{
    LineStyle()
    {
        m_bIsVisible=true;
        m_bIsEnabled=true;
        m_bIsHighlighted=false;
        m_Stipple=Line::SOLID;
        m_Width=1;
        m_Color = {200,200,200};
        m_Symbol=Line::NOSYMBOL;
        m_Tag.clear();
    }


    LineStyle(LineStyle const &ls)
    {
        copy(ls);
    }

    LineStyle(bool bVisible, Line::enumLineStipple style, int width, fl5Color const &color,
              Line::enumPointStyle pointstyle, std::string LineTag="", bool bEnabled=true)
    {
        m_bIsEnabled = bEnabled;
        m_bIsVisible = bVisible;
        m_bIsHighlighted = false;
        m_Stipple = style;
        m_Width = width;
        m_Color = color;
        m_Symbol = pointstyle;
        m_Tag = LineTag;
    }

    ~LineStyle()
    {
    }

    void copy(LineStyle const &ls)
    {
        m_bIsEnabled = ls.m_bIsEnabled;
        m_bIsVisible = ls.m_bIsVisible;
        m_bIsHighlighted = ls.m_bIsHighlighted;
        m_Stipple = ls.m_Stipple;
        m_Width = ls.m_Width;
        m_Color = ls.m_Color;
        m_Symbol = ls.m_Symbol;
        m_Tag = ls.m_Tag;
    }


    void operator = (LineStyle const &ls)
    {
        copy(ls);
    }


    void setStipple(int n) {m_Stipple = convertLineStyle(n);}
    void setWidth(int n) {m_Width = n;}
    void setPointStyle(int n) {m_Symbol = convertPointStyle_old(n);}
    void setColor(fl5Color const &clr) {m_Color=clr;}
    void setEnabled(bool b) {m_bIsEnabled=b;}
    void setVisible(bool b) {m_bIsVisible=b;}
    void setHighlighted(bool b) {m_bIsHighlighted=b;}

    bool m_bIsEnabled=true;
    bool m_bIsVisible=true;
    bool m_bIsHighlighted=false;
    Line::enumLineStipple m_Stipple= Line::SOLID;
    int m_Width=1;
    fl5Color m_Color = {200,200,200};

    Line::enumPointStyle m_Symbol=Line::NOSYMBOL;

    std::string m_Tag="";

    static Line::enumLineStipple convertLineStyle(int iStipple)
    {
        switch (iStipple)
        {
            default:
            case 0: return Line::SOLID;
            case 1: return Line::DASH;
            case 2: return Line::DOT;
            case 3: return Line::DASHDOT;
            case 4: return Line::DASHDOTDOT;
            case 5: return Line::NOLINE;
        }
    }


    static int convertLineStyle(Line::enumLineStipple style)
    {
        switch (style)
        {
            case Line::SOLID:      return 0;
            case Line::DASH:       return 1;
            case Line::DOT:        return 2;
            case Line::DASHDOT:    return 3;
            case Line::DASHDOTDOT: return 4;
            case Line::NOLINE:     return 5;
        }
        return 0;
    }

    // -->v712
    static Line::enumPointStyle convertPointStyle_old(int iStyle)
    {
        switch (iStyle)
        {
            default:
            case 0:  return Line::NOSYMBOL;
            case 1:  return Line::SMALLCIRCLE;
            case 2:  return Line::BIGCIRCLE;
            case 3:  return Line::SMALLSQUARE;
            case 4:  return Line::BIGSQUARE;
            case 5:  return Line::TRIANGLE;
            case 6:  return Line::SMALLCIRCLE_FILLED;
            case 7:  return Line::BIGCIRCLE_FILLED;
            case 8:  return Line::SMALLSQUARE_FILLED;
            case 9:  return Line::BIGSQUARE_FILLED;
            case 10: return Line::TRIANGLE_FILLED;
            case 11: return Line::SMALLCROSS;
            case 12: return Line::BIGCROSS;
        }
    }

    // v713+
    static Line::enumPointStyle convertSymbol(int iStyle)
    {
        switch (iStyle)
        {
            default:
            case 0:  return Line::NOSYMBOL;
            case 1:  return Line::SMALLCIRCLE;
            case 2:  return Line::BIGCIRCLE;
            case 3:  return Line::SMALLSQUARE;
            case 4:  return Line::BIGSQUARE;
            case 5:  return Line::TRIANGLE;
            case 6:  return Line::TRIANGLE_INV;
            case 7:  return Line::SMALLCIRCLE_FILLED;
            case 8:  return Line::BIGCIRCLE_FILLED;
            case 9:  return Line::SMALLSQUARE_FILLED;
            case 10: return Line::BIGSQUARE_FILLED;
            case 11: return Line::TRIANGLE_FILLED;
            case 12: return Line::TRIANGLE_INV_FILLED;
            case 13: return Line::SMALLCROSS;
            case 14: return Line::BIGCROSS;
        }
    }


    static int convertSymbol(Line::enumPointStyle ptStyle)
    {
        switch (ptStyle)
        {
            case Line::NOSYMBOL:        return 0;
            case Line::SMALLCIRCLE:    return 1;
            case Line::BIGCIRCLE:       return 2;
            case Line::SMALLSQUARE:    return 3;
            case Line::BIGSQUARE:       return 4;
            case Line::TRIANGLE:        return 5;
            case Line::TRIANGLE_INV:    return 6;
            case Line::SMALLCIRCLE_FILLED:  return 7;
            case Line::BIGCIRCLE_FILLED:     return 8;
            case Line::SMALLSQUARE_FILLED:  return 9;
            case Line::BIGSQUARE_FILLED:     return 10;
            case Line::TRIANGLE_FILLED:      return 11;
            case Line::TRIANGLE_INV_FILLED:  return 12;
            case Line::SMALLCROSS:     return 13;
            case Line::BIGCROSS:        return 14;
        }
        return 0;
    }

};

