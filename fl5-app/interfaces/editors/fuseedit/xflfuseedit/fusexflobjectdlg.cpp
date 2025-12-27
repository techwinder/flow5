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
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QPainter>
#include <QShowEvent>


#include <interfaces/editors/fuseedit/xflfuseedit/fusexflobjectdlg.h>



#include <api/fusexfl.h>
#include <api/planexfl.h>
#include <api/xmlfusereader.h>
#include <api/xmlfusewriter.h>
#include <core/enums_core.h>
#include <api/units.h>
#include <core/saveoptions.h>
#include <interfaces/editors/editobjectdelegate.h>
#include <interfaces/editors/fuseedit/bodyscaledlg.h>
#include <interfaces/editors/fuseedit/bodytransdlg.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fuseframewt.h>
#include <interfaces/editors/fuseedit/xflfuseedit/fuselinewt.h>
#include <interfaces/editors/inertia/partinertiadlg.h>
#include <interfaces/editors/objecttreeview.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dfuseview.h>


QByteArray FuseXflObjectDlg::m_HSplitterSizes;

FuseXflObjectDlg::FuseXflObjectDlg(QWidget *pParent) : FuseXflDlg(pParent)
{
    setWindowTitle("Body object explorer");

    m_pStruct = nullptr;
    m_pDelegate = nullptr;
    m_pModel = nullptr;

    m_bIsInertiaSelected = false;
    m_iActivePointMass = -1;

    m_pInsertBefore  = new QAction("Insert before", this);
    m_pInsertAfter   = new QAction("Insert after", this);
    m_pDeleteItem    = new QAction("Delete", this);

    m_pContextMenu = new QMenu("Section",this);
    m_pContextMenu->addAction(m_pInsertBefore);
    m_pContextMenu->addAction(m_pInsertAfter);
    m_pContextMenu->addAction(m_pDeleteItem);

    createActions();
    setupLayout();
    connectSignals();
}


/**
 * Overrides the base class showEvent method. Moves the window to its former location.
 * @param event the showEvent.
 */
void FuseXflObjectDlg::showEvent(QShowEvent *pEvent)
{
    FuseXflDlg::showEvent(pEvent);

    if(m_HSplitterSizes.length()>0)
        m_pHSplitter->restoreState(m_HSplitterSizes);

    onResizeColumns();
}


/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void FuseXflObjectDlg::hideEvent(QHideEvent *pEvent)
{
    FuseXflDlg::hideEvent(pEvent);
    m_HSplitterSizes  = m_pHSplitter->saveState();

    pEvent->accept();
}


void FuseXflObjectDlg::resizeEvent(QResizeEvent *pEvent)
{
    FuseXflDlg::resizeEvent(pEvent);
    onResizeColumns();
    pEvent->accept();
}


void FuseXflObjectDlg::onResizeColumns()
{
    QHeaderView *pHHeader = m_pStruct->header();
    pHHeader->setSectionResizeMode(3, QHeaderView::Stretch);

    double w = double(m_pStruct->width())/100.0;
    int wCols  = int(27*w);

    m_pStruct->setColumnWidth(0, wCols);
    m_pStruct->setColumnWidth(1, wCols);
    m_pStruct->setColumnWidth(2, wCols);
}


void FuseXflObjectDlg::contextMenuEvent(QContextMenuEvent *pEvent)
{
    // Display the context menu

    if(m_bIsInertiaSelected)
    {
        m_pInsertBefore->setText("Insert point mass before");
        m_pInsertAfter->setText("Insert point mass after");
        m_pDeleteItem->setText("Delete point mass");
        m_pInsertAfter->setEnabled(true);
        m_pInsertBefore->setEnabled(false);
        m_pDeleteItem->setEnabled(false);
    }
    else
    {
        m_pInsertBefore->setEnabled(true);
        m_pInsertAfter->setEnabled(true);
        m_pDeleteItem->setEnabled(true);

        if(m_pFuseXfl->activeFrameIndex()<0 && m_iActivePointMass<0 ) return;

        if(m_pFuseXfl->activeFrameIndex()>=0)
        {
            if(Frame::selectedIndex()>=0)
            {
                m_pInsertBefore->setText("Insert point before");
                m_pInsertAfter->setText("Insert point after");
                m_pDeleteItem->setText("Delete point");
            }
            else
            {
                m_pInsertBefore->setText("Insert frame before");
                m_pInsertAfter->setText("Insert frame after");
                m_pDeleteItem->setText("Delete frame");
            }
        }
        else if(m_iActivePointMass>=0)
        {
            m_pInsertBefore->setText("Insert point mass before");
            m_pInsertAfter->setText("Insert point mass after");
            m_pDeleteItem->setText("Delete point mass");
        }
    }

    if(m_pStruct->geometry().contains(pEvent->pos())) m_pContextMenu->exec(pEvent->globalPos());
}


