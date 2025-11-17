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

#define _MATH_DEFINES_DEFINED

#include <QHeaderView>
#include <QVBoxLayout>

#include "planetreeview.h"

#include <globals/mainframe.h>
#include <modules/xplane/xplane.h>
#include <modules/xplane/controls/analysis3dctrls.h>
#include <modules/xplane/menus/xplanemenus.h>

#include <api/units.h>
#include <core/xflcore.h>
#include <api/objects3d.h>
#include <api/objects_global.h>
#include <interfaces/widgets/mvc/expandabletreeview.h>
#include <interfaces/widgets/mvc/objecttreeitem.h>
#include <interfaces/widgets/mvc/objecttreemodel.h>
#include <interfaces/widgets/mvc/objecttreedelegate.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/line/linemenu.h>
#include <api/plane.h>
#include <api/planepolar.h>
#include <api/planeopp.h>


MainFrame *PlaneTreeView::s_pMainFrame(nullptr);
XPlane *PlaneTreeView::s_pXPlane(nullptr);
int PlaneTreeView::s_Width(351);
QByteArray PlaneTreeView::s_SplitterSizes;

PlaneTreeView::PlaneTreeView(QWidget *pParent) : QWidget(pParent)
{
    m_pTreeView = nullptr;
    m_pModel    = nullptr;

    m_pptObjectProps = new PlainTextOutput;
    m_Selection = PlaneTreeView::NOOBJECT;

    setupLayout();

    m_pModel = new ObjectTreeModel(this);
    m_pModel->setHeaderData(0, Qt::Horizontal, "Objects", Qt::DisplayRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, "1234567890123", Qt::EditRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, "1234567890123", Qt::DisplayRole);
    m_pModel->setHeaderData(2, Qt::Horizontal, "123", Qt::DisplayRole);
    m_pModel->setHeaderData(2, Qt::Horizontal, Qt::AlignRight, Qt::TextAlignmentRole);

    m_pTreeView->setModel(m_pModel);

    m_pTreeView->setRootIndex(QModelIndex());
    m_pTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_pTreeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // so that ExpandableTreeView::sizeHintForColumn is used
    m_pTreeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // so that ExpandableTreeView::sizeHintForColumn is used

    m_pDelegate = new ObjectTreeDelegate(this);
    m_pTreeView->setItemDelegate(m_pDelegate);

    connect(m_pTreeView,                   SIGNAL(pressed(QModelIndex)),         SLOT(onItemClicked(QModelIndex)));
    connect(m_pTreeView,                   SIGNAL(doubleClicked(QModelIndex)),   SLOT(onItemDoubleClicked(QModelIndex)));
    connect(m_pTreeView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentRowChanged(QModelIndex)));
    connect(m_pTreeView->m_pleFilter,      SIGNAL(returnPressed()),              SLOT(onSetFilter()));
}


void PlaneTreeView::showEvent(QShowEvent *pEvent)
{
    m_pMainSplitter->restoreState(s_SplitterSizes);
    QWidget::showEvent(pEvent);
}


void PlaneTreeView::hideEvent(QHideEvent *)
{
    s_Width = width();
    s_SplitterSizes = m_pMainSplitter->saveState();
}


void PlaneTreeView::resizeEvent(QResizeEvent *pEvent)
{
    updateGeometry(); // Notifies the layout system that the sizeHint() has changed
    pEvent->accept();
}


void PlaneTreeView::setObjectProperties()
{
    Plane const *pPlane = s_pXPlane->m_pCurPlane;
    std::string props;
    switch(m_Selection)
    {
        case PlaneTreeView::PLANE:
        {
            if(pPlane)
            {
                if(pPlane->description().length())
                {
                    props = pPlane->description() + "\n\n";
                }
                if(s_pXPlane->m_pCurWPolar)
                    props += pPlane->planeData(s_pXPlane->m_pCurWPolar->bIncludeOtherWingAreas());
                else
                    props += pPlane->planeData(false);
            }
            break;
        }
        case PlaneTreeView::WPOLAR:
        {
            if(s_pXPlane->m_pCurWPolar && pPlane)
            {
                s_pXPlane->m_pCurWPolar->getProperties(props, pPlane);
                break;
            }
            break;
        }
        case PlaneTreeView::PLANEOPP:
        {
            if(s_pXPlane->m_pCurPOpp)
            {
                s_pXPlane->m_pCurPOpp->getProperties(pPlane, s_pXPlane->m_pCurWPolar, props);
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
    m_pptObjectProps->setStdText(props);
}


void PlaneTreeView::setPropertiesFont(QFont const &fnt)
{
    m_pptObjectProps->setFont(fnt);
}


void PlaneTreeView::setTreeFontStruct(const FontStruct &fntstruct)
{
    m_pTreeView->setFont(fntstruct.font());
    //    setFont(fntstruct.font());
}


void PlaneTreeView::setupLayout()
{
    m_pTreeView = new ExpandableTreeView;
    m_pTreeView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    m_pTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pTreeView->setUniformRowHeights(true);
    m_pTreeView->setRootIsDecorated(true);
    connect(m_pTreeView, SIGNAL(switchAll(bool)), SLOT(onSwitchAll(bool)));


    m_pMainSplitter = new QSplitter;
    m_pMainSplitter->setOrientation(Qt::Vertical);
    {
        m_pMainSplitter->addWidget(m_pTreeView);
        m_pMainSplitter->addWidget(m_pptObjectProps);
    }
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pTreeView->cmdWidget());
        pMainLayout->addWidget(m_pMainSplitter);
    }
    setLayout(pMainLayout);
}


void PlaneTreeView::fillModelView()
{
    m_pModel->removeRows(0, m_pModel->rowCount());

    ObjectTreeItem *pRootItem = m_pModel->rootItem();

    for(int iPlane=0; iPlane<Objects3d::nPlanes(); iPlane++)
    {
        Plane const *pPlane = Objects3d::planeAt(iPlane);
        if(!pPlane) continue;

        ObjectTreeItem *pPlaneItem = m_pModel->appendRow(pRootItem, pPlane->name(), pPlane->theStyle(), planeState(pPlane));

        fillWPolars(pPlaneItem, pPlane);
    }
}


void PlaneTreeView::updatePlane(const Plane *pPlane)
{
    if(!pPlane) return;

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        QModelIndex planeindex = m_pModel->index(ir, 0);
        if(pPlaneItem->name().toStdString().compare(pPlane->name())==0)
        {
            ObjectTreeItem *pItem = m_pModel->itemFromIndex(planeindex);
            if(!pItem) continue;
            m_pModel->blockSignals(true);
            m_pModel->removeRows(0, pItem->rowCount(), planeindex);
            m_pModel->blockSignals(false);

            fillWPolars(pPlaneItem, pPlane);
            break;
        }
    }
}


