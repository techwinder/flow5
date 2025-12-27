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

#include <cmath>


#include <QHBoxLayout>
#include <QHeaderView>
#include <QPlainTextEdit>

#include "fastgraphwt.h"

#include <core/xflcore.h>
#include <interfaces/graphs/controls/graphdlg.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>


QByteArray FastGraphWt::s_Geometry;
QByteArray FastGraphWt::s_HSplitterSizes;


LineStyle FastGraphWt::s_LS[5];

FastGraphWt::FastGraphWt(QWidget *pParent) : QFrame(pParent)
{
    setWindowTitle("Fast graph");
    setupLayout();

    // intialize with template data
    QModelIndex index = m_pDataModel->index(0,0);
    m_pDataModel->setData(index, "X_variable");
    index = m_pDataModel->index(0,1);
    m_pDataModel->setData(index, "Y_variable");
    int nrows=20;
    for(int i=0; i<20; i++)
    {
        double x = double(i)/double(nrows-1)*3.141592654;
        double y = cos(x);
        index = m_pDataModel->index(i+1,0);
        m_pDataModel->setData(index, x);
        index = m_pDataModel->index(i+1,1);
        m_pDataModel->setData(index, y);
    }

    connectSignals();
}


FastGraphWt::~FastGraphWt()
{
    if(m_pGraph && m_pGraph->curveModel()) m_pGraph->deleteCurveModel();
    delete m_pGraph;
    m_pGraph = nullptr;
}


void FastGraphWt::setupLayout()
{
    // strange behaviour when inheriting this widget directly from QSplitter

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        m_pHSplitter = new QSplitter(Qt::Horizontal);
        {
            m_pcptData = new CPTableView;
            {
                m_pcptData->setToolTip("<p>Enter the variable names in the first line, and the data in the following lines."
                                           "The separators recognized to separate the fields when pasting data are "
                                           "the tab, the comma and the semi-colon.<br>"
                                           "Double click on the graph to edit the curves\' style.<br>"
                                           "Activate the in-graph legend in the graph's settings.</p>");
                m_pcptData->setCharSize(3,5);
                m_pcptData->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
                m_pcptData->setEditable(true);
                m_pcptData->horizontalHeader()->setStretchLastSection(true);
            }

            m_pDataModel = new ActionItemModel(this);
            {
                m_pDataModel->setName("Plane model");
                m_pDataModel->setActionColumn(-1);
                m_pDataModel->setRowCount(3000);//temporary
                m_pDataModel->setColumnCount(20); // 10 curves
                for(int i=0; i<m_pDataModel->columnCount()/2; i++)
                {
                    m_pDataModel->setHeaderData(2*i,   Qt::Horizontal, QString::asprintf("x%d", i+1));
                    m_pDataModel->setHeaderData(2*i+1, Qt::Horizontal, QString::asprintf("y%d", i+1));
                }

                m_pDataModel->setHeaderData(0, Qt::Vertical, "Name");
                for(int i=1; i<m_pDataModel->rowCount(); i++)
                {
                    m_pDataModel->setHeaderData(i,   Qt::Vertical, QString::asprintf("%d", i));
                }
            }
            m_pcptData->setModel(m_pDataModel);

            m_pDataDelegate = new XflDelegate(this);
            {
                m_pDataDelegate->setName("Fast graph delegate");
                m_pDataDelegate->setCheckColumn(-1);
                m_pDataDelegate->setActionColumn(-1);
                QVector<int>precision(m_pDataModel->columnCount(),5);
                m_pDataDelegate->setDigits(precision);
                QVector<XflDelegate::enumItemType> types(m_pDataModel->columnCount(), XflDelegate::DOUBLE);
                m_pDataDelegate->setItemTypes(types);
            }
            m_pcptData->setItemDelegate(m_pDataDelegate);

            m_pGraphWt = new GraphWt;
            {
                m_pGraphWt->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                m_pGraphWt->enableContextMenu(true);
                m_pGraphWt->enableCurveStylePage(true);
                m_pGraphWt->showLegend(true);
                m_pGraphWt->setDefaultSize(QSize(500,550));
                m_pGraph = new Graph;
                m_pGraphWt->setGraph(m_pGraph);
                m_pGraph->setName("FastGraph");
                m_pGraph->setCurveModel(new CurveModel);
                m_pGraph->setScaleType(GRAPH::EXPANDING);
                m_pGraph->setAuto(true);
                m_pGraph->setLegendVisible(true);
            }

//            m_pHSplitter->setOpaqueResize(true);
            m_pHSplitter->addWidget(m_pcptData);
            m_pHSplitter->addWidget(m_pGraphWt);
        }
        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);
}


void FastGraphWt::connectSignals()
{
    connect(m_pHSplitter,   SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(m_pDataModel,   SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onMakeGraph()));
    connect(m_pcptData,     SIGNAL(dataPasted()),           SLOT(onMakeGraph()));

    connect(m_pGraphWt,     SIGNAL(graphChanged(Graph*)),   SLOT(onGraphChanged()));
}


