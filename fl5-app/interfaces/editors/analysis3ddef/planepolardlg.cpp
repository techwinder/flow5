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


#include <QFontDatabase>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QButtonGroup>
#include <QHeaderView>


#include "planepolardlg.h"
#include <api/plane.h>
#include <api/planepolar.h>
#include <api/planexfl.h>
#include <api/units.h>
#include <api/utils.h>
#include <core/displayoptions.h>
#include <core/xflcore.h>
#include <interfaces/editors/analysis3ddef/aerodatadlg.h>
#include <interfaces/editors/analysis3ddef/extradragwt.h>
#include <interfaces/editors/analysis3ddef/wpolarautonamedlg.h>
#include <interfaces/widgets/customwts/ctrltabledelegate.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <modules/xplane/analysis/plpolarnamemaker.h>

PlanePolar PlanePolarDlg::s_WPolar;



PlanePolarDlg::PlanePolarDlg(QWidget *pParent) : Polar3dDlg(pParent)
{
    m_pPlane=nullptr;
    makeCommonControls();
}


void PlanePolarDlg::makeCommonControls()
{
    makeBaseCommonControls();
    QFont fixedfnt(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    m_pfrInertia= new QFrame;
    {
        QVBoxLayout *pInertiaPageLayout = new QVBoxLayout;
        {
            m_pchAutoInertia = new QCheckBox(tr("Auto inertia"));
            QGridLayout *pInertiaDataLayout = new QGridLayout;
            {
                pInertiaDataLayout->addWidget(new QLabel(tr("Plane mass =")), 1,1, Qt::AlignVCenter | Qt::AlignRight);
                pInertiaDataLayout->addWidget(new QLabel(tr("X_CoG =")),      2,1, Qt::AlignVCenter | Qt::AlignRight);
                pInertiaDataLayout->addWidget(new QLabel(tr("Z_CoG =")),      3,1, Qt::AlignVCenter | Qt::AlignRight);
                pInertiaDataLayout->addWidget(new QLabel(tr("Ixx   =")),      4,1, Qt::AlignVCenter | Qt::AlignRight);
                pInertiaDataLayout->addWidget(new QLabel(tr("Iyy   =")),      5,1, Qt::AlignVCenter | Qt::AlignRight);
                pInertiaDataLayout->addWidget(new QLabel(tr("Izz   =")),      6,1, Qt::AlignVCenter | Qt::AlignRight);
                pInertiaDataLayout->addWidget(new QLabel(tr("Ixz   =")),      7,1, Qt::AlignVCenter | Qt::AlignRight);
                m_pfePlaneMass  = new FloatEdit;
                m_pfeXCoG = new FloatEdit;
                m_pfeZCoG = new FloatEdit;
                m_pfeIxx  = new FloatEdit;
                m_pfeIyy  = new FloatEdit;
                m_pfeIzz  = new FloatEdit;
                m_pfeIxz  = new FloatEdit;
                pInertiaDataLayout->addWidget(m_pfePlaneMass, 1,2);
                pInertiaDataLayout->addWidget(m_pfeXCoG,      2,2);
                pInertiaDataLayout->addWidget(m_pfeZCoG,      3,2);
                pInertiaDataLayout->addWidget(m_pfeIxx,       4,2);
                pInertiaDataLayout->addWidget(m_pfeIyy,       5,2);
                pInertiaDataLayout->addWidget(m_pfeIzz,       6,2);
                pInertiaDataLayout->addWidget(m_pfeIxz,       7,2);

                pInertiaDataLayout->addWidget(new QLabel(Units::massUnitQLabel()),    1,3);
                pInertiaDataLayout->addWidget(new QLabel(Units::lengthUnitQLabel()),  2,3);
                pInertiaDataLayout->addWidget(new QLabel(Units::lengthUnitQLabel()),  3,3);
                pInertiaDataLayout->addWidget(new QLabel(Units::inertiaUnitQLabel()), 4,3);
                pInertiaDataLayout->addWidget(new QLabel(Units::inertiaUnitQLabel()), 5,3);
                pInertiaDataLayout->addWidget(new QLabel(Units::inertiaUnitQLabel()), 6,3);
                pInertiaDataLayout->addWidget(new QLabel(Units::inertiaUnitQLabel()), 7,3);
                pInertiaDataLayout->setColumnStretch(1,1);
                pInertiaDataLayout->setColumnStretch(2,1);
                pInertiaDataLayout->setColumnStretch(4,3);
                pInertiaDataLayout->setRowStretch(8,1);
            }
            pInertiaPageLayout->addWidget(m_pchAutoInertia);
            pInertiaPageLayout->addLayout(pInertiaDataLayout);
            m_pfrInertia->setLayout(pInertiaPageLayout);
        }
    }

    m_pfrGround = new QFrame;
    {
        QVBoxLayout *pGroundLayout = new QVBoxLayout;
        {
            QHBoxLayout *pGroupLayout = new QHBoxLayout;
            {
                m_pchGround   = new QCheckBox(tr("Ground effect"));
                m_pchGround->setToolTip(tr("<p>Activate this option to simulate an above ground airplane.<br>"
                                        "Height should be positive.</p>"));
                m_pchFreeSurf = new QCheckBox(tr("Free surface"));
                m_pchFreeSurf->setToolTip(tr("<p>Activate this option to simulate an underwater hydrofoil.<br>"
                                          "Height should be negative.</p>"));
                pGroupLayout->addWidget(m_pchGround);
                pGroupLayout->addWidget(m_pchFreeSurf);
                pGroupLayout->addStretch();
            }
            QHBoxLayout *pGroundHeightLayout = new QHBoxLayout;
            {
                QLabel *pLabHeight = new QLabel(tr("Height ="));
                m_pfeHeight = new FloatEdit(0.0f);
                QLabel *plabLengthUnit2 = new QLabel(Units::lengthUnitQLabel());
                pGroundHeightLayout->addWidget(pLabHeight);
                pGroundHeightLayout->addWidget(m_pfeHeight);
                pGroundHeightLayout->addWidget(plabLengthUnit2);
                pGroundHeightLayout->addStretch();
            }

            QLabel *plabWarning = new QLabel(tr("<p>Height should be positive in the case of a ground effect<br>"
                                             "and negative in the case of a free surface effect.</p>"));
            plabWarning->setWordWrap(true);

            QLabel *pFlow5Link = new QLabel;
            pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Analysis/GE.html>https://flow5.tech/docs/flow5_doc/Analysis/GE.html</a>");
            pFlow5Link->setOpenExternalLinks(true);
            pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

            pGroundLayout->addLayout(pGroupLayout);
            pGroundLayout->addLayout(pGroundHeightLayout);
            pGroundLayout->addWidget(plabWarning);
            pGroundLayout->addStretch(1);
            pGroundLayout->addWidget(pFlow5Link);
        }
        m_pfrGround->setLayout(pGroundLayout);
    }

    m_pfrViscosity = new QFrame;
    {
        QVBoxLayout *pViscousPageLayout = new QVBoxLayout;
        {
            m_pchViscAnalysis = new QCheckBox(tr("Viscous analysis"));
            m_pchViscAnalysis->setToolTip(tr("<p>Activate this checkbox to include the viscous drag in the results</p>"));

            QHBoxLayout *pMethodLayout = new QHBoxLayout;
            {
                QButtonGroup *pbgMethod = new QButtonGroup(this);
                {
                    QString tip(tr("<p>"
                                "The viscous drag can be evaluated using the legacy xflr5 method by interpolation "
                                "on the 2d polar mesh, or by on-the-fly XFoil calculations at wing sections. "
                                "Both methods give similar results.<br>"
                                "The interpolation method is mandatory when running an LLT calculation or a panel analysis with the viscous loop activated "
                                "due to the great number of viscous calculations that are required. Irrespective of the time this would take, "
                                "one unconverged XFoil calculation would cause the analysis to fail.<br>"
                                "On the fly calculations take longer than interpolations, but eliminate the need "
                                "of a prior build of a polar mesh for each foil.<br>"
                                "On the fly calculations are strongly recommended if a set of flap deflections is active, "
                                "otherwise the calculation will default to interpolation of the viscous drag on the non-flapped airfoil configuration "
                                "with incorrect results."
                                "</p>"));
                    m_prbViscOnTheFly = new QRadioButton(tr("XFoil on the fly"));
                    m_prbViscOnTheFly->setToolTip(tip);
                    m_prbViscInterpolated = new QRadioButton(tr("Interpolated (xflr5 method) (LLT method)"));
                    m_prbViscInterpolated->setToolTip(tip);
                    pbgMethod->addButton(m_prbViscOnTheFly);
                    pbgMethod->addButton(m_prbViscInterpolated);
                }
                pMethodLayout->addWidget(m_prbViscOnTheFly);
                pMethodLayout->addWidget(m_prbViscInterpolated);
                pMethodLayout->addStretch();
            }

            m_pfrInterpolated = new QFrame;
            {
                QVBoxLayout *pInterpolateLayout = new QVBoxLayout;
                {
                    QButtonGroup *pbgType = new QButtonGroup(this);
                    {
                        m_prbViscFromCl    = new QRadioButton(tr("from Cl - xflr5 method (recommended)"));
                        QString tipCl(tr("<p>Activate this option to interpolate the viscous data from the local lift coefficient, "
                                      "i.e. identical to the method implemented in xflr5</p>"));
                        m_prbViscFromCl->setToolTip(tipCl);

                        m_prbViscFromAlpha = new QRadioButton(tr("from ") + ALPHAch + tr(" - viscous loop method"));
                        QString tipAlpha(tr("<p>Activate this option to interpolate the viscous data from the local apparent aoa, "
                                         "i.e. identical to the method used for the viscous loop in control polars</p>"));
                        m_prbViscFromAlpha->setToolTip(tipAlpha);

                        pbgType->addButton(m_prbViscFromCl);
                        pbgType->addButton(m_prbViscFromAlpha);
                    }

                    m_pchViscousLoop = new  QCheckBox(tr("Viscous loop"));
                    QString tip = tr("<p>Activate this checkbox to enable iterations until a lift distribution is found which satisfies"
                                  "both the panel method's BC and airfoil viscous lift data."
                                  "</p>"
                                  "<p>"
                                  "The method is comparable to the non-linear LLT. Its principle is described in:</p>"
                                  "<p align=center>"
                                     "Computationally Efficient Transonic and Viscous Potential Flow Aero-Structural Method "
                                     "or Rapid Multidisciplinary Design Optimization of Aeroelastic Wing Shaping Control, "
                                     "by Eric Ting and Daniel Chaparro, "
                                     "in Advanced Modeling and Simulation (AMS) Seminar Series, "
                                     "NASA Ames Research Center, June 28, 2017"
                                  "</p>"
                                  "<p>"
                                  "An additional variable dubbed \'virtual twist\' is added at each span spation to adjust the local lift."
                                  "</p>"
                                  "<p>Available for control polars (T6) only.</p>");
                    m_pchViscousLoop->setToolTip(tip);

                    pInterpolateLayout->addWidget(m_prbViscFromCl);
                    pInterpolateLayout->addWidget(m_prbViscFromAlpha);
                    pInterpolateLayout->addStretch();
                    pInterpolateLayout->addWidget(m_pchViscousLoop);
                    pInterpolateLayout->addStretch();
                }
                m_pfrInterpolated->setLayout(pInterpolateLayout);
            }

            m_pfrOntheFly = new QFrame;
            {
                QGridLayout *pOnTheFyLayout = new QGridLayout;
                {
                    QLabel *plabNCrit       = new QLabel(tr("NCrit="));
                    QLabel *plabTopTrip     = new QLabel(tr("Trip location (top)="));
                    QLabel *plabBotTrip     = new QLabel(tr("Trip location (bot)="));

                    m_pfeNCrit  = new FloatEdit;
                    m_pfeXTopTr = new FloatEdit;
                    m_pfeXBotTr = new FloatEdit;

                    m_pchTransAtHinge = new QCheckBox(tr("Force transition at hinge location"));
                    m_pchTransAtHinge->setToolTip(tr("<p>"
                                                  "Forces the laminar to turbulent transition at the hinge's location on both the top and bottom surfaces.<br>"
                                                  "The transition location is set as the most upwind position between the hinge's location "
                                                  "and the forced transition location.<br>"
                                                  "Only used in the case of flapped surfaces.<br>"
                                                  "This greatly increases the convergence success rate and the speed of XFoil calculations."
                                                  "</p>"));

                    QLabel *plabpctop = new QLabel(tr("% chord"));
                    QLabel *plabpcbot = new QLabel(tr("% chord"));

                    pOnTheFyLayout->addWidget(plabNCrit,         1, 1);
                    pOnTheFyLayout->addWidget(m_pfeNCrit,        1, 2);
                    pOnTheFyLayout->addWidget(plabTopTrip,       2, 1);
                    pOnTheFyLayout->addWidget(m_pfeXTopTr,       2, 2);
                    pOnTheFyLayout->addWidget(plabpctop,         2, 3);
                    pOnTheFyLayout->addWidget(plabBotTrip,       3, 1);
                    pOnTheFyLayout->addWidget(m_pfeXBotTr,       3, 2);
                    pOnTheFyLayout->addWidget(plabpcbot,         3, 3);
                    pOnTheFyLayout->addWidget(m_pchTransAtHinge, 4, 1, 1, 3);

                    pOnTheFyLayout->setColumnStretch(3,1);
                }
                m_pfrOntheFly->setLayout(pOnTheFyLayout);
            }

            pViscousPageLayout->addWidget(m_pchViscAnalysis);
            pViscousPageLayout->addLayout(pMethodLayout);
            pViscousPageLayout->addWidget(m_pfrInterpolated);
            pViscousPageLayout->addWidget(m_pfrOntheFly);
            pViscousPageLayout->addStretch();
        }


        m_pfrViscosity->setLayout(pViscousPageLayout);
    }

    m_pfrFuseDrag = new QFrame;
    {
        QGridLayout* pFuseDragLayout = new QGridLayout;
        {
            m_pchFuseDrag = new QCheckBox(tr("Include skin friction drag"));

            QPixmap pixmap;

            m_plabFuseDragFormula = new QLabel();
            if(DisplayOptions::isDarkMode())
                pixmap.load(":/images/Fuse_Drag_inv.png");
            else
                pixmap.load(":/images/Fuse_Drag.png");
            m_plabFuseDragFormula->setPixmap(pixmap);
            m_plabFuseDragFormula->setAlignment(Qt::AlignLeft);

            m_plabFuseWettedArea = new QLabel(tr("Wetted Area (WA) = 0"));
            m_plabFuseWettedArea->setAlignment(Qt::AlignLeft);
            m_plabFuseWettedArea->setFont(fixedfnt);

            m_plabFuseFormFactor = new QLabel(tr("Form Factor (FF) = 1"));
            m_plabFuseFormFactor->setAlignment(Qt::AlignLeft);
            m_plabFuseFormFactor->setFont(fixedfnt);

            QLabel *plabFF = new QLabel();
            if(DisplayOptions::isDarkMode())
                pixmap.load(":/images/Fuse_FF_inv.png");
            else
                pixmap.load(":/images/Fuse_FF.png");
            plabFF->setPixmap(pixmap);
            m_prbKSDrag = new QRadioButton(tr("Karman-Schoenherr implicit method"));

            QLabel *plabKS = new QLabel();
            if(DisplayOptions::isDarkMode())
                pixmap.load(":/images/Fuse_KS_inv.png");
            else
                pixmap.load(":/images/Fuse_KS.png");
            plabKS->setPixmap(pixmap);
            m_prbPSDrag = new QRadioButton(tr("Prandtl-Schlichting method"));

            m_prbCustomFuseDrag = new QRadioButton(tr("Manual input"));
            QLabel *plabCus = new QLabel(tr("Cf="));
            m_pfeCustomFF = new FloatEdit;

            QLabel *plabPS = new QLabel();
            if(DisplayOptions::isDarkMode())
                pixmap.load(":/images/Fuse_PS_inv.png");
            else
                pixmap.load(":/images/Fuse_PS.png");
            plabPS->setPixmap(pixmap);

            pFuseDragLayout->addWidget(m_pchFuseDrag,           1,1);
            pFuseDragLayout->addWidget(m_plabFuseDragFormula,   1,4, Qt::AlignCenter);
            pFuseDragLayout->addWidget(m_plabFuseWettedArea,    3,1);
            pFuseDragLayout->addWidget(m_plabFuseFormFactor,    4,1, Qt::AlignVCenter);
            pFuseDragLayout->addWidget(plabFF,                  4,4,1,2, Qt::AlignVCenter);
            pFuseDragLayout->addWidget(m_prbKSDrag,             6,1);
            pFuseDragLayout->addWidget(plabKS,                  6,4,1,2, Qt::AlignCenter);
            pFuseDragLayout->addWidget(m_prbPSDrag,             7,1);
            pFuseDragLayout->addWidget(plabPS,                  7,4,1,2, Qt::AlignCenter);
            pFuseDragLayout->addWidget(m_prbCustomFuseDrag,     8,1);
            pFuseDragLayout->addWidget(plabCus,                 8,4, Qt::AlignRight);
            pFuseDragLayout->addWidget(m_pfeCustomFF,           8,5);

            pFuseDragLayout->setRowStretch(5,5);
            pFuseDragLayout->setRowStretch(8,5);
            pFuseDragLayout->setColumnStretch(3,6);
        }
        m_pfrFuseDrag->setLayout(pFuseDragLayout);
    }

    m_pfrFlaps = new QFrame;
    {
        QVBoxLayout *pFlapLayout = new QVBoxLayout;
        {
            QHBoxLayout *pFlapSetNameLayout = new QHBoxLayout;
            {
                QLabel *plabFlapSetName = new QLabel(tr("Flap set name:"));
                m_pleFlapSetName = new QLineEdit;
                m_pleFlapSetName->setToolTip(tr("<p>Optional meta-information used to set the polar name</p>"));
                pFlapSetNameLayout->addWidget(plabFlapSetName);
                pFlapSetNameLayout->addWidget(m_pleFlapSetName);
            }

            m_pFlapTreeView = new QTreeView(this);
            m_pFlapTreeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
            m_pFlapTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);
            m_pFlapTreeView->setTabKeyNavigation(true);

            m_pFlapModel = new QStandardItemModel(this);
            m_pFlapModel->setColumnCount(2);

            QStringList labels;
            labels << tr("Wing flap") << tr("Angle (") + DEGch + tr(")");
            m_pFlapModel->setHorizontalHeaderLabels(labels);
            m_pFlapModel->setHeaderData(0, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
            m_pFlapModel->setHeaderData(1, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);

            m_pFlapTreeView->setModel(m_pFlapModel);
            QHeaderView *pHorizontalHeader = m_pFlapTreeView->header();
            pHorizontalHeader->setSectionResizeMode(0, QHeaderView::Stretch);

            m_pFlapDelegate = new CtrlTableDelegate(this);
            m_pFlapDelegate->setEditable({false, true});
            m_pFlapDelegate->setPrecision({-1,3});
            m_pFlapTreeView->setItemDelegate(m_pFlapDelegate);

            QLabel* plabNotes = new QLabel(tr("Notes:\n"
                                           "\t(1) + sign means trailing edge down\n"
                                           "\t(2) Flaps are numbered from left tip to right tip\n"
                                           "\t(3) Use in conjunction with THIN surfaces\n"
                                           "\t(4) Use in conjunction with XFoil on the fly calculations"));

            QLabel *pFlow5Link = new QLabel;
            pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Analysis/Flaps.html>https://flow5.tech/docs/flow5_doc/Analysis/Flaps.html</a>");
            pFlow5Link->setOpenExternalLinks(true);
            pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
            pFlow5Link->setAlignment(Qt::AlignVCenter| Qt::AlignLeft);
            pFlow5Link->setAttribute(Qt::WA_NoSystemBackground);

            pFlapLayout->addLayout(pFlapSetNameLayout);
            pFlapLayout->addWidget(m_pFlapTreeView);
            pFlapLayout->addWidget(plabNotes);
            pFlapLayout->addWidget(pFlow5Link);
        }
        m_pfrFlaps->setLayout(pFlapLayout);
    }
}


void PlanePolarDlg::fillFlapControls()
{
    PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl)
    {
        m_pFlapModel->setRowCount(0);
        return;
    }

    if(s_WPolar.nFlapCtrls() != pPlaneXfl->nWings())
        s_WPolar.resizeFlapCtrls(pPlaneXfl); // cleaning up

    m_pleFlapSetName->setText(QString::fromStdString(s_WPolar.flapCtrlsSetName()));
    m_pleFlapSetName->selectAll();

    QStandardItem *pRootItem = m_pFlapModel->invisibleRootItem();
    pRootItem->setText(tr("Wings"));

    QModelIndex index;
    for(int iw=0; iw<pPlaneXfl->nWings(); iw++)
    {
        WingXfl const *pWing = pPlaneXfl->wingAt(iw);
        AngleControl const &avlc = s_WPolar.flapCtrls(iw);
        QList<QStandardItem *> pWingRowItems;
        pWingRowItems << new QStandardItem(QString::fromStdString(pWing->name())) << new QStandardItem(QString());
        pRootItem->appendRow(pWingRowItems);
        pWingRowItems[0]->setFlags(Qt::NoItemFlags);
        pWingRowItems[1]->setFlags(Qt::NoItemFlags);

        for(int k=0; k<pWing->nFlaps(); k++)
        {
            QList<QStandardItem *>  pFlapRowItems;
            pFlapRowItems << new QStandardItem(QString::asprintf("flap_%d", k+1)) << new QStandardItem(QString::asprintf("%g", avlc.value(k)));
            pWingRowItems[0]->appendRow( pFlapRowItems);

            index = m_pFlapModel->indexFromItem(pFlapRowItems[1]);
            m_pFlapModel->setData(index, Qt::AlignRight, Qt::TextAlignmentRole);
            pFlapRowItems[0]->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            pFlapRowItems[1]->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        }
    }
    m_pFlapTreeView->expandAll();
}


