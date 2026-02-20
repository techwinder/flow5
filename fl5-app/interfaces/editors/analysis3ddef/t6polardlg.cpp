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

#define _MATH_DEFINES_DEFINED


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

#include "t6polardlg.h"

#include <api/planepolar.h>
#include <api/planexfl.h>
#include <api/units.h>
#include <api/utils.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/editors/analysis3ddef/extradragwt.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/ctrltabledelegate.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>


T6PolarDlg::T6PolarDlg(QWidget *pParent) : PlanePolarDlg(pParent)
{
    setWindowTitle("Control analysis");

    s_WPolar.setPointStyle(Line::BIGCIRCLE);
    s_WPolar.setLineWidth(2);

    m_pcptOppRange = nullptr;
    m_pOppRangeControlModel = nullptr;

    m_pcptInertia = nullptr;
    m_pInertiaControlModel = nullptr;

    m_pcptAngles = nullptr;
    m_pAngleControlModel = nullptr;

    s_WPolar.setType(xfl::T6POLAR);

    setupLayout();
    connectSignals();
}


T6PolarDlg::~T6PolarDlg()
{
    if(m_pOppRangeCtrlDelegate) delete m_pOppRangeCtrlDelegate;
    if(m_pAngleCtrlDelegate)    delete m_pInertiaCtrlDelegate;
    if(m_pAngleCtrlDelegate)    delete m_pAngleCtrlDelegate;
    m_pOppRangeCtrlDelegate = nullptr;
    m_pInertiaCtrlDelegate  = nullptr;
    m_pAngleCtrlDelegate    = nullptr;
}



void T6PolarDlg::connectSignals()
{
    PlanePolarDlg::connectSignals();

    connect(m_pchAdjustedVelocity,    SIGNAL(clicked(bool)),         SLOT(onAdjustedVelocity()));

    connect(m_pcptOppRange,           SIGNAL(dataPasted()),          SLOT(onRangeCellChanged()));
    connect(m_pcptInertia,            SIGNAL(dataPasted()),          SLOT(onInertiaCellChanged()));
    connect(m_pcptAngles,             SIGNAL(dataPasted()),          SLOT(onAngleCellChanged()));

    connect(m_pOppRangeCtrlDelegate,  SIGNAL(closeEditor(QWidget*)), SLOT(onRangeCellChanged()));
    connect(m_pInertiaCtrlDelegate,   SIGNAL(closeEditor(QWidget*)), SLOT(onInertiaCellChanged()));
    connect(m_pAngleCtrlDelegate,     SIGNAL(closeEditor(QWidget*)), SLOT(onAngleCellChanged()));
}


