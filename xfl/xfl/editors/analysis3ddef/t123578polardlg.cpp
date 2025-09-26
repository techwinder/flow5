/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

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

#include <xfl/editors/analysis3ddef/ctrltablemodel.h>
#include <xfl/editors/analysis3ddef/extradragwt.h>
#include <xfl/editors/analysis3ddef/t123578polardlg.h>
#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/analysis/wpolar.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflwidgets/customwts/actionitemmodel.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/ctrltabledelegate.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>


T123578PolarDlg::T123578PolarDlg(QWidget *pParent) : PlanePolarDlg(pParent)
{
    setWindowTitle("Analysis definition");

    m_pPlane = nullptr;

    setupLayout();
}


void T123578PolarDlg::connectSignals()
{
    PlanePolarDlg::connectSignals();

    connect(m_prbType1,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));
    connect(m_prbType2,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));
    connect(m_prbType3,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));
    connect(m_prbType4,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));
    connect(m_prbType5,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));
    connect(m_prbType7,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));
    connect(m_prbType8,        SIGNAL(toggled(bool)),   SLOT(onPolarType()));

    connect(m_pdeXCoG,         SIGNAL(floatChanged(float)),  SLOT(onEditingFinished()));
    connect(m_pdeZCoG,         SIGNAL(floatChanged(float)),  SLOT(onEditingFinished()));

    connect(m_pfePlaneMass,    SIGNAL(floatChanged(float)),  SLOT(onEditingFinished()));
    connect(m_pfeQInf,         SIGNAL(floatChanged(float)),  SLOT(onEditingFinished()));
    connect(m_pdeAlphaSpec,    SIGNAL(floatChanged(float)),  SLOT(onEditingFinished()));
    connect(m_pdePhiSpec,      SIGNAL(floatChanged(float)),  SLOT(onEditingFinished()));

    connect(m_pcptAVLCtrls->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onAVLRowChanged(QModelIndex)));
    connect(m_pcptAVLCtrls,                   SIGNAL(customContextMenuRequested(QPoint)),         SLOT(onAVLContextMenu(QPoint)));
    connect(m_pcptAVLGains,                   SIGNAL(dataPasted()),                               SLOT(onAVLGainChanged()));
    connect(m_pAVLGainDelegate,               SIGNAL(closeEditor(QWidget*)),                      SLOT(onAVLGainChanged()));
}


void T123578PolarDlg::enableControls()
{
    PlanePolarDlg::enableControls();

    m_prbType4->setEnabled(false);

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
    m_pdeXCoG->setEnabled(     !s_WPolar.bAutoInertia());
    m_pdeZCoG->setEnabled(     !s_WPolar.bAutoInertia());
    m_pdeIxx->setEnabled(      !s_WPolar.bAutoInertia());
    m_pdeIyy->setEnabled(      !s_WPolar.bAutoInertia());
    m_pdeIzz->setEnabled(      !s_WPolar.bAutoInertia());
    m_pdeIxz->setEnabled(      !s_WPolar.bAutoInertia());

    if(s_WPolar.isBetaPolar())    m_prbVLM1Method->setEnabled(false);
}


void T123578PolarDlg::initPolar3dDlg(const Plane *pPlane, const WPolar *pWPolar)
{
    PlanePolarDlg::initPolar3dDlg(pPlane, pWPolar);

    if(!pWPolar || pWPolar->isType6())
        s_WPolar.setType(xfl::T1POLAR);

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
        m_pdeXCoG->setValue(s_WPolar.CoG().x * Units::mtoUnit());
        m_pdeZCoG->setValue(s_WPolar.CoG().z * Units::mtoUnit());
    }

    m_pfeQInf->setValue(s_WPolar.velocity()*Units::mstoUnit());
    m_pdeAlphaSpec->setValue(s_WPolar.alphaSpec());
    m_pdePhiSpec->setValue(s_WPolar.phi());


    m_pfePlaneMass->setValue(s_WPolar.mass()*Units::kgtoUnit());

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