void PlanePolarDlg::connectSignals()
{    
    connectBaseSignals();

    connect(m_pchFuseDrag,         SIGNAL(clicked(bool)),           SLOT(onFuseDrag()));
    connect(m_prbKSDrag,           SIGNAL(clicked(bool)),           SLOT(onFuseDrag()));
    connect(m_prbPSDrag,           SIGNAL(clicked(bool)),           SLOT(onFuseDrag()));
    connect(m_prbCustomFuseDrag,   SIGNAL(clicked(bool)),           SLOT(onFuseDrag()));

    connect(m_pchAutoInertia,      SIGNAL(clicked(bool)),           SLOT(onPlaneInertia()));

    connect(m_pchGround,           SIGNAL(clicked(bool)),           SLOT(onGroundEffect()));
    connect(m_pchFreeSurf,         SIGNAL(clicked(bool)),           SLOT(onGroundEffect()));
    connect(m_pfeHeight,           SIGNAL(floatChanged(float)),     SLOT(onGroundEffect()));

    connect(m_pchViscAnalysis,     SIGNAL(clicked()),               SLOT(onViscous()));
    connect(m_prbViscInterpolated, SIGNAL(clicked()),               SLOT(onViscous()));
    connect(m_prbViscOnTheFly,     SIGNAL(clicked()),               SLOT(onViscous()));
    connect(m_prbViscFromAlpha,    SIGNAL(clicked()),               SLOT(onViscous()));
    connect(m_prbViscFromCl,       SIGNAL(clicked()),               SLOT(onViscous()));

    connect(m_prbThinSurfaces,     SIGNAL(clicked(bool)),           SLOT(onWingSurfaces()));
    connect(m_prbThickSurfaces,    SIGNAL(clicked(bool)),           SLOT(onWingSurfaces()));

    connect(m_prbArea1,            SIGNAL(clicked(bool)),           SLOT(onArea()));
    connect(m_prbArea2,            SIGNAL(clicked(bool)),           SLOT(onArea()));
    connect(m_prbArea3,            SIGNAL(clicked(bool)),           SLOT(onArea()));
    connect(m_pchOtherWings,       SIGNAL(clicked(bool)),           SLOT(onArea()));

    connect(m_pfeRefArea,          SIGNAL(editingFinished()),       SLOT(onArea()));
    connect(m_pfeRefSpan,          SIGNAL(editingFinished()),       SLOT(onArea()));
    connect(m_pfeRefChord,         SIGNAL(editingFinished()),       SLOT(onArea()));

    connect(m_pleFlapSetName,      &QLineEdit::editingFinished,     this,  &PlanePolarDlg::onFlapControls);
    connect(m_pFlapDelegate,       &CtrlTableDelegate::closeEditor, this,  &PlanePolarDlg::onFlapControls);
}


