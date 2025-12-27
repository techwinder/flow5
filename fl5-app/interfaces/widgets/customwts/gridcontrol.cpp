/****************************************************************************

    xflwidgets Library
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

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include "gridcontrol.h"
#include <interfaces/widgets/view/grid.h>
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/line/linebtn.h>


GridControl::GridControl(QWidget *pParent) : QWidget(pParent)
{
    m_pGrid = nullptr;
    setupLayout();
    connectSignals();
}


void GridControl::connectSignals()
{
    connect(m_pchYAxisShow,   SIGNAL(clicked(bool)),        SLOT(onYAxisShow(bool)));
    connect(m_pchXAxisShow,   SIGNAL(clicked(bool)),        SLOT(onXAxisShow(bool)));
    connect(m_pchXMajShow,    SIGNAL(clicked(bool)),        SLOT(onXMajShow(bool)));
    connect(m_pchYMajShow,    SIGNAL(clicked(bool)),        SLOT(onYMajShow(bool)));
    connect(m_pchXMinShow,    SIGNAL(clicked(bool)),        SLOT(onXMinShow(bool)));
    connect(m_pchYMinShow,    SIGNAL(clicked(bool)),        SLOT(onYMinShow(bool)));

    connect(m_plbYAxisStyle,  SIGNAL(clickedLB(LineStyle)), SLOT(onYAxisStyle()));
    connect(m_plbXAxisStyle,  SIGNAL(clickedLB(LineStyle)), SLOT(onXAxisStyle()));
    connect(m_plbXMajStyle,   SIGNAL(clickedLB(LineStyle)), SLOT(onXMajStyle()));
    connect(m_plbYMajStyle,   SIGNAL(clickedLB(LineStyle)), SLOT(onYMajStyle()));
    connect(m_plbXMinStyle,   SIGNAL(clickedLB(LineStyle)), SLOT(onXMinStyle()));
    connect(m_plbYMinStyle,   SIGNAL(clickedLB(LineStyle)), SLOT(onYMinStyle()));
}


void GridControl::initControls(Grid *pGrid)
{
    m_pGrid = pGrid;

    m_plbXAxisStyle->setTheStyle(m_pGrid->xAxisStyle());
    m_plbYAxisStyle->setTheStyle(m_pGrid->yAxisStyle());

    m_plbXMajStyle->setTheStyle(m_pGrid->xMajStyle());
    m_plbXMinStyle->setTheStyle(m_pGrid->xMinStyle());

    m_plbYMajStyle->setTheStyle(m_pGrid->yMajStyle(0));
    m_plbYMinStyle->setTheStyle(m_pGrid->yMinStyle(0));

    m_plbXAxisStyle->setEnabled(m_pGrid->bXAxis());
    m_plbYAxisStyle->setEnabled(m_pGrid->bYAxis());
    m_plbXMajStyle->setEnabled(m_pGrid->bXMajGrid());
    m_plbYMajStyle->setEnabled(m_pGrid->bYMajGrid(0));
    m_plbXMinStyle->setEnabled(m_pGrid->bXMinGrid());
    m_plbYMinStyle->setEnabled(m_pGrid->bYMinGrid(0));

    m_pchXAxisShow->setChecked(m_pGrid->bXAxis());
    m_pchYAxisShow->setChecked(m_pGrid->bYAxis());
    m_pchXMajShow->setChecked(m_pGrid->bXMajGrid());
    m_pchYMajShow->setChecked(m_pGrid->bYMajGrid(0));
    m_pchXMinShow->setChecked(m_pGrid->bXMinGrid());
    m_pchYMinShow->setChecked(m_pGrid->bYMinGrid(0));
}


void GridControl::setupLayout()
{
    QGridLayout *pMainLayout = new QGridLayout;
    {
        QLabel *plabX = new QLabel("X");
        QLabel *plabY = new QLabel("Y");

        QLabel *plabAxes = new QLabel("Axes");
        QLabel *plabMajGrid = new QLabel("Major grid");
        QLabel *plabMinGrid = new QLabel("Minor grid");

        m_pchXAxisShow  = new QCheckBox(QString());
        m_pchYAxisShow  = new QCheckBox(QString());
        m_pchXMajShow   = new QCheckBox(QString());
        m_pchYMajShow   = new QCheckBox(QString());
        m_pchXMinShow   = new QCheckBox(QString());
        m_pchYMinShow   = new QCheckBox(QString());

        m_plbYAxisStyle = new LineBtn(this);
        m_plbXAxisStyle = new LineBtn(this);
        m_plbXMajStyle  = new LineBtn(this);
        m_plbYMajStyle  = new LineBtn(this);
        m_plbXMinStyle  = new LineBtn(this);
        m_plbYMinStyle  = new LineBtn(this);


        pMainLayout->addWidget(plabX, 1, 2, 1, 2, Qt::AlignCenter);
        pMainLayout->addWidget(plabY, 1, 4, 1, 2, Qt::AlignCenter);

        pMainLayout->addWidget(plabAxes,         2,1);
        pMainLayout->addWidget(m_pchXAxisShow,   2,2);
        pMainLayout->addWidget(m_plbXAxisStyle,  2,3);
        pMainLayout->addWidget(m_pchYAxisShow,   2,4);
        pMainLayout->addWidget(m_plbYAxisStyle,  2,5);
        pMainLayout->addWidget(plabMajGrid,      3,1);
        pMainLayout->addWidget(m_pchXMajShow,    3,2);
        pMainLayout->addWidget(m_plbXMajStyle,   3,3);
        pMainLayout->addWidget(m_pchYMajShow,    3,4);
        pMainLayout->addWidget(m_plbYMajStyle,   3,5);
        pMainLayout->addWidget(plabMinGrid,      4,1);
        pMainLayout->addWidget(m_pchXMinShow,    4,2);
        pMainLayout->addWidget(m_plbXMinStyle,   4,3);
        pMainLayout->addWidget(m_pchYMinShow,    4,4);
        pMainLayout->addWidget(m_plbYMinStyle,   4,5);
    }

    setLayout(pMainLayout);
}


void GridControl::onXAxisStyle()
{
    LineMenu lineMenu;
    lineMenu.initMenu(m_pGrid->xAxisStyle());
    lineMenu.exec(QCursor::pos());
    m_pGrid->setXAxisStyle(lineMenu.theStyle());
    m_plbXAxisStyle->setTheStyle(m_pGrid->xAxisStyle());
    emit gridModified(true);
}


void GridControl::onYAxisStyle()
{
    LineMenu lineMenu;
    lineMenu.initMenu(m_pGrid->yAxisStyle());
    lineMenu.exec(QCursor::pos());
    m_pGrid->setYAxisStyle(lineMenu.theStyle());
    m_plbYAxisStyle->setTheStyle(m_pGrid->yAxisStyle());
    emit gridModified(true);
}


void GridControl::onXMajStyle()
{
    LineMenu lineMenu;
    lineMenu.initMenu(m_pGrid->xMajStyle());
    lineMenu.exec(QCursor::pos());
    m_pGrid->setXMajStyle(lineMenu.theStyle());
    m_plbXMajStyle->setTheStyle(m_pGrid->xMajStyle());
    emit gridModified(true);
}


void GridControl::onXMinStyle()
{
    LineMenu lineMenu;
    lineMenu.initMenu(m_pGrid->xMinStyle());
    lineMenu.exec(QCursor::pos());
    m_pGrid->setXMinStyle(lineMenu.theStyle());
    m_plbXMinStyle->setTheStyle(m_pGrid->xMinStyle());
    emit gridModified(true);
}


void GridControl::onYMajStyle()
{
    LineMenu lineMenu;
    lineMenu.initMenu(m_pGrid->yMajStyle(0));
    lineMenu.exec(QCursor::pos());
    m_pGrid->setYMajStyle(0, lineMenu.theStyle());
    m_plbYMajStyle->setTheStyle(m_pGrid->yMajStyle(0));
    emit gridModified(true);
}


void GridControl::onYMinStyle()
{
    LineMenu lineMenu;
    lineMenu.initMenu(m_pGrid->yMinStyle(0));
    lineMenu.exec(QCursor::pos());
    m_pGrid->setYMinStyle(0, lineMenu.theStyle());
    m_plbYMinStyle->setTheStyle(m_pGrid->yMinStyle(0));
    emit gridModified(true);
}


void GridControl::onXAxisShow(bool bShow)
{
    m_pGrid->showXAxis(bShow);
    m_plbXAxisStyle->setEnabled(m_pGrid->bXAxis());
    emit gridModified(true);
}


void GridControl::onYAxisShow(bool bShow)
{
    m_pGrid->showYAxis(bShow);
    m_plbYAxisStyle->setEnabled(m_pGrid->bYAxis());
    emit gridModified(true);
}


void GridControl::onXMajShow(bool bShow)
{
    m_pGrid->showXMajGrid(bShow);
    m_plbXMajStyle->setEnabled(m_pGrid->bXMajGrid());
    emit gridModified(true);
}


void GridControl::onYMajShow(bool bShow)
{
    m_pGrid->showYMajGrid(0, bShow);
    m_plbYMajStyle->setEnabled(m_pGrid->bYMajGrid(0));
    emit gridModified(true);
}


void GridControl::onXMinShow(bool bShow)
{
    m_pGrid->showXMinGrid(bShow);
    m_plbXMinStyle->setEnabled(m_pGrid->bXMinGrid());
    emit gridModified(true);
}


void GridControl::onYMinShow(bool bShow)
{
    m_pGrid->showYMinGrid(0, bShow);
    m_plbYMinStyle->setEnabled(m_pGrid->bYMinGrid(0));
    emit gridModified(true);
}


