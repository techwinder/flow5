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
#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include <TopoDS_Shape.hxx>

#include "planedlg.h"


#include <interfaces/mesh/panelcheckdlg.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dxflview.h>
#include <api/units.h>
#include <core/xflcore.h>
#include <api/plane.h>
#include <interfaces/widgets/customdlg/doublevaluedlg.h>
#include <interfaces/widgets/customdlg/intvaluedlg.h>

#include <interfaces/widgets/customwts/formattextoutput.h>



Quaternion PlaneDlg::s_ab_quat(-0.212012, 0.148453, -0.554032, -0.79124);


bool PlaneDlg::s_bOutline    = true;
bool PlaneDlg::s_bSurfaces   = true;
bool PlaneDlg::s_bVLMPanels  = false;
bool PlaneDlg::s_bAxes       = true;
bool PlaneDlg::s_bShowMasses = false;
bool PlaneDlg::s_bFoilNames  = false;

double PlaneDlg::s_TEMaxAngle = 15.0; //degrees


PlaneDlg::PlaneDlg(QWidget *pParent) : XflDialog(pParent)
{
    setWindowFlag(Qt::WindowMinMaxButtonsHint);

    m_pPlane = nullptr;
    m_bAcceptName = true;
    m_bDescriptionChanged = false;
    m_bChanged = false;
    makeCommonControls();
}


void PlaneDlg::makeCommonControls()
{
    m_pleName = new QLineEdit;
    m_pleName->setClearButtonEnabled(true);
    m_pleDescription = new QPlainTextEdit;
    m_pleDescription->setToolTip(tr("Enter here a short description for the plane"));

    m_ppto = new FormatTextOutput;

    m_pCheckMesh            = new QAction(tr("Check mesh"), this);
    m_pCheckFreeEdges       = new QAction(tr("Check free edges"), this);
    m_pCheckFreeEdges->setShortcut(QKeySequence(Qt::ALT | Qt::Key_G));
    m_pConnectPanels        = new QAction(tr("Connect panels"), this);
    m_pConnectPanels->setShortcut(QKeySequence(Qt::ALT | Qt::Key_C));
    m_pClearHighlighted     = new QAction(tr("Clear highlighted"), this);
    m_pClearHighlighted->setShortcut(QKeySequence(Qt::ALT | Qt::Key_L));
    m_pMergeFuseToWingNodes = new QAction(tr("Merge fuselage nodes with wing nodes"));
    m_pCenterOnPanel        = new QAction(tr("Center view on panel"), this);

    m_pButtonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard);
    {
        m_pButtonBox->setToolTip(tr("Ctrl+S to save and close\n"
                                 "Ctrl+W to edit the main wing\n"
                                 "Ctrl+E to edit the elevator\n"
                                 "Ctrl+F to edit the fin\n"
                                 "Ctrl+B to edit the first body/fuselage"));
        m_ppbActions = new QPushButton(tr("Actions"));
        {
            QMenu *pCheckMeshMenu = new QMenu(tr("Actions"));
            {
                m_pClearOutput   = new QAction(tr("Clear output"), this);

                m_pPlaneInertia     = new QAction(tr("Inertia"), this);
                m_pPlaneInertia->setShortcut(Qt::Key_F12);
                m_pFlipNormals      = new QAction(tr("Flip normals"), this);

                pCheckMeshMenu->addAction(m_pClearOutput);
                pCheckMeshMenu->addSeparator();
                pCheckMeshMenu->addAction(m_pPlaneInertia);
                pCheckMeshMenu->addSeparator();
                pCheckMeshMenu->addAction(m_pFlipNormals);
            }
            m_ppbActions->setMenu(pCheckMeshMenu);
        }
        m_ppbSaveAsNew = new QPushButton(tr("Save as"));
        m_pButtonBox->addButton(m_ppbActions,   QDialogButtonBox::ActionRole);
        m_pButtonBox->addButton(m_ppbSaveAsNew, QDialogButtonBox::ActionRole);
    }
}


