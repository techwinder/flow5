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


#include <QFormLayout>
#include <QHeaderView>
#include <QFontDatabase>

#include "polar3ddlg.h"


#include <api/planexfl.h>
#include <api/polar3d.h>
#include <api/planepolar.h>
#include <api/units.h>

#include <interfaces/editors/analysis3ddef/extradragwt.h>
#include <interfaces/editors/analysis3ddef/wpolarautonamedlg.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/ctrltabledelegate.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <modules/xplane/analysis/plpolarnamemaker.h>

QByteArray Polar3dDlg::s_Geometry;


Polar3dDlg::Polar3dDlg(QWidget *pParent) : QDialog(pParent)
{
    m_bAutoName=true;
}


Polar3dDlg::~Polar3dDlg()
{
}


void Polar3dDlg::makeBaseCommonControls()
{
    QString strSpeedUnit, strLengthUnit, strWeightUnit;

    strSpeedUnit = Units::speedUnitQLabel();
    strLengthUnit = Units::lengthUnitQLabel();
    strWeightUnit = Units::massUnitQLabel();

    m_pfrPolarName = new QFrame;
    {
        m_pfrPolarName->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
        QVBoxLayout *pMetaDataLayout = new QVBoxLayout;
        {
            m_plabParentObjectName = new QLabel;
            m_plabParentObjectName->setStyleSheet("font: bold;");

            QHBoxLayout *pPolarNameLayout = new QHBoxLayout;
            {
                m_pchAutoName = new QCheckBox(tr("Auto analysis name"));
                m_plePolarName = new QLineEdit(tr("Polar name"));
                m_plePolarName->setClearButtonEnabled(true);

                m_ppbNameOptions = new QPushButton(tr("Options"));
                pPolarNameLayout->addWidget(m_pchAutoName);
                pPolarNameLayout->addWidget(m_plePolarName);
                pPolarNameLayout->addWidget(m_ppbNameOptions);
            }
            pMetaDataLayout->addWidget(m_plabParentObjectName);
            pMetaDataLayout->addLayout(pPolarNameLayout);
        }
        m_pfrPolarName->setLayout(pMetaDataLayout);
    }

    m_pfrRefDims = new QFrame;
    {
        QHBoxLayout *pRefDimsLayout = new QHBoxLayout;
        {
            QVBoxLayout *pAreaTypeLayout = new QVBoxLayout;
            {
                m_prbArea1 = new QRadioButton(tr("Wing planform (deprecated)"));
                m_prbArea2 = new QRadioButton(tr("Wing planform projected on xy plane"));
                m_prbArea3 = new QRadioButton(tr("Custom"));

                QGridLayout *pRefAreaLayout = new QGridLayout;
                {
                    QLabel *pLabRefArea  = new QLabel(tr("Ref. area="));
                    QLabel *pLabRefSpan  = new QLabel(tr("Ref. span length="));
                    QLabel *pLabRefChord = new QLabel(tr("Ref. chord length="));
                    m_pfeRefArea  = new FloatEdit(0.0, 3);
                    m_pfeRefChord = new FloatEdit(0.0, 3);
                    m_pfeRefSpan  = new FloatEdit(0.0, 3);
                    QLabel *pLabAreaUnit = new QLabel(Units::areaUnitQLabel());
                    QLabel *pLabLengthUnit4 = new QLabel(Units::lengthUnitQLabel());
                    QLabel *PlabLengthUnit5 = new QLabel(Units::lengthUnitQLabel());

                    pLabRefArea->setAlignment( Qt::AlignRight | Qt::AlignVCenter);
                    pLabRefSpan->setAlignment( Qt::AlignRight | Qt::AlignVCenter);
                    pLabRefChord->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

/*                    pLabAreaUnit->setAlignment(Qt::AlignLeft | Qt::AlignCenter);
                    pLabLengthUnit4->setAlignment(Qt::AlignLeft | Qt::AlignCenter);
                    PlabLengthUnit5->setAlignment(Qt::AlignLeft | Qt::AlignCenter);*/

                    pRefAreaLayout->addWidget(pLabRefArea,1,1);
                    pRefAreaLayout->addWidget(m_pfeRefArea,1,2);
                    pRefAreaLayout->addWidget(pLabAreaUnit,1,3, Qt::AlignLeft | Qt::AlignVCenter);
                    pRefAreaLayout->addWidget(pLabRefSpan,2,1);
                    pRefAreaLayout->addWidget(m_pfeRefSpan,2,2);
                    pRefAreaLayout->addWidget(pLabLengthUnit4,2,3, Qt::AlignLeft | Qt::AlignVCenter);
                    pRefAreaLayout->addWidget(pLabRefChord,3,1);
                    pRefAreaLayout->addWidget(m_pfeRefChord,3,2);
                    pRefAreaLayout->addWidget(PlabLengthUnit5,3,3, Qt::AlignLeft | Qt::AlignVCenter);

                    pRefAreaLayout->setColumnStretch(4,1);
                }

                pAreaTypeLayout->addWidget(m_prbArea1);
                pAreaTypeLayout->addWidget(m_prbArea2);
                pAreaTypeLayout->addWidget(m_prbArea3);
                pAreaTypeLayout->addLayout(pRefAreaLayout);
                pAreaTypeLayout->addStretch();
            }

            QVBoxLayout *pOtherWingsLayout = new QVBoxLayout;
            {
                m_pchOtherWings = new QCheckBox(tr("Include area of OTHERWINGS"));
                pOtherWingsLayout->addWidget(m_pchOtherWings);
                pOtherWingsLayout->addStretch();
            }

            pRefDimsLayout->addLayout(pAreaTypeLayout);
            pRefDimsLayout->addLayout(pOtherWingsLayout);
        }
        m_pfrRefDims->setLayout(pRefDimsLayout);
    }

    m_pfrMethod = new QFrame;
    {
        QVBoxLayout *pMethodLayout = new QVBoxLayout;
        {
            QGroupBox *pPanelMethodFrame = new QGroupBox(tr("Analysis method"));
            {
                QGridLayout *pPanelMethodLayout= new QGridLayout;
                {
                    m_prbLLTMethod    = new QRadioButton(tr("LLT"));
                    m_prbVLM1Method   = new QRadioButton(tr("Horseshoe vortices"));
                    m_prbVLM2Method   = new QRadioButton(tr("Ring vortices"));
                    m_prbQuadMethod   = new QRadioButton(tr("Uniform density quad panels"));
                    m_prbTriUniMethod = new QRadioButton(tr("Uniform density triangular panels"));
                    m_prbTriLinMethod = new QRadioButton(tr("Linear density triangular panels"));

                    QLabel*plabLLT    = new QLabel(tr("(Main wing only) (2d polar mesh required)"));
                    QLabel*plabVLM1   = new QLabel(tr("(VLM1) (No sideslip) (Fuselage panels will be ignored)"));
                    QLabel*plabVLM2   = new QLabel(tr("(VLM2) (Fuselage panels will be ignored)"));
                    QLabel*plabTriUni = new QLabel(tr("(Recommended method)"));


                    pPanelMethodLayout->addWidget(m_prbLLTMethod,     1, 1);
                    pPanelMethodLayout->addWidget(m_prbVLM1Method,    2, 1);
                    pPanelMethodLayout->addWidget(m_prbVLM2Method,    3, 1);
                    pPanelMethodLayout->addWidget(m_prbQuadMethod,    4, 1);
                    pPanelMethodLayout->addWidget(m_prbTriUniMethod,  5, 1);
                    pPanelMethodLayout->addWidget(m_prbTriLinMethod,  6, 1);

                    pPanelMethodLayout->addWidget(plabLLT,            1, 2);
                    pPanelMethodLayout->addWidget(plabVLM1,           2, 2);
                    pPanelMethodLayout->addWidget(plabVLM2,           3, 2);
                    pPanelMethodLayout->addWidget(plabTriUni,         5, 2);
                    pPanelMethodLayout->setColumnStretch(1,1);
                    pPanelMethodLayout->setColumnStretch(2,1);
                    pPanelMethodLayout->setColumnStretch(3,2);
                }
                pPanelMethodFrame->setLayout(pPanelMethodLayout);
            }

            m_pgbWingSurf = new QGroupBox(tr("Wings as"));
            {
                QVBoxLayout *pWingSurfLayout = new QVBoxLayout;
                {
                    m_prbThinSurfaces  = new QRadioButton(tr("Thin surfaces\t(Recommended method)"));
                    m_prbThinSurfaces->setToolTip(tr("<p>When assembling the plane, "
                                                  "ensure that the fuselage's mesh conforms to the wings' "
                                                  "mid-camber line at their root chords.</p>"));
                    m_prbThickSurfaces = new QRadioButton(tr("Thick surfaces"));
                    m_prbThickSurfaces->setToolTip(tr("<p>When assembling the plane, "
                                                  "ensure that the fuselage's mesh conforms to the wings' "
                                                  "top and bottom surfaces at their root chords .</p>"));

                    pWingSurfLayout->addWidget(m_prbThinSurfaces);
                    pWingSurfLayout->addWidget(m_prbThickSurfaces);
                }
                m_pgbWingSurf->setLayout(pWingSurfLayout);
            }

            m_pgbFuseMi = new QGroupBox(tr("Moments"));
            {
                QVBoxLayout*pFuseMiLayout = new QVBoxLayout;
                {
                    m_pchIncludeFuseMi = new QCheckBox(tr("Include the contribution of fuselage pressure forces"));
                    QString tip(tr("<p>The pressure forces acting on the fuselage panels can be severely disturbed "
                               "and overevaluated at the locations where the wing TE connect to the fuselage, "
                               "and where the wake panels which extend from the wing TE are likely "
                               "to interact numerically with the fuselage. "
                               "In such cases, it is preferable to omit the fuselage pressure forces in the "
                               "calculation of inviscid moments."
                               "<br>"
                               "<b>Recommendation:</b> Activate only in the case of a standalone fuselage, or if the "
                               "wake panels are not likely to interact with the fuselage panels.</p>"));
                    m_pchIncludeFuseMi->setToolTip(tip);

                    m_pchIncludeWingTipMi = new QCheckBox(tr("Include the contribution of wing tip pressure forces"));

                    QLabel *pFlow5Link = new QLabel;
                    pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Analysis/Moments.html>https://flow5.tech/docs/flow5_doc/Analysis/Moments.html</a>");
                    pFlow5Link->setOpenExternalLinks(true);
                    pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
                    pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

                    pFuseMiLayout->addWidget(m_pchIncludeFuseMi);
                    pFuseMiLayout->addWidget(m_pchIncludeWingTipMi);
                    pFuseMiLayout->addWidget(pFlow5Link);
                }
                m_pgbFuseMi->setLayout(pFuseMiLayout);
            }

            m_pgbHullBox = new QGroupBox(tr("Hull"));
            {
                QHBoxLayout *pWingSurfLayout = new QHBoxLayout;
                {
                    m_pchIncludeHull  = new QCheckBox(tr("Include hull (not recommended)"));
                    m_pchIncludeHull->setToolTip(tr("<p>The hull <b>should not be included</b> in the analysis except "
                                                 "if it is fully emerged such as in the case of the AC75.</p>"));
                    pWingSurfLayout->addWidget(m_pchIncludeHull);
                    pWingSurfLayout->addStretch();
                }
                m_pgbHullBox->setLayout(pWingSurfLayout);
            }

            pMethodLayout->addWidget(pPanelMethodFrame);
            pMethodLayout->addWidget(m_pgbWingSurf);
            pMethodLayout->addWidget(m_pgbFuseMi);
            pMethodLayout->addWidget(m_pgbHullBox);

            pMethodLayout->addStretch();
        }
        m_pfrMethod->setLayout(pMethodLayout);
    }

    m_pfrFluid = new QFrame;
    {
        QGridLayout *pAeroDataLayout = new QGridLayout;
        {
            QLabel *plabRho = new QLabel(tr("<p>&rho;=</p>"));
            m_pfeDensity = new FloatEdit;
            m_pfeDensity->setToolTip(tr("Density"));
            QLabel *plabDensityUnit = new QLabel(Units::densityUnitQLabel());
            QLabel *plabNu = new QLabel(tr("<p>&nu;=</p>"));
            m_pfeViscosity = new FloatEdit;
            m_pfeViscosity->setToolTip(tr("Kinematic viscosity"));
            QLabel *plabViscosityUnit = new QLabel(Units::viscosityUnitQLabel());

            m_ppbFromData = new QPushButton(tr("From altitude and temperature"));

            pAeroDataLayout->addWidget(plabRho,              1, 1, Qt::AlignRight);
            pAeroDataLayout->addWidget(m_pfeDensity,         1, 2);
            pAeroDataLayout->addWidget(plabDensityUnit,      1, 3);
            pAeroDataLayout->addWidget(plabNu,               2, 1, Qt::AlignRight);
            pAeroDataLayout->addWidget(m_pfeViscosity,       2, 2);
            pAeroDataLayout->addWidget(plabViscosityUnit,    2, 3);
            pAeroDataLayout->addWidget(m_ppbFromData,        3, 1, 1, 3);
            pAeroDataLayout->setRowStretch(                  4, 1);
            pAeroDataLayout->setColumnStretch(               4, 3);
        }

        m_pfrFluid->setLayout(pAeroDataLayout);
    }

    m_pExtraDragWt = new ExtraDragWt();

    m_pfrWake = new QFrame;
    {
        QVBoxLayout *pWakePageLayout = new QVBoxLayout;
        {
            QVBoxLayout *pWakeTypeLayout = new QVBoxLayout;
            {
                m_prbPanelWake  = new QRadioButton(tr("Flat panel wake"));
                m_prbVortonWake = new QRadioButton(tr("Vortex Particle Wake (Not recommended)"));
                pWakeTypeLayout->addWidget(m_prbPanelWake);
                pWakeTypeLayout->addWidget(m_prbVortonWake);
                pWakeTypeLayout->addStretch();
            }

            m_pgbFlatWakePanels = new QGroupBox(tr("Flat wake panels"));
            {
                QGridLayout *pWakeGridLayout = new QGridLayout;
                {
                    QLabel *pLabWake = new QLabel(tr("The wake should extend to a distance where the influence of the plane's panels "
                                                  "is no longer felt, e.g. > 30 x chord"));
                    m_pieNXWakePanels = new IntEdit(1);
                    m_pieNXWakePanels->setToolTip(tr("<p>The number of panels in each streamwise wake column.</p>"));
                    m_pfeWakeLength = new FloatEdit(1.0f);
                    m_pfeWakeLength->setToolTip(tr("<p>The wake's total length.<br>Defines the position of the Trefftz plane.</p>"));
                    m_pfeWakePanelFactor = new FloatEdit(1.1f);
                    m_pfeWakePanelFactor->setToolTip(tr("<p>The ratio between the length of two wake panels in the x direction</p>"));

                    QLabel *pLab1 = new QLabel(tr("Nb. of wake panels:"));
                    QLabel *pLab2 = new QLabel(tr("Total length:"));
                    QLabel *pLab3 = new QLabel(tr("X-progression factor:"));

                    m_plabWakeLengthLabUnit = new QLabel(tr("x reference chord"));

                    pWakeGridLayout->addWidget(pLabWake,                  1,1,1,3);
                    pWakeGridLayout->addWidget(pLab1,                     2,1, Qt::AlignRight | Qt::AlignVCenter);
                    pWakeGridLayout->addWidget(m_pieNXWakePanels,         2,2);
                    pWakeGridLayout->addWidget(new QLabel(tr("/wake column")),2,3, Qt::AlignLeft | Qt::AlignVCenter);
                    pWakeGridLayout->addWidget(pLab2,                     3,1, Qt::AlignRight | Qt::AlignVCenter);
                    pWakeGridLayout->addWidget(m_pfeWakeLength,           3,2);
                    pWakeGridLayout->addWidget(m_plabWakeLengthLabUnit,   3,3);
                    pWakeGridLayout->addWidget(pLab3,                     4,1, Qt::AlignRight | Qt::AlignVCenter);
                    pWakeGridLayout->addWidget(m_pfeWakePanelFactor,      4,2);
                    pWakeGridLayout->setColumnStretch(3,1);
                }
                m_pgbFlatWakePanels->setLayout(pWakeGridLayout);
            }
            m_pgbVortonWake = new QGroupBox(tr("Vortex Particle Wake (VPW)"));
            {
                QGridLayout *pWakeIterationsLayout = new QGridLayout;
                {
                    QFont fixedfnt(QFontDatabase::systemFont(QFontDatabase::FixedFont));
#ifdef Q_OS_MAC
                    fixedfnt.setPointSize(std::max(fixedfnt.pointSize(),14));
#endif
                    QLabel *pLab0 = new QLabel(tr("Buffer wake panel length:"));
                    pLab0->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_plabVPWBufferWakeUnit = new QLabel(tr("x reference chord"));
                    m_plabVPWBufferWakeUnit->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                    m_plabVPWBufferWakeUnit->setFont(fixedfnt);
                    m_pfeVPWBufferWake = new FloatEdit;
                    QString tip(tr("<p>The buffer wake sheet has two purposes. The first is to provide a well defined potential jump "
                                "at the trailing edge. The second is to satisfy the Kutta condition (D.J. Willis 2005) "
                                "The length of the buffer wake should typically be a fraction of the M.A.C, although "
                                "this does not seem to have a great impact on the results.<br>"
                                "<b>Recommentation:</b> 0.2-1.0 (x MAC)</p>"));
                    m_pfeVPWBufferWake->setToolTip(tip);

                    QLabel *plab1 = new QLabel(tr("Streamwise step:"));
                    plab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_plabVPWStepUnit = new QLabel(tr("x reference chord"));
                    m_plabVPWStepUnit->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                    m_plabVPWStepUnit->setFont(fixedfnt);
                    m_pfeVortonL0 = new FloatEdit(0.2f);
                    tip = tr("<p>The first step is the half-distance of the first vorton from the trailing wake panel's edge.<br>"
                          "<b>Recommendation:</b> L0 = 1.0 (x MAC)</p>");
                    m_pfeVortonL0->setToolTip(tip);

                    QLabel *pLab5 = new QLabel(tr("Vorton core size:"));
                    pLab5->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_pfeVortonCoreSize = new FloatEdit;
                    tip = tr("<p>The core size is the equivalent for the vorton of the vortex's core radius. "
                                  "It is used to calculate a mollification (damping) factor to apply to the potential velocity.<br>"
                                  "<b>Recommendation:</b> the core size should not be less than the distance between two vortons.</p>");
                    m_pfeVortonCoreSize->setToolTip(tip);
                    m_plabVtnCoreUnit = new QLabel(tr("x reference chord"));
                    m_plabVtnCoreUnit->setFont(fixedfnt);

                    QLabel *pLab7 = new QLabel(tr("Discard vortons further downstream than:"));
                    pLab7->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_plabVPWDiscard = new QLabel(tr("x reference chord"));
                    m_plabVPWDiscard->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                    m_plabVPWDiscard->setFont(fixedfnt);

                    m_pfeVPWLength = new FloatEdit;
                    tip = tr("<p>To ensure that a steady-state solution is reached and to increase the speed of the analysis, "
                          "vortons far downstream can be discarded when their influence is no longer felt by the plane. "
                          "This is also a way to remove the initial rows of vortons from the analysis.<br>"
                          "<b>Recommendation:</b><br>"
                          "   - ensure that the number of iterations is sufficient for the vortons to exceed this distance.<br>"
                          "   - check visually that the end vortons are indeed discarded."
                          "</p>");
                    m_pfeVPWLength->setToolTip(tip);

                    QLabel *pLab8 = new QLabel(tr("Number of iterations:"));
                    tip = tr("<p>The number of iterations should be sufficient for the VPW to extend at least"
                          "30 MACs donwstream. The minimum number depends on the streamwise step for the"
                          "vortons that is set in the analysis.</p>");
                    m_pieVPWIterations = new IntEdit;
                    m_pieVPWIterations->setToolTip(tip);
                    m_plabVPWMaxLength = new QLabel;
                    m_plabVPWMaxLength->setFont(fixedfnt);

                    QLabel *pFlow5Link = new QLabel;
                    pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Analysis/VPW.html#Recommendations>https://flow5.tech/docs/flow5_doc/VPW.html#Recommendations</a>");
                    pFlow5Link->setOpenExternalLinks(true);
                    pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
                    pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);

                    pWakeIterationsLayout->addWidget(pLab5,                    2,1);
                    pWakeIterationsLayout->addWidget(m_pfeVortonCoreSize,      2,2);
                    pWakeIterationsLayout->addWidget(m_plabVtnCoreUnit,        2,3);

                    pWakeIterationsLayout->addWidget(pLab0,                    1,1);
                    pWakeIterationsLayout->addWidget(m_pfeVPWBufferWake,       1,2);
                    pWakeIterationsLayout->addWidget(m_plabVPWBufferWakeUnit,  1,3);

                    pWakeIterationsLayout->addWidget(new QLabel(tr("Streamwise wake development:")), 3, 1,1,3);

                    pWakeIterationsLayout->addWidget(plab1,                    4,1);
                    pWakeIterationsLayout->addWidget(m_pfeVortonL0,            4,2);
                    pWakeIterationsLayout->addWidget(m_plabVPWStepUnit,        4,3);

                    pWakeIterationsLayout->addWidget(pLab8,                    5,1, Qt::AlignVCenter | Qt::AlignRight);
                    pWakeIterationsLayout->addWidget(m_pieVPWIterations,       5,2);
                    pWakeIterationsLayout->addWidget(m_plabVPWMaxLength,       5,3);

                    pWakeIterationsLayout->addWidget(pLab7,                    6,1);
                    pWakeIterationsLayout->addWidget(m_pfeVPWLength,           6,2);
                    pWakeIterationsLayout->addWidget(m_plabVPWDiscard,         6,3);

                    pWakeIterationsLayout->addWidget(pFlow5Link,               7,1,1,3, Qt::AlignVCenter | Qt::AlignLeft);

                    pWakeIterationsLayout->setColumnStretch(4,1);
                }

                m_pgbVortonWake->setLayout(pWakeIterationsLayout);
            }

            pWakePageLayout->addLayout(pWakeTypeLayout);
            pWakePageLayout->addWidget(m_pgbFlatWakePanels);
            pWakePageLayout->addWidget(m_pgbVortonWake);
            pWakePageLayout->addStretch();
        }
        m_pfrWake->setLayout(pWakePageLayout);
    }



    //__________________Control buttons___________________
    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard | QDialogButtonBox::Reset);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }
}


