/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <iostream>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QModelIndex>
#include <QVBoxLayout>
#include <QPushButton>
#include <QtConcurrent/QtConcurrentRun>

#include "batchxfoildlg.h"

#include <api/fl5core.h>
#include <api/analysisrange.h>
#include <api/flow5events.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/oppoint.h>
#include <api/polar.h>
#include <api/utils.h>
#include <api/xfoiltask.h>
#include <modules/xdirect/analysis/polarnamemaker.h>


#include <core/displayoptions.h>
#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/controls/analysisrangetable.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <modules/xdirect/xdirect.h>



xfl::enumPolarType BatchXFoilDlg::s_PolarType = xfl::T1POLAR;

double BatchXFoilDlg::s_XTop   = 1.0;
double BatchXFoilDlg::s_XBot   = 1.0;

bool BatchXFoilDlg::s_bTransAtHinge = false;

QVector<bool> BatchXFoilDlg::s_ActiveList;
QVector<double> BatchXFoilDlg::s_ReList;
QVector<double> BatchXFoilDlg::s_MachList;
QVector<double> BatchXFoilDlg::s_NCritList;


BatchXFoilDlg::BatchXFoilDlg(QWidget *pParent) : BatchDlg(pParent)
{
    setWindowTitle("Multi-threaded batch analysis");

    setupLayout();
    connectBaseSignals();
    connectSignals();
}


BatchXFoilDlg::~BatchXFoilDlg()
{
    if(m_pXFile)  delete m_pXFile;
    m_pXFile = nullptr;
}


