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
#include <QMessageBox>
#include <QVBoxLayout>

#include "btpolardlg.h"


#include <api/boat.h>
#include <api/boatpolar.h>
#include <api/sail.h>
#include <api/units.h>
#include <api/utils.h>

#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/editors/analysis3ddef/aerodatadlg.h>
#include <interfaces/editors/analysis3ddef/btpolarautonamedlg.h>
#include <interfaces/editors/analysis3ddef/btpolarnamemaker.h>
#include <interfaces/editors/analysis3ddef/extradragwt.h>
#include <interfaces/graphs/containers/splinedgraphwt.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/graphs/graph/graph.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/xfldelegate.h>


BoatPolar BtPolarDlg::s_BtPolar;

BtPolarDlg::BtPolarDlg(QWidget *pParent) : Polar3dDlg(pParent)
{
    m_pBoat     = nullptr;
    m_BtPolarName = tr("SailPolar Name");

    makeBaseCommonControls();
    makeTables();
    setupLayout();

    connectSignals();
}


BtPolarDlg::~BtPolarDlg()
{
    delete m_pBtVariableDelegate;

    if(m_pWindGraphWt->graph())
    {
        m_pWindGraphWt->graph()->deleteCurveModel();
        delete m_pWindGraphWt->graph();
    }
}


void BtPolarDlg::makeTables()
{
    QString strLength = Units::lengthUnitQLabel();

    //--------- VARIABLE TABLE -----------
    m_pcptBtVariable = new CPTableView(this);
    m_pcptBtVariable->setEditable(true);
    m_pcptBtVariable->setWindowTitle(tr("Wind gradient"));
    m_pcptBtVariable->setCharSize(50,15);

    //Create the model associated to the table of variables
    m_pBtVariableModel = new QStandardItemModel(this);
    m_pBtVariableModel->setRowCount(5);//temporary
    m_pBtVariableModel->setColumnCount(4);
    m_pBtVariableModel->setHeaderData(0, Qt::Horizontal, tr("Design Variable"));
    m_pBtVariableModel->setHeaderData(1, Qt::Horizontal, tr("Min."));
    m_pBtVariableModel->setHeaderData(2, Qt::Horizontal, tr("Max."));
    m_pBtVariableModel->setHeaderData(3, Qt::Horizontal, tr("Unit"));
    m_pcptBtVariable->setModel(m_pBtVariableModel);

    QHeaderView *pHorizontalHeader = m_pcptBtVariable->horizontalHeader();
    pHorizontalHeader->setStretchLastSection(true);

    m_pBtVariableDelegate = new XflDelegate(this);
    m_pcptBtVariable->setItemDelegate(m_pBtVariableDelegate);
    m_pBtVariableDelegate->setActionColumn(-1);
    m_pBtVariableDelegate->setCheckColumn(-1);

    m_pBtVariableDelegate->setDigits({-1,3,3,-1});
    m_pBtVariableDelegate->setItemTypes({XflDelegate::STRING, XflDelegate::DOUBLE, XflDelegate::DOUBLE,XflDelegate::STRING});

    QItemSelectionModel *pSelectionModel = new QItemSelectionModel(m_pBtVariableModel);
    m_pcptBtVariable->setSelectionModel(pSelectionModel);

}


/** resets all default values in the dialog */
void BtPolarDlg::onReset()
{
    s_BtPolar.setDefaultSpec(m_pBoat);
    initDialog(m_pBoat, &s_BtPolar);
    m_bAutoName = true;
    setPolar3dName();
}


void BtPolarDlg::connectSignals()
{
    connectBaseSignals();

    connect(m_pTabWidget,          SIGNAL(currentChanged(int)),  SLOT(onTabChanged(int)));
    connect(m_pBtVariableDelegate, SIGNAL(commitData(QWidget*)), SLOT(onBtVariableChanged()));
    connect(m_pchUseSailDim,       SIGNAL(clicked(bool)),        SLOT(onRefDims()));
}


