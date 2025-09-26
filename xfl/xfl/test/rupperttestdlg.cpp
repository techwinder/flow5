/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#define _MATH_DEFINES_DEFINED


#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QLabel>

#include "rupperttestdlg.h"


#include <xflpanels/mesh/interfaces/pslgtestview.h>
#include <xfl3d/controls/gl3dcontrols.h>
#include <xflcore/xflcore.h>
#include <xflgeom/geom2d/segment2d.h>
#include <xflgeom/geom2d/triangle2d.h>
#include <xflgeom/geom2d/vector2d.h>
#include <xflgeom/geom_globals/geom_global.h>
#include <xflmath/mathelem.h>
#include <xflpanels/panels/panel3.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>

bool RuppertTestDlg::s_bTrianglePSLG = true;
bool RuppertTestDlg::s_bIncludeFoilPSLG=true;
bool RuppertTestDlg::s_bIncludeFreeEdgePSLG=true;

int RuppertTestDlg::s_Nx=1;
int RuppertTestDlg::s_Ny=2;
double RuppertTestDlg::s_ySide=2.5;
double RuppertTestDlg::s_yMid =1.0;
double RuppertTestDlg::s_xLength=3.0;
double RuppertTestDlg::s_FoilXPos = 2.5;
double RuppertTestDlg::s_FoilYPos = 0.3;

int RuppertTestDlg::s_MaxMeshIters=1;
int RuppertTestDlg::s_MaxPanelCount=5000;

double RuppertTestDlg::s_MinEdgeLength=0.05;
double RuppertTestDlg::s_MaxEdgeLength=1.0;

QByteArray RuppertTestDlg::s_Geometry;


RuppertTestDlg::RuppertTestDlg(QWidget *parent) : QDialog(parent)
{
//    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Mesher test");

	setupLayout();
    connectSignals();
}


void RuppertTestDlg::initDialog(PSLG2d const &contourPSLG, QVector<PSLG2d> const &innerPSLG, double minedgelength, double maxedgelength)
{
    initDialog();

    s_MinEdgeLength = minedgelength;
    s_MaxEdgeLength = maxedgelength;
    m_pdeMinEdgeSize->setValue(s_MinEdgeLength);
    m_pdeMaxEdgeSize->setValue(s_MaxEdgeLength);

    m_ContourPSLG = contourPSLG;
    m_PSLG.append(m_ContourPSLG);
    if(innerPSLG.size())
    {
        m_FoilPSLG = innerPSLG.first();
        m_PSLG.append(m_FoilPSLG);
    }

    QString strange;
    strange = QString::asprintf("   Total:          %d segments\n", int(m_PSLG.size()));
    updateOutput(strange+"\n");

    m_RuppMesher.clearAll();
    m_RuppMesher.setPSLG(m_PSLG);

    m_pRuppView->setPSLG(m_PSLG);
    m_pRuppView->listNonCompliant(maxedgelength);
    m_pRuppView->update();
}


void RuppertTestDlg::initDialog()
{
    m_pchIncludeFoilPSLG->setChecked(s_bIncludeFoilPSLG);
    m_pchIncludeFreeEdgePSLG->setChecked(s_bIncludeFreeEdgePSLG);

	//PSLG
    m_xDist = xfl::UNIFORM;
    m_yDist = xfl::UNIFORM;

    m_prbTrianglePSLG->setChecked(s_bTrianglePSLG);
    m_prbComplexPSLG->setChecked(!s_bTrianglePSLG);

    m_pdeYSide->setValue(s_ySide);
    m_pdeYMid->setValue(s_yMid);
    m_pdeXLength->setValue(s_xLength);

    m_pieNx->setValue(s_Nx);
    m_pieNy->setValue(s_Ny); // set such that there is a vertice coincident with central PSLG
    m_pdeFoilXPos->setValue(s_FoilXPos);
    m_pdeFoilYPos->setValue(s_FoilYPos);

    s_MinEdgeLength = std::min(s_MinEdgeLength, s_MaxEdgeLength);
    m_pdeMinEdgeSize->setValue(s_MinEdgeLength);
    m_pdeMaxEdgeSize->setValue(s_MaxEdgeLength);

    m_pdeQualityBound->setValue(Triangle2d::qualityBound());
    m_pieMaxMeshIters->setValue(s_MaxMeshIters);
    m_pieMaxPanelCount->setValue(s_MaxPanelCount);
}