void PlaneDlg::connectBaseSignals()
{
    connect(m_pleName,               SIGNAL(editingFinished()),      SLOT(onMetaDataChanged()));
    connect(m_pleDescription,        SIGNAL(textChanged()),          SLOT(onMetaDataChanged()));
    connect(m_pFlipNormals,          SIGNAL(triggered()),            SLOT(onFlipNormals()));
    connect(m_pPlaneInertia,         SIGNAL(triggered()),            SLOT(onPlaneInertia()));
    connect(m_pCheckMesh,            SIGNAL(triggered()),            SLOT(onCheckMesh()));
    connect(m_pCenterOnPanel,        SIGNAL(triggered()),            SLOT(onCenterViewOnPanel()));
    connect(m_pClearHighlighted,     SIGNAL(triggered()),            SLOT(onClearHighlighted()));
    connect(m_pMergeFuseToWingNodes,      SIGNAL(triggered()),       SLOT(onMergeFuseToWingNodes()));
    connect(m_pConnectPanels,        SIGNAL(triggered()),            SLOT(onConnectPanels()));
    connect(m_pCheckFreeEdges,       SIGNAL(triggered()),            SLOT(onCheckFreeEdges()));
    connect(m_pglPlaneView,          SIGNAL(pickedNodeIndex(int)),   SLOT(onPickedNode(int)));
}


void PlaneDlg::initDialog(Plane *pPlane, bool )
{
    m_pPlane = pPlane;

    m_pleName->setText(QString::fromStdString(pPlane->name()));
    m_pleDescription->setPlainText(QString::fromStdString(pPlane->description()));
    if(!m_bAcceptName) m_pleName->setEnabled(false);

    m_bChanged = m_bDescriptionChanged = false;
}


void PlaneDlg::showEvent(QShowEvent *pEvent)
{
    m_pglPlaneView->setFlags(s_bOutline, s_bSurfaces, s_bVLMPanels, s_bAxes, s_bShowMasses, s_bFoilNames, false, false, false);
    m_pglControls->setControls();
    m_pglPlaneView->restoreViewPoint(s_ab_quat);

    QDialog::showEvent(pEvent);
}


void PlaneDlg::hideEvent(QHideEvent *)
{
    s_bOutline    = m_pglPlaneView->bOutline();
    s_bSurfaces   = m_pglPlaneView->bSurfaces();
    s_bVLMPanels  = m_pglPlaneView->bVLMPanels();
    s_bAxes       = m_pglPlaneView->bAxes();
    s_bShowMasses = m_pglPlaneView->bMasses();
    s_bFoilNames  = m_pglPlaneView->bFoilNames();
    m_pglPlaneView->saveViewPoint(s_ab_quat);
}


void PlaneDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)       onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)    reject();
    else if (m_ppbSaveAsNew==pButton)                                       onOK(10);
}


void PlaneDlg::reject()
{
    if(m_bChanged && xfl::bConfirmDiscard())
    {
        QString strong = tr("Discard the changes?");
        int Ans = QMessageBox::question(this, tr("Question"), strong,
                                        QMessageBox::Yes | QMessageBox::Cancel);
        if (QMessageBox::Yes == Ans)
        {

            done(QDialog::Rejected);
            return;
        }
        else return;
    }

    done(QDialog::Rejected);
}


void PlaneDlg::onMetaDataChanged()
{
    m_pPlane->setName(m_pleName->text().toStdString());
    m_pPlane->setDescription(m_pleDescription->toPlainText().toStdString());
    m_bDescriptionChanged = true;
}


void PlaneDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("PlaneDlg");
    {
        s_bOutline    = settings.value("Outline",   true).toBool();
        s_bSurfaces   = settings.value("Surfaces",  true).toBool();
        s_bVLMPanels  = settings.value("VLMPanels", false).toBool();
        s_bShowMasses = settings.value("Masses",    false).toBool();
        s_bFoilNames  = settings.value("FoilNames", false).toBool();

        s_TEMaxAngle  = settings.value("TEMaxAngle", s_TEMaxAngle).toDouble();
    }
    settings.endGroup();
}


void PlaneDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("PlaneDlg");
    {
        settings.setValue("Outline",   s_bOutline);
        settings.setValue("Surfaces",  s_bSurfaces);
        settings.setValue("VLMPanels", s_bVLMPanels);
        settings.setValue("Masses",    s_bShowMasses);
        settings.setValue("FoilNames", s_bFoilNames);

        settings.setValue("TEMaxAngle", s_TEMaxAngle);
    }
    settings.endGroup();
}


void PlaneDlg::outputPanelProperties(int panelindex)
{
    std::string strange;

    // check index validity
    if(panelindex<0 || panelindex>=m_pPlane->nPanel3()) return;

    bool bLong = false;
#ifdef QT_DEBUG
    bLong = true;
#endif
    strange = m_pPlane->panel3At(panelindex).properties(bLong);
    strange +="\n\n";

    updateStdOutput(strange);
}


void PlaneDlg::onNodeDistance()
{
    if(!m_pglControls->getDistance()) m_pglPlaneView->clearMeasure();

    m_pglPlaneView->setPicking(m_pglControls->getDistance() ? xfl::MESHNODE : xfl::NOPICK);
    m_pglPlaneView->setSurfacePick(xfl::NOSURFACE);
    m_pglPlaneView->update();
}


void PlaneDlg::onConnectPanels()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    updateOutput("Connecting panels...");

    if(!m_pPlane->connectTriMesh(true, m_pPlane->isSTLType(), xfl::isMultiThreaded()))
    {
        updateOutput(" ... error connecting panels\n\n");
    }
    else
    {
        QString log(" ... done\n\n");
        updateOutput(log);
    }

    m_bChanged = true;
    QApplication::restoreOverrideCursor();
}


void PlaneDlg::onCheckFreeEdges()
{
    std::vector<Segment3d> freeedges;
    m_pPlane->refTriMesh().getFreeEdges(freeedges);
    QVector<Segment3d> qVec = QVector<Segment3d>(freeedges.begin(), freeedges.end());

    m_pglPlaneView->setSegments(qVec);

    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();
    QString strange;
    strange = QString::asprintf("Found %d free edges\n\n", int(freeedges.size()));
    updateOutput(strange);
}


void PlaneDlg::onClearHighlighted()
{
    m_pglPlaneView->clearHighlightList();
    m_pglPlaneView->clearSegments();

    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();
}


void PlaneDlg::onMergeFuseToWingNodes()
{
    if(m_pPlane->nPanel3()<=0)
    {
        updateOutput("No panels detected\n");
        return;
    }
    Panel3 &p3t = m_pPlane->refTriMesh().panel(0);
    if(p3t.neighbourCount()==0)
    {
        updateOutput("Connect the panels before attempting to merge nodes\n");
        return;
    }

    QStringList labels("Merge nodes closer than:");
    QStringList rightlabels(Units::lengthUnitQLabel());
    QVector<double> vals({XflMesh::nodeMergeDistance()*Units::mtoUnit()});
    DoubleValueDlg dlg(this, vals, labels, rightlabels);
    if(dlg.exec()!=QDialog::Accepted) return;
    XflMesh::setNodeMergeDistance(dlg.value(0)/Units::mtoUnit());

    QString strange;
    strange = QString::asprintf("Merging fuselage nodes to wing nodes within precision %g ", XflMesh::nodeMergeDistance()*Units::mtoUnit());
    strange += Units::lengthUnitQLabel() + "\n";
    updateOutput(strange);

    std::string logmsg, prefix("   ");
    m_pPlane->refTriMesh().mergeFuseToWingNodes(XflMesh::nodeMergeDistance(), logmsg, prefix);
    updateStdOutput(logmsg);
    m_pglPlaneView->resetgl3dMesh();
    m_pglPlaneView->update();
    m_bChanged = true;
}


