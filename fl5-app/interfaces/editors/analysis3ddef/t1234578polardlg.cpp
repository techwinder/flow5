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



#include <QApplication>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QMessageBox>
#include <QMenu>

#include <api/objects3d.h>
#include <api/planexfl.h>
#include <api/units.h>
#include <api/utils.h>
#include <api/planepolar.h>

#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/editors/analysis3ddef/ctrltablemodel.h>
#include <interfaces/editors/analysis3ddef/extradragwt.h>
#include <interfaces/editors/analysis3ddef/t1234578polardlg.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/ctrltabledelegate.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>


T1234578PolarDlg::T1234578PolarDlg(QWidget *pParent) : PlanePolarDlg(pParent)
{
    setWindowTitle(tr("Analysis definition"));

    m_pPlane = nullptr;

    setupLayout();
}


void T1234578PolarDlg::connectSignals()
{
    PlanePolarDlg::connectSignals();

    connect(m_prbType1,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));
    connect(m_prbType2,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));
    connect(m_prbType3,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));
    connect(m_prbType4,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));
    connect(m_prbType5,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));
    connect(m_prbType7,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));
    connect(m_prbType8,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));

    connect(m_pfeXCoG,         SIGNAL(floatChanged(float)),  SLOT(onEditingFinished()));
    connect(m_pfeZCoG,         SIGNAL(floatChanged(float)),  SLOT(onEditingFinished()));

    connect(m_pfePlaneMass,    SIGNAL(floatChanged(float)),  SLOT(onEditingFinished()));
    connect(m_pfeQInf,         SIGNAL(floatChanged(float)),  SLOT(onEditingFinished()));
    connect(m_pfeAlphaSpec,    SIGNAL(floatChanged(float)),  SLOT(onEditingFinished()));
    connect(m_pfePhiSpec,      SIGNAL(floatChanged(float)),  SLOT(onEditingFinished()));

    connect(m_pcptAVLCtrls->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onAVLRowChanged(QModelIndex)));
    connect(m_pcptAVLCtrls,                   SIGNAL(customContextMenuRequested(QPoint)),         SLOT(onAVLContextMenu(QPoint)));
    connect(m_pAVLCtrlModel,                  SIGNAL(dataChanged(QModelIndex,QModelIndex)),       SLOT(onAVLCtrlChanged()));
    connect(m_pcptAVLGains,                   SIGNAL(dataPasted()),                               SLOT(onAVLGainChanged()));
    connect(m_pAVLGainDelegate,               SIGNAL(closeEditor(QWidget*)),                      SLOT(onAVLGainChanged()));
}


void T1234578PolarDlg::enableControls()
{
    PlanePolarDlg::enableControls();

//    m_prbType4->setEnabled(false);

    switch (s_WPolar.type())
    {
        default:
        case xfl::T1POLAR:
        {
            m_pfrAlpha->setVisible(    false);
            m_pfrQInf->setVisible(     true);
            m_plabGlide->setVisible(   false);
            m_plabVh->setVisible(      false);
            m_frFlightInfo->setVisible(true);
            break;
        }
        case xfl::T2POLAR:
        case xfl::T7POLAR:
        {
            m_pfrAlpha->setVisible(    false);
            m_pfrQInf->setVisible(     false);
            m_plabGlide->setVisible(   false);
            m_plabVh->setVisible(      true);
            m_frFlightInfo->setVisible(true);
            break;
        }
        case xfl::T3POLAR:
        {
            m_pfrAlpha->setVisible( false);
            m_pfrQInf->setVisible(  false);
            m_plabGlide->setVisible(true);
            m_plabVh->setVisible(   false);
            m_frFlightInfo->setVisible(false);
            break;
        }
        case xfl::T4POLAR:
        {
            m_pfrAlpha->setVisible(    true);
            m_pfrQInf->setVisible(     false);
            m_plabGlide->setVisible(   false);
            m_plabVh->setVisible(      false);
            m_frFlightInfo->setVisible(false);
            break;
        }
        case xfl::T5POLAR:
        {
            m_pfrAlpha->setVisible(    true);
            m_pfrQInf->setVisible(     true);
            m_plabGlide->setVisible(   false);
            m_plabVh->setVisible(      false);
            m_frFlightInfo->setVisible(false);
            break;
        }
        case xfl::T8POLAR:
        {
            m_pfrAlpha->setVisible(    false);
            m_pfrQInf->setVisible(     false);
            m_plabGlide->setVisible(   false);
            m_plabVh->setVisible(      false);
            m_frFlightInfo->setVisible(false);
            break;
        }
    }

    m_pchViscAnalysis->setEnabled(s_WPolar.isPanelMethod());

    m_pfePlaneMass->setEnabled(!s_WPolar.bAutoInertia());
    m_pfeXCoG->setEnabled(     !s_WPolar.bAutoInertia());
    m_pfeZCoG->setEnabled(     !s_WPolar.bAutoInertia());
    m_pfeIxx->setEnabled(      !s_WPolar.bAutoInertia());
    m_pfeIyy->setEnabled(      !s_WPolar.bAutoInertia());
    m_pfeIzz->setEnabled(      !s_WPolar.bAutoInertia());
    m_pfeIxz->setEnabled(      !s_WPolar.bAutoInertia());

    if(s_WPolar.isBetaPolar())    m_prbVLM1Method->setEnabled(false);
}