void RuppertTestDlg::setupLayout()
{
    m_pRuppView = new PSLGTestView(this);

    QVBoxLayout *pVerticesLayout = new QVBoxLayout;
    {
        m_pgbPSLG = new QGroupBox("PSLG");
        {
            QGridLayout *pPSLGLayout = new QGridLayout;
            {
                QLabel *pLabPSLGType     = new QLabel("PSLG type:");
                QLabel *pLabXSide     = new QLabel("Side height");
                QLabel *pLabXMid      = new QLabel("Mid height");
                QLabel *pLabYSide     = new QLabel("X-length");

                QLabel *pLabNx         = new QLabel("Nx, Ny:");
                QLabel *pLabFoilPos    = new QLabel("Foil LE pos.:");
                pLabPSLGType->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                pLabXSide->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                pLabXMid->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                pLabYSide->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                pLabNx->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                pLabFoilPos->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

                m_prbTrianglePSLG = new QRadioButton("Triangle");
                m_prbComplexPSLG  = new QRadioButton("Complex");

                m_pdeYSide   = new FloatEdit;
                m_pdeYMid    = new FloatEdit;
                m_pdeXLength = new FloatEdit;
                m_pieNx = new IntEdit;
                m_pieNy = new IntEdit;

                m_pdeFoilXPos = new FloatEdit;
                m_pdeFoilYPos = new FloatEdit;
                m_pchIncludeFoilPSLG = new QCheckBox("Foil PSLG");
                m_pchIncludeFreeEdgePSLG = new QCheckBox("free edge PSLG");
                m_ppbDefaultPSLG = new QPushButton("Make PSLG");

                pPSLGLayout->addWidget(pLabPSLGType,        1,1);
                pPSLGLayout->addWidget(m_prbTrianglePSLG,   1,2);
                pPSLGLayout->addWidget(m_prbComplexPSLG,    1,3);
                pPSLGLayout->addWidget(pLabXSide,           2,1);
                pPSLGLayout->addWidget(pLabXMid,            3,1);
                pPSLGLayout->addWidget(pLabYSide,           4,1);
                pPSLGLayout->addWidget(m_pdeYSide,        2,2);
                pPSLGLayout->addWidget(m_pdeYMid,         3,2);
                pPSLGLayout->addWidget(m_pdeXLength,      4,2);

                pPSLGLayout->addWidget(pLabNx,              5,1);
                pPSLGLayout->addWidget(m_pieNx,           5,2);
                pPSLGLayout->addWidget(m_pieNy,           5,3);
                pPSLGLayout->addWidget(pLabFoilPos,         6,1);
                pPSLGLayout->addWidget(m_pdeFoilXPos,     6,2);
                pPSLGLayout->addWidget(m_pdeFoilYPos,     6,3);
                pPSLGLayout->addWidget(m_pchIncludeFoilPSLG,    7,1);
                pPSLGLayout->addWidget(m_pchIncludeFreeEdgePSLG,7,2);
                pPSLGLayout->addWidget(m_ppbDefaultPSLG,        7,3);
            }
            m_pgbPSLG->setLayout(pPSLGLayout);
        }

        QGroupBox *pMeshBox = new QGroupBox("Mesh");
        {
            QGridLayout *pMeshLayout = new QGridLayout;
            {
                QLabel *pLabSize      = new QLabel("Edge size min/max:");
                QLabel *pLabQuality   = new QLabel("Max. CCR/min edge:");
                QLabel *labQualityMax = new QLabel(">sqrt(2)");
                QLabel *pLabMaxIter   = new QLabel("Max. iterations:");
                QLabel *pLabMaxPanels = new QLabel("Max. panel count:");
                QLabel *pLabNSeed     = new QLabel("NSeed:");
                pLabSize->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                pLabQuality->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                pLabMaxIter->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                pLabMaxPanels->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                pLabNSeed->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

                m_pdeMinEdgeSize   = new FloatEdit;
                m_pdeMaxEdgeSize   = new FloatEdit;
                m_pdeQualityBound  = new FloatEdit(sqrt(2.0), 3);
                QString tip;
                tip = tr("The upper limit for the quality of the triangular panels.\n"
                      "The quality is defined as the ratio of the radius of the triangle's circumcircle to the length of the shortest edge.\n"
                      "See for instance:\n"
                      "Delaunay Refinement Algorithms for Triangular Mesh Generation, Jonathan R. Shewchuk, May 21, 2001.\n"
                      "A max. quality factor equal to sqrt(2) is recommended.");
                m_pdeQualityBound->setToolTip(tip);
                m_pieMaxMeshIters  = new IntEdit;
                m_pieMaxPanelCount = new IntEdit;
                tip = tr("The mesh operation will stop if the number of triangular panels\n"
                      "exceeds this value. Prevents excessive mesh sizes and and lengthy operations.\n"
                      "Recommendation 1000-3000.");
                m_pieMaxPanelCount->setToolTip(tip);

                m_ppbRestorePSLG = new QPushButton("Restore PSLG");
                m_ppbSplitOversizePSLG = new QPushButton("Split oversized PSLG");
                m_ppbMakeDelaunay = new QPushButton("Delaunay");
                m_ppbMakeCDelaunay = new QPushButton("Constrained D.");
                m_ppbClearFoilPSLG = new QPushButton("Clear foil PSLG");
                m_ppbClearContourPSLG = new QPushButton("Clear contour PSLG");
                m_ppbDoRuppert = new QPushButton("do Ruppert");
                m_ppbSplitOversize = new QPushButton("Split oversized triangles");
                m_ppbFlipDelaunay = new QPushButton("Flip Delaunay");
                pMeshLayout->addWidget(pLabSize,                 1,1);
                pMeshLayout->addWidget(m_pdeMinEdgeSize,       1,2);
                pMeshLayout->addWidget(m_pdeMaxEdgeSize,       1,3);
                pMeshLayout->addWidget(pLabQuality,              2,1);
                pMeshLayout->addWidget(m_pdeQualityBound,      2,2);
                pMeshLayout->addWidget(labQualityMax,            2,3);
                pMeshLayout->addWidget(pLabMaxPanels,            3,1);
                pMeshLayout->addWidget(m_pieMaxPanelCount,     3,2);

                pMeshLayout->addWidget(m_ppbRestorePSLG,       5,1);
                pMeshLayout->addWidget(m_ppbSplitOversizePSLG, 5,2);
                pMeshLayout->addWidget(m_ppbMakeDelaunay,      8,1);
                pMeshLayout->addWidget(m_ppbMakeCDelaunay,     8,2);
                pMeshLayout->addWidget(m_ppbClearFoilPSLG,     9,1);
                pMeshLayout->addWidget(m_ppbClearContourPSLG,  9,2);
                pMeshLayout->addWidget(m_ppbSplitOversize,     10,1);
                pMeshLayout->addWidget(m_ppbFlipDelaunay,      10,2);
                pMeshLayout->addWidget(m_ppbDoRuppert,         11,1);
                pMeshLayout->addWidget(pLabMaxIter,              11,2);
                pMeshLayout->addWidget(m_pieMaxMeshIters,      11,3);
                pMeshLayout->setRowStretch(                      12,1);
            }
            pMeshBox->setLayout(pMeshLayout);
        }

        QHBoxLayout *pIndexesLayout = new QHBoxLayout;
        {
            m_pchTriIndexes = new QCheckBox(tr("Triangle indexes"));
            m_pchPSLGIndexes = new QCheckBox(tr("PSLG indexes"));
            pIndexesLayout->addWidget(m_pchTriIndexes);
            pIndexesLayout->addWidget(m_pchPSLGIndexes);
            pIndexesLayout->addStretch();
        }

        m_ppbDoAllSteps = new QPushButton("do all steps");

        m_ppto = new PlainTextOutput;

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        {
            QPushButton *pClearOutput = new QPushButton(tr("Clear output"));
            m_pButtonBox->addButton(pClearOutput, QDialogButtonBox::ActionRole);
            connect(pClearOutput, SIGNAL(clicked()), m_ppto, SLOT(clear()));
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
        }
        pVerticesLayout->addWidget(m_pgbPSLG);
        pVerticesLayout->addWidget(pMeshBox);
        pVerticesLayout->addWidget(m_ppbDoAllSteps);
        pVerticesLayout->addWidget(m_ppto);
        pVerticesLayout->addLayout(pIndexesLayout);
        pVerticesLayout->addWidget(m_pButtonBox);
    }

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        pMainLayout->addWidget(m_pRuppView);
        pMainLayout->addLayout(pVerticesLayout);
        pMainLayout->setStretchFactor(m_pRuppView,5);
    }

    setLayout(pMainLayout);
}