void PlanePolarDlg::onReset()
{
    s_WPolar.setDefaultSpec(m_pPlane);
    initPolar3dDlg(m_pPlane, &s_WPolar);
    m_bAutoName = true;
    m_pchAutoName->setChecked(true);
    setPolar3dName();
}


void PlanePolarDlg::initPolar3dDlg(const Plane *pPlane, PlanePolar const *pWPolar)
{
    m_pPlane = pPlane;

    if(m_pPlane)
        s_WPolar.setPlaneName(m_pPlane->name());
    else
        s_WPolar.setPlaneName("");
    m_plabParentObjectName->setText(QString::fromStdString(s_WPolar.planeName()));

    if(pWPolar)
    {
        m_bAutoName = false;
        m_pchAutoName->setChecked(false);
        s_WPolar.duplicateSpec(pWPolar);
        s_WPolar.setName(pWPolar->name());
        s_WPolar.setPlaneName(pWPolar->planeName());
    }
    else
    {
        m_pchAutoName->setChecked(true);
    }

    m_plePolarName->setText(QString::fromStdString(s_WPolar.name()));

    checkMethods();

    m_pchIncludeWingTipMi->setChecked(s_WPolar.bWingTipMi());

    m_pchViscAnalysis->setEnabled(true);
    m_pchViscAnalysis->setChecked(s_WPolar.isViscous());
    m_prbViscInterpolated->setChecked(s_WPolar.isViscInterpolated());
    m_prbViscOnTheFly->setChecked(!s_WPolar.isViscInterpolated());
    m_prbViscFromCl->setChecked(s_WPolar.isViscFromCl());
    m_prbViscFromAlpha->setChecked(!s_WPolar.isViscFromCl());
    m_pfeNCrit ->setValuef(s_WPolar.NCrit());
    m_pfeXTopTr->setValue(s_WPolar.XTrTop()*100.0);
    m_pfeXBotTr->setValue(s_WPolar.XTrBot()*100.0);
    m_pchTransAtHinge->setChecked(s_WPolar.bTransAtHinge());
    m_pchViscousLoop->setChecked(s_WPolar.bViscousLoop());

    m_prbArea1->setChecked(s_WPolar.referenceDim()==xfl::PLANFORM);
    m_prbArea2->setChecked(s_WPolar.referenceDim()==xfl::PROJECTED);
    m_prbArea3->setChecked(s_WPolar.referenceDim()==xfl::CUSTOM);

    if(pPlane)
    {
        if(pPlane->hasOtherWing())
        {
            m_pchOtherWings->setEnabled(true);
            m_pchOtherWings->setChecked(s_WPolar.bIncludeOtherWingAreas());
        }
        else
        {
            m_pchOtherWings->setEnabled(false);
            m_pchOtherWings->setChecked(false);
        }
    }
    else
    {
        m_pchOtherWings->setEnabled(true);
        m_pchOtherWings->setChecked(s_WPolar.bIncludeOtherWingAreas());
    }

    if(m_prbArea1->isChecked())
    {
        if(pPlane)
        {
            s_WPolar.setReferenceArea(m_pPlane->planformArea(s_WPolar.bIncludeOtherWingAreas()));
            s_WPolar.setReferenceSpanLength(m_pPlane->planformSpan());
            s_WPolar.setReferenceChordLength(m_pPlane->mac());
        }
        else
        {
            s_WPolar.setReferenceArea(0);
            s_WPolar.setReferenceSpanLength(0);
            s_WPolar.setReferenceChordLength(0);
        }
        m_pfeRefArea->setValue(s_WPolar.referenceArea()*Units::m2toUnit());
        m_pfeRefSpan->setValue(s_WPolar.referenceSpanLength()*Units::mtoUnit());
    }
    else if(m_prbArea2->isChecked())
    {
        if(pPlane)
        {
            s_WPolar.setReferenceArea(m_pPlane->projectedArea(s_WPolar.bIncludeOtherWingAreas()));
            s_WPolar.setReferenceSpanLength(m_pPlane->projectedSpan());
            s_WPolar.setReferenceChordLength(m_pPlane->mac());
        }
        else
        {
            s_WPolar.setReferenceArea(0);
            s_WPolar.setReferenceSpanLength(0);
            s_WPolar.setReferenceChordLength(0);
        }
        m_pfeRefArea->setValue(s_WPolar.referenceArea()*Units::m2toUnit());
        m_pfeRefSpan->setValue(s_WPolar.referenceSpanLength()*Units::mtoUnit());
    }
    else if(m_prbArea3->isChecked())
    {
        m_pfeRefArea->setValue(s_WPolar.referenceArea()*Units::m2toUnit());
        m_pfeRefSpan->setValue(s_WPolar.referenceSpanLength()*Units::mtoUnit());
    }
    m_pfeRefChord->setValue(s_WPolar.referenceChordLength()*Units::mtoUnit());

    m_pchAutoInertia->setChecked(s_WPolar.bAutoInertia());

    m_pfeDensity->setValue(s_WPolar.density()*Units::densitytoUnit());
    m_pfeViscosity->setValue(s_WPolar.viscosity()*Units::viscositytoUnit());

    //initialize ground data
    m_pchGround->setChecked(s_WPolar.bGroundEffect());
    m_pchFreeSurf->setChecked(s_WPolar.bFreeSurfaceEffect());
    m_pfeHeight->setValue(s_WPolar.groundHeight()*Units::mtoUnit());
    m_pfeHeight->setEnabled(s_WPolar.bGroundEffect() || s_WPolar.bFreeSurfaceEffect());

    s_WPolar.setNXWakePanel4(std::max(s_WPolar.NXWakePanel4(),1));
    s_WPolar.setTotalWakeLengthFactor(std::max(s_WPolar.totalWakeLengthFactor(),1.0));
    s_WPolar.setWakePanelFactor(std::max(s_WPolar.wakePanelFactor(),1.0));

    m_prbPanelWake->setChecked(!s_WPolar.bVortonWake());
    m_prbVortonWake->setChecked(s_WPolar.bVortonWake());
    m_pieNXWakePanels->setValue(s_WPolar.NXWakePanel4());
    m_pfeWakeLength->setValue(s_WPolar.totalWakeLengthFactor());
    m_pfeWakePanelFactor->setValue(s_WPolar.wakePanelFactor());

    m_pfeVPWBufferWake->setValue(s_WPolar.bufferWakeFactor());
    m_pfeVPWLength->setValue(s_WPolar.VPWMaxLength());
    m_pfeVortonCoreSize->setValue(s_WPolar.vortonCoreSize());
    m_pfeVortonL0->setValue(s_WPolar.vortonL0());
    m_pieVPWIterations->setValue(s_WPolar.VPWIterations());
    setVPWUnits(s_WPolar);

    if((m_pPlane && m_pPlane->hasFuse()) || !m_pPlane)
    {
        PlaneXfl const * pPlaneXfl = dynamic_cast<PlaneXfl const*>(pPlane);
        m_pchIncludeFuseMi->setChecked(s_WPolar.bFuseMi());
        m_pchFuseDrag->setEnabled(true);
        m_pchFuseDrag->setChecked(           s_WPolar.hasFuseDrag());
        m_prbPSDrag->setChecked((            s_WPolar.fuseDragMethod()==PlanePolar::PRANDTLSCHLICHTING));
        m_prbKSDrag->setChecked((            s_WPolar.fuseDragMethod()==PlanePolar::KARMANSCHOENHERR));
        m_prbCustomFuseDrag->setChecked((    s_WPolar.fuseDragMethod()==PlanePolar::MANUALFUSECF));
        m_prbPSDrag->setEnabled(             s_WPolar.hasFuseDrag());
        m_prbKSDrag->setEnabled(             s_WPolar.hasFuseDrag());
        m_prbCustomFuseDrag->setEnabled(     s_WPolar.hasFuseDrag());
        m_pfeCustomFF->setEnabled(           s_WPolar.hasFuseDrag() && m_prbCustomFuseDrag->isChecked());
        m_pfeCustomFF->setValue(             s_WPolar.customFuseCf());

        m_plabFuseWettedArea->setEnabled(s_WPolar.hasFuseDrag());

        if(pPlaneXfl && pPlaneXfl->fuseAt(0))
        {
            double wa = pPlaneXfl->fuseAt(0)->wettedArea();
            QString strong;
            strong = QString::asprintf("%.2g", wa*Units::m2toUnit());
            m_plabFuseWettedArea->setText(tr("Wetted Area (WA) = %1 %2").arg(strong).arg(Units::areaUnitQLabel()));
            strong = QString::asprintf("%.2g", pPlaneXfl->fuseAt(0)->formFactor());
            m_plabFuseFormFactor->setText(tr("Form Factor (FF) = %1").arg(strong));
        }
    }
    else
    {
        m_pfrFuseDrag->setEnabled(false);
    }
    m_plabFuseFormFactor->setEnabled(s_WPolar.hasFuseDrag());
    m_plabFuseFormFactor->setEnabled(s_WPolar.hasFuseDrag());
    m_plabFuseDragFormula->setEnabled(s_WPolar.hasFuseDrag());

    m_pExtraDragWt->initWt(&s_WPolar);

    m_pgbWingSurf->setVisible(true);
    m_pgbFuseMi->setVisible(true);
    m_pgbHullBox->setVisible(false);
}


