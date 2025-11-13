/****************************************************************************

    flow5
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
#include <QVBoxLayout>
#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QClipboard>
#include <QFileDialog>

#include "xsail.h"



#include <xfl/controls/crossflowctrls.h>
#include <xfl/controls/flowctrls.h>
#include <xfl/controls/opp3dscalesctrls.h>
#include <xfl/controls/streamlinesctrls.h>
#include <xfl/controls/w3dprefs.h>
#include <xfl/editors/analysis3ddef/btpolarautonamedlg.h>
#include <xfl/editors/analysis3ddef/btpolardlg.h>
#include <xfl/editors/boatedit/boatdlg.h>
#include <xfl/editors/boatedit/nurbssaildlg.h>
#include <xfl/editors/boatedit/occsaildlg.h>
#include <xfl/editors/boatedit/sailscaledlg.h>
#include <xfl/editors/boatedit/splinesaildlg.h>
#include <xfl/editors/boatedit/stlsaildlg.h>
#include <xfl/editors/boatedit/wingsaildlg.h>
#include <xfl/editors/editplrdlg.h>
#include <xfl/editors/fuseedit/bodyscaledlg.h>
#include <xfl/editors/fuseedit/fuseoccdlg.h>
#include <xfl/editors/fuseedit/fusestldlg.h>
#include <xfl/editors/fuseedit/xflfuseedit/fusexfldefdlg.h>
#include <xfl/editors/translatedlg.h>
#include <xfl/globals/mainframe.h>
#include <xfl/graphs/cpgraphctrls.h>
#include <xfl/graphs/cpviewwt.h>
#include <xfl/graphs/graphtilectrls.h>
#include <xfl/test/panelanalysistest.h>
#include <xfl/xplane/analysis/analysis3dsettings.h>
#include <xfl/xsail/analysis/boatanalysisdlg.h>
#include <xfl/xsail/controls/boattreeview.h>
#include <xfl/xsail/controls/gl3dxsailctrls.h>
#include <xfl/xsail/controls/xsailanalysisctrls.h>
#include <xfl/xsail/controls/xsaildisplayctrls.h>
#include <xfl/xsail/menus/xsailactions.h>
#include <xfl/xsail/menus/xsailmenus.h>
#include <xfl/xsail/view/gl3dxsailview.h>
#include <xfl/xsail/view/xsaillegendwt.h>
#include <xflcore/displayoptions.h>
#include <xflcore/saveoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflgraph/controls/graphtilevariableset.h>
#include <xflgraph/graph/curvemodel.h>
#include <xflgraph/graph/graph.h>
#include <xflobjects/analysis3d/boattask.h>
#include <xflobjects/analysis3d/panelanalysis.h>
#include <xflobjects/exchange/cadexportdlg.h>
#include <xflobjects/exchange/stlwriterdlg.h>
#include <xflobjects/objects3d/analysis/analysisseldlg.h>
#include <xflobjects/objects3d/analysis/boatopp.h>
#include <xflobjects/objects3d/analysis/boatpolar.h>
#include <xflobjects/objects3d/fuse/fuseocc.h>
#include <xflobjects/objects3d/fuse/fusestl.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflobjects/sailobjects/boat.h>
#include <xflobjects/sailobjects/sailobjects.h>
#include <xflobjects/sailobjects/sails/nurbssail.h>
#include <xflobjects/sailobjects/sails/occsail.h>
#include <xflobjects/sailobjects/sails/splinesail.h>
#include <xflobjects/sailobjects/sails/stlsail.h>
#include <xflobjects/sailobjects/sails/wingsail.h>
#include <xflpanels/mesh/interfaces/panelcheckdlg.h>
#include <xflscript/xml/xsail/xmlboatreader.h>
#include <xflscript/xml/xsail/xmlboatwriter.h>
#include <xflscript/xml/xsail/xmlbtpolarreader.h>
#include <xflscript/xml/xsail/xmlbtpolarwriter.h>
#include <xflscript/xml/xsail/xmlsailwriter.h>
#include <xflwidgets/customdlg/doublevaluedlg.h>
#include <xflwidgets/customdlg/objectpropsdlg.h>
#include <xflwidgets/customdlg/renamedlg.h>
#include <xflwidgets/line/linemenu.h>


XSail::XSail(MainFrame *pMainFrame) : QObject()
{
    s_pMainFrame = pMainFrame;
    gl3dXSailView::setXSail(this);
    BoatTreeView::setXSail(this);
    GraphTiles::setMainFrame(s_pMainFrame);
    GraphTiles::setXSail(this);
    XSailLegendWt::setXSail(this);
    BoatAnalysisDlg::setXSail(this);
    gl3dXSailCtrls::setXSail(this);
    CrossFlowCtrls::setXSail(this);

    m_pCurBoat    = nullptr;
    m_pCurBtPolar = nullptr;
    m_pCurBtOpp   = nullptr;

    m_eView = W3DVIEW;
    m_bResetCurves = true;

    m_LastCtrl = 0.0;

    QString strSpeed   = Units::speedUnitLabel();
    QString strForce   = Units::forceUnitLabel();
    QString strMoment  = Units::momentUnitLabel();
    BoatPolar::setVariableNames(strSpeed, strForce, strMoment);

    m_pBtAnalysisDlg = new BoatAnalysisDlg;

    makeControlWts();

    makePlrgraphs();

    connectSignals();

    m_pBoatTreeView = new BoatTreeView;
}


XSail::~XSail()
{
    for(int ig=m_PlrGraph.count()-1; ig>=0; ig--)
    {
        delete m_PlrGraph.at(ig);
        m_PlrGraph.removeAt(ig);
    }
    for(int ig=m_PlrCurveModel.count()-1; ig>=0; ig--)
    {
        delete m_PlrCurveModel.at(ig);
        m_PlrCurveModel.removeAt(ig);
    }
}


void XSail::makePlrgraphs()
{
    for(int ig=0; ig<MAXGRAPHS; ig++)
    {
        m_PlrGraph.append(new Graph);
        m_PlrCurveModel.append(new CurveModel);
        m_PlrGraph.last()->setXVariableList(BoatPolar::variableNames());
        m_PlrGraph.last()->setYVariableList(BoatPolar::variableNames());
        m_PlrGraph.last()->setCurveModel(m_PlrCurveModel.last());
        m_PlrGraph.last()->setName(QString::asprintf("Sail polar graph %d", ig+1));
        m_PlrGraph.last()->setGraphType(GRAPH::POLARGRAPH);
        m_PlrGraph.last()->setXMin(0.0);
        m_PlrGraph.last()->setXMax(0.1);
        m_PlrGraph.last()->setYMin(0, -0.1);
        m_PlrGraph.last()->setYMax(0, 0.1);
        m_PlrGraph.last()->setScaleType(GRAPH::RESETTING);
        m_PlrGraph.last()->setBorderColor(QColor(200,200,200));
        m_PlrGraph.last()->setBorder(true);
        m_PlrGraph.last()->setBorderStyle(Line::SOLID);
        m_PlrGraph.last()->setBorderWidth(3);
        m_PlrGraph.last()->setMargins(50);
        m_PlrGraph.last()->enableRightAxis(true);

    }

    m_PlrGraph[0]->setVariables(2,11);
    m_PlrGraph[1]->setVariables(2,10);
    m_PlrGraph[2]->setVariables(2,6);
    m_PlrGraph[3]->setVariables(2,14);
    m_PlrGraph[4]->setVariables(2,15);
}


void XSail::makeControlWts()
{
    m_pgl3dXSailView = new gl3dXSailView;
    m_pgl3dXSailView->setReferenceLength(10.0);
    m_pgl3dXSailView->reset3dScale();


    m_pActions = new XSailActions(this);
    m_pgl3dControls = new gl3dXSailCtrls(m_pgl3dXSailView, s_pMainFrame);
//    m_pgl3dXSailView->setResultControls(m_pgl3dControls, s_pMainFrame->m_pgl3dScales);

    m_pAnalysisCtrls = new XSailAnalysisCtrls(this);
    m_pMenus = new XSailMenus(s_pMainFrame, this);
}


void XSail::connectSignals()
{
    connect(s_pMainFrame->m_pXSailTiles,   SIGNAL(varSetChanged(int)),    SLOT(onVarSetChanged(int)));
    connect(s_pMainFrame->m_pXSailTiles,   SIGNAL(graphChanged(int)),     SLOT(onGraphChanged(int)));


    connect(m_pBtAnalysisDlg, SIGNAL(analysisFinished()), SLOT(onFinishAnalysis()));
    connect(m_pAnalysisCtrls->m_ppbAnalyze, SIGNAL(clicked(bool)), SLOT(onAnalyze()));

    connect(m_pgl3dControls->m_pOpp3dScalesCtrls,    SIGNAL(update3dScales()),      SLOT(onUpdate3dScales()));
    connect(m_pgl3dControls->m_pCrossFlowCtrls,      SIGNAL(updateWake()),          SLOT(onUpdateWake()));
    connect(m_pgl3dControls->m_pStreamLinesCtrls,    SIGNAL(update3dStreamlines()), SLOT(onUpdate3dStreamlines()));
}


void XSail::onGraphChanged(int)
{
    resetCurves();
    updateView();
}


void XSail::onVarSetChanged(int)
{
    if (isPolarView())
    {
        for(int ig=0; ig<m_PlrGraph.size(); ig++)
        {
            m_PlrGraph.at(ig)->setAuto(true);
            m_PlrGraph.at(ig)->resetLimits();
            m_PlrGraph.at(ig)->invalidate();
        }
    }
    resetCurves();
    updateView();
}


void XSail::onUpdate3dScales()
{
    gl3dXSailView::s_bResetglPanelCp        = true;
    gl3dXSailView::s_bResetglPanelGamma     = true;
    gl3dXSailView::s_bResetglPanelCp        = true;
    gl3dXSailView::s_bResetglPanelForce     = true;
    gl3dXSailView::s_bResetglLift           = true;
    gl3dXSailView::s_bResetglMoments        = true;
    gl3dXSailView::s_bResetglDrag           = true;
    gl3dXSailView::s_bResetglVorticity      = true;
    gl3dXSailView::s_bResetglGridVelocities = true;
    m_pgl3dXSailView->m_LegendOverlay.makeLegend();
    m_pgl3dXSailView->update();
}


void XSail::onUpdateWake()
{
    gl3dXSailView::s_bResetglMesh           = true;
    gl3dXSailView::s_bResetglVorticity      = true;
    gl3dXSailView::s_bResetglGridVelocities = true;
    m_pgl3dXSailView->m_LegendOverlay.makeLegend();
    m_pgl3dXSailView->update();
}


void XSail::onUpdate3dStreamlines()
{
    gl3dXSailView::s_bResetglStream = true;
    m_pgl3dXSailView->update();
}


void XSail::setControls()
{
    if(isPolarView())
    {
        s_pMainFrame->m_pdwXSail3dCtrls->setVisible(false); // hide first
        if(MainFrame::xflApp()==xfl::XSAIL)
            s_pMainFrame->m_pdwGraphControls->setVisible(true); // show only after
        s_pMainFrame->m_pXSailTiles->updateControls();
        m_pActions->m_pPolarView->setChecked(true);
        m_pActions->m_p3dView->setChecked(false);
    }
    else
    {
        s_pMainFrame->m_pdwGraphControls->setVisible(false);// hide first
        if(MainFrame::xflApp()==xfl::XSAIL)
            s_pMainFrame->m_pdwXSail3dCtrls->setVisible(true);// show only after

        m_pActions->m_pPolarView->setChecked(false);
        m_pActions->m_p3dView->setChecked(true);
    }

    m_pActions->m_pConvertMainSailToNURBS->setEnabled(m_pCurBoat && m_pCurBoat->mainSail() && m_pCurBoat->mainSail()->isSplineSail());
    m_pActions->m_pConvertJibToNURBS->setEnabled(m_pCurBoat && m_pCurBoat->jib() && m_pCurBoat->jib()->isSplineSail());

    m_pBoatTreeView->setCurveParams();
    m_pAnalysisCtrls->onSetControls();

    m_pgl3dControls->setControls();

    checkActions();
}


void XSail::checkActions()
{
    m_pActions->checkActions();
    m_pActions->m_pExportMainSailToStep->setEnabled(m_pCurBoat && m_pCurBoat->hasSail() && m_pCurBoat->mainSail()->isNURBSSail());
    m_pActions->m_pExportJibToStep->setEnabled(m_pCurBoat && m_pCurBoat->hasJib() && m_pCurBoat->jib()->isNURBSSail());
    m_pMenus->checkMenus();
}


void XSail::updateObjectView()
{
    m_pBoatTreeView->setObjectProperties();
    m_pBoatTreeView->fillModelView();
}


void XSail::loadSettings(QSettings &settings)
{
    settings.beginGroup("XSail");
    {
        int k = settings.value("iView").toInt();
        switch(k)
        {
            default:
            case 0: m_eView = W3DVIEW;      break;
            case 1: m_eView = POLARVIEW;    break;
        }

        BoatDlg::loadSettings(settings);
        SailDlg::loadSettings(settings);
        BoatTreeView::loadSettings(settings);
        BoatAnalysisDlg::loadSettings(settings);
        BtPolarAutoNameDlg::loadSettings(settings);
        XSailDisplayCtrls::loadSettings(settings);

        m_pgl3dXSailView->loadXSailSettings(settings);
    }
    settings.endGroup();

    for(int ig=0; ig<m_PlrGraph.size(); ig++) m_PlrGraph[ig]->loadSettings(settings);
    for(int ig=0; ig<m_PlrGraph.size(); ig++)
    {
        m_PlrGraph.at(ig)->setXVariableList(BoatPolar::variableNames());
        m_PlrGraph.at(ig)->setYVariableList(BoatPolar::variableNames());
    }

    s_pMainFrame->m_pXSailTiles->loadSettings(settings, "XSailTiles");
    if(s_pMainFrame->m_pXSailTiles->variableSetCount()>0)
    {
        GraphTileVariableSet const & variableset = s_pMainFrame->m_pXSailTiles->variableSet(0);
        for(int ig=0; ig<m_PlrGraph.size(); ig++)
        {
            m_PlrGraph.at(ig)->setVariables(variableset.XVar(ig), variableset.YVar(ig));
        }
    }

    m_pgl3dControls->initWidget();

    m_pBoatTreeView->setPropertiesFont(DisplayOptions::tableFont());
    m_pBoatTreeView->setTreeFontStruct(DisplayOptions::treeFontStruct());
}


void XSail::saveSettings(QSettings &settings)
{
    settings.beginGroup("XSail");
    {
        switch(m_eView)
        {
            case W3DVIEW:               settings.setValue("iView", 0);    break;
            case POLARVIEW:             settings.setValue("iView", 1);    break;
        }

        BoatTreeView::saveSettings(settings);
        BoatDlg::saveSettings(settings);
        SailDlg::saveSettings(settings);
        BoatAnalysisDlg::saveSettings(settings);
        BtPolarAutoNameDlg::saveSettings(settings);
        XSailDisplayCtrls::saveSettings(settings);
        m_pgl3dXSailView->saveXSailSettings(settings);
    }
    settings.endGroup();
    for(int ig=0; ig<m_PlrGraph.count(); ig++) m_PlrGraph[ig]->saveSettings(settings);

    s_pMainFrame->m_pXSailTiles->saveSettings(settings, "XSailTiles");
}


void XSail::keyPressEvent(QKeyEvent *pEvent)
{
    bool bShift = (pEvent->modifiers() & Qt::ShiftModifier) ? true : false;

    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if (pEvent->modifiers().testFlag(Qt::AltModifier))
            {
                if(bShift) onBtOppProps();
                else       onBtPolarProps();
                break;
            }
            pEvent->accept();
            break;
        }
        case Qt::Key_Escape:
        {
            m_pgl3dControls->m_pXSailDisplayCtrls->stopAnimate();
            break;
        }
#ifdef QT_DEBUG
        case Qt::Key_F1:
        {
            if(!m_pPanelResultTest)
                m_pPanelResultTest = new PanelAnalysisTest;

            m_pPanelResultTest->set3dView(m_pgl3dXSailView);
            m_pPanelResultTest->setAnalysis(m_pCurBoat, m_pCurBtPolar, m_pCurBtOpp);
            m_pPanelResultTest->showLegend(true);
            m_pPanelResultTest->show();
            s_pMainFrame->lower();
            m_pPanelResultTest->raise();
            m_pPanelResultTest->activateWindow();

            break;
        }
#endif
        case Qt::Key_F2:
        {
            onSail();
            pEvent->accept();
            break;
        }
        case Qt::Key_F3:
        {
            onNewBoat();
            pEvent->accept();
            break;
        }
        case Qt::Key_F4:
        {
            on3dView();
            pEvent->accept();
            break;
        }
        case Qt::Key_F6:
        {
            onDefinePolar();
            pEvent->accept();
            break;
        }
        case Qt::Key_F8:
        {
            onPolarView();
            pEvent->accept();
            break;
        }
        default:
            pEvent->ignore();
            break;
    }
}


void XSail::onNewBoat()
{
    Boat *pNewBoat = new Boat;
    BoatDlg dlg(s_pMainFrame);
    dlg.initDialog(pNewBoat, true);

    if(dlg.exec()==QDialog::Rejected)
    {
        delete pNewBoat;
        return;
    }
    SailObjects::appendBoat(pNewBoat);
    setBoat(pNewBoat);
    m_pBoatTreeView->insertBoat(pNewBoat);
    m_pBoatTreeView->selectBoat(pNewBoat);
    if(m_pCurBoat) m_pCurBoat->makeRefTriMesh(true, xfl::isMultiThreaded());
    m_pCurBtPolar = nullptr;
    m_pCurBtOpp=nullptr;

    m_pgl3dXSailView->resetglBoat();
    updateView();

    emit projectModified();
}


void XSail::onEditCurBoat()
{
    m_pgl3dXSailView->hideArcball();
    if(!m_pCurBoat) return;
    Boat *pModBoat = new Boat;
    pModBoat->duplicate(m_pCurBoat);

    BoatDlg dlg(s_pMainFrame);
    dlg.initDialog(pModBoat, false);

    if(dlg.exec()==QDialog::Rejected)
    {
        delete pModBoat;
        return;
    }

    if(dlg.bDescriptionChanged() && !dlg.bChanged())
    {
        m_pCurBoat->setDescription(pModBoat->description());
        for(int is=0; is<m_pCurBoat->nSails(); is++)
        {
            if(is<pModBoat->nSails()) m_pCurBoat->sail(is)->setName(pModBoat->sailAt(is)->name());
        }
        for(int is=0; is<m_pCurBoat->nHulls(); is++)
        {
            if(is<pModBoat->nHulls()) m_pCurBoat->hull(is)->setPartName(pModBoat->hullAt(is)->name());
        }
        m_pBoatTreeView->setObjectProperties();
        m_pgl3dXSailView->resetglBoat();
        updateObjectView();
        return;
    }

    if(!setModBoat(pModBoat))
    {
        delete pModBoat;
        return; // the user has changed his mind
    }

    SailObjects::deleteBoatResults(m_pCurBoat, false);

    m_pCurBoat->makeRefTriMesh(m_pCurBtPolar&&!m_pCurBtPolar->bIgnoreBodyPanels(), xfl::isMultiThreaded());

    m_pBoatTreeView->removeBtOpps(m_pCurBoat);
    m_pCurBtOpp=nullptr;

    m_pgl3dXSailView->resetglBoat();
    m_pgl3dXSailView->resetglMesh();
    m_pgl3dXSailView->resetglBtOpp();
    m_pgl3dXSailView->s_bResetglWake = true;

    resetCurves();
    updateView();

    emit projectModified();
}


void XSail::onSail()
{
    NURBSSail nsail;
    nsail.makeDefaultSail();

    NURBSSailDlg  dlg;
    dlg.initDialog(&nsail);
    dlg.exec();
}


void XSail::onDefinePolar()
{
    if(!m_pCurBoat) return;

    BtPolarDlg PlrDlg(s_pMainFrame);

    PlrDlg.initDialog(m_pCurBoat);

    if(PlrDlg.exec()==QDialog::Accepted)
    {
        //Then create and add a BoatPolar to the array
        BoatPolar *pNewBtPolar = new BoatPolar;

        pNewBtPolar->duplicateSpec(&BtPolarDlg::staticBtPolar());
        pNewBtPolar->setTheStyle(m_pCurBoat->theStyle());
        pNewBtPolar->setVisible(true);

        //Then add WPolar to array
        emit projectModified();

        pNewBtPolar->setBoatName(m_pCurBoat->name());
        pNewBtPolar->setName(PlrDlg.polarName());

        pNewBtPolar = insertNewBtPolar(pNewBtPolar, m_pCurBoat);

        if(pNewBtPolar)
        {
            pNewBtPolar->makeDefaultArrays();
            setBtPolar(pNewBtPolar);
            m_pBoatTreeView->insertBtPolar(pNewBtPolar);
            m_pBoatTreeView->selectBtPolar(pNewBtPolar);
            m_pCurBtOpp = nullptr;
        }

        m_pCurBtOpp = nullptr;

        setControls();
        resetCurves();
        updateView();
    }
}


void XSail::onDuplicateCurAnalysis()
{
    if(!m_pCurBoat || !m_pCurBtPolar) return;

    BoatPolar *pNewWPolar = new BoatPolar(*m_pCurBtPolar);

/*    pNewWPolar->duplicateSpec(m_pCurBtPolar);
    pNewWPolar->setBoatName(m_pCurBoat->name());
    pNewWPolar->setName(m_pCurBtPolar->name());
    pNewWPolar->setTheStyle(m_pCurBoat->theStyle());*/
    pNewWPolar->setVisible(true);

    pNewWPolar = insertNewBtPolar(pNewWPolar, m_pCurBoat);

    if(pNewWPolar)
    {
        setBtPolar(pNewWPolar);
        m_pBoatTreeView->insertBtPolar(pNewWPolar);
        m_pBoatTreeView->selectBtPolar(pNewWPolar);
        m_pCurBtOpp = nullptr;
    }

    gl3dXSailView::resetglBoat();
    gl3dXSailView::resetglMesh();
    m_pgl3dXSailView->s_bResetglWake = true;
    gl3dXSailView::resetglBtOpp();
    updateView();
    setControls();

    emit projectModified();
}