void FuseXflObjectDlg::setupLayout()
{
    QStringList labels;
    labels << "Object" << "Field"<<"Value"<<"Unit";

    m_pStruct = new ObjectTreeView(this);

//    m_pStruct->setFont(DisplayOptions::treeFont());

    m_pStruct->header()->setSectionResizeMode(QHeaderView::Interactive);

    //    m_pPlaneStruct->header()->setDefaultSectionSize(239);
    m_pStruct->header()->setStretchLastSection(true);
    m_pStruct->header()->setDefaultAlignment(Qt::AlignCenter);

    m_pStruct->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_pStruct->setSelectionBehavior (QAbstractItemView::SelectRows);
    //    m_pStruct->setIndentation(31);
    m_pStruct->setWindowTitle("Objects");

    m_pModel = new QStandardItemModel(this);
    m_pModel->setColumnCount(4);
    m_pModel->clear();
    m_pModel->setHorizontalHeaderLabels(labels);
    m_pStruct->setModel(m_pModel);

    QFont font;
    QFontMetrics fm(font);
    m_pStruct->setColumnWidth(0, fm.averageCharWidth()*37);
    m_pStruct->setColumnWidth(1, fm.averageCharWidth()*29);
    m_pStruct->setColumnWidth(2, fm.averageCharWidth()*17);


    QItemSelectionModel *selectionModel = new QItemSelectionModel(m_pModel);
    m_pStruct->setSelectionModel(selectionModel);
    connect(selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));


    m_pDelegate = new EditObjectDelegate(this);
    m_pStruct->setItemDelegate(m_pDelegate);
    connect(m_pDelegate,  SIGNAL(closeEditor(QWidget*)), SLOT(onRedraw()));


    QSizePolicy szPolicyMinimumExpanding;
    szPolicyMinimumExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    szPolicyMinimumExpanding.setVerticalPolicy(QSizePolicy::MinimumExpanding);

    QSizePolicy szPolicyMaximum;
    szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
    szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);

    m_pHSplitter = new QSplitter(Qt::Horizontal, this);
    {
        m_pHSplitter->setChildrenCollapsible(false);
        QFrame *pLeftFrame = new QFrame;
        {
            QVBoxLayout *pLeftLayout = new QVBoxLayout;
            {
                pLeftLayout->addWidget(m_pMetaFrame);
                pLeftLayout->addWidget(m_pStruct);
                pLeftLayout->addWidget(m_pButtonBox);
                pLeftLayout->setStretchFactor(m_pMetaFrame,1);
                pLeftLayout->setStretchFactor(m_pStruct,20);
                pLeftLayout->setStretchFactor(m_pButtonBox,1);
            }
            pLeftFrame->setLayout(pLeftLayout);
        }

        m_pHSplitter->addWidget(pLeftFrame);
        m_pHSplitter->addWidget(m_pViewHSplitter);
    }

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        pMainLayout->addWidget(m_pHSplitter);
    }

    setLayout(pMainLayout);
}


void FuseXflObjectDlg::initDialog(Fuse *pFuse)
{
    FuseXflDlg::initDialog(pFuse);
    FuseXfl*pFuseXfl = dynamic_cast<FuseXfl*>(pFuse);

    m_pFuseLineView->setUnitFactor(Units::mtoUnit());
    m_pFrameView->setUnitFactor(Units::mtoUnit());
    setBody(pFuseXfl);
}


void FuseXflObjectDlg::setBody(FuseXfl *pFuseXfl)
{
    FuseXflDlg::setBody(pFuseXfl);

    fillFuseXflTreeView();
}


void FuseXflObjectDlg::keyPressEvent(QKeyEvent *pEvent)
{
    //    bool bShift = false;
    //    bool bCtrl  = false;
    //    if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
    //    if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus()) m_pButtonBox->setFocus();
            else accept();

            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }

        default:
            pEvent->ignore();
    }
}


