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
#include <QHeaderView>
#include <QPalette>

#include "extradragwt.h"

#include <api/planepolar.h>
#include <api/units.h>
#include <interfaces/graphs/containers/splinedgraphwt.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/graphs/graph/graph.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/xfldelegate.h>

ExtraDragWt::ExtraDragWt(QWidget *pParent) : QWidget(pParent)
{
    m_bChanged = false;
    setupLayout();
}


ExtraDragWt::~ExtraDragWt()
{
    delete m_pExtraDragDelegate;
    m_pExtraDragDelegate = nullptr;

    if(m_pParabolicGraphWt->graph())
    {
        m_pParabolicGraphWt->graph()->deleteCurveModel();
        delete m_pParabolicGraphWt->graph();
    }
}


void ExtraDragWt::setupLayout()
{
    QVBoxLayout *pExtraDragPageLayout  = new QVBoxLayout;
    {
        QHBoxLayout *pDragsLayout = new QHBoxLayout;
        {
            QGroupBox *pExtraBox = new QGroupBox(tr("Constant coef. drag"));
            {
                QVBoxLayout *pExtraLayout = new QVBoxLayout;
                {
                    m_pcptExtraDragTable = new CPTableView(this);
                    m_pcptExtraDragTable->setEditable(true);
                    m_pcptExtraDragTable->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

                    m_pcptExtraDragTable->setWindowTitle(tr("Extra drag"));

                    m_pExtraDragModel = new QStandardItemModel(this);
                    m_pExtraDragModel->setColumnCount(3);
                    m_pExtraDragModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
                    m_pExtraDragModel->setHeaderData(1, Qt::Horizontal, tr("Area")+" ("+Units::areaUnitQLabel()+")");
                    m_pExtraDragModel->setHeaderData(2, Qt::Horizontal, tr("Drag coef."));
                    m_pcptExtraDragTable->setModel(m_pExtraDragModel);
                    m_pcptExtraDragTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
                    m_pcptExtraDragTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
                    m_pcptExtraDragTable->horizontalHeader()->setStretchLastSection(true);

                    m_pExtraDragDelegate = new XflDelegate(this);
                    m_pcptExtraDragTable->setItemDelegate(m_pExtraDragDelegate);

                    m_pExtraDragDelegate->setNCols(3, XflDelegate::DOUBLE);
                    m_pExtraDragDelegate->setDigits({0,3,5});
                    m_pExtraDragDelegate->setItemType(0, XflDelegate::STRING);

                    pExtraLayout->addWidget(m_pcptExtraDragTable);
                }
                pExtraBox->setLayout(pExtraLayout);
            }
            QVBoxLayout *pAVLDragLayout = new QVBoxLayout;
            {
                m_pchAVLDrag = new QCheckBox(tr("AVL type parabolic drag"));

                m_pParabolicGraphWt = new SplinedGraphWt;
                {
                    Graph *pGraph = new Graph;
                    pGraph->setCurveModel(new CurveModel);
                    m_pParabolicGraphWt->setGraph(pGraph);
                    GraphOptions::resetGraphSettings(*pGraph);
                    pGraph->setLegendVisible(true);
                    pGraph->setLegendPosition(Qt::AlignHCenter | Qt::AlignTop);
                    pGraph->setScaleType(GRAPH::RESETTING);
                    pGraph->setAuto(true);
                    pGraph->setXVariableList(QStringList({"CDv"}));
                    pGraph->setYVariableList(QStringList({"CL"}));
                }
                pAVLDragLayout->addWidget(m_pchAVLDrag);
                pAVLDragLayout->addWidget(m_pParabolicGraphWt);
            }

            pDragsLayout->addWidget(pExtraBox);
            pDragsLayout->addLayout(pAVLDragLayout);
            pDragsLayout->setStretchFactor(pExtraBox, 1);
            pDragsLayout->setStretchFactor(pAVLDragLayout, 2);
        }

        QLabel* plabExtraDrag = new QLabel();
        QPixmap pixmap;


        QColor back = this->palette().window().color();
        if(back.valueF()<0.5f)
            pixmap.load(":/images/extra_drag_inv.png");
        else
            pixmap.load(":/images/extra_drag.png");
        plabExtraDrag->setPixmap(pixmap);

        pExtraDragPageLayout->addLayout(pDragsLayout);
        pExtraDragPageLayout->addWidget(plabExtraDrag);
    }
    setLayout(pExtraDragPageLayout);
    connect(m_pchAVLDrag, SIGNAL(toggled(bool)), m_pParabolicGraphWt, SLOT(setEnabled(bool)));
    //    connect(m_pExtraDragControlModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onExtraDragChanged(QModelIndex,QModelIndex)));
    connect(m_pcptExtraDragTable, SIGNAL(dataPasted()), SLOT(onExtraDragChanged()));
    connect(m_pExtraDragDelegate, SIGNAL(closeEditor(QWidget*)), SLOT(onExtraDragChanged()));
}