void BtPolarDlg::onRefDims()
{
    m_pfeRefArea->setEnabled(!m_pchUseSailDim->isChecked());
    m_pfeRefChord->setEnabled(!m_pchUseSailDim->isChecked());

    s_BtPolar.setAutoRefDims(m_pchUseSailDim->isChecked());
    if(s_BtPolar.bAutoRefDims())
    {
        if(m_pBoat && m_pBoat->hasSail())
        {
            Sail const*pSail = m_pBoat->sailAt(0);
            s_BtPolar.setReferenceArea(pSail->refArea());
            s_BtPolar.setReferenceChordLength(pSail->refChord());
            m_pfeRefArea->setValue(pSail->refArea()*Units::m2toUnit());
            m_pfeRefChord->setValue(pSail->refChord()*Units::mtoUnit());
        }
    }
}


void BtPolarDlg::readFluidProperties()
{
    s_BtPolar.setDensity(m_pfeDensity->value() / Units::densitytoUnit());
    s_BtPolar.setViscosity(m_pfeViscosity->value() / Units::viscositytoUnit());
}


void BtPolarDlg::fillVariableList()
{
    //fill the sail list
//    m_pBtVariableModel->blockSignals(true);
    QModelIndex ind;

    int globalvars = 4;
    m_pBtVariableModel->setRowCount(globalvars+m_pBoat->nSails()*2);

    // row 0 is Boat Speed range
    int row = 0;
    ind = m_pBtVariableModel->index(row, 0, QModelIndex());
    m_pBtVariableModel->setData(ind, tr("Boat speed"));
    ind = m_pBtVariableModel->index(row, 1, QModelIndex());
    m_pBtVariableModel->setData(ind, s_BtPolar.m_VBtMin*Units::mstoUnit());
    ind = m_pBtVariableModel->index(row, 2, QModelIndex());
    m_pBtVariableModel->setData(ind, s_BtPolar.m_VBtMax*Units::mstoUnit());
    ind = m_pBtVariableModel->index(row, 3, QModelIndex());
    m_pBtVariableModel->setData(ind, Units::speedUnitQLabel());

    // row 1 is AWS range
    row++;
    ind = m_pBtVariableModel->index(row, 0, QModelIndex());
    m_pBtVariableModel->setData(ind, tr("True Wind Speed (TWS)"));
    ind = m_pBtVariableModel->index(row, 1, QModelIndex());
    m_pBtVariableModel->setData(ind, s_BtPolar.m_TWSMin*Units::mstoUnit());
    ind = m_pBtVariableModel->index(row, 2, QModelIndex());
    m_pBtVariableModel->setData(ind, s_BtPolar.m_TWSMax*Units::mstoUnit());
    ind = m_pBtVariableModel->index(row, 3, QModelIndex());
    m_pBtVariableModel->setData(ind, Units::speedUnitQLabel());

    // row 2 is AWA range
    row++;
    ind = m_pBtVariableModel->index(row, 0, QModelIndex());
    m_pBtVariableModel->setData(ind, tr("True Wind Angle (TWA)"));
    ind = m_pBtVariableModel->index(row, 1, QModelIndex());
    m_pBtVariableModel->setData(ind, s_BtPolar.m_TWAMin);
    ind = m_pBtVariableModel->index(row, 2, QModelIndex());
    m_pBtVariableModel->setData(ind, s_BtPolar.m_TWAMax);
    ind = m_pBtVariableModel->index(row, 3, QModelIndex());
    m_pBtVariableModel->setData(ind, DEGch);

    // row 3 is the bank angle range
    row++;
    ind = m_pBtVariableModel->index(row, 0, QModelIndex());
    m_pBtVariableModel->setData(ind, tr("Heeling angle"));
    ind = m_pBtVariableModel->index(row, 1, QModelIndex());
    m_pBtVariableModel->setData(ind, s_BtPolar.m_PhiMin);
    ind = m_pBtVariableModel->index(row, 2, QModelIndex());
    m_pBtVariableModel->setData(ind, s_BtPolar.m_PhiMax);
    ind = m_pBtVariableModel->index(row, 3, QModelIndex());
    m_pBtVariableModel->setData(ind, DEGch);

    // row 4 is the range of Ry
    row++;
    ind = m_pBtVariableModel->index(row, 0, QModelIndex());
    m_pBtVariableModel->setData(ind, tr("Ry"));
    m_pBtVariableModel->setData(ind, tr("<p>Defines the rotation around the Y axis.<br>"
                                     "Typically of use in the case of windsurf sails</p>"), Qt::ToolTipRole);
    ind = m_pBtVariableModel->index(row, 1, QModelIndex());
    m_pBtVariableModel->setData(ind, s_BtPolar.RyMin());
    m_pBtVariableModel->setData(ind, tr("<p>Defines the minimum rotation around the Y axis</p>"), Qt::ToolTipRole);
    ind = m_pBtVariableModel->index(row, 2, QModelIndex());
    m_pBtVariableModel->setData(ind, s_BtPolar.RyMax());
    m_pBtVariableModel->setData(ind, tr("<p>Defines the maximum rotation around the Y axis</p>"), Qt::ToolTipRole);
    ind = m_pBtVariableModel->index(row, 3, QModelIndex());
    m_pBtVariableModel->setData(ind, DEGch);

    // the next set of rows are the sail angles
    for(int is=0; is<m_pBoat->nSails(); is++)
    {
        row++;
        Sail const *pSail = m_pBoat->sailAt(is);
        ind = m_pBtVariableModel->index(row, 0, QModelIndex());
        m_pBtVariableModel->setData(ind, QString::fromStdString(pSail->name())+tr(" angle"));
        m_pBtVariableModel->setData(ind, tr("<p>Defines the sail's rotation around its luff axis</p>"), Qt::ToolTipRole);
        ind = m_pBtVariableModel->index(row, 1, QModelIndex());
        m_pBtVariableModel->setData(ind, s_BtPolar.m_SailAngleMin[is]);
        m_pBtVariableModel->setData(ind, tr("<p>Defines the sail's min. rotation around its luff axis</p>"), Qt::ToolTipRole);
        ind = m_pBtVariableModel->index(row, 2, QModelIndex());
        m_pBtVariableModel->setData(ind, s_BtPolar.m_SailAngleMax[is]);
        m_pBtVariableModel->setData(ind, tr("<p>Defines the sail's max. rotation around its luff axis</p>"), Qt::ToolTipRole);
        ind = m_pBtVariableModel->index(row, 3, QModelIndex());
        m_pBtVariableModel->setData(ind, DEGch);
    }
//    m_pBtVariableModel->blockSignals(false);
}