void FuseXflObjectDlg::connectSignals()
{
    connectFuseXflSignals();

    connect(m_pInsertBefore,   SIGNAL(triggered()),             SLOT(onInsertBefore()));
    connect(m_pInsertAfter,    SIGNAL(triggered()),             SLOT(onInsertAfter()));
    connect(m_pDeleteItem,     SIGNAL(triggered()),             SLOT(onDelete()));
    connect(m_ppblRedraw,      SIGNAL(clicked()),               SLOT(onRedraw()));
    connect(m_pHSplitter,      SIGNAL(splitterMoved(int,int)),  SLOT(onResizeColumns()));

    connect(m_pFuseLineView,   SIGNAL(selectedChanged(int)),    SLOT(onFrameClickedIn2dView()));
    connect(m_pFrameView,      SIGNAL(selectedChanged(int)),    SLOT(onPointClickedIn2dView()));

    connect(m_pFuseLineView,   SIGNAL(mouseDragReleased()),     SLOT(onUpdateFuseDlg()));
    connect(m_pFrameView,      SIGNAL(mouseDragReleased()),     SLOT(onUpdateFuseDlg()));
}


void FuseXflObjectDlg::enableStackBtns()
{
}


QList<QStandardItem *> FuseXflObjectDlg::prepareRow(const QString &object, const QString &field, const QString &value,  const QString &unit)
{
    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(object) << new QStandardItem(field) << new QStandardItem(value) << new QStandardItem(unit);
    for(int ii=0; ii<rowItems.size(); ii++) rowItems.at(ii)->setData(xfl::STRING, Qt::UserRole);
    return rowItems;
}


QList<QStandardItem *> FuseXflObjectDlg::prepareBoolRow(const QString &object, const QString &field, const bool &value)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem);

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::BOOLVALUE, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}


QList<QStandardItem *> FuseXflObjectDlg::prepareIntRow(const QString &object, const QString &field, const int &value)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem);

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::INTEGER, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}


QList<QStandardItem *> FuseXflObjectDlg::prepareDoubleRow(const QString &object, const QString &field, const double &value,  const QString &unit)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem(unit));

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}