void Polar3dDlg::keyPressEvent(QKeyEvent *pEvent)
{
    // Prevent Return Key from closing App
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            m_pButtonBox->button(QDialogButtonBox::Save)->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            pEvent->ignore();
    }
}


void Polar3dDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
//    resizeEvent(nullptr); // useless; to force table column resize
}


void Polar3dDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void Polar3dDlg::resizeEvent(QResizeEvent *)
{
    resizeColumns();
}


void Polar3dDlg::onTabChanged(int)
{
    resizeColumns();
}


void Polar3dDlg::connectBaseSignals()
{
    connect(m_prbLLTMethod,        SIGNAL(clicked(bool)),          SLOT(onMethod()));
    connect(m_prbVLM1Method,       SIGNAL(clicked(bool)),          SLOT(onMethod()));
    connect(m_prbVLM2Method,       SIGNAL(clicked(bool)),          SLOT(onMethod()));
    connect(m_prbQuadMethod,       SIGNAL(clicked(bool)),          SLOT(onMethod()));
    connect(m_prbTriUniMethod,     SIGNAL(clicked(bool)),          SLOT(onMethod()));
    connect(m_prbTriLinMethod,     SIGNAL(clicked(bool)),          SLOT(onMethod()));

    connect(m_plePolarName,        SIGNAL(editingFinished()),      SLOT(onPolar3dName()));
    connect(m_pchAutoName,         SIGNAL(toggled(bool)),          SLOT(onAutoName()));
    connect(m_ppbNameOptions,      SIGNAL(clicked(bool)),          SLOT(onNameOptions()));

    connect(m_ppbFromData,         SIGNAL(clicked()),              SLOT(onAeroData()));

    connect(m_pfeDensity,          SIGNAL(floatChanged(float)),    SLOT(onEditingFinished()));
    connect(m_pfeViscosity,        SIGNAL(floatChanged(float)),    SLOT(onEditingFinished()));

    connect(m_pfeRefArea,          SIGNAL(floatChanged(float)),    SLOT(onEditingFinished()));
    connect(m_pfeRefSpan,          SIGNAL(floatChanged(float)),    SLOT(onEditingFinished()));
    connect(m_pfeRefChord,         SIGNAL(floatChanged(float)),    SLOT(onEditingFinished()));

    connect(m_prbPanelWake,        SIGNAL(clicked()),              SLOT(onVortonWake()));
    connect(m_prbVortonWake,       SIGNAL(clicked()),              SLOT(onVortonWake()));

    connect(m_pfeVPWBufferWake,    SIGNAL(floatChanged(float)),    SLOT(onEditingFinished()));
    connect(m_pfeVortonL0,         SIGNAL(floatChanged(float)),    SLOT(onEditingFinished()));
    connect(m_pfeVortonCoreSize,   SIGNAL(floatChanged(float)),    SLOT(onEditingFinished()));
    connect(m_pfeVPWLength,        SIGNAL(floatChanged(float)),    SLOT(onEditingFinished()));
    connect(m_pieVPWIterations,    SIGNAL(intChanged(int)),        SLOT(onEditingFinished()));
}


