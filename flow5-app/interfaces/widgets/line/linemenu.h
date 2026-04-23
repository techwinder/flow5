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

#include <QMenu>
#include <api/linestyle.h>
#include <interfaces/widgets/line/lineaction.h>



class LinePicker;
class LineAction;

class LineMenu : public QMenu
{
    Q_OBJECT

    public:
        LineMenu(QWidget *pParent=nullptr, bool bShowPointStyleMenu=true);
        ~LineMenu();

        LineStyle const &theStyle() const {return m_theStyle;}
        void setTheStyle(LineStyle const &ls) {m_theStyle=ls;}
        void setStyle(Line::enumLineStipple style) {m_theStyle.m_Stipple=style;}
        void setWidth(int width) {m_theStyle.m_Width=width;}
        void setColor(const QColor &color);
        void setPointStyle(Line::enumPointStyle pointstyle) {m_theStyle.m_Symbol=pointstyle;}
        void setTag(QString const &tag) {m_theStyle.m_Tag=tag.toStdString();}

        void initMenu(LineStyle ls);
        void initMenu(Line::enumLineStipple style, int width, QColor const &color, Line::enumPointStyle pointStyle);
        void initMenu(Line::enumLineStipple style, int width, fl5Color const &clr, Line::enumPointStyle pointStyle);

        void showPointStyle(bool bShow);

        void setParentMenu(QMenu *pMenu){m_pParentMenu = pMenu;}

        void enableSubMenus();
        void enableSubMenus(bool blineStyle, bool bLineWidth, bool bLineColor, bool bPointStyle);

        bool styleChanged()  const {return m_bStyleChanged;}
        bool widthChanged()  const {return m_bWidthChanged;}
        bool colorChanged()  const {return m_bColorChanged;}
        bool pointsChanged() const {return m_bPointsChanged;}

    signals:
        void lineChanged(LineStyle ls);

    private slots:
        void onPointStyle();
        void onLineStyle();
        void onLineWidth();
        void onColorChanged(QColor clr);

    private:
        void fillMenu(bool bShowPointStyleMenu);
        void updateLineActions();

        LineStyle m_theStyle;

        LineAction m_SymbolAction[NPOINTSTYLES];
        LineAction m_LineStyleAction[NLINESTYLES];
        LineAction m_LineWidthAction[NLINEWIDTHS];
        LinePicker *m_pck;
        QMenu *m_pPointStyleMenu, *m_pLineStyleMenu, *m_pLineWidthMenu, *m_pLineColorMenu;

        QMenu *m_pParentMenu;

        bool m_bStyleChanged, m_bWidthChanged, m_bColorChanged, m_bPointsChanged;

    public:
        bool m_bEnableLineStyle, m_bEnableLineWidth, m_bEnableLineColor, m_bEnablePointStyle;
};