void T123578PolarDlg::setType7Polar()
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


void T123578PolarDlg::onEditingFinished()
{
    readData();
    setReynolds();
    setPolar3dName();
    enableControls();
}


void T123578PolarDlg::onOK()
{
    readData();

    if(!checkWPolarData()) return;

    if(!s_WPolar.bAutoInertia())
    {
        if(fabs(s_WPolar.mass())<PRECISION && s_WPolar.isFixedLiftPolar())
        {
            QMessageBox::warning(this, "Warning", "Mass must be non-zero for type 2 polars");
            m_pfePlaneMass->setFocus();
            return;
        }
    }

    accept();
}


void T123578PolarDlg::onPolarType()
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


void T123578PolarDlg::readData()
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
    s_WPolar.setAlphaSpec(m_pdeAlphaSpec->value());
    s_WPolar.setPhi(m_pdePhiSpec->value());

    s_WPolar.setGroundHeight(m_pfeHeight->value() / Units::mtoUnit());

    setWingLoad();
}


void T123578PolarDlg::setupLayout()
{
    QString strSpeedUnit, strLengthUnit, strWeightUnit;

    Units::getSpeedUnitLabel(strSpeedUnit);
    Units::getLengthUnitLabel(strLengthUnit);
    Units::getMassUnitLabel(strWeightUnit);

    QFont fixedfnt(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    QTabWidget *pTabWt = new QTabWidget(this);
    pTabWt->setMovable(true);

    QFrame *pfrPolarType = new QFrame;
    {
        QHBoxLayout *pPolarTypePageLayout = new QHBoxLayout;
        {
            QVBoxLayout *pAnalysisTypeLayout = new QVBoxLayout;
            {
                m_prbType1 = new QRadioButton(QString("Type 1 (fixed V")+INFCHAR+")");
                m_prbType2 = new QRadioButton("Type 2 (fixed lift)");
                m_prbType3 = new QRadioButton("Type 3 (speed polar)");
                m_prbType4 = new QRadioButton(QString("Type 4 (fixed ") + ALPHACHAR + ") - deprecated, use T8 polars instead");
                m_prbType5 = new QRadioButton(QString("Type 5 (")+BETACHAR+"  range, fixed "+ALPHACHAR+" and V"+INFCHAR+")");
                m_prbType7 = new QRadioButton("Type 7 (stability analysis)");
                m_prbType8 = new QRadioButton("Type 8");

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
                        m_pdeAlphaSpec = new FloatEdit;
                        QLabel *pLabDeg = new QLabel("<p>&deg;</p>");

                        pAlphaLayout->addWidget(pLabAlpha, 0, Qt::AlignRight);
                        pAlphaLayout->addWidget(m_pdeAlphaSpec);
                        pAlphaLayout->addWidget(pLabDeg);
                    }
                    m_pfrAlpha->setLayout(pAlphaLayout);
                }

                m_pfrPhi = new QFrame;
                {
                    QHBoxLayout *pPhiLayout = new QHBoxLayout;
                    {
                        QLabel *pLabPhi = new QLabel("<p>&phi; =</p>");
                        m_pdePhiSpec = new FloatEdit;
                        m_pdePhiSpec->setToolTip("<p>The bank angle</p>");
                        QLabel *pLabDeg = new QLabel("<p>&deg;</p>");

                        pPhiLayout->addWidget(pLabPhi, 0, Qt::AlignRight);
                        pPhiLayout->addWidget(m_pdePhiSpec);
                        pPhiLayout->addWidget(pLabDeg);
                    }
                    m_pfrPhi->setLayout(pPhiLayout);
                }


                m_pfrQInf = new QFrame;
                {
                    QHBoxLayout *pQinfLayout = new QHBoxLayout;
                    {
                        QLabel *plabinf = new QLabel("<p>V<sub>&infin;</sub>=");

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
                        m_plabWingLoad  = new QLabel("Wing loading = 0.033 kg/dm2");
                        m_plabReInfo    = new QLabel("Re info");

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
                m_pcptAVLCtrls->setWindowTitle("Controls");
                m_pcptAVLCtrls->setContextMenuPolicy(Qt::CustomContextMenu);
                m_pAVLCtrlModel = new ActionItemModel(this);
                m_pAVLCtrlModel->setRowCount(0);//temporary
                m_pAVLCtrlModel->setColumnCount(1);
                m_pAVLCtrlModel->setHeaderData(0, Qt::Horizontal, "Control name");

                m_pcptAVLCtrls->setModel(m_pAVLCtrlModel);
                m_pcptAVLCtrls->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

                m_pcptAVLGains = new CPTableView(this);
                m_pcptAVLGains->setEditable(true);
                m_pcptAVLGains->setWindowTitle("Controls");

                m_pAVLGainModel = new CtrlTableModel(this);
                m_pAVLGainModel->setRowCount(0);//temporary
                m_pAVLGainModel->setColumnCount(2);
                m_pAVLGainModel->setHeaderData(0, Qt::Horizontal, "Control surfaces");
                m_pAVLGainModel->setHeaderData(1, Qt::Horizontal, QString("Gain (") + DEGCHAR + ")");

                m_pcptAVLGains->setModel(m_pAVLGainModel);
                m_pcptAVLGains->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

                m_pAVLGainDelegate = new CtrlTableDelegate(this);
                m_pAVLGainDelegate->setPrecision({1,3});
                m_pAVLGainDelegate->setEditable({false, true});
                m_pcptAVLGains->setItemDelegate(m_pAVLGainDelegate);

                pAVLCtrlsLayout->addWidget(m_pcptAVLCtrls);
                pAVLCtrlsLayout->addWidget(m_pcptAVLGains);
            }

            QLabel *plabNote = new QLabel("<p>Each set of controls is used to calculate a control derivative.<br>"
                                          "The influence matrix is rebuilt and the linear system is solved for each set of controls "
                                          "and for each operating point.<br>"
                                          "</p>");
            pAVLPageLayout->addLayout(pAVLCtrlsLayout);
            pAVLPageLayout->addWidget(plabNote);
        }
        pfrAVLCtrls->setLayout(pAVLPageLayout);
    }


    pTabWt->addTab(pfrPolarType,   "Polar type");
    pTabWt->addTab(m_pfrMethod,    "Method");
    pTabWt->addTab(m_pfrViscosity, "Viscosity");
    pTabWt->addTab(m_pfrFlaps,     "Flaps");
    pTabWt->addTab(pfrAVLCtrls,    "AVL-type ctrls");
    pTabWt->addTab(m_pfrInertia,   "Inertia");
    pTabWt->addTab(m_pfrRefDims,   "Ref. dimensions");
    pTabWt->addTab(m_pfrFluid,     "Fluid");
    pTabWt->addTab(m_pfrGround,    "Ground");
    pTabWt->addTab(m_pfrFuseDrag,  "Fuselage");
    pTabWt->addTab(m_pExtraDragWt, "Extra drag");
    pTabWt->addTab(m_pfrWake,      "Wake");

    pTabWt->setCurrentIndex(0);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pfrPolarName);
        pMainLayout->addWidget(pTabWt);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void T123578PolarDlg::setWingLoad()
{
    QString str,str1, str2;

    if(s_WPolar.referenceArea()>0)
    {
        double WingLoad = s_WPolar.mass()/s_WPolar.referenceArea();//kg/dm2

        str = QString::asprintf("Wing loading = %.3f ", WingLoad * Units::kgtoUnit() / Units::m2toUnit());

        Units::getMassUnitLabel(str1);
        Units::getAreaUnitLabel(str2);
        m_plabWingLoad->setText(str+str1+"/"+str2);
    }
}


void T123578PolarDlg::setReynolds()
{
    QString strange, strUnit;
    QString lab;
    Units::getSpeedUnitLabel(strUnit);

    if(!m_pPlane)
    {
        m_plabReInfo->setText(QString());
        return;
    }

    if(s_WPolar.isFixedSpeedPolar())
    {
        double RRe = m_pPlane->rootChord() * s_WPolar.velocity()/s_WPolar.viscosity();
        strange =  QString::asprintf("Root Re = %.0f", RRe);
        lab = strange.rightJustified(37, ' ') + EOLCHAR;
        double SRe = m_pPlane->tipChord() * s_WPolar.velocity()/s_WPolar.viscosity();
        strange = QString::asprintf("Tip Re   = %.0f", SRe);
        lab += strange.rightJustified(37, ' ');
        m_plabReInfo->setText(lab);
    }
    else if(s_WPolar.isFixedLiftPolar())
    {
        double QCl = sqrt(2.* 9.81 /s_WPolar.density()* s_WPolar.mass() /s_WPolar.referenceArea());
        strange = "V"+INFCHAR+".sqrt(Cl) = " + QString::asprintf("%.3f ", QCl) + strUnit;
        lab = strange.rightJustified(37, ' ') + EOLCHAR;

        double RRe = m_pPlane->rootChord() * QCl/s_WPolar.viscosity();
        strange = QString::asprintf("Root Re.sqrt(Cl) = %.0f", RRe);
        lab += strange.rightJustified(37, ' ') + EOLCHAR;

        double SRe = m_pPlane->tipChord() * QCl/s_WPolar.viscosity();
        strange = QString::asprintf("Tip Re.sqrt(Cl) = %.0f", SRe);
        lab += strange.rightJustified(37, ' ');

        m_plabReInfo->setText(lab);
    }
    else
    {
        m_plabReInfo->setText(QString());
    }
}


void T123578PolarDlg::onAVLRowChanged(QModelIndex index)
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl)
    {
        m_pAVLCtrlModel->setRowCount(0);
        m_pAVLGainModel->setRowCount(0);
        return;
    }

    int row = index.row();

    if(row==s_WPolar.nAVLControls())
    {
        s_WPolar.addAVLControl();
        s_WPolar.AVLCtrl(row).resizeValues(pPlaneXfl->nAVLGains());
    }

    fillAVLGains();
}