void PlanePolarDlg::readData()
{
    readMethodData();
    readViscousData();

    m_pExtraDragWt->setExtraDragData(s_WPolar);

    readFuseDragData();
    readWakeData(s_WPolar);

    readFluidProperties();
    readReferenceDimensions();

    setVPWUnits(s_WPolar);
//    setDensity(s_WPolar);
}


void PlanePolarDlg::readMethodData()
{
    if (m_prbLLTMethod->isChecked())
    {
        s_WPolar.setAnalysisMethod(xfl::LLT);
        s_WPolar.setViscous(true);
        s_WPolar.setIgnoreBodyPanels(true);
        s_WPolar.setIncludeWingTipMi(false);
        s_WPolar.setIncludeFuseMi(false);
        m_pchViscAnalysis->setEnabled(true);
        m_prbViscFromCl->setEnabled(true);
    }
    else if (m_prbVLM1Method->isChecked())
    {
        s_WPolar.setVLM1();
        s_WPolar.setIgnoreBodyPanels(true);
        s_WPolar.setIncludeWingTipMi(false);
        s_WPolar.setIncludeFuseMi(false);
    }
    else if (m_prbVLM2Method->isChecked())
    {
        s_WPolar.setVLM2();
        s_WPolar.setIgnoreBodyPanels(true);
        s_WPolar.setIncludeWingTipMi(false);
        s_WPolar.setIncludeFuseMi(false);
    }
    else if (m_prbQuadMethod->isChecked())
    {
        s_WPolar.setAnalysisMethod(xfl::QUADS);
        s_WPolar.setIgnoreBodyPanels(false);
        s_WPolar.setIncludeWingTipMi(m_pchIncludeWingTipMi->isChecked());
        s_WPolar.setIncludeFuseMi(m_pchIncludeFuseMi->isChecked());
    }
    else if (m_prbTriUniMethod->isChecked())
    {
        s_WPolar.setAnalysisMethod(xfl::TRIUNIFORM);
        s_WPolar.setIgnoreBodyPanels(false);
        s_WPolar.setIncludeWingTipMi(m_pchIncludeWingTipMi->isChecked());
        s_WPolar.setIncludeFuseMi(m_pchIncludeFuseMi->isChecked());
    }
    else if (m_prbTriLinMethod->isChecked())
    {
        s_WPolar.setAnalysisMethod(xfl::TRILINEAR);
        s_WPolar.setIgnoreBodyPanels(false);
        s_WPolar.setIncludeWingTipMi(m_pchIncludeWingTipMi->isChecked());
        s_WPolar.setIncludeFuseMi(m_pchIncludeFuseMi->isChecked());
    }

    s_WPolar.setTrefftz(true);
    s_WPolar.setBoundaryCondition(xfl::DIRICHLET);
}