void BtPolarDlg::readData()
{
    m_BtPolarName = m_plePolarName->text();

    s_BtPolar.setIgnoreBodyPanels(!m_pchIncludeHull->isChecked());

    s_BtPolar.m_TotalWakeLengthFactor = m_pfeWakeLength->value() /Units::mtoUnit();

    double x     = m_pdeXCoG->value() / Units::mtoUnit();
    double y     = m_pdeYCoG->value() / Units::mtoUnit();
    double z     = m_pdeZCoG->value() / Units::mtoUnit();
    s_BtPolar.setCoG(Vector3d(x,y,z));

    s_BtPolar.setViscous(false);
    s_BtPolar.setGroundEffect(true);

    readBtVariables();
    readWindGradient();

    readMethodData();
    readWakeData(s_BtPolar);
    setVPWUnits(s_BtPolar);

    readFluidProperties();

    readReferenceDimensions();

    std::vector<ExtraDrag> extra;
    m_pExtraDragWt->setExtraDragData(extra);
    s_BtPolar.setExtraDrag(extra);

    s_BtPolar.setWindSpline(m_pWindGraphWt->spline());
}


void BtPolarDlg::readMethodData()
{
    if (m_prbLLTMethod->isChecked())
    {
        s_BtPolar.setAnalysisMethod(xfl::LLT);
    }
    else if (m_prbVLM1Method->isChecked())
    {
        s_BtPolar.setVLM1();
        s_BtPolar.setIgnoreBodyPanels(true);
    }
    else if (m_prbVLM2Method->isChecked())
    {
        s_BtPolar.setVLM2();
        s_BtPolar.setIgnoreBodyPanels(true);
    }
    else if (m_prbQuadMethod->isChecked())
    {
        s_BtPolar.setAnalysisMethod(xfl::QUADS);
    }
    else if (m_prbTriUniMethod->isChecked())
    {
        s_BtPolar.setAnalysisMethod(xfl::TRIUNIFORM);
    }
    else if (m_prbTriLinMethod->isChecked())
    {
        s_BtPolar.setAnalysisMethod(xfl::TRILINEAR);
    }

    s_BtPolar.setTrefftz(true);
    s_BtPolar.setBoundaryCondition(xfl::DIRICHLET);
}