void PlaneDlg::onCheckMesh()
{
    std::string log = "Checking panels\n";
    updateStdOutput(log);

    log.clear();
    PanelCheckDlg dlg(false);
    int res = dlg.exec();
    bool bCheck = dlg.checkSkinny() || dlg.checkMinArea() || dlg.checkMinAngles() || dlg.checkMinQuadWarp() || dlg.checkMinSize();
    if(res!=QDialog::Accepted)
    {
        return;
    }

    m_pglPlaneView->resetgl3dMesh();

    m_pglPlaneView->clearHighlightList();
    QVector<int> highlist = dlg.panelIndexes();
    m_pglPlaneView->appendHighlightList(highlist);
    if(highlist.isEmpty())
    {
        updateOutput("No panels found which meet the criteria\n\n");
    }
    else
    {
        for(int i3=0; i3<highlist.size(); i3++)
        {
            outputPanelProperties(highlist.at(i3));
        }
    }

    if(!bCheck)
    {
        m_pglPlaneView->update();
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // check Triangles
    std::vector<int> skinnylist, anglelist, arealist, sizelist;

    m_pPlane->refTriMesh().checkPanels(log, dlg.checkSkinny(), dlg.checkMinAngles(), dlg.checkMinArea(), dlg.checkMinSize(),
                                       skinnylist, anglelist, arealist, sizelist,
                                       PanelCheckDlg::qualityFactor(), PanelCheckDlg::minAngle(), PanelCheckDlg::minArea(), PanelCheckDlg::minSize());

    QVector<int> qVec;
    if(dlg.checkSkinny())    qVec = QVector<int>(skinnylist.begin(), skinnylist.end());
    if(dlg.checkMinAngles()) qVec = QVector<int>(anglelist.begin(), anglelist.end());
    if(dlg.checkMinArea())   qVec = QVector<int>(arealist.begin(), arealist.end());
    if(dlg.checkMinSize())   qVec = QVector<int>(sizelist.begin(), sizelist.end());
    m_pglPlaneView->appendHighlightList(qVec);

    updateStdOutput(log + "\n");

    QApplication::restoreOverrideCursor();
}


void PlaneDlg::onCenterViewOnPanel()
{
    if(!m_pPlane) return;

    IntValueDlg dlg(this);
    dlg.setValue(-1);
    dlg.setLeftLabel("Panel index:");
    if(dlg.exec()==QDialog::Accepted)
    {
        int ip = dlg.value();
        if(ip>=0 && ip<m_pPlane->refTriMesh().nPanels())
        {
            m_pglPlaneView->centerViewOn(m_pPlane->refTriMesh().panelAt(ip).CoG());
            updateStdOutput(m_pPlane->refTriMesh().panelAt(ip).properties(true)+"\n\n");
        }

        QVector<int> highlist = {ip};
        m_pglPlaneView->setHighlightList(highlist);
        m_pglPlaneView->resetgl3dMesh();
    }
    m_pglPlaneView->update();
}


void PlaneDlg::onUpdatePlaneProps()
{
    if(!m_pPlane) return;
    m_pglPlaneView->setBotLeftOutput(m_pPlane->planeData(true));
}


void PlaneDlg::onPickedNode(int iNode)
{
    Node const &nsrc  = m_pPlane->node(iNode);
    QString strange;
    strange  = QString::asprintf("Node %5d: (%9g, %9g, %9g) ", iNode, nsrc.x*Units::mtoUnit(), nsrc.y*Units::mtoUnit(), nsrc.z*Units::mtoUnit());
    strange += Units::lengthUnitQLabel() + "\n";
    updateOutput(strange);
}


void PlaneDlg::updateStdOutput(std::string const &msg)
{
    m_ppto->onAppendStdText(msg);
}


void PlaneDlg::updateOutput(QString const &msg)
{
    m_ppto->onAppendQText(msg);

}
