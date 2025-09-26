/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QStringList>
#include <QPushButton>
#include <QMenu>

#include <xflwidgets/line/linebtn.h>


#define NCOLORROWS 24
#define NCOLORCOLS 3

class TextClrBtn;

class LinePicker : public QWidget
{
    Q_OBJECT

    public:
        LinePicker(QWidget *pParent=nullptr);

        void initDialog(LineStyle const &ls);

        void setColor(QColor const &clr);

        static void setColorList(QStringList const&colorlist, QStringList const &colornames);

        static QColor randomColor(bool bLightColor);


    signals:
        void colorChanged(QColor);

    public slots:
        void onClickedLB(LineStyle ls);

        void onOtherColor();

    private:
        void setupLayout();

    private:
        LineBtn m_lb[NCOLORROWS*NCOLORCOLS];

        TextClrBtn *m_pColorButton;

        LineStyle m_theStyle;

        static QStringList s_LineColorList;
        static QStringList s_LineColorNames;

};


