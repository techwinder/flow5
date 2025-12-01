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

#include <QVBoxLayout>
#include <QMenu>
#include <QContextMenuEvent>
#include <QHeaderView>

#include "foilexplorer.h"

#include <globals/mainframe.h>
#include <modules/xdirect/menus/xdirectmenus.h>
#include <modules/xdirect/view2d/dfoilwt.h>
#include <modules/xdirect/xdirect.h>

#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/oppoint.h>
#include <api/polar.h>
#include <api/geom_params.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/mvc/expandabletreeview.h>
#include <interfaces/widgets/mvc/objecttreedelegate.h>
#include <interfaces/widgets/mvc/objecttreeitem.h>
#include <interfaces/widgets/mvc/objecttreemodel.h>


MainFrame *FoilExplorer::s_pMainFrame = nullptr;
XDirect *FoilExplorer::s_pXDirect   = nullptr;

int FoilExplorer::s_Width=351;

FoilExplorer::FoilExplorer(QWidget *pParent) : QWidget(pParent)
{
    m_pTreeView = nullptr;
    m_pModel  = nullptr;
    m_pDelegate = nullptr;

    m_Selection = FoilExplorer::NONE;
    m_ppto = new PlainTextOutput;


    QStringList labels;
    labels << "Object" << "1234567"<< "";

    m_pTreeView = new ExpandableTreeView;
    m_pTreeView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    m_pTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pTreeView->setUniformRowHeights(true);
    m_pTreeView->setRootIsDecorated(true);
    connect(m_pTreeView, SIGNAL(switchAll(bool)), SLOT(onSwitchAll(bool)));

    m_pModel = new ObjectTreeModel(this);
    m_pModel->setHeaderData(0, Qt::Horizontal, "Objects", Qt::DisplayRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, "1234567890123", Qt::EditRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, "1234567890123", Qt::DisplayRole);
    m_pModel->setHeaderData(2, Qt::Horizontal, "123", Qt::DisplayRole);
    m_pModel->setHeaderData(2, Qt::Horizontal, Qt::AlignRight, Qt::TextAlignmentRole);

    m_pTreeView->setModel(m_pModel);
    connect(m_pTreeView->m_pleFilter, SIGNAL(returnPressed()), this, SLOT(onSetFilter()));

    m_pTreeView->header()->hide();
    m_pTreeView->header()->setStretchLastSection(false);
    m_pTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_pTreeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // so that ExpandableTreeView::sizeHintForColumn is used
    m_pTreeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // so that ExpandableTreeView::sizeHintForColumn is used

    m_pDelegate = new ObjectTreeDelegate(this);
    m_pTreeView->setItemDelegate(m_pDelegate);

    connect(m_pTreeView, SIGNAL(pressed(QModelIndex)), SLOT(onItemClicked(QModelIndex)));
    //    connect(m_pStruct, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked(QModelIndex)));
    connect(m_pTreeView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentRowChanged(QModelIndex,QModelIndex)));

    setupLayout();

}


FoilExplorer::~FoilExplorer()
{
}


void FoilExplorer::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    m_pMainSplitter->restoreState(m_SplitterSizes);
    pEvent->accept();
}


void FoilExplorer::hideEvent(QHideEvent *pEvent)
{
    QWidget::hideEvent(pEvent);
    m_SplitterSizes = m_pMainSplitter->saveState();
    s_Width = width();
}


void FoilExplorer::resizeEvent(QResizeEvent *pEvent)
{
    s_Width = width();
    updateGeometry(); // Notifies the layout system that the sizeHint() has changed
    pEvent->accept();
}


void FoilExplorer::updateObjectView()
{
    setObjectProperties();
    fillModelView();

    switch (s_pXDirect->m_eView)
    {
        case XDirect::DESIGNVIEW:
        {
            selectFoil(XDirect::curFoil());
            break;
        }
        case XDirect::BLVIEW:
        case XDirect::OPPVIEW:
        {
            if      (XDirect::curOpp())   selectOpPoint();
            else if (XDirect::curPolar()) selectPolar(XDirect::curPolar());
            else                          selectFoil(XDirect::curFoil());
            break;
        }
        case XDirect::POLARVIEW:
        {
            if (XDirect::curPolar()) selectPolar(XDirect::curPolar());
            else                     selectFoil(XDirect::curFoil());
            break;
        }
    }
}


void FoilExplorer::selectObjects()
{
    if      (XDirect::curOpp())   selectOpPoint();
    else if (XDirect::curPolar()) selectPolar(XDirect::curPolar());
    else                          selectFoil(XDirect::curFoil());
}


void FoilExplorer::setObjectProperties()
{
    std::string props;
    switch(m_Selection)
    {
        //        {NONE, FOIL, POLAR, OPPOINT}
        case FoilExplorer::FOIL:
        {
            if(XDirect::curFoil())
            {
                props = XDirect::curFoil()->properties(true);
                break;
            }
            break;
        }
        case FoilExplorer::POLAR:
        {
            if(XDirect::curPolar())
            {
                props = s_pXDirect->curPolar()->properties();
                break;
            }
            break;
        }
        case FoilExplorer::OPPOINT:
        {
            if(XDirect::curOpp())
            {
                props = XDirect::curOpp()->properties(SaveOptions::textSeparator().toStdString());
                break;
            }
            break;
        }
        default:
        {
            props.clear();
            break;
        }
    }
    m_ppto->setStdText(props);
}