void FuseXflObjectDlg::fillFuseXflTreeView()
{
    m_pModel->removeRows(0, m_pModel->rowCount());

    QList<QStandardItem*> dataItem;
    QStandardItem *pRootItem = m_pModel->invisibleRootItem();

    QModelIndex ind = m_pModel->index(0,0);
    m_pStruct->expand(ind);


    QList<QStandardItem*> bodyFolder = prepareRow("Body");
    pRootItem->appendRow(bodyFolder);

    m_pStruct->expand(m_pModel->indexFromItem(bodyFolder.front()));

    dataItem = prepareRow("Type", "Type", QString::fromStdString(Fuse::bodyPanelType(m_pFuseXfl->fuseType())));
    dataItem.at(2)->setData(xfl::BODYTYPE, Qt::UserRole);
    bodyFolder.front()->appendRow(dataItem);

    QList<QStandardItem*> bodyInertiaFolder = prepareRow("Inertia");
    bodyFolder.front()->appendRow(bodyInertiaFolder);
    m_pStruct->fillInertia(m_pFuseXfl->inertia(), bodyInertiaFolder.front());

    QList<QStandardItem*> NURBSFolder = prepareRow("NURBS");
    bodyFolder.front()->appendRow(NURBSFolder);
    {
        QList<QStandardItem*> dataItem = prepareIntRow("", "NURBS degree (lengthwise)", m_pFuseXfl->nurbs().uDegree());
        NURBSFolder.front()->appendRow(dataItem);

        dataItem = prepareIntRow("", "NURBS degree (hoop)", m_pFuseXfl->nurbs().vDegree());
        NURBSFolder.front()->appendRow(dataItem);

        dataItem = prepareIntRow("", "Mesh panels (lengthwise)", m_pFuseXfl->nxNurbsPanels());
        NURBSFolder.front()->appendRow(dataItem);

        dataItem = prepareIntRow("", "Mesh panels (hoop)", m_pFuseXfl->nhNurbsPanels());
        NURBSFolder.front()->appendRow(dataItem);
    }

    QList<QStandardItem*> hoopFolder = prepareRow("Hoop_panels (FLATPANELS case)");
    bodyFolder.front()->appendRow(hoopFolder);
    {
        for(int isl=0; isl<m_pFuseXfl->sideLineCount(); isl++)
        {
            QList<QStandardItem*> dataItem = prepareIntRow("", QString("Hoop panels in stripe %1").arg(isl+1), m_pFuseXfl->hPanels(isl));
            hoopFolder.front()->appendRow(dataItem);
        }
    }

    QList<QStandardItem*> bodyFrameFolder = prepareRow("Frames");
    bodyFolder.front()->appendRow(bodyFrameFolder);
    {
        m_pStruct->expand(m_pModel->indexFromItem(bodyFrameFolder.front()));

        for(int iFrame=0; iFrame <m_pFuseXfl->nurbs().frameCount(); iFrame++)
        {
            Frame const &pFrame = m_pFuseXfl->nurbs().frame(iFrame);

            QList<QStandardItem*> sectionFolder = prepareRow(QString("Frame_%1").arg(iFrame+1));
            bodyFrameFolder.front()->appendRow(sectionFolder);
            {
                if(m_pFuseXfl->activeFrameIndex()==iFrame) m_pStruct->expand(m_pModel->indexFromItem(sectionFolder.front()));

                dataItem = prepareIntRow("", "Lengthwise panels (FLATPANELS case)", m_pFuseXfl->xPanels(iFrame));
                sectionFolder.front()->appendRow(dataItem);

                QList<QStandardItem*> dataItem = prepareDoubleRow("x_Position", "x", pFrame.position().x*Units::mtoUnit(), Units::lengthUnitQLabel());
                sectionFolder.front()->appendRow(dataItem);

                for(int iPt=0; iPt<pFrame.nCtrlPoints(); iPt++)
                {
                    QList<QStandardItem*> pointFolder = prepareRow(QString("Point %1").arg(iPt+1));
                    sectionFolder.front()->appendRow(pointFolder);
                    {
                        if(Frame::selectedIndex()==iPt) m_pStruct->expand(m_pModel->indexFromItem(pointFolder.front()));

                        Vector3d Pt(pFrame.pointAt(iPt));
                        QList<QStandardItem*> dataItem = prepareDoubleRow("", "x", Pt.x*Units::mtoUnit(), Units::lengthUnitQLabel());
                        pointFolder.front()->appendRow(dataItem);

                        dataItem = prepareDoubleRow("", "y", Pt.y*Units::mtoUnit(), Units::lengthUnitQLabel());
                        pointFolder.front()->appendRow(dataItem);

                        dataItem = prepareDoubleRow("", "z", Pt.z*Units::mtoUnit(), Units::lengthUnitQLabel());
                        pointFolder.front()->appendRow(dataItem);
                    }
                }
            }
        }
    }
}


void FuseXflObjectDlg::onRedraw()
{
    QStandardItem *pItem = m_pModel->itemFromIndex(m_pModel->index(0,0));

    int iActiveFrame = m_pFuseXfl->activeFrameIndex();
    readFuseTree(pItem->child(0,0)->index());

    m_pFuseXfl->setActiveFrameIndex(iActiveFrame);

    m_pFuseXfl->makeFuseGeometry();
    updateFuseXfl();
    m_pglFuseView->resetFuse();
    updateView();

    m_bChanged = true;
}