void FastGraphWt::loadSettings(QSettings &settings)
{
    for(int i=0; i<5; i++)
    {
        s_LS[i].m_Width = Curve::defaultLineWidth();
    }
    settings.beginGroup("FastGraphWt");
    {
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
        s_HSplitterSizes      = settings.value("HSplitterSizes").toByteArray();

        for(int i=0; i<5; i++)
        {
            s_LS[i].m_Color = xfl::tofl5Clr(QColor::fromHsv(105,105,int(double(i+1)/6.0*255.0)));
            s_LS[i].m_Width = Curve::defaultLineWidth();
            xfl::loadLineSettings(settings, s_LS[i], QString::asprintf("Line_%d", i));
        }
    }
    settings.endGroup();
}


void FastGraphWt::saveSettings(QSettings &settings)
{
    settings.beginGroup("FastGraphWt");
    {
        settings.setValue("WindowGeom",     s_Geometry);
        settings.setValue("HSplitterSizes", s_HSplitterSizes);

        for(int i=0; i<5; i++)
            xfl::saveLineSettings(settings, s_LS[i], QString::asprintf("Line_%d", i));
    }
    settings.endGroup();
}


void FastGraphWt::keyPressEvent(QKeyEvent *pEvent)
{
//    bool bCtrl = pEvent->modifiers() & Qt::ControlModifier;
    switch(pEvent->key())
    {
        case Qt::Key_Escape:
            hide();
            break;
    }
    pEvent->ignore();
}


void FastGraphWt::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);

    GraphDlg::setActivePage(5);

    onMakeGraph();
}


void FastGraphWt::hideEvent(QHideEvent *pEvent)
{
    QWidget::hideEvent(pEvent);
    s_Geometry = saveGeometry();
    s_HSplitterSizes  = m_pHSplitter->saveState();
}


void FastGraphWt::onSplitterMoved()
{

}

/** Settings have been edited */
void FastGraphWt::onGraphChanged()
{
    Graph const*pGraph = m_pGraphWt->graph();
    for(int ic=0; ic<pGraph->curveCount(); ic++)
    {
        Curve const *pCurve = pGraph->curve(ic);
        QModelIndex ind = m_pDataModel->index(0, 2*ic+1);
        m_pDataModel->setData(ind, pCurve->name());

        if(ic<5) s_LS[ic] = pCurve->theStyle();
    }
    m_pDataModel->updateData();
}


void FastGraphWt::onMakeGraph()
{
    Graph *pGraph = m_pGraphWt->graph();
    pGraph->deleteCurves();

    // find the number of active columns
    int nCols=0;
    int nRows=0;
    QModelIndex index = m_pDataModel->index(nRows, nCols);
    QVariant var = m_pDataModel->data(index);
    while(var.isValid() && !var.isNull() && nCols<m_pDataModel->columnCount())
    {
        nCols++;
        index = m_pDataModel->index(nRows, nCols);
        var = m_pDataModel->data(index);
    }

    if(nCols%2==1) nCols--; // can only used even numbers

    for(int iCol=0; iCol<nCols; iCol++)
    {
        int rows=0;
        QModelIndex index = m_pDataModel->index(rows, iCol);
        QVariant var = m_pDataModel->data(index);
        while(var.isValid() && !var.isNull() && rows<m_pDataModel->rowCount())
        {
            rows++;
            index = m_pDataModel->index(rows, iCol);
            var = m_pDataModel->data(index);
        }
        nRows = std::max(nRows, rows);
    }

    //read the columns' data
    bool bOkx=false, bOky=false;
    double dx=0, dy=0;
    QVector<double> x, y;
    for(int i=0; i<nCols/2; i++)
    {
        int xcol = 2*i;
        int ycol = 2*i+1;
        x.clear();
        y.clear();
        int row = 0;
        QModelIndex indx = m_pDataModel->index(row, xcol);
        QVariant varx = m_pDataModel->data(indx);
        QModelIndex indy = m_pDataModel->index(row, ycol);
        QVariant vary = m_pDataModel->data(indy);
        QString curvename = vary.toString();

        row++; // skip label line
        indx = m_pDataModel->index(row, xcol);
        varx = m_pDataModel->data(indx);
        indy = m_pDataModel->index(row, ycol);
        vary = m_pDataModel->data(indy);
        while(varx.isValid() && !varx.isNull() && vary.isValid() && !vary.isNull() && row<m_pDataModel->rowCount())
        {
            dx = varx.toDouble(&bOkx);
            dy = vary.toDouble(&bOky);
            if(bOkx && bOky)
            {
                x.append(dx);
                y.append(dy);
            }
            row++;
            indx = m_pDataModel->index(row, xcol);
            varx = m_pDataModel->data(indx);
            indy = m_pDataModel->index(row, ycol);
            vary = m_pDataModel->data(indy);
        }

        if(x.size())
        {
            Curve *pCurve = pGraph->addCurve(curvename);
            pCurve->setTheStyle(s_LS[(pGraph->curveCount()-1)%5]);
            pCurve->setPoints(x, y);
        }
    }
    pGraph->invalidate();
    update();
}