void XSail::onDuplicateAnalysis()
{
    if(!m_pCurBoat) return;

    AnalysisSelDlg dlg(s_pMainFrame);
    dlg.initDialog(nullptr, nullptr, m_pCurBoat);
    if(dlg.exec()!=QDialog::Accepted) return;
    if(!dlg.selected3dPolars().size()) return;

    BoatPolar* pNewWPolar(nullptr);
    for(int ip=0; ip<dlg.selected3dPolars().size(); ip++)
    {
        pNewWPolar= new BoatPolar;

        pNewWPolar->duplicateSpec(dlg.selectedPolar3d(ip));
        pNewWPolar->setBoatName(m_pCurBoat->name());
        pNewWPolar->setName(dlg.selectedPolar3d(ip)->name());
        pNewWPolar->setTheStyle(m_pCurBoat->theStyle());
        pNewWPolar->setVisible(true);

        pNewWPolar = insertNewBtPolar(pNewWPolar, m_pCurBoat);
        if(pNewWPolar) m_pBoatTreeView->insertBtPolar(pNewWPolar);
    }

    if(pNewWPolar)
    {
        setBtPolar(pNewWPolar);
        m_pBoatTreeView->selectBtPolar(pNewWPolar);
        m_pCurBtOpp = nullptr;
    }

    gl3dXSailView::resetglBoat();
    gl3dXSailView::resetglMesh();
    m_pgl3dXSailView->s_bResetglWake = true;
    gl3dXSailView::resetglBtOpp();
    updateView();
    setControls();

    emit projectModified();
}


void XSail::on3dView()
{
    // m_pgl3dXSailView->s_bResetglLegend = true;

    if(is3dView())
    {
        setControls();
        updateView();
        return;
    }

    m_eView = W3DVIEW;
    setControls();

    s_pMainFrame->setActiveCentralWidget();

    updateView();
    return;
}


void XSail::onPolarView()
{
    if(isPolarView()) return;
    m_eView = POLARVIEW;
    m_bResetCurves = true;

    m_pBoatTreeView->setCurveParams();
    setGraphTiles();
    setControls();
    s_pMainFrame->setActiveCentralWidget();

    updateView();
}


void XSail::onAnalyze()
{
    if(!m_pCurBoat || !m_pCurBtPolar) return;

    for(int isail=0; isail<m_pCurBoat->nSails(); isail++)
    {
        Sail const*pSail = m_pCurBoat->sail(isail);
        if(pSail->isStlSail() && pSail->leadingEdgeAxis().norm()<0.001)
        {
            QString strange = "The corner points and luff axis of sail "+pSail->name()+" are not defined.\nAborting analysis.\n\n";
            displayMessage(strange, true);
            return ;
        }
    }

    // restore the boat's mesh, which may have been modified by BoatOpp selections
    m_pCurBoat->restoreMesh();

    m_pBtAnalysisDlg->analyze(m_pCurBoat, m_pCurBtPolar, m_pAnalysisCtrls->oppList());
    m_pBtAnalysisDlg->show();
    m_pBtAnalysisDlg->raise();

    m_pgl3dXSailView->resetglMesh();
}


void XSail::onFinishAnalysis()
{
    m_pgl3dXSailView->m_LiveVortons.clear();

    if(m_pBtAnalysisDlg->btOppList().size())
    {
        if(BoatOpp::bStoreOpps3d())
        {
            SailObjects::storeBtOpps(m_pCurBtPolar, m_pBtAnalysisDlg->btOppList());
            m_pCurBtOpp = nullptr;
        }
        else
        {
            for(int i=0; i<m_pBtAnalysisDlg->btOppList().size(); i++)
            {
                delete m_pBtAnalysisDlg->btOppList().at(i);
            }
             m_pBtAnalysisDlg->clearBtOppList();
        }
    }

    if(m_pBtAnalysisDlg->hasErrors() && Analysis3dSettings::keepOpenOnErrors())
    {
    }
    else
    {
        m_pBtAnalysisDlg->close();
    }


    m_pBoatTreeView->addBtOpps(m_pCurBtPolar);
    setBtOpp(m_pBtAnalysisDlg->lastBtOpp());
    m_pBoatTreeView->selectBtOpp();
    m_pBoatTreeView->setCurveParams();

    //refresh the view
    m_bResetCurves = true;
    updateView();
    setControls();

    m_pAnalysisCtrls->m_ppbAnalyze->setEnabled(true);

    emit projectModified();
}


