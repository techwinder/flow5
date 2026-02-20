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



#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include <QTime>
#include <QApplication>

#include <QPushButton>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QColorDialog>
#include <QHeaderView>
#include <QButtonGroup>
#include <QToolBar>


#include <api/constants.h>
#include <api/objects_global.h>
#include <api/occ_globals.h>
#include <api/sail.h>
#include <api/sailnurbs.h>
#include <api/trimesh.h>
#include <api/units.h>


#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/controls/w3dprefs.h>
#include <interfaces/editors/boatedit/saildlg.h>
#include <interfaces/editors/boatedit/sailscaledlg.h>
#include <interfaces/editors/boatedit/sailsectionview.h>
#include <interfaces/editors/scaledlg.h>
#include <interfaces/editors/translatedlg.h>
#include <interfaces/exchange/cadexportdlg.h>
#include <interfaces/exchange/stlwriterdlg.h>
#include <interfaces/mesh/afmesher.h>
#include <interfaces/mesh/gmesherwt.h>
#include <interfaces/mesh/mesherwt.h>
#include <interfaces/mesh/meshevent.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dsailview.h>
#include <interfaces/widgets/color/colorbtn.h>
#include <interfaces/widgets/customdlg/doublevaluedlg.h>
#include <interfaces/widgets/customdlg/helpimgdlg.h>
#include <interfaces/widgets/customdlg/intvaluesdlg.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <api/xmlsailwriter.h>
#include <modules/xobjects.h>

Quaternion SailDlg::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);

QByteArray SailDlg::s_WindowGeometry;
QByteArray SailDlg::s_HSplitterSizes;
QByteArray SailDlg::s_ExtSplitterSizes;
QByteArray SailDlg::s_IntSplitterSizes;
QByteArray SailDlg::s_VSplitterSizes;
QByteArray SailDlg::s_TableSplitterSizes;

bool SailDlg::s_bAxes       = true;
bool SailDlg::s_bOutline    = true;
bool SailDlg::s_bSurfaces   = true;
bool SailDlg::s_bPanels     = false;
bool SailDlg::s_bCtrlPoints = false;
bool SailDlg::s_bCornerPts  = false;

bool SailDlg::s_bRuledMesh = true;

bool SailDlg::s_bGuessOpposite = false;
double SailDlg::s_TEMaxAngle = 0;


Grid SailDlg::s_SectionGrid;

SailDlg::SailDlg(QWidget *pParent) : XflDialog(pParent)
{
    setWindowTitle(tr("Sail Edition"));
//    setWindowFlags(Qt::Window);

    m_pTabWidget = nullptr;
    m_pViewVSplitter = m_pViewHSplitter = m_pExternalSplitter = m_pInternalSplitter = m_pSectionTableSplitter = nullptr;

    m_pcptSections = m_pcptPoints = nullptr;
    m_pPointModel      = nullptr;
    m_pPointDelegate   = nullptr;

    m_pSail = nullptr;

    m_bIsMeshing = false;
    m_bChanged = m_bDescriptionChanged = false;

    m_iActiveSection  = -1;

    makeCommonWts();
}


void SailDlg::connectBaseSignals()
{
    connect(m_pcbColor,             SIGNAL(clicked()),            SLOT(onSailColor()));

    connect(m_p2dSectionView,       SIGNAL(selectedChanged(int)), SLOT(onSelectCtrlPoint(int)));
    connect(m_p2dSectionView,       SIGNAL(sectionSelected(int)), SLOT(onSelectSection(int)));

    connect(m_pfeRefArea,           SIGNAL(floatChanged(float)),  SLOT(onSetChanged()));
    connect(m_pfeRefChord,          SIGNAL(floatChanged(float)),  SLOT(onSetChanged()));

    connect(m_pieNXPanels,          SIGNAL(intChanged(int)),      SLOT(onUpdateMesh()));
    connect(m_pcbXDistType,         SIGNAL(activated(int)),       SLOT(onUpdateMesh()));

    connect(m_pieNZPanels,          SIGNAL(intChanged(int)),      SLOT(onUpdateMesh()));
    connect(m_pcbZDistType,         SIGNAL(activated(int)),       SLOT(onUpdateMesh()));


    connect(m_pchFillFoil,          SIGNAL(clicked(bool)), m_p2dSectionView, SLOT(onFillFoil(bool)));
    connect(m_plbSectionStyle,      SIGNAL(clickedLB(LineStyle)), SLOT(onLineStyle(LineStyle)));

    connect(m_pglSailView,          SIGNAL(pickedNodePair(QPair<int,int>)), SLOT(onPickedNodePair(QPair<int,int>)));
    connect(m_pglSailControls->m_ptbDistance, SIGNAL(clicked()), SLOT(onNodeDistance()));

    connect(m_p3dLightAct,          SIGNAL(triggered()), m_pglSailView, SLOT(onSetupLight()));
    connect(m_pBackImageLoad,       SIGNAL(triggered()), m_pglSailView, SLOT(onLoadBackImage()));
    connect(m_pBackImageClear,      SIGNAL(triggered()), m_pglSailView, SLOT(onClearBackImage()));
    connect(m_pBackImageSettings,   SIGNAL(triggered()), m_pglSailView, SLOT(onBackImageSettings()));


    connect(m_prbRuledMesh,         SIGNAL(clicked()),            SLOT(onRuledMesh()));
    connect(m_prbFreeMesh,          SIGNAL(clicked()),            SLOT(onRuledMesh()));

    connect(m_ppbGuessTE,           SIGNAL(clicked(bool)),        SLOT(onGuessTE()));
    connect(m_ppbClearTE,           SIGNAL(clicked(bool)),        SLOT(onClearTEPanels()));
    connect(m_ppbTETop,             SIGNAL(clicked(bool)),        SLOT(onTopTEPanels(bool)));
    connect(m_ppbTEBotMid,          SIGNAL(clicked(bool)),        SLOT(onBotTEPanels(bool)));
    connect(m_pglSailView,          SIGNAL(panelSelected(int)),   SLOT(onPanelSelected(int)));
    connect(m_ppbCheckTE,           SIGNAL(clicked(bool)),        SLOT(onCheckTEPanels()));

    connect(m_pClearTE,             SIGNAL(triggered(bool)),      SLOT(onClearTEPanels()));
    connect(m_pClearHighlighted,    SIGNAL(triggered()),          SLOT(onClearHighlighted()));
    connect(m_pConnectPanels,       SIGNAL(triggered()),          SLOT(onConnectPanels()));
    connect(m_pCheckFreeEdges,      SIGNAL(triggered()),          SLOT(onCheckFreeEdges()));


    connect(m_ppbConnectPanels,     SIGNAL(clicked()),          SLOT(onConnectPanels()));
}