void BtPolarDlg::readReferenceDimensions()
{
    s_BtPolar.setAutoRefDims(m_pchUseSailDim->isChecked());
    if(s_BtPolar.bAutoRefDims())
    {
        if(m_pBoat && m_pBoat->hasSail())
        {
            s_BtPolar.setReferenceArea(m_pBoat->sailAt(0)->refArea());
            s_BtPolar.setReferenceChordLength(m_pBoat->sailAt(0)->refChord());
        }
        else
        {
            // not sure about this
            s_BtPolar.setReferenceArea(m_pfeRefArea->value()/Units::m2toUnit());
            s_BtPolar.setReferenceChordLength(m_pfeRefChord->value()/Units::mtoUnit());
        }
    }
    else
    {
        s_BtPolar.setReferenceArea(m_pfeRefArea->value()/Units::m2toUnit());
        s_BtPolar.setReferenceChordLength(m_pfeRefChord->value()/Units::mtoUnit());
    }
}


void BtPolarDlg::readBtVariables()
{
    QModelIndex index;
    int row = 0;

    // row 0 is the Boat Speed range
    index = m_pBtVariableModel->index(row, 1 , QModelIndex());
    s_BtPolar.m_VBtMin = index.data().toDouble()/Units::mstoUnit();
    index = m_pBtVariableModel->index(row, 2, QModelIndex());
    s_BtPolar.m_VBtMax = index.data().toDouble()/Units::mstoUnit();

    // row 1 is the TWS range
    row++;
    index = m_pBtVariableModel->index(row, 1 , QModelIndex());
    s_BtPolar.m_TWSMin = index.data().toDouble()/Units::mstoUnit();
    index = m_pBtVariableModel->index(row, 2, QModelIndex());
    s_BtPolar.m_TWSMax = index.data().toDouble()/Units::mstoUnit();

    // row 2 is the TWA range
    row++;
    index = m_pBtVariableModel->index(row, 1, QModelIndex());
    s_BtPolar.m_TWAMin = index.data().toDouble();
    index = m_pBtVariableModel->index(row, 2, QModelIndex());
    s_BtPolar.m_TWAMax = index.data().toDouble();

    // row 3 is the bank angle range
    row++;
    index = m_pBtVariableModel->index(row, 1, QModelIndex());
    s_BtPolar.m_PhiMin = index.data().toDouble();
    index = m_pBtVariableModel->index(row, 2, QModelIndex());
    s_BtPolar.m_PhiMax = index.data().toDouble();

    // row 4 is the bank angle range
    row++;
    index = m_pBtVariableModel->index(row, 1, QModelIndex());
    s_BtPolar.setRyMin(index.data().toDouble());
    index = m_pBtVariableModel->index(row, 2, QModelIndex());
    s_BtPolar.setRyMax(index.data().toDouble());

    // the next set of rows are the sail angles
    for(int is=0; is<m_pBoat->nSails(); is++)
    {
        row++;
        index = m_pBtVariableModel->index(row,1, QModelIndex());
        s_BtPolar.m_SailAngleMin[is] = index.data().toDouble();
        index = m_pBtVariableModel->index(row,2, QModelIndex());
        s_BtPolar.m_SailAngleMax[is] = index.data().toDouble();
    }
}


void BtPolarDlg::readWindGradient()
{
    s_BtPolar.setWindSpline(m_pWindGraphWt->spline());
}


