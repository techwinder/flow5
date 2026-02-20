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


#include <QKeyEvent>
#include <QColor>
#include <QColorDialog>
#include <QGridLayout>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include <core/xflcore.h>
#include <core/displayoptions.h>
#include <interfaces/widgets/line/linepicker.h>
#include <interfaces/widgets/color/textclrbtn.h>


QStringList LinePicker::s_LineColorList;
QStringList LinePicker::s_LineColorNames;



LinePicker::LinePicker(QWidget *pParent) : QWidget(pParent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setupLayout();
}


void LinePicker::setupLayout()
{
    QGridLayout *pGridLayout = new QGridLayout;

    pGridLayout->setContentsMargins(QMargins(0,0,0,0));
    pGridLayout->setHorizontalSpacing(0);
    pGridLayout->setVerticalSpacing(0);

    for(int j=0; j<NCOLORCOLS; j++)
    {
        for(int i=0; i<NCOLORROWS; i++)
        {
            int p = j*NCOLORROWS+i;
            pGridLayout->addWidget(&m_lb[p], i+1,j+1);
        }
    }

    m_pColorButton = new TextClrBtn();
    m_pColorButton->setText(tr("Other"));
    m_pColorButton->setBackgroundColor(DisplayOptions::backgroundColor());
    m_pColorButton->setTextColor(DisplayOptions::textColor());
    m_pColorButton->setContour(false);

    connect(m_pColorButton, SIGNAL(clickedTB()), SLOT(onOtherColor()));
    pGridLayout->addWidget(m_pColorButton, NCOLORROWS+1, 1, 1, NCOLORCOLS, Qt::AlignVCenter);
    setLayout(pGridLayout);
}


void LinePicker::initDialog(LineStyle const &ls)
{
    for(int j=0; j<NCOLORROWS*NCOLORCOLS; j++)
    {
        m_lb[j].setBtnColor(s_LineColorList.at(j));
        m_lb[j].setToolTip(s_LineColorNames.at(j));
        m_lb[j].setBackground(true);
        connect(&m_lb[j], SIGNAL(clickedLB(LineStyle)), SLOT(onClickedLB(LineStyle)));
    }

    m_theStyle = ls;
    setColor(ls.m_Color);
}

void LinePicker::setColor(const QColor &color)
{
    setColor(xfl::tofl5Clr(color));
}

void LinePicker::setColor(const fl5Color &color)
{
    for(int p=0; p<NCOLORROWS*NCOLORCOLS; p++)
    {
        m_lb[p].setCurrent(xfl::fromfl5Clr(color)==m_lb[p].btnColor());
        m_lb[p].setStipple(Line::SOLID);
        m_lb[p].setWidth(m_theStyle.m_Width);
    }
    m_theStyle.m_Color = color;
}


void LinePicker::onOtherColor()
{
    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;

    QColor Color = QColorDialog::getColor(xfl::fromfl5Clr(m_theStyle.m_Color),
                                          nullptr, "Select the color", dialogOptions);
    if(Color.isValid()) m_theStyle.m_Color = xfl::tofl5Clr(Color);
    emit colorChanged(xfl::fromfl5Clr(m_theStyle.m_Color));
}


void LinePicker::onClickedLB(LineStyle ls)
{
    m_theStyle.m_Color = ls.m_Color;
    emit colorChanged(xfl::fromfl5Clr(ls.m_Color));
}


void LinePicker::setColorList(QStringList const&colorlist, QStringList const &colornames)
{
    s_LineColorList = colorlist;
    s_LineColorNames = colornames;
}


QColor LinePicker::randomColor(bool bLightColor)
{
    int row=3;
    if(bLightColor) row =              rand()%(NCOLORROWS/2);
    else            row = NCOLORROWS/2+rand()%(NCOLORROWS/2);
    int col = rand()%NCOLORCOLS;
    int randindex = row*NCOLORCOLS+col;
    return s_LineColorList.at(randindex);
}