void RuppertTestDlg::connectSignals()
{
    connect(m_pchTriIndexes,        SIGNAL(toggled(bool)), SLOT(onIndexes()));
    connect(m_pchPSLGIndexes,       SIGNAL(toggled(bool)), SLOT(onIndexes()));
    connect(m_ppbDoAllSteps,        SIGNAL(clicked(bool)), SLOT(onDoAllSteps()));
    connect(m_ppbDefaultPSLG,       SIGNAL(clicked(bool)), SLOT(onMakePSLG()));
    connect(m_ppbMakeDelaunay,      SIGNAL(clicked(bool)), SLOT(onMakeDelaunay()));
    connect(m_ppbMakeCDelaunay,     SIGNAL(clicked(bool)), SLOT(onMakeConstrainedDelaunay()));
    connect(m_ppbClearFoilPSLG,     SIGNAL(clicked(bool)), SLOT(onClearFoilPSLG()));
    connect(m_ppbClearContourPSLG,  SIGNAL(clicked(bool)), SLOT(onClearContourPSLG()));
    connect(m_ppbDoRuppert,         SIGNAL(clicked(bool)), SLOT(onDoRuppert()));
    connect(m_ppbRestorePSLG,       SIGNAL(clicked(bool)), SLOT(onRestorePSLG()));
    connect(m_ppbSplitOversizePSLG, SIGNAL(clicked(bool)), SLOT(onSplitOversizePSLG()));
    connect(m_ppbSplitOversize,     SIGNAL(clicked(bool)), SLOT(onSplitOversizeTriangles()));
    connect(m_ppbFlipDelaunay,      SIGNAL(clicked(bool)), SLOT(onFlipDelaunay()));
}


void RuppertTestDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
    m_pRuppView->setReferenceLength(s_xLength);
    m_pRuppView->resetDefaultScale();
    m_pRuppView->setAutoUnits();
}


void RuppertTestDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
    readParams();
}


void RuppertTestDlg::readParams()
{
    s_bTrianglePSLG = m_prbTrianglePSLG->isChecked();
    s_bIncludeFoilPSLG = m_pchIncludeFoilPSLG->isChecked();
    s_bIncludeFreeEdgePSLG = m_pchIncludeFreeEdgePSLG->isChecked();

    s_MaxMeshIters = m_pieMaxMeshIters->value();
    s_MaxPanelCount = m_pieMaxPanelCount->value();
    s_MinEdgeLength = m_pdeMinEdgeSize->value();
    s_MaxEdgeLength = m_pdeMaxEdgeSize->value();

    Triangle2d::setQualityBound(m_pdeQualityBound->value());
}


void RuppertTestDlg::setRuppParams()
{
    readParams();
    m_RuppMesher.setMaxIter(s_MaxMeshIters);
    m_RuppMesher.setMaxPanelCount(s_MaxPanelCount);
    m_RuppMesher.setMinEdgeLength(s_MinEdgeLength);
    m_RuppMesher.setMaxEdgeLength(s_MaxEdgeLength);
}