void BtPolarDlg::initDialog(Boat const *pBoat, BoatPolar *pBtPolar)
{
    m_pBoat=pBoat;

    if(pBoat)
        m_plabParentObjectName->setText(QString::fromStdString(pBoat->name()));

    if(pBtPolar)
    {
        m_bAutoName = false;
        m_plePolarName->setText(QString::fromStdString(pBtPolar->name()));
        if(int(pBtPolar->m_SailAngleMax.size())!=m_pBoat->nSails())
            pBtPolar->m_SailAngleMax.resize(m_pBoat->nSails());
        if(int(pBtPolar->m_SailAngleMin.size())!=m_pBoat->nSails())
            pBtPolar->m_SailAngleMin.resize(m_pBoat->nSails());
        s_BtPolar.duplicateSpec(pBtPolar);
    }
    else
    {
        m_bAutoName = true;
        setPolar3dName();
        s_BtPolar.m_SailAngleMin.resize(m_pBoat->nSails());
        s_BtPolar.m_SailAngleMax.resize(m_pBoat->nSails());
    }
    m_pchAutoName->setChecked(m_bAutoName);

    m_pdeXCoG->setValue(s_BtPolar.CoG().x*Units::mtoUnit());
    m_pdeYCoG->setValue(s_BtPolar.CoG().y*Units::mtoUnit());
    m_pdeZCoG->setValue(s_BtPolar.CoG().z*Units::mtoUnit());

    m_pchUseSailDim->setChecked(s_BtPolar.bAutoRefDims());
    m_pfeRefArea->setValue(s_BtPolar.referenceArea()*Units::m2toUnit());
    m_pfeRefChord->setValue(s_BtPolar.referenceChordLength()*Units::mtoUnit());
    onRefDims();

    //fill the wind gradient
    m_pWindGraphWt->setAutoConvert(true);
    BSpline &spline = m_pWindGraphWt->spline();
    {
        spline.duplicate(s_BtPolar.m_WindSpline);
        if(m_pWindGraphWt->graph())
        {
            m_pWindGraphWt->graph()->addCurve(tr("Wind gradient"));
            spline.setColor(m_pWindGraphWt->graph()->curve(0)->fl5Clr());
        }
        spline.setStipple(Line::NOLINE);
        spline.updateSpline();
        spline.makeCurve();
        m_pWindGraphWt->convertSpline();
        m_pWindGraphWt->graph()->resetLimits();
        m_pWindGraphWt->update();
    }

    fillVariableList();
    m_pcptBtVariable->setFocus();

    //initialize base widgets

    if(s_BtPolar.isVLM())
    {
        m_prbVLM1Method->setChecked( s_BtPolar.isVLM1());
        m_prbVLM2Method->setChecked(!s_BtPolar.isVLM1());
    }
    else if(s_BtPolar.isPanel4Method())
    {
        m_prbQuadMethod->setChecked(true);
    }
    else if(s_BtPolar.isTriUniformMethod())
    {
        m_prbTriUniMethod->setChecked(true);
    }
    else if(s_BtPolar.isTriLinearMethod())
    {
        m_prbTriLinMethod->setChecked(true);
    }

    m_pfeDensity->setValue(s_BtPolar.density()*Units::densitytoUnit());
    m_pfeViscosity->setValue(s_BtPolar.viscosity()*Units::viscositytoUnit());

    m_pExtraDragWt->initWt(s_BtPolar.extraDragList());

    // wake data
    s_BtPolar.setNXWakePanel4(std::max(s_BtPolar.NXWakePanel4(),1));
    s_BtPolar.setTotalWakeLengthFactor(std::max(s_BtPolar.totalWakeLengthFactor(),1.0));
    s_BtPolar.setWakePanelFactor(std::max(s_BtPolar.wakePanelFactor(),1.0));

    m_prbPanelWake->setChecked(!s_BtPolar.bVortonWake());
    m_prbVortonWake->setChecked(s_BtPolar.bVortonWake());
    m_pieNXWakePanels->setValue(s_BtPolar.NXWakePanel4());
    m_pfeWakeLength->setValue(s_BtPolar.totalWakeLengthFactor());
    m_pfeWakePanelFactor->setValue(s_BtPolar.wakePanelFactor());

    m_pfeVPWBufferWake->setValue(s_BtPolar.bufferWakeFactor());
    m_pfeVPWLength->setValue(s_BtPolar.VPWMaxLength());
    m_pfeVortonCoreSize->setValue(s_BtPolar.vortonCoreSize());
    m_pfeVortonL0->setValue(s_BtPolar.vortonL0());
    m_pieVPWIterations->setValue(s_BtPolar.VPWIterations());
    setVPWUnits(s_BtPolar);

    m_pgbFlatWakePanels->setEnabled(!s_BtPolar.bVortonWake());
    m_pgbVortonWake->setEnabled(s_BtPolar.bVortonWake());

    m_pgbWingSurf->setVisible(false);
    m_pgbFuseMi->setVisible(false);
    m_pgbHullBox->setVisible(true);
    m_pchIncludeHull->setChecked(!s_BtPolar.bIgnoreBodyPanels());

    enableControls();
}