void XSail::setLiveVortons(double ctrlparam, QVector<QVector<Vorton>> const &vortons)
{
    if(!is3dView()) return;
    if(vortons.size()<=0) return;

    m_pgl3dXSailView->m_LiveCtrlParam = ctrlparam;
    m_pgl3dXSailView->setLiveVortons(vortons);
    m_pgl3dXSailView->update();
}


void XSail::setGraphTiles()
{
    if(isPolarView())
    {
        switch(s_pMainFrame->m_pXSailTiles->iView())
        {
            case xfl::ONEGRAPH:
                s_pMainFrame->m_pXSailTiles->setGraphLayout(1, 0);
                break;
            case xfl::TWOGRAPHS:
                s_pMainFrame->m_pXSailTiles->setGraphLayout(2, 0);
                break;
            case xfl::FOURGRAPHS:
                s_pMainFrame->m_pXSailTiles->setGraphLayout(4, 0);
                break;
            default:
            case xfl::ALLGRAPHS:
                s_pMainFrame->m_pXSailTiles->setGraphLayout(m_PlrGraph.count(), 0);
                break;
        }
    }
}


void XSail::updateUnits()
{
    QString strSpeed  = Units::speedUnitLabel();
    QString strForce  = Units::forceUnitLabel();
    QString strMoment = Units::momentUnitLabel();
    BoatPolar::setVariableNames(strSpeed, strForce, strMoment);

    for(int ig=0; ig<m_PlrGraph.size(); ig++)
    {
        m_PlrGraph.at(ig)->setXVariableList(BoatPolar::variableNames());
        m_PlrGraph.at(ig)->setYVariableList(BoatPolar::variableNames());
    }

    m_bResetCurves = true;

    m_pBoatTreeView->setCurveParams();
    m_bResetCurves = true;
    updateView();
}


void XSail::updateView()
{
    if(isPolarView())
    {
        if(m_bResetCurves)
        {
            //remake the polar curves always to highlight selection in separate window
            createBtPolarCurves();
            s_pMainFrame->m_pXSailTiles->makeLegend(true);
        }
        s_pMainFrame->m_pXSailTiles->update();

    }
    else
        m_pgl3dXSailView->update();

    m_bResetCurves = false;
}


BoatPolar* XSail::insertNewBtPolar(BoatPolar *pNewPolar, Boat *pCurBoat)
{
    if(!pNewPolar) return nullptr;
    BoatPolar *pBtPolar=nullptr, *pOldBtPolar=nullptr;

    bool bExists = true;


    //check if this WPolar is already inserted
    for(int ip=0; ip<SailObjects::s_oaBtPolar.size(); ip++)
    {
        pOldBtPolar = SailObjects::s_oaBtPolar.at(ip);
        if(pOldBtPolar==pNewPolar)
        {
//            Trace("this WPolar is already in the array, nothing inserted");
            return pOldBtPolar;
        }
    }

    //make a list of existing names
    QStringList NameList;
    for(int k=0; k<SailObjects::s_oaBtPolar.size(); k++)
    {
        pBtPolar = SailObjects::s_oaBtPolar.at(k);
        if(pCurBoat && pBtPolar->boatName()==pCurBoat->name())
            NameList.append(pBtPolar->name());
    }

    //Is the new BtPolar's name already used?
    bExists = false;
    for (int k=0; k<NameList.count(); k++)
    {
        if(pNewPolar->name()==NameList.at(k))
        {
            bExists = true;
            break;
        }
    }

    if(!bExists)
    {
        //just insert the BtPolar in alphabetical order
        for (int l=0; l<SailObjects::s_oaBtPolar.size();l++)
        {
            pOldBtPolar = SailObjects::s_oaBtPolar.at(l);

            if(pOldBtPolar->name().compare(pNewPolar->name(), Qt::CaseInsensitive) >0)
            {
                //then insert before
                SailObjects::s_oaBtPolar.insert(l, pNewPolar);
                return pNewPolar;
            }
        }
        //not inserted, append
        SailObjects::s_oaBtPolar.append(pNewPolar);
        return pNewPolar;
    }

    // an old object with the BtPolar's name exists for this Boat, ask for a new one
    RenameDlg dlg;
    dlg.initDialog(pNewPolar->name(), SailObjects::polarNames(pCurBoat), "Enter the Polar's new name:");
    int resp = dlg.exec();

    if(resp==10)
    {
        //user wants to overwrite an existing name
        //so find the existing BtPolar with that name
        pBtPolar = nullptr;
        for(int ipb=0; ipb<SailObjects::s_oaBtPolar.size(); ipb++)
        {
            pOldBtPolar = SailObjects::s_oaBtPolar.at(ipb);
            if(pCurBoat && pOldBtPolar->name()==dlg.newName() && pOldBtPolar->boatName()==pCurBoat->name())
            {
                pBtPolar = pOldBtPolar;
                break;
            }
        }

        if(pBtPolar)
        {
            //remove and delete its children BtOpps from the array
            for (int l=SailObjects::btOppCount()-1; l>=0; l--)
            {
                BoatOpp *pPOpp = SailObjects::btOpp(l);
                if (pPOpp->boatName()==pBtPolar->boatName() && pPOpp->polarName()==pBtPolar->name())
                {
                    SailObjects::s_oaBtOpp.removeAt(l);
                    delete pPOpp;
                }
            }

            for(int ipb=0; ipb<SailObjects::btPolarCount(); ipb++)
            {
                pOldBtPolar = SailObjects::btPolar(ipb);
                if(pOldBtPolar==pBtPolar)
                {
                    SailObjects::s_oaBtPolar.removeAt(ipb);
                    delete pOldBtPolar;
                    break;
                }
            }
        }

        //room has been made, insert the new WBtPolar in alphabetical order
        pNewPolar->setName(dlg.newName());
        for (int l=0; l<SailObjects::btPolarCount();l++)
        {
            pOldBtPolar = SailObjects::btPolar(l);

            if(pOldBtPolar->name().compare(pNewPolar->name(), Qt::CaseInsensitive) >0)
            {
                //then insert before
                SailObjects::insertBtPolar(l, pNewPolar);
                return pNewPolar;
            }
        }
        //not inserted, append
        SailObjects::appendBtPolar(pNewPolar);
        return pNewPolar;

    }
    else if(resp==QDialog::Rejected)
    {
        return nullptr;
    }
    else if(resp==QDialog::Accepted)
    {
        //not rejected, no overwrite, else the user has selected a non-existing name, rename and insert
        pNewPolar->setName(dlg.newName());
        for (int l=0; l<SailObjects::s_oaBtPolar.size(); l++)
        {
            pOldBtPolar = SailObjects::s_oaBtPolar.at(l);

            if(pOldBtPolar->name().compare(pNewPolar->name(), Qt::CaseInsensitive) >0)
            {
                //then insert before
                SailObjects::s_oaBtPolar.insert(l, pNewPolar);
                return pNewPolar;
            }
        }
        //not inserted, append
        SailObjects::s_oaBtPolar.append(pNewPolar);
        return pNewPolar;

    }
    return nullptr;//should never get here
}


void XSail::onEditCurBtPolar()
{
    if(!m_pCurBoat || !m_pCurBtPolar) return;

    QString BtPolarName;

    BoatPolar *pNewBtPolar = new BoatPolar;

    BtPolarDlg dlg(s_pMainFrame);
    dlg.initDialog(m_pCurBoat, m_pCurBtPolar);

    if (dlg.exec() == QDialog::Accepted)
    {
        pNewBtPolar->duplicateSpec(&dlg.staticBtPolar());
        BtPolarName=dlg.polarName();

        pNewBtPolar->setBoatName(m_pCurBoat->name());
        pNewBtPolar->setName(BtPolarName);
        pNewBtPolar->setVisible(true);

        pNewBtPolar = insertNewBtPolar(pNewBtPolar, m_pCurBoat);

        m_pCurBtOpp = nullptr;

        setBtPolar(pNewBtPolar);

        m_pBoatTreeView->insertBtPolar(pNewBtPolar);
        m_pBoatTreeView->selectBtPolar(pNewBtPolar);

        m_pgl3dXSailView->resetglBoat();
        m_pgl3dXSailView->resetglMesh();
        m_pgl3dXSailView->resetglBtOpp();
        m_pgl3dXSailView->s_bResetglWake = true;

        m_bResetCurves = true;

        emit projectModified();
    }
    else
    {
        delete pNewBtPolar;
    }
    setControls();
    updateView();
}


void XSail::onRenameCurBtPolar()
{
    if(!m_pCurBtPolar) return;
    if(!m_pCurBoat) return;

    renameBtPolar(m_pCurBtPolar, m_pCurBoat);
    m_pBoatTreeView->selectBtPolar(m_pCurBtPolar);
}


void XSail::renameBtPolar(BoatPolar *pBtPolar, Boat const *pBoat)
{
    if(!pBoat) return;

    RenameDlg dlg;
    dlg.initDialog(pBtPolar->name(), SailObjects::polarNames(pBoat), "Enter the Polar's new name:");
    int resp = dlg.exec();
    if(resp==QDialog::Rejected)
    {
        return;
    }
    else if(resp==10)
    {
        //the user wants to overwrite an existing name
        if(dlg.newName()==pBtPolar->name()) return; //what's the point?

        // it's a real overwrite
        // so find and delete the existing WPolar with the new name
        for(int ipb=0; ipb<SailObjects::btPolarCount(); ipb++)
        {
            BoatPolar *pOldBtPolar = SailObjects::btPolar(ipb);
            if(pOldBtPolar->name()==dlg.newName() && pOldBtPolar->boatName()==m_pCurBoat->name())
            {
                SailObjects::deleteBtPolar(pOldBtPolar);
                break;
            }
        }
    }

    //ready to insert
    //remove the WPolar from its current position in the array
    for (int l=0; l<SailObjects::btPolarCount();l++)
    {
        BoatPolar *pOldBtPolar = SailObjects::btPolar(l);
        if(pOldBtPolar==pBtPolar)
        {
            SailObjects::s_oaBtPolar.removeAt(l);
            break;
        }
    }

    //set the new name
    for (int l=SailObjects::btOppCount()-1;l>=0; l--)
    {
        BoatOpp *pBtOpp = SailObjects::btOpp(l);
        if (pBtOpp->boatName() == m_pCurBoat->name() && pBtOpp->polarName()==pBtPolar->name())
        {
            pBtOpp->setName(dlg.newName());
        }
    }

    pBtPolar->setName(dlg.newName());

    //insert alphabetically
    bool bInserted = false;
    for (int l=0; l<SailObjects::btPolarCount();l++)
    {
        BoatPolar *pOldBtPolar = SailObjects::btPolar(l);

        if(pOldBtPolar->name().compare(pBtPolar->name(), Qt::CaseInsensitive) >0)
        {
            //then insert before
            SailObjects::s_oaBtPolar.insert(l, pBtPolar);
            bInserted = true;
            break;
        }
    }

    if(!bInserted) SailObjects::s_oaBtPolar.append(pBtPolar);

    updateObjectView();

    emit projectModified();

    m_bResetCurves = true;
    updateView();
}



/**
 * The user has requested a modification of the styles for the 3D view
 * Launhes the dialog box, reads the data, and updates the view
*/
void XSail::resetPrefs()
{
    if(m_pCurBoat)
    {
        m_pCurBoat->triangulateSails();
    }

    m_pgl3dXSailView->s_bResetglWake = true;
    m_pgl3dXSailView->s_bResetglBoat = true;
    m_pgl3dXSailView->s_bResetglMesh = true;
    m_pgl3dXSailView->resetglBtOpp();
    m_pgl3dXSailView->resetglStreamLines();

    m_pgl3dXSailView->glMakeArcBall(m_pgl3dXSailView->m_ArcBall);
    m_pgl3dXSailView->glMakeArcPoint(m_pgl3dXSailView->m_ArcBall);

    m_pgl3dXSailView->setLabelFonts();

    m_pBoatTreeView->setPropertiesFont(DisplayOptions::tableFont());
    m_pBoatTreeView->setTreeFontStruct(DisplayOptions::treeFontStruct());

    if(!W3dPrefs::isClipPlaneEnabled())
    {
        m_pgl3dXSailView->m_ClipPlanePos = 500.0f;
     }

    if(s_pMainFrame->xflApp()==xfl::XSAIL)
    {
        m_bResetCurves = true;
        setControls();
        updateView();
    }
}


