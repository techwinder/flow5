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


#include <QApplication>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QFileDialog>

#include <QMenu>

#include "fusestldlg.h"

#include <core/saveoptions.h>
#include <api/units.h>
#include <api/geom_global.h>
#include <interfaces/editors/inertia/partinertiadlg.h>
#include <api/fusestl.h>
#include <api/part.h>
#include <interfaces/mesh/mesherwt.h>
#include <interfaces/mesh/afmesher.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>

QByteArray FuseStlDlg::s_Geometry;


FuseStlDlg::FuseStlDlg(QWidget *pParent) : FuseDlg(pParent)
{
    setWindowTitle(tr("STL fuse editor"));

    m_pFuseStl = nullptr;
    setupLayout();
    connectSignals();
}


void FuseStlDlg::initDialog(Fuse *pFuse)
{
    FuseDlg::initDialog(pFuse);

    FuseStl *pFuseStl = dynamic_cast<FuseStl*>(pFuse);

    m_pFuseStl = pFuseStl;
    m_pFuse = pFuseStl;
    m_pglFuseView->setFuse(m_pFuseStl);

    m_pdeMaxEdgeLength->setValue(m_pFuseStl->maxElementSize()*Units::mtoUnit());
    m_pieMaxPanelCount->setValue(AFMesher::maxPanelCount());
    updateProperties();
}


void FuseStlDlg::showEvent(QShowEvent *pEvent)
{
    FuseDlg::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void FuseStlDlg::hideEvent(QHideEvent *pEvent)
{
    FuseDlg::hideEvent(pEvent);
     s_Geometry = saveGeometry();
}


void FuseStlDlg::setupLayout()
{    
    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        QVBoxLayout *pLeftLayout = new QVBoxLayout;
        {
            QTabWidget *pDefinitionTabWt = new QTabWidget;
            {
                QFrame *pMeshTab = new QFrame;
                {
                    QVBoxLayout *pMeshLayout = new QVBoxLayout;
                    {
                        QGroupBox *pSplitTriangleBox = new QGroupBox(tr("Split mesh panels"));
                        {
                            QGridLayout *pSplitLayout = new QGridLayout;
                            {
                                QString tip = tr("<p>This action splits all triangular panels with any edge length "
                                                 "greater than the specified value at the midpoints of all 3 edges.<br>"
                                                 "This generate 4 new triangular sub-panels which are added to the mesh"
                                                 "in replacement of the original triangular panel.<br>"
                                                 "Make sure to select a not-too-small max. edge length to avoid excessive "
                                                 "mesh sizes.<br>"
                                                 "This has no effect on the tessellation.</p>");
                                m_ppbSplitTriangles = new QPushButton(tr("Split oversized panels"));
                                m_ppbSplitTriangles->setToolTip(tip);

                                tip = tr("<p>This action restores a mesh by converting the base STL triangulation "
                                         "to triangular panels</p>");
                                m_ppbRestoreDefaultMesh = new QPushButton(tr("Restore default mesh"));
                                m_ppbRestoreDefaultMesh->setToolTip(tip);
                                QLabel *pLabMaxEdgeLength = new QLabel(tr("Max. edge length"));
                                pLabMaxEdgeLength->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                                QLabel *pLabUnit = new QLabel(Units::lengthUnitQLabel());
                                m_pdeMaxEdgeLength = new FloatEdit;
                                m_pdeMaxEdgeLength->setToolTip(tip);

                                QLabel *pLabMaxPanelCount = new QLabel(tr("Max. panel count"));
                                pLabMaxPanelCount->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                                tip = tr("<p>The split operation will stop if the total number of panels exceeds this value</p>");
                                m_pieMaxPanelCount = new IntEdit;
                                m_pieMaxPanelCount->setToolTip(tip);

                                pSplitLayout->addWidget(pLabMaxEdgeLength,1,1);
                                pSplitLayout->addWidget(m_pdeMaxEdgeLength,1,2);
                                pSplitLayout->addWidget(pLabUnit,1,3);
                                pSplitLayout->addWidget(pLabMaxPanelCount,2,1);
                                pSplitLayout->addWidget(m_pieMaxPanelCount,2,2);

                                pSplitLayout->addWidget(m_ppbSplitTriangles,3,1,1,3);
                                pSplitLayout->addWidget(m_ppbRestoreDefaultMesh,4,1,1,3);
                            }
                            pSplitTriangleBox->setLayout(pSplitLayout);
                        }

                        m_pptoOutput = new PlainTextOutput;

                        QHBoxLayout *pOtherActionsLayout = new QHBoxLayout;
                        {
                            QMenu *pBodyMenu = new QMenu(tr("Actions..."), this);
                            {
                                pBodyMenu->addAction(m_pTranslate);
                                pBodyMenu->addAction(m_pScale);
                                pBodyMenu->addAction(m_pRotate);
                                pBodyMenu->addSeparator();
                                pBodyMenu->addAction(m_pFuseInertia);
                            }
                            QPushButton *pMenuButton = new QPushButton(tr("Actions"));
                            pMenuButton->setMenu(pBodyMenu);

                            pOtherActionsLayout->addWidget(pMenuButton);
                            pOtherActionsLayout->addStretch();
                        }

                        pMeshLayout->addWidget(pSplitTriangleBox);
                        pMeshLayout->setStretchFactor(pSplitTriangleBox,1);
                    }
                    pMeshTab->setLayout(pMeshLayout);
                }
                pDefinitionTabWt->addTab(m_pMetaFrame, tr("Meta"));
                pDefinitionTabWt->addTab(pMeshTab, tr("Mesh"));
            }

            m_pptoOutput = new PlainTextOutput;

            QPushButton *pMenuButton = new QPushButton(tr("Actions"));
            {
                QMenu *pBodyMenu = new QMenu(tr("Actions..."),this);
                {
                    pBodyMenu->addAction(m_pTranslate);
                    pBodyMenu->addAction(m_pScale);
                    pBodyMenu->addAction(m_pRotate);
                    pBodyMenu->addSeparator();
                    pBodyMenu->addAction(m_pFuseInertia);
                }
                pMenuButton->setMenu(pBodyMenu);
                pMenuButton->setDefault(false);
                pMenuButton->setAutoDefault(false);
            }
            m_pButtonBox->addButton(pMenuButton, QDialogButtonBox::ActionRole);

            pLeftLayout->addWidget(pDefinitionTabWt);
            pLeftLayout->addWidget(m_pptoOutput);
            pLeftLayout->addWidget(m_pButtonBox);
            pLeftLayout->setStretchFactor(pDefinitionTabWt, 1);
            pLeftLayout->setStretchFactor(m_pptoOutput,     5);
            pLeftLayout->setStretchFactor(m_pButtonBox,     1);
        }
        QVBoxLayout *pFuseViewLayout = new QVBoxLayout;
        {
            pFuseViewLayout->addWidget(m_pglFuseView);
            pFuseViewLayout->addWidget(m_pglControls);
        }

        pMainLayout->addLayout(pLeftLayout);
        pMainLayout->addLayout(pFuseViewLayout);
        pMainLayout->setStretchFactor(pLeftLayout,1);
        pMainLayout->setStretchFactor(pFuseViewLayout,5);
    }

    setLayout(pMainLayout);
}