/** make a random Planar Straight Line Graph (PSLG) */
void RuppertTestDlg::onMakePSLG()
{
    s_bTrianglePSLG = m_prbTrianglePSLG->isChecked();

    QString strange, log;
    log = "Making PSLG:\n";
    m_PSLG.clear();

    makeContourPSLG();
    m_PSLG.append(m_ContourPSLG);
    strange = QString::asprintf("   Contour PSLG:   %d segments\n", int(m_PSLG.size()));
    log+=strange;

    s_bIncludeFoilPSLG = m_pchIncludeFoilPSLG->isChecked();
    if(s_bIncludeFoilPSLG)
    {
        makeFoilPSLG();
        m_RuppMesher.clearInnerPSLG();
        m_PSLG.append(m_FoilPSLG);
        strange = QString::asprintf("   Foil PSLG:      %d segments\n", int(m_FoilPSLG.size()));
        log += strange;
    }

    s_bIncludeFreeEdgePSLG = m_pchIncludeFreeEdgePSLG->isChecked();
    if(s_bIncludeFreeEdgePSLG)
    {
        double y = (s_ySide+s_yMid)/2.0/2.0;
        m_PSLG.push_back({{s_xLength*4/5, y*2/3},  {s_xLength*4/5, -y*2/3}});
        strange = QString::asprintf("   Free edge PSLG: %d segments\n", 1);
        log += strange;
    }

    strange = QString::asprintf("   Total:          %d segments\n", int(m_PSLG.size()));
    log += strange;
    updateOutput(log+"\n");

    m_RuppMesher.clearAll();
    m_RuppMesher.setPSLG(m_PSLG);

    m_pRuppView->clearEncroached();
    m_pRuppView->setPSLG(m_PSLG);
    m_pRuppView->clearTriangles();
    m_pRuppView->update();
}


void RuppertTestDlg::onMakeDelaunay()
{
	QVector<Vector2d> vertices;

    m_RuppMesher.addPSLGVertices(vertices);

    setRuppParams();

	Triangle2d supertriangle;
    m_RuppMesher.makeDelaunayTriangulation(vertices, m_RuppMesher.lastTriangles(), supertriangle, false);

    updateRuppStatus();
}


void RuppertTestDlg::onMakeConstrainedDelaunay()
{
    QVector<Vector2d> vertices;
    m_RuppMesher.addPSLGVertices(vertices);

    QString strange;
    QString log = "Making constrained Delaunay triangulation\n";
    int nRemoved = m_RuppMesher.lastPSLG().cleanNullSegments();
    strange = QString::asprintf("   Removed %d null PSLG segments\n", nRemoved);
    updateOutput(log+strange);

    setRuppParams();

    int n = 200; // max number of pslg insertions, to put a limit
    m_RuppMesher.makeConstrainedDelaunay(vertices, m_RuppMesher.lastPSLG(), m_RuppMesher.lastTriangles(), n, true);

    updateRuppStatus();
}


void RuppertTestDlg::updateRuppStatus()
{
    QString log, str;
    QVector<int> encroached, skinny, longtris;
    m_RuppMesher.listNonCompliant(encroached, skinny, longtris);

    str = QString::asprintf("Size of modified PSLG:   %d\n", int(m_RuppMesher.lastPSLG().size()));
    log += "   " + str;
    str = QString::asprintf("Nb. of triangles:        %d\n", int(m_RuppMesher.lastTriangles().size()));
    log += "   " + str;
    str = QString::asprintf("Nb. of skinny triangles: %d\n", int(skinny.size()));
    log += "   " + str;
    str = QString::asprintf("Nb. of long triangles:   %d\n", int(longtris.size()));
    log += "   " + str;
    str = QString::asprintf("Nb. of encroached segs:  %d\n", int(encroached.size()));
    log += "   " + str;

    updateOutput(log+"\n");

    m_pRuppView->setTriangles(m_RuppMesher.lastTriangles());
    m_pRuppView->setPSLG(m_RuppMesher.lastPSLG());
    m_pRuppView->setSkinny(skinny);
    m_pRuppView->setLongTriangles(longtris);
    m_pRuppView->clearFlipList();
    m_pRuppView->setEncroachedSegments(encroached);
    m_pRuppView->update();
}


void RuppertTestDlg::onClearFoilPSLG()
{
    if(!s_bIncludeFoilPSLG) return;
    int removed = m_RuppMesher.clearTrianglesInPSLG(m_FoilPSLG);
    QString strange;
    strange = QString::asprintf("Removed %d triangles, new size=%d\n\n", removed, m_RuppMesher.lastTriangleCount());
    updateOutput(strange);

    updateRuppStatus();
}


