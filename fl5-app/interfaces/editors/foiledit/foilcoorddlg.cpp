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


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringList>
#include <QHeaderView>
#include <QHideEvent>
#include <QKeyEvent>

#include "foilcoorddlg.h"
#include <interfaces/editors/foiledit/foilwt.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <core/xflcore.h>
#include <api/foil.h>

QByteArray FoilCoordDlg::s_HSplitterSizes;

FoilCoordDlg::FoilCoordDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle(tr("Foil coordinates"));
    setupLayout();
}


void FoilCoordDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);

    if(pFoil)
    {
        pFoil->show();
        m_pFoilWt->addFoil(pFoil);
        if(pFoil->nNodes()>0)    fillCoordinates();
    }
    else
    {
        m_pCoords->setPlainText("Paste the coordinates here and Apply");
        m_pCoords->selectAll();
    }
    m_pFoilWt->addFoil(m_pBufferFoil);
    m_pBufferFoil->show();

}


void FoilCoordDlg::onReset()
{
    FoilDlg::onReset();
    fillCoordinates();
    update();
}


void FoilCoordDlg::fillCoordinates()
{
    QString strange, strong;
    for(int i=0; i<m_pRefFoil->nNodes(); i++)
    {
        strong = QString::asprintf("  %11.5g  %11.5g\n", m_pRefFoil->x(i), m_pRefFoil->y(i));
        strange += strong;
    }
    m_pCoords->setPlainText(strange);
}


void FoilCoordDlg::setupLayout()
{
    m_pHSplitter = new QSplitter;
    {
        m_pCoords = new PlainTextOutput;
        m_pCoords->setReadOnly(false);
        m_pHSplitter->addWidget(m_pCoords);
        m_pHSplitter->addWidget(m_pFoilWt);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(m_pHSplitter);
    setLayout(pMainLayout);
}


void FoilCoordDlg::showEvent(QShowEvent *pEvent)
{
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);

    resizeEvent(nullptr);
    pEvent->accept();
}


void FoilCoordDlg::hideEvent(QHideEvent *pEvent)
{
    FoilDlg::hideEvent(pEvent);
    s_HSplitterSizes  = m_pHSplitter->saveState();

}


void FoilCoordDlg::onApply()
{
    readCoordinates();

    m_bModified = true;
    update();
}


void FoilCoordDlg::readCoordinates()
{
    QString str =m_pCoords->toPlainText();
    QStringList coords;
#if QT_VERSION >= 0x050F00
    coords = str.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
#else
    coords = str.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
#endif

    std::vector<Node2d> basenodes;

    double x(0), y(0), z(0);
    m_pBufferFoil->clearPointArrays();
    for(int i=0; i<coords.size(); i++)
    {
        if(xfl::readValues(coords[i], x, y, z)>=2)
        {
            basenodes.push_back({x,y});
        }
        else
        {
            m_pBufferFoil->setName(coords.front().toStdString());
        }
    }

    // Check if the foil was written clockwise or counter-clockwise
    int ip = 0;
    double area = 0.0;
    for (int i=0; i<m_pBufferFoil->nBaseNodes(); i++)
    {
        if(i==m_pBufferFoil->nBaseNodes()-1) ip = 0;
        else                                 ip = i+1;
        area += 0.5*(m_pBufferFoil->yb(i)+m_pBufferFoil->yb(ip))*(m_pBufferFoil->xb(i)-m_pBufferFoil->xb(ip));
    }

    if(area < 0.0)
    {
        //reverse the points order
        for (int i=0; i<m_pBufferFoil->nBaseNodes()/2; i++)
        {
            double xtmp         = m_pBufferFoil->xb(i);
            double ytmp         = m_pBufferFoil->yb(i);
            basenodes[i].x = m_pBufferFoil->xb(m_pBufferFoil->nBaseNodes()-i-1);
            basenodes[i].y = m_pBufferFoil->yb(m_pBufferFoil->nBaseNodes()-i-1);
            basenodes[m_pBufferFoil->nBaseNodes()-i-1].x = xtmp;
            basenodes[m_pBufferFoil->nBaseNodes()-i-1].y = ytmp;
        }
    }


    m_pBufferFoil->setBaseNodes(basenodes);
    m_pBufferFoil->initGeometry();
}