Boat *XSail::setBoat(QString const &BoatName)
{
    if(BoatName.length())
    {
        for(int i=0; i<SailObjects::boatCount(); i++)
        {
            Boat *pBoat = SailObjects::boat(i);
            if(pBoat->name()==BoatName) setBoat(pBoat);
        }
    }
    else
    {
        if(SailObjects::boatCount())
            setBoat(SailObjects::boat(0));
        else
            setBoat(nullptr);
    }
    return m_pCurBoat;
}


Boat *XSail::setBoat(Boat *pBoat)
{
    m_pCurBoat = pBoat;
    cancelDisplayThreads();

    if(!m_pCurBoat)
    {
        m_pgl3dXSailView->setBoat(nullptr);
        setBtPolar();
        return nullptr;
    }

    m_pgl3dXSailView->setBoat(m_pCurBoat);
    m_pgl3dXSailView->resetglBoat();

    if(W3dPrefs::autoAdjust3dScale()) m_pgl3dXSailView->reset3dScale();

    return m_pCurBoat;
}


BoatPolar *XSail::setBtPolar(QString BPlrName)
{
    cancelDisplayThreads();

    if(!m_pCurBoat)
    {
        m_pCurBtPolar = nullptr;
        m_pCurBtOpp   = nullptr;
        return nullptr;
    }

    if(BPlrName.isEmpty())
    {
        // find the first in the list which matches the boat's name
        for(int j=0; j<SailObjects::btPolarCount(); j++)
        {
            BoatPolar *pBtPolar = SailObjects::btPolar(j);
            if(pBtPolar->boatName()==m_pCurBoat->name())
            {
                return setBtPolar(pBtPolar);
            }
        }
    }
    else
    {
        for(int j=0; j<SailObjects::btPolarCount(); j++)
        {
            BoatPolar *pBtPolar = SailObjects::btPolar(j);
            if(pBtPolar->name()==BPlrName && pBtPolar->boatName()==m_pCurBoat->name())
            {
                return setBtPolar(pBtPolar);
            }
        }
    }
    return setBtPolar(nullptr);
}


BoatPolar *XSail::setBtPolar(BoatPolar *pBtPolar)
{
    cancelDisplayThreads();
    m_pAnalysisCtrls->setPolar3d(pBtPolar);
    m_pgl3dXSailView->resetglMesh();
    m_pCurBtPolar = pBtPolar;

    if(!m_pCurBoat)
    {
        m_pCurBtPolar = nullptr;
        m_pCurBtOpp = nullptr;
    }
    else if(!pBtPolar || m_pCurBoat->name()!=pBtPolar->boatName())
    {
        m_pCurBoat->makeRefTriMesh(true, xfl::isMultiThreaded());
        m_pCurBoat->restoreMesh();
        m_pCurBtOpp = nullptr;
    }
    else
    {
        m_pCurBoat->makeRefTriMesh(!m_pCurBtPolar->bIgnoreBodyPanels(), xfl::isMultiThreaded());
        m_pCurBoat->restoreMesh();

        // clean up legacy data
        if(m_pCurBtPolar->bAutoRefDims())
        {
            if(m_pCurBoat->hasSail())
            {
                Sail const*pSail = m_pCurBoat->sailAt(0);
                if(pSail->refArea()>1.e-6) m_pCurBtPolar->setReferenceArea(pSail->refArea());
                else                       m_pCurBtPolar->setReferenceArea(1.0);
                if(pSail->refChord()>1.e-4) m_pCurBtPolar->setReferenceChordLength(pSail->refChord());
                else                       m_pCurBtPolar->setReferenceChordLength(1.0);
            }
        }
    }

    m_bResetCurves = true;

    return m_pCurBtPolar;
}


BoatOpp *XSail::setBtOpp(double ctrl)
{
    cancelDisplayThreads();
    if(!m_pCurBoat || !m_pCurBtPolar)
    {
        m_pCurBtOpp = nullptr;
        m_pgl3dXSailView->setBotRightOutput(QString());
//        s_pMainFrame->m_pgl3dScales->enableFlowControls();
        return nullptr;
    }

    BoatOpp *pNewBtOpp = SailObjects::getBoatOppObject(m_pCurBoat, m_pCurBtPolar, ctrl);
    if(pNewBtOpp)
    {
        m_LastCtrl = pNewBtOpp->ctrl();
    }

    if(pNewBtOpp==m_pCurBtOpp)
    {
//        s_pMainFrame->m_pgl3dScales->enableFlowControls();
        return pNewBtOpp;
    }

    return setBtOpp(pNewBtOpp);
}


BoatOpp *XSail::setBtOpp(BoatOpp *pBoatOpp)
{
    m_pCurBtOpp = pBoatOpp;

    setControls();

    m_bResetCurves = true;

    if(!m_pCurBtOpp || !m_pCurBtPolar || !m_pCurBoat)
    {
        m_pgl3dXSailView->setBotRightOutput(QString());
        if(m_pCurBoat) m_pCurBoat->restoreMesh();
//        s_pMainFrame->m_pgl3dScales->enableFlowControls();
        return nullptr;
    }

    m_pCurBoat->restoreMesh();
    m_pCurBoat->rotateMesh(m_pCurBtPolar, m_pCurBtOpp->phi(), m_pCurBtOpp->Ry(), m_pCurBtOpp->ctrl(), m_pCurBoat->triMesh().panels());

    if(!m_pCurBtPolar->bVortonWake())
    {
        Vector3d pt;
        m_pCurBoat->triMesh().getLastTrailingPoint(pt);
        m_pCurBoat->triMesh().makeWakePanels(m_pCurBtPolar->NXWakePanel4(), m_pCurBtPolar->wakePanelFactor(),
                                             pt.x+m_pCurBtPolar->wakeLength(), m_pCurBtOpp->windDir(), true);
    }
    else
    {
        m_pCurBoat->triMesh().makeWakePanels(3, 1.0, m_pCurBtPolar->bufferWakeLength(), m_pCurBtOpp->windDir(), false);
    }

    m_pgl3dXSailView->resetglMesh();
    m_pgl3dXSailView->resetglBtOpp();
    m_pgl3dXSailView->resetFlow();
    m_pgl3dXSailView->s_bResetglWake = true;

//    s_pMainFrame->m_pgl3dScales->makeXSailVelocityVectors();
//    s_pMainFrame->m_pgl3dScales->enableFlowControls();
    return m_pCurBtOpp;
}


void XSail::onDeleteCurBtPolar()
{
    if(!m_pCurBtPolar) return;

    QString strong = "Are you sure you want to delete the polar :\n" +  m_pCurBtPolar->name() +"?\n";
    if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, "Question", strong,
                                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)) return;

    QString nextWPolarName = m_pBoatTreeView->removeBtPolar(m_pCurBtPolar);
    SailObjects::deleteBtPolar(m_pCurBtPolar);

    m_pCurBtOpp = nullptr;
    m_pCurBtPolar = nullptr;
    emit projectModified();

    setBtPolar(nextWPolarName);

    if(m_pCurBtPolar) m_pBoatTreeView->selectBtPolar(m_pCurBtPolar);
    else              m_pBoatTreeView->selectBoat(m_pCurBoat);

    m_pBoatTreeView->setObjectProperties();

    setControls();
    resetCurves();
    updateView();
}


void XSail::onCurveClicked(Curve* pCurve, int)
{
    if(!pCurve)
    {
        for(int ig=0; ig<m_PlrGraph.size(); ig++) m_PlrGraph[ig]->clearSelection();
        s_pMainFrame->m_pXSailTiles->legendWt()->makeLegend(false);
        updateView();
        return;
    }

    if(isPolarView())
    {
        for(int iwp=0; iwp<SailObjects::btPolarCount(); iwp++)
        {
            BoatPolar *pBtPolar=SailObjects::btPolar(iwp);
            for(int ic=0; ic<pBtPolar->curveCount(); ic++)
            {
                if(pBtPolar->curve(ic)==pCurve)
                {
                    //this is the one which has been clicked
                    setBoat(pBtPolar->boatName());
                    setBtPolar(pBtPolar);
                    m_pBoatTreeView->selectBtPolar(pBtPolar);
                    m_pBoatTreeView->setCurveParams();
                    m_bResetCurves = true;
                    updateView();
                    s_pMainFrame->m_pXSailTiles->update();
                    return;
                }
            }
        }
    }
}


void XSail::onCurveDoubleClicked(Curve* pCurve)
{
    if(!pCurve) return;

    if(isPolarView())
    {
        for(int iwp=0; iwp<SailObjects::btPolarCount(); iwp++)
        {
            BoatPolar *pBtPolar=SailObjects::btPolar(iwp);
            for(int ic=0; ic<pBtPolar->curveCount(); ic++)
            {
                if(pBtPolar->curve(ic)==pCurve)
                {
                    //this is the one which has been clicked
                    LineStyle ls(pBtPolar->theStyle());
                    LineMenu *pLineMenu = new LineMenu(nullptr);
                    pLineMenu->initMenu(ls);
                    pLineMenu->exec(QCursor::pos());
                    ls = pLineMenu->theStyle();
                    pBtPolar->setLineStipple(ls.m_Stipple);
                    pBtPolar->setLineWidth(ls.m_Width);
                    pBtPolar->setLineColor(ls.m_Color);
                    pBtPolar->setPointStyle(ls.m_Symbol);

                    m_pBoatTreeView->setCurveParams();
                    m_bResetCurves = true;
                    updateView();
                    s_pMainFrame->m_pXSailTiles->update();
                    return;
                }
            }
        }
    }
}


void XSail::onRenameCurBoat()
{
    if(!m_pCurBoat)    return;
    QString oldName = m_pCurBoat->name();
    renameBoat(m_pCurBoat);

    QString newName = m_pCurBoat->name();
    if(newName.compare(oldName, Qt::CaseInsensitive)!=0)
    {
        m_pBoatTreeView->removeBoat(oldName);
        m_pBoatTreeView->insertBoat(m_pCurBoat);
        m_pBoatTreeView->selectBoat(m_pCurBoat);
        emit projectModified();
    }

    updateView();
}


/**
 * Renames the active wing or plane
 * Updates the references in child polars and oppoints
 * @param PlaneName the new name for the wing or plane
 */
void XSail::renameBoat(Boat *pBoat)
{
    if(!pBoat) return;

    QString OldName = pBoat->name();
    setModifiedBoat(pBoat);

    for (int l=SailObjects::btPolarCount()-1;l>=0; l--)
    {
        BoatPolar *pBtPolar = SailObjects::btPolar(l);
        if (pBtPolar->boatName() == OldName)
        {
            pBtPolar->setBoatName(pBoat->name());
        }
    }
    for (int l=SailObjects::btOppCount()-1; l>=0; l--)
    {
        BoatOpp *pBtOpp = SailObjects::btOpp(l);
        if (pBtOpp->boatName() == OldName)
        {
            pBtOpp->setBoatName(pBoat->name());
        }
    }
}


/** ask and process the user's intention for this new modified boat */
Boat* XSail::setModBoat(Boat *pModBoat)
{
    emit projectModified();

    //save mods to a new plane object
    Boat *pBoat = setModifiedBoat(pModBoat);
    if(!pBoat) return nullptr;

    setBoat(pBoat);
    setBtPolar(m_pCurBtPolar);
    setBtOpp(m_pCurBtOpp);

    setControls();
    if(pBoat->hull(0)) pBoat->hull(0)->makeFuseGeometry();
    m_pBoatTreeView->insertBoat(pBoat);
    m_pBoatTreeView->selectBoat(pBoat);

    gl3dXSailView::s_bResetglBoat   = true;
    gl3dXSailView::s_bResetglMesh   = true;
    gl3dXSailView::s_bResetglWake   = true;
    m_bResetCurves = true;
    updateView();
    return pBoat;
}


void XSail::onDuplicateCurBoat()
{
    if(!m_pCurBoat) return;
    Boat *pNewBoat = new Boat(*m_pCurBoat);
    pNewBoat->setName(m_pCurBoat->name() + "(2)");
    SailObjects::appendBoat(pNewBoat);
    setBoat(pNewBoat);
    if(m_pCurBoat)
    {
        m_pBoatTreeView->insertBoat(m_pCurBoat);
        m_pBoatTreeView->selectBoat(m_pCurBoat);
        m_pCurBoat->makeRefTriMesh(true, xfl::isMultiThreaded());
        m_pCurBoat->restoreMesh();
    }
    m_pCurBtPolar = nullptr;
    m_pCurBtOpp=nullptr;

    updateView();
    emit projectModified();
}



void XSail::onShowBtPolars()
{
    if(!m_pCurBoat) return;
    for(int i=0; i<SailObjects::btPolarCount(); i++)
    {
        BoatPolar *pBtPolar = SailObjects::btPolar(i);
        if(pBtPolar->boatName()==m_pCurBoat->name())
            pBtPolar->setVisible(true);
    }
    updateView();
}

void XSail::onHideBtPolars()
{
    if(!m_pCurBoat) return;
    for(int i=0; i<SailObjects::btPolarCount(); i++)
    {
        BoatPolar *pBtPolar = SailObjects::btPolar(i);
        if(pBtPolar->boatName()==m_pCurBoat->name())
            pBtPolar->setVisible(false);
    }
    updateView();
}

