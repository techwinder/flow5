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

#include <QVBoxLayout>
#include <QHeaderView>


#include "wingobjectdlg.h"

#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dwingview.h>
#include <interfaces/editors/editobjectdelegate.h>
#include <interfaces/editors/objecttreeview.h>
#include <api/wingxfl.h>
#include <interfaces/widgets/color/colorbtn.h>

QByteArray WingObjectDlg::s_HSplitterSizes;

WingObjectDlg::WingObjectDlg(QWidget *pParent) : WingDlg(pParent)
{
    setupLayout();
    connectSignals();
    m_iActivePointMass = -1;
}


WingObjectDlg::~WingObjectDlg()
{
    delete m_pDelegate;
}


void WingObjectDlg::setupLayout()
{
    m_pHSplitter = new QSplitter;
    {
        m_pHSplitter->setChildrenCollapsible(false);
        QFrame *pLeftFrame = new QFrame;
        {
            QVBoxLayout*pLeftLayout = new QVBoxLayout;
            {
                QFrame *pMetaFrame = new QFrame;
                {
                    pMetaFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
                    QVBoxLayout *pMetaLayout = new QVBoxLayout;
                    {
                        QHBoxLayout *pNameClrLayout = new QHBoxLayout;
                        {
                            pNameClrLayout->addWidget(m_pleWingName);
                            pNameClrLayout->addWidget(m_pcbColor);
                            pNameClrLayout->setStretchFactor(m_pleWingName, 3);
                            pNameClrLayout->setStretchFactor(m_pcbColor, 1);
                        }

                        QLabel *pWingDescription = new QLabel("Description:");

                        pMetaLayout->addLayout(pNameClrLayout);
                        pMetaLayout->addWidget(pWingDescription);
                        pMetaLayout->addWidget(m_ppteDescription);
                    }
                    pMetaFrame->setLayout(pMetaLayout);
                }

                m_pTreeView = new ObjectTreeView(this);

                m_pModel = new QStandardItemModel(this);
                m_pModel->setColumnCount(4);
                m_pModel->clear();

                m_pDelegate = new EditObjectDelegate(this);
                m_pTreeView->setItemDelegate(m_pDelegate);
                connect(m_pDelegate,  SIGNAL(closeEditor(QWidget*)), SLOT(onEndEdit()));

                QStringList labels;
                labels << "Object" << "Field"<<"Value"<<"Unit";
                m_pModel->setHorizontalHeaderLabels(labels);
                m_pTreeView->setModel(m_pModel);
                QItemSelectionModel *pSelectionModel = new QItemSelectionModel(m_pModel);
                m_pTreeView->setSelectionModel(pSelectionModel);
                connect(pSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onItemClicked(QModelIndex)));

                pLeftLayout->addWidget(pMetaFrame);
                pLeftLayout->addWidget(m_pTreeView);
                pLeftLayout->addWidget(m_pButtonBox);
            }
            pLeftFrame->setLayout(pLeftLayout);
        }
        QFrame *pRightFrame = new QFrame;
        {
            QVBoxLayout*pRightLayout = new QVBoxLayout;
            {
                m_pglControls = new gl3dGeomControls(m_pglWingView, Qt::Horizontal, false);

                pRightLayout->addWidget(m_pglWingView);
                pRightLayout->addWidget(m_pglControls);
            }
            pRightFrame->setLayout(pRightLayout);
        }
        m_pHSplitter->addWidget(pLeftFrame);
        m_pHSplitter->addWidget(pRightFrame);
    }

    QHBoxLayout * pMainLayout = new QHBoxLayout;
    {
        pMainLayout->addWidget(m_pHSplitter);
    }
    setLayout(pMainLayout);
}


void WingObjectDlg::connectSignals()
{
    connectWingSignals();
//    connect(m_pModel, SIGNAL(itemChanged(QStandardItem *)), SLOT(onItemChanged(QStandardItem *)));
//    connect(m_pModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onCellChanged(QModelIndex ,QModelIndex)));
    connect(m_pHSplitter, SIGNAL(splitterMoved(int,int)), SLOT(onResizeColumns()));
}


void WingObjectDlg::initDialog(WingXfl*pWing)
{
    WingDlg::initDialog(pWing);
    m_pTreeView->fillWingTreeView(pWing);

    setWingProps();
}


void WingObjectDlg::onResizeColumns()
{
    QHeaderView *pHHeader = m_pTreeView->header();
    //pHHeader->setDefaultSectionSize(1);
    pHHeader->setSectionResizeMode(3, QHeaderView::Stretch);

    double w = double(m_pTreeView->width())/100.0;
    int wCols  = int(25*w);

    m_pTreeView->setColumnWidth(0, wCols);
    m_pTreeView->setColumnWidth(1, wCols);
    m_pTreeView->setColumnWidth(2, wCols);
}


void WingObjectDlg::showEvent(QShowEvent *pEvent)
{
    WingDlg::showEvent(pEvent);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    resizeEvent(nullptr);
}


void WingObjectDlg::resizeEvent(QResizeEvent *pEvent)
{
    WingDlg::resizeEvent(pEvent);
    onResizeColumns();
}

void WingObjectDlg::hideEvent(QHideEvent *pEvent)
{
    WingDlg::hideEvent(pEvent);
    s_HSplitterSizes  = m_pHSplitter->saveState();
}


void WingObjectDlg::updateData()
{
    m_pTreeView->fillWingTreeView(m_pWing);
}


void WingObjectDlg::onEndEdit()
{
    m_pWing->clearWingSections();
    m_pWing->clearPointMasses();
    m_pTreeView->readWingTree(m_pWing);
    m_bChanged = true;

    computeGeometry();
    setWingProps();

    m_pglWingView->resetglWing();
    m_pglWingView->update();
}


void WingObjectDlg::setCurrentSection(int )
{
}


void WingObjectDlg::readParams()
{
//    m_pObjectTreeView->readWingTree(m_pWing);
}


void WingObjectDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("WingObjectDlg");
    {
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
        s_HSplitterSizes    = settings.value("HSplitterSizes").toByteArray();
    }
    settings.endGroup();
}


void WingObjectDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("WingObjectDlg");
    {
        settings.setValue("WindowGeom", s_Geometry);

        settings.setValue("HSplitterSizes",  s_HSplitterSizes);
    }
    settings.endGroup();
}


void WingObjectDlg::onItemClicked(const QModelIndex &index)
{
    identifySelection(index);
    m_pglWingView->update(); // highlight
}


void WingObjectDlg::identifySelection(const QModelIndex &indexSel)
{
    // we highlight wing sections and body frames
    // so check if the user's selection is one of these

    m_iSection   = -1;
    m_iActivePointMass = -1;

    QModelIndex indexLevel = indexSel;
    QString object;
    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();

        if(object.indexOf("Section_", 0, Qt::CaseInsensitive)>=0)
        {
            m_iSection = object.right(object.length()-8).toInt() -1;

            m_iActivePointMass = -1;
            m_pglWingView->resetglSectionHighlight(m_iSection, false);
            return;
        }
        else if(object.indexOf("Point_Mass_", 0, Qt::CaseInsensitive)>=0)
        {
            m_iActivePointMass = object.right(object.length()-11).toInt() -1;
            return;
        }
        indexLevel = indexLevel.parent();
    } while(indexLevel.isValid());
}