void FoilExplorer::setupLayout()
{
    m_pMainSplitter = new QSplitter;
    m_pMainSplitter->setOrientation(Qt::Vertical);
    {
        m_pMainSplitter->addWidget(m_pTreeView);
        m_pMainSplitter->addWidget(m_ppto);
    }
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pTreeView->cmdWidget());
        pMainLayout->addWidget(m_pMainSplitter);
    }
    setLayout(pMainLayout);
}


void FoilExplorer::fillModelView()
{
    m_pModel->removeRows(0, m_pModel->rowCount());

    ObjectTreeItem *pRootItem = m_pModel->rootItem();

//    m_pTreeView->selectionModel()->blockSignals(true);

    for (int i=0; i<Objects2d::nFoils(); i++)
    {
        Foil const *pFoil = Objects2d::foilAt(i);
        if(!pFoil) continue;

        ObjectTreeItem *pFoilItem = m_pModel->appendRow(pRootItem, pFoil->name(), pFoil->theStyle(), foilState(pFoil));
        fillPolars(pFoilItem, pFoil);
    }

//    m_pTreeView->selectionModel()->blockSignals(false);
}


void FoilExplorer::fillPolars(ObjectTreeItem *pFoilItem, Foil const *pFoil)
{
    if(!pFoil || !pFoilItem) return;

    for(int iPolar=0; iPolar<Objects2d::nPolars(); iPolar++)
    {
        Polar *pPolar = Objects2d::polarAt(iPolar);
        if(pPolar && pPolar->foilName().compare(pFoil->name())==0)
        {
            Polar *pPolar = Objects2d::polarAt(iPolar);
            if(!pPolar) continue;
            if(pPolar && pPolar->foilName().compare(pFoil->name())==0)
            {
                LineStyle ls(pPolar->theStyle());
                ls.m_bIsEnabled = true;
                m_pModel->appendRow(pFoilItem, pPolar->name(), ls, polarState(pPolar));
            }
            addOpps(pPolar);
        }
    }
}


Qt::CheckState FoilExplorer::foilState(Foil const *pFoil) const
{
    bool bAll = true;
    bool bNone = true;
    if(s_pXDirect->isDesignView())
    {
        if(pFoil->isVisible()) return Qt::Checked;
        else                   return Qt::Unchecked;
    }
    else if(s_pXDirect->isPolarView())
    {
        for(int iplr=0; iplr<Objects2d::nPolars(); iplr++)
        {
            Polar const *pPolar = Objects2d::polarAt(iplr);
            if(pFoil->name()==pPolar->foilName())
            {
                bAll = bAll && pPolar->isVisible();
                bNone = bNone && !pPolar->isVisible();
            }
        }
        if(bAll && bNone)  return Qt::Unchecked; // foil has no polars
        else if(bAll)      return Qt::Checked;
        else if(bNone)     return Qt::Unchecked;
        else               return Qt::PartiallyChecked;
    }
    else
    {
        for(int iopp=0; iopp<Objects2d::nOpPoints(); iopp++)
        {
            OpPoint const *pOpp = Objects2d::opPointAt(iopp);
            if(pFoil->name()==pOpp->foilName())
            {
                bAll = bAll && pOpp->isVisible();
                bNone = bNone && !pOpp->isVisible();
            }
        }
        if(bAll)       return Qt::Checked;
        else if(bNone) return Qt::Unchecked;
        else           return Qt::PartiallyChecked;
    }
}


Qt::CheckState FoilExplorer::polarState(Polar const *pPolar) const
{
    switch(s_pXDirect->eView())
    {
        case XDirect::OPPVIEW:
        case XDirect::BLVIEW:
        {
            bool bAll = true;
            bool bNone = true;
            for(int iopp=0; iopp<Objects2d::nOpPoints(); iopp++)
            {
                OpPoint const *pOpp = Objects2d::opPointAt(iopp);
                if(pPolar->hasOpp(pOpp))
                {
                    bAll = bAll && pOpp->isVisible();
                    bNone = bNone && !pOpp->isVisible();
                }
            }
            if     (bNone) return Qt::Unchecked;
            else if(bAll)  return Qt::Checked;
            else           return Qt::PartiallyChecked;
        }
        case XDirect::POLARVIEW:
        {
            return pPolar->isVisible() ? Qt::Checked : Qt::Unchecked;
        }
        default:
        case XDirect::DESIGNVIEW:
            return Qt::Unchecked; // doesn't matter
    }
    return Qt::Unchecked;
}


