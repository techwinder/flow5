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

#include <QDebug>
#include <QApplication>
#include <QPalette>
#include <QToolTip>

#include "displayoptions.h"


QColor DisplayOptions::g_BackgroundColor2d(QColor(237, 237, 237));
QColor DisplayOptions::g_TextColor(Qt::black);


FontStruct DisplayOptions::g_TextFontStruct;
FontStruct DisplayOptions::g_TableFontStruct;
FontStruct DisplayOptions::g_TreeFontStruct;
FontStruct DisplayOptions::g_ToolTipFontStruct;

DisplayOptions::enumThemeType DisplayOptions::g_Theme = DisplayOptions::LIGHTTHEME;

bool DisplayOptions::g_bStyleSheet = false;


double DisplayOptions::g_ScaleFactor = 0.07;
double DisplayOptions::g_IconSize = 32;


void DisplayOptions::setTextFont(QFont const & fnt)
{
    g_TextFontStruct.m_Family = fnt.family();
    g_TextFontStruct.m_PtSize = fnt.pointSize();
    g_TextFontStruct.m_bItalic = fnt.italic();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    switch(fnt.weight())
    {
        case 0:  g_TextFontStruct.m_Weight = QFont::Thin;       break;
        case 12: g_TextFontStruct.m_Weight = QFont::ExtraLight; break;
        case 25: g_TextFontStruct.m_Weight = QFont::Light;      break;
        default:
        case 50: g_TextFontStruct.m_Weight = QFont::Normal;     break;
        case 57: g_TextFontStruct.m_Weight = QFont::Medium;     break;
        case 63: g_TextFontStruct.m_Weight = QFont::DemiBold;   break;
        case 75: g_TextFontStruct.m_Weight = QFont::Bold;       break;
        case 81: g_TextFontStruct.m_Weight = QFont::ExtraBold;  break;
        case 87: g_TextFontStruct.m_Weight = QFont::Black;      break;
    }
#else
    g_TextFontStruct.m_Weight    = fnt.weight();
#endif
    g_TextFontStruct.m_StyleHint = fnt.styleHint();
}


void DisplayOptions::setTableFont(QFont const & fnt)
{
    g_TableFontStruct.m_Family = fnt.family();
    g_TableFontStruct.m_PtSize = fnt.pointSize();
    g_TableFontStruct.m_bItalic = fnt.italic();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    switch(fnt.weight())
    {
        case 0:  g_TableFontStruct.m_Weight = QFont::Thin;       break;
        case 12: g_TableFontStruct.m_Weight = QFont::ExtraLight; break;
        case 25: g_TableFontStruct.m_Weight = QFont::Light;      break;
        default:
        case 50: g_TableFontStruct.m_Weight = QFont::Normal;     break;
        case 57: g_TableFontStruct.m_Weight = QFont::Medium;     break;
        case 63: g_TableFontStruct.m_Weight = QFont::DemiBold;   break;
        case 75: g_TableFontStruct.m_Weight = QFont::Bold;       break;
        case 81: g_TableFontStruct.m_Weight = QFont::ExtraBold;  break;
        case 87: g_TableFontStruct.m_Weight = QFont::Black;      break;
    }
#else
    g_TableFontStruct.m_Weight    = fnt.weight();
#endif
    g_TableFontStruct.m_StyleHint = fnt.styleHint();
}


void DisplayOptions::setTreeFont(QFont const & fnt)
{
    g_TreeFontStruct.m_Family = fnt.family();
    g_TreeFontStruct.m_PtSize = fnt.pointSize();
    g_TreeFontStruct.m_bItalic = fnt.italic();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    switch(fnt.weight())
    {
        case 0:  g_TreeFontStruct.m_Weight = QFont::Thin;       break;
        case 12: g_TreeFontStruct.m_Weight = QFont::ExtraLight; break;
        case 25: g_TreeFontStruct.m_Weight = QFont::Light;      break;
        default:
        case 50: g_TreeFontStruct.m_Weight = QFont::Normal;     break;
        case 57: g_TreeFontStruct.m_Weight = QFont::Medium;     break;
        case 63: g_TreeFontStruct.m_Weight = QFont::DemiBold;   break;
        case 75: g_TreeFontStruct.m_Weight = QFont::Bold;       break;
        case 81: g_TreeFontStruct.m_Weight = QFont::ExtraBold;  break;
        case 87: g_TableFontStruct.m_Weight = QFont::Black;      break;
    }
#else
    g_TreeFontStruct.m_Weight    = fnt.weight();
#endif
    g_TreeFontStruct.m_StyleHint = fnt.styleHint();
}