void PlaneTreeView::fillWPolars(ObjectTreeItem *pPlaneItem, const Plane *pPlane)
{
    if(!pPlane || !pPlaneItem) return;

    for(int iPolar=0; iPolar<Objects3d::nPolars(); iPolar++)
    {
        PlanePolar *pWPolar = Objects3d::wPolarAt(iPolar);
        if(!pWPolar) continue;
        if(pWPolar && pWPolar->planeName().compare(pPlane->name())==0)
        {
            LineStyle ls(pWPolar->theStyle());
            ls.m_bIsEnabled = true;
            m_pModel->appendRow(pPlaneItem, pWPolar->name(), ls, wPolarState(pWPolar));

            addPOpps(pWPolar);
        }
    }
}


void PlaneTreeView::updatePOpps()
{
    for(int iPolar=0; iPolar<Objects3d::nPolars(); iPolar++)
    {
        PlanePolar const*pWPolar = Objects3d::wPolarAt(iPolar);
        addPOpps(pWPolar);
    }
}


void PlaneTreeView::addPOpps(PlanePolar const *pWPolar)
{
    if(!pWPolar) pWPolar = s_pXPlane->curWPolar();
    if(!pWPolar) return;

    bool bAdded(false);

    //find this polar's plane parent
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        // find the polar's parent Plane
        if(pPlaneItem->name().toStdString().compare(pWPolar->planeName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pWPolarItem = pPlaneItem->child(jr);
                if(pWPolarItem->name().toStdString().compare(pWPolar->name())==0)
                {
                    QModelIndex polarindex = m_pModel->index(jr, 0, pPlaneItem);
                    if(pWPolarItem->rowCount())
                        m_pModel->removeRows(0, pWPolarItem->rowCount(), polarindex);

                    for(int iOpp=0; iOpp<Objects3d::nPOpps(); iOpp++)
                    {
                        PlaneOpp const *pPOpp = Objects3d::POppAt(iOpp);
                        if(pPOpp->planeName().compare(pWPolar->planeName())==0 && pPOpp->polarName().compare(pWPolar->name())==0)
                        {
                            QString strange = QString::fromStdString(pPOpp->name());
                            if(s_pXPlane->isPOppView())
                            {
                                LineStyle ls(pPOpp->theStyle());
                                ls.m_bIsEnabled = true;
                                m_pModel->appendRow(pWPolarItem, strange, ls, pPOpp->isVisible() ? Qt::Checked : Qt::Unchecked);
                            }
                            else
                            {
                                LineStyle ls(pPOpp->theStyle());
                                ls.m_bIsEnabled = false;
                                m_pModel->appendRow(pWPolarItem, strange, ls, Qt::PartiallyChecked);
                            }
                        }
                    }
                    bAdded = true;
                    break;
                }
            }
        }
        if(bAdded) break;
    }

    setOverallCheckStatus();
}


void PlaneTreeView::onCurrentRowChanged(QModelIndex currentfilteredidx)
{
    setObjectFromIndex(currentfilteredidx);
    s_pXPlane->updateView();
}


