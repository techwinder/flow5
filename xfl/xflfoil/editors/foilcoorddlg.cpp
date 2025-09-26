/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringList>
#include <QHeaderView>
#include <QHideEvent>
#include <QKeyEvent>

#include "foilcoorddlg.h"
#include <xflfoil/editors/foilwt.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/actionitemmodel.h>
#include <xflwidgets/customwts/plaintextoutput.h>
#include <xflcore/xflcore.h>

#include <xflfoil/objects2d/foil.h>

QByteArray FoilCoordDlg::s_HSplitterSizes;

FoilCoordDlg::FoilCoordDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle("Foil coordinates");
    setupLayout();
}


void FoilCoordDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);

    m_pFoilWt->addFoil(pFoil);

    if(pFoil && pFoil->nNodes()>0)
        fillCoordinates();
    else
    {
        m_pCoords->setPlainText("Paste the coordinates here and Apply");
        m_pCoords->selectAll();
    }
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

    double x(0), y(0), z(0);
    m_pBufferFoil->clearPointArrays();
    for(int i=0; i<coords.size(); i++)
    {
        if(xfl::readValues(coords[i], x, y, z)>=2)
        {
            m_pBufferFoil->m_BaseNode.push_back({x,y});
        }
    }

    // Check if the foil was written clockwise or counter-clockwise
    int ip = 0;
    double area = 0.0;
    for (int i=0; i<m_pBufferFoil->nBaseNodes(); i++)
    {
        if(i==m_pBufferFoil->nBaseNodes()-1) ip = 0;
        else                         ip = i+1;
        area += 0.5*(m_pBufferFoil->yb(i)+m_pBufferFoil->yb(ip))*(m_pBufferFoil->xb(i)-m_pBufferFoil->xb(ip));
    }

    if(area < 0.0)
    {
        //reverse the points order
        for (int i=0; i<m_pBufferFoil->nBaseNodes()/2; i++)
        {
            double xtmp         = m_pBufferFoil->xb(i);
            double ytmp         = m_pBufferFoil->yb(i);
            m_pBufferFoil->m_BaseNode[i].x = m_pBufferFoil->xb(m_pBufferFoil->nBaseNodes()-i-1);
            m_pBufferFoil->m_BaseNode[i].y = m_pBufferFoil->yb(m_pBufferFoil->nBaseNodes()-i-1);
            m_pBufferFoil->m_BaseNode[m_pBufferFoil->nBaseNodes()-i-1].x = xtmp;
            m_pBufferFoil->m_BaseNode[m_pBufferFoil->nBaseNodes()-i-1].y = ytmp;
        }
    }

    m_pBufferFoil->m_Node.resize(m_pBufferFoil->nBaseNodes());
    for(int i=0; i<m_pBufferFoil->nBaseNodes(); i++)
    {
        m_pBufferFoil->m_Node[i].x = m_pBufferFoil->xb(i);
        m_pBufferFoil->m_Node[i].y = m_pBufferFoil->yb(i);
    }

    m_pBufferFoil->initGeometry();
}