void XSail::onDeleteBtPolars()
{
    if(!m_pCurBoat) return;

    QString BoatName, strong;

    BoatName = m_pCurBoat->name();

    strong = "Are you sure you want to delete the polars associated to :\n" +  BoatName +"?\n";
    if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, "Question", strong, QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
        return;

    if(m_pCurBoat)
    {
        for (int i=SailObjects::btPolarCount()-1; i>=0; i--)
        {
            BoatPolar *pBtPolar = SailObjects::btPolar(i);
            if (pBtPolar->boatName() == m_pCurBoat->name())
            {
                m_pBoatTreeView->removeBtPolar(pBtPolar);
            }
        }
    }

    SailObjects::deleteBoatResults(m_pCurBoat, true);

    m_pCurBtPolar = nullptr;
    m_pCurBtOpp = nullptr;

    setBtPolar(nullptr);
    m_pBoatTreeView->selectBoat(m_pCurBoat);

    emit projectModified();
    setControls();
}

void XSail::onShowOnlyBtPolars()
{
    if(!m_pCurBoat) return;
    for(int i=0; i<SailObjects::btPolarCount(); i++)
    {
        BoatPolar *pBtPolar = SailObjects::btPolar(i);
        if(pBtPolar->boatName()==m_pCurBoat->name())
            pBtPolar->setVisible(true);
        else
            pBtPolar->setVisible(false);
    }
    updateView();
}


void XSail::onShowBtOpps()
{
    if(!m_pCurBoat) return;
    for(int i=0; i<SailObjects::btOppCount(); i++)
    {
        BoatOpp *pBtOpp = SailObjects::btOpp(i);
        if(pBtOpp->boatName()==m_pCurBoat->name()) pBtOpp->setVisible(true);
    }
    updateView();
}


void XSail::onHideBtOpps()
{
    if(!m_pCurBoat) return;
    for(int i=0; i<SailObjects::btOppCount(); i++)
    {
        BoatOpp *pBtOpp = SailObjects::btOpp(i);
        if(pBtOpp->boatName()==m_pCurBoat->name()) pBtOpp->setVisible(false);
    }

    updateView();
}


void XSail::onDeleteCurBtOpp()
{
    BoatOpp *pBtOpp = m_pCurBtOpp;
    if(!pBtOpp) return;

    m_LastCtrl = pBtOpp->ctrl();

    BoatOpp* pOldBtOpp=nullptr;

    for (int i=SailObjects::btOppCount()-1; i>=0; i--)
    {
        pOldBtOpp = SailObjects::btOpp(i);
        if(pOldBtOpp == pBtOpp)
        {
            SailObjects::s_oaBtOpp.removeAt(i);
            m_pBoatTreeView->removeBoatOpp(pBtOpp);
            if(m_pCurBtOpp==pBtOpp) m_pCurBtOpp = nullptr;
            delete pBtOpp;
            break;
        }
    }
    setBtOpp(m_LastCtrl);

    if     (m_pCurBtOpp)   m_pBoatTreeView->selectBtOpp(m_pCurBtOpp);
    else if(m_pCurBtPolar) m_pBoatTreeView->selectBtPolar(m_pCurBtPolar);
    else                   m_pBoatTreeView->selectBoat(m_pCurBoat);
    m_pBoatTreeView->setObjectProperties();

    emit projectModified();

    m_pgl3dXSailView->resetglMesh();
    m_pgl3dXSailView->resetglBtOpp();
    m_bResetCurves = true;
    updateView();
}


void XSail::onDeleteBoatBtOpps()
{
    if(!m_pCurBoat) return;
    SailObjects::deleteBoatBtOpps(m_pCurBoat);
    m_pCurBtOpp = nullptr;

    emit projectModified();
    updateObjectView();
    m_pBoatTreeView->selectBtPolar(m_pCurBtPolar);

    setControls();
    m_bResetCurves = true;
    updateView();
}


void XSail::onEditSail()
{
    m_pgl3dXSailView->hideArcball();
    if(!m_pCurBoat) return;

    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    Sail *pSail = nullptr;

    if(pSenderAction==m_pActions->m_pEditMainSail)
        pSail = m_pCurBoat->mainSail();
    else if(pSenderAction==m_pActions->m_pEditJib)
        pSail = m_pCurBoat->jib();

    if(!pSail) return;

    editSail(pSail);
    updateView();
}


void XSail::editSail(Sail *pSail)
{
    if(pSail->isNURBSSail())
    {
        NURBSSail MemSail;
        MemSail.duplicate(pSail);

        NURBSSailDlg dlg(s_pMainFrame);
        dlg.initDialog(pSail);
        if(dlg.exec()!=QDialog::Accepted)
        {
            pSail->duplicate(&MemSail);
            return;
        }
        if(dlg.bDescriptionChanged() && !dlg.bChanged())
        {
            return; // save the meta data = colour + description without deleting the results
        }
    }
    else if(pSail->isSplineSail())
    {
        SplineSail MemSail;

        MemSail.duplicate(pSail);

        SplineSailDlg dlg(s_pMainFrame);
        dlg.initDialog(pSail);

        if(dlg.exec()!=QDialog::Accepted)
        {
            pSail->duplicate(&MemSail);
            return;
        }
        if(dlg.bDescriptionChanged() && !dlg.bChanged())
        {
            return; // save the meta data = colour + description without deleting the results
        }
    }
    else if(pSail->isWingSail())
    {
        WingSail MemSail;
        MemSail.duplicate(pSail);

        WingSailDlg dlg(s_pMainFrame);
        dlg.initDialog(pSail);
        if(dlg.exec()!=QDialog::Accepted)
        {
            pSail->duplicate(&MemSail);
            return;
        }
        if(dlg.bDescriptionChanged() && !dlg.bChanged())
        {
            return; // save the meta data = colour + description without deleting the results
        }
    }
    else if(pSail->isStlSail())
    {
        STLSail MemSail;
        MemSail.duplicate(pSail);

        STLSailDlg dlg(s_pMainFrame);
        dlg.initDialog(pSail);
        if(dlg.exec()!=QDialog::Accepted)
        {
            pSail->duplicate(&MemSail);
            return;
        }
        if(dlg.bDescriptionChanged() && !dlg.bChanged())
        {
            return; // save the meta data = colour + description without deleting the results
        }
    }
    else if(pSail->isOccSail())
    {
        OccSail MemSail;
        MemSail.duplicate(pSail);

        OccSailDlg dlg(s_pMainFrame);
        dlg.initDialog(pSail);
        if(dlg.exec()!=QDialog::Accepted)
        {
            pSail->duplicate(&MemSail);
            return;
        }
        if(dlg.bDescriptionChanged() && !dlg.bChanged())
        {
            return; // save the meta data = colour + description without deleting the results
        }
    }

    SailObjects::deleteBoatResults(m_pCurBoat, false);
    m_pBoatTreeView->removeBtOpps(m_pCurBoat);
    m_pCurBtOpp=nullptr;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_pCurBoat->makeRefTriMesh(m_pCurBtPolar && !m_pCurBtPolar->bIgnoreBodyPanels(), xfl::isMultiThreaded());
    m_pCurBoat->restoreMesh();

    m_pgl3dXSailView->resetglBoat();
    resetCurves();
    emit projectModified();
    QApplication::restoreOverrideCursor();
    updateView();
}


void XSail::onScaleSailShape()
{
    if(!m_pCurBoat) return;
    if(!m_pCurBoat->hasSail()) return;

    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    Boat *pModBoat = new Boat;
    pModBoat->duplicate(m_pCurBoat);

    Sail *pSail = nullptr;

    if(pSenderAction==m_pActions->m_pScaleMainShape)
        pSail = pModBoat->mainSail();
    else if(pSenderAction==m_pActions->m_pScaleJibShape)
        pSail = pModBoat->jib();

    if(!pSail) return;
    SailScaleDlg dlg(s_pMainFrame);
    dlg.initDialog(pSail);
    if(dlg.exec()==QDialog::Rejected) return;
    if (dlg.m_bArea || dlg.m_bAR || dlg.m_bTwist)
    {
        if(dlg.m_bArea)  pSail->scaleArea( dlg.m_NewArea);
        if(dlg.m_bAR)    pSail->scaleAR(   dlg.m_NewAR);
        if(dlg.m_bTwist) pSail->scaleTwist(dlg.m_NewTwist);
        pSail->makeSurface();
        pSail->makeTriangulation();
        if(pSail->bRuledMesh()) pSail->makeRuledMesh(Vector3d());
        else                    pSail->clearRefTriangles();
    }
    else return;

    if(!setModBoat(pModBoat))
    {
        delete pModBoat;
        return; // the user has changed his mind
    }

    m_pgl3dXSailView->resetglSail();
    m_pgl3dXSailView->resetglMesh();
    updateView();
    emit projectModified();
}


void XSail::onScaleSailSize()
{
    if(!m_pCurBoat) return;
    if(!m_pCurBoat->hasSail()) return;

    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    Boat *pModBoat = new Boat;
    pModBoat->duplicate(m_pCurBoat);

    Sail *pSail = nullptr;

    if(pSenderAction==m_pActions->m_pScaleMainSize)
        pSail = pModBoat->mainSail();
    else if(pSenderAction==m_pActions->m_pScaleJibSize)
        pSail = pModBoat->jib();

    if(!pSail) return;
    QStringList labels({"Scale factor",});
    QStringList rightlabels({QString()});
    QVector<double> vals({1.0});
    DoubleValueDlg dlg(s_pMainFrame, vals, labels, rightlabels);

    if(dlg.exec()!=QDialog::Accepted) return;

    pSail->scale(dlg.value(0), dlg.value(0), dlg.value(0));
    pSail->makeTriangulation();
    pSail->makeTriPanels(Vector3d());

    if(!setModBoat(pModBoat))
    {
        delete pModBoat;
        return; // the user has changed his mind
    }

    m_pgl3dXSailView->resetglSail();
    m_pgl3dXSailView->resetglMesh();
    updateView();
    emit projectModified();
}


void XSail::onTranslateSail()
{
    if(!m_pCurBoat) return;
    if(!m_pCurBoat->hasSail()) return;

    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    Boat *pModBoat = new Boat;
    pModBoat->duplicate(m_pCurBoat);
    Sail *pSail = nullptr;
    if(pSenderAction==m_pActions->m_pTranslateMainSail)
        pSail = pModBoat->mainSail();
    else if(pSenderAction==m_pActions->m_pTranslateJib)
        pSail = pModBoat->jib();
    if(!pSail) return;

    TranslateDlg dlg(s_pMainFrame);
    if(dlg.exec()==QDialog::Rejected) return;

    Vector3d pos = pSail->position();
    pos += dlg.translationVector();
    pSail->setPosition(pos);
    pSail->makeSurface();
    pSail->makeTriangulation();
    if(pSail->bRuledMesh()) pSail->makeRuledMesh(Vector3d());
    else                    pSail->clearRefTriangles();

    if(!setModBoat(pModBoat))
    {
        delete pModBoat;
        return; // the user has changed his mind
    }

    m_pgl3dXSailView->resetglSail();
    m_pgl3dXSailView->resetglMesh();
    updateView();
    emit projectModified();
}


void XSail::onExportSailToXML() const
{
    if(!m_pCurBoat) return;
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    Sail *pSail = nullptr;
    if(pSenderAction==m_pActions->m_pExportMainSailToXml) pSail = m_pCurBoat->mainSail();
    else if(pSenderAction==m_pActions->m_pExportJibToXml) pSail = m_pCurBoat->jib();
    if(!pSail) return;

    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = pSail->name().trimmed();
    strong.replace(' ', '_');
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export to xml file",
                                            SaveOptions::xmlPlaneDirName() +'/'+strong,
                                            filter, &filter);

    if(!FileName.length()) return;

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    XmlSailWriter writer(XFile);
    writer.writeXMLSail(pSail);

    XFile.close();
}


void XSail::onExportSailToSTL() const
{
    if(!m_pCurBoat) return;
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    Sail *pSail = nullptr;
    if(pSenderAction==m_pActions->m_pExportMainSailToSTL)
    {
        pSail = m_pCurBoat->mainSail();
    }
    else if(pSenderAction==m_pActions->m_pExportJibToSTL)
    {
        pSail = m_pCurBoat->jib();
    }
    if(!pSail) return;

    Sail *pExportSail = pSail->clone();

    pExportSail->makeTriPanels(Vector3d());
    exportMeshToSTLFile(pExportSail->triMesh(), pSail->name(), SaveOptions::STLDirName(), 1.0);
    delete pExportSail;
}