void PlaneTreeView::onItemClicked(const QModelIndex &index)
{
    PlaneOpp *pPOpp   = s_pXPlane->m_pCurPOpp;
    PlanePolar   *pWPolar = s_pXPlane->m_pCurWPolar;
    Plane    *pPlane  = s_pXPlane->m_pCurPlane;

    if(index.column()==1)
    {
        ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);

        if(pPOpp)
        {
            if(s_pXPlane->isPOppView())
            {
                LineStyle ls(pPOpp->theStyle());
                LineMenu *pLineMenu = new LineMenu(nullptr);
                pLineMenu->initMenu(ls);
                pLineMenu->exec(QCursor::pos());
                ls = pLineMenu->theStyle();
                pPOpp->setLineStipple(ls.m_Stipple);
                pPOpp->setLineWidth(ls.m_Width);
                pPOpp->setLineColor(ls.m_Color);
                pPOpp->setPointStyle(ls.m_Symbol);
                pItem->setTheStyle(ls);
                s_pXPlane->resetCurves();
                emit s_pXPlane->projectModified();
            }
        }
        else if(pWPolar)
        {
            if(s_pXPlane->isPOppView() || s_pXPlane->isPolarView() || s_pXPlane->isStabilityView())
            {
                LineStyle ls(pWPolar->theStyle());
                LineMenu *pLineMenu = new LineMenu(nullptr);
                pLineMenu->initMenu(ls);
                pLineMenu->exec(QCursor::pos());
                ls = pLineMenu->theStyle();

                Objects3d::setWPolarStyle(pWPolar, ls, pLineMenu->styleChanged(), pLineMenu->widthChanged(), pLineMenu->colorChanged(), pLineMenu->pointsChanged(), xfl::darkFactor());
                setCurveParams();

                if(pItem) pItem->setTheStyle(ls);
                s_pXPlane->resetCurves();
                emit s_pXPlane->projectModified();
            }
        }
        else if(pPlane)
        {
            LineStyle ls(pPlane->theStyle());
            LineMenu *pLineMenu = new LineMenu(nullptr);
            pLineMenu->initMenu(ls);
            pLineMenu->exec(QCursor::pos());
            ls = pLineMenu->theStyle();

            Objects3d::setPlaneStyle(pPlane, ls, pLineMenu->styleChanged(), pLineMenu->widthChanged(), pLineMenu->colorChanged(), pLineMenu->pointsChanged(),xfl::darkFactor());
            setCurveParams();

            if(pItem) pItem->setTheStyle(ls);
            s_pXPlane->resetCurves();
            emit s_pXPlane->projectModified();

            s_pXPlane->resetCurves();
        }
    }
    else if (index.column()==2)
    {
        ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);
        if(pPOpp)
        {
            if(s_pXPlane->isPOppView())
            {
                if(pItem)
                {
                    pPOpp->setVisible(!pPOpp->isVisible());
                    setCurveParams();
                    s_pXPlane->resetCurves();
                    emit s_pXPlane->projectModified();
                }
            }
        }
        else if(pWPolar)
        {
//            if(s_pXPlane->isPOppView() || s_pXPlane->isPolarView())
            {
                if(pItem)
                {
                    Qt::CheckState state = wPolarState(pWPolar);
                    if(state==Qt::PartiallyChecked || state==Qt::Unchecked)
                        Objects3d::setWPolarVisible(pWPolar, true);
                    else
                        Objects3d::setWPolarVisible(pWPolar, false);

                    setCurveParams();
                    s_pXPlane->resetCurves();
                    emit s_pXPlane->projectModified();
                }
            }
        }
        else if(pPlane)
        {
            if(pItem)
            {
                Qt::CheckState state = planeState(pPlane);
                if(state==Qt::PartiallyChecked || state==Qt::Unchecked)
                    Objects3d::setPlaneVisible(pPlane, true,  s_pXPlane->isStabPolarView());
                else if(state==Qt::Checked)
                    Objects3d::setPlaneVisible(pPlane, false, s_pXPlane->isStabPolarView());

                setCurveParams();
                s_pXPlane->resetCurves();
                emit s_pXPlane->projectModified();
            }
        }
        setOverallCheckStatus();
    }

    m_pModel->updateData();
    s_pXPlane->m_pAnalysisControls->setAnalysisRange();
    s_pXPlane->updateView();

    update();
}


void PlaneTreeView::onItemDoubleClicked(const QModelIndex &index)
{
    setObjectFromIndex(index);

    s_pXPlane->m_pAnalysisControls->setAnalysisRange();
    s_pXPlane->updateView();
    if(index.column()==0)
    {
        if(m_Selection==PlaneTreeView::PLANE)
        {
            s_pXPlane->onEditCurPlane();
        }
        else if(m_Selection==PlaneTreeView::WPOLAR)
        {
            s_pXPlane->onEditCurWPolar();
        }
    }
}


void PlaneTreeView::setObjectFromIndex(QModelIndex index)
{
    ObjectTreeItem *pSelectedItem = nullptr;

    if(index.column()==0)
    {
        pSelectedItem = m_pModel->itemFromIndex(index);
    }
    else if(index.column()>=1)
    {
        QModelIndex ind = index.siblingAtColumn(0);
        pSelectedItem = m_pModel->itemFromIndex(ind);
    }

    if(!pSelectedItem) return;

    if(pSelectedItem->level()==1)
    {
        s_pXPlane->setPlane(pSelectedItem->name());
        s_pXPlane->m_pCurWPolar = nullptr;
        s_pXPlane->m_pCurPOpp = nullptr;
        m_Selection = PlaneTreeView::PLANE;
    }
    else if(pSelectedItem->level()==2)
    {
        ObjectTreeItem *pPlaneItem = pSelectedItem->parentItem();
        Plane *pPlane  = Objects3d::plane(pPlaneItem->name().toStdString());
        PlanePolar *pWPolar = Objects3d::wPolar(pPlane, pSelectedItem->name().toStdString());
        s_pXPlane->setPlane(pPlane);
        s_pXPlane->setWPolar(pWPolar);
        s_pXPlane->m_pCurPOpp = nullptr;

        m_Selection = PlaneTreeView::WPOLAR;
    }
    else if(pSelectedItem->level()==3)
    {
        ObjectTreeItem *pWPolarItem = pSelectedItem->parentItem();
        ObjectTreeItem *pPlaneItem = pWPolarItem->parentItem();
        Plane *pPlane  = Objects3d::plane(pPlaneItem->name().toStdString());
        PlanePolar *pWPolar = Objects3d::wPolar(pPlane, pWPolarItem->name().toStdString());

        PlaneOpp *pPOpp   = Objects3d::planeOpp(pPlane, pWPolar, pSelectedItem->name().toStdString());

        m_Selection = PlaneTreeView::PLANEOPP;

        if(pPlane!=s_pXPlane->m_pCurPlane)
        {
            s_pXPlane->setPlane(pPlane);
            s_pXPlane->setWPolar(pWPolar);
        }
        else if(pWPolar != s_pXPlane->m_pCurWPolar) s_pXPlane->setWPolar(pWPolar);
        if(pPOpp)
        {
            s_pXPlane->setPlaneOpp(pPOpp);
            s_pXPlane->resetCurves();
        }
    }
    else m_Selection = PlaneTreeView::NOOBJECT;

    s_pXPlane->setControls();
    setObjectProperties();
}