void SailDlg::makeCommonWts()
{
    m_ppto = new PlainTextOutput;

    m_pfrMeta = new QFrame;
    {
        QVBoxLayout *pMetaLayout = new QVBoxLayout;
        {
            QGroupBox *pInfoBox = new QGroupBox(tr("Description"));
            {
                QVBoxLayout *pInfoBoxLayout = new QVBoxLayout;
                {
                    QHBoxLayout *m_pSailInfoLayout = new QHBoxLayout;
                    {
                        m_pleSailName = new QLineEdit;
                        m_pleSailName->setClearButtonEnabled(true);
                        m_pleSailName->setToolTip(tr("Enter the sail's name"));

                        m_pcbColor = new ColorBtn;

                        m_pSailInfoLayout->addWidget(m_pleSailName);
                        m_pSailInfoLayout->addWidget(m_pcbColor);
                        m_pSailInfoLayout->setStretchFactor(m_pleSailName, 5);
                        m_pSailInfoLayout->setStretchFactor(m_pcbColor, 1);
                    }
                    m_pteSailDescription = new QTextEdit;
                    m_pteSailDescription->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
                    m_pteSailDescription->setToolTip(tr("Use this field to enter a short text to describe the sail"));

                    pInfoBoxLayout->addLayout(m_pSailInfoLayout);
                    pInfoBoxLayout->addWidget(m_pteSailDescription);
                }
                pInfoBox->setLayout(pInfoBoxLayout);
            }
            m_pSurfaceBox = new QGroupBox(tr("Sail surface"));
            {
                QVBoxLayout *pSurfaceLayout = new QVBoxLayout;
                {
                    m_pfrThickness = new QFrame;
                    {
                        QHBoxLayout *pThickLayout = new QHBoxLayout;
                        {
                            QLabel *plabThick = new QLabel(tr("Sail is a"));
                            QLabel *plabSurf  = new QLabel(tr("surface"));
                            m_prbThin  = new QRadioButton(tr("thin"));
                            m_prbThick = new QRadioButton(tr("thick"));
                            pThickLayout->addWidget(plabThick);
                            pThickLayout->addWidget(m_prbThin);
                            pThickLayout->addWidget(m_prbThick);
                            pThickLayout->addWidget(plabSurf);
                            pThickLayout->addStretch();
                        }
                        m_pfrThickness->setLayout(pThickLayout);
                    }

                    QGridLayout *pAreaLayout = new QGridLayout;
                    {
                        QLabel *plabRefArea    = new QLabel(tr("Reference area:"));
                        QLabel *plabAreaUnit   = new QLabel(Units::areaUnitQLabel());
                        QLabel *plabRefChord   = new QLabel(tr("Reference chord:"));
                        QLabel *plabLengthUnit = new QLabel(Units::lengthUnitQLabel());
                        m_pfeRefArea  = new FloatEdit;
                        m_pfeRefChord = new FloatEdit;
                        QString top(tr("Set the reference dimension used to calculate lift and drag coefficents"));
                        m_pfeRefArea->setToolTip(top);
                        m_pfeRefChord->setToolTip(top);

                        pAreaLayout->addWidget(plabRefArea,    1, 1, Qt::AlignVCenter | Qt::AlignRight);
                        pAreaLayout->addWidget(m_pfeRefArea,   1, 2);
                        pAreaLayout->addWidget(plabAreaUnit,   1, 3);
                        pAreaLayout->addWidget(plabRefChord,   2, 1, Qt::AlignVCenter | Qt::AlignRight);
                        pAreaLayout->addWidget(m_pfeRefChord,  2, 2);
                        pAreaLayout->addWidget(plabLengthUnit, 2, 3);

                        pAreaLayout->setColumnStretch(4,1);
                    }

                    pSurfaceLayout->addWidget(m_pfrThickness);
                    pSurfaceLayout->addLayout(pAreaLayout);
                }
                m_pSurfaceBox->setLayout(pSurfaceLayout);
            }

            pMetaLayout->addWidget(pInfoBox);
            pMetaLayout->addWidget(m_pSurfaceBox);
            pMetaLayout->addStretch();
        }
        m_pfrMeta->setLayout(pMetaLayout);
    }

    m_pfrMesh = new QFrame;
    {
        QVBoxLayout *pMeshLayout = new QVBoxLayout;
        {
            m_pgbMeshType = new QGroupBox(tr("Mesh type"));
            {
                QHBoxLayout *pMeshTypeLayout = new QHBoxLayout;
                {
                    QLabel *plabMeshType = new QLabel(tr("Mesh type:"));
                    m_prbRuledMesh = new QRadioButton(tr("Ruled"));
                    m_prbFreeMesh  = new QRadioButton(tr("Free"));

                    pMeshTypeLayout->addWidget(plabMeshType);
                    pMeshTypeLayout->addWidget(m_prbRuledMesh);
                    pMeshTypeLayout->addWidget(m_prbFreeMesh);
                    pMeshTypeLayout->addStretch();

                }
                m_pgbMeshType->setLayout(pMeshTypeLayout);
            }

            m_pfrRuledMesh = new QFrame;
            {
                QGridLayout *pRuledMeshLayout = new QGridLayout;
                {
                    QLabel *plabX = new QLabel(tr("x"));
                    QLabel *plabZ = new QLabel(tr("z"));
                    QLabel *plabNPanels  = new QLabel(tr("Number of panels="));
                    QLabel *plabDistType = new QLabel(tr("Distribution="));
                    m_pieNXPanels   = new IntEdit(17);
                    m_pieNZPanels   = new IntEdit(17);
                    QStringList disttypes = {tr("UNIFORM"), tr("COSINE"), tr("SINE"), tr("INV_SINE"), tr("TANH"), tr("EXP"), tr("INV_EXP")};
                    m_pcbXDistType = new QComboBox;
                    m_pcbZDistType = new QComboBox;
                    m_pcbXDistType->addItems(disttypes);
                    m_pcbZDistType->addItems(disttypes);

                    pRuledMeshLayout->addWidget(plabX,           1, 2, Qt::AlignCenter);
                    pRuledMeshLayout->addWidget(plabZ,           1, 3, Qt::AlignCenter);
                    pRuledMeshLayout->addWidget(plabNPanels,     2, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pRuledMeshLayout->addWidget(m_pieNXPanels,   2, 2);
                    pRuledMeshLayout->addWidget(m_pieNZPanels,   2, 3);
                    pRuledMeshLayout->addWidget(plabDistType,    3, 1, Qt::AlignRight | Qt::AlignVCenter);
                    pRuledMeshLayout->addWidget(m_pcbXDistType,  3, 2);
                    pRuledMeshLayout->addWidget(m_pcbZDistType,  3, 3);

                    pRuledMeshLayout->setColumnStretch(5,1);
                    pRuledMeshLayout->setRowStretch(4,1);

                }
                m_pfrRuledMesh->setLayout(pRuledMeshLayout);
            }

            m_pGMesherWt = new GMesherWt(this);

            pMeshLayout->addWidget(m_pgbMeshType);
            pMeshLayout->addWidget(m_pfrRuledMesh);
            pMeshLayout->addWidget(m_pGMesherWt);
        }

        m_pfrMesh->setLayout(pMeshLayout);
    }


    m_pfrTE = new QFrame;
    {
        QGridLayout *pTELayout = new QGridLayout;
        {
            m_ppbConnectPanels = new QPushButton(tr("Connect panels"));
            QLabel *plabMaxAngle = new QLabel(tr("Max T.E. angle for guesses:"));
            m_pfeTEAngle = new FloatEdit();
            QLabel *plabDegree = new QLabel(tr("<p>&deg;</p>"));

            QLabel *plabAutoGuess = new QLabel(tr("Automatic detection:"));
            m_ppbGuessTE = new QPushButton(tr("Guess T.E."));

            m_ppbClearTE = new QPushButton(tr("Clear T.E. panels"));

            QLabel *pLabTE = new QLabel(tr("Manual selection:"));
            m_ppbTEBotMid = new QPushButton(tr("Mid. panels"));
            m_ppbTEBotMid->setCheckable(true);
            m_ppbTETop = new QPushButton(tr("Top panels"));
            m_ppbTETop->setCheckable(true);
            m_pchGuessOpposite = new QCheckBox(tr("Guess opposite"));

            QLabel *pLabCheck = new QLabel(tr("Verification"));
            m_ppbCheckTE = new QPushButton(tr("Check T.E."));

            pTELayout->addWidget(m_ppbConnectPanels,  1, 2);
            pTELayout->addWidget(plabMaxAngle,        2, 1);
            pTELayout->addWidget(m_pfeTEAngle,        2, 2);
            pTELayout->addWidget(plabDegree,          2, 3);
            pTELayout->addWidget(plabAutoGuess,       3, 1);
            pTELayout->addWidget(m_ppbGuessTE,        3, 2);
            pTELayout->addWidget(m_ppbClearTE,        3, 3);
            pTELayout->addWidget(pLabTE,              4, 1);
            pTELayout->addWidget(m_ppbTETop,          4, 2);
            pTELayout->addWidget(m_pchGuessOpposite,  4, 3);
            pTELayout->addWidget(m_ppbTEBotMid,       5, 2);
            pTELayout->addWidget(pLabCheck,           6, 1);
            pTELayout->addWidget(m_ppbCheckTE,        6, 2);
            pTELayout->setColumnStretch(2, 1);
            pTELayout->setRowStretch(7, 1);
        }

        m_pfrTE->setLayout(pTELayout);
    }

    m_pfr2dView = new QFrame;
    {
        QVBoxLayout *p2dViewLayout = new QVBoxLayout;
        {
            m_p2dSectionView = new SailSectionView;
            m_p2dSectionView->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
            m_p2dSectionView->setGrid(s_SectionGrid);
            QToolBar *pActionButtons = new QToolBar(this);
            {
                m_pUndo = new QAction(QIcon(":/icons/OnUndo.png"), tr("Undo"), this);
                m_pUndo->setShortcut(Qt::CTRL | Qt::Key_Z);
                m_pUndo->setStatusTip(tr("Cancels the last modifiction made to the splines"));

                m_pRedo = new QAction(QIcon(":/icons/OnRedo.png"), tr("Redo"), this);
                m_pRedo->setShortcut(Qt::CTRL | Qt::Key_Y);
                m_pRedo->setStatusTip(tr("Restores the last cancelled modifiction made to the splines"));

                m_pchFillFoil = new QCheckBox(tr("Fill selected"));
                m_plbSectionStyle = new LineBtn;
                m_plbSectionStyle->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

                QFrame* pStretch = new QFrame;
                pStretch->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

                pActionButtons->addAction(m_p2dSectionView->m_pZoomInAct);
                pActionButtons->addAction(m_p2dSectionView->m_pZoomLessAct);
                pActionButtons->addSeparator();
                pActionButtons->addAction(m_p2dSectionView->m_pResetXScaleAct);
                pActionButtons->addAction(m_p2dSectionView->m_pResetXYScaleAct);
                pActionButtons->addAction(m_p2dSectionView->m_pZoomYAct);

                pActionButtons->addSeparator();
                pActionButtons->addAction(m_p2dSectionView->m_pGridAct);
                pActionButtons->addSeparator();
                pActionButtons->addAction(m_pUndo);
                pActionButtons->addAction(m_pRedo);

                pActionButtons->addSeparator();
                pActionButtons->addWidget(pStretch);
                pActionButtons->addWidget(m_pchFillFoil);
                pActionButtons->addSeparator();
                pActionButtons->addWidget(m_plbSectionStyle);
            }

            p2dViewLayout->addWidget(pActionButtons);
            p2dViewLayout->addWidget(m_p2dSectionView);
        }
        m_pfr2dView->setLayout(p2dViewLayout);
    }

    m_pfr3dView = new QFrame;
    {
        QVBoxLayout *pViewLayout = new QVBoxLayout;
        {
            m_pglSailView = new gl3dSailView;
            m_pglSailView->showPartFrame(false);
            m_pglSailControls = new gl3dGeomControls(m_pglSailView, SailLayout, false);

            m_p3dLightAct        = new  QAction(QIcon(":/icons/light.png"), tr("Light settings\t(Alt+L)"), this);

            m_pBackImageLoad     = new QAction(tr("Load"), this);
            m_pBackImageClear    = new QAction(tr("Clear"), this);
            m_pBackImageSettings = new QAction(tr("Settings"), this);

            pViewLayout->addWidget(m_pglSailView);
            pViewLayout->addWidget(m_pglSailControls);
        }
        m_pfr3dView->setLayout(pViewLayout);
    }

    m_pButtonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard);
    {
        m_ppbMeshOps = new QPushButton(tr("Mesh actions"));
        {
            QMenu *pMeshMenu = new QMenu(tr("Mesh"));
            {
                m_pCheckFreeEdges   = new QAction(tr("Check free edges"), this);
                m_pCheckFreeEdges->setShortcut(QKeySequence(Qt::ALT |  Qt::Key_G));
                m_pConnectPanels    = new QAction(tr("Connect panels"), this);
                m_pConnectPanels->setShortcut(QKeySequence(Qt::ALT |  Qt::Key_C));
                m_pClearHighlighted = new QAction(tr("Clear highlighted"), this);
                m_pClearHighlighted->setShortcut(QKeySequence(Qt::ALT | Qt::Key_L));

                m_pClearTE = new QAction(tr("Clear T.E. panels"));

                pMeshMenu->addAction(m_pClearTE);
                pMeshMenu->addSeparator();
                pMeshMenu->addAction(m_pConnectPanels);
                pMeshMenu->addAction(m_pCheckFreeEdges);
                pMeshMenu->addAction(m_pClearHighlighted);
            }
            m_ppbMeshOps->setMenu(pMeshMenu);
        }
        m_pButtonBox->addButton(m_ppbMeshOps, QDialogButtonBox::ActionRole);

        m_ppbSailOps = new QPushButton(tr("Sail actions"));
        {
            QMenu *pSailMenu = new QMenu(tr("Sail"));
            {
                m_pDefinitions = new QAction(tr("Definitions"), this);
                connect(m_pDefinitions, SIGNAL(triggered(bool)), SLOT(onDefinitions()));

                m_pTranslate = new QAction(tr("Translate"), this);
                connect(m_pTranslate, SIGNAL(triggered(bool)), SLOT(onTranslateSail()));

                m_pRotate = new QAction(tr("Rotate"), this); // connected in the ExternalSail subclass

                m_pScaleSize = new QAction(tr("Scale size"), this);
                m_pScaleSize->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F10));
                connect(m_pScaleSize, SIGNAL(triggered(bool)), SLOT(onScaleSize()));

                m_pScaleShape = new QAction(tr("Scale shape"), this);
                m_pScaleShape->setShortcut(Qt::Key_F10);
                connect(m_pScaleShape, SIGNAL(triggered(bool)), SLOT(onScaleShape()));


                m_pFlipXZ = new QAction(tr("Flip XZ"), this);
                connect(m_pFlipXZ, SIGNAL(triggered(bool)), SLOT(onFlipXZ()));

                m_pAlignLuff = new QAction(tr("Align luff points"), this);
                QString tip(tr("<p>Translates the intermediate sections to align the "
                            "leading points between the top and bottom sections</p>"));
                m_pAlignLuff->setToolTip(tip);
                connect(m_pAlignLuff, SIGNAL(triggered(bool)), SLOT(onAlignLuffPoints()));

                pSailMenu->addAction(m_pDefinitions);
                pSailMenu->addSeparator();
                pSailMenu->addAction(m_pTranslate);
                pSailMenu->addAction(m_pRotate);
                pSailMenu->addAction(m_pScaleShape);
                pSailMenu->addAction(m_pScaleSize);
                pSailMenu->addSeparator();
                pSailMenu->addAction(m_pFlipXZ);
                pSailMenu->addSeparator();
                pSailMenu->addAction(m_pAlignLuff);

                QMenu *pExportMenu = pSailMenu->addMenu(tr("Export"));
                {
                    m_pExportXML = new QAction(tr("to XML"), this);
                    connect(m_pExportXML, SIGNAL(triggered(bool)), SLOT(onExportToXml()));
                    pExportMenu->addAction(m_pExportXML);

                    m_pExportStep = new QAction(tr("to STEP"), this);
                    connect(m_pExportStep, SIGNAL(triggered(bool)), SLOT(onExportToStep()));
                    pExportMenu->addAction(m_pExportStep);

                    m_pExportMeshToSTL = new QAction(tr("mesh to STL"), this);
                    connect(m_pExportMeshToSTL, SIGNAL(triggered(bool)), SLOT(onExportMeshToStl()));
                    pExportMenu->addAction(m_pExportMeshToSTL);

                    m_pExportTrianglesToSTL = new QAction(tr("triangulation to STL"), this);
                    connect(m_pExportTrianglesToSTL, SIGNAL(triggered(bool)), SLOT(onExportTrianglesToStl()));
                    pExportMenu->addAction(m_pExportTrianglesToSTL);
                }
            }
            m_ppbSailOps->setMenu(pSailMenu);
            m_ppbSailOps->setDefault(false);
            m_ppbSailOps->setAutoDefault(false);
        }
    }
}


