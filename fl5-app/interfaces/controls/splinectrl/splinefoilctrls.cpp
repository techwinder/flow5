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

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>


#include "splinefoilctrls.h"
#include <api/splinefoil.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>

SplineFoilCtrls::SplineFoilCtrls(QWidget *pParent): QWidget(pParent)
{
    setWindowTitle("Spline Parameters");
    m_pSF = nullptr;
    setupLayout();
    connectSignals();
}


void SplineFoilCtrls::initDialog(SplineFoil *pSF)
{
    m_pSF = pSF;

    fillPointLists();

    m_plbSplineStyle->setTheStyle(pSF->theStyle());

    m_pcbDegIntrados->setCurrentIndex(m_pSF->intrados().degree()-2);
    m_pieOutIntrados->setValue(m_pSF->intrados().outputSize());
    m_pcbDegExtrados->setCurrentIndex(m_pSF->extrados().degree()-2);
    m_pieOutExtrados->setValue(m_pSF->extrados().outputSize());

    setControls();
}


void SplineFoilCtrls::showEvent(QShowEvent *pEvent)
{
    QWidget::showEvent(pEvent);
    int w = m_pcptUpperList->width();
    m_pcptUpperList->setColumnWidth(0,int(w/3)-20);
    m_pcptUpperList->setColumnWidth(1,int(w/3)-20);
    m_pcptUpperList->setColumnWidth(2,int(w/3)-20);
    w = m_pcptLowerList->width();
    m_pcptLowerList->setColumnWidth(0,int(w/3)-20);
    m_pcptLowerList->setColumnWidth(1,int(w/3)-20);
    m_pcptLowerList->setColumnWidth(2,int(w/3)-20);
}


void SplineFoilCtrls::setupLayout()
{
    QVBoxLayout *pSideLayout = new QVBoxLayout;
    {
        QGroupBox *pUpperSideBox = new QGroupBox("Upper side");
        {
            QVBoxLayout *pUpperSideLayout = new QVBoxLayout;
            {
                QGridLayout *pUpperLayout = new QGridLayout;
                {
                    QLabel *pLabUpper1 = new QLabel("Spline degree");
                    QLabel *pLabUpper2 = new QLabel("Number of curve points");
                    pLabUpper1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    pLabUpper2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_pcbDegExtrados = new QComboBox;
                    m_pieOutExtrados = new IntEdit;
                    QString str;
                    m_pcbDegExtrados->clear();
                    for (int i=2; i<6; i++)
                    {
                        str = QString("%1").arg(i);
                        m_pcbDegExtrados->addItem(str);
                    }
                    m_pcbDegExtrados->setEnabled(true);

                    pUpperLayout->addWidget(pLabUpper1, 1,1);
                    pUpperLayout->addWidget(pLabUpper2, 2,1);
                    pUpperLayout->addWidget(m_pcbDegExtrados, 1,2);
                    pUpperLayout->addWidget(m_pieOutExtrados, 2,2);
                }

                m_pcptUpperList = new CPTableView(this);
                pUpperSideLayout->addLayout(pUpperLayout);
                pUpperSideLayout->addWidget(m_pcptUpperList);


                //upper point list
                m_pUpperListModel = new QStandardItemModel(this);
                m_pUpperListModel->setRowCount(10);//temporary
                m_pUpperListModel->setColumnCount(3);

                m_pUpperListModel->setHeaderData(0, Qt::Horizontal, "Point");
                m_pUpperListModel->setHeaderData(1, Qt::Horizontal, "x");
                m_pUpperListModel->setHeaderData(2, Qt::Horizontal, "y");

                m_pcptUpperList->setModel(m_pUpperListModel);

                QHeaderView *pHorizontalHeader = m_pcptUpperList->horizontalHeader();
                pHorizontalHeader->setStretchLastSection(true);

                m_pUpperFloatDelegate = new XflDelegate(this);
                m_pUpperFloatDelegate->setDigits({0,3,3});
                m_pUpperFloatDelegate->setItemTypes({XflDelegate::STRING, XflDelegate::DOUBLE, XflDelegate::DOUBLE});
                m_pcptUpperList->setItemDelegate(m_pUpperFloatDelegate);

            }
            pUpperSideBox->setLayout(pUpperSideLayout);
        }

        QGroupBox *pLowerSideBox = new QGroupBox("Lower side");
        {
            QVBoxLayout *pLowerSideLayout = new QVBoxLayout;
            {
                QGridLayout *pLowerLayout = new QGridLayout;
                {
                    QLabel *pLabLower1 = new QLabel("Spline degree");
                    QLabel *pLabLower2 = new QLabel("Number of curve points");
                    pLabLower1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    pLabLower2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

                    m_pieOutIntrados = new IntEdit;
                    m_pcbDegIntrados = new QComboBox;

                    QString str;
                    m_pcbDegIntrados->clear();
                    for (int i=2; i<6; i++)
                    {
                        str = QString("%1").arg(i);
                        m_pcbDegIntrados->addItem(str);
                    }

                    m_pcbDegIntrados->setEnabled(true);

                    pLowerLayout->addWidget(pLabLower1,       1,1);
                    pLowerLayout->addWidget(m_pcbDegIntrados, 1,2);
                    pLowerLayout->addWidget(pLabLower2,       2,1);
                    pLowerLayout->addWidget(m_pieOutIntrados, 2,2);
                }

                m_pcptLowerList = new CPTableView(this);
                pLowerSideLayout->addLayout(pLowerLayout);
                pLowerSideLayout->addWidget(m_pcptLowerList);

                //Lower point list
                m_pLowerListModel = new QStandardItemModel(this);
                m_pLowerListModel->setRowCount(10);//temporary
                m_pLowerListModel->setColumnCount(3);

                m_pLowerListModel->setHeaderData(0, Qt::Horizontal, "Point");
                m_pLowerListModel->setHeaderData(1, Qt::Horizontal, "x");
                m_pLowerListModel->setHeaderData(2, Qt::Horizontal, "y");

                m_pcptLowerList->setModel(m_pLowerListModel);

                QHeaderView *pHorizontalHeader = m_pcptLowerList->horizontalHeader();
                pHorizontalHeader->setStretchLastSection(true);

                m_pLowerFloatDelegate = new XflDelegate(this);
                m_pLowerFloatDelegate->setDigits({0,3,3});
                m_pLowerFloatDelegate->setItemTypes({XflDelegate::STRING, XflDelegate::DOUBLE, XflDelegate::DOUBLE});
                m_pcptLowerList->setItemDelegate(m_pLowerFloatDelegate);
             }
            pLowerSideBox->setLayout(pLowerSideLayout);
        }


        pSideLayout->addWidget(pUpperSideBox);
        pSideLayout->addWidget(pLowerSideBox);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGroupBox *pSFParams = new QGroupBox("Parameters");
        {
            QVBoxLayout *pClosedLayout = new QVBoxLayout;
            {
                m_pchSymmetric = new QCheckBox("Symmetric foil");
                m_pchCloseLE   = new QCheckBox("Force closed L.E.");
                m_pchCloseTE   = new QCheckBox("Force closed T.E.");
                pClosedLayout->addWidget(m_pchSymmetric);
                pClosedLayout->addWidget(m_pchCloseLE);
                pClosedLayout->addWidget(m_pchCloseTE);
            }
            pSFParams->setLayout(pClosedLayout);
        }

        QGroupBox *pDisplayBox = new QGroupBox("Display");
        {
            QGridLayout *pDisplayLayout = new QGridLayout;
            {
                QLabel *pLabStyle = new QLabel("Style:");
                pLabStyle->setAlignment(Qt::AlignRight);
                m_plbSplineStyle = new LineBtn;

                pDisplayLayout->addWidget(pLabStyle,        2,1);
                pDisplayLayout->addWidget(m_plbSplineStyle, 2,2);
            }
            pDisplayBox->setLayout(pDisplayLayout);
        }

        pMainLayout->addLayout(pSideLayout);
        pMainLayout->addWidget(pSFParams);
        pMainLayout->addWidget(pDisplayBox);

        setLayout(pMainLayout);
    }
}