void PlaneTreeView::insertWPolar(PlanePolar const *pWPolar)
{
    if(!pWPolar) pWPolar = s_pXPlane->curWPolar();
    if(!pWPolar) return;

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);

        // find the polar's parent Plane
        if(pPlaneItem->name().toStdString().compare(pWPolar->planeName())==0)
        {
            ObjectTreeItem *pNewPolarItem = nullptr;
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pOldPolarItem = pPlaneItem->child(jr);
                if(pOldPolarItem->name().toStdString().compare(pWPolar->name())==0)
                {
                    pNewPolarItem = pOldPolarItem;
                }
                else if(pOldPolarItem->name().toStdString().compare(pWPolar->name())>0)
                {
                    //insert before
//                    pNewPolarItem = pPlaneItem->insertRow(jr, pWPolar->name(), pWPolar->theStyle(), wPolarState(pWPolar));
                    pNewPolarItem = m_pModel->insertRow(pPlaneItem, jr, pWPolar->name(), pWPolar->theStyle(), wPolarState(pWPolar));
                }
                if(pNewPolarItem) break;
            }
            if(!pNewPolarItem)
            {
                //append
                pNewPolarItem = m_pModel->appendRow(pPlaneItem, pWPolar->name(), pWPolar->theStyle(), wPolarState(pWPolar));
            }

            if(pNewPolarItem)
            {
                addPOpps(pWPolar);

                // set the curve data
                LineStyle ls(pWPolar->theStyle());
                ls.m_bIsEnabled = (s_pXPlane->isPolarView() || s_pXPlane->isStabilityView());
                pNewPolarItem->setTheStyle(ls);
                pNewPolarItem->setCheckState(wPolarState(pWPolar));
            }
            return;
        }
    }

    m_pModel->updateData();
    setOverallCheckStatus();
}


void PlaneTreeView::insertPlane(Plane* pPlane)
{
    if(!pPlane) pPlane = s_pXPlane->curPlane();
    if(!pPlane) return;

    bool bInserted = false;
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pItem = m_pModel->item(ir);
        // insert alphabetically
        if(pItem->name().toStdString().compare(pPlane->name())==0)
        {
            //A Plane of that name already exists
            return;
        }
        else if(pItem->name().toStdString().compare(pPlane->name())>0)
        {
            //insert before
            m_pModel->insertRow(m_pModel->rootItem(), ir, pPlane->name(), pPlane->theStyle(), planeState(pPlane));
//            m_pModel->rootItem()->insertRow(ir, pPlane->planeName(), pPlane->theStyle(), planeState(pPlane));
            bInserted = true;
            break;
        }
    }
    if(!bInserted)
    {
        //not inserted, append
        m_pModel->appendRow(m_pModel->rootItem(), pPlane->name(), pPlane->theStyle(), planeState(pPlane));
    }

    for(int iwp=0; iwp<Objects3d::nPolars(); iwp++)
    {
        PlanePolar const *pWPolar = Objects3d::wPolarAt(iwp);
        if(pWPolar->planeName().compare(pPlane->name())==0)
        {
            insertWPolar(pWPolar);
        }
    }

    m_pModel->updateData();
    setOverallCheckStatus();
}


void PlaneTreeView::selectPlane(Plane *pPlane)
{
    if(!pPlane) pPlane = s_pXPlane->m_pCurPlane;
    if(!pPlane) return;

//    m_pStruct->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        if(pPlaneItem->name().toStdString().compare(pPlane->name())==0)
        {
            m_Selection = PlaneTreeView::PLANE;

            QModelIndex ind = m_pModel->index(ir, 0, m_pModel->rootItem());
            if(ind.isValid())
            {
                if(ind.isValid())
                {
                    m_pTreeView->setCurrentIndex(ind);
                    m_pTreeView->selectionModel()->select(ind, QItemSelectionModel::Rows);
                    m_pTreeView->scrollTo(ind);
                }
            }

            break;
        }
    }
//    m_pStruct->selectionModel()->blockSignals(false);
}


void PlaneTreeView::selectWPolar(PlanePolar *pWPolar, bool bSelectPOpp)
{
    if(!pWPolar) pWPolar = s_pXPlane->curWPolar();
    if(!pWPolar) return;

    //    m_pStruct->selectionModel()->blockSignals(true);

    bool bSelected=false;

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);

        QModelIndex planeindex = m_pModel->index(ir, 0);
        Q_ASSERT(planeindex.isValid());
        // find the polar's parent Plane
        if(pPlaneItem->name().toStdString().compare(pWPolar->planeName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pPlaneItem->child(jr);
                if(pPolarItem->name().toStdString().compare(pWPolar->name())==0)
                {
                    m_Selection = PlaneTreeView::WPOLAR;

                    QModelIndex polarindex = m_pModel->index(jr, 0, planeindex);
                    if(polarindex.isValid())
                    {
                        m_pTreeView->setCurrentIndex(polarindex);
                        m_pTreeView->selectionModel()->select(polarindex, QItemSelectionModel::Rows);
                        m_pTreeView->scrollTo(polarindex);

                        bSelected = true;
                    }
                    break;
                }
            }
        }
        if(bSelected) break;
    }
    //    m_pStruct->selectionModel()->blockSignals(false);
    if(bSelectPOpp) selectPlaneOpp();
    setObjectProperties();
    update();
}