void T6PolarDlg::fillOppRangePage()
{
    m_pchAdjustedVelocity->setChecked(s_WPolar.isAdjustedVelocity());

    m_pOppRangeControlModel->setRowCount(4); // Vel, aoa, beta, phi

    QModelIndex ind;

    ind = m_pOppRangeControlModel->index(0, 0, QModelIndex()); // Velocity
    ind = m_pOppRangeControlModel->index(1, 0, QModelIndex()); // aoa
    ind = m_pOppRangeControlModel->index(2, 0, QModelIndex()); // beta
    ind = m_pOppRangeControlModel->index(3, 0, QModelIndex()); // phi


    ind = m_pOppRangeControlModel->index(0, 0, QModelIndex());
    m_pOppRangeControlModel->setData(ind, "V"+INFch);
    ind = m_pOppRangeControlModel->index(0, 1, QModelIndex());
    m_pOppRangeControlModel->setData(ind, s_WPolar.m_OperatingRange.at(0).ctrlMin()*Units::mstoUnit());
    ind = m_pOppRangeControlModel->index(0, 2, QModelIndex());
    m_pOppRangeControlModel->setData(ind, s_WPolar.m_OperatingRange.at(0).ctrlMax()*Units::mstoUnit());
    ind = m_pOppRangeControlModel->index(0, 3, QModelIndex());
    m_pOppRangeControlModel->setData(ind, Units::speedUnitQLabel());

    ind = m_pOppRangeControlModel->index(1, 0, QModelIndex());
    m_pOppRangeControlModel->setData(ind, ALPHAch);
    ind = m_pOppRangeControlModel->index(1, 1, QModelIndex());
    m_pOppRangeControlModel->setData(ind, s_WPolar.m_OperatingRange.at(1).ctrlMin());
    ind = m_pOppRangeControlModel->index(1, 2, QModelIndex());
    m_pOppRangeControlModel->setData(ind, s_WPolar.m_OperatingRange.at(1).ctrlMax());
    ind = m_pOppRangeControlModel->index(1, 3, QModelIndex());
    m_pOppRangeControlModel->setData(ind, DEGch);

    ind = m_pOppRangeControlModel->index(2, 0, QModelIndex());
    m_pOppRangeControlModel->setData(ind, BETAch);
    ind = m_pOppRangeControlModel->index(2, 1, QModelIndex());
    m_pOppRangeControlModel->setData(ind, s_WPolar.m_OperatingRange.at(2).ctrlMin());
    ind = m_pOppRangeControlModel->index(2, 2, QModelIndex());
    m_pOppRangeControlModel->setData(ind, s_WPolar.m_OperatingRange.at(2).ctrlMax());
    ind = m_pOppRangeControlModel->index(2, 3, QModelIndex());
    m_pOppRangeControlModel->setData(ind, DEGch);

    ind = m_pOppRangeControlModel->index(3, 0, QModelIndex());
    m_pOppRangeControlModel->setData(ind, PHIch);
    ind = m_pOppRangeControlModel->index(3, 1, QModelIndex());
    m_pOppRangeControlModel->setData(ind, s_WPolar.m_OperatingRange.at(3).ctrlMin());
    ind = m_pOppRangeControlModel->index(3, 2, QModelIndex());
    m_pOppRangeControlModel->setData(ind, s_WPolar.m_OperatingRange.at(3).ctrlMax());
    ind = m_pOppRangeControlModel->index(3, 3, QModelIndex());
    m_pOppRangeControlModel->setData(ind, DEGch);
}


void T6PolarDlg::fillInertiaPage()
{
    QString strLen, strMass, strInertia;
    strLen     = Units::lengthUnitQLabel();
    strMass    = Units::massUnitQLabel();
    strInertia = Units::inertiaUnitQLabel();

    if(m_pPlane)
    {
        if(s_WPolar.bAutoInertia())
        {
            s_WPolar.setMass(m_pPlane->totalMass());
            s_WPolar.setCoG(m_pPlane->CoG_t());
            s_WPolar.setIxx(m_pPlane->Ixx_t());
            s_WPolar.setIyy(m_pPlane->Iyy_t());
            s_WPolar.setIzz(m_pPlane->Izz_t());
            s_WPolar.setIxz(m_pPlane->Ixz_t());
        }
    }
    else
    {
        s_WPolar.setMass(0);
        s_WPolar.setCoG(Vector3d());
        s_WPolar.setIxx(0);
        s_WPolar.setIyy(0);
        s_WPolar.setIzz(0);
        s_WPolar.setIxz(0);
    }

    m_pInertiaControlModel->setRowCount(3);

    QModelIndex ind;

/*    ind = m_pInertiaControlModel->index(0, 0, QModelIndex()); // mass
    ind = m_pInertiaControlModel->index(1, 0, QModelIndex()); // x_CoG
    ind = m_pInertiaControlModel->index(2, 0, QModelIndex()); // z_CoG
    ind = m_pInertiaControlModel->index(3, 0, QModelIndex()); // Ixx
    ind = m_pInertiaControlModel->index(4, 0, QModelIndex()); // Iyy
    ind = m_pInertiaControlModel->index(5, 0, QModelIndex()); // Izz
    ind = m_pInertiaControlModel->index(6, 0, QModelIndex()); // Ixz */

    ind = m_pInertiaControlModel->index(0, 0, QModelIndex());
    m_pInertiaControlModel->setData(ind, "Mass");
    ind = m_pInertiaControlModel->index(0, 1, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_WPolar.m_InertiaRange.at(0).ctrlMin()*Units::kgtoUnit());
    ind = m_pInertiaControlModel->index(0, 2, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_WPolar.m_InertiaRange.at(0).ctrlMax()*Units::kgtoUnit());
    ind = m_pInertiaControlModel->index(0, 3, QModelIndex());
    m_pInertiaControlModel->setData(ind, strMass);

    ind = m_pInertiaControlModel->index(1, 0, QModelIndex());
    m_pInertiaControlModel->setData(ind, "CoG_x");
    ind = m_pInertiaControlModel->index(1, 1, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_WPolar.m_InertiaRange.at(1).ctrlMin()*Units::mtoUnit());
    ind = m_pInertiaControlModel->index(1, 2, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_WPolar.m_InertiaRange.at(1).ctrlMax()*Units::mtoUnit());
    ind = m_pInertiaControlModel->index(1, 3, QModelIndex());
    m_pInertiaControlModel->setData(ind, strLen);

    ind = m_pInertiaControlModel->index(2, 0, QModelIndex());
    m_pInertiaControlModel->setData(ind, "CoG_z");
    ind = m_pInertiaControlModel->index(2, 1, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_WPolar.m_InertiaRange.at(2).ctrlMin()*Units::mtoUnit());
    ind = m_pInertiaControlModel->index(2, 2, QModelIndex());
    m_pInertiaControlModel->setData(ind, s_WPolar.m_InertiaRange.at(2).ctrlMax()*Units::mtoUnit());
    ind = m_pInertiaControlModel->index(2, 3, QModelIndex());
    m_pInertiaControlModel->setData(ind, strLen);
}