void BtPolarDlg::onOK()
{
    readData();

    if(fabs(s_BtPolar.referenceArea())<1.e-4 || fabs(s_BtPolar.referenceChordLength())<1.e-4)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Reference dimensions cannot be null."));
        m_pTabWidget->setCurrentIndex(2);
        return;
    }
    accept();
}


void BtPolarDlg::setupLayout()
{
    m_prbLLTMethod->setVisible(false);

    QFrame *pVariableFrame = new QFrame;
    {
        QVBoxLayout *pVariableLayout = new QVBoxLayout;
        {
            QLabel* pParamLabel = new QLabel();
            QPixmap pixmap;
            if(DisplayOptions::isDarkMode())
                pixmap.load(":/images/param_inv.png");
            else
                pixmap.load(":/images/param.png");
            pParamLabel->setPixmap(pixmap);
            pParamLabel->setAlignment(Qt::AlignLeft |Qt::AlignVCenter);
            pVariableLayout->addWidget(m_pcptBtVariable);
            pVariableLayout->addWidget(pParamLabel);
        }
        pVariableFrame->setLayout(pVariableLayout);
    }

    QFrame *pRefDimFrame = new QFrame;
    {
        QGridLayout *pRefDimLayout = new QGridLayout;
        {
            m_pchUseSailDim = new QCheckBox(tr("Use main sail reference dimensions"));
            m_pfeRefArea = new FloatEdit;
            m_pfeRefChord = new FloatEdit;
            QLabel *pLabRefArea    = new QLabel(tr("Reference area:"));
            QLabel *pLabAreaUnit   = new QLabel(Units::areaUnitQLabel());
            QLabel *pLabRefChord   = new QLabel(tr("Reference chord:"));
            QLabel *pLabLengthUnit = new QLabel(Units::lengthUnitQLabel());

            pRefDimLayout->addWidget(m_pchUseSailDim, 1,1,1,2);
            pRefDimLayout->addWidget(pLabRefArea,     2, 1, Qt::AlignVCenter | Qt::AlignRight);
            pRefDimLayout->addWidget(m_pfeRefArea,    2, 2);
            pRefDimLayout->addWidget(pLabAreaUnit,    2, 3);
            pRefDimLayout->addWidget(pLabRefChord,    3, 1, Qt::AlignVCenter | Qt::AlignRight);
            pRefDimLayout->addWidget(m_pfeRefChord,   3, 2);
            pRefDimLayout->addWidget(pLabLengthUnit,  3, 3);

            pRefDimLayout->setRowStretch(4,1);
            pRefDimLayout->setColumnStretch(4,1);
        }
        pRefDimFrame->setLayout(pRefDimLayout);
    }

    QFrame *pInertiaFrame = new QFrame;
    {
        QGridLayout *pInertiaDataLayout = new QGridLayout;
        {

            QLabel *plab3 = new QLabel(tr("X_CoG ="));
            QLabel *plab4 = new QLabel(tr("Y_CoG ="));
            QLabel *plab5 = new QLabel(tr("Z_CoG ="));
            plab3->setAlignment(Qt::AlignRight | Qt::AlignCenter);
            plab4->setAlignment(Qt::AlignRight | Qt::AlignCenter);
            plab5->setAlignment(Qt::AlignRight | Qt::AlignCenter);
            m_pdeXCoG  = new FloatEdit(100.00,3);
            m_pdeYCoG  = new FloatEdit(100.00,3);
            m_pdeZCoG  = new FloatEdit(100.00,3);
            QLabel *plabLengthUnit1 = new QLabel(Units::lengthUnitQLabel());
            QLabel *plabLengthUnit2 = new QLabel(Units::lengthUnitQLabel());
            QLabel *plabLengthUnit3 = new QLabel(Units::lengthUnitQLabel());

            pInertiaDataLayout->addWidget(plab3,2,1);
            pInertiaDataLayout->addWidget(plab4,3,1);
            pInertiaDataLayout->addWidget(plab5,4,1);
            pInertiaDataLayout->addWidget(m_pdeXCoG,2,2);
            pInertiaDataLayout->addWidget(m_pdeYCoG,3,2);
            pInertiaDataLayout->addWidget(m_pdeZCoG,4,2);
            pInertiaDataLayout->addWidget(plabLengthUnit1,2,3);
            pInertiaDataLayout->addWidget(plabLengthUnit2,3,3);
            pInertiaDataLayout->addWidget(plabLengthUnit3,4,3);
            pInertiaDataLayout->setRowStretch(1,1);
            pInertiaDataLayout->setRowStretch(5,1);
            pInertiaDataLayout->setColumnStretch(1,1);
            pInertiaDataLayout->setColumnStretch(3,2);
        }
        pInertiaFrame->setLayout(pInertiaDataLayout);
    }

    QFrame *pAeroData = new QFrame;
    {
        QHBoxLayout *pAeroLayout = new QHBoxLayout;
        {
            QGroupBox *pWindGradientBox = new QGroupBox(tr("Wind gradient"));
            {
                QVBoxLayout *pWindGradientLayout = new QVBoxLayout;
                {
                    m_pWindGraphWt = new SplinedGraphWt;
                    {
                        Graph *pGraph = new Graph;
                        pGraph->setCurveModel(new CurveModel);
                        m_pWindGraphWt->setGraph(pGraph);
                        GraphOptions::resetGraphSettings(*pGraph);
                        pGraph->setLegendVisible(false);
                        pGraph->setLegendPosition(Qt::AlignHCenter | Qt::AlignTop);
                        pGraph->setScaleType(GRAPH::RESETTING);
                        pGraph->setAuto(true);
                        pGraph->setXVariableList({tr("Wind factor")});
                        pGraph->setYVariableList({tr("Height (m)")});

                        m_pWindGraphWt->setXLimits(0.0, 1.0);
                        m_pWindGraphWt->setYLimits(0.0, 1.e10);
                    }
                    pWindGradientLayout->addWidget(m_pWindGraphWt);
                }
                pWindGradientBox->setLayout(pWindGradientLayout);
            }
            pAeroLayout->addWidget(m_pfrFluid);
            pAeroLayout->addWidget(pWindGradientBox);
            pAeroLayout->setStretchFactor(m_pfrFluid, 1);
            pAeroLayout->setStretchFactor(pWindGradientBox, 5);
        }
        pAeroData->setLayout(pAeroLayout);
    }

    m_pTabWidget = new QTabWidget;
    m_pTabWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_pTabWidget->addTab(m_pfrMethod,         tr("Method"));
    m_pTabWidget->addTab(pVariableFrame,      tr("Variables"));
    m_pTabWidget->addTab(pRefDimFrame,        tr("Ref. dimensions"));
    m_pTabWidget->addTab(pInertiaFrame,       tr("Inertia"));
    m_pTabWidget->addTab(pAeroData,           tr("Air"));
    m_pTabWidget->addTab(m_pExtraDragWt,      tr("Extra drag"));
    m_pTabWidget->addTab(m_pfrWake,           tr("Wake"));

    m_pTabWidget->setCurrentIndex(0);

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    {
        pMainLayout->addWidget(m_pfrPolarName);
        pMainLayout->addWidget(m_pTabWidget);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
    adjustSize();

    m_prbThinSurfaces->setVisible(false);
    m_plabWakeLengthLabUnit->setText(Units::lengthUnitQLabel());
}


void BtPolarDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)     onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}