void PlaneTreeView::selectPlaneOpp(PlaneOpp *pPOpp)
{
    if(!pPOpp) pPOpp = s_pXPlane->m_pCurPOpp;
    if(!pPOpp) return;

    bool bSelected = false;

    //    m_pStruct->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        // find the polar's parent Plane
        if(pPlaneItem->name().toStdString().compare(pPOpp->planeName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                //                const QModelIndex &oldWPolarChild = pPlaneItem->index().child(jr, 0);
                //                PlaneTreeItem *pPolarItem = m_pModel->itemFromIndex(oldWPolarChild);
                ObjectTreeItem *pPolarItem = pPlaneItem->child(jr);

                if(pPolarItem->name().toStdString().compare(pPOpp->polarName())==0)
                {
                    //find the POpp item
                    for(int jr=0; jr<pPolarItem->rowCount(); jr++)
                    {
                        //                        const QModelIndex &poppChild = pPolarItem->index().child(jr, 0);
                        //                        PlaneTreeItem *poppItem = m_pModel->itemFromIndex(poppChild);
                        ObjectTreeItem *pPOppItem = pPolarItem->child(jr);
                        QModelIndex poppChild = m_pModel->index(jr,0, pPolarItem);
                        QString strange = QString::fromStdString(pPOpp->name());

                        if(strange.compare(pPOppItem->name())==0)
                        {
                            bSelected = true;
                            m_Selection = PlaneTreeView::PLANEOPP;
                            m_pTreeView->setCurrentIndex(poppChild);
                            m_pTreeView->scrollTo(poppChild);
                            break;
                        }
/*                        bool bOK=false;
                        QString strange = pPOppItem->name().trimmed();
                        double val = locale().toDouble(strange, &bOK);

                        if(bOK)
                        {
                            switch(pPOpp->polarType())
                            {
                            case xfl::T1POLAR:
                            case xfl::T2POLAR:
                            case xfl::T3POLAR:
                            {
                                bSelected = fabs(val-pPOpp->alpha())<0.0005;
                                break;
                            }
                            case xfl::T5POLAR:
                            {
                                bSelected = fabs(val-pPOpp->beta())<0.0005;
                                break;
                            }
                            case xfl::T7POLAR:
                            case xfl::T6POLAR:
                            {
                                bSelected = fabs(val-pPOpp->ctrl())<0.0005;
                                break;
                            }
                            default:
                                bSelected = false; //never reached
                                break;
                            }
                            if(bSelected)
                            {
                                m_Selection = xfl::PLANEOPP;
                                m_pTreeView->setCurrentIndex(poppChild);
                                m_pTreeView->scrollTo(poppChild);
                                break;
                            }
                        }*/
                    }
                }
                if(bSelected) break;
            }
        }
        if(bSelected) break;
    }
    setObjectProperties();

    //    m_pStruct->selectionModel()->blockSignals(false);
}


/**
 * Removes the plane defined by the pointer and returns the name
 * of the previous plane in the list, or of the next plane if none
 * @param pPlane a pointer to the plane object to be removed
 * @return the name of the next plane to select
 */
QString PlaneTreeView::removePlane(Plane *pPlane)
{
    if(!pPlane) return "";
    return removePlane(QString::fromStdString(pPlane->name()));
}


/**
 * Removes the plane defined by the name and returns the name
 * of the previous plane in the list, or of the next plane if none
 * @param planeName the name of the plane object to be removed
 * @return the name of the next plane to select
 */
QString PlaneTreeView::removePlane(QString const &planeName)
{
    if(!planeName.length()) return QString();

    m_pTreeView->selectionModel()->blockSignals(true);

    int irow = 0;
    for(irow=0; irow<m_pModel->rowCount(); irow++)
    {
        ObjectTreeItem *pItem = m_pModel->item(irow);
        // scan
        if(pItem && pItem->level()==1 && pItem->name().compare(planeName)==0)
        {
            // plane found
//            QModelIndex rootindex = m_pModel->index(0,0, QModelIndex());
//            ObjectTreeItem *proot = m_pModel->itemFromIndex(rootindex);
            m_pModel->removeRow(irow, QModelIndex());
            break;
        }
    }

    m_pModel->updateData();
    setOverallCheckStatus();
    m_pTreeView->selectionModel()->blockSignals(false);

    if(irow+1<Objects3d::nPlanes())
        return QString::fromStdString(Objects3d::planeAt(irow+1)->name());
    else if(irow-1>=0)
        return QString::fromStdString(Objects3d::planeAt(irow-1)->name());


    m_pModel->updateData();
    return QString("");
}


QString PlaneTreeView::removeWPolar(PlanePolar *pWPolar)
{
    if(!pWPolar) return "";

    m_pTreeView->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        QModelIndex planeindex = m_pModel->index(ir, 0);
        // find the polar's parent Plane
        if(pPlaneItem->name().toStdString().compare(pWPolar->planeName())==0)
        {
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pOldPolarItem = pPlaneItem->child(jr);

                if(pOldPolarItem)
                {
                    if(pOldPolarItem->name().toStdString().compare(pWPolar->name())==0)
                    {
                        m_pModel->removeRow(jr, planeindex);

                        m_pTreeView->selectionModel()->blockSignals(false);

                        // find the previous item, or the next one if this polar is the first
                        if(pPlaneItem->rowCount())
                        {
                            jr =std::min(jr, pPlaneItem->rowCount()-1);
                            return pPlaneItem->child(jr)->name();
                        }
                        return QString();
                    }
                }
            }
        }
    }

    m_pModel->updateData();
    setOverallCheckStatus();
    m_pTreeView->selectionModel()->blockSignals(false);
    return QString();
}


void PlaneTreeView::removeWPolars(Plane const*pPlane)
{
    if(!pPlane) return;

    m_pTreeView->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        // find the polar's parent Plane
        if(pPlaneItem->name().toStdString().compare(pPlane->name())==0)
        {
            QModelIndex planeindex = m_pModel->index(ir, 0);
            Q_ASSERT(planeindex.isValid());

            m_pModel->removeRows(0, pPlaneItem->rowCount(), planeindex);
        }
    }

    m_pModel->updateData();
    setOverallCheckStatus();
    m_pTreeView->selectionModel()->blockSignals(false);
}


void PlaneTreeView::removeWPolarPOpps(PlanePolar const*pWPolar)
{
    if(!pWPolar) return;

    m_pTreeView->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        // find the polar's parent Plane item
        if(pPlaneItem->name().toStdString().compare(pWPolar->planeName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pPlaneItem->child(jr);
                if(pPolarItem->name().toStdString().compare(pWPolar->name())==0)
                {
                    QModelIndex polarindex = m_pModel->index(jr, 0, pPlaneItem);
                    m_pModel->removeRows(0, pPolarItem->rowCount(), polarindex);
                    break;
                }
            }
        }
    }

    m_pModel->updateData();
    setOverallCheckStatus();
    m_pTreeView->selectionModel()->blockSignals(false);
}