void XSail::onExportSailToStep()
{
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    Sail *pSail = nullptr;
    if(pSenderAction==m_pActions->m_pExportMainSailToStep)
    {
        pSail = m_pCurBoat->mainSail();
    }
    else if(pSenderAction==m_pActions->m_pExportJibToStep)
    {
        pSail = m_pCurBoat->jib();
    }
    if(!pSail) return;
    if(!pSail->isNURBSSail()) return;

    TopoDS_Shape shape;
    QString log;
    NURBSSail *pNS = dynamic_cast<NURBSSail*>(pSail);
    if(!pNS->makeOccShell(shape, log) || shape.IsNull())
    {
        displayMessage("Failed to build the nurbs ...aborting", true);
        return;
    }

/*    VrmlAPI_Writer aWriter;
    const TCollection_AsciiString aFilePath ("/home/techwinder/tmp/file.vmrl");
    aWriter.Write (shape, aFilePath.ToCString());*/

    CADExportDlg dlg(s_pMainFrame);
    dlg.init(shape, pNS->name());
    dlg.exec();
}


void XSail::onConvertSailToNURBS()
{
    if(!m_pCurBoat) return;
    QAction *pSenderAction = qobject_cast<QAction *>(sender());
    if (!pSenderAction) return;

    Sail *pSail = nullptr;

    if(pSenderAction==m_pActions->m_pConvertMainSailToNURBS)
        pSail = m_pCurBoat->mainSail();
    else if(pSenderAction==m_pActions->m_pConvertJibToNURBS)
        pSail = m_pCurBoat->jib();
    if(!pSail) return;
    if(!pSail->isSplineSail()) return;

    int index=0;
    for(index=0; index<m_pCurBoat->sailCount(); index++)
    {
        if(m_pCurBoat->sail(index)==pSail) break;
    }
    if(index>=m_pCurBoat->sailCount()) return;

    SplineSail *pSSail = dynamic_cast<SplineSail*>(pSail);

    NURBSSail *pNS = new NURBSSail;
    NURBSSurface &nurbs = pNS->nurbs();
    nurbs.setFrameCount(pSSail->sectionCount());
    nurbs.setFramePointCount(pSSail->firstSpline()->ctrlPointCount());
    for(int isec=0; isec<pSSail->sectionCount(); isec++)
    {
        Spline const *pSpline = pSSail->spline(isec);
        Frame &pFrame = nurbs.frame(isec);
        for(int ic=0; ic<pSpline->ctrlPointCount(); ic++)
        {
            Vector2d const &pt2d = pSpline->controlPoint(ic);
            Vector3d pt3d(pt2d.x, pt2d.y, pSSail->sectionPosition(isec).z);
            pt3d.rotateY(pSSail->sectionPosition(isec), pSSail->sectionAngle(isec));
            pFrame.setCtrlPoint(ic, pt3d);
            pFrame.setZPosition(pSSail->sectionPosition(isec).z);
        }
    }
    pNS->makeSurface();
    pNS->makeTriangulation();
    pNS->setPartDescription("Converted from spline type sail "+ pSSail->name()+", with potential loss of geometric accuracy");

    Boat *pModBoat = new Boat;
    pModBoat->setDescription("Duplicate of boat "+m_pCurBoat->name()
                             +" with sail "+pSSail->name()+" converted to NURBS");
    for(int is=0; is<m_pCurBoat->nSails(); is++)
    {
        if(is==index) pModBoat->appendSail(pNS);
        else
        {
            Sail *pSail = m_pCurBoat->sail(is)->clone();
            pModBoat->appendSail(pSail);
        }
    }
    for(int ih=0; ih<m_pCurBoat->nHulls(); ih++)
    {
        Fuse *pFuse = m_pCurBoat->hull(ih)->clone();
        pModBoat->appendHull(pFuse);
    }

    setModBoat(pModBoat);
    setBtPolar(m_pCurBtPolar);
    setControls();

    m_pgl3dXSailView->resetglBoat();
    updateView();
    emit projectModified();
}


void XSail::onEditHull()
{
    m_pgl3dXSailView->hideArcball();

    if(!m_pCurBoat) return;
    Fuse *pFuse = m_pCurBoat->hull();
    if(!pFuse) return;

    if(pFuse->isXflType())
    {
        FuseXfl *pXflFuse = dynamic_cast<FuseXfl*>(pFuse);
        Fuse *pMemBody = pXflFuse->clone();

        FuseXflDefDlg glbDlg(s_pMainFrame);
        glbDlg.enableName(false);
        glbDlg.initDialog(pXflFuse);

        if(glbDlg.exec() != QDialog::Accepted)
        {
            pXflFuse->duplicateFuse(*pMemBody);
            delete pMemBody;
            return;
        }
        delete pMemBody;
    }
    else if(pFuse->isOccType())
    {
        FuseOcc *pOccFuse = dynamic_cast<FuseOcc*>(pFuse);
        FuseOcc memBody(*pOccFuse);
        FuseOccDlg obDlg(s_pMainFrame);
        obDlg.initDialog(pOccFuse);
        if(obDlg.exec() != QDialog::Accepted)
        {
            pOccFuse->duplicateFuse(memBody);
            return;
        }
    }
    else if(pFuse->isStlType())
    {
        FuseStl *pStlFuse = dynamic_cast<FuseStl*>(pFuse);
        FuseStl memFuseStl(*pStlFuse);
        FuseStlDlg sbDlg(s_pMainFrame);

        sbDlg.initDialog(pStlFuse);
        if(sbDlg.exec() != QDialog::Accepted)
        {
            pStlFuse->duplicateFuse(memFuseStl);
            return;
        }
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    SailObjects::deleteBoatResults(m_pCurBoat, false);
    m_pBoatTreeView->removeBtOpps(m_pCurBoat);
    m_pCurBtOpp=nullptr;

    pFuse->makeFuseGeometry();
//    pFuse->makeDefaultTriMesh(logmsg);
    m_pCurBoat->makeRefTriMesh(m_pCurBtPolar && !m_pCurBtPolar->bIgnoreBodyPanels(), xfl::isMultiThreaded());

    m_pgl3dXSailView->resetglHull();
    updateView();
    emit projectModified();
    QApplication::restoreOverrideCursor();
}


void XSail::onScaleHull()
{
    if(!m_pCurBoat) return;
    if(!m_pCurBoat->hasHull()) return;
    Fuse *pHull = m_pCurBoat->hull();
    if(!pHull) return;
    BodyScaleDlg dlg;
    dlg.initDialog(false);
    if(dlg.exec()==QDialog::Rejected) return;

    pHull->scale(dlg.XFactor(), dlg.YFactor(), dlg.ZFactor());
    m_pgl3dXSailView->resetglHull();
    updateView();
    emit projectModified();
}


void XSail::onTranslateHull()
{
    if(!m_pCurBoat) return;
    if(!m_pCurBoat->hasHull()) return;
    Fuse *pHull = m_pCurBoat->hull(0);
    if(!pHull) return;
    TranslateDlg dlg;
    if(dlg.exec()==QDialog::Rejected) return;

    Vector3d pos = pHull->position();
    pos += dlg.translationVector();
    pHull->setPosition(pos);
    m_pgl3dXSailView->resetglHull();
    updateView();
    emit projectModified();
}


void XSail::onShowAllBtPolars()
{
    BoatPolar *pBtPolar;
    for (int i=0; i<SailObjects::btPolarCount(); i++)
    {
        pBtPolar = SailObjects::btPolar(i);
        pBtPolar->setVisible(true);
    }

    emit projectModified();
    m_pBoatTreeView->setCurveParams();
    m_bResetCurves = true;
    updateView();
}


void XSail::onHideAllBtPolars()
{
    BoatPolar *pBtPolar;
    for (int i=0; i<SailObjects::btPolarCount(); i++)
    {
        pBtPolar = SailObjects::btPolar(i);
        pBtPolar->setVisible(false);
    }

    emit projectModified();
    m_pBoatTreeView->setCurveParams();
    m_pBoatTreeView->update();
    m_bResetCurves = true;
    updateView();
}


void XSail::onResetBtPolar()
{
    if (!m_pCurBtPolar) return;
    QString strong = "Are you sure you want to reset the content of the polar :\n" +  m_pCurBtPolar->name() +"?\n";
    if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, "Question", strong,
                                                  QMessageBox::Yes|QMessageBox::No,
                                                  QMessageBox::Yes)) return;
//    m_bResetTextLegend = true;
//    m_pgl3dXSailView->s_bResetglLegend = true;
    m_pCurBtPolar->clearData();

    SailObjects::deleteBtPolarOpps(m_pCurBtPolar);

    m_pCurBtOpp = nullptr;

    m_pBoatTreeView->removeBtPolarBtOpps(m_pCurBtPolar);

    emit projectModified();
    resetCurves();
    updateView();
}


void XSail::onHideBtPolarOpps()
{
    if(!m_pCurBoat || !m_pCurBtPolar) return;
    for(int i=0; i<SailObjects::btOppCount(); i++)
    {
        BoatOpp *pBtOpp = SailObjects::btOpp(i);
        if(pBtOpp->boatName().compare(m_pCurBoat->name())==0 && pBtOpp->polarName().compare(m_pCurBtPolar->name())==0)
        {
            pBtOpp->setVisible(false);
        }
    }
    m_bResetCurves = true;
    updateView();
}


void XSail::onDeleteBtPolarOpps()
{
    SailObjects::deleteBtPolarOpps(m_pCurBtPolar);
    m_pCurBtOpp = nullptr;

    emit projectModified();
    updateObjectView();
    m_pBoatTreeView->selectBtPolar(m_pCurBtPolar);
    setControls();
    m_bResetCurves = true;
    updateView();
}


void XSail::onShowBtPolarOpps()
{
    if(!m_pCurBoat || !m_pCurBtPolar) return;
    for(int i=0; i<SailObjects::btOppCount(); i++)
    {
        BoatOpp *pBtOpp = SailObjects::btOpp(i);
        if(pBtOpp->boatName().compare(m_pCurBoat->name())==0 && pBtOpp->polarName().compare(m_pCurBtPolar->name())==0)
        {
            pBtOpp->setVisible(true);
        }
    }
    m_bResetCurves = true;
    updateView();
}


void XSail::onShowOnlyBtPolarOpps()
{
    if(!m_pCurBoat || !m_pCurBtPolar) return;
    for(int i=0; i<SailObjects::btOppCount(); i++)
    {
        BoatOpp *pBtOpp = SailObjects::btOpp(i);
        if(pBtOpp->boatName().compare(m_pCurBoat->name())==0 && pBtOpp->polarName().compare(m_pCurBtPolar->name())==0)
        {
            pBtOpp->setVisible(true);
        }
        else pBtOpp->setVisible(false);
    }
    m_bResetCurves = true;
    updateView();
}


void XSail::onDeleteCurBoat()
{
    if(!m_pCurBoat) return;

    QString strong;
    if(m_pCurBoat) strong = "Are you sure you want to delete the Boat :\n" +  m_pCurBoat->name() +"?\n";
    if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, "Question", strong, QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)) return;
    QString nextBoatName = m_pBoatTreeView->removeBoat(m_pCurBoat);
    SailObjects::deleteBoat(m_pCurBoat, true);

    m_pCurBoat = nullptr;
    m_pCurBtPolar = nullptr;
    m_pCurBtOpp = nullptr;

    setBoat(nextBoatName);
    m_pBoatTreeView->selectObjects();
    setControls();
    m_bResetCurves = true;

    emit projectModified();
    updateView();
}


/**
* Resets and fills the polar graphs curves with the data from the WPolar objects
*/
void XSail::createBtPolarCurves()
{
    for(int ig=0; ig<m_PlrGraph.count(); ig++)
    {
        m_PlrGraph[ig]->deleteCurves();
        m_PlrGraph[ig]->clearSelection();
    }

    for (int k=0; k<SailObjects::btPolarCount(); k++)
    {
        BoatPolar *pBoatPolar = SailObjects::btPolar(k);
        pBoatPolar->clearCurves();

        if (pBoatPolar->isVisible() && pBoatPolar->dataSize()>0)
        {
            for(int ig=0; ig<m_PlrGraph.count(); ig++)
            {
                Graph *pGraph = m_PlrGraph[ig];
                Curve *pCurve = pGraph->addCurve(pBoatPolar->boatName() + " / " + pBoatPolar->name(), AXIS::LEFTYAXIS, DisplayOptions::isDarkTheme());
                fillBtPlrCurve(pCurve, pBoatPolar, pGraph->xVariable(), pGraph->yVariable(0));
                pCurve->setTheStyle(pBoatPolar->theStyle());
                pBoatPolar->appendCurve(pCurve);
                if(pBoatPolar==m_pCurBtPolar)
                    pGraph->selectCurve(pCurve);
                if(pGraph->hasRightAxis())
                {
                    Curve *pCurve = pGraph->addCurve(pBoatPolar->boatName() + " / " + pBoatPolar->name(), AXIS::RIGHTYAXIS, DisplayOptions::isDarkTheme());
                    fillBtPlrCurve(pCurve, pBoatPolar, pGraph->xVariable(), pGraph->yVariable(1));
                    pCurve->setTheStyle(pBoatPolar->theStyle());
                    pBoatPolar->appendCurve(pCurve);
                    if(pBoatPolar==m_pCurBtPolar)
                        pGraph->selectCurve(pCurve);
                }
            }
        }
    }

    m_bResetCurves = false;

    for(int ig=0; ig<m_PlrGraph.count(); ig++)
    {
        m_PlrGraph[ig]->invalidate();
    }
    emit curvesUpdated();
}