void T1234578PolarDlg::initPolar3dDlg(const Plane *pPlane, const PlanePolar *pWPolar)
{
    PlanePolarDlg::initPolar3dDlg(pPlane, pWPolar);

    if(pWPolar && pWPolar->isType6())
        s_WPolar.setType(xfl::T1POLAR);

    if(s_WPolar.isType6()) s_WPolar.setType(xfl::T1POLAR);

    switch(s_WPolar.type())
    {
        default:
        case xfl::T1POLAR:   m_prbType1->setChecked(true);   break;
        case xfl::T2POLAR:   m_prbType2->setChecked(true);   break;
        case xfl::T3POLAR:   m_prbType3->setChecked(true);   break;
        case xfl::T4POLAR:   m_prbType4->setChecked(true);   break;
        case xfl::T5POLAR:   m_prbType5->setChecked(true);   break;
        case xfl::T7POLAR:   m_prbType7->setChecked(true);   break;
        case xfl::T8POLAR:   m_prbType8->setChecked(true);   break;
    }

    // force viscous LLT
    if(s_WPolar.isLLTMethod())
    {
        s_WPolar.setViscous(true);
        m_pchViscAnalysis->setChecked(true);
        m_prbViscFromCl->setChecked(true);
        m_prbViscFromAlpha->setChecked(false);
    }

    //initialize inertia
    if(m_pPlane && s_WPolar.bAutoInertia())
    {
        fillInertiaPage();
        s_WPolar.setMass(m_pPlane->totalMass());
        s_WPolar.setCoG(m_pPlane->CoG_t());
    }
    else
    {
        m_pfePlaneMass->setValue(s_WPolar.mass() * Units::kgtoUnit());
        m_pfeXCoG->setValue(s_WPolar.CoG().x * Units::mtoUnit());
        m_pfeZCoG->setValue(s_WPolar.CoG().z * Units::mtoUnit());
    }

    m_pfeQInf->setValue(s_WPolar.velocity()*Units::mstoUnit());
    m_pfeAlphaSpec->setValue(s_WPolar.alphaSpec());
    m_pfePhiSpec->setValue(s_WPolar.phi());


    m_pfePlaneMass->setValue(s_WPolar.mass()*Units::kgtoUnit());

    s_WPolar.setViscousLoop(false);
    m_pchViscousLoop->setChecked(false);

    disableVortonWake(s_WPolar);

    setWingLoad();
    setReynolds();
    setPolar3dName();


    m_pfeQInf->setSelection(0,-1);
    m_pfeQInf->setFocus();

    // for consistency with stab and control polars
    PlaneXfl const*pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    s_WPolar.resetAngleRanges(pPlaneXfl);
    s_WPolar.resizeFlapCtrls(pPlaneXfl);

    fillFlapControls();
    fillAVLCtrlList();

    connectSignals();
    enableControls();
}