void FuseXflObjectDlg::selectThing(int iFrame, int iPoint)
{
    QStandardItem *pItem = m_pModel->itemFromIndex(m_pModel->index(0,0));
    QModelIndex indexlevel = pItem->child(0,0)->index();

    QString object, field, value;
    do
    {
        object = indexlevel.sibling(indexlevel.row(),0).data().toString();
        field = indexlevel.sibling(indexlevel.row(),1).data().toString();
        value = indexlevel.sibling(indexlevel.row(),2).data().toString();

        QStandardItem *pItem = m_pModel->itemFromIndex(indexlevel);
        if(pItem->child(0,0))
        {
            if(object.compare("Frames", Qt::CaseInsensitive)==0)
            {
                QModelIndex subIndex = pItem->child(0,0)->index();
                do
                {
                    object = subIndex.sibling(subIndex.row(),0).data().toString();

                    QString strange;
                    strange = QString::asprintf("Frame_%d", iFrame+1);
                    if(object.compare(strange)==0)
                    {
                        // select this frame
                        if(iPoint<0)
                        {
                            m_pStruct->selectionModel()->clearSelection();
                            m_pStruct->selectionModel()->select(subIndex, QItemSelectionModel::Select);
                            m_pStruct->scrollTo(subIndex);
                            return;
                        }
                        // select the point
                        strange = QString::asprintf("Point %d", iPoint+1);
                        QStandardItem *pSubItem = m_pModel->itemFromIndex(subIndex)->child(0,0);
                        QModelIndex subSubIndex = pSubItem->index();

                        if(pSubItem)
                        {
                            do
                            {
                                QString txt = subSubIndex.data().toString();
                                if(txt.compare(strange)==0)
                                {
                                    //select this point
                                    m_pStruct->selectionModel()->clearSelection();
                                    m_pStruct->selectionModel()->select(subSubIndex, QItemSelectionModel::Select);
                                    m_pStruct->scrollTo(subSubIndex);
                                    return;
                                }
                                subSubIndex = subSubIndex.sibling(subSubIndex.row()+1,0);
                            }
                            while(subSubIndex.isValid());
                        }

                    }
                    subIndex = subIndex.sibling(subIndex.row()+1,0);
                }
                while(subIndex.isValid());
            }
        }

        indexlevel = indexlevel.sibling(indexlevel.row()+1,0);

    } while(indexlevel.isValid());
}


void FuseXflObjectDlg::onFrameClickedIn2dView()
{
    int iFrame = m_pFuseXfl->activeFrameIndex();
    selectThing(iFrame);

    m_pFrameView->update();
}


void FuseXflObjectDlg::onPointClickedIn2dView()
{
    selectThing(m_pFuseXfl->activeFrameIndex(), Frame::selectedIndex());
}


void FuseXflObjectDlg::onUpdateFuseDlg()
{
    takePicture();
    updateFuseDlg();
}


void FuseXflObjectDlg::updateFuseDlg()
{
    m_bChanged = true;

    fillFuseXflTreeView();
    updateFuseXfl();
    m_pglFuseView->resetFuse();
    updateView();
}


void FuseXflObjectDlg::onRefillBodyTree()
{
    fillFuseXflTreeView();
    m_pglFuseView->resetFuse();

    m_pglFuseView->update();
    m_pFuseLineView->update();
    m_pFrameView->update();
}


void FuseXflObjectDlg::readFuseTree(QModelIndex indexLevel)
{
    QString object, field, value;
    QModelIndex dataIndex, subIndex;

    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        field = indexLevel.sibling(indexLevel.row(),1).data().toString();
        value = indexLevel.sibling(indexLevel.row(),2).data().toString();

        QStandardItem *pItem = m_pModel->itemFromIndex(indexLevel);
        if(pItem->child(0,0))
        {
            if(object.compare("Inertia", Qt::CaseInsensitive)==0)
            {
                QStandardItem *pItem = m_pModel->itemFromIndex(indexLevel);
                if(pItem)
                {
                    m_pStruct->readInertiaTree(m_pFuseXfl->inertia(), pItem->child(0,0)->index());
                }
            }
            else if(object.compare("NURBS", Qt::CaseInsensitive)==0)
            {
                subIndex = pItem->child(0,0)->index();
                do
                {
                    object = subIndex.sibling(subIndex.row(),0).data().toString();
                    field = subIndex.sibling(subIndex.row(),1).data().toString();
                    value = subIndex.sibling(subIndex.row(),2).data().toString();

                    dataIndex = subIndex.sibling(subIndex.row(),2);

                    if(field.compare("NURBS degree (lengthwise)", Qt::CaseInsensitive)==0)     m_pFuseXfl->nurbs().setuDegree(dataIndex.data().toInt());
                    else if(field.compare("NURBS degree (hoop)", Qt::CaseInsensitive)==0)      m_pFuseXfl->nurbs().setvDegree(dataIndex.data().toInt());
                    else if(field.compare("Mesh panels (lengthwise)", Qt::CaseInsensitive)==0) m_pFuseXfl->setNxNurbsPanels(dataIndex.data().toInt());
                    else if(field.compare("Mesh panels (hoop)", Qt::CaseInsensitive)==0)       m_pFuseXfl->setNhNurbsPanels(dataIndex.data().toInt());

                    subIndex = subIndex.sibling(subIndex.row()+1,0);
                }
                while(subIndex.isValid());
            }
            else if(object.compare("Hoop_panels (FLATPANELS case)", Qt::CaseInsensitive)==0)
            {
                subIndex = pItem->child(0,0)->index();
                do
                {
                    object = subIndex.sibling(subIndex.row(),0).data().toString();
                    field = subIndex.sibling(subIndex.row(),1).data().toString();
                    value = subIndex.sibling(subIndex.row(),2).data().toString();
                    dataIndex = subIndex.sibling(subIndex.row(),2);

                    int idx = field.right(field.length()-22).toInt()-1;
                    m_pFuseXfl->setHPanels(idx,  dataIndex.data().toInt());

                    subIndex = subIndex.sibling(subIndex.row()+1,0);
                }
                while(subIndex.isValid());
            }
            else if(object.compare("Frames", Qt::CaseInsensitive)==0)
            {
                m_pFuseXfl->nurbs().clearFrames();
                subIndex = pItem->child(0,0)->index();
                do
                {
                    object = subIndex.sibling(subIndex.row(),0).data().toString();
                    if(object.indexOf("Frame_")>=0)
                    {
                        Frame pFrame;
                        QStandardItem *pSubItem = m_pModel->itemFromIndex(subIndex);
                        if(pSubItem->child(0,0))
                        {
                            readBodyFrameTree(pFrame, pSubItem->child(0,0)->index());
                            m_pFuseXfl->nurbs().appendFrame(pFrame);
                        }
                    }

                    subIndex = subIndex.sibling(subIndex.row()+1,0);
                }
                while(subIndex.isValid());
            }
        }
        else
        {
            //no more children
            object = indexLevel.sibling(indexLevel.row(),0).data().toString();
            field = indexLevel.sibling(indexLevel.row(),1).data().toString();
            value = indexLevel.sibling(indexLevel.row(),2).data().toString();

            dataIndex = indexLevel.sibling(indexLevel.row(),2);

            if(field.compare("Type", Qt::CaseInsensitive)==0) m_pFuseXfl->setFuseType(Fuse::bodyPanelType(value.toStdString()));
        }

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

    } while(indexLevel.isValid());
}