void ExtraDragWt::onExtraDragChanged()
{
    m_bChanged = true;
    readExtraDragData();
    m_pExtraDragModel->setRowCount(int(m_ExtraDrag.size())+1);
}


void ExtraDragWt::initWt(std::vector<ExtraDrag> const &extradrag)
{
    m_ExtraDrag = extradrag;
    m_bAVLDrag = false;

    m_pParabolicGraphWt->setEnabled(false);
    m_pchAVLDrag->setEnabled(false);

    fillExtraDrag();
}


void ExtraDragWt::initWt(PlanePolar const *pWPolar)
{
    m_ExtraDrag = pWPolar->extraDragList();

    m_bAVLDrag = pWPolar->bAVLDrag();
    m_AVLSpline = pWPolar->AVLSpline();
    m_pParabolicGraphWt->graph()->addCurve(tr("Parabolic drag"));
    m_pParabolicGraphWt->setAutoConvert(true);
    fillExtraDrag();
}


void ExtraDragWt::fillExtraDrag()
{
    m_pExtraDragModel->setRowCount(int(m_ExtraDrag.size())+1);
//    QString str = Units::lengthUnitLabel();
    QModelIndex ind;

    for(uint i=0; i<m_ExtraDrag.size(); i++)
    {
        ind = m_pExtraDragModel->index(i, 0, QModelIndex());
        if(ind.isValid())
            m_pExtraDragModel->setData(ind, QString::fromStdString(m_ExtraDrag.at(i).tag()));
        ind = m_pExtraDragModel->index(i, 1, QModelIndex());
        if(ind.isValid())
        {
            m_pExtraDragModel->setData(ind, m_ExtraDrag.at(i).area()*Units::m2toUnit());
        }
        ind = m_pExtraDragModel->index(i, 2, QModelIndex());
        if(ind.isValid())
        {
            m_pExtraDragModel->setData(ind, m_ExtraDrag.at(i).coef());
        }
    }

    m_pParabolicGraphWt->setEnabled(m_bAVLDrag);
    Spline &spline = m_pParabolicGraphWt->spline();
    {
        spline.duplicate(m_AVLSpline);
        if(m_pParabolicGraphWt->graph() && m_pParabolicGraphWt->graph()->hasCurve())
            spline.setColor(m_pParabolicGraphWt->graph()->curve(0)->fl5Clr());
        spline.setStipple(Line::NOLINE);
        spline.updateSpline();
        spline.makeCurve();
    }

    m_pParabolicGraphWt->convertSpline();
    m_pParabolicGraphWt->graph()->resetLimits();
    m_pParabolicGraphWt->update();

    m_pchAVLDrag->setChecked(m_bAVLDrag);
}


void ExtraDragWt::readExtraDragData()
{
    m_ExtraDrag.clear();
    for(int i=0; i<m_pExtraDragModel->rowCount(); i++)
    {
        QString strange = m_pExtraDragModel->index(i, 0, QModelIndex()).data().toString();
        double area = m_pExtraDragModel->index(i, 1, QModelIndex()).data().toDouble()/Units::m2toUnit();
        double coef = m_pExtraDragModel->index(i, 2, QModelIndex()).data().toDouble();
        if(strange.length()>0 || fabs(area)>PRECISION || fabs(coef)>PRECISION)
        {
            m_ExtraDrag.push_back({strange.toStdString(), area, coef});
        }
    }

    m_bAVLDrag = m_pchAVLDrag->isChecked();
    m_AVLSpline.duplicate(m_pParabolicGraphWt->spline());
}


void ExtraDragWt::setExtraDragData(std::vector<ExtraDrag> &extra)
{
    readExtraDragData();
    extra = m_ExtraDrag;
}


void ExtraDragWt::setExtraDragData(PlanePolar &pWPolar)
{
    readExtraDragData();
    pWPolar.setExtraDrag(m_ExtraDrag);
    pWPolar.setAVLDrag(m_bAVLDrag, m_AVLSpline);
}