void T1234578PolarDlg::setType7Polar()
{
    s_WPolar.setType(xfl::T7POLAR);

    m_prbType1->setChecked(false);
    m_prbType2->setChecked(false);
    m_prbType3->setChecked(false);
    m_prbType4->setChecked(false);
    m_prbType5->setChecked(false);
    m_prbType7->setChecked(true);
    m_prbType8->setChecked(false);
}


void T1234578PolarDlg::onEditingFinished()
{
    readData();
    setReynolds();
    setPolar3dName();
    enableControls();
}


void T1234578PolarDlg::onOK()
{
    readData();

    if(!checkWPolarData()) return;

    if(!s_WPolar.bAutoInertia())
    {
        if(fabs(s_WPolar.mass())<PRECISION && s_WPolar.isFixedLiftPolar())
        {
            QMessageBox::warning(this, tr("Warning"), tr("Mass must be non-zero for type 2 polars"));
            m_pfePlaneMass->setFocus();
            return;
        }
    }

    accept();
}


void T1234578PolarDlg::onPolarType()
{
    if     (m_prbType1->isChecked())    s_WPolar.setType(xfl::T1POLAR);
    else if(m_prbType2->isChecked())    s_WPolar.setType(xfl::T2POLAR);
    else if(m_prbType3->isChecked())    s_WPolar.setType(xfl::T3POLAR);
    else if(m_prbType4->isChecked())    s_WPolar.setType(xfl::T4POLAR);
    else if(m_prbType5->isChecked())    s_WPolar.setType(xfl::T5POLAR);
    else if(m_prbType7->isChecked())    s_WPolar.setType(xfl::T7POLAR);
    else if(m_prbType8->isChecked())    s_WPolar.setType(xfl::T8POLAR);

    enableControls();
    setReynolds();
    setPolar3dName();
}


void T1234578PolarDlg::readData()
{
    PlanePolarDlg::readData();

    s_WPolar.setThinSurfaces(m_prbThinSurfaces->isChecked());
    if(s_WPolar.isVLM()) s_WPolar.setThinSurfaces(true);

    s_WPolar.setBeta(0.0);
    if(fabs(s_WPolar.betaSpec())>PRECISION)
    {
        if(m_prbVLM1Method->isChecked())
        {
            m_prbVLM1Method->blockSignals(true);
            m_prbVLM2Method->blockSignals(true);
            m_prbVLM2Method->setChecked(true);
            m_prbVLM1Method->blockSignals(false);
            m_prbVLM2Method->blockSignals(false);
        }
    }

    onFlapControls();
    readInertiaData();

    s_WPolar.setVelocity(m_pfeQInf->value() / Units::mstoUnit());
    s_WPolar.setAlphaSpec(m_pfeAlphaSpec->value());
    s_WPolar.setPhi(m_pfePhiSpec->value());

    s_WPolar.setGroundHeight(m_pfeHeight->value() / Units::mtoUnit());

    setWingLoad();
}