void SailDlg::initDialog(Sail *pSail)
{
    m_pSail = pSail;
    m_p2dSectionView->setSail(pSail);

    m_pglSailView->setSail(pSail);

    m_iActiveSection = 0;

    // update the mesh for display
    m_pSail->clearTriMesh();
    m_pSail->makeTriPanels(Vector3d());
    m_pglSailView->resetglSail();

    SailSectionView::sectionStyle().m_Color = pSail->color();
    m_plbSectionStyle->setTheStyle(SailSectionView::sectionStyle());

    m_pfeRefArea->setValue(pSail->refArea()*Units::m2toUnit());
    m_pfeRefChord->setValue(pSail->refChord()*Units::mtoUnit());

    m_pGMesherWt->initWt(pSail);
}


void SailDlg::onSetChanged()
{
    m_bChanged = true;
}


void SailDlg::onLineStyle(LineStyle const &ls)
{
    LineMenu *pLineMenu = new LineMenu(nullptr, false);
    pLineMenu->initMenu(ls);
    pLineMenu->exec(QCursor::pos());
    SailSectionView::setSectionStyle(pLineMenu->theStyle());
    m_plbSectionStyle->setTheStyle(pLineMenu->theStyle());
    m_p2dSectionView->update();
}


void SailDlg::readMeshData()
{
    int nx = m_pieNXPanels->value();
    int nz = m_pieNZPanels->value();

    xfl::enumDistribution xdist = xfl::distributionType(m_pcbXDistType->currentText().toStdString());
    xfl::enumDistribution zdist = xfl::distributionType(m_pcbZDistType->currentText().toStdString());

    m_pSail->setNXPanels(nx);
    m_pSail->setXDistType(xdist);

    m_pSail->setNZPanels(nz);
    m_pSail->setZDistType(zdist);
}


void SailDlg::onUpdateMesh()
{
    readMeshData();

    updateTriMesh();
    m_pglSailView->update();
    m_bChanged = true;
}


void SailDlg::updateTriMesh()
{
    m_pSail->clearTriMesh();
    m_pSail->makeTriPanels(Vector3d());
    m_pglSailView->resetgl3dMesh();
}