void RuppertTestDlg::onClearContourPSLG()
{
    int removed = m_RuppMesher.clearTrianglesOutsidePSLG(m_ContourPSLG, m_RuppMesher.lastTriangles());
    QString strange;
    strange = QString::asprintf("Removed %d triangles, new size=%d\n\n", removed, m_RuppMesher.lastTriangleCount());
    updateOutput(strange);

    updateRuppStatus();
}


void RuppertTestDlg::onDoAllSteps()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString strange;

    setRuppParams();

    QVector<PSLG2d> innerpslg = {m_FoilPSLG};
    m_RuppMesher.setPSLG(m_PSLG);
    m_RuppMesher.doAllMeshSteps(m_PSLG, m_ContourPSLG, innerpslg);
    strange = QString::asprintf("Mesh initialized:\n   %d triangles\n   %d PSLG segments\n",
                    m_RuppMesher.lastTriangleCount(), m_RuppMesher.lastPSLGCount());

    updateOutput(strange);
    updateRuppStatus();
    QApplication::restoreOverrideCursor();
}


void RuppertTestDlg::onDoRuppert()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString log, strange;
    bool bConverged=false;

    setRuppParams();

    strange = QString::asprintf("Resuming mesh with:\n   %d triangles\n   %d PSLG segments\n",
                    m_RuppMesher.lastTriangleCount(), m_RuppMesher.lastPSLGCount());

    updateOutput(strange);

    m_RuppMesher.doRuppert(m_ContourPSLG, s_MaxMeshIters, s_MaxPanelCount, log, bConverged);

    updateOutput(log);

    updateRuppStatus();
    QApplication::restoreOverrideCursor();
}


void RuppertTestDlg::onRestorePSLG()
{
    QString log, strange;
    QVector<int> encroached, skinny, longTris;
    setRuppParams();

    updateOutput("Restoring PSLG\n");
    m_RuppMesher.clearAll();
    m_RuppMesher.setPSLG(m_PSLG);
    updateRuppStatus();
}


void RuppertTestDlg::onSplitOversizePSLG()
{
    QString log, strange;
    QVector<int> encroached, skinny, longTris;
    setRuppParams();

    updateOutput("Splitting PSLG to max. edge length\n");
    int nSplit = m_RuppMesher.splitOversizePSLG();
    strange = QString::asprintf("   split %d PSLG segments\n", nSplit);
    updateOutput(strange);

    updateRuppStatus();
}


void RuppertTestDlg::onSplitOversizeTriangles()
{
    QString log, strange;
    QVector<int> encroached, skinny, longTris;
	setRuppParams();

    updateOutput("Splitting oversized triangles\n");
    int nSplit = m_RuppMesher.splitOversizedTriangles(false);
    if(nSplit<0) strange = "   Max. number of panels exceeded\n";
    else         strange = QString::asprintf("   split %d triangles\n", nSplit);
    updateOutput(strange);

    updateRuppStatus();
}


void RuppertTestDlg::onFlipDelaunay()
{
    QString log, strange;

    QVector<int> fliplist;

    setRuppParams();

    updateOutput("Splitting oversized triangles\n");
    int nFlipped = m_RuppMesher.flipDelaunay(fliplist);
    strange = QString::asprintf("   performed %d flips\n", nFlipped);
    updateOutput(strange);

    updateRuppStatus();
    m_pRuppView->setFlipList(fliplist);
    m_pRuppView->update();
}


void RuppertTestDlg::updateOutput(QString const &msg)
{
    m_ppto->onAppendThisPlainText(msg);
//    
}


void RuppertTestDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton) close();
}