void T1234578PolarDlg::setupLayout()
{
    QString strSpeedUnit, strLengthUnit, strWeightUnit;
    strSpeedUnit = Units::speedUnitQLabel();
    strLengthUnit = Units::lengthUnitQLabel();
    strWeightUnit = Units::massUnitQLabel();

    QFont fixedfnt(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    QTabWidget *pTabWt = new QTabWidget(this);
    connect(pTabWt, SIGNAL(currentChanged(int)), SLOT(onTabChanged(int)));
    pTabWt->setMovable(true);

    QFrame *pfrPolarType = new QFrame;
    {
        QHBoxLayout *pPolarTypePageLayout = new QHBoxLayout;
        {
            QVBoxLayout *pAnalysisTypeLayout = new QVBoxLayout;
            {
                m_prbType1 = new QRadioButton(tr("Type 1 (fixed V") + INFch + tr(")"));
                m_prbType2 = new QRadioButton(tr("Type 2 (fixed lift)"));
                m_prbType3 = new QRadioButton(tr("Type 3 (speed polar)"));
                m_prbType4 = new QRadioButton(tr("Type 4 (fixed ") + ALPHAch + tr(")"));
                m_prbType5 = new QRadioButton(tr("Type 5 (") + BETAch + tr("  range, fixed ") + ALPHAch + tr(" and V") + INFch + tr(")"));
                m_prbType7 = new QRadioButton(tr("Type 7 (stability analysis)"));
                m_prbType8 = new QRadioButton(tr("Type 8"));

                QLabel *pFlow5Link = new QLabel;
                pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Analysis/T8_polars.html>https://flow5.tech/docs/flow5_doc/Analysis/T8_polars.html</a>");
                pFlow5Link->setOpenExternalLinks(true);
                pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
                pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignRight);

                pAnalysisTypeLayout->addWidget(m_prbType1);
                pAnalysisTypeLayout->addWidget(m_prbType2);
                pAnalysisTypeLayout->addWidget(m_prbType3);
                pAnalysisTypeLayout->addWidget(m_prbType4);
                pAnalysisTypeLayout->addWidget(m_prbType5);
                pAnalysisTypeLayout->addWidget(m_prbType7);
                pAnalysisTypeLayout->addWidget(m_prbType8);
                pAnalysisTypeLayout->addWidget(pFlow5Link);
                pAnalysisTypeLayout->addStretch();
            }

            QVBoxLayout *pRightLayout = new QVBoxLayout;
            {
                QPixmap pixmap;
                m_plabVh = new QLabel();
                if(DisplayOptions::isDarkMode())
                    pixmap.load(":/images/V_h_inv.png");
                else
                    pixmap.load(":/images/V_h.png");

                m_plabVh->setPixmap(pixmap);
                m_plabVh->setAlignment(Qt::AlignHCenter |Qt::AlignVCenter);

                m_plabGlide = new QLabel();
                if(DisplayOptions::isDarkMode())
                    pixmap.load(":/images/V_glide_inv.png");
                else            pixmap.load(":/images/V_glide.png");
                m_plabGlide->setPixmap(pixmap);
                m_plabGlide->setAlignment(Qt::AlignHCenter |Qt::AlignVCenter);


                m_pfrAlpha = new QFrame;
                {
                    QHBoxLayout *pAlphaLayout = new QHBoxLayout;
                    {
                        QLabel *pLabAlpha = new QLabel("<p>&alpha; =</p>");
                        m_pfeAlphaSpec = new FloatEdit;
                        QLabel *pLabDeg = new QLabel("<p>&deg;</p>");

                        pAlphaLayout->addWidget(pLabAlpha, 0, Qt::AlignRight);
                        pAlphaLayout->addWidget(m_pfeAlphaSpec);
                        pAlphaLayout->addWidget(pLabDeg);
                    }
                    m_pfrAlpha->setLayout(pAlphaLayout);
                }

                m_pfrPhi = new QFrame;
                {
                    QHBoxLayout *pPhiLayout = new QHBoxLayout;
                    {
                        QLabel *pLabPhi = new QLabel(tr("<p>&phi; =</p>"));
                        m_pfePhiSpec = new FloatEdit;
                        m_pfePhiSpec->setToolTip(tr("<p>The bank angle</p>"));
                        QLabel *pLabDeg = new QLabel(tr("<p>&deg;</p>"));

                        pPhiLayout->addWidget(pLabPhi, 0, Qt::AlignRight);
                        pPhiLayout->addWidget(m_pfePhiSpec);
                        pPhiLayout->addWidget(pLabDeg);
                    }
                    m_pfrPhi->setLayout(pPhiLayout);
                }


                m_pfrQInf = new QFrame;
                {
                    QHBoxLayout *pQinfLayout = new QHBoxLayout;
                    {
                        QLabel *plabinf = new QLabel(tr("<p>V<sub>&infin;</sub>="));

                        m_pfeQInf = new FloatEdit;
                        m_pfeQInf->setMin(0.0);
                        QLabel *plabSpeedUnit   = new QLabel(strSpeedUnit);

                        pQinfLayout->addWidget(plabinf, 0, Qt::AlignRight);
                        pQinfLayout->addWidget(m_pfeQInf);
                        pQinfLayout->addWidget(plabSpeedUnit);
                    }
                    m_pfrQInf->setLayout(pQinfLayout);
                }

                m_frFlightInfo = new QFrame;
                {
                    QVBoxLayout *pFlightLayout = new QVBoxLayout;
                    {
                        m_plabWingLoad  = new QLabel(tr("Wing loading = 0.033 kg/dm2"));
                        m_plabReInfo    = new QLabel(tr("Re info"));

                        m_plabWingLoad->setFont(fixedfnt);
                        m_plabReInfo->setFont(fixedfnt);

                        pFlightLayout->addWidget(m_plabWingLoad,  0, Qt::AlignRight);
                        pFlightLayout->addWidget(m_plabReInfo,    0, Qt::AlignRight);
                    }
                    m_frFlightInfo->setLayout(pFlightLayout);
                }

                pRightLayout->addWidget(m_pfrPhi);
                pRightLayout->addWidget(m_pfrQInf);
                pRightLayout->addWidget(m_pfrAlpha);
                pRightLayout->addWidget(m_plabVh);
                pRightLayout->addWidget(m_plabGlide);
                pRightLayout->addStretch();
                pRightLayout->addWidget(m_frFlightInfo);
            }


            pPolarTypePageLayout->addLayout(pAnalysisTypeLayout);
            pPolarTypePageLayout->addStretch();
            pPolarTypePageLayout->addLayout(pRightLayout);
        }
        pfrPolarType ->setLayout(pPolarTypePageLayout);
    }

    QFrame *pfrAVLCtrls = new QFrame(this);
    {
        QVBoxLayout *pAVLPageLayout = new QVBoxLayout;
        {
            QHBoxLayout *pAVLCtrlsLayout = new QHBoxLayout;
            {
                m_pcptAVLCtrls = new CPTableView(this);
                m_pcptAVLCtrls->setEditable(true);
                m_pcptAVLCtrls->setWindowTitle(tr("Controls"));
                m_pcptAVLCtrls->setContextMenuPolicy(Qt::CustomContextMenu);
                m_pAVLCtrlModel = new QStandardItemModel(this);
                m_pAVLCtrlModel->setRowCount(0);//temporary
                m_pAVLCtrlModel->setColumnCount(1);
                m_pAVLCtrlModel->setHeaderData(0, Qt::Horizontal, tr("Control name"));

                m_pcptAVLCtrls->setModel(m_pAVLCtrlModel);
                m_pcptAVLCtrls->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

                m_pcptAVLGains = new CPTableView(this);
                m_pcptAVLGains->setEditable(true);
                m_pcptAVLGains->setWindowTitle(tr("Controls"));

                m_pAVLGainModel = new CtrlTableModel(this);
                m_pAVLGainModel->setRowCount(0);//temporary
                m_pAVLGainModel->setColumnCount(2);
                m_pAVLGainModel->setHeaderData(0, Qt::Horizontal, tr("Control surfaces"));
                m_pAVLGainModel->setHeaderData(1, Qt::Horizontal, tr("Gain (") + DEGch + tr(")/ ctrl unit"));

                m_pcptAVLGains->setModel(m_pAVLGainModel);
//                m_pcptAVLGains->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
                m_pcptAVLGains->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

                m_pAVLGainDelegate = new CtrlTableDelegate(this);
                m_pAVLGainDelegate->setPrecision({1,3});
                m_pAVLGainDelegate->setEditable({false, true});
                m_pcptAVLGains->setItemDelegate(m_pAVLGainDelegate);

                pAVLCtrlsLayout->addWidget(m_pcptAVLCtrls);
                pAVLCtrlsLayout->addWidget(m_pcptAVLGains);
            }

            QLabel *plabNote = new QLabel(tr("<p>Each set of controls is used to calculate a control derivative."
                                          "</p>"));
            pAVLPageLayout->addLayout(pAVLCtrlsLayout);
            pAVLPageLayout->addWidget(plabNote);
        }
        pfrAVLCtrls->setLayout(pAVLPageLayout);
    }


    pTabWt->addTab(pfrPolarType,   tr("Polar type"));
    pTabWt->addTab(m_pfrMethod,    tr("Method"));
    pTabWt->addTab(m_pfrRefDims,   tr("Ref. dimensions"));
    pTabWt->addTab(m_pfrFluid,     tr("Fluid"));
    pTabWt->addTab(m_pfrInertia,   tr("Inertia"));
    pTabWt->addTab(m_pfrViscosity, tr("Viscosity"));
    pTabWt->addTab(m_pfrFlaps,     tr("Flaps"));
    pTabWt->addTab(pfrAVLCtrls,    tr("AVL-type ctrls"));
    pTabWt->addTab(m_pfrGround,    tr("Ground"));
    pTabWt->addTab(m_pfrFuseDrag,  tr("Fuselage"));
    pTabWt->addTab(m_pExtraDragWt, tr("Extra drag"));
    pTabWt->addTab(m_pfrWake,      tr("Wake"));

    pTabWt->setCurrentIndex(0);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pfrPolarName);
        pMainLayout->addWidget(pTabWt);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void T1234578PolarDlg::resizeColumns()
{
    double w = double(m_pcptAVLGains->width())*.7;
    m_pcptAVLGains->setColumnWidth(0, w);
}


