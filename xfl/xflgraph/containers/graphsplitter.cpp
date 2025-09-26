/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QKeyEvent>

#include "graphsplitter.h"
#include <xflgraph/graph/graph.h>
#include <xflgraph/containers/graphwt.h>

/**
 * Note: the number of graphs should be even, so that they can be spread
 * in two rows of equal number
 */
GraphSplitter::GraphSplitter(QVector<Graph*> &graphlist, QWidget *parent) : QSplitter(parent)
{
//    setAttribute(Qt::WA_DeleteOnClose);
    m_pGraphList = &graphlist;

    setupLayout();
}


void GraphSplitter::setupLayout()
{
    QFont fnt10("Noto Sans");
    fnt10.setPointSize(10);
    QFont fnt8("Hack");
    fnt8.setPointSize(8);

    int nCols = m_pGraphList->size()/2;
    for(int i=0; i<m_pGraphList->size(); i++) m_GraphWt.append(new GraphWt);

    for(int i=0; i<m_GraphWt.size(); i++)
    {
        m_GraphWt.at(i)->setGraph(m_pGraphList->at(i));
        m_GraphWt.at(i)->setGraph(m_pGraphList->at(i));
        m_GraphWt.at(i)->enableContextMenu(true);
    }

    m_pTopSplitter = new QSplitter;
    {
        connect(m_pTopSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(onTopBLSplitterMoved()));
        for(int ig=0; ig<nCols; ig++)
        {
            m_pTopSplitter->addWidget(m_GraphWt.at(ig));
        }
//        m_pTopSplitter->setChildrenCollapsible(false);
    }
    m_pBotSplitter = new QSplitter;
    {
        connect(m_pBotSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(onBotBLSplitterMoved()));
        for(int ig=nCols; ig<m_pGraphList->size(); ig++)
        {
            m_pBotSplitter->addWidget(m_GraphWt.at(ig));
        }
//        m_pBotSplitter->setChildrenCollapsible(false);
    }
//    setChildrenCollapsible(false);

    setOrientation(Qt::Vertical);
    addWidget(m_pTopSplitter);
    addWidget(m_pBotSplitter);
}


void GraphSplitter::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl = pEvent->modifiers() & Qt::ControlModifier;
    switch (pEvent->key())
    {
        case Qt::Key_R:
        {
//            if(bCtrl)
            {
                onResetSplitters();
                pEvent->accept();
            }
            break;
        }
        case Qt::Key_W:
        {
            if(bCtrl) hide();
            break;
        }
        default:
            pEvent->ignore();
            break;
    }
    pEvent->ignore();
}


void GraphSplitter::onResetSplitters()
{
    QList<int> hSizes, vSizes;
    int ng = m_pGraphList->size();
    for(int i=0; i<ng; i++) hSizes.append(1);
    vSizes.append(1);
    vSizes.append(1);
    m_pTopSplitter->setSizes(hSizes);
    m_pBotSplitter->setSizes(hSizes);
    setSizes(vSizes);
    update();
}


void GraphSplitter::onTopBLSplitterMoved()
{
    m_pBotSplitter->setSizes(m_pTopSplitter->sizes());
}


void GraphSplitter::onBotBLSplitterMoved()
{
    m_pTopSplitter->setSizes(m_pBotSplitter->sizes());
}