void SailDlg::setSailData()
{
    m_pleSailName->setText(QString::fromStdString(m_pSail->name()));
    m_pteSailDescription->setPlainText(QString::fromStdString(m_pSail->description()));
    m_pcbColor->setColor(xfl::fromfl5Clr(m_pSail->color()));

    m_prbRuledMesh->setChecked(m_pSail->bRuledMesh());
    m_prbFreeMesh->setChecked(!m_pSail->bRuledMesh());


    m_pieNXPanels->setValue(m_pSail->nXPanels());

    switch(m_pSail->xDistType())
    {
        default:
        case xfl::UNIFORM:       m_pcbXDistType->setCurrentIndex(0);  break;
        case xfl::COSINE:        m_pcbXDistType->setCurrentIndex(1);  break;
        case xfl::SINE:          m_pcbXDistType->setCurrentIndex(2);  break;
        case xfl::INV_SINE:      m_pcbXDistType->setCurrentIndex(3);  break;
        case xfl::TANH:          m_pcbXDistType->setCurrentIndex(4);  break;
        case xfl::EXP:           m_pcbXDistType->setCurrentIndex(5);  break;
        case xfl::INV_EXP:       m_pcbXDistType->setCurrentIndex(6);  break;
    }


    m_pieNZPanels->setValue(m_pSail->nZPanels());
    switch(m_pSail->zDistType())
    {
        default:
        case xfl::UNIFORM:       m_pcbZDistType->setCurrentIndex(0);  break;
        case xfl::COSINE:        m_pcbZDistType->setCurrentIndex(1);  break;
        case xfl::SINE:          m_pcbZDistType->setCurrentIndex(2);  break;
        case xfl::INV_SINE:      m_pcbZDistType->setCurrentIndex(3);  break;
        case xfl::TANH:          m_pcbZDistType->setCurrentIndex(4);  break;
        case xfl::EXP:           m_pcbZDistType->setCurrentIndex(5);  break;
        case xfl::INV_EXP:       m_pcbZDistType->setCurrentIndex(6);  break;
    }

    AFMesher::setMaxEdgeLength(m_pSail->maxElementSize()*Units::mtoUnit());

    m_iActiveSection = 0;
}


void SailDlg::updateSailDataOutput()
{
    std::string saildata, frontspacer;
    m_pSail->properties(saildata, frontspacer);
    m_pglSailView->setBotLeftOutput(saildata);
}


void SailDlg::updateSailSectionOutput()
{
}


void SailDlg::setControls()
{
    m_pchFillFoil->setEnabled(m_pSail && m_pSail->isWingSail());
    m_pchFillFoil->setChecked(SailSectionView::bFill());
    if(m_pSail) m_pcbColor->setColor(xfl::fromfl5Clr(m_pSail->color()));

    m_pRotate->setEnabled(m_pSail && m_pSail->isExternalSail());
    m_pieNZPanels->setEnabled(m_pSail && (m_pSail->isNURBSSail() || m_pSail->isSplineSail()));
    m_pcbZDistType->setEnabled(m_pSail && (m_pSail->isNURBSSail() || m_pSail->isSplineSail()));

    if(m_pSail)
    {
        m_pfrRuledMesh->setVisible(m_pSail->bRuledMesh());
        m_pGMesherWt->setVisible(!m_pSail->bRuledMesh());
    }
}


void SailDlg::makeBaseTables()
{
    QString str = Units::lengthUnitQLabel();
//    NURBSSail *pNS = dynamic_cast<NURBSSail*>(pSail);

    //Set Frame Table
    m_pcptSections = new CPTableView(this);
    m_pcptSections->setEditable(true);
    m_pcptSections->setWindowTitle(tr("Sail definition"));
    m_pcptSections->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_pcptSections->setEditTriggers(QAbstractItemView::AllEditTriggers);

    m_pcptSections->horizontalHeader()->setStretchLastSection(true);

    m_pcptSections->horizontalHeader()->setStretchLastSection(true);

    //Set Point Table
    m_pcptPoints = new CPTableView(this);
    m_pcptPoints->setEditable(true);
    m_pcptPoints->setWindowTitle(tr("Point coordinates"));
    m_pcptPoints->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_pcptPoints->setEditTriggers(QAbstractItemView::DoubleClicked  | QAbstractItemView::SelectedClicked |
                                  QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
    m_pcptPoints->horizontalHeader()->setStretchLastSection(true);
    m_pPointModel = new ActionItemModel(this);
    m_pPointModel->setRowCount(10);//temporary
    m_pPointModel->setColumnCount(3);
    m_pPointModel->setHeaderData(0, Qt::Horizontal, "x ("+str+")");
    m_pPointModel->setHeaderData(1, Qt::Horizontal, "y ("+str+")");
    m_pPointModel->setHeaderData(2, Qt::Horizontal, "Actions");
    m_pPointModel->setActionColumn(2);
    m_pcptPoints->setModel(m_pPointModel);
    QItemSelectionModel *pSelPointModel = new QItemSelectionModel(m_pPointModel);
    m_pcptPoints->setSelectionModel(pSelPointModel);
    m_pcptPoints->horizontalHeader()->setStretchLastSection(true);

    m_pPointDelegate = new XflDelegate(this);
    m_pPointDelegate->setActionColumn(2);
    m_pcptPoints->setItemDelegate(m_pPointDelegate);
    m_pPointDelegate->setDigits({3,3,0});
    m_pPointDelegate->setItemTypes({XflDelegate::DOUBLE, XflDelegate::DOUBLE, XflDelegate::ACTION});
    m_pSectionTableSplitter = new QSplitter(Qt::Horizontal);
    {
        QGroupBox *pSectionBox = new QGroupBox(tr("Sections"));
        {
            QVBoxLayout *pSectionBoxLayout = new QVBoxLayout;
            {
                pSectionBoxLayout->addWidget(m_pcptSections);
            }
            pSectionBox->setLayout(pSectionBoxLayout);
        }

        //lays out the control point table
        QGroupBox *pgbNURBS = new QGroupBox(tr("Section points coordinates"));
        {
            QVBoxLayout *pPointDataLayout = new QVBoxLayout;
            {
                pPointDataLayout->addWidget(m_pcptPoints);
                pgbNURBS->setLayout(pPointDataLayout);
            }
        }

        m_pSectionTableSplitter->addWidget(pSectionBox);
        m_pSectionTableSplitter->addWidget(pgbNURBS);
    }
}


void SailDlg::accept()
{
    readData();

    if(m_pSail->refArea()<0.00001)
    {
        if(m_pTabWidget) m_pTabWidget->setCurrentIndex(0);
        m_ppto->onAppendQText("The reference dimensions cannot be null\n");
        m_pfeRefArea->setFocus();
        m_pfeRefArea->selectAll();
        return;
    }

    if(m_pSail->refChord()<0.00001)
    {
        if(m_pTabWidget) m_pTabWidget->setCurrentIndex(0);
        m_ppto->onAppendQText("The reference dimensions cannot be null\n");
        m_pfeRefChord->setFocus();
        m_pfeRefChord->selectAll();
        return;
    }

    m_pSail->makeSurface();

    XflDialog::accept();
}


void SailDlg::onSailColor()
{
    if(!m_pSail) return;

    QColor clr = QColorDialog::getColor(xfl::fromfl5Clr(m_pSail->color()), this, "Sail colour", QColorDialog::ShowAlphaChannel);
    if(clr.isValid())
    {
        m_pSail->setColor(xfl::tofl5Clr(clr));
        m_pcbColor->setColor(clr);
    }

    SailSectionView::sectionStyle().m_Color = xfl::tofl5Clr(clr);
    m_plbSectionStyle->setBtnColor(clr);


    m_bDescriptionChanged = true;
    m_pglSailView->resetglSail();
    updateView();
}


void SailDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bCtrl  = false;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;
/*    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;*/

    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            if(m_pglSailControls->getDistance())
            {
                m_pglSailControls->stopDistance();
                m_pglSailView->stopPicking();

                m_pglSailView->clearMeasure();
                return;
            }
            else
            {
                reject();
                return;
            }
        }
        case Qt::Key_S:
        {
            if(bCtrl)
                accept();
            break;
        }
        default:
            XflDialog::keyPressEvent(pEvent);
    }
}