void PlanePolarDlg::readViscousData()
{
    s_WPolar.setViscous(m_pchViscAnalysis->isChecked());
    s_WPolar.setViscInterpolated(m_prbViscInterpolated->isChecked());
    s_WPolar.setViscFromCl(m_prbViscFromCl->isChecked());

    s_WPolar.setNCrit(m_pfeNCrit->value());
    s_WPolar.setXTrTop(m_pfeXTopTr->value()/100.0);
    s_WPolar.setXTrBot(m_pfeXBotTr->value()/100.0);
    s_WPolar.setTransAtHinge(m_pchTransAtHinge->isChecked());

    s_WPolar.setViscousLoop(m_pchViscousLoop->isChecked());
}


void PlanePolarDlg::readReferenceDimensions()
{
    s_WPolar.setIncludeOtherWingAreas(m_pchOtherWings->isChecked());
    if(m_prbArea1->isChecked())
    {
        s_WPolar.setReferenceDim(xfl::PLANFORM);
        if(m_pPlane)
        {
            s_WPolar.setReferenceArea(m_pPlane->planformArea(s_WPolar.bIncludeOtherWingAreas()));
            s_WPolar.setReferenceSpanLength(m_pPlane->planformSpan());
            s_WPolar.setReferenceChordLength(m_pPlane->mac());
        }
        else
        {
            s_WPolar.setReferenceArea(0.0);
            s_WPolar.setReferenceSpanLength(0.0);
            s_WPolar.setReferenceChordLength(0.0);
        }
    }
    else if(m_prbArea2->isChecked())
    {
        s_WPolar.setReferenceDim(xfl::PROJECTED);
        if(m_pPlane)
        {
            s_WPolar.setReferenceArea(m_pPlane->projectedArea(s_WPolar.bIncludeOtherWingAreas()));
            s_WPolar.setReferenceSpanLength(m_pPlane->projectedSpan());
            s_WPolar.setReferenceChordLength(m_pPlane->mac());
        }
        else
        {
            s_WPolar.setReferenceArea(0.0);
            s_WPolar.setReferenceSpanLength(0.0);
            s_WPolar.setReferenceChordLength(0.0);
        }
    }
    else if(m_prbArea3->isChecked())
    {
        s_WPolar.setReferenceDim(xfl::CUSTOM);
        s_WPolar.setReferenceArea(       m_pfeRefArea->value()  /Units::m2toUnit());
        s_WPolar.setReferenceSpanLength( m_pfeRefSpan->value()  /Units::mtoUnit());
        s_WPolar.setReferenceChordLength(m_pfeRefChord->value() /Units::mtoUnit());
    }
}