void T6PolarDlg::resizeColumns()
{
    PlanePolarDlg::resizeColumns();

    double wr = double(m_pcptOppRange->width())*.93;
    int wCols  = int(wr/4.0);
    m_pcptOppRange->setColumnWidth(0, wCols);
    m_pcptOppRange->setColumnWidth(1, wCols);
    m_pcptOppRange->setColumnWidth(2, wCols);

    double wi = double(m_pcptInertia->width())*.93;
    wCols  = int(wi/4.0);
    m_pcptInertia->setColumnWidth(0, wCols);
    m_pcptInertia->setColumnWidth(1, wCols);
    m_pcptInertia->setColumnWidth(2, wCols);

    double wc = double(m_pcptAngles->width())*.93;
    wCols  = int(wc/4.0);
    m_pcptAngles->setColumnWidth(0, wCols);
    m_pcptAngles->setColumnWidth(1, wCols);
    m_pcptAngles->setColumnWidth(2, wCols);
    //    m_pAngleControlTable->setColumnWidth(3, wCols);

//qDebug()<<"wwwwidths"<< wr << wi << wc;
}


void T6PolarDlg::setViscous()
{
}


void T6PolarDlg::initPolar3dDlg(Plane const *pPlane, PlanePolar const *pWPolar)
{
    if(s_WPolar.m_OperatingRange.size()<4) s_WPolar.m_OperatingRange.resize(4); // cleaning up old mistakes - first

    PlanePolarDlg::initPolar3dDlg(pPlane, pWPolar);
    s_WPolar.setType(xfl::T6POLAR);

    if(s_WPolar.isLLTMethod()) s_WPolar.setVLM2();
    checkMethods();

    if(pWPolar)
    {
//        m_bAutoName = false;
//        m_plePolarName->setText(QString::fromStdString(pWPolar->name()));
        s_WPolar.duplicateSpec(pWPolar);
        s_WPolar.setName(pWPolar->name());
        s_WPolar.setPlaneName(pWPolar->planeName());
    }
    else
    {
//        m_bAutoName = true;
        s_WPolar.clearAngleRangeList();
        if(s_WPolar.bAutoInertia()) s_WPolar.retrieveInertia(m_pPlane);
    }

//    m_pchAutoName->setChecked(m_bAutoName);
//    m_plePolarName->setEnabled(!m_bAutoName);

    if(s_WPolar.m_OperatingRange.size()<4) s_WPolar.m_OperatingRange.resize(4); // cleaning up old mistakes

    if(pWPolar)
    {
        s_WPolar.m_AngleRange = pWPolar->m_AngleRange;
    }
    else
    {
        s_WPolar.resetAngleRanges(m_pPlane);
    }

    fillOppRangePage();
    fillAngleControlList();
    fillInertiaPage();

    setPolar3dName();

    m_pcptAngles->setFocus();

    enableControls();
}