void PlaneTreeView::removePlaneOpp(PlaneOpp *pPOpp)
{
    if(!pPOpp) return;

//    m_pTreeView->selectionModel()->blockSignals(true);
//    m_pModel->blockSignals(true);

    bool bRemoved(false);

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        Plane *m_pPlane = Objects3d::plane(pPlaneItem->name().toStdString());

        // find the polar's parent Plane
        if(pPlaneItem->name().toStdString().compare(pPOpp->planeName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pPlaneItem->child(jr);
                if(pPolarItem->name().toStdString().compare(pPOpp->polarName())==0)
                {
                    QModelIndex polarindex = m_pModel->index(jr, 0, pPlaneItem);

                    PlanePolar *pWPolar = Objects3d::wPolar(m_pPlane, pPolarItem->name().toStdString());
                    if(!pWPolar) continue;

                    //find the POpp item
                    for(int jr=0; jr<pPolarItem->rowCount(); jr++)
                    {
                        ObjectTreeItem *poppItem = pPolarItem->child(jr);
                        if(poppItem->name().toStdString().compare(pPOpp->name())==0)
                        {
                            m_pModel->removeRow(jr,polarindex);
                            bRemoved = true;
                            break;
                        }
                    }
                }
                if(bRemoved) break;
            }
        }
    }

//    m_pModel->blockSignals(false);
//    m_pTreeView->selectionModel()->blockSignals(false);

//    m_pTreeView->update();
//    m_pModel->updateData();
    setOverallCheckStatus();
}


void PlaneTreeView::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QModelIndex index = m_pTreeView->currentIndex();
    setObjectFromIndex(index);

    ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);

    PlaneOpp *m_pPOpp = s_pXPlane->m_pCurPOpp;
    PlanePolar *m_pWPolar = s_pXPlane->m_pCurWPolar;
    Plane* m_pPlane = s_pXPlane->m_pCurPlane;

    QString strong;

    if(!pItem) return;

    if(pItem->level()==1)
    {
        // no parent, a plane is selected
        m_pPlane = Objects3d::plane(pItem->name().toStdString());
        if(m_pPlane) strong = QString::fromStdString(m_pPlane->name());
        else         strong.clear();
    }
    else if(pItem->level()==2)
    {
        // a WPolar is selected;
        ObjectTreeItem *pParent = pItem->parentItem();
        m_pPlane = Objects3d::plane(pParent->name().toStdString());
        m_pWPolar = Objects3d::wPolar(m_pPlane, pItem->name().toStdString());
        if(m_pWPolar) strong = QString::fromStdString(m_pWPolar->name());
        else          strong.clear();
    }
    else if(pItem->level()==3)
    {
        // a POpp is selected;
        ObjectTreeItem *pParent = pItem->parentItem();
        ObjectTreeItem *pParent2 = pParent->parentItem();
        m_pPlane  = Objects3d::plane(pParent2->name().toStdString());
        m_pWPolar = Objects3d::wPolar(m_pPlane, pParent->name().toStdString());
        m_pPOpp   = Objects3d::planeOpp(m_pPlane, m_pWPolar, pItem->name().toStdString());
        if(m_pPOpp) strong = pItem->name();
        else        strong.clear();
    }

    if     (m_Selection==PlaneTreeView::PLANEOPP && m_pPOpp)
        s_pXPlane->m_pMenus->m_pCurPOppCtxMenu->exec(pEvent->globalPos());
    else if(m_Selection==PlaneTreeView::WPOLAR && m_pWPolar)
        s_pXPlane->m_pMenus->m_pCurWPlrCtxMenu->exec(pEvent->globalPos());
    else if(m_Selection==PlaneTreeView::PLANE && m_pPlane)
        s_pXPlane->m_pMenus->m_pCurrentPlaneCtxMenu->exec(pEvent->globalPos());

    pEvent->accept();
}


void PlaneTreeView::keyPressEvent(QKeyEvent *pEvent)
{
    PlaneOpp *m_pPOpp = s_pXPlane->m_pCurPOpp;
    PlanePolar *m_pWPolar = s_pXPlane->m_pCurWPolar;
    Plane* m_pPlane = s_pXPlane->m_pCurPlane;

    switch (pEvent->key())
    {
        case Qt::Key_Delete:
        {
            if(m_Selection==PlaneTreeView::PLANEOPP && m_pPOpp)
                s_pXPlane->onDeleteCurPOpp();
            else if(m_Selection==PlaneTreeView::WPOLAR && m_pWPolar)
                s_pXPlane->onDeleteCurWPolar();
            else if(m_Selection==PlaneTreeView::PLANE && m_pPlane)
                s_pXPlane->onDeleteCurPlane();

            pEvent->accept();
            return;
        }
        default:
            s_pXPlane->keyPressEvent(pEvent);
    }
}


void PlaneTreeView::loadSettings(QSettings &settings)
{
    settings.beginGroup("PlaneTreeView");
    {
        s_SplitterSizes = settings.value("HSplitterSizes").toByteArray();
    }
    settings.endGroup();
}


void PlaneTreeView::saveSettings(QSettings &settings)
{
    settings.beginGroup("PlaneTreeView");
    {
        settings.setValue("HSplitterSizes", s_SplitterSizes);
    }
    settings.endGroup();
}


/**
 * @brief PlaneTreeView::selectCurrentItem
 * Selects the active object, a PlaneOpp, a WPolar or a Plane
 */