void BatchXFoilDlg::setupLayout()
{
    m_plwNameList = new QListWidget;
    m_plwNameList->setSelectionMode(QAbstractItemView::MultiSelection);

    m_pfrPolars = new QFrame;
    {
        QVBoxLayout *pPolarsLayout = new QVBoxLayout;
        {
            QGroupBox *pPolarTypeBox = new QGroupBox("Polar type");
            {
                QHBoxLayout *pPolarTypeLayout =new QHBoxLayout;
                {
                    m_prbT1 = new QRadioButton("T1");
                    m_prbT1->setToolTip("Fixed speed polar");
                    m_prbT2 = new QRadioButton("T2");
                    m_prbT2->setToolTip("Fixed lift polar");
                    m_prbT3 = new QRadioButton("T3");
                    m_prbT3->setToolTip("Rubber chord polar");
                    pPolarTypeLayout->addStretch();
                    pPolarTypeLayout->addWidget(m_prbT1);
                    pPolarTypeLayout->addStretch();
                    pPolarTypeLayout->addWidget(m_prbT2);
                    pPolarTypeLayout->addStretch();
                    pPolarTypeLayout->addWidget(m_prbT3);
                    pPolarTypeLayout->addStretch();
                }
                pPolarTypeBox->setLayout(pPolarTypeLayout);
            }

            m_pcptReTable = new CPTableView(this);
            {
                m_pcptReTable->setEditable(true);
                m_pcptReTable->setEditTriggers(QAbstractItemView::CurrentChanged |
                                               QAbstractItemView::DoubleClicked |
                                               QAbstractItemView::SelectedClicked |
                                               QAbstractItemView::EditKeyPressed |
                                               QAbstractItemView::AnyKeyPressed);
                m_pReModel = new ActionItemModel(this);
                m_pReModel->setRowCount(5);//temporary
                m_pReModel->setColumnCount(5);
                m_pReModel->setActionColumn(4);
                m_pReModel->setHeaderData(0, Qt::Horizontal, QString());
                m_pReModel->setHeaderData(1, Qt::Horizontal, "Re");
                m_pReModel->setHeaderData(2, Qt::Horizontal, "Mach");
                m_pReModel->setHeaderData(3, Qt::Horizontal, "NCrit");
                m_pReModel->setHeaderData(4, Qt::Horizontal, "Actions");

                m_pcptReTable->setModel(m_pReModel);

                int n = m_pReModel->actionColumn();
                QHeaderView *pHHeader = m_pcptReTable->horizontalHeader();
                pHHeader->setSectionResizeMode(n, QHeaderView::Stretch);
                pHHeader->resizeSection(n, 1);

                m_pFloatDelegate = new XflDelegate(this);
                m_pFloatDelegate->setCheckColumn(0);
                m_pFloatDelegate->setActionColumn(4);
                m_pFloatDelegate->setDigits({-1,0,2,2,-1});
                m_pFloatDelegate->setItemTypes({XflDelegate::CHECKBOX, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::ACTION});
                m_pcptReTable->setItemDelegate(m_pFloatDelegate);

                m_pInsertBeforeAct  = new QAction("Insert before", this);
                m_pInsertAfterAct   = new QAction("Insert after",  this);
                m_pDeleteAct        = new QAction("Delete",        this);
            }

            QGroupBox *m_pgbTransVars = new QGroupBox("Forced Transitions");
            {
                QGridLayout *pTransVarsLayout = new QGridLayout;
                {
                    QLabel *plabTopTrans = new QLabel("Top transition location (x/c)");
                    QLabel *plabBotTrans = new QLabel("Bottom transition location (x/c)");
                    m_pfeXTopTr = new FloatEdit(1.0f);
                    m_pfeXBotTr = new FloatEdit(1.0f);

                    m_pchTransAtHinge = new QCheckBox("Force transition at hinge location");
                    m_pchTransAtHinge->setToolTip("<p>"
                                                  "Forces the laminar to turbulent transition at the hinge's location on both the top and bottom surfaces.<br>"
                                                  "The transition location is set as the most upwind position between the hinge's location "
                                                  "and the forced transition location.<br>"
                                                  "Only used in the case of flapped surfaces.<br>"
                                                  "This greatly increases the convergence success rate and the speed of XFoil calculations."
                                                  "</p>");

                    pTransVarsLayout->addWidget(plabTopTrans,      1, 1);
                    pTransVarsLayout->addWidget(m_pfeXTopTr,       1, 2);
                    pTransVarsLayout->addWidget(plabBotTrans,      2, 1);
                    pTransVarsLayout->addWidget(m_pfeXBotTr,       2, 2);
                    pTransVarsLayout->addWidget(m_pchTransAtHinge, 3,1,1,2);
                    pTransVarsLayout->setColumnStretch(3, 1);
                }
                m_pgbTransVars->setLayout(pTransVarsLayout);
            }


            pPolarsLayout->addWidget(pPolarTypeBox);
            pPolarsLayout->addWidget(m_pcptReTable);
            pPolarsLayout->addWidget(m_pgbTransVars);
        }
        m_pfrPolars->setLayout(pPolarsLayout);
    }

    m_pLeftTabWt->addTab(m_plwNameList,  "Foils");
    m_pLeftTabWt->addTab(m_pfrPolars,    "Polars");
    m_pLeftTabWt->addTab(m_pfrRangeVars, "Operating points");

    QHBoxLayout *pBoxesLayout = new QHBoxLayout;
    {
        pBoxesLayout->addWidget(m_pHSplitter);
    }

    setLayout(pBoxesLayout);
}


void BatchXFoilDlg::connectSignals()
{
    connect(m_pLeftTabWt,         SIGNAL(currentChanged(int)),                  SLOT(onResizeColumns()));
    connect(m_pHSplitter,         SIGNAL(splitterMoved(int,int)),               SLOT(onResizeColumns()));

    connect(m_pcptReTable,        SIGNAL(clicked(QModelIndex)),                 SLOT(onReTableClicked(QModelIndex)));
    connect(m_pReModel,           SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onCellChanged(QModelIndex,QModelIndex)));
    connect(m_pDeleteAct,         SIGNAL(triggered(bool)),                      SLOT(onDelete()));
    connect(m_pInsertBeforeAct,   SIGNAL(triggered(bool)),                      SLOT(onInsertBefore()));
    connect(m_pInsertAfterAct,    SIGNAL(triggered(bool)),                      SLOT(onInsertAfter()));
}