void XSail::fillBtPlrCurve(Curve *pCurve, BoatPolar const *pBtPolar, int XVar, int YVar)
{
    pCurve->setSelectedPoint(-1);
    for (int i=0; i<pBtPolar->dataSize(); i++)
    {
        double x = pBtPolar->getVariable(XVar, i);
        double y = pBtPolar->getVariable(YVar, i);

        //Set user units
        if(XVar>=4 && XVar<=9)  x *= Units::NtoUnit(); //force
        if(YVar>=4 && YVar<=9)  y *= Units::NtoUnit(); //force
        if(XVar>=10)  x *= Units::NmtoUnit();
        if(YVar>=10)  y *= Units::NmtoUnit();

        pCurve->appendPoint(x,y);
        if(m_pCurBtOpp && Graph::isHighLighting())
        {
            if(m_pCurBoat
                    && pBtPolar->boatName()==m_pCurBoat->name()
                    && m_pCurBtOpp->polarName() ==pBtPolar->name())
            {

                if(fabs(pBtPolar->m_Ctrl.at(i)-m_pCurBtOpp->ctrl())<0.01)
                {
                    pCurve->setSelectedPoint(i);
                }
            }
        }
    }
}


void XSail::outputPanelProperties(int panelindex)
{
    if(!m_pCurBoat) return;
    QString strange, strong;

    // check index validity
    if(m_pCurBtPolar->isTriangleMethod())
    {
        if(panelindex<0 || panelindex>=m_pCurBoat->nPanel3()) return;
    }
    else if(m_pCurBtPolar->isQuadMethod())
    {
        // not activated
    }

    if(m_pCurBtPolar)
    {
        if(m_pCurBtPolar->isTriangleMethod())
        {
            Panel3 const &p3 = m_pCurBoat->panel3(panelindex);
            strange = p3.properties(true);
        }
        else if(m_pCurBtPolar->isQuadMethod())
        {
            // not activated
        }
    }

    if(m_pCurBtOpp && (XSailDisplayCtrls::s_b3dCp || XSailDisplayCtrls::s_bGamma || XSailDisplayCtrls::s_bPanelForce))
    {
        strong = QString::asprintf("  Cp   = %7.2f", m_pCurBtOpp->Cp(panelindex));
        strange += strong + "\n";
        double q = 0.5*m_pCurBtPolar->density() *m_pCurBtOpp->QInf()*m_pCurBtOpp->QInf();
        strong = QString::asprintf("  F/s  = %7.2f ", m_pCurBtOpp->Cp(panelindex)*q*Units::PatoUnit());
        strong += Units::pressureUnitLabel() +"\n";
        strange += strong + "\n";
    }
    strange +="\n\n";

    displayMessage(strange, true);
}


void XSail::outputNodeProperties(int nodeindex, double pickedval)
{
    if(!m_pCurBoat) return;
    if(!m_pCurBtPolar) return;
    if(!m_pCurBtPolar->isTriLinearMethod()) return;

    QString strange, strong;

    // check index validity
    if(m_pCurBtPolar->isTriangleMethod())
    {
        if(nodeindex<0 || nodeindex>=m_pCurBoat->nNodes()) return;
    }

    Node const &node = m_pCurBoat->triMesh().node(nodeindex);

    if(m_pCurBtPolar)
    {
        strange = node.properties() + "\n";
    }
    if(m_pCurBtOpp && (XSailDisplayCtrls::s_b3dCp || XSailDisplayCtrls::s_bGamma || XSailDisplayCtrls::s_bPanelForce))
    {
        if(XSailDisplayCtrls::s_b3dCp)
        {
            strong = QString::asprintf("   Cp = %g\n", pickedval);
            strange += strong;
        }
        else if(XSailDisplayCtrls::s_bGamma)
        {
            strong = QString::asprintf("   Mu = %g\n", pickedval);
            strange += strong;
        }
    }
    strange += "\n\n";

    s_pMainFrame->onShowLogWindow(true);
    s_pMainFrame->displayMessage(strange, false);
}


void XSail::onExportBtPolarToClipboard()
{
    if(!m_pCurBtPolar) return;
    QString polardata;
    QString sep = "  ";
    if(SaveOptions::exportFileType()==xfl::CSV) sep = SaveOptions::textSeparator()+ " ";
    m_pCurBtPolar->getBtPolarData(polardata, sep);
    QClipboard *pClipBoard = QApplication::clipboard();
    pClipBoard->setText(polardata);
}


void XSail::onExportBtPolarToFile()
{
    if (!m_pCurBtPolar) return;

    QString FileName, filter;

    if(SaveOptions::exportFileType()==xfl::TXT) filter = "Text File (*.txt)";
    else                                        filter = "Comma Separated Values (*.csv)";

    FileName = m_pCurBtPolar->name();
    FileName.replace("/", "_");
    FileName.replace(".", "_");
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export polar",
                                            SaveOptions::lastDirName() + "/"+FileName,
                                            "Text File (*.txt);;Comma Separated Values (*.csv)",
                                            &filter);

    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(FileName.left(pos));

    if(filter.indexOf("*.txt")>0)
    {
        SaveOptions::setExportFileType(xfl::TXT);
        if(FileName.indexOf(".txt")<0) FileName +=".txt";
    }
    else if(filter.indexOf("*.csv")>0)
    {
        SaveOptions::setExportFileType(xfl::CSV);
        if(FileName.indexOf(".csv")<0) FileName +=".csv";
    }

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&XFile);
    QString polardata;
    QString sep = "  ";
    if(SaveOptions::exportFileType()==xfl::CSV) sep = SaveOptions::textSeparator()+ " ";
    m_pCurBtPolar->getBtPolarData(polardata, sep);
    out << polardata;
    //    exportWPolarToTextStream(m_pCurWPolar, out, SaveOptions::exportFileType());
    XFile.close();

    updateView();
}


void XSail::onBtPolarProps()
{
    if(!m_pCurBtPolar) return;
    ObjectPropsDlg *pOPDlg = new ObjectPropsDlg(s_pMainFrame);
    QString strangeProps;

    QString lenlab, arealab, masslab, speedlab;
    Units::getLengthUnitLabel(lenlab);
    Units::getAreaUnitLabel(arealab);
    Units::getMassUnitLabel(masslab);
    Units::getSpeedUnitLabel(speedlab);

    m_pCurBtPolar->getProperties(strangeProps);

    pOPDlg->initDialog(m_pCurBoat->name()+ "/" +m_pCurBtPolar->name(), strangeProps);
    pOPDlg->show();
}


void XSail::onBtOppProps()
{
    if(!m_pCurBoat || !m_pCurBtPolar|| !m_pCurBtOpp) return;

    ObjectPropsDlg *pOPDlg = new ObjectPropsDlg(s_pMainFrame);
    QString strangeprops;
    m_pCurBtOpp->getProperties(m_pCurBoat, m_pCurBtPolar->density(), strangeprops);
    pOPDlg->initDialog(m_pCurBtOpp->title(false), strangeprops);
    pOPDlg->show();
}


void XSail::onImportBtPolarFromXML()
{
    QStringList PathNameList;
    PathNameList = QFileDialog::getOpenFileNames(s_pMainFrame, "Open XML file",
                                                 SaveOptions::xmlWPolarDirName(),
                                                 "Analysis XML file (*.xml)");
    if(!PathNameList.size()) return;

    BoatPolar *pBtPolar=nullptr;
    for(int iFile=0; iFile<PathNameList.size(); iFile++)
    {
        QFile XFile(PathNameList.at(iFile));
        pBtPolar = importBtPolarFromXML(XFile);
        if(pBtPolar)
        {
            if(pBtPolar->boatName().isEmpty())
            {
                if(m_pCurBoat)
                {
                    pBtPolar->setBoatName(m_pCurBoat->name());
                    pBtPolar->resizeSailAngles(m_pCurBoat->sailCount());
                }
            }
            else
            {
                Boat const*pBoat = SailObjects::boat(pBtPolar->boatName());
                if(pBoat)
                {
                    pBtPolar->resizeSailAngles(pBoat->sailCount());
                }
                else
                {
                    // can't do anything with this polar

                    QString strange;
                    strange = QString::asprintf("   no boat with the name %s\n", pBtPolar->boatName().toStdString().c_str());
                    displayMessage(strange, true);

                    delete pBtPolar;
                    pBtPolar = nullptr;
                }
            }
            if(pBtPolar)
            {
                SailObjects::insertBtPolar(pBtPolar);
                m_pBoatTreeView->insertBtPolar(pBtPolar);
            }
        }
    }
    if(pBtPolar)
    {
        m_pCurBtOpp = nullptr;
        setBtPolar(pBtPolar);
        m_pBoatTreeView->selectBtPolar(pBtPolar);
        emit projectModified();

        gl3dXSailView::s_bResetglBoat = true;
        gl3dXSailView::s_bResetglMesh = true;
        gl3dXSailView::resetglBtOpp();
        updateView();
    }

    displayMessage("Done importing analyses\n\n", false);
}


BoatPolar *XSail::importBtPolarFromXML(QFile &xmlFile)
{
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not open the file "+xmlFile.fileName()+"\n";
        displayMessage(strange, true);
        return nullptr;
    }

    QFileInfo fi(xmlFile);
    displayMessage("Importing xml file " + fi.fileName() + "\n", false);

    XmlBtPolarReader polarreader(xmlFile);
    polarreader.readXMLPolarFile();

    if(polarreader.hasError())
    {
        QString errorMsg = "   "+polarreader.errorString();
        QString strange = QString("   error at line %1 column %2\n").arg(polarreader.lineNumber()).arg(polarreader.columnNumber());
        displayMessage(errorMsg + "\n" + strange, true);
        return nullptr;
    }

    if(!polarreader.btPolar())
    {
        displayMessage("   no valid analysis definition found in the file\n", true);
        return nullptr;
    }

    return polarreader.btPolar();
}


void XSail::onExportBtPolarToXML()
{
    if(!m_pCurBoat || !m_pCurBtPolar) return ;// is there anything to export?

    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = m_pCurBtPolar->name();
    strong.replace("/", "_");
    strong.replace(".", "_");

    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export analysis definition to xml file",
                                            SaveOptions::xmlWPolarDirName() +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";


    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    XmlBtPolarWriter wpolarWriter(XFile);
    wpolarWriter.writeXMLBtPolar(m_pCurBtPolar);

    XFile.close();
}


void XSail::onMeshInfo()
{
    if(!m_pCurBoat) return;
    QString log;
    QString strong;

    log = m_pCurBoat->name() + "\n";

    strong = "  Triangular panels:";
    log += strong +"\n";
    m_pCurBoat->triMesh().getMeshInfo(log);

    displayMessage(log, true);
}


void XSail::onImportBoatFromXml()
{
    QStringList pathNames;
    pathNames = QFileDialog::getOpenFileNames(s_pMainFrame, "Open XML file",
                                              SaveOptions::xmlPlaneDirName(),
                                              "Plane XML file (*.xml)");
    if(!pathNames.size()) return;

    Boat *pBoat = nullptr;
    for(int iFile=0; iFile<pathNames.size(); iFile++)
    {
        QFile XFile(pathNames.at(iFile));
        pBoat = importBoatFromXML(XFile);
    }

    if(pBoat)
    {
        setBoat();
        m_pBoatTreeView->selectBoat(pBoat);
        updateObjectView();
        emit projectModified();
        setControls();
    }
    updateView();
}


/**
 * Imports the plane geometry from an XML file
 */
Boat * XSail::importBoatFromXML(QFile &xmlFile)
{
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not open the file "+xmlFile.fileName() +"\n";
        displayMessage(strange, true);
        return nullptr;
    }

    XmlBoatReader btreader(xmlFile);
    if(!btreader.readXMLBoatFile())
    {
        QString strong;
        QString errormsg;
        errormsg = "Failed to read the file "+xmlFile.fileName()+"\n";
        errormsg += "   " +btreader.errorString() + "\n";
        strong = QString::asprintf("line %d column %d\n",int(btreader.lineNumber()),int(btreader.columnNumber()));
        errormsg += "   error at " + strong;
        displayMessage(errormsg, true);
        return nullptr;
    }

    Boat *pBoat = btreader.boat();

    if(!pBoat)
    {
        QString errormsg;
        errormsg = "No boat definition found in the file "+xmlFile.fileName();
        displayMessage(errormsg, true);
        return nullptr;
    }
    else
    {
        displayMessage("Boat "+pBoat->name()+" imported successfully\n", false);
    }

    if(SailObjects::boatExists(pBoat->name())) m_pCurBoat = setModifiedBoat(pBoat);
    else                                       m_pCurBoat = SailObjects::appendBoat(pBoat);
    m_pBoatTreeView->insertBoat(pBoat);

    return pBoat;
}


void XSail::onExportToXML()
{
    if(!m_pCurBoat)return ;// is there anything to export?


    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = m_pCurBoat->name().trimmed();
    //    strong.replace(' ', '_');
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export to xml file",
                                            SaveOptions::xmlPlaneDirName() +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    XmlBoatWriter boatwriter(XFile);
    boatwriter.writeXMLBoat(*m_pCurBoat);

    XFile.close();
}


