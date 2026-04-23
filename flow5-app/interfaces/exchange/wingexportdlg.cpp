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


#include <QVBoxLayout>


#include <BRepBuilderAPI_Transform.hxx>
//#include <StdFail_NotDone.hxx>
#include <Standard_NoSuchObject.hxx>


#include "wingexportdlg.h"

#include <api/units.h>
#include <api/wingxfl.h>
#include <api/occ_globals.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>

int WingExportDlg::s_SurfaceType=0;
int WingExportDlg::s_iChordRes=37;

double WingExportDlg::s_StitchPrecision(1.e-4);
int WingExportDlg::s_SplineDegree(3);
int WingExportDlg::s_nSplineCtrlPts(11);


WingExportDlg::WingExportDlg(QWidget *pParent) : CADExportDlg(pParent)
{
    setWindowTitle("Wing export");
    setupLayout();
}


void WingExportDlg::setupLayout()
{
    QFrame *pWingFrame = new QFrame;
    {
       QVBoxLayout *pWingLayout = new QVBoxLayout;
       {
            QHBoxLayout *pExportTypeLayout = new QHBoxLayout;
            {
                QLabel *pLabType = new QLabel("Wings surfaces as:");
                m_prbFacets = new QRadioButton("Facets");
                m_prbNURBS  = new QRadioButton("NURBS");
                m_prbNURBS->setToolTip("flow5 will create one NURBS for each top and bottom surface between two wing sections");
                m_prbSwept  = new QRadioButton("Swept splines");
                m_prbSwept->setToolTip("flow5 will first convert wing sections to splines,<br>then create a swept surface between the splines");

                m_prbFacets->setChecked(s_SurfaceType==0);
                m_prbNURBS->setChecked(s_SurfaceType==1);
                m_prbSwept->setChecked(s_SurfaceType==2);

                connect(m_prbFacets, SIGNAL(clicked(bool)), SLOT(onExportType()));
                connect(m_prbNURBS,  SIGNAL(clicked(bool)), SLOT(onExportType()));
                connect(m_prbSwept,  SIGNAL(clicked(bool)), SLOT(onExportType()));

                pExportTypeLayout->addStretch();
                pExportTypeLayout->addWidget(pLabType);
                pExportTypeLayout->addWidget(m_prbFacets);
                pExportTypeLayout->addWidget(m_prbNURBS);
                pExportTypeLayout->addWidget(m_prbSwept);
                pExportTypeLayout->addStretch();
            }

            QGridLayout *pCommonLayout = new QGridLayout;
            {
                QLabel *plabRes = new QLabel("Chordwise points");
                m_pieChordRes = new IntEdit(s_iChordRes);
                m_pieChordRes->setToolTip("This parameter defines the discretization level of each wing section before it is used "
                                          "to build facets or as the base points on which the splines are built.<br>"
                                          "<b>flow5 default setting:</b> 30");

                QLabel *plabStitch   = new QLabel("Stitch precision:");
                m_pdeStitchPrecision = new FloatEdit(s_StitchPrecision*Units::mtoUnit());
                m_pdeStitchPrecision->setToolTip("<b>OpenCascade documentation:</b><br>"
                                                 "The working tolerance defines the maximal distance between topological elements "
                                                 "which can be sewn. It is not ultimate that such elements will be actually sewn "
                                                 "as many other criteria are applied to make the final decision.<br>"
                                                 "<b>flow5 default setting:</b> &le; 0.1 mm");
                QLabel *plabLen      = new QLabel(Units::lengthUnitQLabel());

                QString tip("These parameters control the end splines on which the NURBS or swept surfaces are built.<br>"
                            "The splines are constructed as approximations of the wing's chordwise points.<br>"
                            "<b>flow5 default settings:</b><br>"
                            "degree = 3<br>"
                            "nbr. of ctrl points = 11");
                QLabel *plabDegree   = new QLabel("Spline degree:");
                m_pieSplineDegre     = new IntEdit(s_SplineDegree);
                m_pieSplineDegre->setToolTip(tip);
                QLabel *plabCtrl     = new QLabel("Nbr. of spline ctrl points:");
                m_pieSplineCtrlPts   = new IntEdit(s_nSplineCtrlPts);
                m_pieSplineCtrlPts->setToolTip(tip);

                pCommonLayout->addWidget(plabRes,               1, 1);
                pCommonLayout->addWidget(m_pieChordRes,         1, 2);
                pCommonLayout->addWidget(plabStitch,            2, 1);
                pCommonLayout->addWidget(m_pdeStitchPrecision,  2, 2);
                pCommonLayout->addWidget(plabLen,               2, 3);

                pCommonLayout->addWidget(plabDegree,            3, 1);
                pCommonLayout->addWidget(m_pieSplineDegre,      3, 2);
                pCommonLayout->addWidget(plabCtrl,              4, 1);
                pCommonLayout->addWidget(m_pieSplineCtrlPts,    4, 2);

                pCommonLayout->setColumnStretch(4,1);
            }


            pWingLayout->addLayout(pExportTypeLayout);
            pWingLayout->addLayout(pCommonLayout);
       }
        pWingFrame->setLayout(pWingLayout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(pWingFrame);
        pMainLayout->addWidget(m_pfrControls);
        pMainLayout->addWidget(m_ppto);
        pMainLayout->addWidget(m_pButtonBox);

//        pMainLayout->setStretchFactor(pWingFrame,   1);
        pMainLayout->setStretchFactor(m_pfrControls, 1);
        pMainLayout->setStretchFactor(m_ppto,    1);
    }
    setLayout(pMainLayout);
}


void WingExportDlg::onExportType()
{
    if(m_prbFacets->isChecked())
    {
        s_SurfaceType=0;
    }
    else if(m_prbNURBS->isChecked())
    {
        s_SurfaceType=1;
    }
    else if(m_prbSwept->isChecked())
    {
        s_SurfaceType=2;
    }
    m_pieSplineDegre->setEnabled(s_SurfaceType>0);
    m_pieSplineCtrlPts->setEnabled(s_SurfaceType>0);
    readParams();
}


void WingExportDlg::hideEvent(QHideEvent*pEvent)
{
    onExportType();
    CADExportDlg::hideEvent(pEvent);
}


void WingExportDlg::init(WingXfl const*pWing)
{
    m_PartName = QString::fromStdString(pWing->name());
    m_pWing = pWing;

    onExportType();
}


void WingExportDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("WingExportDlg");
    {
        s_SurfaceType     = settings.value("SurfaceType",     s_SurfaceType).toInt();
        s_iChordRes       = settings.value("iChordRes",       s_iChordRes).toInt();
        s_SplineDegree    = settings.value("SplineDegree",    s_SplineDegree).toInt();
        s_nSplineCtrlPts  = settings.value("SplineCtrlPts",   s_nSplineCtrlPts).toInt();
        s_StitchPrecision = settings.value("StitchPrecision", s_StitchPrecision).toDouble();
    }
    settings.endGroup();
}


void WingExportDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("WingExportDlg");
    {
        settings.setValue("SurfaceType",     s_SurfaceType);
        settings.setValue("iChordRes",       s_iChordRes);
        settings.setValue("SplineDegree",    s_SplineDegree);
        settings.setValue("SplineCtrlPts",   s_nSplineCtrlPts);
        settings.setValue("StitchPrecision", s_StitchPrecision);
    }
    settings.endGroup();
}


void WingExportDlg::readParams()
{
    s_iChordRes = m_pieChordRes->value();

    s_SplineDegree    = m_pieSplineDegre->value();
    s_nSplineCtrlPts  = m_pieSplineCtrlPts->value();
    s_StitchPrecision = m_pdeStitchPrecision->value()/Units::mtoUnit();
}


void WingExportDlg::exportShapes()
{
    std::string logmsg;
    TopoDS_Shape wingshape;

    readParams();

    setFocus();
    onExportType();

    WingXfl ExportWing(*m_pWing);
    for(int i=0; i<ExportWing.nSections(); i++)
    {
        ExportWing.setNXPanels(i, s_iChordRes);
        ExportWing.setXPanelDist(i, xfl::COSINE);
    }
    ExportWing.createSurfaces(Vector3d(), 0.0, 0.0);

    m_ShapesToExport.Clear();

    if      (s_SurfaceType==0) occ::makeWingShape(      &ExportWing, s_StitchPrecision, wingshape, logmsg);
    else if (s_SurfaceType==1) occ::makeWing2NurbsShape(&ExportWing, s_StitchPrecision, s_SplineDegree, s_nSplineCtrlPts, s_iChordRes, wingshape, logmsg);
    else if (s_SurfaceType==2)
//            makeWingSplineSweepSolid(&ExportWing, 1.e-4, wingshape, logmsg);
        occ::makeWingSplineSweep(&ExportWing, s_StitchPrecision, s_SplineDegree, s_nSplineCtrlPts, s_iChordRes, wingshape, logmsg);
//        makeWingSplineSweepMultiSections(&ExportWing, 1.e-4, wingshape, logmsg);

    updateStdOutput(logmsg+"\n");

    if(!wingshape.IsNull())
        m_ShapesToExport.Append(wingshape);

    


    if(m_ShapesToExport.IsEmpty())
    {
        updateOutput("Nothing to export.\n");
        return;
    }

    if     (m_prbBRep->isChecked()) exportBRep();
    else if(m_prbSTEP->isChecked()) exportSTEP();
}