void BatchXFoilDlg::initDialog()
{
    BatchDlg::initDialog();

    m_pT4RangeTable->setEnabled(false);
    m_pT6RangeTable->setEnabled(false);

    if(s_ActiveList.size()==0 || s_ReList.size()==0 || s_MachList.size()==0 || s_NCritList.size()==0)
        initReList();

    for(int i=0; i<Objects2d::nFoils(); i++)
    {
        Foil const *pFoil = Objects2d::foilAt(i);
        if(pFoil)
        {
            m_plwNameList->addItem(QString::fromStdString(pFoil->name()));
            if(m_pFoil==pFoil)
            {
                QListWidgetItem *pItem =  m_plwNameList->item(i);
                pItem->setSelected(true);
            }
        }
    }

    switch (s_PolarType)
    {
        default:
        case xfl::T1POLAR:   m_prbT1->setChecked(true);   break;
        case xfl::T2POLAR:   m_prbT2->setChecked(true);   break;
        case xfl::T3POLAR:   m_prbT3->setChecked(true);   break;
    }


    m_pfeXTopTr->setValue(s_XTop);
    m_pfeXBotTr->setValue(s_XBot);
    m_pchTransAtHinge->setChecked(s_bTransAtHinge);

    fillReModel();
}



void BatchXFoilDlg::onSpecChanged()
{
    readParams();
}


void BatchXFoilDlg::readParams()
{
    BatchDlg::readParams();

    if     (m_prbT2->isChecked()) s_PolarType = xfl::T2POLAR;
    else if(m_prbT3->isChecked()) s_PolarType = xfl::T3POLAR;
    else                          s_PolarType = xfl::T1POLAR;

    s_XTop   = m_pfeXTopTr->value();
    s_XBot   = m_pfeXBotTr->value();
    s_bTransAtHinge = m_pchTransAtHinge->isChecked();
}


void BatchXFoilDlg::outputReList()
{
    m_ppto->appendPlainText("Reynolds numbers to analyze:\n");

    for(int i=0; i<s_ReList.count(); i++)
    {
        if(s_ActiveList.at(i))
        {
            QString strong = QString::asprintf("   Re = %10.0f  /  Mach = %5.3f  /  NCrit = %5.2f\n", s_ReList.at(i), s_MachList.at(i), s_NCritList.at(i));
            m_ppto->appendPlainText(strong);
        }
    }

    m_ppto->appendPlainText("\n");
}


void BatchXFoilDlg::onResizeColumns()
{
    double w = double(m_pcptReTable->width())*.93;
    int wCols  = int(w/10);
    m_pcptReTable->setColumnWidth(0, wCols);
    m_pcptReTable->setColumnWidth(1, 2*wCols);
    m_pcptReTable->setColumnWidth(2, 2*wCols);
    m_pcptReTable->setColumnWidth(3, 2*wCols);
    m_pcptReTable->setColumnWidth(4, 2*wCols);
    update();
}


void BatchXFoilDlg::resizeEvent(QResizeEvent*pEvent)
{
    BatchDlg::resizeEvent(pEvent);
    onResizeColumns();
}


void BatchXFoilDlg::showEvent(QShowEvent *pEvent)
{
    BatchDlg::showEvent(pEvent);
    onResizeColumns();
}


void BatchXFoilDlg::initReList()
{
    s_ActiveList.resize(12);
    s_ReList.resize(12);
    s_MachList.resize(12);
    s_NCritList.resize(12);

    s_ActiveList.fill(true);

    s_ReList[0]  =   30000.0;
    s_ReList[1]  =   40000.0;
    s_ReList[2]  =   60000.0;
    s_ReList[3]  =   80000.0;
    s_ReList[4]  =  100000.0;
    s_ReList[5]  =  130000.0;
    s_ReList[6]  =  160000.0;
    s_ReList[7]  =  200000.0;
    s_ReList[8]  =  300000.0;
    s_ReList[9]  =  500000.0;
    s_ReList[10] = 1000000.0;
    s_ReList[11] = 3000000.0;

    s_MachList.fill(0);
    s_NCritList.fill(9);
}


void BatchXFoilDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("XFoilBatchDlg");
    {
        int iType   = settings.value("PolarType",    1).toInt();
        if     (iType==2)  s_PolarType = xfl::T2POLAR;
        else if(iType==3)  s_PolarType = xfl::T3POLAR;
        else               s_PolarType = xfl::T1POLAR;

        s_bAlpha        = settings.value("bAlpha",        s_bAlpha).toBool();
        s_XTop          = settings.value("XTrTop",        s_XTop).toDouble();
        s_XBot          = settings.value("XTrBot",        s_XBot).toDouble();
        s_bTransAtHinge = settings.value("bTransAtHinge", s_bTransAtHinge).toBool();

        if(settings.contains("NReynolds"))
        {
            int NRe = settings.value("NReynolds").toInt();
            s_ActiveList.clear();
            s_ReList.clear();
            s_MachList.clear();
            s_NCritList.clear();
            for (int i=0; i<NRe; i++)
            {
                QString str0 = QString("ActiveList%1").arg(i);
                QString str1 = QString("ReList%1").arg(i);
                QString str2 = QString("MaList%1").arg(i);
                QString str3 = QString("NcList%1").arg(i);
                if(settings.contains(str0)) s_ActiveList.append(settings.value(str0).toBool());
                if(settings.contains(str1)) s_ReList.append(settings.value(str1).toDouble());
                if(settings.contains(str2)) s_MachList.append(settings.value(str2).toDouble());
                if(settings.contains(str3)) s_NCritList.append(settings.value(str3).toDouble());
            }
        }

        s_HSplitterSizes = settings.value("HSplitterSizes").toByteArray();
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void BatchXFoilDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("XFoilBatchDlg");
    {
        switch (s_PolarType)
        {
            default:
            case xfl::T1POLAR:   settings.setValue("PolarType", 1);   break;
            case xfl::T2POLAR:   settings.setValue("PolarType", 2);   break;
            case xfl::T3POLAR:   settings.setValue("PolarType", 3);   break;
        }

        settings.setValue("bAlpha",         s_bAlpha);

        settings.setValue("XTrTop",         s_XTop);
        settings.setValue("XTrBot",         s_XBot);
        settings.setValue("bTransAtHinge",  s_bTransAtHinge);

        settings.setValue("NReynolds", s_ReList.count());
        if(s_ActiveList.size()!=s_ReList.size()) s_ActiveList.resize(s_ReList.size());
        if(s_MachList.size()  !=s_ReList.size()) s_MachList.resize(s_ReList.size());
        if(s_NCritList.size() !=s_ReList.size()) s_NCritList.resize(s_ReList.size());


        for (int i=0; i<s_ReList.count(); i++)
        {
            QString str0 = QString("ActiveList%1").arg(i);
            QString str1 = QString("ReList%1").arg(i);
            QString str2 = QString("MaList%1").arg(i);
            QString str3 = QString("NcList%1").arg(i);
            settings.setValue(str0, s_ActiveList.at(i));
            settings.setValue(str1, s_ReList.at(i));
            settings.setValue(str2, s_MachList.at(i));
            settings.setValue(str3, s_NCritList.at(i));
        }

        settings.setValue("VSplitterSizes",  s_HSplitterSizes);
        settings.setValue("WindowGeom",      s_Geometry);
    }
    settings.endGroup();
}


void BatchXFoilDlg::fillReModel()
{
    m_pReModel->setRowCount(s_ReList.count());
    m_pReModel->blockSignals(true);

    for (int i=0; i<s_ReList.count(); i++)
    {
        QModelIndex chindex = m_pReModel->index(i, 0, QModelIndex());
        m_pReModel->setData(chindex, s_ActiveList.at(i), Qt::UserRole);

        QModelIndex Xindex = m_pReModel->index(i, 1, QModelIndex());
        m_pReModel->setData(Xindex, s_ReList.at(i));

        QModelIndex Yindex = m_pReModel->index(i, 2, QModelIndex());
        m_pReModel->setData(Yindex, s_MachList.at(i));

        QModelIndex Zindex = m_pReModel->index(i, 3, QModelIndex());
        m_pReModel->setData(Zindex, s_NCritList.at(i));

        QModelIndex actionindex = m_pReModel->index(i, 4, QModelIndex());
        m_pReModel->setData(actionindex, QString("..."));
    }
    m_pReModel->blockSignals(false);
    m_pcptReTable->resizeRowsToContents();
}