void T6PolarDlg::onRangeCellChanged()
{
    readOperatingData();
    setPolar3dName();
}


void T6PolarDlg::onInertiaCellChanged()
{
    readInertiaData();
    setPolar3dName();
}


void T6PolarDlg::onAngleCellChanged()
{
    readAngleRangeData();
    setPolar3dName();
}


void T6PolarDlg::onEditingFinished()
{
    readData();
    setPolar3dName();
    enableControls();
}


void T6PolarDlg::onOK()
{
    readData();
    if(!checkWPolarData()) return;
    accept();
}


void T6PolarDlg::readAngleRangeData()
{
    if(! m_pPlane || !m_pPlane->isXflType()) return;
    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);

    s_WPolar.clearAngleRangeList();
    int iCtrl=0;
    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        s_WPolar.m_AngleRange.push_back({});
        s_WPolar.m_AngleRange.back().push_back({m_pAngleControlModel->index(iCtrl, 0, QModelIndex()).data().toString().toStdString(),
                                                m_pAngleControlModel->index(iCtrl, 1, QModelIndex()).data().toDouble(),
                                                m_pAngleControlModel->index(iCtrl, 2, QModelIndex()).data().toDouble()});
        iCtrl++;
        for(int ic=0; ic<pPlaneXfl->wingAt(iw)->nFlaps(); ic++)
        {
            s_WPolar.m_AngleRange.back().push_back({m_pAngleControlModel->index(iCtrl, 0, QModelIndex()).data().toString().toStdString(),
                                                    m_pAngleControlModel->index(iCtrl, 1, QModelIndex()).data().toDouble(),
                                                    m_pAngleControlModel->index(iCtrl, 2, QModelIndex()).data().toDouble()});
            iCtrl++;
        }
    }

    setViscous();
}


void T6PolarDlg::readOperatingData()
{
    double d=0;
    QString strange;
    s_WPolar.m_OperatingRange.resize(4);

    // Velocity min-max
    strange = m_pOppRangeControlModel->index(0, 0, QModelIndex()).data().toString();
    s_WPolar.m_OperatingRange[0].setName(strange.toStdString());
    d = m_pOppRangeControlModel->index(0, 1, QModelIndex()).data().toDouble() / Units::mstoUnit();
    s_WPolar.m_OperatingRange[0].setCtrlMin(d);
    d = m_pOppRangeControlModel->index(0, 2, QModelIndex()).data().toDouble() / Units::mstoUnit();
    s_WPolar.m_OperatingRange[0].setCtrlMax(d);

    // aoa min-max
    strange = m_pOppRangeControlModel->index(1, 0, QModelIndex()).data().toString();
    s_WPolar.m_OperatingRange[1].setName(strange.toStdString());
    d = m_pOppRangeControlModel->index(1, 1, QModelIndex()).data().toDouble();
    s_WPolar.m_OperatingRange[1].setCtrlMin(d);
    d = m_pOppRangeControlModel->index(1, 2, QModelIndex()).data().toDouble();
    s_WPolar.m_OperatingRange[1].setCtrlMax(d);

    // sideslip min-max
    strange = m_pOppRangeControlModel->index(2, 0, QModelIndex()).data().toString();
    s_WPolar.m_OperatingRange[2].setName(strange.toStdString());
    d = m_pOppRangeControlModel->index(2, 1, QModelIndex()).data().toDouble();
    s_WPolar.m_OperatingRange[2].setCtrlMin(d);
    d = m_pOppRangeControlModel->index(2, 2, QModelIndex()).data().toDouble() ;
    s_WPolar.m_OperatingRange[2].setCtrlMax(d);

    // Bank min-max
    strange = m_pOppRangeControlModel->index(3, 0, QModelIndex()).data().toString();
    s_WPolar.m_OperatingRange[3].setName(strange.toStdString());
    d = m_pOppRangeControlModel->index(3, 1, QModelIndex()).data().toDouble();
    s_WPolar.m_OperatingRange[3].setCtrlMin(d);
    d = m_pOppRangeControlModel->index(3, 2, QModelIndex()).data().toDouble();
    s_WPolar.m_OperatingRange[3].setCtrlMax(d);
}