bool PlanePolarDlg::checkWPolarData()
{
    if(s_WPolar.isVLM()) s_WPolar.setThinSurfaces(true);

    if(!m_plePolarName->text().length())
    {
        QMessageBox::warning(this, "Warning", "Must enter a name for the polar");
        m_plePolarName->setFocus();
        return false;
    }
    s_WPolar.setName(m_plePolarName->text().toStdString());

    if(s_WPolar.referenceDim()==xfl::CUSTOM)
    {
        if(s_WPolar.referenceArea()<1.-6 || s_WPolar.referenceSpanLength()<1.e-4 || s_WPolar.referenceChordLength()<1.e-4)
        {
            QMessageBox::warning(this, "Warning", "Invalid reference dimensions");
            return false;
        }
    }

    if(s_WPolar.bVortonWake())
    {
        if(!s_WPolar.isType6() || s_WPolar.isLLTMethod() || s_WPolar.isVLM())
        {
            QMessageBox::warning(this, "Warning", "The vorton wake is only compatible with T6 analyses and panel methods");
            return false;
        }
    }
    return true;
}


void PlanePolarDlg::enableControls()
{
    if(m_pPlane && !m_pPlane->isXflType())
    {
        if(!s_WPolar.isTriangleMethod()) s_WPolar.setAnalysisMethod(xfl::TRIUNIFORM);
        if(s_WPolar.isViscous())
        {
            s_WPolar.setViscous(false);
            m_pchViscAnalysis->setChecked(false);
            m_pfrViscosity->setEnabled(false);
        }
        if(s_WPolar.hasFuseDrag()) s_WPolar.setIncludeFuseDrag(false);
        s_WPolar.setThickSurfaces(true);

        m_prbThinSurfaces->setChecked(false);
        m_prbThickSurfaces->setChecked(true);
        m_prbThinSurfaces->setEnabled(false);
        m_prbThickSurfaces->setEnabled(false);

        m_prbLLTMethod->setEnabled(false);
        m_prbQuadMethod->setEnabled(false);
        m_prbVLM1Method->setEnabled(false);
        m_prbVLM2Method->setEnabled(false);
        m_pchIncludeFuseMi->setEnabled(false);
    }

    if(m_prbLLTMethod->isChecked())
    {
        m_prbThinSurfaces->setEnabled(false);
        m_prbThickSurfaces->setEnabled(false);
        m_prbThinSurfaces->setChecked(true);
    }
    else if(m_prbVLM1Method->isChecked() || m_prbVLM2Method->isChecked())
    {
        m_prbThinSurfaces->setEnabled(false);
        m_prbThickSurfaces->setEnabled(false);
        m_prbThinSurfaces->setChecked(true);
    }
    else
    {
        m_prbThinSurfaces->setEnabled(true);
        m_prbThickSurfaces->setEnabled(true);
        m_prbThinSurfaces->setChecked(s_WPolar.bThinSurfaces());
        m_prbThickSurfaces->setChecked(!s_WPolar.bThinSurfaces());
    }

    m_plePolarName->setEnabled(!m_pchAutoName->isChecked());

    m_pfeRefArea->setEnabled(m_prbArea3->isChecked());
    m_pfeRefChord->setEnabled(m_prbArea3->isChecked());
    m_pfeRefSpan->setEnabled(m_prbArea3->isChecked());

    m_pieNXWakePanels->setEnabled(true);
    m_pfeWakeLength->setEnabled(true);
    m_pfeWakePanelFactor->setEnabled(true);

    m_pgbFlatWakePanels->setEnabled(!s_WPolar.bVortonWake());
    m_pgbVortonWake->setEnabled(s_WPolar.bVortonWake());

    bool bVisc = m_pchViscAnalysis->isChecked();
    m_prbViscInterpolated->setEnabled(bVisc);
    m_prbViscOnTheFly->setEnabled(bVisc);

    m_pfrInterpolated->setEnabled(bVisc && m_prbViscInterpolated->isChecked());
    m_pfrOntheFly->setEnabled(bVisc && m_prbViscOnTheFly->isChecked());


    m_pchViscousLoop->setEnabled(s_WPolar.isType6() && m_pchViscAnalysis->isChecked() && m_prbViscInterpolated->isChecked());

    m_pfrInterpolated->setVisible(m_prbViscInterpolated->isChecked());
    m_pfrOntheFly->setVisible(m_prbViscOnTheFly->isChecked());

    m_prbViscFromCl->setEnabled(m_pchViscAnalysis->isChecked() && m_prbViscInterpolated->isChecked());
    m_prbViscFromAlpha->setEnabled(m_pchViscAnalysis->isChecked() && m_prbViscInterpolated->isChecked());

}