void PlaneTreeView::selectCurrentObject()
{
    //    qDebug("selectCurrentObject");
    if(s_pXPlane->isPOppView() || s_pXPlane->m_eView==XPlane::CPVIEW)
    {
        if     (s_pXPlane->m_pCurPOpp)   selectPlaneOpp(s_pXPlane->m_pCurPOpp);
        else if(s_pXPlane->m_pCurWPolar) selectWPolar(s_pXPlane->m_pCurWPolar, false);
        else if(s_pXPlane->m_pCurPlane)  selectPlane(s_pXPlane->m_pCurPlane);
    }
    else if(s_pXPlane->m_eView==XPlane::WPOLARVIEW || s_pXPlane->m_eView==XPlane::STABPOLARVIEW)
    {
        if     (s_pXPlane->m_pCurWPolar) selectWPolar(s_pXPlane->m_pCurWPolar, false);
        else if(s_pXPlane->m_pCurPlane)  selectPlane(s_pXPlane->m_pCurPlane);
    }
    else if (s_pXPlane->is3dView() || s_pXPlane->m_eView==XPlane::OTHERVIEW)
    {
        selectPlane(s_pXPlane->m_pCurPlane);
    }
}


void PlaneTreeView::selectObjects()
{
    if     (s_pXPlane->curPOpp())   selectPlaneOpp();
    else if(s_pXPlane->curWPolar()) selectWPolar(s_pXPlane->curWPolar(), false);
    else                            selectPlane(s_pXPlane->curPlane());
}


/** update the line properties for each polar and popp item in the treeview */
void PlaneTreeView::setCurveParams()
{
    ObjectTreeItem *pRootItem = m_pModel->rootItem();
    if(!pRootItem)
    {
        qDebug()<<"No root item=";
        return;
    }

    for(int i0=0; i0<pRootItem->rowCount(); i0++)
    {
        QModelIndex planeindex = m_pModel->index(i0,0);
        ObjectTreeItem *pPlaneItem = m_pModel->itemFromIndex(planeindex);
        Plane const *pPlane = nullptr;
        if(!pPlaneItem) pPlane = nullptr;
        else            pPlane = Objects3d::plane(pPlaneItem->name().toStdString());
        if(!pPlane) continue;
        {
            pPlaneItem->setCheckState(planeState(pPlane));

            for(int i1=0; i1< pPlaneItem->rowCount(); i1++)
            {
                if(pPlaneItem->child(i1))
                {
                    ObjectTreeItem *pWPolarItem = pPlaneItem->child(i1);
                    PlanePolar const *pWPolar = nullptr;
                    if(pWPolarItem) pWPolar = Objects3d::wPolar(pPlane, pWPolarItem->name().toStdString());
                    else            pWPolar = nullptr;

                    if(pWPolar)
                    {
                        LineStyle ls(pWPolar->theStyle());
                        ls.m_bIsEnabled = !s_pXPlane->is3dView();
                        pWPolarItem->setTheStyle(ls);
                        if(s_pXPlane->isPolarView() || s_pXPlane->isStabPolarView())
                        {
                            bool bCheck = pWPolar->isVisible();
                            bCheck = bCheck && (s_pXPlane->isPolarView() || s_pXPlane->isStabilityView() || s_pXPlane->isPOppView()) ;
                            pWPolarItem->setCheckState(bCheck ? Qt::Checked : Qt::Unchecked);
                        }
                        else
                        {
                            pWPolarItem->setCheckState(wPolarState(pWPolar));
                        }

                        for(int i2=0; i2<pWPolarItem->rowCount(); i2++)
                        {
                            ObjectTreeItem *pOppItem = pWPolarItem->child(i2);
                            PlaneOpp *pPOpp = nullptr;

                            if(pOppItem)
                            {
                                pPOpp = Objects3d::planeOpp(pPlane, pWPolar, pOppItem->name().toStdString());
                            }
                            else pPOpp = nullptr;
                            if(pPOpp)
                            {
                                LineStyle ls(pPOpp->theStyle());
                                ls.m_bIsEnabled = s_pXPlane->isPOppView();
                                pOppItem->setTheStyle(ls);

                                bool bCheck = pPOpp->isVisible()&& s_pXPlane->isPOppView();
                                pOppItem->setCheckState(bCheck ? Qt::Checked : Qt::Unchecked);
                            }
                        }
                    }
                }
            }
        }
    }
    m_pModel->updateData();
}


Qt::CheckState PlaneTreeView::planeState(const Plane *pPlane) const
{
    bool bAll = true;
    bool bNone = true;
    if(s_pXPlane->isPolarView() || s_pXPlane->isStabPolarView())
    {
        for(int iplr=0; iplr<Objects3d::nPolars(); iplr++)
        {
            PlanePolar const*pWPolar = Objects3d::wPolarAt(iplr);
            if(pPlane->hasWPolar(pWPolar))
            {
                bAll = bAll && pWPolar->isVisible();
                bNone = bNone && !pWPolar->isVisible();
            }
        }
    }
    else if(s_pXPlane->isStabPolarView())
    {
        for(int iplr=0; iplr<Objects3d::nPolars(); iplr++)
        {
            PlanePolar const*pWPolar = Objects3d::wPolarAt(iplr);
            if(pWPolar->isStabilityPolar() && pPlane->hasWPolar(pWPolar))
            {
                bAll = bAll && pWPolar->isVisible();
                bNone = bNone && !pWPolar->isVisible();
            }
        }
    }
    else
    {
        for(int iopp=0; iopp<Objects3d::nPOpps(); iopp++)
        {
            PlaneOpp const*pOpp = Objects3d::POppAt(iopp);
            if(pPlane->hasPOpp(pOpp))
            {
                bAll = bAll && pOpp->isVisible();
                bNone = bNone && !pOpp->isVisible();
            }
        }
    }
    if(bNone)      return Qt::Unchecked;
    else if(bAll)  return Qt::Checked;
    else           return Qt::PartiallyChecked;
}