void RuppertTestDlg::makeFoilPSLG()
{
    m_FoilPSLG.clear();

    s_FoilXPos = m_pdeFoilXPos->value();
    s_FoilYPos = m_pdeFoilYPos->value();

    QVector<Vector2d> pts;
//    pts.push_back({    1,      0.0006}); // avoid ultrafin pslg at TE
    pts.push_back({    1     , -0.0006});
    pts.push_back({0.91374   , 0.020501});
    pts.push_back({0.78737   , 0.046262});
    pts.push_back({0.66105   , 0.067444});
    pts.push_back({0.53474   , 0.082871});
    pts.push_back({0.40842   , 0.090948});
    pts.push_back({0.28211   , 0.090081});
    pts.push_back({0.15579   , 0.076753});
    pts.push_back({0.044937  , 0.041741});
    pts.push_back({0.0022087 , 0.0061878});
    pts.push_back({0.0022087 , -0.008177});
    pts.push_back({0.044937  , -0.025315});
    pts.push_back({0.15579   , -0.030267});
    pts.push_back({0.28211   , -0.02699});
    pts.push_back({0.40842   , -0.022323});
    pts.push_back({0.53474   , -0.017684});
    pts.push_back({0.66105   , -0.013051});
    pts.push_back({0.78737   , -0.008408});
    pts.push_back({0.91374   , -0.003768});
//    pts.push_back({    1     , -0.0006});

    for(int i=1; i<pts.size(); i++)
    {
        m_FoilPSLG.append({{-pts.at(i-1).x+s_FoilXPos, pts.at(i-1).y+s_FoilYPos},
                           {-pts.at(i).x  +s_FoilXPos, pts.at(i).y  +s_FoilYPos}});
    }
    m_FoilPSLG.append({{-pts.last().x +s_FoilXPos, pts.last().y +s_FoilYPos},
                       {-pts.first().x+s_FoilXPos, pts.first().y+s_FoilYPos}});
}


void RuppertTestDlg::makeContourPSLG()
{
    m_ContourPSLG.clear();

    s_ySide   = m_pdeYSide->value();
    s_yMid    = m_pdeYMid->value();
    s_xLength = m_pdeXLength->value();

    s_Nx = m_pieNx->value();
    s_Ny = m_pieNy->value();

    if(s_bTrianglePSLG)
    {
        //bot side
        s_Nx = std::max(s_Nx, 2);
        Vector2d pt0(0,0), pt1;
        for(int ix=1; ix<s_Nx; ix++)
        {
            double l =double(ix)/double(s_Nx-1) * s_xLength;
            pt1 = {l,0};
            m_ContourPSLG.append({pt0, pt1});
            pt0 = pt1;
        }
        // right side
        for(int iy=1; iy<s_Ny; iy++)
        {
            double l =double(iy)/double(s_Ny-1) * s_ySide;
            pt1 = {s_xLength,l};
            m_ContourPSLG.append({pt0, pt1});
            pt0 = pt1;
        }
        // hypotenuse
        for(int ix=s_Nx-2; ix>=0; ix--)
        {
            double lx =double(ix)/double(s_Nx-1) * s_xLength;
            double ly =double(ix)/double(s_Nx-1) * s_ySide;
            pt1 = {lx,ly};
            m_ContourPSLG.append({pt0, pt1});
            pt0 = pt1;
        }
    }
    else
    {
        double y0=0.0, x0=0.0, y=0.0, x=0.0;
        QVector<double> Points;

        //LEFTSIDE
        getPointDistribution(Points, s_Ny, m_xDist);
        x0 = 0.0;
        y0 = Points.at(0) * s_ySide - s_ySide/2.0;
        for(int l=1; l<Points.size(); l++)
        {
            y = Points.at(l) * s_ySide - s_ySide/2.0;
            m_ContourPSLG.append({Vector2d(x0, y0), Vector2d(x0, y)});
            y0=y;
        }

        //RIGHT SIDE
        getPointDistribution(Points, s_Ny, m_xDist);
        x0 = s_xLength;
        y0 = Points.at(0) * s_ySide - s_ySide/2.0;
        for(int l=1; l<Points.size(); l++)
        {
            y = Points.at(l) * s_ySide - s_ySide/2.0;
            m_ContourPSLG.append({Vector2d(x0, y0), Vector2d(x0, y)});
            y0=y;
        }


        // top and bot: SINE  distributions
        getPointDistribution(Points, s_Nx, m_yDist);

        // make a side parabola
        double xs = s_ySide/2.0;
        double xm = s_yMid/2.0;
        double ys = Points.at(0) * s_xLength - s_xLength/2.0;

        y0 = -xs;
        x0 =  ys;

        double tx = s_xLength/2.0; // translate the parabola along the x-axis

        double a = (xs-xm)/ys/ys;
        double c = xm;
        for(int l=1; l<Points.size(); l++)
        {
            x = (Points.at(l) - 0.5)* s_xLength;
            y = -c -a*x*x;
            m_ContourPSLG.append({Vector2d(x0+tx, y0), Vector2d(x+tx, y)});
            y0=y;
            x0=x;
        }

        y0 = s_ySide/2.0;
        x0 = Points.at(0) * s_xLength - s_xLength/2.0;

        for(int l=1; l<Points.size(); l++)
        {
            x = Points.at(l) * s_xLength - s_xLength/2.0;
            y = c +a*x*x;
            m_ContourPSLG.append({Vector2d(x0+tx, y0), Vector2d(x+tx, y)});
            y0=y;
            x0=x;
        }
    }
}