void FoilExplorer::setObjectFromIndex(const QModelIndex &index)
{
    ObjectTreeItem *pSelectedItem = nullptr;

    if(index.column()==0)
    {
        pSelectedItem = m_pModel->itemFromIndex(index);
    }
    else if(index.column()>=1)
    {
        QModelIndex ind = index.sibling(index.row(), 0);
        pSelectedItem = m_pModel->itemFromIndex(ind);
    }

    if(!pSelectedItem) return;

    if(pSelectedItem->level()==1)
    {
        Foil *m_pFoil = Objects2d::foil(pSelectedItem->name().toStdString());
        s_pXDirect->setFoil(m_pFoil);
        XDirect::setCurPolar(nullptr);
        XDirect::setCurOpp(nullptr);
        m_Selection = FoilExplorer::FOIL;
    }
    else if(pSelectedItem->level()==2)
    {
        ObjectTreeItem const*pFoilItem = pSelectedItem->parentItem();
        Foil *m_pFoil = Objects2d::foil(pFoilItem->name().toStdString());
        Polar *m_pPolar = Objects2d::polar(m_pFoil, pSelectedItem->name().toStdString());
        s_pXDirect->setFoil(m_pFoil);
        s_pXDirect->setPolar(m_pPolar);
        XDirect::setCurOpp(nullptr);
        m_Selection = FoilExplorer::POLAR;
    }
    else if(pSelectedItem->level()==3)
    {
        ObjectTreeItem *pWPolarItem = pSelectedItem->parentItem();
        ObjectTreeItem *pFoilItem = pWPolarItem->parentItem();
        Foil *pFoil   = Objects2d::foil(pFoilItem->name().toStdString());
        Polar *pPolar = Objects2d::polar(pFoil->name(), pWPolarItem->name().toStdString());

        QString strange = pSelectedItem->name();
        strange = strange.remove(DEGch);
        double val = strange.toDouble();
        OpPoint *pOpp = Objects2d::opPointAt(pFoil, pPolar, val);

        s_pXDirect->setFoil(pFoil);
        s_pXDirect->setPolar(pPolar);
        s_pXDirect->setOpp(pOpp);
        m_Selection = FoilExplorer::OPPOINT;
    }
    else m_Selection = FoilExplorer::NONE;

    s_pXDirect->setControls();
    setObjectProperties();
}


void FoilExplorer::onCurrentRowChanged(QModelIndex currentIndex, QModelIndex)
{
    setObjectFromIndex(currentIndex);
    s_pXDirect->updateView();
}


void FoilExplorer::onItemClicked(const QModelIndex &index)
{
    Foil *pFoil   = s_pXDirect->curFoil();
    Polar *pPolar = s_pXDirect->curPolar();
    OpPoint *pOpp = s_pXDirect->curOpp();
    if(index.column()==1)
    {
        ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);

        if(pOpp)
        {
            LineStyle ls(pOpp->theStyle());
            LineMenu *pLineMenu = new LineMenu(nullptr);
            pLineMenu->initMenu(ls);
            pLineMenu->exec(QCursor::pos());
            ls = pLineMenu->theStyle();
            pOpp->setLineStipple(ls.m_Stipple);
            pOpp->setLineWidth(ls.m_Width);
            pOpp->setLineColor(ls.m_Color);
            pOpp->setPointStyle(ls.m_Symbol);
            pItem->setTheStyle(ls);
        }
        else if(pPolar)
        {
            LineStyle ls(pPolar->theStyle());
            LineMenu *pLineMenu = new LineMenu(nullptr);
            pLineMenu->initMenu(ls);
            pLineMenu->exec(QCursor::pos());
            ls = pLineMenu->theStyle();
            Objects2d::setPolarStyle(pPolar, ls, pLineMenu->styleChanged(), pLineMenu->widthChanged(), pLineMenu->colorChanged(), pLineMenu->pointsChanged(), true, xfl::darkFactor());
            pItem->setTheStyle(ls);
            // update children items
            updateLineStyles();
        }
        else if(pFoil)
        {
            LineStyle ls(pFoil->theStyle());
            LineMenu *pLineMenu = new LineMenu(nullptr);
            pLineMenu->initMenu(ls);
            pLineMenu->exec(QCursor::pos());
            ls = pLineMenu->theStyle();
            Objects2d::setFoilStyle(pFoil, ls, pLineMenu->styleChanged(), pLineMenu->widthChanged(), pLineMenu->colorChanged(), pLineMenu->pointsChanged(), true, xfl::darkFactor());
            pItem->setTheStyle(ls);
            // update children items
            updateLineStyles();
        }
    }
    else if (index.column()==2)
    {
        if(pOpp)
        {
            pOpp->setVisible(!pOpp->isVisible());
            updateVisibilityBoxes(); // to update polar and foil states
        }
        else if(pPolar)
        {
            if(s_pXDirect->isPolarView())
            {
                Objects2d::setPolarVisible(pPolar, !pPolar->isVisible());
            }
            else
            {
                Qt::CheckState state = polarState(pPolar);
                if(state==Qt::PartiallyChecked || state==Qt::Unchecked)
                    Objects2d::setPolarVisible(pPolar, true);
                else if(state==Qt::Checked)
                    Objects2d::setPolarVisible(pPolar, false);
            }

            updateVisibilityBoxes();
        }
        else if(pFoil)
        {
            Qt::CheckState state = foilState(pFoil);
            if(s_pXDirect->isDesignView())
            {
                if(state==Qt::PartiallyChecked || state==Qt::Unchecked)
                    pFoil->setVisible(true);
                else if(state==Qt::Checked)
                    pFoil->setVisible(false);
            }
            else
            {
                if(state==Qt::PartiallyChecked || state==Qt::Unchecked)
                    Objects2d::setFoilVisible(pFoil, true, true);
                else if(state==Qt::Checked)
                    Objects2d::setFoilVisible(pFoil, false, false);
            }

            updateVisibilityBoxes();
        }
        setOverallCheckStatus();
    }
    s_pXDirect->resetCurves();
    s_pXDirect->m_pDFoilWt->resetLegend();
    s_pXDirect->updateView();

    m_pTreeView->update();
    emit(s_pXDirect->projectModified());