void Polar3dDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save)            == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard)         == pButton)  reject();
    else if (m_pButtonBox->button(QDialogButtonBox::RestoreDefaults) == pButton)  onReset();
}


void Polar3dDlg::onPolar3dName()
{
    m_bAutoName = false;
    m_pchAutoName->setChecked(false);
}


void Polar3dDlg::onAutoName()
{
    m_bAutoName = m_pchAutoName->isChecked();
    if(m_bAutoName) setPolar3dName();
    enableControls();
}


void Polar3dDlg::readWakeData(Polar3d &polar3d) const
{
    polar3d.setVortonWake(      m_prbVortonWake->isChecked());

    polar3d.setNXWakePanel4(         m_pieNXWakePanels->value());
    polar3d.setTotalWakeLengthFactor(m_pfeWakeLength->value());
    polar3d.setWakePanelFactor(      m_pfeWakePanelFactor->value());

    polar3d.setBufferWakeFactor(m_pfeVPWBufferWake->value());
    polar3d.setVortonL0(        m_pfeVortonL0->value());
    polar3d.setVPWMaxLength(    m_pfeVPWLength->value());
    polar3d.setVortonCoreSize(  m_pfeVortonCoreSize->value());
    polar3d.setVPWIterations(   m_pieVPWIterations->value());
}