void FuseXflObjectDlg::readBodyFrameTree(Frame &pFrame, QModelIndex indexLevel)
{
    QString object, field, value;
    QModelIndex dataIndex;
    double xPt = 0.0;

    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        field = indexLevel.sibling(indexLevel.row(),1).data().toString();
        value = indexLevel.sibling(indexLevel.row(),2).data().toString();

        dataIndex = indexLevel.sibling(indexLevel.row(),2);

        if      (field.compare("Lengthwise panels (FLATPANELS case)", Qt::CaseInsensitive)==0)   m_pFuseXfl->appendXPanel(dataIndex.data().toInt());
        else if (object.compare("x_Position", Qt::CaseInsensitive)==0) xPt = dataIndex.data().toDouble()/Units::mtoUnit();
        else if (object.indexOf("Point", Qt::CaseInsensitive)==0)
        {
            Vector3d Pt;
            QStandardItem *pItem = m_pModel->itemFromIndex(indexLevel);
            if(pItem)
            {
                readVectorTree(Pt, pItem->child(0,0)->index());
                pFrame.appendPoint(Pt);
            }
        }
        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
    } while(indexLevel.isValid());

    pFrame.setuPosition(0, xPt);
}



void FuseXflObjectDlg::readInertiaTree(double &volumeMass, QVector<PointMass>&pointMasses, QModelIndex indexLevel)
{
    pointMasses.clear();

    QString object, field, value;
    QModelIndex dataIndex;
    do
    {
        QStandardItem *pItem = m_pModel->itemFromIndex(indexLevel);
        if(pItem->child(0,0))
        {
            object = indexLevel.sibling(indexLevel.row(),0).data().toString();
            if(object.indexOf("Point_mass_", Qt::CaseInsensitive)>=0)
            {
                PointMass ppm;
                readPointMassTree(ppm, pItem->child(0,0)->index());
                pointMasses.append(ppm);
            }
        }
        else
        {
            //no more children
            object = indexLevel.sibling(indexLevel.row(),0).data().toString();
            field = indexLevel.sibling(indexLevel.row(),1).data().toString();
            value = indexLevel.sibling(indexLevel.row(),2).data().toString();
            dataIndex = indexLevel.sibling(indexLevel.row(),2);

            if     (field.compare("Volume Mass", Qt::CaseInsensitive)==0)   volumeMass = dataIndex.data().toDouble()/Units::kgtoUnit();
        }

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

    } while(indexLevel.isValid());
}