//    m_pModel->updateDataFromRoot();
}


void FoilExplorer::onItemDoubleClicked(const QModelIndex &index)
{
    setObjectFromIndex(index);
    s_pXDirect->resetCurves();
    s_pXDirect->updateView();
    if(m_Selection==FoilExplorer::POLAR)
        s_pXDirect->onEditCurPolar();
}


void FoilExplorer::selectFoil(Foil*pFoil)
{
    if(!pFoil) pFoil = XDirect::curFoil();
    if(!pFoil)
    {
        setObjectProperties();
        return;
    }

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pItem = m_pModel->item(ir);

        // is it the correct foil name?
        if(pItem->name().toStdString().compare(pFoil->name())==0)
        {
            // select the foil's row
            QModelIndex idx = m_pModel->index(pItem->parentItem(), pItem);
            m_pTreeView->setCurrentIndex(idx);
            m_pTreeView->scrollTo(idx);
            m_Selection = FoilExplorer::FOIL;

            break;
        }
    }
    setObjectProperties();
}


void FoilExplorer::selectPolar(Polar*pPolar)
{
    if(!pPolar) pPolar = XDirect::curPolar();
    if(!pPolar)
    {
        setObjectProperties();
        return;
    }

    // the foil item is the polar's parent item
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);

        // is it the correct foil name?
        if(pFoilItem->name().toStdString().compare(pPolar->foilName())==0)
        {
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                if(pFoilItem->child(jr))
                {
                    const QModelIndex &polarChild = m_pModel->index(jr,0,pFoilItem);

                    ObjectTreeItem *pPolarItem = m_pModel->itemFromIndex(polarChild);
                    // is it the correct polar name?
                    if(pPolarItem->name().toStdString().compare(pPolar->name())==0)
                    {
                        // select the polar row
                        QModelIndex idx = m_pModel->index(pFoilItem, pPolarItem);
                        m_pTreeView->expand(idx);
                        m_pTreeView->setCurrentIndex(polarChild);
                        m_pTreeView->scrollTo(polarChild);
                        m_Selection = FoilExplorer::POLAR;

                        break;
                    }
                }
            }
        }
    }
    setObjectProperties();
}


void FoilExplorer::selectOpPoint(OpPoint *pOpp)
{
    if(!pOpp) pOpp = XDirect::curOpp();
    if(!pOpp)
    {
        setObjectProperties();
        return;
    }

    bool bSelected = false;

    for(int ir = 0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);

        // is it the correct foil name?
        if(pFoilItem->name().toStdString().compare(pOpp->foilName())==0)
        {
            //browse the polars
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pFoilItem->child(jr);
                if(pPolarItem)
                {
                    const QModelIndex &polarChild = m_pModel->index(pFoilItem, pPolarItem);

                    ObjectTreeItem *pPolarItem = m_pModel->itemFromIndex(polarChild);
                    // is it the correct polar name?
                    if(pPolarItem && pPolarItem->name().toStdString().compare(pOpp->polarName())==0)
                    {
                        //browse the opps
                        for(int kr=0; kr<pPolarItem->rowCount(); kr++)
                        {
                            Foil *m_pFoil = Objects2d::foil(pFoilItem->name().toStdString());
                            Polar *m_pPolar = Objects2d::polar(m_pFoil, pPolarItem->name().toStdString());


                            ObjectTreeItem *pOppItem = pPolarItem->child(kr);
                            if(pOppItem)
                            {
                                const QModelIndex &oppChild = m_pModel->index(pPolarItem, pOppItem);
                                double val = pOppItem->name().toDouble();
                                // is it the correct aoa?
                                bool bCorrect = false;
                                Q_ASSERT(m_pPolar!=nullptr);
                                if(m_pPolar->isControlPolar())
                                {
                                    bCorrect =(fabs(val-pOpp->theta())<FLAPANGLEPRECISION);
                                }
                                else if(m_pPolar->isFixedaoaPolar())
                                {
                                    bCorrect =(fabs(val-pOpp->Reynolds())<REYNOLDSPRECISION);
                                }
                                else
                                {
                                    bCorrect =(fabs(val-pOpp->aoa())<AOAPRECISION);
                                }
                                if(bCorrect)
                                {
                                    // select the opp row
                                    m_pTreeView->setCurrentIndex(oppChild);
                                    m_pTreeView->scrollTo(oppChild);
                                    m_Selection = FoilExplorer::OPPOINT;
                                    setCurveParams();
                                    bSelected = true;
                                    break;
                                }

                            }
                        }
                    }
                }
                if(bSelected) break;
            }
        }
        if(bSelected) break;
    }
    setObjectProperties();
}