void Polar3dDlg::setVPWUnits(Polar3d &polar3d)
{
    m_plabVPWBufferWakeUnit->setText(QString::asprintf("x reference chord = %9.3g" , polar3d.bufferWakeFactor()*polar3d.referenceChordLength()*Units::mtoUnit())
                               +Units::lengthUnitQLabel());
    m_plabVPWStepUnit->setText(QString::asprintf("x reference chord = %9.3g ", polar3d.vortonL0()*polar3d.referenceChordLength()*Units::mtoUnit())
                               +Units::lengthUnitQLabel());
    m_plabVtnCoreUnit->setText(QString::asprintf("x reference chord = %9.3g ", polar3d.vortonCoreSize()*polar3d.referenceChordLength()*Units::mtoUnit())
                               +Units::lengthUnitQLabel());
    m_plabVPWDiscard->setText(QString::asprintf("x reference chord = %9.3g ", polar3d.VPWMaxLength()*polar3d.referenceChordLength()*Units::mtoUnit())
                               +Units::lengthUnitQLabel());

    double l = polar3d.vortonL0() * polar3d.referenceChordLength() * double(polar3d.VPWIterations());
    l += polar3d.bufferWakeLength() * polar3d.referenceChordLength();


    m_plabVPWMaxLength->setText(QString(QChar(0x21D2)) + QString::asprintf(" max. distance   = %9.3g ", l*Units::mtoUnit())
                               +Units::lengthUnitQLabel());
}


void Polar3dDlg::onMethod()
{
    readMethodData();
    enableControls();
    setPolar3dName();
}


void Polar3dDlg::disableVortonWake(Polar3d &polar3d)
{
    m_pgbVortonWake->setEnabled(false);
    m_prbPanelWake->setEnabled(false);
    m_prbVortonWake->setEnabled(false);
    m_prbPanelWake->setChecked(true);
    polar3d.setVortonWake(false);
}