void T1234578PolarDlg::setWingLoad()
{
    QString str,str1, str2;

    if(s_WPolar.referenceArea()>0)
    {
        double WingLoad = s_WPolar.mass()/s_WPolar.referenceArea();//kg/dm2

        str = tr("Wing loading = ") + QString::asprintf("%.3f ", WingLoad * Units::kgtoUnit() / Units::m2toUnit());

        str1 = Units::massUnitQLabel();
        str2 = Units::areaUnitQLabel();
        m_plabWingLoad->setText(str+str1+"/"+str2);
    }
}


void T1234578PolarDlg::setReynolds()
{
    QString strange, strUnit;
    QString lab;
    strUnit = Units::speedUnitQLabel();

    if(!m_pPlane)
    {
        m_plabReInfo->setText(QString());
        return;
    }

    if(s_WPolar.isFixedSpeedPolar())
    {
        double RRe = m_pPlane->rootChord() * s_WPolar.velocity()/s_WPolar.viscosity();
        strange =  tr("Root Re = %1").arg(RRe, 0, 'f', 0);
        lab = strange.rightJustified(37, ' ') + EOLch;
        double SRe = m_pPlane->tipChord() * s_WPolar.velocity()/s_WPolar.viscosity();
        strange = tr("Tip Re   = %1").arg(SRe, 0, 'f', 0);
        lab += strange.rightJustified(37, ' ');
        m_plabReInfo->setText(lab);
    }
    else if(s_WPolar.isFixedLiftPolar())
    {
        double QCl = sqrt(2.* 9.81 /s_WPolar.density()* s_WPolar.mass() /s_WPolar.referenceArea());
        strange = tr("V") + INFch + tr(".sqrt(Cl) = %1").arg(QCl, 0, 'f', 3) + strUnit;
        lab = strange.rightJustified(37, ' ') + EOLch;

        double RRe = m_pPlane->rootChord() * QCl/s_WPolar.viscosity();
        strange = tr("Root Re.sqrt(Cl) = %1").arg(RRe, 0, 'f', 0);
        lab += strange.rightJustified(37, ' ') + EOLch;

        double SRe = m_pPlane->tipChord() * QCl/s_WPolar.viscosity();
        strange = tr("Tip Re.sqrt(Cl) = %1").arg(SRe, 0, 'f', 0);
        lab += strange.rightJustified(37, ' ');

        m_plabReInfo->setText(lab);
    }
    else
    {
        m_plabReInfo->setText(QString());
    }
}