void FoilExplorer::addOpps(Polar *pPolar)
{
    if(!pPolar) return;
//    QString format = xfl::isLocalized() ? "%L1" : "%1";
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);
        // find the polar's parent foil
        if(pFoilItem->name().toStdString().compare(pPolar->foilName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                ObjectTreeItem *pOldPolarItem = pFoilItem->child(jr);
                if(pOldPolarItem)
                {
                    const QModelIndex &oldPolarChild = m_pModel->index(pFoilItem, pOldPolarItem);
                    if(pOldPolarItem->name().toStdString().compare(pPolar->name())==0)
                    {
                        //clear the children
                        m_pModel->removeRows(0, pOldPolarItem->rowCount(), oldPolarChild);
                        //insert POpps
                        for(int kr=0; kr<Objects2d::nOpPoints(); kr++)
                        {
                            OpPoint * pOpp = Objects2d::opPointAt(kr);
                            if(pOpp->foilName().compare(pPolar->foilName())==0 && pOpp->polarName().compare(pPolar->name())==0)
                            {
                                LineStyle ls(pOpp->theStyle());
                                ls.m_bIsEnabled = !s_pXDirect->isPolarView();
                                m_pModel->appendRow(pOldPolarItem, pOpp->name(), ls, Qt::PartiallyChecked);
                            }
                        }
                        return;
                    }
                }
            }
        }
    }
    setOverallCheckStatus();
}


void FoilExplorer::insertFoil(Foil *pFoil)
{
    if(!pFoil) pFoil = XDirect::curFoil();
    if(!pFoil) return;

    bool bInserted = false;
    for(int ir = 0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);

        // insert alphabetically
        if(pFoilItem->name().toStdString().compare(pFoil->name())==0)
        {
            //A foil of that name already exists
            // clear its rows
            QModelIndex foilindex= m_pModel->index(m_pModel->rootItem(), pFoilItem);
            m_pModel->removeRows(0, pFoilItem->rowCount(), foilindex);
            bInserted = true;
            break;
        }
        else if(pFoilItem->name().toStdString().compare(pFoil->name())>0)
        {
            //insert before
            m_pModel->insertRow(m_pModel->rootItem(), ir, pFoil->name(), pFoil->theStyle(), foilState(pFoil));
            bInserted = true;
            break;
        }
    }

    if(!bInserted)
    {
        //not inserted, append
        ObjectTreeItem* pRootItem = m_pModel->rootItem();
        m_pModel->appendRow(pRootItem, pFoil->name(), pFoil->theStyle(), foilState(pFoil));

    }

    for(int ip=0; ip<Objects2d::nPolars(); ip++)
    {
        Polar *pPolar = Objects2d::polarAt(ip);
        if(pPolar->foilName().compare(pFoil->name())==0)
            insertPolar(pPolar);
    }

    setOverallCheckStatus();
}


void FoilExplorer::insertPolar(Polar const*pPolar)
{
    if(!pPolar) pPolar = XDirect::curPolar();
    if(!pPolar) return;

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);

        // find the polar's parent foil
        if(pFoilItem->name().toStdString().compare(pPolar->foilName())==0)
        {
            //if this polar name exists, don't insert it
            ObjectTreeItem *pNewPolarItem = nullptr;
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                ObjectTreeItem *pOldPolarItem = pFoilItem->child(jr);
                Polar const*pOldPolar = Objects2d::polar(pPolar->foilName(), pOldPolarItem->name().toStdString());
                if(pOldPolar)
                {
                    int res = pPolar->compareTo(pOldPolar);
                    if(res==0)
                    {
                        QModelIndex polarindex = m_pModel->index(jr, 0, pFoilItem);
                        // remove the discarded children Opps
                        m_pModel->removeRows(0, pOldPolarItem->rowCount(), polarindex);

                        pNewPolarItem = pOldPolarItem;
                    }
                    else if(res<0)
                    {
                        //insert before
                        pNewPolarItem = m_pModel->insertRow(pFoilItem, jr, pPolar->name(), pPolar->theStyle(), polarState(pPolar));
                    }
                    if(pNewPolarItem) break;
                }
            }

            if(!pNewPolarItem)
            {
                m_pModel->appendRow(pFoilItem, pPolar->name(), pPolar->theStyle(), polarState(pPolar));
                break;
            }
        }
    }

    setOverallCheckStatus();
}


void FoilExplorer::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QModelIndex idx = m_pTreeView->currentIndex();
    ObjectTreeItem *pItem = m_pModel->itemFromIndex(idx);
    if(!pItem) return;
    QString strong;
    Foil *pFoil   = s_pXDirect->curFoil();
    Polar *pPolar = s_pXDirect->curPolar();
    OpPoint *pOpp = s_pXDirect->curOpp();

    if(pItem->level()==1)
    {
        pFoil = Objects2d::foil(pItem->name().toStdString());
        if(pFoil) strong  = QString::fromStdString(pFoil->name());
        else      strong.clear();
    }
    else if(pItem->level()==2)
    {
        strong = QString::fromStdString(pPolar->name());
    }
    else if(pItem->level()==3)
    {
        ObjectTreeItem *pPolarItem = pItem->parentItem();
        ObjectTreeItem *pFoilItem = pPolarItem->parentItem();
        pFoil  = Objects2d::foil(pFoilItem->name().toStdString());
        pPolar = Objects2d::polar(pFoil, pPolarItem->name().toStdString());
        pOpp   = Objects2d::opPointAt(pFoil, pPolar, pItem->name().toDouble());
        strong = pItem->name();
    }


    if(m_Selection==FoilExplorer::OPPOINT && pOpp)
        s_pXDirect->m_pMenus->m_pActiveOppMenu->exec(pEvent->globalPos());
    else if(m_Selection==FoilExplorer::POLAR && pPolar)
        s_pXDirect->m_pMenus->m_pActivePolarMenu->exec(pEvent->globalPos());
    else if(m_Selection==FoilExplorer::FOIL && pFoil)
    {
        s_pXDirect->m_pMenus->m_pActiveFoilMenu->exec(pEvent->globalPos());
        setCurveParams();
    }
    s_pXDirect->updateView();
    update();

    pEvent->accept();
}