void BatchXFoilDlg::setRowEnabled(int  row, bool bEnabled)
{
    for(int col=0; col<m_pReModel->columnCount(); col++)
    {
        QModelIndex ind = m_pReModel->index(row, col, QModelIndex());
        m_pReModel->setData(ind, bEnabled, Qt::UserRole); // used to display the row as enabled or disabled
    }
}


void BatchXFoilDlg::onDelete()
{
    if(m_pReModel->rowCount()<=1) return;

    QModelIndex index = m_pcptReTable->currentIndex();
    int sel = index.row();

    if(sel<0 || sel>=s_ReList.count()) return;

    s_ActiveList.removeAt(sel);
    s_ReList.removeAt(sel);
    s_MachList.removeAt(sel);
    s_NCritList.removeAt(sel);

    fillReModel();
    m_pcptReTable->closePersistentEditor(m_pcptReTable->currentIndex());
}


void BatchXFoilDlg::onInsertBefore()
{
    int sel = m_pcptReTable->currentIndex().row();

    s_ActiveList.insert(sel, true);
    s_ReList.insert(sel, 0.0);
    s_MachList.insert(sel, 0.0);
    s_NCritList.insert(sel, 0.0);

    if     (sel>0)   s_ReList[sel] = (s_ReList.at(sel-1)+s_ReList.at(sel+1)) /2.0;
    else if(sel==0)  s_ReList[sel] =  s_ReList.at(sel+1)                     /2.0;
    else             s_ReList[0]   = 100000.0;

    if(sel>=0)
    {
        s_MachList[sel]  = s_MachList.at(sel+1);
        s_NCritList[sel] = s_NCritList.at(sel+1);
    }
    else
    {
        sel = 0;
        s_MachList[sel]  = 0.0;
        s_NCritList[sel] = 0.0;
    }

    fillReModel();
    m_pcptReTable->closePersistentEditor(m_pcptReTable->currentIndex());


    QModelIndex index = m_pReModel->index(sel, 0, QModelIndex());
    m_pcptReTable->setCurrentIndex(index);
    m_pcptReTable->selectRow(index.row());
}


void BatchXFoilDlg::onInsertAfter()
{
    int sel = m_pcptReTable->currentIndex().row()+1;

    s_ActiveList.insert(sel, true);
    s_ReList.insert(sel, 0.0);
    s_MachList.insert(sel, 0.0);
    s_NCritList.insert(sel, 0.0);

    if(sel==s_ReList.size()-1) s_ReList[sel] = s_ReList[sel-1]*2.0;
    else if(sel>0)             s_ReList[sel] = (s_ReList[sel-1]+s_ReList[sel+1]) /2.0;
    else if(sel==0)            s_ReList[sel] = s_ReList[sel+1]                   /2.0;

    if(sel>0)
    {
        s_MachList[sel]  = s_MachList[sel-1];
        s_NCritList[sel] = s_NCritList[sel-1];
    }
    else
    {
        sel = 0;
        s_MachList[sel]  = 0.0;
        s_NCritList[sel] = 0.0;
    }

    fillReModel();
    m_pcptReTable->closePersistentEditor(m_pcptReTable->currentIndex());

    QModelIndex index = m_pReModel->index(sel, 0, QModelIndex());
    m_pcptReTable->setCurrentIndex(index);
    m_pcptReTable->selectRow(index.row());
}