void FuseXflObjectDlg::readPointMassTree(PointMass &ppm, QModelIndex indexLevel)
{
    QString object, field, value;
    QModelIndex dataIndex;
    // not expecting any more children
    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        field = indexLevel.sibling(indexLevel.row(),1).data().toString();
        value = indexLevel.sibling(indexLevel.row(),2).data().toString();
        dataIndex = indexLevel.sibling(indexLevel.row(),2);

        if      (field.compare("mass", Qt::CaseInsensitive)==0) ppm.setMass(dataIndex.data().toDouble()/Units::kgtoUnit());
        else if (field.compare("tag",  Qt::CaseInsensitive)==0) ppm.setTag(value.toStdString());
        else if (field.compare("x",    Qt::CaseInsensitive)==0) ppm.setXPosition(dataIndex.data().toDouble()/Units::mtoUnit());
        else if (field.compare("y",    Qt::CaseInsensitive)==0) ppm.setYPosition(dataIndex.data().toDouble()/Units::mtoUnit());
        else if (field.compare("z",    Qt::CaseInsensitive)==0) ppm.setZPosition(dataIndex.data().toDouble()/Units::mtoUnit());

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

    } while(indexLevel.isValid());
}



void FuseXflObjectDlg::readVectorTree(Vector3d &V, QModelIndex indexLevel)
{
    QString object, field, value;
    QModelIndex dataIndex;
    // not expecting any more children
    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        field = indexLevel.sibling(indexLevel.row(),1).data().toString();
        value = indexLevel.sibling(indexLevel.row(),2).data().toString();
        dataIndex = indexLevel.sibling(indexLevel.row(),2);

        if (field.compare("x", Qt::CaseInsensitive)==0)      V.x = dataIndex.data().toDouble()/Units::mtoUnit();
        else if (field.compare("y", Qt::CaseInsensitive)==0) V.y = dataIndex.data().toDouble()/Units::mtoUnit();
        else if (field.compare("z", Qt::CaseInsensitive)==0) V.z = dataIndex.data().toDouble()/Units::mtoUnit();

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
    } while(indexLevel.isValid());
}



void FuseXflObjectDlg::onItemClicked(const QModelIndex &index)
{
    identifySelection(index);
    update();
}


void FuseXflObjectDlg::identifySelection(const QModelIndex &indexSel)
{
    // we highlight wing sections and body frames
    // so check if the user's selection is one of these
    m_bIsInertiaSelected = false;
    m_iActivePointMass = -1;
    setActiveFrame(-1);
    Frame::setSelected(-1);

    QModelIndex indexLevel = indexSel;
    QString object;

    object = indexLevel.data().toString();
    if(object.compare("Inertia", Qt::CaseInsensitive)==0)
    {
        m_bIsInertiaSelected = true;
        return;
    }
    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();

        if(object.indexOf("Frame_", 0, Qt::CaseInsensitive)>=0)
        {
            setActiveFrame(object.right(object.length()-6).toInt() -1);
            //            Frame::setSelected(-1);
            m_pglFuseView->resetFrameHighlight();
            m_iActivePointMass = -1;
            return;
        }
        else if(object.indexOf("Point_Mass_", 0, Qt::CaseInsensitive)>=0)
        {
            m_bIsInertiaSelected = true;
            m_iActivePointMass = object.right(object.length()-11).toInt() -1;
            setActiveFrame(-1);
            return;
        }
        else if(object.indexOf("Point", 0, Qt::CaseInsensitive)==0)
        {
            Frame::setSelected(object.right(object.length()-6).toInt() -1);
            //identify the parent Frame object

            indexLevel = indexLevel.parent();
            do
            {
                object = indexLevel.sibling(indexLevel.row(),0).data().toString();

                if(object.indexOf("Frame_", 0, Qt::CaseInsensitive)>=0)
                {
                    setActiveFrame(object.right(object.length()-6).toInt() -1);
                    return;
                }

                indexLevel = indexLevel.parent();
            } while(indexLevel.isValid());

            setActiveFrame(-1);
            return;
        }
        indexLevel = indexLevel.parent();
    } while(indexLevel.isValid());
}