double RuppertTestDlg::makeUniqueVertexList(QVector<Triangle2d> const &PanelList, QVector<Vector2d> &VertexList)
{
	double maxlength = 0.1;

	VertexList.clear();
	for(int i=0; i<PanelList.size(); i++)
	{
		for(int j=0; j<3; j++)
		{
			Vector2d V = PanelList.at(i).vertex(j);
			bool bFound = false;
			for(int nv=0; nv<VertexList.size(); nv++)
			{
				if(V.isSame(VertexList.at(nv)))
				{
					bFound = true;
					break;
				}
			}
			if(!bFound)
			{
				VertexList.append(V);
				maxlength = std::max(maxlength, V.norm());
			}
		}
	}
	return maxlength;
}


void RuppertTestDlg::onIndexes()
{
    m_pRuppView->showTriangleIndexes(m_pchTriIndexes->isChecked());
    m_pRuppView->showPSLGIndexes(m_pchPSLGIndexes->isChecked());
	m_pRuppView->update();
}


void RuppertTestDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("RuppertTest");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();

        s_bTrianglePSLG = settings.value("TrianglePSLG",               s_bTrianglePSLG).toBool();
        s_bIncludeFoilPSLG = settings.value("IncludeFoilPSLG",         s_bIncludeFoilPSLG).toBool();
        s_bIncludeFreeEdgePSLG = settings.value("IncludeFreeEdgePSLG", s_bIncludeFreeEdgePSLG).toBool();

        s_MinEdgeLength = settings.value("MinEdgeLength",        s_MinEdgeLength).toDouble();
        s_MaxEdgeLength = settings.value("MaxEdgeLength",        s_MaxEdgeLength).toDouble();

        s_MaxMeshIters = settings.value("MaxMeshIters",   s_MaxMeshIters).toInt();
        s_MaxPanelCount = settings.value("MaxPanelCount", s_MaxPanelCount).toInt();

        s_Nx       = settings.value("Nx",          s_Nx).toInt();
        s_Ny       = settings.value("Ny",          s_Ny).toInt();
        s_ySide    = settings.value("XSideLength", s_ySide).toDouble();
        s_yMid     = settings.value("XMidLength",  s_yMid).toDouble();
        s_xLength  = settings.value("YSideLength", s_xLength).toDouble();
        s_FoilXPos = settings.value("FoilXPos",    s_FoilXPos).toDouble();
        s_FoilYPos = settings.value("FoilYPos",    s_FoilYPos).toDouble();
    }
    settings.endGroup();
}


void RuppertTestDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("RuppertTest");
    {
        settings.setValue("WindowGeometry", s_Geometry);

        settings.setValue("TrianglePSLG",        s_bTrianglePSLG);
        settings.setValue("IncludeFoilPSLG",     s_bIncludeFoilPSLG);
        settings.setValue("IncludeFreeEdgePSLG", s_bIncludeFreeEdgePSLG);

        settings.setValue("MaxMeshIters",      s_MaxMeshIters);
        settings.setValue("MaxPanelCount",     s_MaxPanelCount);

        settings.setValue("MinEdgeLength",     s_MinEdgeLength);
        settings.setValue("MaxEdgeLength",     s_MaxEdgeLength);

        settings.setValue("Nx",          s_Nx);
        settings.setValue("Ny",          s_Ny);
        settings.setValue("XSideLength", s_ySide);
        settings.setValue("XMidLength",  s_yMid);
        settings.setValue("YSideLength", s_xLength);
        settings.setValue("FoilXPos",    s_FoilXPos);
        settings.setValue("FoilYPos",    s_FoilYPos);
    }
    settings.endGroup();
}


