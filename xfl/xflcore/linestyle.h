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

#include <QMetaType>
#include <QColor>
#include <QSettings>
#include <QDataStream>


#define NPOINTSTYLES   15
#define NLINESTYLES    6
#define NLINEWIDTHS    10

namespace Line
{
    /** @enum The different applications available to the user */
    enum enumLineStipple {SOLID, DASH, DOT, DASHDOT, DASHDOTDOT, NOLINE};

    /**< @enum The different image formats usable to export screen captures*/
    enum enumPointStyle{NOSYMBOL,
                  LITTLECIRCLE,   BIGCIRCLE, LITTLESQUARE, BIGSQUARE, TRIANGLE, TRIANGLE_INV,
                  LITTLECIRCLE_F, BIGCIRCLE_F, LITTLESQUARE_F, BIGSQUARE_F, TRIANGLE_F, TRIANGLE_INV_F,
                  LITTLECROSS, BIGCROSS};
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
        m_Color = Qt::gray;
        m_Symbol=Line::NOSYMBOL;
        m_Tag = QString();
    }


    LineStyle(bool bVisible, Line::enumLineStipple style, int width, QColor color,  Line::enumPointStyle pointstyle, QString LineTag=QString(), bool bEnabled=true)
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

    void setStipple(int n) {m_Stipple = convertLineStyle(n);}
    void setPointStyle(int n) {m_Symbol = convertPointStyle_old(n);}

    bool m_bIsEnabled=true;
    bool m_bIsVisible=true;
    bool m_bIsHighlighted=false;
    Line::enumLineStipple m_Stipple= Line::SOLID;
    int m_Width=1;
    QColor m_Color=Qt::gray;

    Line::enumPointStyle m_Symbol=Line::NOSYMBOL;

    QString m_Tag=QString();


    Qt::PenStyle getStipple()
    {
         switch(m_Stipple)
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
            case 1:  return Line::LITTLECIRCLE;
            case 2:  return Line::BIGCIRCLE;
            case 3:  return Line::LITTLESQUARE;
            case 4:  return Line::BIGSQUARE;
            case 5:  return Line::TRIANGLE;
            case 6:  return Line::LITTLECIRCLE_F;
            case 7:  return Line::BIGCIRCLE_F;
            case 8:  return Line::LITTLESQUARE_F;
            case 9:  return Line::BIGSQUARE_F;
            case 10: return Line::TRIANGLE_F;
            case 11: return Line::LITTLECROSS;
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
            case 1:  return Line::LITTLECIRCLE;
            case 2:  return Line::BIGCIRCLE;
            case 3:  return Line::LITTLESQUARE;
            case 4:  return Line::BIGSQUARE;
            case 5:  return Line::TRIANGLE;
            case 6:  return Line::TRIANGLE_INV;
            case 7:  return Line::LITTLECIRCLE_F;
            case 8:  return Line::BIGCIRCLE_F;
            case 9:  return Line::LITTLESQUARE_F;
            case 10: return Line::BIGSQUARE_F;
            case 11: return Line::TRIANGLE_F;
            case 12: return Line::TRIANGLE_INV_F;
            case 13: return Line::LITTLECROSS;
            case 14: return Line::BIGCROSS;
        }
    }


    static int convertSymbol(Line::enumPointStyle ptStyle)
    {
        switch (ptStyle)
        {
            case Line::NOSYMBOL:        return 0;
            case Line::LITTLECIRCLE:    return 1;
            case Line::BIGCIRCLE:       return 2;
            case Line::LITTLESQUARE:    return 3;
            case Line::BIGSQUARE:       return 4;
            case Line::TRIANGLE:        return 5;
            case Line::TRIANGLE_INV:    return 6;
            case Line::LITTLECIRCLE_F:  return 7;
            case Line::BIGCIRCLE_F:     return 8;
            case Line::LITTLESQUARE_F:  return 9;
            case Line::BIGSQUARE_F:     return 10;
            case Line::TRIANGLE_F:      return 11;
            case Line::TRIANGLE_INV_F:  return 12;
            case Line::LITTLECROSS:     return 13;
            case Line::BIGCROSS:        return 14;
        }
        return 0;
    }




    void serializeXfl(QDataStream &ar, bool bIsStoring)
    {
        int k=0;
        if(bIsStoring)
        {
            ar << convertLineStyle(m_Stipple);
            ar << m_Width;
            ar << convertSymbol(m_Symbol);
            ar << m_Color;
            ar << m_bIsVisible;
        }
        else
        {
            ar >> k; m_Stipple=convertLineStyle(k);
            ar >> m_Width;
            ar >> k; m_Symbol=convertSymbol(k);
            ar >> m_Color;
            ar >> m_bIsVisible;
        }
    }

    void serializeFl5(QDataStream &ar, bool bIsStoring)
    {
        int k=0;
        int ArchiveFormat = 500001;
        if(bIsStoring)
        {
            ar << ArchiveFormat;
            ar << LineStyle::convertLineStyle(m_Stipple);
            ar << m_Width;
            ar << LineStyle::convertSymbol(m_Symbol);
            ar << m_Color;
            ar << m_bIsVisible;
            ar << m_Tag;
        }
        else
        {
            ar >> k;
            if(k<500001)
            {
                // --> v712 format
                m_Stipple=LineStyle::convertLineStyle(k);
                ar >> m_Width;
                ar >> k; m_Symbol=LineStyle::convertPointStyle_old(k);
                ar >> m_Color;
                ar >> m_bIsVisible;
                ar >> m_Tag;
            }
            else
            {
                // v713+ format
                ar >> k; m_Stipple=LineStyle::convertLineStyle(k);
                ar >> m_Width;
                ar >> k; m_Symbol=LineStyle::convertSymbol(k);
                ar >> m_Color;
                ar >> m_bIsVisible;
                ar >> m_Tag;

            }
        }
    }


    void loadSettings(QSettings &settings, QString const &name)
    {
        if(settings.contains(name+"_visible")) m_bIsVisible = settings.value(name+"_visible", true).toBool();
        if(settings.contains(name+"_color"))   m_Color      = settings.value(name+"_color", QColor(205,205,205)).value<QColor>();
        if(settings.contains(name+"_width"))   m_Width      = settings.value(name+"_width", 1).toInt();
        if(settings.contains(name+"_tag"))     m_Tag        = settings.value(name+"_tag", QString()).toString();

        if(settings.contains(name+"_line"))
        {
            int istyle = settings.value(name+"_line", 0).toInt();
            switch (istyle)
            {
                default:
                case 0: m_Stipple = Line::SOLID; break;
                case 1: m_Stipple = Line::DASH; break;
                case 2: m_Stipple = Line::DOT; break;
                case 3: m_Stipple = Line::DASHDOT; break;
                case 4: m_Stipple = Line::DASHDOTDOT; break;
                case 5: m_Stipple = Line::NOLINE; break;
            }
        }
        if(settings.contains(name+"_pts"))
        {
            int ipts = settings.value(name+"_pts", 0).toInt();
            m_Symbol = convertSymbol(ipts);
        }
    }


    void saveSettings(QSettings &settings, QString const &name) const
    {
        settings.setValue(name+"_visible", m_bIsVisible);
        settings.setValue(name+"_color", m_Color);
        settings.setValue(name+"_width", m_Width);
        settings.setValue(name+"_tag", m_Tag);

        switch (m_Stipple)
        {
            case Line::SOLID:      settings.setValue(name+"_line", 0);  break;
            case Line::DASH:       settings.setValue(name+"_line", 1);  break;
            case Line::DOT:        settings.setValue(name+"_line", 2);  break;
            case Line::DASHDOT:    settings.setValue(name+"_line", 3);  break;
            case Line::DASHDOTDOT: settings.setValue(name+"_line", 4);  break;
            case Line::NOLINE:     settings.setValue(name+"_line", 5);  break;
        }

        int ist = convertSymbol(m_Symbol);
        settings.setValue(name+"_pts", ist);

    }
};

// allow use as QVariant
Q_DECLARE_METATYPE(LineStyle)