void T6PolarDlg::readInertiaData()
{
    double d=0;
    QString strange;

    // mass min-max
    strange = m_pInertiaControlModel->index(0, 0, QModelIndex()).data().toString();
    s_WPolar.m_InertiaRange[0].setName(strange.toStdString());
    d = m_pInertiaControlModel->index(0, 1, QModelIndex()).data().toDouble() / Units::kgtoUnit();
    s_WPolar.m_InertiaRange[0].setCtrlMin(d);
    d = m_pInertiaControlModel->index(0, 2, QModelIndex()).data().toDouble() / Units::kgtoUnit();
    s_WPolar.m_InertiaRange[0].setCtrlMax(d);

    //CoG.x min-max
    strange = m_pInertiaControlModel->index(1, 0, QModelIndex()).data().toString();
    s_WPolar.m_InertiaRange[1].setName(strange.toStdString());
    d = m_pInertiaControlModel->index(1, 1, QModelIndex()).data().toDouble() / Units::mtoUnit();
    s_WPolar.m_InertiaRange[1].setCtrlMin(d);
    d = m_pInertiaControlModel->index(1, 2, QModelIndex()).data().toDouble() / Units::mtoUnit();
    s_WPolar.m_InertiaRange[1].setCtrlMax(d);

    //CoG.z min-max
    strange = m_pInertiaControlModel->index(2, 0, QModelIndex()).data().toString();
    s_WPolar.m_InertiaRange[2].setName(strange.toStdString());
    d = m_pInertiaControlModel->index(2, 1, QModelIndex()).data().toDouble() / Units::mtoUnit();
    s_WPolar.m_InertiaRange[2].setCtrlMin(d);
    d = m_pInertiaControlModel->index(2, 2, QModelIndex()).data().toDouble() / Units::mtoUnit();
    s_WPolar.m_InertiaRange[2].setCtrlMax(d);

    //for(int i=0; i<s_WPolar.m_InertiaRange.size(); i++)    qDebug("inange %d=  %7.3g  %7.3g", i, s_WPolar.m_InertiaRange.at(i).ctrlMin(), s_WPolar.m_InertiaRange.at(i).ctrlMax());
}


void T6PolarDlg::readData()
{
    PlanePolarDlg::readData();

    if(s_WPolar.isVLM()) s_WPolar.setThinSurfaces(true);

    readOperatingData();
    readInertiaData();
    readAngleRangeData();

    s_WPolar.setAdjustedVelocity(m_pchAdjustedVelocity->isChecked());
}