QString FoilExplorer::removeFoil(Foil* pFoil)
{
    if(!pFoil) return "false";
    return removeFoil(QString::fromStdString(pFoil->name()));
}


QString FoilExplorer::removeFoil(QString const &foilName)
{
    if(!foilName.length()) return "";

    m_pTreeView->selectionModel()->blockSignals(true);

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pItem = m_pModel->item(ir);
        // scan
        if(pItem->name().compare(foilName)==0)
        {
            // foil found
            m_pModel->removeRow(ir);
            break;
        }
    }

    m_pTreeView->selectionModel()->blockSignals(false);
    setOverallCheckStatus();
    return QString("");
}


QString FoilExplorer::removePolar(Polar *pPolar)
{
    if(!pPolar) return QString("");

    m_pTreeView->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);
        QModelIndex planeindex = m_pModel->index(ir, 0);
        // find the polar's parent Plane
        if(pFoilItem->name().toStdString().compare(pPolar->foilName())==0)
        {
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                ObjectTreeItem *pOldPolarItem = pFoilItem->child(jr);

                if(pOldPolarItem)
                {
                    if(pOldPolarItem->name().toStdString().compare(pPolar->name())==0)
                    {
                        m_pModel->removeRow(jr, planeindex);
//                        pPlaneItem->removeRow(jr);
                        m_pTreeView->selectionModel()->blockSignals(false);

                        // find the previous item, or the next one if this polar is the first
                        if(pFoilItem->rowCount())
                        {
                            jr =std::min(jr, pFoilItem->rowCount()-1);
                            return pFoilItem->child(jr)->name();
                        }
                        return "";
                    }
                }
            }
        }
    }

    setOverallCheckStatus();
    m_pTreeView->selectionModel()->blockSignals(false);
    return QString();
}


void FoilExplorer::removeOpPoint(OpPoint *pOpp)
{
    if(!pOpp) return;
    m_pTreeView->selectionModel()->blockSignals(true);

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);
        // find the polar's parent foil
        if(pFoilItem->name().toStdString().compare(pOpp->foilName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pFoilItem->child(jr);
                if(pPolarItem && pPolarItem->name().toStdString().compare(pOpp->polarName())==0)
                {
                    Polar *pPolar = Objects2d::polar(pOpp->foilName(), pOpp->polarName());
                    if(!pPolar)
                    {
                        m_pTreeView->selectionModel()->blockSignals(false);
                        return; // error, should exist
                    }

                    //find the POpp item
                    for(int kr=0; kr<pPolarItem->rowCount(); kr++)
                    {
                        QModelIndex polarindex = m_pModel->index(pFoilItem, pPolarItem);
                        ObjectTreeItem *pOppItem = pPolarItem->child(kr);
                        if(pOppItem)
                        {
                            if(pOppItem->name().compare(QString::fromStdString(pOpp->name()))==0)
                            {
                                m_pModel->removeRow(kr, polarindex);
                                m_pTreeView->selectionModel()->blockSignals(false);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    setOverallCheckStatus();
    m_pTreeView->selectionModel()->blockSignals(false);
}


void FoilExplorer::removeFoilPolars(Foil *pFoil)
{
    if(!pFoil) return;
    m_pTreeView->selectionModel()->blockSignals(true);

    for(int ir = 0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);

        if(pFoilItem->name().toStdString().compare(pFoil->name())==0)
        {
            // foil found
            QModelIndex foilindex= m_pModel->index(m_pModel->rootItem(), pFoilItem);

            // remove the children opps - may not be necessary
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pFoilItem->child(jr);

                QModelIndex polarindex = m_pModel->index(jr, 0, pFoilItem);
                // remove the children Opps
                m_pModel->removeRows(0, pPolarItem->rowCount(), polarindex);
            }
            // remove the polars
            m_pModel->removeRows(0, pFoilItem->rowCount(), foilindex);
            break;
        }
    }
    setOverallCheckStatus();

    m_pTreeView->selectionModel()->blockSignals(false);
}


void FoilExplorer::removePolarOpps(Polar const*pPolar)
{
    if(!pPolar) return;
    m_pTreeView->selectionModel()->blockSignals(true);

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);
        // find the polar's parent Plane item
        if(pFoilItem->name().toStdString().compare(pPolar->foilName())==0)
        {
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pFoilItem->child(jr);
                if(pPolarItem->name().toStdString().compare(pPolar->name())==0)
                {
                    QModelIndex polarindex = m_pModel->index(jr, 0, pFoilItem);
                    m_pModel->removeRows(0, pPolarItem->rowCount(), polarindex);
                    break;
                }
            }
        }
    }

//    m_pModel->updateData();
    setOverallCheckStatus();
    m_pTreeView->selectionModel()->blockSignals(false);

}


void FoilExplorer::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Delete:
        {
            if     (m_Selection==FoilExplorer::OPPOINT) s_pXDirect->onDeleteCurOpp();
            else if(m_Selection==FoilExplorer::POLAR)   s_pXDirect->onDeleteCurPolar();
            else if(m_Selection==FoilExplorer::FOIL)    s_pXDirect->onDeleteCurFoil();

            pEvent->accept();
            break;
        }
        default:
            pEvent->ignore();
    }
    if(!pEvent->isAccepted())
        s_pXDirect->keyPressEvent(pEvent);
}


