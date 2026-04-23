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

#include <QString>
#include <QFont>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QSettings>

// We need to make a data structure for QFonts because static QFont variables cannot be
// initialized before the GUI is constructed
// From the Qt dochttps://doc.qt.io/qt-5/qfont.html:
//     Note that a QGuiApplication instance must exist before a QFont can be used.
// This causes errors when linking statically to the Qt libs

struct FontStruct
{
    FontStruct()
    {
        // useless, returns an empty font due to static initialization sequence
        m_Family = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();
        m_PtSize = 12;
        m_Weight = QFont::Medium;
        m_bItalic = false;
        m_StyleHint = QFont::AnyStyle;
    }

    FontStruct(QString const &fam, int s, QFont::Weight w, bool bIt, QFont::StyleHint style) : m_Family(fam), m_PtSize(s), m_Weight(w), m_bItalic(bIt)
    {
        if(fam.length()) m_Family = fam;
        else             m_Family = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();

        m_PtSize = s;
        m_Weight = w;
        m_bItalic = bIt;
        m_StyleHint = style;
    }

    QFont font() const
    {
        QFont fnt(m_Family, m_PtSize, m_Weight, m_bItalic);
        fnt.setStyleHint(m_StyleHint);
        fnt.setFamily(m_Family);
        return fnt;
    }

    void setFont(QFont const &fnt, QFont::StyleHint hint)
    {
        m_Family    = fnt.family();
        m_PtSize    = fnt.pointSize();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        switch(fnt.weight())
        {
            case 0:  m_Weight = QFont::Thin;       break;
            case 12: m_Weight = QFont::ExtraLight; break;
            case 25: m_Weight = QFont::Light;      break;
            default:
            case 50: m_Weight = QFont::Normal;     break;
            case 57: m_Weight = QFont::Medium;     break;
            case 63: m_Weight = QFont::DemiBold;   break;
            case 75: m_Weight = QFont::Bold;       break;
            case 81: m_Weight = QFont::ExtraBold;  break;
            case 87: m_Weight = QFont::Black;      break;
        }
#else
        m_Weight    = fnt.weight();
#endif

        m_bItalic   = fnt.italic();
        m_StyleHint = hint;
    }

    void loadSettings(QSettings &settings, QString const & groupname)
    {
        settings.beginGroup(groupname);
        {
            if(settings.contains("Family"))
                m_Family = settings.value("Family", QString()).toString();
            if(settings.contains("PointSize"))
                m_PtSize = settings.value("PointSize", 11).toInt();
            if(settings.contains("Weight"))
            {
                int w = settings.value("Weight", 3).toInt();
                switch(w)
                {
                    case 0: m_Weight = QFont::Thin;       break;
                    case 1: m_Weight = QFont::ExtraLight; break;
                    case 2: m_Weight = QFont::Light;      break;
                    default:
                    case 3: m_Weight = QFont::Normal;     break;
                    case 4: m_Weight = QFont::Medium;     break;
                    case 5: m_Weight = QFont::DemiBold;   break;
                    case 6: m_Weight = QFont::Bold;       break;
                    case 7: m_Weight = QFont::ExtraBold;  break;
                    case 8: m_Weight = QFont::Black;      break;
                }
            }
            if(settings.contains("bItalic"))
                m_bItalic = settings.value("bItalic", false).toBool();
            int hint = settings.value("StyleHint", QFont::AnyStyle).toInt();
            switch(hint)
            {
                case 0: m_StyleHint = QFont::SansSerif;       break;
                case 1: m_StyleHint = QFont::Serif;           break;
                case 2: m_StyleHint = QFont::TypeWriter;      break;
                case 3: m_StyleHint = QFont::OldEnglish;      break;
                case 4: m_StyleHint = QFont::System;          break;
                default:
                case 5: m_StyleHint = QFont::AnyStyle;        break;
                case 6: m_StyleHint = QFont::Cursive;         break;
                case 7: m_StyleHint = QFont::Monospace;       break;
                case 8: m_StyleHint = QFont::Fantasy;         break;
            }
        }
        settings.endGroup();
    }

    void saveSettings(QSettings &settings, QString const & groupname)
    {
        settings.beginGroup(groupname);
        {
            settings.setValue("Family",    m_Family);
            settings.setValue("PointSize", m_PtSize);
            int w=3;
            switch(m_Weight)
            {
                case QFont::Thin:       w=0;   break;
                case QFont::ExtraLight: w=1;   break;
                case QFont::Light:      w=2;   break;
                case QFont::Normal:     w=3;   break;
                case QFont::Medium:     w=4;   break;
                case QFont::DemiBold:   w=5;   break;
                case QFont::Bold:       w=6;   break;
                case QFont::ExtraBold:  w=7;   break;
                case QFont::Black:      w=8;   break;
            }
            settings.setValue("Weight", w);
            settings.setValue("bItalic",   m_bItalic);
            settings.setValue("StyleHint", m_StyleHint);
        }
        settings.endGroup();
    }

    QString const &family() const {return m_Family;}
    bool bItalic() const {return m_bItalic;}
    QFont::Weight weight() const {return m_Weight;}

    int pointSize() const {return m_PtSize;}
    int height() const {return QFontMetrics(font()).height();}
    int averageCharWidth() const {return QFontMetrics(font()).averageCharWidth();}
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int width(QString const & string) const
    {
        return QFontMetrics(font()).horizontalAdvance(string);
    }
#else
    int width(QString const & string) const {return QFontMetrics(font()).width(string);}
#endif
    QString m_Family;
    int m_PtSize=-1;
    QFont::Weight m_Weight=QFont::Medium;
    bool m_bItalic = false;
    QFont::StyleHint m_StyleHint; //useful to export to SVG
};