void FuseStlDlg::connectSignals()
{
    connectBaseSignals();

    connect(m_ppbSplitTriangles,     SIGNAL(clicked(bool)), SLOT(onSplitMeshPanels()));
    connect(m_ppbRestoreDefaultMesh, SIGNAL(clicked(bool)), SLOT(onRestoreDefaultMesh()));
}


void FuseStlDlg::updateOutput(QString const &msg)
{
    m_pptoOutput->onAppendQText(msg);
}


void FuseStlDlg::enableControls(bool )
{
}


void FuseStlDlg::onSplitMeshPanels()
{
    int maxpanels = m_pieMaxPanelCount->value();
    double maxsize = m_pdeMaxEdgeLength->value()/Units::mtoUnit();

    AFMesher::setMaxPanelCount(maxpanels);
    m_pFuseStl->setMaxElementSize(maxsize);

    TriMesh &mesh = m_pFuseStl->triMesh();
    int firstindex = mesh.panel(0).index();
    int nPanels0 = mesh.nPanels();
    double maxedgelength = 0;
    int nSplits = 0;
    std::vector<Panel3> splitpanels;
    for(int i3=mesh.nPanels()-1; i3>=0; i3--)
    {
        Panel3 &p3 = mesh.panel(i3);
        if(p3.maxEdgeLength()>maxsize)
        {
            p3.splitAtEdgeMidPoints(splitpanels);
            mesh.removePanelAt(i3);
            mesh.addPanels(splitpanels);

            maxedgelength = std::max(maxedgelength, p3.maxEdgeLength());
            nSplits++;

            if(mesh.nPanels()>maxpanels) break;
        }
    }
    for(int i3=0; i3<mesh.nPanels(); i3++)
    {
        mesh.panel(i3).setIndex(i3);
    }

    QString log, strange;
    if(mesh.nPanels()>maxpanels)
    {
        strange = QString::asprintf("Max. panel count exceeded... aborting\n");
        updateOutput(strange);
    }

    strange = QString::asprintf("Split %d triangles out of %d\n", nSplits, nPanels0);
    log += strange;
    strange = QString::asprintf("New panel count      = %d\n", mesh.nPanels());
    log += strange;
    strange = QString::asprintf("New max. edge length = %g", maxedgelength*Units::mtoUnit());
    strange += Units::lengthUnitQLabel() + "\n";
    log += strange;
    updateOutput(log);

    strange.clear();

    std::string str;
    mesh.makeNodeArrayFromPanels(firstindex, str, "");

    strange = QString::fromStdString(str);

    updateOutput(strange+"\n");
    m_pglFuseView->resetPanels();
    m_pglFuseView->update();
    updateProperties();
}


void FuseStlDlg::onRestoreDefaultMesh()
{
    std::string strange;
    m_pFuseStl->makeDefaultTriMesh(strange, "");
    updateOutput(QString::fromStdString(strange)+"\n");
    m_pglFuseView->resetPanels();
    m_pglFuseView->update();
    updateProperties();
}


void FuseStlDlg::updateProperties(bool)
{
    if(!m_pFuseStl) return;

    std::string str;
    m_pFuseStl->computeSurfaceProperties(str, "");

    QString log, strong;
    strong = QString::asprintf("Triangles       = %6d", m_pFuseStl->nPanel3());
    log = QString::fromStdString(str) + "\n" + strong;
    m_pglFuseView->setBotLeftOutput(log);
}


void FuseStlDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("StlFuseDlg");
    {
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void FuseStlDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("StlFuseDlg");
    {
        settings.setValue("WindowGeom", s_Geometry);
    }
    settings.endGroup();
}