void SailDlg::customEvent(QEvent *pEvent)
{
    if(pEvent->type() == MESH_UPDATE_EVENT)
    {
        QApplication::restoreOverrideCursor();
        m_bIsMeshing = false;

        MeshEvent *pMeshEvent = dynamic_cast<MeshEvent*>(pEvent);

        QString strange;
        strange = "   making mesh from triangles\n";
        m_ppto->onAppendQText(strange);
        m_pSail->setRefTriangles(pMeshEvent->triangles());
        m_pSail->makeTriPanels(Vector3d());
        m_pSail->setMaxElementSize(AFMesher::maxEdgeLength());

        m_pglSailView->clearSegments();

        strange = QString::asprintf("\nTriangle count = %d\n", m_pSail->nPanel3());
        m_ppto->onAppendQText(strange);
        strange = QString::asprintf(  "Node count     = %d\n", m_pSail->triMesh().nNodes());
        strange += "\n_______\n\n";
        m_ppto->onAppendQText(strange);

        m_pglSailView->resetgl3dMesh();
        m_pglSailView->update();

        m_pglSailView->clearDebugPoints();
        if(AFMesher::s_DebugPts.size())
        {
            m_pglSailView->setDebugPoints(AFMesher::s_DebugPts);
            m_pglSailView->appendDebugVec(AFMesher::s_DebugPts.back()-AFMesher::s_DebugPts.front());
/*m_pSail->setRefTriangles(NURBSSurface::s_DbgTriangles);
m_pSail->makeTriPanels(Vector3d());*/
/*            Triangle3d const& t3d = NURBSSurface::s_DbgTriangles.at(5);
            Vector3d I;
            t3d.intersectSegmentInside(AFMesher::s_DebugPts.front(), AFMesher::s_DebugPts.back(), I, true);
            m_pglSailView->appendDebugPoint(I);*/
        }
        updateSailDataOutput();

        m_bChanged = true;
        
    }
    else if(pEvent->type() == MESSAGE_EVENT)
    {
        MessageEvent *pMsgEvent = dynamic_cast<MessageEvent*>(pEvent);
        m_ppto->onAppendQText(pMsgEvent->msg());
    }
    else
        QDialog::customEvent(pEvent);
}


void SailDlg::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QMenu *pContextMenu = new QMenu(tr("GraphMenu"));
    {
        pContextMenu->addAction(m_pTranslate);
        pContextMenu->addAction(m_pScaleShape);
        pContextMenu->addAction(m_pRotate);
//        pContextMenu->addSeparator();
//        pContextMenu->addAction(m_pScaleArea);
        pContextMenu->addSeparator();
        pContextMenu->addAction(m_pFlipXZ);
        pContextMenu->addSeparator();
        pContextMenu->addAction(m_pAlignLuff);
        pContextMenu->addSeparator();
        QMenu *pExportMenu = pContextMenu->addMenu(tr("Export"));
        {
            pExportMenu->addAction(m_pExportXML);
            pExportMenu->addAction(m_pExportMeshToSTL);
            if(m_pSail && m_pSail->isNURBSSail()) pExportMenu->addAction(m_pExportStep);
        }
        pContextMenu->addSeparator();
        pContextMenu->addAction(m_p3dLightAct);
        pContextMenu->addSeparator();

        QMenu *pBackImageMenu = pContextMenu->addMenu(tr("Background image"));
        {
            pBackImageMenu->addAction(m_pBackImageLoad);
            pBackImageMenu->addAction(m_pBackImageClear);
            pBackImageMenu->addAction(m_pBackImageSettings);
        }
    }

    pContextMenu->exec(QCursor::pos());
    update();
    pEvent->accept();
}


void SailDlg::resizeEvent(QResizeEvent *)
{
    resizeSectionTableColumns();
    resizePointTableColumns();

    m_p2dSectionView->resetDefaultScale();
    m_p2dSectionView->setAutoUnits();
}


void SailDlg::resizePointTableColumns()
{
    if(m_pcptPoints)
    {
        int w2  = int(double(m_pcptPoints->width()) / 3.5);
        m_pcptPoints->setColumnWidth(0, w2);
        m_pcptPoints->setColumnWidth(1, w2);
        m_pcptPoints->setColumnWidth(3, w2);
    }
}


void SailDlg::onResizeTableColumns()
{
    resizePointTableColumns();
    resizeSectionTableColumns();
}


void SailDlg::showEvent(QShowEvent *pEvent)
{
    XflDialog::showEvent(pEvent);
    restoreGeometry(s_WindowGeometry);

    if(m_pExternalSplitter   && s_ExtSplitterSizes.length()>0)  m_pExternalSplitter->restoreState(s_ExtSplitterSizes);
    if(m_pInternalSplitter   && s_IntSplitterSizes.length()>0)  m_pInternalSplitter->restoreState(s_IntSplitterSizes);
    if(m_pViewHSplitter && s_HSplitterSizes.length()>0)    m_pViewHSplitter->restoreState(s_HSplitterSizes);
    if(m_pViewVSplitter && s_VSplitterSizes.length()>0)    m_pViewVSplitter->restoreState(s_VSplitterSizes);
    if(m_pSectionTableSplitter && s_TableSplitterSizes.length()>0)
        m_pSectionTableSplitter->restoreState(s_TableSplitterSizes);

    m_p2dSectionView->setAutoUnits();
    updateSailSectionOutput();

    m_pglSailView->setFlags(s_bOutline, s_bSurfaces, s_bPanels, s_bAxes, false, false, false, false, s_bCtrlPoints);
    m_pglSailView->showCornerPoints(s_bCornerPts);
    m_pglSailControls->setControls();

    if(W3dPrefs::s_bSaveViewPoints)
    {
        m_pglSailView->restoreViewPoint(s_ab_quat);
    }

    resizePointTableColumns();
    resizeSectionTableColumns();

}


void SailDlg::hideEvent(QHideEvent *pEvent)
{
    XflDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();

    if(m_pInternalSplitter)     s_IntSplitterSizes   = m_pInternalSplitter->saveState();
    if(m_pExternalSplitter)     s_ExtSplitterSizes   = m_pExternalSplitter->saveState();
    if(m_pViewHSplitter)        s_HSplitterSizes     = m_pViewHSplitter->saveState();
    if(m_pViewVSplitter)        s_VSplitterSizes     = m_pViewVSplitter->saveState();
    if(m_pSectionTableSplitter) s_TableSplitterSizes = m_pSectionTableSplitter->saveState();

    s_SectionGrid = m_p2dSectionView->grid();

    s_bAxes       = m_pglSailView->bAxes();
    s_bOutline    = m_pglSailView->bOutline();
    s_bSurfaces   = m_pglSailView->bSurfaces();
    s_bPanels     = m_pglSailView->bVLMPanels();
    s_bCtrlPoints = m_pglSailView->bCtrlPts();
    s_bCornerPts  = m_pglSailView->bSailCornerPts();

    if(W3dPrefs::s_bSaveViewPoints)
    {
        m_pglSailView->saveViewPoint(s_ab_quat);
    }
}


void SailDlg::readData()
{
    if(!m_pSail) return;
    m_pSail->setName(m_pleSailName->text().toStdString());
    m_pSail->setDescription(m_pteSailDescription->toPlainText().toStdString());

    m_pSail->setRefArea(m_pfeRefArea->value()/Units::m2toUnit());
    m_pSail->setRefChord(m_pfeRefChord->value()/Units::mtoUnit());

    readMeshData();

    if(!m_pSail) return;

    Vector3d LE = m_pSail->leadingEdgeAxis();
    m_pSail->setLuffAngle(atan2(LE.x, LE.z) * 180./PI);

    readSectionData();
}


void SailDlg::onUpdate()
{
    m_bChanged = true;

    fillPointModel();

    updateSailGeometry();

    if(m_pSail->bRuledMesh()) m_pSail->makeRuledMesh(Vector3d());
    else                      m_pSail->clearRefTriangles();

    updateTriMesh();
    updateSailDataOutput();
    updateSailSectionOutput();

    updateView();
}


void SailDlg::updateSailGeometry()
{
    m_pSail->makeSurface();
    Objects3d::makeSailTriangulation(m_pSail);
    m_pglSailView->resetglSail();
}


void SailDlg::onPointDataChanged()
{
    m_bChanged = true;
    readPointData();

    updateSailGeometry();
    updateSailDataOutput();

    if(m_pSail->bRuledMesh()) m_pSail->makeRuledMesh(Vector3d());
    else                      m_pSail->clearRefTriangles();
    updateTriMesh();

    m_pglSailView->resetglSail();
    updateView();
}


void SailDlg::onSelectCtrlPoint(int iPoint)
{
    QModelIndex index = m_pPointModel->index(iPoint, 0);
    m_pcptPoints->setCurrentIndex(index);
}


void SailDlg::onSectionDataChanged()
{
    m_bChanged = true;
    readSectionData();

    fillPointModel();//in case an angle has been set

    updateSailGeometry();

    if(m_pSail->bRuledMesh()) m_pSail->makeRuledMesh(Vector3d());
    else                      m_pSail->clearRefTriangles();

    updateTriMesh();
    updateSailDataOutput();
    m_pglSailView->resetglSail();
    updateView();
}


void SailDlg::updateView()
{
    m_pglSailView->update();
    m_p2dSectionView->update();
}


void SailDlg::onFlipXZ()
{
    if(!m_pSail) return;

    m_pSail->flipXZ();
    m_pglSailView->resetglSail();
    updateTriMesh();
    fillSectionModel();
    fillPointModel();
    updateView();
    m_bChanged = true;
}