void SplineFoilCtrls::connectSignals()
{
    connect(m_pUpperFloatDelegate, SIGNAL(closeEditor(QWidget*)), SLOT(onUpdate()));
    connect(m_pLowerFloatDelegate, SIGNAL(closeEditor(QWidget*)), SLOT(onUpdate()));

    connect(m_pchSymmetric,        SIGNAL(clicked()),              SLOT(onUpdate()));
    connect(m_pchCloseLE,          SIGNAL(clicked()),              SLOT(onUpdate()));
    connect(m_pchCloseTE,          SIGNAL(clicked()),              SLOT(onUpdate()));
    connect(m_pcbDegExtrados,      SIGNAL(activated(int)),         SLOT(onUpdate()));
    connect(m_pcbDegIntrados,      SIGNAL(activated(int)),         SLOT(onUpdate()));
    connect(m_pieOutExtrados,      SIGNAL(intChanged(int)),        SLOT(onUpdate()));
    connect(m_pieOutIntrados,      SIGNAL(intChanged(int)),        SLOT(onUpdate()));

    connect(m_plbSplineStyle,      SIGNAL(clickedLB(LineStyle)),   SLOT(onSplineStyle(LineStyle)));
}


void SplineFoilCtrls::onSplineStyle(LineStyle)
{
    if(!m_pSF) return;
    LineMenu *pLineMenu = new LineMenu(nullptr, true);
    pLineMenu->initMenu(m_pSF->stipple(), m_pSF->width(), m_pSF->color(), m_pSF->pointStyle());
    pLineMenu->exec(QCursor::pos());
    LineStyle ls = pLineMenu->theStyle();
    m_plbSplineStyle->setTheStyle(ls);
    m_pSF->setTheStyle(ls);
    onUpdate();
}


