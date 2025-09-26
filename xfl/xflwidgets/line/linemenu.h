/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QMenu>
#include <xflcore/linestyle.h>
#include <xflwidgets/line/lineaction.h>



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
        void setColor(QColor color) {m_theStyle.m_Color=color;}
        void setPointStyle(Line::enumPointStyle pointstyle) {m_theStyle.m_Symbol=pointstyle;}
        void setTag(QString tag) {m_theStyle.m_Tag=tag;}

        void initMenu(LineStyle ls);
        void initMenu(Line::enumLineStipple style, int width, QColor color, Line::enumPointStyle pointStyle);

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