void BtPolarDlg::resizeEvent(QResizeEvent *)
{
    resizeColumns();
}


void BtPolarDlg::resizeColumns()
{
    double w = double(m_pcptBtVariable->width())*.93;
    int wCols  = int(w/6);
    m_pcptBtVariable->setColumnWidth(0, wCols*3);
    m_pcptBtVariable->setColumnWidth(1, wCols);
    m_pcptBtVariable->setColumnWidth(2, wCols);
    m_pcptBtVariable->setColumnWidth(3, wCols);
}


void BtPolarDlg::onAeroData()
{
    AeroDataDlg dlg;
    if(dlg.exec() == QDialog::Accepted)
    {
        s_BtPolar.setDensity(dlg.airDensity());
        s_BtPolar.setViscosity(dlg.kinematicViscosity());

        m_pfeViscosity->setValue(s_BtPolar.viscosity() * Units::densitytoUnit());
        m_pfeDensity->setValue(  s_BtPolar.density()   * Units::viscositytoUnit());
    }
}


void BtPolarDlg::checkMethods()
{
    if(s_BtPolar.isVLM())
    {
        m_prbVLM1Method->setChecked( s_BtPolar.isVLM1());
        m_prbVLM2Method->setChecked(!s_BtPolar.isVLM1());
    }
    else if(s_BtPolar.isPanel4Method())
    {
        m_prbQuadMethod->setChecked(true);
    }
    else if(s_BtPolar.isTriUniformMethod())
    {
        m_prbTriUniMethod->setChecked(true);
    }
    else if(s_BtPolar.isTriLinearMethod())
    {
        m_prbTriLinMethod->setChecked(true);
    }
}