void T6PolarDlg::setupLayout()
{
    QString strLen, strMass, strInertia, strArea;
    strArea = Units::areaUnitQLabel();
    strLen = Units::lengthUnitQLabel();
    strMass = Units::massUnitQLabel();
    strInertia = strMass+"."+strLen+QString::fromUtf8("²");

    QWidget *pOppRangePage = new QWidget(this);
    {
        QVBoxLayout *pOppRangePageLayout  = new QVBoxLayout;
        {
            m_pcptOppRange = new CPTableView(this);
            m_pcptOppRange->setEditable(true);
            m_pcptOppRange->setWindowTitle(tr("Operating range"));
//            m_pcptOppRange->setMinimumWidth(fm.averageCharWidth() * 53);
//            m_pcptOppRange->setMinimumHeight(fm.height() * 10);

            m_pOppRangeControlModel = new CtrlTableModel(this);
            m_pOppRangeControlModel->setRowCount(4);
            m_pOppRangeControlModel->setColumnCount(4);
            m_pOppRangeControlModel->setHeaderData(0, Qt::Horizontal, tr("Operating parameter"));
            m_pOppRangeControlModel->setHeaderData(1, Qt::Horizontal, tr("Min."));
            m_pOppRangeControlModel->setHeaderData(2, Qt::Horizontal, tr("Max."));
            m_pOppRangeControlModel->setHeaderData(3, Qt::Horizontal, tr("Unit"));

            m_pcptOppRange->setModel(m_pOppRangeControlModel);
            m_pcptOppRange->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
            m_pcptOppRange->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

            m_pOppRangeCtrlDelegate = new CtrlTableDelegate(this);
            m_pcptOppRange->setItemDelegate(m_pOppRangeCtrlDelegate);

            m_pOppRangeCtrlDelegate->setPrecision({2,3,3,-1});
            m_pOppRangeCtrlDelegate->setEditable({false, true, true, false});

            QHBoxLayout *pVhLayout = new QHBoxLayout;
            {
                m_pchAdjustedVelocity = new QCheckBox();
                QPixmap pixmap;
                if(DisplayOptions::isDarkMode())
                    pixmap.load(":/images/V_h_inv.png");
                else
                    pixmap.load(":/images/V_h.png");
                QLabel * pLabVh = new QLabel();
                pLabVh->setPixmap(pixmap);
                pLabVh->setAlignment(Qt::AlignLeft |Qt::AlignVCenter);
                pVhLayout->addWidget(m_pchAdjustedVelocity);
                pVhLayout->addWidget(pLabVh);
                pVhLayout->addStretch();
            }

            QString tip = tr("<p>Check this option if the velocity is to be adjusted to balance the weight.<br>"
                          "In this case, the velocity parameter defined in the table below will be ignored.<br>"
                          "This is the equivalent of a Type 2 fixed lift polar.</p>");
            m_pchAdjustedVelocity->setToolTip(tip);

            QLabel* pParamLabel = new QLabel();
            QPixmap pixmap;
            if(DisplayOptions::isDarkMode())
                pixmap.load(":/images/param_inv.png");
            else
                pixmap.load(":/images/param.png");
            pParamLabel->setPixmap(pixmap);
            pParamLabel->setAlignment(Qt::AlignLeft |Qt::AlignVCenter);

            pOppRangePageLayout->addLayout(pVhLayout);
            pOppRangePageLayout->addWidget(m_pcptOppRange);
            pOppRangePageLayout->addWidget(pParamLabel);
            pOppRangePage->setLayout(pOppRangePageLayout);
        }
    }

    QWidget *pMassControlPage  = new QWidget(this);
    {
        QVBoxLayout *pMassControlPageLayout  = new QVBoxLayout;
        {
            m_pcptInertia = new CPTableView(this);
            m_pcptInertia->setEditable(true);
            m_pcptInertia->setWindowTitle(tr("Controls"));
//            m_pcptInertia->setMinimumWidth(400);
//            m_pcptInertia->setMinimumHeight(150);
            //        m_pInertiaControlTable->horizontalHeader()->setStretchLastSection(true);

            m_pInertiaControlModel = new CtrlTableModel(this);
            m_pInertiaControlModel->setRowCount(3);
            m_pInertiaControlModel->setColumnCount(4);
            m_pInertiaControlModel->setHeaderData(0, Qt::Horizontal, tr("Inertia parameter"));
            m_pInertiaControlModel->setHeaderData(1, Qt::Horizontal, tr("Min."));
            m_pInertiaControlModel->setHeaderData(2, Qt::Horizontal, tr("Max."));
            m_pInertiaControlModel->setHeaderData(3, Qt::Horizontal, tr("Unit"));

            m_pcptInertia->setModel(m_pInertiaControlModel);
            m_pcptInertia->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
            m_pcptInertia->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

            m_pInertiaCtrlDelegate = new CtrlTableDelegate(this);
            m_pcptInertia->setItemDelegate(m_pInertiaCtrlDelegate);
            m_pInertiaCtrlDelegate->setPrecision({2,3,3,-1});
            m_pInertiaCtrlDelegate->setEditable({false, true, true, false});

            QLabel* pParamLabel = new QLabel();
            QPixmap pixmap;
            if(DisplayOptions::isDarkMode())
                pixmap.load(":/images/param_inv.png");
            else
                pixmap.load(":/images/param.png");
            pParamLabel->setPixmap(pixmap);
            pParamLabel->setAlignment(Qt::AlignLeft |Qt::AlignVCenter);

            pMassControlPageLayout->addWidget(m_pchAutoInertia);
            pMassControlPageLayout->addWidget(m_pcptInertia);
            pMassControlPageLayout->addWidget(pParamLabel);

            pMassControlPage->setLayout(pMassControlPageLayout);
        }
    }

    QWidget *pAngleControlPage = new QWidget(this);
    {
        QVBoxLayout *pAngleControlPageLayout  = new QVBoxLayout;
        {
            m_pcptAngles = new CPTableView(this);
            m_pcptAngles->setEditable(true);

            m_pcptAngles->setWindowTitle(tr("Controls"));
//            m_pcptAngles->setMinimumWidth(400);
//            m_pcptAngles->setMinimumHeight(150);
            //        m_pAngleControlTable->horizontalHeader()->setStretchLastSection(true);

            m_pAngleControlModel = new CtrlTableModel(this);
            m_pAngleControlModel->setRowCount(10);//temporary
            m_pAngleControlModel->setColumnCount(4);
            m_pAngleControlModel->setHeaderData(0, Qt::Horizontal, tr("Surface"));
            m_pAngleControlModel->setHeaderData(1, Qt::Horizontal, tr("Min."));
            m_pAngleControlModel->setHeaderData(2, Qt::Horizontal, tr("Max."));
            m_pAngleControlModel->setHeaderData(3, Qt::Horizontal, tr("Unit"));

            m_pcptAngles->setModel(m_pAngleControlModel);
            m_pcptAngles->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
            m_pcptAngles->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

            m_pAngleCtrlDelegate = new CtrlTableDelegate(this);
            m_pcptAngles->setItemDelegate(m_pAngleCtrlDelegate);


            m_pAngleCtrlDelegate->setPrecision({2,3,3,-1}); m_pAngleCtrlDelegate->setEditable({false, true, true, false});

            QLabel* pNotes = new QLabel(tr("Notes:"));
            QLabel* pSignLabel = new QLabel(tr("(1)\t+ sign means trailing edge down"));

            QHBoxLayout *pAngleParamLayout = new QHBoxLayout;
            {
                QLabel* pAngleLabel = new QLabel(tr("(2)\tThe angles are ADDED to the preset wing and flap angles:"));
                pAngleLabel->setAlignment(Qt::AlignLeft |Qt::AlignVCenter);
                QLabel* pParamLabel = new QLabel();
                QPixmap pixmap;
                if(DisplayOptions::isDarkMode())
                    pixmap.load(":/images/control_angles_inv.png");
                else
                    pixmap.load(":/images/control_angles.png");
                pParamLabel->setPixmap(pixmap);
                pParamLabel->setAlignment(Qt::AlignLeft |Qt::AlignVCenter);

                pAngleParamLayout->addWidget(pAngleLabel);
                pAngleParamLayout->addWidget(pParamLabel);
                pAngleParamLayout->addStretch();
            }

            QLabel* pOriginLabel = new QLabel(tr("(3)\tThe center of rotations are the wings leading point at the root chord"));
            QLabel* pNumberLabel = new QLabel(tr("(4)\tFlaps are numbered from left tip to right tip"));

            pAngleControlPageLayout->addWidget(m_pcptAngles);
            pAngleControlPageLayout->addWidget(pNotes);
            pAngleControlPageLayout->addWidget(pSignLabel);
            pAngleControlPageLayout->addLayout(pAngleParamLayout);
            pAngleControlPageLayout->addWidget(pOriginLabel);
            pAngleControlPageLayout->addWidget(pNumberLabel);
            pAngleControlPage->setLayout(pAngleControlPageLayout);
        }
    }

    QTabWidget *pTabWt = new QTabWidget(this);
    {
        pTabWt->setMovable(true);
        pTabWt->addTab(m_pfrMethod,       tr("Method"));
        pTabWt->addTab(m_pfrRefDims,      tr("Ref. dimensions"));
        pTabWt->addTab(m_pfrFluid,        tr("Fluid"));
        pTabWt->addTab(m_pfrViscosity,    tr("Viscosity"));
        pTabWt->addTab(pOppRangePage,     tr("Operating range"));
        pTabWt->addTab(pMassControlPage,  tr("Mass and inertia"));
        pTabWt->addTab(pAngleControlPage, tr("Angles"));
        pTabWt->addTab(m_pfrGround,       tr("Ground"));
        pTabWt->addTab(m_pfrFuseDrag,     tr("Fuselage"));
        pTabWt->addTab(m_pExtraDragWt,    tr("Extra drag"));
        pTabWt->addTab(m_pfrWake,         tr("Wake"));
        connect(pTabWt, &QTabWidget::currentChanged, this, &T6PolarDlg::onTabChanged);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    {
        pMainLayout->addWidget(m_pfrPolarName);
        pMainLayout->addWidget(pTabWt);
        pMainLayout->addWidget(m_pButtonBox);
    }

    setLayout(pMainLayout);
}


void T6PolarDlg::enableControls()
{
    PlanePolarDlg::enableControls();

    m_prbLLTMethod->setEnabled(false);

    m_prbVLM1Method->setEnabled(!s_WPolar.isBetaPolar() && fabs(s_WPolar.betaSpec())<PRECISION);

    if(m_prbVLM1Method->isChecked() || m_prbVLM2Method->isChecked())
    {
        m_prbThinSurfaces->setEnabled(false);
        m_prbThickSurfaces->setEnabled(false);
        m_prbThinSurfaces->setChecked(false);
    }
/*    else
    {
        m_prbThinSurfaces->setEnabled(true);
        m_prbThickSurfaces->setEnabled(true);
    }*/

    m_pcptInertia->setEnabled(!s_WPolar.bAutoInertia());
    m_pcptOppRange->setRowHidden(0, m_pchAdjustedVelocity->isChecked());
}


void T6PolarDlg::fillAngleControlList()
{
    if(!m_pPlane || !m_pPlane->isXflType()) return;
    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);

    m_pAngleControlModel->setRowCount(s_WPolar.nAngleRangeCtrls());//temporary
    QString  strong;
    QString strdeg = DEGch;
    QModelIndex ind;

    int nctrls = 0;
    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);
        ind = m_pAngleControlModel->index(nctrls, 0, QModelIndex());
        strong = QString::fromStdString(pWing->name()) + " Tilt";
        m_pAngleControlModel->setData(ind, strong);

        ind = m_pAngleControlModel->index(nctrls, 1, QModelIndex());
        m_pAngleControlModel->setData(ind, s_WPolar.angleRange(iw, 0).ctrlMin());

        ind = m_pAngleControlModel->index(nctrls, 2, QModelIndex());
        m_pAngleControlModel->setData(ind, s_WPolar.angleRange(iw, 0).ctrlMax());

        ind = m_pAngleControlModel->index(nctrls, 3, QModelIndex());
        m_pAngleControlModel->setData(ind, strdeg);

        nctrls++;

        int iFlapCtrl=1;
        for(int iFlap=0; iFlap<pWing->nFlaps(); iFlap++)
        {
            ind = m_pAngleControlModel->index(iFlap+nctrls, 0, QModelIndex());
            strong = QString::fromStdString(pWing->name()) + " " + QString("Flap %1 ").arg(iFlapCtrl);
            m_pAngleControlModel->setData(ind, strong);

            ind = m_pAngleControlModel->index(iFlap+nctrls, 1, QModelIndex());
            m_pAngleControlModel->setData(ind, s_WPolar.angleRange(iw, iFlapCtrl).ctrlMin());

            ind = m_pAngleControlModel->index(iFlap+nctrls, 2, QModelIndex());
            m_pAngleControlModel->setData(ind, s_WPolar.angleRange(iw, iFlapCtrl).ctrlMax());

            ind = m_pAngleControlModel->index(iFlap+nctrls, 3, QModelIndex());
            m_pAngleControlModel->setData(ind, strdeg);

            iFlapCtrl++;
        }
        nctrls += pWing->nFlaps();
    }
}


void T6PolarDlg::onAdjustedVelocity()
{
    s_WPolar.setAdjustedVelocity(m_pchAdjustedVelocity->isChecked());
    m_pcptOppRange->setRowHidden(0, m_pchAdjustedVelocity->isChecked());
    setPolar3dName();
}


void T6PolarDlg::onPlaneInertia()
{
    s_WPolar.setAutoInertia(m_pchAutoInertia->isChecked());
    if(s_WPolar.bAutoInertia())
    {
        s_WPolar.retrieveInertia(m_pPlane);
        fillInertiaPage();
    }
    else
    {
    }

    enableControls();

    setPolar3dName();
}