/** update the line properties for each foil, polar and opp item in the treeview */
void FoilExplorer::setCurveParams()
{
    ObjectTreeItem const *pRootItem = m_pModel->rootItem();
    for(int i0=0; i0<pRootItem->rowCount(); i0++)
    {
        ObjectTreeItem *pFoilItem = pRootItem->child(i0);
        if(!pFoilItem) continue;
        Foil *pFoil = Objects2d::foil(pFoilItem->name().toStdString());
        if(!pFoil) continue;

        LineStyle ls(pFoil->isVisible(), pFoil->lineStipple(), pFoil->lineWidth(), pFoil->lineColor(), pFoil->pointStyle());
        ls.m_bIsEnabled = true;
        pFoilItem->setTheStyle(ls);
        pFoilItem->setCheckState(foilState(pFoil));

        for(int i1=0; i1< pFoilItem->rowCount(); i1++)
        {
            if(s_pXDirect->isDesignView())
            {
            }
            else
            {
                ObjectTreeItem *pPolarItem = pFoilItem->child(i1);
                if(pPolarItem)
                {
                    Polar *pPolar = Objects2d::polar(pFoilItem->name().toStdString(), pPolarItem->name().toStdString());
                    if(!pPolar) continue;

                    LineStyle ls(pPolar->theStyle());
                    ls.m_bIsEnabled = true;
                    pPolarItem->setTheStyle(ls);
                    pPolarItem->setCheckState(polarState(pPolar));

                    for(int i2=0; i2<pPolarItem->rowCount(); i2++)
                    {
                        ObjectTreeItem *pOppItem = pPolarItem->child(i2);
                        if(pOppItem)
                        {
                            QString strange = pOppItem->name();
                            strange = strange.remove(DEGch);
                            double val = strange.toDouble();
                            OpPoint *pOpp = Objects2d::opPointAt(pFoil, pPolar, val);

                            if(!pOpp) continue;

                            LineStyle ls(pOpp->theStyle());
                            ls.m_bIsEnabled = true;
                            pOppItem->setTheStyle(ls);
                            bool bChecked = pOpp->isVisible() && (s_pXDirect->isOppView() || s_pXDirect->isBLView());
                            pOppItem->setCheckState(bChecked ? Qt::Checked : Qt::Unchecked);
                        }
                    }
                }
            }
        }
    }
}


void FoilExplorer::setTreeFont(QFont const &fnt)
{
    m_pTreeView->setFont(fnt);
}



void FoilExplorer::setPropertiesFont(QFont const &fnt)
{
    m_ppto->setFont(fnt);
}


void FoilExplorer::onSwitchAll(bool bChecked)
{
    if(s_pXDirect->isOppView())
    {
        if(bChecked) s_pXDirect->onShowAllOpps();
        else         s_pXDirect->onHideAllOpps();
    }
    else if(s_pXDirect->isPolarView())
    {
        if(bChecked) s_pXDirect->onShowAllPolars();
        else         s_pXDirect->onHideAllPolars();
    }
    s_pXDirect->resetCurves();
    s_pXDirect->updateView();

    updateVisibilityBoxes();
}


void FoilExplorer::setOverallCheckStatus()
{
    if(s_pXDirect->isOppView())
    {
        bool bAllChecked   = true;
        bool bAllUnchecked = true;
        for(int io=0; io<Objects2d::nOpPoints(); io++)
        {
            OpPoint *const pOpp=Objects2d::opPointAt(io);
            if(pOpp->isVisible()) bAllUnchecked = false;
            else                  bAllChecked   = false;
        }

        if     (bAllChecked)   m_pTreeView->setOverallCheckedState(Qt::Checked);
        else if(bAllUnchecked) m_pTreeView->setOverallCheckedState(Qt::Unchecked);
        else                   m_pTreeView->setOverallCheckedState(Qt::PartiallyChecked);
    }
    else if(s_pXDirect->isPolarView())
    {
        bool bAllChecked   = true;
        bool bAllUnchecked = true;
        for(int io=0; io<Objects2d::nPolars(); io++)
        {
            Polar *const pPolar=Objects2d::polarAt(io);
            if(pPolar->isVisible()) bAllUnchecked = false;
            else                    bAllChecked   = false;
        }

        if     (bAllChecked)   m_pTreeView->setOverallCheckedState(Qt::Checked);
        else if(bAllUnchecked) m_pTreeView->setOverallCheckedState(Qt::Unchecked);
        else                   m_pTreeView->setOverallCheckedState(Qt::PartiallyChecked);
    }
    else
    {
        m_pTreeView->setOverallCheckedState(Qt::Unchecked);
    }
}