void SailDlg::onScaleSize()
{
    if(!m_pSail) return;

    QStringList labels({"x-factor", "y-factor", "z-factor"});
    QStringList rightlabels({QString(),QString(),QString()});
    QVector<double> vals({1.0, 1.0, 1.0});
    DoubleValueDlg dlg(this, vals, labels, rightlabels);

    if(dlg.exec()!=QDialog::Accepted)
    {
        return;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_pSail->scale(dlg.value(0), dlg.value(1), dlg.value(2));
    Objects3d::makeSailTriangulation(m_pSail);

    updateTriMesh();
    fillSectionModel();
    fillPointModel();
    m_pglSailView->setSail(m_pSail); // resets the reference length
    m_pglSailView->resetglSail();

    updateView();
    m_bChanged = true;
    QApplication::restoreOverrideCursor();
}


void SailDlg::onScaleShape()
{
    if(!m_pSail) return;
    SailScaleDlg dlg(this);
    dlg.initDialog(m_pSail);

    if(dlg.exec()==QDialog::Accepted)
    {
        if (!dlg.m_bArea && !dlg.m_bAR && !dlg.m_bTwist) return;

        QApplication::setOverrideCursor(Qt::WaitCursor);

        if(dlg.m_bArea)  m_pSail->scaleArea( dlg.m_NewArea);
        if(dlg.m_bAR)    m_pSail->scaleAR(   dlg.m_NewAR);
        if(dlg.m_bTwist) m_pSail->scaleTwist(dlg.m_NewTwist);

        m_bChanged = true;

        m_pSail->makeSurface();
        Objects3d::makeSailTriangulation(m_pSail);

        m_pglSailView->setSail(m_pSail); // resets the reference length
        m_pglSailView->resetglSail();

        if(m_pSail->bRuledMesh()) m_pSail->makeRuledMesh(Vector3d());
        else                      m_pSail->clearRefTriangles();
        updateTriMesh();

        fillSectionModel();
        fillPointModel();
        updateView();
        m_bChanged = true;
        QApplication::restoreOverrideCursor();
    }
}


void SailDlg::onTranslateSail()
{
    if(!m_pSail) return;
    TranslateDlg dlg(this);

    if(dlg.exec()==QDialog::Accepted)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        m_bChanged = true;
        m_pSail->translate(dlg.translationVector());
        Objects3d::makeSailTriangulation(m_pSail);

        updateTriMesh();
        fillSectionModel();
        fillPointModel();
        m_pglSailView->resetglSail();
        updateView();
        m_bChanged = true;
        QApplication::restoreOverrideCursor();
    }
}


void SailDlg::onScaleSection()
{
    if(!m_pSail) return;
    DoubleValueDlg dlg(this, {1.0}, {"Scale factor"}, {QString()});

    if(dlg.exec()==QDialog::Accepted)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        m_bChanged = true;
        m_pSail->scale(dlg.value(0), dlg.value(0), 1.0);

        Objects3d::makeSailTriangulation(m_pSail);

        updateTriMesh();
        fillSectionModel();
        fillPointModel();
        m_pglSailView->resetglSail();
        updateView();
        m_bChanged = true;
        QApplication::restoreOverrideCursor();
    }
}


void SailDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("SailDlg");
    {
        s_WindowGeometry     = settings.value("Geometry"          ).toByteArray();
        s_IntSplitterSizes   = settings.value("IntSplitterSizes"  ).toByteArray();
        s_ExtSplitterSizes   = settings.value("ExtSplitterSizes"  ).toByteArray();
        s_HSplitterSizes     = settings.value("HSplitterSizes"    ).toByteArray();
        s_VSplitterSizes     = settings.value("VSplitterSizes"    ).toByteArray();
        s_TableSplitterSizes = settings.value("TableSplitterSizes").toByteArray();

        s_bOutline           = settings.value("Outline",    s_bOutline).toBool();
        s_bSurfaces          = settings.value("Surfaces",   s_bSurfaces).toBool();
        s_bPanels            = settings.value("MeshPanels", s_bPanels).toBool();
        s_bCornerPts         = settings.value("CornerPts",  s_bCornerPts).toBool();

        SailSectionView::setFilled(settings.value("FillFoil", SailSectionView::bFill()).toBool());
        xfl::loadLineSettings(settings, SailSectionView::sectionStyle(), "SailSectionStyle");

        s_bRuledMesh         = settings.value("RuledMesh",          s_bRuledMesh).toBool();
    }
    settings.endGroup();
}


void SailDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("SailDlg");
    {
        settings.setValue("Geometry",           s_WindowGeometry);
        settings.setValue("IntSplitterSizes",   s_IntSplitterSizes);
        settings.setValue("ExtSplitterSizes",   s_ExtSplitterSizes);
        settings.setValue("HSplitterSizes",     s_HSplitterSizes);
        settings.setValue("VSplitterSizes",     s_VSplitterSizes);
        settings.setValue("TableSplitterSizes", s_TableSplitterSizes);

        settings.setValue("Outline",    s_bOutline);
        settings.setValue("Surfaces",   s_bSurfaces);
        settings.setValue("MeshPanels", s_bPanels);
        settings.setValue("CornerPts",  s_bCornerPts);

        settings.setValue("FillFoil",           SailSectionView::bFill());
        xfl::saveLineSettings(settings, SailSectionView::sectionStyle(), "SailSectionStyle");

        settings.setValue("RuledMesh",          s_bRuledMesh);

     }
    settings.endGroup();
}


void SailDlg::onExportToXml()
{
    if(!m_pSail) return;

    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = QString::fromStdString(m_pSail->name()).trimmed();
    strong.replace(' ', '_');
    FileName = QFileDialog::getSaveFileName(this, "Export to xml file",
                                            SaveOptions::xmlPlaneDirName() +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    XmlSailWriter writer(XFile);
    writer.writeXMLSail(m_pSail);

    XFile.close();
}


void SailDlg::onExportMeshToStl()
{
    if(!m_pSail) return;


    QString filter ="STL File (*.stl)";


    QString filename(QString::fromStdString(m_pSail->name()).trimmed()+".stl");
    filename.replace('/', '_');
    filename.replace(' ', '_');
    filename += "_mesh";

    filename = QFileDialog::getSaveFileName(nullptr, "Export to STL File",
                                            SaveOptions::STLDirName() + "/"+filename,
                                            "STL File (*.stl)",
                                            &filter);
    if(!filename.length()) return;

    Sail *pExportSail = m_pSail->clone();

    pExportSail->makeTriPanels(Vector3d());
    Objects3d::exportMeshToSTLFile(filename, pExportSail->triMesh(), 1.0);
    delete pExportSail;
}


void SailDlg::onExportTrianglesToStl()
{
    if(!m_pSail) return;
    if(!m_pSail) return;
    QString filter ="STL File (*.stl)";
    QString FileName;

    FileName = QString::fromStdString(m_pSail->name()).trimmed();
    FileName.replace('/', '_');
    FileName.replace(' ', '_');

    QFileDialog Fdlg(this);
    FileName = Fdlg.getSaveFileName(this, "Export to STL File",
                                    SaveOptions::CADDirName() + "/"+FileName+".stl",
                                    "STL File (*.stl)",
                                    &filter);

    if(!FileName.length()) return;


    bool bBinary = true;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(FileName.left(pos));

    pos = FileName.indexOf(".stl", Qt::CaseInsensitive);
    if(pos<0) FileName += ".stl";

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(window(), tr("Warning"), tr("Could not open the file for writing"));
        return;
    }

    if(bBinary)
    {
        QDataStream out(&XFile);
        out.setByteOrder(QDataStream::LittleEndian);
//        m_pFuse->exportStlTriangulation(out,1);
        Objects3d::exportTriangulation(out, 1.0, m_pSail->triangles());
    }
    else
    {
//        QTextStream out(&XFile);
    }

    XFile.close();
}


void SailDlg::onNodeDistance()
{
    m_pglSailView->setPicking(m_pglSailControls->getDistance() ? xfl::MESHNODE : xfl::NOPICK);
    if(!m_pglSailControls->getDistance()) m_pglSailView->clearMeasure();
    m_pglSailView->setSurfacePick(xfl::NOSURFACE);
    m_pglSailView->update();
}


void SailDlg::onPickedNodePair(QPair<int, int> nodepair)
{
    Node nsrc, ndest;
    if(m_pglSailView->pickType()==xfl::MESHNODE)
    {
        if(nodepair.first <0 || nodepair.first >=m_pSail->triMesh().nNodes()) return;
        if(nodepair.second<0 || nodepair.second>=m_pSail->triMesh().nNodes()) return;
        nsrc  = m_pSail->triMesh().nodeAt(nodepair.first);
        ndest = m_pSail->triMesh().nodeAt(nodepair.second);
    }
    else if(m_pglSailView->pickType()==xfl::TRIANGLENODE)
    {
        if(nodepair.first <0 || nodepair.first >=m_pSail->triangulation().nNodes()) return;
        if(nodepair.second<0 || nodepair.second>=m_pSail->triangulation().nNodes()) return;
        nsrc  = m_pSail->triangulation().nodeAt(nodepair.first);
        ndest = m_pSail->triangulation().nodeAt(nodepair.second);
    }

    // just taking a measure
    Segment3d seg(nsrc, ndest);
    m_pglSailView->setMeasure(seg);

    m_pglSailView->resetPickedNodes();
    m_pglSailView->update();
    return;
}