void PlanePolarDlg::setPolar3dName()
{
    if(!m_bAutoName) return;
    s_WPolar.setName(PlPolarNameMaker::makeName(m_pPlane, &s_WPolar).toStdString());
    m_plePolarName->setText(QString::fromStdString(s_WPolar.name()));
}


void PlanePolarDlg::onNameOptions()
{
    WPolarAutoNameDlg dlg;
    dlg.initDialog(s_WPolar);
    dlg.exec();
    setPolar3dName();
}


void PlanePolarDlg::onViscous()
{
    readViscousData();

    setPolar3dName();
    enableControls();
}


void PlanePolarDlg::checkMethods()
{
    if(s_WPolar.isLLTMethod())
    {
        m_prbLLTMethod->setChecked(true);
    }
    else if(s_WPolar.isVLM())
    {
        m_prbVLM1Method->setChecked( s_WPolar.isVLM1());
        m_prbVLM2Method->setChecked(!s_WPolar.isVLM1());
    }
    else if(s_WPolar.isPanel4Method())
    {
        m_prbQuadMethod->setChecked(true);
    }
    else if(s_WPolar.isTriUniformMethod())
    {
        m_prbTriUniMethod->setChecked(true);
    }
    else if(s_WPolar.isTriLinearMethod())
    {
        m_prbTriLinMethod->setChecked(true);
    }
}


void PlanePolarDlg::readFluidProperties()
{
    s_WPolar.setDensity(m_pfeDensity->value() / Units::densitytoUnit());
    s_WPolar.setViscosity(m_pfeViscosity->value() / Units::viscositytoUnit());

    s_WPolar.setGroundEffect(m_pchGround->isChecked());
    s_WPolar.setGroundHeight(m_pfeHeight->value()/Units::mtoUnit());
}


void PlanePolarDlg::readFuseDragData()
{
    if(!m_pPlane) return;
    if(m_pPlane->hasFuse())
    {
        s_WPolar.setIncludeFuseDrag(m_pchFuseDrag->isChecked());
        //        s_WPolar.setFuseFormFactor(m_pPlane->fuse(0)->formFactor());
        if     (m_prbKSDrag->isChecked())   s_WPolar.setFuseDragMethod(PlanePolar::KARMANSCHOENHERR);
        else if(m_prbPSDrag->isChecked()) s_WPolar.setFuseDragMethod(PlanePolar::PRANDTLSCHLICHTING);
        else if(m_prbCustomFuseDrag->isChecked())
        {
            s_WPolar.setFuseDragMethod(PlanePolar::MANUALFUSECF);
            s_WPolar.setCustomFuseCf(m_pfeCustomFF->value());
        }
    }
    else
    {
        s_WPolar.setIncludeFuseDrag(false);
        //        s_WPolar.setFuseFormFactor(1.0);
    }
}


void PlanePolarDlg::onGroundEffect()
{
    QCheckBox *pSenderBox = qobject_cast<QCheckBox *>(sender());

    if(pSenderBox==m_pchGround)
    {
        if(m_pchGround->isChecked()) m_pchFreeSurf->setChecked(false);
    }
    else if(pSenderBox==m_pchFreeSurf)
    {
        if(m_pchFreeSurf->isChecked()) m_pchGround->setChecked(false);
    }
    s_WPolar.setGroundEffect(m_pchGround->isChecked());
    s_WPolar.setFreeSurfaceEffect(m_pchFreeSurf->isChecked());
    s_WPolar.setGroundHeight(m_pfeHeight->value()/Units::mtoUnit());
    m_pfeHeight->setEnabled(s_WPolar.bGroundEffect() || s_WPolar.bFreeSurfaceEffect());
    setPolar3dName();
}


void PlanePolarDlg::onArea()
{
    s_WPolar.setIncludeOtherWingAreas(m_pchOtherWings->isChecked());

    if(m_prbArea1->isChecked())
    {
        s_WPolar.setReferenceDim(xfl::PLANFORM);
        if(m_pPlane)
        {
            m_pfeRefArea->setValue(m_pPlane->planformArea(s_WPolar.bIncludeOtherWingAreas())*Units::m2toUnit());
            m_pfeRefChord->setValue(m_pPlane->mac()*Units::mtoUnit());
            m_pfeRefSpan->setValue(m_pPlane->planformSpan()*Units::mtoUnit());
        }
        else
        {
            m_pfeRefArea->setValue(0);
            m_pfeRefChord->setValue(0);
            m_pfeRefSpan->setValue(0);
        }
    }
    else if(m_prbArea2->isChecked())
    {
        s_WPolar.setReferenceDim(xfl::PROJECTED);
        if(m_pPlane)
        {
            m_pfeRefArea->setValue(m_pPlane->projectedArea(s_WPolar.bIncludeOtherWingAreas())*Units::m2toUnit());
            m_pfeRefSpan->setValue(m_pPlane->projectedSpan()*Units::mtoUnit());
            m_pfeRefChord->setValue(m_pPlane->mac()*Units::mtoUnit());
        }
        else
        {
            m_pfeRefArea->setValue(0);
            m_pfeRefChord->setValue(0);
            m_pfeRefSpan->setValue(0);
        }
    }
    else if(m_prbArea3->isChecked())
    {
        s_WPolar.setReferenceDim(xfl::CUSTOM);
    }

    setPolar3dName();
    enableControls();
}


void PlanePolarDlg::onAeroData()
{
    AeroDataDlg dlg;
    if(dlg.exec() == QDialog::Accepted)
    {
        s_WPolar.setDensity(dlg.airDensity());
        s_WPolar.setViscosity(dlg.kinematicViscosity());
        m_pfeDensity->setValue(  s_WPolar.density()  * Units::densitytoUnit());
        m_pfeViscosity->setValue(s_WPolar.viscosity()* Units::viscositytoUnit());
    }
}