void FoilExplorer::onSetFilter()
{
    QString filter = m_pTreeView->filter();
    QStringList filters = filter.split(QRegularExpression("\\s+"));

    if(filters.size()==0)
    {
        for(int jp=0; jp<Objects2d::nPolars(); jp++)
        {
            Objects2d::polarAt(jp)->setVisible(true);
        }
    }
    else if(filters.size()==1)
    {
        for(int jp=0; jp<Objects2d::nPolars(); jp++)
        {
            Polar *pPolar = Objects2d::polarAt(jp);
            bool bVisible = QString::fromStdString(pPolar->name()).contains(filter, Qt::CaseInsensitive);
            Objects2d::setPolarVisible(pPolar, bVisible);
        }

        for(int ip=0; ip<Objects2d::nFoils(); ip++)
        {
            Foil *pFoil = Objects2d::foil(ip);
            if(QString::fromStdString(pFoil->name()).contains(filter, Qt::CaseInsensitive))
                Objects2d::setFoilVisible(pFoil, true, false);
        }
    }
    else
    {
        QString foilfilter = filters.front();
        QString polarfilter = filters.at(1);
        for(int jp=0; jp<Objects2d::nPolars(); jp++)
        {
            Objects2d::polarAt(jp)->setVisible(false);
        }

        for(int ip=0; ip<Objects2d::nFoils(); ip++)
        {
            Foil const *pFoil = Objects2d::foilAt(ip);
            if(QString::fromStdString(pFoil->name()).contains(foilfilter, Qt::CaseInsensitive))
            {
                for(int jp=0; jp<Objects2d::nPolars(); jp++)
                {
                    Polar *pPolar = Objects2d::polarAt(jp);
                    if(pPolar->foilName()==pFoil->name())
                    {
                        bool bVisible = QString::fromStdString(pPolar->name()).contains(polarfilter, Qt::CaseInsensitive);
                        Objects2d::setPolarVisible(pPolar, bVisible);
                    }
                }
            }
        }
    }

    updateVisibilityBoxes();

    setOverallCheckStatus();

    s_pXDirect->resetCurves();
    s_pXDirect->updateView();

    emit s_pXDirect->projectModified();

}


void FoilExplorer::updateVisibilityBoxes()
{
    const int CHECKCOLUMN = 2;
    ObjectTreeItem const *pRootItem = m_pModel->rootItem();
    for(int ir=0; ir<pRootItem->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = pRootItem->child(ir);
        if(!pFoilItem) continue;
        Foil *pFoil = Objects2d::foil(pFoilItem->name().toStdString());
        if(!pFoil) continue;

        QModelIndex checkindex = m_pModel->index(ir, CHECKCOLUMN);
        m_pModel->setData(checkindex, foilState(pFoil), Qt::DisplayRole);

        for(int jr=0; jr<pFoilItem->rowCount(); jr++)
        {
            ObjectTreeItem *pPolarItem = pFoilItem->child(jr);
            Polar *pPolar = Objects2d::polar(pFoil, pPolarItem->name().toStdString());
            if(!pPolar) continue;

            QModelIndex checkindex = m_pModel->index(jr, CHECKCOLUMN, pFoilItem);
            m_pModel->setData(checkindex, polarState(pPolar), Qt::DisplayRole);

            for(int i2=0; i2<pPolarItem->rowCount(); i2++)
            {
                ObjectTreeItem *pOppItem = pPolarItem->child(i2);
                if(pOppItem)
                {
                    QString strange = pOppItem->name();
                    strange = strange.remove(DEGch);
                    double val = strange.toDouble();
                    OpPoint *pOpp = Objects2d::opPointAt(pFoil, pPolar, val);

                    if(!pOpp) continue;

                    bool bChecked = pOpp->isVisible() && (s_pXDirect->isOppView() || s_pXDirect->isBLView());
                    Qt::CheckState state = bChecked ? Qt::Checked : Qt::Unchecked;

                    QModelIndex checkindex = m_pModel->index(i2, CHECKCOLUMN, pPolarItem);
                    m_pModel->setData(checkindex, state, Qt::DisplayRole);
                }
            }
        }
    }
}


void FoilExplorer::updateLineStyles()
{
    const int STYLECOLUMN = 1;
    ObjectTreeItem const *pRootItem = m_pModel->rootItem();
    for(int ir=0; ir<pRootItem->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = pRootItem->child(ir);
        if(!pFoilItem) continue;
        Foil *pFoil = Objects2d::foil(pFoilItem->name().toStdString());
        if(!pFoil) continue;

        QModelIndex styleindex = m_pModel->index(ir, STYLECOLUMN);
        m_pModel->setData(styleindex, QVariant::fromValue(pFoil->theStyle()), Qt::DisplayRole);

        for(int jr=0; jr<pFoilItem->rowCount(); jr++)
        {
            ObjectTreeItem *pPolarItem = pFoilItem->child(jr);
            Polar *pPolar = Objects2d::polar(pFoil, pPolarItem->name().toStdString());
            if(!pPolar) continue;

            QModelIndex plrstyleindex = m_pModel->index(jr, STYLECOLUMN, pFoilItem);
            m_pModel->setData(plrstyleindex, QVariant::fromValue(pPolar->theStyle()), Qt::DisplayRole);

            for(int i2=0; i2<pPolarItem->rowCount(); i2++)
            {
                ObjectTreeItem *pOppItem = pPolarItem->child(i2);
                if(pOppItem)
                {
                    QString strange = pOppItem->name();
                    strange = strange.remove(DEGch);
                    double val = strange.toDouble();
                    OpPoint *pOpp = Objects2d::opPointAt(pFoil, pPolar, val);

                    if(!pOpp) continue;

                    QModelIndex checkindex = m_pModel->index(i2, STYLECOLUMN, pPolarItem);
                    m_pModel->setData(checkindex, QVariant::fromValue(pOpp->theStyle()), Qt::DisplayRole);
                }
            }
        }
    }
}