void SailDlg::onDefinitions()
{
    HelpImgDlg *pDlg = new HelpImgDlg(QString::fromUtf8(":/sailimages/AC48.png"), this);
    pDlg->show();
}


void SailDlg::onClearHighlighted()
{
    m_pglSailView->clearHighlightList();
    m_pglSailView->clearSegments();
    m_pglSailView->update();
}


void SailDlg::onPanelSelected(int i3)
{
   if(i3<0 || i3>=m_pSail->nPanel3()) return;

    s_TEMaxAngle = m_pfeTEAngle->value();
    s_bGuessOpposite = m_pchGuessOpposite->isChecked();
    double ccrit = cos((180.0-s_TEMaxAngle)*PI/180.0);

    if(m_ppbTEBotMid->isChecked())
    {
//        Panel3 const &p3 = m_pSail->triMesh().panelAt(i3);
        Triangle3d &t3d = m_pSail->refPanel(i3);
        Vector3d N(t3d.normal());//keep a copy of the normal vector
        if(t3d.neighbourCount()==0)
        {
            m_ppto->onAppendQText("Connect the panels before attempting to guess the T.E.\n");
            return;
        }

        if(m_pSail->removeTEindex(i3, true))
        {
            // deselected
        }
        else
        {
            if(m_pSail->isThinSurface())
            {
                if(t3d.neighbourCount()!=2)
                {
                    m_ppto->onAppendQText("A thin T.E. panel must have one and only one free edge.\n");
                    return;
                }
                m_pSail->addTEindex(i3, true);
                int iEdge = -1;
                for(iEdge=0; iEdge<3; iEdge++)
                    if(t3d.neighbour(iEdge)<0) break;

                Segment3d seg = t3d.edge(iEdge);
                //re-order the vertices so that vertices 1 and 2 are trailing
                t3d.setVertex(0, t3d.vertexAt(iEdge));
                t3d.setVertex(1, seg.vertexAt(0));
                t3d.setVertex(2, seg.vertexAt(1));
                t3d.setTriangle();

                //make sure that the panel normal is unchanged
                if(t3d.normal().dot(N)<0.0)
                {
                    t3d.setVertex(1, seg.vertexAt(1));
                    t3d.setVertex(2, seg.vertexAt(0));
                    t3d.setTriangle();
                }
            }
            else
                m_pSail->addTEindex(i3, true);


            if(s_bGuessOpposite)
            {
                for(int i3=0; i3<t3d.neighbourCount(); i3++)
                {
                    int idx = t3d.neighbour(i3);
                    if(idx>=0 && idx<m_pSail->triMesh().nPanels())
                    {
                        Panel3 &p3o = m_pSail->triMesh().panel(idx);
                        int i3o = p3o.index();
                        double cos = t3d.normal().dot(p3o.normal());
                        if(cos<ccrit)
                        {
                            m_pSail->addTEindex(i3o, false);
                            m_ppto->onAppendQText(QString::asprintf("Added opposite top side panel %d\n", i3o));
                        }
                    }
                }
            }
        }
    }
    else if(m_ppbTETop->isChecked())
    {
        Panel3 &p3 = m_pSail->triMesh().panel(i3);
        if(p3.neighbourCount()==0 && s_bGuessOpposite)
        {
            m_ppto->onAppendQText("Connect the panels before attempting to guess the T.E.\n");
            return;
        }

        if(m_pSail->removeTEindex(i3, false))
        {
            // deselected
        }
        else
        {
            m_pSail->addTEindex(i3, false);
            if(s_bGuessOpposite)
            {
                for(int i3=0; i3<p3.neighbourCount(); i3++)
                {
                    int idx = p3.neighbour(i3);
                    if(idx>=0 && idx<m_pSail->triMesh().nPanels())
                    {
                        Panel3 &p3o = m_pSail->triMesh().panel(idx);
                        int i3o = p3o.index();
                        double cos = p3.normal().dot(p3o.normal());
                        if(cos<ccrit)
                        {
                            m_pSail->addTEindex(i3o, true);
                            m_ppto->onAppendQText(QString::asprintf("Added opposite bottom side panel %d\n", i3o));
                        }
                    }
                }
            }
        }
    }

    m_pSail->setTEfromIndexes();

    m_pglSailView->resetgl3dMesh();
    m_pglSailView->update();

    m_bChanged = true;
}


void SailDlg::onThinSurface()
{
    m_pSail->setThinSurface(m_prbThin->isChecked());
    setControls();
    m_bChanged = true;
}


void SailDlg::onTopTEPanels(bool bChecked)
{
    if(m_pSail->nPanel3()<=0)
    {
        m_ppto->onAppendQText("Make the mesh before setting the T.E.\n");
        return;
    }

    Panel3 &p3t = m_pSail->triMesh().panel(0);
    if(p3t.neighbourCount()==0)
    {
        m_ppto->onAppendQText("Connect the panels before attempting to guess the T.E.\n");
        m_ppbTEBotMid->setChecked(false);
        m_ppbTETop->setChecked(false);
        return;
    }
    deselectButtons();

    if(bChecked) m_pglSailView->setPicking(xfl::PANEL3);
    else         m_pglSailView->setPicking(xfl::NOPICK);
    m_ppbTEBotMid->setChecked(false);
    m_pglSailView->selectPanels(bChecked);
    m_pglSailView->update();
}


void SailDlg::onBotTEPanels(bool bChecked)
{
    if(m_pSail->nPanel3()<=0)
    {
        m_ppto->onAppendQText("Make the mesh before setting the T.E.\n");
        return;
    }

    Panel3 &p3t = m_pSail->triMesh().panel(0);
    if(p3t.neighbourCount()==0)
    {
        m_ppto->onAppendQText("Connect the panels before attempting to guess the T.E.\n");
        m_ppbTEBotMid->setChecked(false);
        m_ppbTETop->setChecked(false);
        return;
    }

    deselectButtons();

    if(bChecked) m_pglSailView->setPicking(xfl::PANEL3);
    else         m_pglSailView->setPicking(xfl::NOPICK);
    m_ppbTETop->setChecked(false);
    m_pglSailView->selectPanels(bChecked);
    m_pglSailView->update();
}


void SailDlg::onClearTEPanels()
{
    m_pSail->clearTEIndexes();

    m_pglSailView->resetgl3dMesh();
    m_pglSailView->update();

    m_bChanged = true;
}


void SailDlg::onConnectPanels()
{
    if(m_pSail->isWingSail()) return; // connections are made at mesh construction time

    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_ppto->onAppendQText("Connecting panels...");
    
    m_pSail->triMesh().makeConnectionsFromNodePosition(true, true);
    // duplicate the connections in the refpanels - will be used when defining the TE panels;
    Q_ASSERT(int(m_pSail->refTriangles().size())==m_pSail->nPanel3());
    m_pSail->saveConnections();

    QString log(" done\n\n");
    m_ppto->onAppendQText(log);
    QApplication::restoreOverrideCursor();
}


void SailDlg::onCheckFreeEdges()
{

    std::vector<Segment3d> freeedges;

    m_pSail->triMesh().getFreeEdges(freeedges);

    QVector<Segment3d> qVec(freeedges.begin(), freeedges.end());
    m_pglSailView->setSegments(qVec);

    m_pglSailView->resetgl3dMesh();
    m_pglSailView->update();
    QString strange;
    strange = QString::asprintf("Found %d free edges\n\n", int(freeedges.size()));
    m_ppto->onAppendQText(strange);
}


void SailDlg::onCheckTEPanels()
{
    deselectButtons();

    std::vector<int> errorlist;
    bool bNoError = m_pSail->triMesh().connectTrailingEdges(errorlist);

    if(errorlist.size())
    {
        QString log;
        log = "The following panels miss an opposite TE panel:\n";
        for(uint i=0; i<errorlist.size(); i++)
        {
            log += QString::asprintf("   %4d\n", errorlist.at(i));
        }
        m_ppto->onAppendQText(log);
    }

    if(bNoError)
    {
        m_ppto->onAppendQText("No TE error.\n");
    }

    QVector<int> qVec(errorlist.begin(), errorlist.end());
    m_pglSailView->setHighlightList(qVec);
    m_pglSailView->resetgl3dMesh();
    m_pglSailView->update();

    m_ppto->onAppendQText("\n");
}


/** Finds the shortest of the two paths from clew to peak through the free dges
 * and returns it as the TE */