void BatchXFoilDlg::onCellChanged(QModelIndex topLeft, QModelIndex )
{
    s_ActiveList.clear();
    s_ReList.clear();
    s_MachList.clear();
    s_NCritList.clear();

    for (int i=0; i<m_pReModel->rowCount(); i++)
    {
        s_ActiveList.append(m_pReModel->index(i, 0, QModelIndex()).data(Qt::UserRole).toBool());
        s_ReList.append(    m_pReModel->index(i, 1, QModelIndex()).data().toDouble());
        s_MachList.append(  m_pReModel->index(i, 2, QModelIndex()).data().toDouble());
        s_NCritList.append( m_pReModel->index(i, 3, QModelIndex()).data().toDouble());
    }

    if(topLeft.column()==0)
    {
        sortRe();

        //and fill back the model
        fillReModel();
    }
}


/**
* Bubble sort algorithm for the arrays of Reynolds, Mach and NCrit numbers.
* The arrays are sorted by crescending Re numbers.
*/
void BatchXFoilDlg::sortRe()
{
    int indx(0), indx2(0);
    bool Chtemp(true), Chtemp2(true);
    double Retemp(0), Retemp2(0);
    double Matemp(0), Matemp2(0);
    double NCtemp(0), NCtemp2(0);
    int flipped(0);

    if (s_ReList.size()<=1) return;

    indx = 1;
    do
    {
        flipped = 0;
        for (indx2 = s_ReList.size() - 1; indx2 >= indx; --indx2)
        {
            Chtemp  = s_ActiveList.at(indx2);
            Chtemp2 = s_ActiveList.at(indx2 - 1);
            Retemp  = s_ReList.at(indx2);
            Retemp2 = s_ReList.at(indx2 - 1);
            Matemp  = s_MachList.at(indx2);
            Matemp2 = s_MachList.at(indx2 - 1);
            NCtemp  = s_NCritList.at(indx2);
            NCtemp2 = s_NCritList.at(indx2 - 1);
            if (Retemp2> Retemp)
            {
                s_ActiveList[indx2 - 1] = Chtemp;
                s_ActiveList[indx2]     = Chtemp2;
                s_ReList[indx2 - 1]     = Retemp;
                s_ReList[indx2]         = Retemp2;
                s_MachList[indx2 - 1]   = Matemp;
                s_MachList[indx2]       = Matemp2;
                s_NCritList[indx2 - 1]  = NCtemp;
                s_NCritList[indx2]      = NCtemp2;
                flipped = 1;
            }
        }
    } while ((++indx < s_ReList.size()) && flipped);
}


void BatchXFoilDlg::onReTableClicked(QModelIndex index)
{
    if(!index.isValid())  return;

    int row = index.row();
    if(row<0 || row>=m_pReModel->rowCount()) return;

    m_pcptReTable->selectRow(row);

    switch(index.column())
    {
        case 0:
        {
            bool bActive = m_pReModel->data(index, Qt::UserRole).toBool();
            if(row<s_ActiveList.size())
            {
                s_ActiveList[row] = !bActive; // toggle
                m_pReModel->setData(index, s_ActiveList.at(row), Qt::UserRole);
            }
            break;
        }
        case 4:
        {
            QRect itemrect = m_pcptReTable->visualRect(index);
            QPoint menupos = m_pcptReTable->mapToGlobal(itemrect.topLeft());
            QMenu *pReTableRowMenu = new QMenu("Section", this);
            pReTableRowMenu->addAction(m_pInsertBeforeAct);
            pReTableRowMenu->addAction(m_pInsertAfterAct);
            pReTableRowMenu->addAction(m_pDeleteAct);
            pReTableRowMenu->exec(menupos, m_pInsertBeforeAct);

            break;
        }
        default:  break;

    }
    update();
}


void BatchXFoilDlg::readFoils(QVector<Foil*> &foils)
{
    foils.clear();
    for(int i=0; i<m_plwNameList->count();i++)
    {
        QListWidgetItem *pItem = m_plwNameList->item(i);
        if(pItem && pItem->isSelected())
        {
            Foil *pFoil = Objects2d::foil(pItem->text().toStdString());
            if(pFoil)
                foils.append(pFoil);
        }
    }
}