Qt::CheckState PlaneTreeView::wPolarState(PlanePolar const*pWPolar) const
{
    if(s_pXPlane->isPOppView())
    {
        bool bAll = true;
        bool bNone = true;
        for(int iopp=0; iopp<Objects3d::nPOpps(); iopp++)
        {
            PlaneOpp const *pOpp = Objects3d::POppAt(iopp);
            if(pWPolar->hasPOpp(pOpp))
            {
                bAll = bAll && pOpp->isVisible();
                bNone = bNone && !pOpp->isVisible();
            }
        }
        if     (bNone) return Qt::Unchecked;
        else if(bAll)  return Qt::Checked;
        else           return Qt::PartiallyChecked;
    }
    else if(s_pXPlane->isPolarView())
    {
        return pWPolar->isVisible() ? Qt::Checked : Qt::Unchecked;
    }
    else if(s_pXPlane->isStabPolarView())
    {
        return pWPolar->isVisible() ? Qt::Checked : Qt::Unchecked;
    }
    return Qt::Unchecked;
}


void PlaneTreeView::setOverallCheckStatus()
{
    if(s_pXPlane->isPOppView())
    {
        m_pTreeView->enableSelectBox(true);

        bool bAllChecked   = true;
        bool bAllUnchecked = true;
        for(int io=0; io<Objects3d::nPOpps(); io++)
        {
            PlaneOpp *const pPOpp=Objects3d::POppAt(io);
            if(pPOpp->isVisible()) bAllUnchecked = false;
            else                   bAllChecked   = false;
        }

        if     (bAllChecked)   m_pTreeView->setOverallCheckedState(Qt::Checked);
        else if(bAllUnchecked) m_pTreeView->setOverallCheckedState(Qt::Unchecked);
        else                   m_pTreeView->setOverallCheckedState(Qt::PartiallyChecked);
    }
    else if(s_pXPlane->isPolarView() || s_pXPlane->isStabPolarView())
    {
        m_pTreeView->enableSelectBox(true);
        bool bAllChecked   = true;
        bool bAllUnchecked = true;
        for(int io=0; io<Objects3d::nPolars(); io++)
        {
            PlanePolar *const pWPolar = Objects3d::wPolarAt(io);
            if(pWPolar->isVisible()) bAllUnchecked = false;
            else                     bAllChecked   = false;
        }

        if     (bAllChecked)   m_pTreeView->setOverallCheckedState(Qt::Checked);
        else if(bAllUnchecked) m_pTreeView->setOverallCheckedState(Qt::Unchecked);
        else                   m_pTreeView->setOverallCheckedState(Qt::PartiallyChecked);
    }
    else
    {
        m_pTreeView->enableSelectBox(false);
        m_pTreeView->setOverallCheckedState(Qt::Unchecked);
    }
}


void PlaneTreeView::onSwitchAll(bool bChecked)
{
    if(s_pXPlane->isPOppView())
    {
        if(bChecked) s_pXPlane->onShowAllPOpps();
        else         s_pXPlane->onHideAllPOpps();
    }
    else if(s_pXPlane->isPolarView())
    {
        if(bChecked) s_pXPlane->onShowAllWPolars();
        else         s_pXPlane->onHideAllWPolars();
    }
    else if(s_pXPlane->isStabPolarView())
    {
        if(bChecked) s_pXPlane->onShowAllWPolars();
        else         s_pXPlane->onHideAllWPolars();
    }
    m_pModel->updateData();
}


void PlaneTreeView::onSetFilter()
{
    QString filter = m_pTreeView->filter();
    QStringList filters = filter.split(QRegularExpression("\\s+"));

    if(filters.size()==0)
    {
        for(int jp=0; jp<Objects3d::nPolars(); jp++)
        {
            Objects3d::wPolarAt(jp)->setVisible(true);
        }
    }
    else if(filters.size()==1)
    {
        for(int jp=0; jp<Objects3d::nPolars(); jp++)
        {
            PlanePolar *pWPolar = Objects3d::wPolarAt(jp);
            bool bVisible = QString::fromStdString(pWPolar->name()).contains(filter, Qt::CaseInsensitive);
            pWPolar->setVisible(bVisible);
        }

        for(int ip=0; ip<Objects3d::nPlanes(); ip++)
        {
            Plane const *pPlane = Objects3d::planeAt(ip);
            if(QString::fromStdString(pPlane->name()).contains(filter, Qt::CaseInsensitive))
            {
                for(int jp=0; jp<Objects3d::nPolars(); jp++)
                {
                    PlanePolar *pWPolar = Objects3d::wPolarAt(jp);
                    if(pWPolar->planeName()==pPlane->name())
                        pWPolar->setVisible(true);
                }
            }
        }
    }
    else
    {
        QString planefilter = filters.front();
        QString polarfilter = filters.at(1);
        for(int jp=0; jp<Objects3d::nPolars(); jp++)
        {
            PlanePolar *pWPolar = Objects3d::wPolarAt(jp);
            pWPolar->setVisible(false);
        }

        for(int ip=0; ip<Objects3d::nPlanes(); ip++)
        {
            Plane const *pPlane = Objects3d::planeAt(ip);
            if(QString::fromStdString(pPlane->name()).contains(planefilter, Qt::CaseInsensitive))
            {
                for(int jp=0; jp<Objects3d::nPolars(); jp++)
                {
                    PlanePolar *pWPolar = Objects3d::wPolarAt(jp);
                    if(pWPolar->planeName()==pPlane->name())
                    {
                        bool bVisible = QString::fromStdString(pWPolar->name()).contains(polarfilter, Qt::CaseInsensitive);
                        pWPolar->setVisible(bVisible);
                    }
                }
            }
        }
    }


    setCurveParams();
    setOverallCheckStatus();

    s_pXPlane->resetCurves();
    s_pXPlane->updateView();

    m_pTreeView->update();

    emit s_pXPlane->projectModified();

}