void SplineFoilCtrls::fillPointLists()
{
    m_pUpperListModel->setRowCount(m_pSF->extrados().ctrlPointCount());
    for (int i=0; i<m_pSF->extrados().ctrlPointCount(); i++)
    {
        QModelIndex index = m_pUpperListModel->index(i, 0, QModelIndex());
        m_pUpperListModel->setData(index, i+1);

        QModelIndex Xindex =m_pUpperListModel->index(i, 1, QModelIndex());
        m_pUpperListModel->setData(Xindex, m_pSF->extrados().controlPoint(i).x);

        QModelIndex Zindex =m_pUpperListModel->index(i, 2, QModelIndex());
        m_pUpperListModel->setData(Zindex, m_pSF->extrados().controlPoint(i).y);
    }

    m_pLowerListModel->setRowCount(m_pSF->intrados().ctrlPointCount());
    for (int i=0; i<m_pSF->intrados().ctrlPointCount(); i++)
    {
        QModelIndex index = m_pLowerListModel->index(i, 0, QModelIndex());
        m_pLowerListModel->setData(index, i+1);

        QModelIndex Xindex =m_pLowerListModel->index(i, 1, QModelIndex());
        m_pLowerListModel->setData(Xindex, m_pSF->intrados().controlPoint(i).x);

        QModelIndex Zindex =m_pLowerListModel->index(i, 2, QModelIndex());
        m_pLowerListModel->setData(Zindex, m_pSF->intrados().controlPoint(i).y);
    }
}


void SplineFoilCtrls::readData()
{
    double x=0, y=0;
    for(int i=0; i<m_pSF->extrados().ctrlPointCount(); i++)
    {
        QModelIndex index = m_pUpperListModel->index(i, 1, QModelIndex());
        x = index.data().toDouble();

        index = m_pUpperListModel->index(i, 2, QModelIndex());
        y = index.data().toDouble();
        m_pSF->extrados().setCtrlPoint(i,x,y);
    }
    for (int i=0; i<m_pSF->intrados().ctrlPointCount(); i++)
    {
        QModelIndex index = m_pLowerListModel->index(i, 1, QModelIndex());
        x = index.data().toDouble();

        index = m_pLowerListModel->index(i, 2, QModelIndex());
        y = index.data().toDouble();
        m_pSF->intrados().setCtrlPoint(i,x,y);
    }

    int ideg = m_pcbDegExtrados->currentIndex()+2;
    if(ideg<m_pSF->extrados().ctrlPointCount())
    {
        // there are enough control points for this degree
        m_pSF->extrados().setDegree(ideg);
    }
    else
    {
        // too few control points, adjust the degree
        QMessageBox::warning(this, "Warning", "The spline degree must be less than the number of control points");
        m_pSF->extrados().setDegree(std::max(2,m_pSF->extrados().ctrlPointCount()-1));
        m_pcbDegExtrados->setCurrentIndex(m_pSF->extrados().degree()-2);
    }

    ideg = m_pcbDegIntrados->currentIndex()+2;
    if(ideg<m_pSF->intrados().ctrlPointCount())
    {
        // there are enough control points for this degree
        m_pSF->intrados().setDegree(ideg);
    }
    else
    {
        // too few control points, adjust the degree
        QMessageBox::warning(this, "Warning", "The spline degree must be less than the number of control points");

        m_pSF->intrados().setDegree(std::max(2,m_pSF->intrados().ctrlPointCount()-1));
        m_pcbDegIntrados->setCurrentIndex(m_pSF->intrados().degree()-2);
    }

    m_pSF->extrados().setOutputSize(m_pieOutExtrados->value());
    m_pSF->intrados().setOutputSize(m_pieOutExtrados->value());
    m_pSF->setSymmetric(m_pchSymmetric->isChecked());

    if(m_pSF->isSymmetric())
    {
        m_pSF->intrados().copySymmetric(m_pSF->extrados());
        m_pSF->intrados().makeCurve();
    }

    m_pSF->setClosedLE(m_pchCloseLE->isChecked());
    m_pSF->setClosedTE(m_pchCloseTE->isChecked());
}


void SplineFoilCtrls::setControls()
{
    m_pchSymmetric->setChecked(m_pSF->isSymmetric());
    if(m_pSF->isSymmetric())
    {
        m_pcbDegIntrados->setCurrentIndex(m_pSF->intrados().degree()-2);
        m_pieOutIntrados->setValue(m_pSF->intrados().outputSize());
        fillPointLists();
    }
    m_pcptLowerList->setEnabled(!m_pSF->isSymmetric());
    m_pcbDegIntrados->setEnabled(!m_pSF->isSymmetric());
    m_pieOutIntrados->setEnabled(!m_pSF->isSymmetric());

    m_pchCloseLE->setChecked(m_pSF->bClosedLE());
    m_pchCloseTE->setChecked(m_pSF->bClosedTE());
}


void SplineFoilCtrls::onUpdate()
{
    readData();
    setControls();

    updateSplines();
}


void SplineFoilCtrls::updateSplines()
{
    m_pSF->makeSplineFoil();
    emit splineFoilChanged();
}