void T123578PolarDlg::fillAVLCtrlList()
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl)
    {
        m_pAVLCtrlModel->setRowCount(0);
        m_pAVLGainModel->setRowCount(0);
        return;
    }

    m_pAVLCtrlModel->setRowCount(s_WPolar.nAVLControls());
    QModelIndex ind;

    for(int ic=0; ic<s_WPolar.nAVLControls(); ic++)
    {
        ind = m_pAVLCtrlModel->index(ic, 0, QModelIndex());
        m_pAVLCtrlModel->setData(ind, s_WPolar.AVLCtrlName(ic));
    }
}


void T123578PolarDlg::fillAVLGains()
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
        m_pAVLGainModel->setData(ind, pPlaneXfl->controlSurfaceName(ig));

        ind = m_pAVLGainModel->index(ig, 1, QModelIndex());
        m_pAVLGainModel->setData(ind, avlc.value(ig));
    }
}


void T123578PolarDlg::readAVLCtrls()
{
    s_WPolar.clearAVLCtrls();

    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl) return;

    for(int ic=0; ic<m_pAVLCtrlModel->rowCount(); ic++)
    {
        AngleControl avlc;
        QString name = m_pAVLCtrlModel->index(ic, 0, QModelIndex()).data().toString();
        avlc.setName(name);
        avlc.resizeValues(pPlaneXfl->nAVLGains());
        for(int ig=0; ig<m_pAVLGainModel->rowCount(); ig++)
        {
            avlc.setValue(ig, m_pAVLGainModel->index(ig, 1, QModelIndex()).data().toDouble());
        }
        s_WPolar.addAVLControl(avlc);
    }
}