void FuseXflObjectDlg::setActiveFrame(int iFrame)
{
    if(iFrame<0 || iFrame>=m_pFuseXfl->frameCount()) return;

    m_pFuseXfl->setActiveFrameIndex(iFrame);
    m_pglFuseView->resetFrameHighlight();
    m_pglFuseView->update();
}


void FuseXflObjectDlg::onInsertBefore()
{
    if(!m_pFuseXfl) return;
    if( m_pFuseXfl->activeFrameIndex()>=0)
    {
        if(Frame::selectedIndex()>=0)
        {
            insertPointBefore();
        }
        else
        {
            m_pFuseXfl->insertFrameBefore(m_pFuseXfl->activeFrameIndex());
        }

        m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
        fillFuseXflTreeView();

        m_bChanged = true;
        m_pglFuseView->resetFrameHighlight();
        m_pglFuseView->resetFuse();
    }
    else if(m_iActivePointMass>=0)
    {
        m_pFuseXfl->insertPointMass(m_iActivePointMass, PointMass());

        m_pStruct->closePersistentEditor(m_pStruct->currentIndex());

        m_bChanged = true;
        m_pglFuseView->resetFrameHighlight();
        m_pglFuseView->update();
    }
    updateView();
}


void FuseXflObjectDlg::onInsertAfter()
{
    if(!m_pFuseXfl) return;

    if(m_pFuseXfl->activeFrameIndex()>=0)
    {
        if(Frame::selectedIndex()>=0)
        {
            insertPointAfter();
        }
        else
        {
            m_pFuseXfl->insertFrameAfter(m_pFuseXfl->activeFrameIndex());
            int newindex = m_pFuseXfl->activeFrameIndex()+1;
            m_pFuseXfl->setActiveFrameIndex(newindex);
        }

        m_pglFuseView->resetFuse();
    }
    else if(m_bIsInertiaSelected || m_iActivePointMass>=0)
    {
        if(m_bIsInertiaSelected)
        {
            m_pFuseXfl->appendPointMass(PointMass());
            m_iActivePointMass = m_pFuseXfl->pointMassCount()-1;
        }
        else if(m_iActivePointMass>=0)
        {
            m_pFuseXfl->insertPointMass(m_iActivePointMass+1, PointMass());
            m_iActivePointMass++;
        }
    }
    m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
    fillFuseXflTreeView();

    m_bChanged = true;
    updateView();
}


void FuseXflObjectDlg::onDelete()
{
    if(m_pFuseXfl && m_pFuseXfl->activeFrameIndex()>=0)
    {
        if(Frame::selectedIndex()>=0)
        {
            removeSelectedPoint();
        }
        else
        {
            m_pFuseXfl->removeFrame(m_pFuseXfl->activeFrameIndex());
        }

        m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
        fillFuseXflTreeView();

        m_bChanged = true;
        m_pglFuseView->resetFrameHighlight();
        m_pglFuseView->resetFuse();
        updateFuseXfl();
    }
    else if(m_iActivePointMass>=0)
    {
        if(m_pFuseXfl)
        {
            m_pFuseXfl->removePointMass(m_iActivePointMass);
        }

        m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
        fillFuseXflTreeView();

        m_bChanged = true;
        m_pglFuseView->resetFrameHighlight();
    }
    updateView();
}



void FuseXflObjectDlg::insertPointBefore()
{
    if(m_pFuseXfl->activeFrameIndex()<0) return;

    int iSel = Frame::selectedIndex();
    m_pFuseXfl->insertPoint(iSel);

    takePicture();
    updateFuseXfl();
    m_pglFuseView->resetFuse();
    updateView();
}


void FuseXflObjectDlg::insertPointAfter()
{
    if(m_pFuseXfl->activeFrameIndex()<0) return;
    int iSel = Frame::selectedIndex();
    m_pFuseXfl->insertPoint(iSel);

    Frame::setSelected(iSel+1);

    takePicture();
    updateFuseXfl();
    m_pglFuseView->resetFuse();
    updateView();
}


void FuseXflObjectDlg::removeSelectedPoint()
{
    if(m_pFuseXfl->activeFrameIndex()<0) return;

    int iSel = Frame::selectedIndex();

    if (iSel>=0)
    {
        m_pFuseXfl->removeSideLine(iSel);
    }
    takePicture();
}


void FuseXflObjectDlg::onConvertToFlatFace()
{
    FuseXflDlg::onConvertToFlatFace();
    updateFuseDlg();
}