void PlanePolarDlg::onVortonWake()
{
    s_WPolar.setVortonWake(m_prbVortonWake->isChecked());
    m_pgbFlatWakePanels->setEnabled(!s_WPolar.bVortonWake());
    m_pgbVortonWake->setEnabled(s_WPolar.bVortonWake());
    setPolar3dName();
}


void PlanePolarDlg::onFuseDrag()
{
    m_pfrFuseDrag->setEnabled(m_pPlane->hasFuse());

    s_WPolar.setIncludeFuseDrag(m_pchFuseDrag->isChecked());
    m_plabFuseWettedArea->setEnabled(s_WPolar.hasFuseDrag());
    m_plabFuseFormFactor->setEnabled(s_WPolar.hasFuseDrag());
    m_plabFuseDragFormula->setEnabled(s_WPolar.hasFuseDrag());

    m_prbKSDrag->setEnabled(m_pchFuseDrag->isChecked());
    m_prbPSDrag->setEnabled(m_pchFuseDrag->isChecked());
    m_prbCustomFuseDrag->setEnabled(m_pchFuseDrag->isChecked());
    m_pfeCustomFF->setEnabled(m_pchFuseDrag->isChecked() && m_prbCustomFuseDrag->isChecked());

    setPolar3dName();
}


void PlanePolarDlg::onWingSurfaces()
{
    s_WPolar.setThinSurfaces(m_prbThinSurfaces->isChecked());
    enableControls();
    setPolar3dName();
}


void PlanePolarDlg::fillInertiaPage()
{
    m_pchAutoInertia->setChecked(s_WPolar.bAutoInertia());
    m_pfePlaneMass->setValue(s_WPolar.mass()*Units::kgtoUnit());
    m_pfeXCoG->setValue(s_WPolar.CoG().x*Units::mtoUnit());
    m_pfeZCoG->setValue(s_WPolar.CoG().z*Units::mtoUnit());
    m_pfeIxx->setValue(s_WPolar.Ixx()*Units::kgm2toUnit());
    m_pfeIyy->setValue(s_WPolar.Iyy()*Units::kgm2toUnit());
    m_pfeIzz->setValue(s_WPolar.Izz()*Units::kgm2toUnit());
    m_pfeIxz->setValue(s_WPolar.Ixz()*Units::kgm2toUnit());
}


void PlanePolarDlg::readInertiaData()
{
    if(m_pchAutoInertia->isChecked() && m_pPlane)
    {
        s_WPolar.setAutoInertia(true);
        s_WPolar.setInertia(m_pPlane->inertia());
    }
    else
    {
        s_WPolar.setAutoInertia(false);
        s_WPolar.setMass(m_pfePlaneMass->value()/Units::kgtoUnit());
        s_WPolar.setCoGx(m_pfeXCoG->value()/Units::mtoUnit());
        s_WPolar.setCoGz(m_pfeZCoG->value()/Units::mtoUnit());
        s_WPolar.setIxx(m_pfeIxx->value()/Units::kgm2toUnit());
        s_WPolar.setIyy(m_pfeIyy->value()/Units::kgm2toUnit());
        s_WPolar.setIzz(m_pfeIzz->value()/Units::kgm2toUnit());
        s_WPolar.setIxz(m_pfeIxz->value()/Units::kgm2toUnit());
    }
}


void PlanePolarDlg::onPlaneInertia()
{
    readInertiaData();

    if(m_pPlane && m_pchAutoInertia->isChecked())
    {
        fillInertiaPage();
    }

    setPolar3dName();
    enableControls();
}


void PlanePolarDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("PlanePolarDlg");
    {
        s_WPolar.setAutoInertia(settings.value("StabPolarAutoInertia", true).toBool());
        s_WPolar.setMass(       settings.value("StabPolarMass", 0.0).toDouble());
        double x  = settings.value("StabPolarCoGx", 0.0).toDouble();
        double y  = settings.value("StabPolarCoGy", 0.0).toDouble();
        double z  = settings.value("StabPolarCoGz", 0.0).toDouble();
        s_WPolar.setCoG({x,y,z});
        s_WPolar.setIxx(settings.value("StabPolarCoGIxx", 0.0).toDouble());
        s_WPolar.setIyy(settings.value("StabPolarCoGIyy", 0.0).toDouble());
        s_WPolar.setIzz(settings.value("StabPolarCoGIzz", 0.0).toDouble());
        s_WPolar.setIxz(settings.value("StabPolarCoGIxz", 0.0).toDouble());
        s_WPolar.setAutoInertia(settings.value("bAutoInertia", false).toBool());

        s_Geometry = settings.value("WindowGeometry").toByteArray();
    }
    settings.endGroup();
}


void PlanePolarDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("PlanePolarDlg");
    {
        settings.setValue("bVLM1",                s_WPolar.isVLM1());
        settings.setValue("bAutoInertia",         s_WPolar.bAutoInertia());
        settings.setValue("StabPolarAutoInertia", s_WPolar.bAutoInertia());
        settings.setValue("StabPolarMass",        s_WPolar.mass());
        settings.setValue("StabPolarCoGx",        s_WPolar.CoG().x);
        settings.setValue("StabPolarCoGy",        s_WPolar.CoG().y);
        settings.setValue("StabPolarCoGz",        s_WPolar.CoG().z);
        settings.setValue("StabPolarCoGIxx",      s_WPolar.Ixx());
        settings.setValue("StabPolarCoGIyy",      s_WPolar.Iyy());
        settings.setValue("StabPolarCoGIzz",      s_WPolar.Izz());
        settings.setValue("StabPolarCoGIxz",      s_WPolar.Ixz());


        settings.setValue("WindowGeometry", s_Geometry);
    }
    settings.endGroup();
}


void PlanePolarDlg::onFlapControls()
{
    s_WPolar.clearFlapCtrls();

    PlaneXfl const *pPlaneXfl = dynamic_cast<PlaneXfl const*>(m_pPlane);
    if(!pPlaneXfl) return;

    s_WPolar.resizeFlapCtrls(pPlaneXfl);

    QString setname = m_pleFlapSetName->text();

    for(int iw=0; iw<m_pFlapModel->rowCount(); iw++)
    {
        QStandardItem *pWingItem = m_pFlapModel->item(iw);
        QModelIndex wingindex = m_pFlapModel->index(iw,0);

        WingXfl const *pWing = pPlaneXfl->wingAt(iw);

        if(iw>= s_WPolar.nFlapCtrls()) continue; //error

        AngleControl &avlc = s_WPolar.flapCtrls(iw);
        avlc.setName(setname.toStdString()); // optional meta-information
        avlc.resizeValues(pWing->nFlaps());

        for(int iflap=0; iflap<pWingItem->rowCount(); iflap++)
        {
            double flapangle =  m_pFlapModel->index(iflap, 1, wingindex).data().toDouble();
            avlc.setValue(iflap, flapangle);
        }
    }

    setPolar3dName();
}