void T1234578PolarDlg::onAVLRowChanged(QModelIndex index)
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl)
    {
        m_pAVLCtrlModel->setRowCount(0);
        m_pAVLGainModel->setRowCount(0);
        return;
    }

    int row = index.row();

    if(row==s_WPolar.nAVLCtrls())
    {
        s_WPolar.addAVLControl();
        s_WPolar.AVLCtrl(row).resizeValues(pPlaneXfl->nAVLGains());
    }

    fillAVLGains();
}


void T1234578PolarDlg::fillAVLCtrlList()
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl)
    {
        m_pAVLCtrlModel->setRowCount(0);
        m_pAVLGainModel->setRowCount(0);
        return;
    }

    m_pAVLCtrlModel->setRowCount(s_WPolar.nAVLCtrls());
    QModelIndex ind;

    for(int ic=0; ic<s_WPolar.nAVLCtrls(); ic++)
    {
        ind = m_pAVLCtrlModel->index(ic, 0, QModelIndex());
        m_pAVLCtrlModel->setData(ind, QString::fromStdString(s_WPolar.AVLCtrlName(ic)));
    }
}


void T1234578PolarDlg::fillAVLGains()
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl)
    {
        m_pAVLGainModel->setRowCount(0);
        return;
    }

    QModelIndex idx = m_pcptAVLCtrls->selectionModel()->currentIndex();
    if(!idx.isValid())
    {
        m_pAVLGainModel->setRowCount(0);
        return;
    }
    int iCtrl = idx.row();


    AngleControl const &avlc = s_WPolar.m_AVLControls.at(iCtrl);

    m_pAVLGainModel->setRowCount(avlc.nValues());

    QModelIndex ind;
    for(int ig=0; ig<avlc.nValues(); ig++)
    {
        ind = m_pAVLGainModel->index(ig, 0, QModelIndex());
        m_pAVLGainModel->setData(ind, QString::fromStdString(pPlaneXfl->controlSurfaceName(ig)));

        ind = m_pAVLGainModel->index(ig, 1, QModelIndex());
        m_pAVLGainModel->setData(ind, avlc.value(ig));
    }
}