void XSail::onExportBtOppToFile()
{
    if(!m_pCurBtOpp)return ;// is there anything to export?

    xfl::enumTextFileType exporttype;
    QString filter;
    if(SaveOptions::exportFileType()==xfl::TXT) filter = "Text File (*.txt)";
    else                                        filter = "Comma Separated Values (*.csv)";

    QString FileName, sep, str, strong, strange;

    strong = QString("a=%1_v=%2").arg(m_pCurBtOpp->ctrl(), 5,'f',2).arg(m_pCurBtOpp->QInf()*Units::mstoUnit(), 6,'f',2);
    Units::getSpeedUnitLabel(str);
    strong = m_pCurBtOpp->boatName()+"_"+strong+str;

    strong.replace(" ","");
    strong.replace("/", "");
    FileName = QFileDialog::getSaveFileName(s_pMainFrame, "Export OpPoint",
                                            SaveOptions::lastDirName() +'/'+strong,
                                            "Text File (*.txt);;Comma Separated Values (*.csv)",
                                            &filter);

    if(!FileName.length()) return;
    int pos = FileName.lastIndexOf("/");
    if(pos>0) SaveOptions::setLastDirName(FileName.left(pos));
    pos = FileName.lastIndexOf(".csv");
    if (pos>0) SaveOptions::setExportFileType(xfl::CSV);
    else       SaveOptions::setExportFileType(xfl::TXT);
    exporttype = SaveOptions::exportFileType();


    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);

    sep = SaveOptions::textSeparator();

    m_pCurBtOpp->exportMainDataToString(m_pCurBoat, strange, SaveOptions::exportFileType(), SaveOptions::textSeparator());
    out <<strange;

    m_pCurBtOpp->exportPanel3DataToString(m_pCurBoat, exporttype, strange);
    out <<strange;

    out << ("\n\n");

    XFile.close();
}


void XSail::onExportBtOppToClipboard()
{
    if (!m_pCurBtOpp) return;

    QClipboard *pClipBoard = QApplication::clipboard();
    QString strange, strong;
    m_pCurBtOpp->exportMainDataToString(m_pCurBoat, strange, SaveOptions::exportFileType(), SaveOptions::textSeparator());

    m_pCurBtOpp->exportPanel3DataToString(m_pCurBoat, SaveOptions::exportFileType(), strong);

    strange += "\n\n" + strong;
    pClipBoard->setText(strange);
    displayMessage("The data of the operating point has been copied to the clipboard", false);
}


void XSail::onEditBtPolarPts()
{
    if(!m_pCurBoat || !m_pCurBtPolar) return;

    BoatPolar MemBtPolar;
    MemBtPolar.copy(m_pCurBtPolar);
    //Edit the current WPolar data


    EditPlrDlg epDlg(s_pMainFrame);
    epDlg.initDialog(nullptr, nullptr, m_pCurBtPolar);

    connect(&epDlg, SIGNAL(dataChanged()), this, SLOT(onResetBtPolarCurves()));

    Line::enumPointStyle iPoints = m_pCurBtPolar->pointStyle();
    m_pCurBtPolar->setPointStyle(Line::NOSYMBOL);

    m_bResetCurves = true;
    updateView();

    if(epDlg.exec() == QDialog::Accepted)
    {
        emit projectModified();
    }
    else
    {
        m_pCurBtPolar->copy(&MemBtPolar);
    }
    m_pCurBtPolar->setPointStyle(iPoints);

    m_bResetCurves = true;
    updateView();
    setControls();
}


void XSail::onResetBtPolarCurves()
{
    m_bResetCurves=true;
    updateView();
}


void XSail::onExportAllBtPolars()
{
    QString filename, DirName, polarname;
    QFile XFile;
    QTextStream out;

    //select the directory for output
    DirName = QFileDialog::getExistingDirectory(s_pMainFrame, "Export Directory", SaveOptions::lastDirName());

    BoatPolar *pWPolar;
    for(int l=0; l<SailObjects::btPolarCount(); l++)
    {
        pWPolar = SailObjects::btPolar(l);
        polarname = pWPolar->boatName() + "_" + pWPolar->name();
        polarname.replace("/", "_");
        polarname.replace(".", "_");
        filename = polarname;
        filename = DirName + "/" +filename;
        if(SaveOptions::exportFileType()==xfl::TXT) filename += ".txt";
        else                                          filename += ".csv";

        XFile.setFileName(filename);
        if (XFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            out.setDevice(&XFile);

            QString sep = "  ";
            if(SaveOptions::exportFileType()==xfl::CSV) sep = SaveOptions::textSeparator()+ " ";
            QString data, speedlab;
            Units::getSpeedUnitLabel(speedlab);
            pWPolar->getBtPolarData(data, sep);
            out << data;
            XFile.close();
            displayMessage("Exported the polar:" + polarname + "\n", false);
        }
        else
        {
            displayMessage("Error exporting the polar:" + polarname + "\n", true);
            s_pMainFrame->onShowLogWindow(true);
        }
    }
}


void XSail::enableObjectView(bool bEnable)
{
    m_pBoatTreeView->setEnabled(bEnable);
}


void XSail::cancelDisplayThreads()
{
    m_pgl3dXSailView->onCancelThreads();
}


void XSail::onManageBoats()
{
}


void XSail::onOpen3dViewInNewWindow()
{
}


void XSail::onConnectTriangles()
{
    if(!m_pCurBoat) return;
    QString log("\nMaking triangle connections\n");
    QElapsedTimer t; t.start();
//    m_pCurBoat->refTriMesh().makeConnectionsFromNodePosition(false, xfl::isMultiThreaded());
    m_pCurBoat->makeConnections(); // avoids the connection of one sail to another

    QVector<int>errorlist;
    if(!m_pCurBoat->refTriMesh().connectTrailingEdges(errorlist))
        log += "   Error connecting trailing edges.\n";

    log += QString::asprintf("   Triangle connections done in %d ms\n\n", int(t.elapsed()));
    m_pCurBoat->restoreMesh();
    s_pMainFrame->displayMessage(log, false);
}


void XSail::onCheckPanels()
{
    m_pgl3dXSailView->clearHighlightList();

    if(!m_pCurBtPolar || !m_pCurBoat) return;

    m_pgl3dXSailView->m_bMeshPanels = true;
    m_pgl3dControls->m_pXSailDisplayCtrls->m_pchPanels->setChecked(m_pgl3dXSailView->m_bMeshPanels);
    XSailDisplayCtrls::s_b3dCp  = false;
    XSailDisplayCtrls::s_bGamma = false;
    m_pgl3dXSailView->m_pDisplayCtrls->m_pchCp->setChecked(false);
//    m_pgl3dXSailView->m_pBtResults3dCtrls->m_pchGamma->setChecked(false);

    QString log = "Checking panels\n";
    s_pMainFrame->displayMessage(log, false);
    log.clear();
    PanelCheckDlg dlg(m_pCurBtPolar->isQuadMethod());
    int res = dlg.exec();
    bool bCheck = dlg.checkSkinny() || dlg.checkMinArea() || dlg.checkMinAngles() || dlg.checkMinQuadWarp();
    if(res!=QDialog::Accepted)
    {
        m_pgl3dXSailView->s_bResetglMesh = true;
        return;
    }

    QVector<int> highpanellist = dlg.panelIndexes();
    m_pgl3dXSailView->appendHighlightList(highpanellist);
    for(int i3=0; i3<highpanellist.size(); i3++)
    {
        outputPanelProperties(highpanellist.at(i3));
    }

    QVector<int> highnodelist = dlg.nodeIndexes();
//    m_pgl3dXSailView->appendHighlightList(highpanellist);
    for(int in=0; in<highnodelist.size(); in++)
    {
        outputNodeProperties(highnodelist.at(in),0.0);
    }
    s_pMainFrame->onShowLogWindow(true);

    if(!bCheck)
    {
        m_pgl3dXSailView->s_bResetglMesh = true;
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);


    // check Triangles
    if(m_pCurBtPolar->isTriangleMethod())
    {
        QVector<int> skinnylist, anglelist, arealist, sizelist;

        m_pCurBoat->triMesh().checkPanels(log, dlg.checkSkinny(), dlg.checkMinAngles(), dlg.checkMinArea(), dlg.checkMinSize(),
                                           skinnylist, anglelist, arealist, sizelist,
                                           PanelCheckDlg::qualityFactor(), PanelCheckDlg::minAngle(), PanelCheckDlg::minArea(), PanelCheckDlg::minSize());
        if(dlg.checkSkinny())    m_pgl3dXSailView->appendHighlightList(skinnylist);
        if(dlg.checkMinAngles()) m_pgl3dXSailView->appendHighlightList(anglelist);
        if(dlg.checkMinArea())   m_pgl3dXSailView->appendHighlightList(arealist);
        if(dlg.checkMinSize())   m_pgl3dXSailView->appendHighlightList(sizelist);
    }

    // check Quads
    if(m_pCurBoat && m_pCurBtPolar->isQuadMethod())
    {
        // not activated
    }

    s_pMainFrame->displayMessage(log + "\n\n", false);

    m_pgl3dXSailView->s_bResetglMesh = true;
    updateView();
    QApplication::restoreOverrideCursor();
}


void XSail::onShowNormals()
{
    m_pgl3dXSailView->m_bNormals = m_pActions->m_pShowNormals->isChecked();
    if(m_pgl3dXSailView->m_bNormals) m_pgl3dXSailView->s_bResetglMesh=true;
    m_pgl3dXSailView->update();
}


void XSail::cancelStreamLines()
{
    m_pgl3dXSailView->m_pDisplayCtrls->s_bStreamLines = false;
}


bool XSail::isAnalysisRunning() const
{
    if(m_pBtAnalysisDlg) return m_pBtAnalysisDlg->isAnalysisRunning();
    return false;
}


Boat* XSail::setModifiedBoat(Boat *pModBoat)
{
    if(!pModBoat) return nullptr;
    Boat * pBoat=nullptr;
    bool bExists = true;

    QString OldName = pModBoat->name();

    RenameDlg renDlg(s_pMainFrame);
    renDlg.initDialog(pModBoat->name(), SailObjects::boatNames(), "Enter the new name for the boat:");

    while (bExists)
    {
        int resp = renDlg.exec();
        if(resp==QDialog::Accepted)
        {
            if (OldName == renDlg.newName()) return pModBoat;

            //Is the new name already used?
            bExists = false;
            for (int k=0; k<SailObjects::boatCount(); k++)
            {
                pBoat = SailObjects::boat(k);
                if (pBoat->name() == renDlg.newName())
                {
                    bExists = true;
                    break;
                }
            }

            if(!bExists)
            {
                // we have a valid name
                // rename the Boat
                pModBoat->setName(renDlg.newName());

                bool bInserted = false;
                for (int l=0; l<SailObjects::boatCount();l++)
                {
                    pBoat = SailObjects::boat(l);
                    if(pBoat == pModBoat)
                    {
                        // remove the current Boat from the array
                        SailObjects::s_oaBoat.removeAt(l);
                        // but don't delete it !
                        break;
                    }
                }
                //and re-insert it
                for (int l=0; l<SailObjects::boatCount();l++)
                {
                    pBoat = SailObjects::boat(l);
                    if(pBoat->name().compare(pModBoat->name(), Qt::CaseInsensitive) >0)
                    {
                        //then insert before
                        SailObjects::insertThisBoat(l, pModBoat);
                        bInserted = true;
                        break;
                    }
                }
                if(!bInserted)    SailObjects::appendBoat(pModBoat);
                break;

            }
        }
        else if(resp ==10)
        {
            //the user wants to overwrite the old Boat/wing
            pBoat = SailObjects::boat(renDlg.newName());
            if(pBoat==m_pCurBoat)
            {
                m_pCurBtPolar=nullptr;
                m_pCurBtOpp = nullptr;
            }

            SailObjects::deleteBoatResults(m_pCurBoat, false);
            SailObjects::deleteBoat(pBoat, false);

            pModBoat->setName(renDlg.newName());

            //place the Boat in alphabetical order in the array
            //remove the current Boat from the array
            for (int l=0; l<SailObjects::boatCount(); l++)
            {
                pBoat = SailObjects::boat(l);
                if(pBoat == pModBoat)
                {
                    SailObjects::s_oaBoat.removeAt(l);
                    // but don't delete it !
                    break;
                }
            }
            //and re-insert it
            bool bInserted = false;
            for (int l=0; l<SailObjects::boatCount(); l++)
            {
                pBoat = SailObjects::boat(l);
                if(pModBoat->name().compare(pBoat->name(), Qt::CaseInsensitive) <0)
                {
                    //then insert before
                    SailObjects::insertThisBoat(l, pModBoat);
                    bInserted = true;
                    break;
                }
            }
            if(!bInserted) SailObjects::appendBoat(pModBoat);
            bExists = false;
        }
        else
        {
            return nullptr;//cancelled
        }
    }
    return pModBoat;
}



void XSail::displayMessage(QString const &msg, bool bShow)
{
    s_pMainFrame->displayMessage(msg, bShow);
}