void BatchXFoilDlg::onAnalyze()
{
    if(m_bIsRunning)
    {
        m_bCancel = true;
        XFoilTask::setCancelled(true);
        XFoil::setCancel(true);
        return;
    }

    m_bCancel    = false;
    m_bIsRunning = true;

    m_pButtonBox->button(QDialogButtonBox::Close)->setEnabled(false);

    QString FileName = SaveOptions::newLogFileName();
    m_pXFile = new QFile(FileName);
    if (!m_pXFile->open(QIODevice::WriteOnly | QIODevice::Text)) m_pXFile = nullptr;

    readParams();

    setFileHeader();

    m_ppbAnalyze->setFocus();

    QString strong;

    QVector<Foil*> foils;
    readFoils(foils);

    if(foils.isEmpty())
    {
        strong ="No foil defined for analysis\n\n";
        m_ppto->onAppendQText(strong);
        cleanUp();
        return;
    }

    if(s_bTransAtHinge) m_ppto->onAppendQText("\nForcing transition at hinge location\n");
    strong = QString::asprintf("Forced top  transition = %7.1f%%\n", s_XTop*100.0);
    m_ppto->onAppendQText(strong);
    strong = QString::asprintf("Forced bot. transition = %7.1f%%\n", s_XBot*100.0);
    m_ppto->onAppendQText(strong);


    m_ppbAnalyze->setText("Cancel");

    m_nAnalysis = 0;
    m_nTaskDone = 0;
    m_nTaskStarted = 0;

    m_AnalysisPair.clear();
    for(int ifoil=0; ifoil<foils.count(); ifoil++)
    {
        Foil *pFoil = foils.at(ifoil);
        if(pFoil)
        {
            double XtrTop = s_XTop;
            double XtrBot = s_XBot;
            if(pFoil->hasTEFlap() && s_bTransAtHinge)
            {
                XtrTop = std::min(XtrTop, pFoil->TEXHinge());
                XtrBot = std::min(XtrBot, pFoil->TEXHinge());
            }

            for (int iRe=0; iRe<s_ActiveList.size(); iRe++)
            {
                if(s_ActiveList.at(iRe))
                {
                    m_AnalysisPair.push_back(FoilAnalysis());
                    FoilAnalysis &analysis = m_AnalysisPair.back();
                    analysis.m_Foil.copy(pFoil);

                    Polar *pNewPolar = Objects2d::createPolar(pFoil, s_PolarType,
                                                              s_ReList.at(iRe), s_MachList.at(iRe), s_NCritList.at(iRe),
                                                              XtrTop, XtrBot);

                    pNewPolar->setName(PolarNameMaker::makeName(pNewPolar).toStdString());

                    Polar *pOldPolar = Objects2d::polar(pFoil, pNewPolar->name());

                    if(pOldPolar)
                    {
                        delete pNewPolar;
                        analysis.m_pPolar = pOldPolar;
                    }
                    else
                    {
                        analysis.m_pPolar = pNewPolar;
                        Objects2d::insertPolar(pNewPolar);
                    }


                    m_nAnalysis++;
                }
            }
        }
    }


    strong = QString::asprintf("\nFound %d foil/polar pairs to analyze\n", m_nAnalysis);
    m_ppto->onAppendQText(strong);


    XFoilTask::setCancelled(false);

    // failsafe: clean up any remaining tasks from a previous batch
    if(m_Tasks.size()!=0)
    {
        m_ppto->appendPlainText("Uncleaned !\n");
        for(uint it=0; it<m_Tasks.size(); it++)
            delete m_Tasks.at(it);

        m_Tasks.clear();
    }

    QApplication::setOverrideCursor(Qt::BusyCursor);

    m_ppto->appendPlainText("\nStarted/Done/Total\n");

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
            QFuture<void> future = QtConcurrent::run(&BatchXFoilDlg::batchLaunch, this);
#else
            QtConcurrent::run(pXFoilTask, &XFoilTask::run);
#endif
}