void T1234578PolarDlg::readAVLCtrls()
{
    s_WPolar.clearAVLCtrls();

    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl) return;

    for(int ic=0; ic<m_pAVLCtrlModel->rowCount(); ic++)
    {
        AngleControl avlc;
        std::string name = m_pAVLCtrlModel->index(ic, 0, QModelIndex()).data().toString().toStdString();
        avlc.setName(name);
        avlc.resizeValues(pPlaneXfl->nAVLGains());
        for(int ig=0; ig<m_pAVLGainModel->rowCount(); ig++)
        {
            avlc.setValue(ig, m_pAVLGainModel->index(ig, 1, QModelIndex()).data().toDouble());
        }
        s_WPolar.addAVLControl(avlc);
    }
}


void T1234578PolarDlg::onAVLContextMenu(QPoint)
{
    QModelIndex ind = m_pcptAVLCtrls->selectionModel()->currentIndex();

    QString ctrlname;
    if (ind.isValid()) ctrlname = m_pAVLCtrlModel->data(ind).toString();

    QMenu *pAVLActionsMenu = new QMenu;
    {
        QAction *pAppendRow = new QAction(tr("Append new control set"), this);
        QAction *pDeleteRow = new QAction(tr("Delete selected"),        this);
        QAction *pDuplicate = new QAction(tr("Duplicate"),              this);
        QAction *pMoveUp    = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowUp),   tr("Move up"),   this);
        QAction *pMoveDown  = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowDown), tr("Move down"), this);
        pMoveUp->setData(0);
        pMoveDown->setData(1);
        pAVLActionsMenu->addAction(pAppendRow);
        pAVLActionsMenu->addAction(pDuplicate);
        pAVLActionsMenu->addAction(pDeleteRow);
        pAVLActionsMenu->addSeparator();
        pAVLActionsMenu->addAction(pMoveUp);
        pAVLActionsMenu->addAction(pMoveDown);

        if(ind.isValid())
        {
            pDuplicate->setText(tr("Duplicate %1").arg(ctrlname));
            pDeleteRow->setText(tr("Delete %1").arg(ctrlname));
        }
        pDuplicate->setEnabled(ind.isValid());
        pDeleteRow->setEnabled(ind.isValid());
        pMoveUp->setEnabled(ind.isValid());
        pMoveDown->setEnabled(ind.isValid());

        connect(pAppendRow, SIGNAL(triggered()), SLOT(onAppendAVLCtrl()));
        connect(pDuplicate, SIGNAL(triggered()), SLOT(onDuplicateAVLCtrl()));
        connect(pDeleteRow, SIGNAL(triggered()), SLOT(onDeleteAVLCtrl()));
        connect(pMoveUp,    SIGNAL(triggered()), SLOT(onMoveAVLCtrl()));
        connect(pMoveDown,  SIGNAL(triggered()), SLOT(onMoveAVLCtrl()));

    }
    pAVLActionsMenu->exec(QCursor::pos());
}