void T123578PolarDlg::onAVLContextMenu(QPoint)
{
    QModelIndex ind = m_pcptAVLCtrls->selectionModel()->currentIndex();

    QString ctrlname;
    if (ind.isValid()) ctrlname = m_pAVLCtrlModel->data(ind).toString();

    QMenu *pAVLActionsMenu = new QMenu;
    {
        QAction *pAppendRow = new QAction("Append new control set", this);
        QAction *pDeleteRow = new QAction("Delete selected",        this);
        QAction *pDuplicate = new QAction("Duplicate",              this);
        QAction *pMoveUp    = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowUp),   "Move up",   this);
        QAction *pMoveDown  = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowDown), "Move down", this);
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
            pDuplicate->setText("Duplicate "+ctrlname);
            pDeleteRow->setText("Delete "+ctrlname);
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


void T123578PolarDlg::onAppendAVLCtrl()
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl) return;

    AngleControl avlc;
    avlc.setName(QString::asprintf("new control_%d", s_WPolar.nAVLControls()+1));
    avlc.resizeValues(pPlaneXfl->nAVLGains());
    s_WPolar.addAVLControl(avlc);
    fillAVLCtrlList();
    QModelIndex ind = m_pAVLCtrlModel->index(s_WPolar.nAVLControls()-1, 0);
    m_pcptAVLCtrls->setCurrentIndex(ind);
    fillAVLGains();
}