void DisplayOptions::setToolTipFont(QFont const & fnt)
{
    g_ToolTipFontStruct.m_Family = fnt.family();
    g_ToolTipFontStruct.m_PtSize = fnt.pointSize();
    g_ToolTipFontStruct.m_bItalic = fnt.italic();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    switch(fnt.weight())
    {
        case 0:  g_ToolTipFontStruct.m_Weight = QFont::Thin;       break;
        case 12: g_ToolTipFontStruct.m_Weight = QFont::ExtraLight; break;
        case 25: g_ToolTipFontStruct.m_Weight = QFont::Light;      break;
        default:
        case 50: g_ToolTipFontStruct.m_Weight = QFont::Normal;     break;
        case 57: g_ToolTipFontStruct.m_Weight = QFont::Medium;     break;
        case 63: g_ToolTipFontStruct.m_Weight = QFont::DemiBold;   break;
        case 75: g_ToolTipFontStruct.m_Weight = QFont::Bold;       break;
        case 81: g_ToolTipFontStruct.m_Weight = QFont::ExtraBold;  break;
        case 87: g_ToolTipFontStruct.m_Weight = QFont::Black;      break;
    }
#else
    g_ToolTipFontStruct.m_Weight    = fnt.weight();
#endif
    g_ToolTipFontStruct.m_StyleHint = fnt.styleHint();
}


void DisplayOptions::listFontStruct(FontStruct const &fntstruct)
{
    qDebug() << "Family"     << fntstruct.m_Family;
    qDebug() << "Pointsize=" << fntstruct.m_PtSize;
    qDebug() << "Weight="    << fntstruct.m_Weight;
    qDebug() << "italic="    << fntstruct.m_bItalic;
    qDebug() << "StyleHint"  << fntstruct.m_StyleHint;
}


bool DisplayOptions::isDarkMode()
{
    return g_bStyleSheet || (qApp->palette().window().color().value()<125);
}


void DisplayOptions::loadSettings(QSettings &settings)
{
    settings.beginGroup("DisplayOptions");
    {
        g_BackgroundColor2d = settings.value("BackgroundColor", g_BackgroundColor2d).value<QColor>();

        g_TextColor = settings.value("TextColor", g_TextColor).value<QColor>();

        g_TextFontStruct.loadSettings(   settings, "TextFont");
        g_TableFontStruct.loadSettings(  settings, "TableFont");
        g_TreeFontStruct.loadSettings(   settings, "TreeFont");
        g_ToolTipFontStruct.loadSettings(settings, "ToolTipFont");
        QToolTip::setFont(DisplayOptions::toolTipFont());

        int iTheme = settings.value("Theme",0).toInt();
        if(iTheme==0) g_Theme = DisplayOptions::LIGHTTHEME;
        else          g_Theme = DisplayOptions::DARKTHEME;

        g_ScaleFactor = settings.value("ScaleFactor", g_ScaleFactor).toDouble();
        g_IconSize    = settings.value("IconSize",    g_IconSize).toInt();
    }
    settings.endGroup();
}


void DisplayOptions::saveSettings(QSettings &settings)
{
    settings.beginGroup("DisplayOptions");
    {

        settings.setValue("BackgroundColor", g_BackgroundColor2d);
        settings.setValue("TextColor",       g_TextColor);

        g_TextFontStruct.saveSettings(   settings, "TextFont");
        g_TableFontStruct.saveSettings(  settings, "TableFont");
        g_TreeFontStruct.saveSettings(   settings, "TreeFont");
        g_ToolTipFontStruct.saveSettings(settings, "ToolTipFont");

        if(isLightTheme()) settings.setValue("Theme",0);
        else               settings.setValue("Theme",1);

        settings.setValue("ScaleFactor", g_ScaleFactor);
        settings.setValue("IconSize",    g_IconSize);
    }
    settings.endGroup();
}