bool SailDlg::makeFreeTELine(Vector3d const& clew, Vector3d const& peak,
                             QVector<Segment3d> const &freeedges, QVector<Segment3d> &TELine)
{
    double dcrit = 1.e-4;
    TELine.clear();
    QVector<Segment3d> edges=freeedges;
    QVector<Segment3d> line[2];
    Vector3d v[2];
    v[0] = clew;
    v[1] = clew;

    // initialize
    int iline = 0;

    for(int is=edges.size()-1; is>=0; is--)
    {
        Segment3d const &seg = edges.at(is);
        for(int ivtx=0; ivtx<2; ivtx++)
        {
            if(seg.vertexAt(ivtx).isSame(clew, dcrit))
            {
                v[iline].set(seg.vertexAt(1-ivtx)); // move the end-point to the segment's opposite vertex
                line[iline].append({clew, v[iline]});
                edges.remove(is);
                iline++;
                if(iline==2) break;
            }
            if(iline==2) break;
        }
        if(iline==2) break;
    }
    if(iline!=2) return false; // we should find two lines starting at the clew


    for(iline=0; iline<2; iline++)
    {
        bool bPeak = false;
        for(int is=edges.size()-1; is>=0; is--)
        {
            Segment3d const &seg = edges.at(is);
            for(int ivtx=0; ivtx<2; ivtx++)
            {
                if(seg.vertexAt(ivtx).isSame(v[iline], dcrit))
                {
                    line[iline].append({v[iline], seg.vertexAt(1-ivtx)});
                    v[iline].set(seg.vertexAt(1-ivtx)); // move the end-point to the segments opposite vertex
                    if(v[iline].isSame(peak, dcrit))
                    {
                        bPeak=true; // we've reached the peak, the line is complete
                        break; // no need to check the other vertex
                    }
                    edges.remove(is);
                    is = edges.size(); // rescan them all
                    break; // no need to check the other vertex
                }
            }
            if(bPeak)
                break; // move on to next line
        }
    }

    double length[]{0.0,0.0};
    for(iline=0; iline<2; iline++)
    {
        length[iline] += line[iline].length();
    }

    if(length[0]<length[1]) TELine = line[0];
    else                    TELine = line[1];

    return true;
}


void SailDlg::onGuessTE()
{
    deselectButtons();
    m_ppbTETop->setChecked(false);
    m_ppbTEBotMid->setChecked(false);

    if(m_pSail->nPanel3()<=0)
    {
        m_ppto->onAppendQText("Build the mesh before setting the T.E.\n");
        return;
    }

    Panel3 &p3t = m_pSail->triMesh().panel(0);
    if(p3t.neighbourCount()==0)
    {
        m_ppto->onAppendQText("Connect the panels before attempting to guess the T.E.\n");
        return;
    }

    m_pSail->clearTEIndexes();

    if(m_pSail->isThinSurface())
    {
        if(guessThinTE())
            m_ppto->onAppendQText(QString::asprintf("Found %d TE panels.\n", int(m_pSail->botMidTEIndexes().size())));
    }
    else
    {
        if(guessThickTE())
            m_ppto->onAppendQText(QString::asprintf("Found %d pairs of TE panels.\n", int(m_pSail->topTEIndexes().size())));
    }

    m_pSail->makeTriPanels(Vector3d());
    m_pSail->setTEfromIndexes();
    m_pSail->updateStations();

    m_pglSailView->clearSegments();
    m_pglSailView->resetglSail();
    m_pglSailView->resetgl3dMesh();
    m_pglSailView->update();

    m_bChanged = true;
}


bool SailDlg::guessThickTE()
{
    s_TEMaxAngle = m_pfeTEAngle->value();
    double ccrit = cos((180.0-s_TEMaxAngle)*PI/180.0);

    TriMesh &mesh3 = m_pSail->triMesh();

    int iTE=0;
    m_pSail->clearTEIndexes();

    double ycrit = 0.1;

    for(int i3t=0; i3t<mesh3.nPanels(); i3t++)
    {
        Panel3 &p3t = mesh3.panel(i3t);
        for(int i3b=0; i3b<p3t.neighbourCount(); i3b++)
        {
            int idx = p3t.neighbour(i3b);
            if(idx>=0 && idx<mesh3.nPanels())
            {
                Panel3 &p3b = mesh3.panel(idx);
                double cos = p3t.normal().dot(p3b.normal());
                if(cos<ccrit)
                {
                    // we have a TE
                    if(p3t.normal().y>ycrit && p3b.normal().y<-ycrit)
                    {
                        m_pSail->addTEindex(p3t.index(), false);
                        m_pSail->addTEindex(p3b.index(), true);
                    }
                    else if(p3b.normal().y>ycrit && p3t.normal().y<-ycrit)
                    {
                        m_pSail->addTEindex(p3t.index(), true);
                        m_pSail->addTEindex(p3b.index(), false);
                    }
                    else if(p3t.normal().y>ycrit && p3b.normal().y<-ycrit)
                    {
                        m_pSail->addTEindex(p3t.index(), false);
                        m_pSail->addTEindex(p3b.index(), true);
                    }
                    else if(p3b.normal().y>ycrit && p3t.normal().y<-ycrit)
                    {
                        m_pSail->addTEindex(p3t.index(), true);
                        m_pSail->addTEindex(p3b.index(), false);
                    }
                    else
                    {
                        //anything
                    }
                    iTE++;
                    break;
                }
            }
        }
    }
    return true;
}


/**
 * Guesses the TE panels using the following workflow
 * (1) identify the free edges
 * (2) find the shortest path of free edge segments from clew to peak
 * (3) identify which triangles have one such segment as an edge and mark them as trailing
 * (4) reorder the vertices of the trailing panels so that their vertices 1 and 2 are trailing
 */
bool SailDlg::guessThinTE()
{
    QString strange;
    m_pSail->clearTEIndexes();

    std::vector<Segment3d> freeedges;

    m_pSail->triMesh().getFreeEdges(freeedges);
    QVector<Segment3d> qVec(freeedges.begin(), freeedges.end());
    m_pglSailView->setSegments(qVec);

    if(freeedges.size()==0)
    {
        strange ="No free edges found: connect the panels\n";
        m_ppto->onAppendQText(strange);
        return false;
    }

    if(m_pSail->clew().isSame(m_pSail->peak(), 0.0001))
    {
        strange = "Clew is same as peak: cannot make TE curve\n";
        m_ppto->onAppendQText(strange);
        return false;
    }

    QVector<Segment3d> TELine;
    bool bRes = makeFreeTELine(m_pSail->clew(), m_pSail->peak(), qVec, TELine);
    if(!bRes)
    {
        strange ="Failed to build the TE line: invalid geometry\n";
        m_ppto->onAppendQText(strange);
        return false;
    }

    m_pSail->m_BotMidTEIndexes.clear();
    m_pSail->m_TopTEIndexes.clear();
    std::vector<Triangle3d> &triangles = m_pSail->m_RefTriangles;

    for(uint it3=0; it3<triangles.size(); it3++)
    {
        Triangle3d &t3d = triangles[it3];
        Vector3d N(t3d.normal());//keep a copy of the normal vector
        for(int il=0; il<TELine.size(); il++)
        {
            Segment3d const &seg = TELine.at(il);
            int iEdge = t3d.edgeIndex(seg, 0.0001);
            if(iEdge>=0)
            {
                m_pSail->m_BotMidTEIndexes.push_back(it3);

                //re-order the vertices so that vertices 1 and 2 are trailing
                t3d.setVertex(0, t3d.vertexAt(iEdge));
                t3d.setVertex(1, seg.vertexAt(0));
                t3d.setVertex(2, seg.vertexAt(1));
                t3d.setTriangle();

                //make sure that the panel normal is unchanged
                if(t3d.normal().dot(N)<0.0)
                {
                    t3d.setVertex(1, seg.vertexAt(1));
                    t3d.setVertex(2, seg.vertexAt(0));
                    t3d.setTriangle();
                }
                TELine.remove(il);
                break;
            }
        }
        if(TELine.isEmpty()) break; //done
    }

    return true;
}


void SailDlg::onPickEdge(bool bPick)
{
    if(bPick)
        m_pglSailView->setPicking(xfl::SEGMENT3D);
    else
        m_pglSailView->stopPicking();

    m_pglSailView->showEdgeNodes(bPick);
    m_pglSailView->update();
}


void SailDlg::onPickedEdge(int , int )
{
}


void SailDlg::onMakeEdgeSplits()
{
}


void SailDlg::makeEdgeNodes(std::vector<Node> &)
{
}


void SailDlg::onExportToStep()
{
    if(!m_pSail) return;
    if(!m_pSail->isSplineSail() && !m_pSail->isNURBSSail()) return;

    TopoDS_Shape shape;
    std::string log;
    if(!m_pSail->makeOccShell(shape, log))
        return;

    CADExportDlg dlg(this);
    dlg.init(shape, QString::fromStdString(m_pSail->name()));
    dlg.exec();
}