void T123578PolarDlg::onDuplicateAVLCtrl()
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


void T123578PolarDlg::onDeleteAVLCtrl()
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl) return;

    QModelIndex ind = m_pcptAVLCtrls->selectionModel()->currentIndex();
    if(!ind.isValid()) return;
    int iCtrl = ind.row();
    if(iCtrl>=0 && iCtrl<s_WPolar.nAVLControls())
    {
        s_WPolar.removeAVLControl(iCtrl);
    }
    fillAVLCtrlList();
}




void T123578PolarDlg::onAVLCtrlChanged()
{
    QModelIndex ind = m_pcptAVLCtrls->selectionModel()->currentIndex();
    if(!ind.isValid()) return;

    int iCtrl = ind.row();
    if(iCtrl<0 || iCtrl>=s_WPolar.nAVLControls()) return;

    AngleControl &avlc = s_WPolar.AVLCtrl(iCtrl);
    QString name = m_pAVLCtrlModel->index(iCtrl, 0, QModelIndex()).data().toString();
    avlc.setName(name);
}


void T123578PolarDlg::onAVLGainChanged()
{
    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl) return;

    QModelIndex ind = m_pcptAVLCtrls->selectionModel()->currentIndex();
    if(!ind.isValid()) return;

    int iCtrl = ind.row();
    if(iCtrl<0 || iCtrl>=s_WPolar.nAVLControls()) return;

    AngleControl &avlc = s_WPolar.AVLCtrl(iCtrl);
    QString name = m_pAVLCtrlModel->index(iCtrl, 0, QModelIndex()).data().toString();
    avlc.setName(name);
    avlc.resizeValues(pPlaneXfl->nAVLGains());
    for(int ig=0; ig<m_pAVLGainModel->rowCount(); ig++)
    {
        avlc.setValue(ig, m_pAVLGainModel->index(ig, 1, QModelIndex()).data().toDouble());
    }
}


void T123578PolarDlg::onMoveAVLCtrl()
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
            AngleControl ctrl = s_WPolar.m_AVLControls.takeAt(sel);
            sel--;
            s_WPolar.insertAVLControl(sel, ctrl);
        }
    }
    else
    {
        if(ind.row()<s_WPolar.nAVLControls()-1)
        {
            AngleControl ctrl = s_WPolar.m_AVLControls.takeAt(sel);
            sel++;
            s_WPolar.insertAVLControl(sel, ctrl);
        }
    }
    fillAVLCtrlList();
    fillAVLGains();

    QModelIndex idx = m_pAVLCtrlModel->index(sel, 0);
    m_pcptAVLCtrls->setCurrentIndex(idx);
}