void T1234578PolarDlg::onAppendAVLCtrl()
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl) return;

    AngleControl avlc;
    avlc.setName((tr("new control_%1").arg(s_WPolar.nAVLCtrls()+1)).toStdString());
    avlc.resizeValues(pPlaneXfl->nAVLGains());
    s_WPolar.addAVLControl(avlc);
    fillAVLCtrlList();
    QModelIndex ind = m_pAVLCtrlModel->index(s_WPolar.nAVLCtrls()-1, 0);
    m_pcptAVLCtrls->setCurrentIndex(ind);
    fillAVLGains();
}


void T1234578PolarDlg::onDuplicateAVLCtrl()
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl) return;
    QModelIndex ind = m_pcptAVLCtrls->selectionModel()->currentIndex();
    if(!ind.isValid()) return;

    AngleControl avlc = s_WPolar.AVLCtrl(ind.row());
    s_WPolar.insertAVLControl(ind.row()+1,avlc);
    fillAVLCtrlList();
    ind = m_pAVLCtrlModel->index(ind.row()+1, 0);
    m_pcptAVLCtrls->setCurrentIndex(ind);
    fillAVLGains();
}


void T1234578PolarDlg::onDeleteAVLCtrl()
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl) return;

    QModelIndex ind = m_pcptAVLCtrls->selectionModel()->currentIndex();
    if(!ind.isValid()) return;
    int iCtrl = ind.row();
    if(iCtrl>=0 && iCtrl<s_WPolar.nAVLCtrls())
    {
        s_WPolar.removeAVLControl(iCtrl);
    }
    fillAVLCtrlList();
}


void T1234578PolarDlg::onAVLCtrlChanged()
{
    QModelIndex ind = m_pcptAVLCtrls->selectionModel()->currentIndex();
    if(!ind.isValid()) return;

    int iCtrl = ind.row();
    if(iCtrl<0 || iCtrl>=s_WPolar.nAVLCtrls()) return;

    AngleControl &avlc = s_WPolar.AVLCtrl(iCtrl);
    std::string name = m_pAVLCtrlModel->index(iCtrl, 0, QModelIndex()).data().toString().toStdString();
    avlc.setName(name);
}


void T1234578PolarDlg::onAVLGainChanged()
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl) return;

    QModelIndex ind = m_pcptAVLCtrls->selectionModel()->currentIndex();
    if(!ind.isValid()) return;

    int iCtrl = ind.row();
    if(iCtrl<0 || iCtrl>=s_WPolar.nAVLCtrls()) return;

    AngleControl &avlc = s_WPolar.AVLCtrl(iCtrl);
    std::string name = m_pAVLCtrlModel->index(iCtrl, 0, QModelIndex()).data().toString().toStdString();
    avlc.setName(name);
    avlc.resizeValues(pPlaneXfl->nAVLGains());
    for(int ig=0; ig<m_pAVLGainModel->rowCount(); ig++)
    {
        avlc.setValue(ig, m_pAVLGainModel->index(ig, 1, QModelIndex()).data().toDouble());
    }
}


void T1234578PolarDlg::onMoveAVLCtrl()
{
    QModelIndex ind = m_pcptAVLCtrls->selectionModel()->currentIndex();
    if(!ind.isValid()) return;

    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    int id = pSenderAction->data().toInt();

    int sel=ind.row();

    if(id==0)
    {
        if(sel>0)
        {
            AngleControl ctrl = s_WPolar.AVLCtrl(sel);
            s_WPolar.removeAVLControl(sel);
            sel--;
            s_WPolar.insertAVLControl(sel, ctrl);
        }
    }
    else
    {
        if(ind.row()<s_WPolar.nAVLCtrls()-1)
        {
            AngleControl ctrl = s_WPolar.AVLCtrl(sel);
            s_WPolar.removeAVLControl(sel);
            sel++;
            s_WPolar.insertAVLControl(sel, ctrl);
        }
    }
    fillAVLCtrlList();
    fillAVLGains();

    QModelIndex idx = m_pAVLCtrlModel->index(sel, 0);
    m_pcptAVLCtrls->setCurrentIndex(idx);
}