void BtPolarDlg::onEditingFinished()
{
    readData();
    setPolar3dName();
    enableControls();
}


void BtPolarDlg::enableControls()
{
    if(s_BtPolar.isViscous()) s_BtPolar.setViscous(false);

    m_prbThinSurfaces->setChecked(false);
    m_prbThickSurfaces->setChecked(true);
    m_prbThinSurfaces->setEnabled(false);
    m_prbThickSurfaces->setEnabled(false);

    m_prbLLTMethod->setEnabled(false);
    m_prbQuadMethod->setEnabled(false);
    m_prbVLM1Method->setEnabled(false);
    m_prbVLM2Method->setEnabled(false);

    m_plePolarName->setEnabled(!m_pchAutoName->isChecked());

    m_pgbFlatWakePanels->setEnabled(!s_BtPolar.bVortonWake());
    m_pgbVortonWake->setEnabled(s_BtPolar.bVortonWake());
}


void BtPolarDlg::setPolar3dName()
{
    if(!m_bAutoName) return;
    s_BtPolar.setName(BtPolarNameMaker::makeName(m_pBoat, &s_BtPolar).toStdString());
    m_plePolarName->setText(QString::fromStdString(s_BtPolar.name()));
}


void BtPolarDlg::onVortonWake()
{
    s_BtPolar.setVortonWake(m_prbVortonWake->isChecked());
    m_pgbFlatWakePanels->setEnabled(!s_BtPolar.bVortonWake());
    m_pgbVortonWake->setEnabled(s_BtPolar.bVortonWake());
    setPolar3dName();
}


void BtPolarDlg::onNameOptions()
{
    BtPolarAutoNameDlg dlg;
    dlg.initDialog(*m_pBoat, s_BtPolar);
    dlg.exec();
    setPolar3dName();
}


void BtPolarDlg::onMethod()
{
    readMethodData();
//    m_prbVortonWake->setEnabled(false);

    enableControls();
    setPolar3dName();
}


void BtPolarDlg::onBtVariableChanged()
{
    readBtVariables();
    setPolar3dName();
}



