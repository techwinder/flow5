/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QKeyEvent>
#include <QColor>
#include <QColorDialog>
#include <QGridLayout>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include <xflcore/displayoptions.h>
#include <xflwidgets/line/linepicker.h>
#include <xflwidgets/color/textclrbtn.h>


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
    m_pColorButton->setText("Other");
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
        m_lb[j].setColor(s_LineColorList.at(j));
        m_lb[j].setToolTip(s_LineColorNames.at(j));
        m_lb[j].setBackground(true);
        connect(&m_lb[j], SIGNAL(clickedLB(LineStyle)), SLOT(onClickedLB(LineStyle)));
    }

    m_theStyle = ls;
    setColor(ls.m_Color);
}


void LinePicker::setColor(const QColor &color)
{
    for(int p=0; p<NCOLORROWS*NCOLORCOLS; p++)
    {
        m_lb[p].setCurrent(color==m_lb[p].btnColor());
        m_lb[p].setStipple(Line::SOLID);
        m_lb[p].setWidth(m_theStyle.m_Width);
    }
    m_theStyle.m_Color = color;
}


void LinePicker::onOtherColor()
{
    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;

    QColor Color = QColorDialog::getColor(m_theStyle.m_Color,
                                          this, "Select the color", dialogOptions);
    if(Color.isValid()) m_theStyle.m_Color = Color;
    emit colorChanged(m_theStyle.m_Color);
}


void LinePicker::onClickedLB(LineStyle ls)
{
    m_theStyle.m_Color = ls.m_Color;
    emit colorChanged(ls.m_Color);
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



