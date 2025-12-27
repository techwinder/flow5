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


#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QFileInfo>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QSplitter>


#include <BRepBuilderAPI_Transform.hxx>
#include <BRepTools.hxx>
#include <Interface_Static.hxx>
#include <STEPControl_Writer.hxx>
#include <Standard_NoSuchObject.hxx>
#include <StdFail_NotDone.hxx>
#include <TopoDS_Shape.hxx>
#include <UnitsAPI.hxx>

#include "cadexportdlg.h"
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <core/saveoptions.h>


int CADExportDlg::s_ExportIndex=0;
QByteArray CADExportDlg::s_Geometry;


CADExportDlg::CADExportDlg(QWidget*pParent) : QDialog(pParent)
{
    makeCommonWts();

    m_PartName = QString("Part_Name");
}


void CADExportDlg::init(TopoDS_ListOfShape const & listofshape, QString partname)
{
    setupLayout();

    m_PartName = partname.trimmed();
    m_PartName.replace(' ', '_');

    //OCC assumes internal dimensions are mm, so scale by a factor 1000 before exporting
    //better way would be to change default units in OCC modules
    gp_Trsf Scale;
    Scale.SetScale(gp_Pnt(0.0,0.0,0.0), 1000.0);
    BRepBuilderAPI_Transform thescaler(Scale);
    TopoDS_ListIteratorOfListOfShape iterator;
    for (iterator.Initialize(listofshape); iterator.More(); iterator.Next())
    {
        thescaler.Perform(iterator.Value(), Standard_True);
        m_ShapesToExport.Append(thescaler.Shape());
    }
}


void CADExportDlg::init(const TopoDS_Shape &shape, QString partname)
{
    setupLayout();

    m_PartName = partname.trimmed();
    m_PartName.replace(' ', '_');

    if(shape.IsNull())
    {
        updateOutput("Shape is null - cannot export.\n"
                     "Wing case: check that trailing edges are closed.");
        m_ppbExport->setEnabled(false);
        return;
    }

    //OCC assumes internal dimensions are mm, so scale by a factor 1000 before exporting
    gp_Trsf Scale;
    Scale.SetScale(gp_Pnt(0.0,0.0,0.0), 1000.0);
    try {
        BRepBuilderAPI_Transform thescaler(Scale);
        thescaler.Perform(shape, Standard_True);
        m_ShapesToExport.Append(thescaler.Shape());

    }  catch (StdFail_NotDone &) {
        updateOutput("Error setting export unit to mm: StdFail_NotDone\n");
        m_ppbExport->setEnabled(false);
        return;
    }  catch (Standard_NoSuchObject &) {
        updateOutput("Error setting export unit to mm: Standard_NoSuchObject\n");
        m_ppbExport->setEnabled(false);
        return;
    }  catch (...) {
        updateOutput("Error setting export unit to mm: Something unexpected happened....\n");
        m_ppbExport->setEnabled(false);
        return;
    }
}


void CADExportDlg::makeCommonWts()
{
    m_pfrControls = new QFrame;
    {
        QVBoxLayout*pControlsLayout = new QVBoxLayout;
        {
            QHBoxLayout *pFormatSelLayout = new QHBoxLayout;
            {
                m_prbBRep = new QRadioButton("BRep");
                m_prbSTEP = new QRadioButton("STEP");
                pFormatSelLayout->addStretch();
                pFormatSelLayout->addWidget(m_prbBRep);
                pFormatSelLayout->addWidget(m_prbSTEP);
                pFormatSelLayout->addStretch();
                m_prbSTEP->setChecked(true);
                connect(m_prbBRep, SIGNAL(clicked(bool)), SLOT(onFormat()));
                connect(m_prbSTEP, SIGNAL(clicked(bool)), SLOT(onFormat()));
            }

            QLabel *pLab = new QLabel("Select STEP Format:");
            m_plwListFormat = new QListWidget;
            {
                QString tip("<p>OpenCascade documentation:<br>"
                            "Gives you the choice of translation mode for an Open CASCADE shape that is being translated to STEP."
                            "<ul>"
                            "  <li>STEPControl_AsIs translates an Open CASCADE shape to its highest possible STEP representation./li>"
                            "  <li>STEPControl_ManifoldSolidBrep translates an Open CASCADE shape to a STEP manifold_solid_brep or brep_with_voids entity./li>"
                            "  <li>STEPControl_FacetedBrep translates an Open CASCADE shape into a STEP faceted_brep entity./li>"
                            "  <li>STEPControl_ShellBasedSurfaceModel translates an Open CASCADE shape into a STEP shell_based_surface_model entity.</li>"
                            "  <li>STEPControl_GeometricCurveSet translates an Open CASCADE shape into a STEP geometric_curve_set entity.</li>"
                            "</ul>"
                            "</p>");
                m_plwListFormat->setToolTip(tip);
                QStringList formats = {"STEPControl_AsIs", "STEPControl_ManifoldSolidBrep",
                                       "STEPControl_BrepWithVoids", "STEPControl_FacetedBrep",
                                       "STEPControl_FacetedBrepAndBrepWithVoids","STEPControl_ShellBasedSurfaceModel",
                                       "STEPControl_GeometricCurveSet",    "STEPControl_Hybrid"};
                m_plwListFormat->addItems(formats);
                m_plwListFormat->setCurrentRow(0);
            }
            m_ppbExport = new QPushButton("Export");
            connect(m_ppbExport, SIGNAL(clicked(bool)), SLOT(onExport()));

            pControlsLayout->addLayout(pFormatSelLayout);

            pControlsLayout->addWidget(pLab);
            pControlsLayout->addWidget(m_plwListFormat);
            pControlsLayout->addWidget(m_ppbExport);
        }
        m_pfrControls->setLayout(pControlsLayout);
    }
    m_ppto = new PlainTextOutput;

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    {
        QPushButton *ppbClearOutput = new QPushButton("Clear output");
        connect(ppbClearOutput, SIGNAL(clicked(bool)), m_ppto, SLOT(clear()));
        m_pButtonBox->addButton(ppbClearOutput, QDialogButtonBox::ActionRole);
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }
}


void CADExportDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pfrControls);
        pMainLayout->addWidget(m_ppto);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void CADExportDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)       accept();
}


void CADExportDlg::onFormat()
{
    m_plwListFormat->setEnabled(m_prbSTEP->isChecked());
    if     (m_prbSTEP->isChecked()) m_ppbExport->setText("Export STEP");
    else if(m_prbBRep->isChecked()) m_ppbExport->setText("Export BRep");
}


void CADExportDlg::onExport()
{
    exportShapes();
    m_pButtonBox->setFocus();
}


void CADExportDlg::exportShapes()
{
    if(m_ShapesToExport.IsEmpty())
    {
        updateOutput("Nothing to export");
        return;
    }

    //    QString unit = m_pExportUnit->currentText();
    // Tell OCC that all dimensions are in meters
//    UnitsAPI::SetLocalSystem(UnitsAPI_SI);

    if     (m_prbBRep->isChecked()) exportBRep();
    else if(m_prbSTEP->isChecked()) exportSTEP();
}


void CADExportDlg::exportSTEP()
{
    QString filter = "STEP Files (*.step)";
    QString filename = SaveOptions::CADDirName()+QDir::separator()+m_PartName+".step";
    filename = QFileDialog::getSaveFileName(this, "Export STEP file",filename,filter);
    if(!filename.length())
    {
        return;
    }
    QFileInfo fi(filename);
    if(fi.suffix().isNull())
    {
        filename +=".step";
        fi.setFile(filename);
    }

    // inform OCC that internal units are meters
    STEPControl_Writer aWriter;

    STEPControl_StepModelType aValue = STEPControl_AsIs;

    QModelIndex index = m_plwListFormat->currentIndex();

    // set the units after the writer is created
//qDebug("%s",UnitsAPI::CurrentUnit("LENGTH"));
//UnitsAPI::SetCurrentUnit("LENGTH","meter");
//qDebug()<<UnitsAPI::CurrentUnit("LENGTH");
//Interface_Static::SetCVal("write.step.unit", "M");
//qDebug() << Interface_Static::SetIVal("write.step.unit", 0);
//qDebug()    <<"exportSTEP"<<Interface_Static::CVal("write.step.unit")<<Interface_Static::IVal("write.step.unit")<<Interface_Static::RVal("write.step.unit");

    switch(index.row())
    {
        case 0:
            aValue = STEPControl_AsIs;
            break;
        case 1:
            aValue = STEPControl_ManifoldSolidBrep;
            break;
        case 2:
            aValue = STEPControl_BrepWithVoids;
            break;
        case 3:
            aValue = STEPControl_FacetedBrep;
            break;
        case 4:
            aValue = STEPControl_FacetedBrepAndBrepWithVoids;
            break;
        case 5:
            aValue = STEPControl_ShellBasedSurfaceModel;
            break;
        case 6:
            aValue = STEPControl_GeometricCurveSet;
            break;
        case 7:
            aValue = STEPControl_Hybrid;
            break;
        default:
            aValue = STEPControl_AsIs;
            break;
    }

    std::stringstream buffer;
    std::streambuf *originalBuffer = std::cout.rdbuf(buffer.rdbuf());

    //    aWriter.Transfer(solid, STEPControl_AsIs);
    //    aWriter.Write("cylinder.step");

    int nShapes=0;
    TopoDS_ListIteratorOfListOfShape iterator;
    TopoDS_Shape exportshape;
    for (iterator.Initialize(m_ShapesToExport); iterator.More(); iterator.Next())
    {

        //OCC assumes internal dimensions are mm, so scale by a factor 1000 before exporting
        gp_Trsf Scale;
        Scale.SetScale(gp_Pnt(0.0,0.0,0.0), 1000.0);
        try {
            BRepBuilderAPI_Transform thescaler(Scale);
            thescaler.Perform(iterator.Value(), Standard_True);
            exportshape = thescaler.Shape();

        }  catch (StdFail_NotDone &) {
            updateOutput("Error scaling the model: StdFail_NotDone\n");
             return;
        }  catch (Standard_NoSuchObject &) {
            updateOutput("Error scaling the model: Standard_NoSuchObject\n");
            return;
        }  catch (...) {
            updateOutput("Error scaling the model: Something unexpected happened....\n");
            return;
        }
        if(exportshape.IsNull())
        {
            updateOutput("exportshape is null - cannot export.");
            return;
        }


        switch(aWriter.Transfer(exportshape, aValue))
        {
            case IFSelect_RetVoid:
                updateOutput("Normal execution - created nothing or no data to process\n");
                return;
            case IFSelect_RetError:
                updateOutput("Error in command or input data, no execution\n");
                return;
            case IFSelect_RetFail:
                updateOutput("Execution was run and has failed\n");
                return;
            case IFSelect_RetStop:
                updateOutput("End or stop (such as Raise)\n");
                return;
            default:
                break;
        }
        nShapes++;
    }


    aWriter.Write(filename.toStdString().c_str());

    QString strong;
    // buffer.str() contains now the output from STEPControl
    std::cout.rdbuf(originalBuffer);
    strong = QString::fromStdString(buffer.str()) + "\n";

    updateOutput(strong);

    strong = QString::asprintf("Exported %d shape(s) to file %s", nShapes, fi.fileName().toStdString().c_str());
    updateOutput(strong);
}


void CADExportDlg::exportBRep()
{
    QString filter = "BRep Files (*.brep)";
    QString filename = SaveOptions::CADDirName()+QDir::separator()+m_PartName+".brep";
    filename = QFileDialog::getSaveFileName(this, "Export BRep file",filename,filter);
    if(!filename.length())
    {
        return;
    }
    QFileInfo fi(filename);
    if(fi.suffix().isNull())
    {
        filename +=".brep";
        fi.setFile(filename);
    }

    std::ofstream brepfile;
    brepfile.open(filename.toStdString());
    if(brepfile.is_open())
    {
        TopoDS_ListIteratorOfListOfShape iterator;
        for (iterator.Initialize(m_ShapesToExport); iterator.More(); iterator.Next())
        {
            BRepTools::Write(iterator.Value(), brepfile);
        }
        brepfile.close();
    }
    QString strong;
    strong = QString::asprintf("Exported shape(s) to file %s", fi.fileName().toStdString().c_str());
    updateOutput(strong);
}

void CADExportDlg::updateStdOutput(std::string const &strong)
{
    m_ppto->onAppendStdText(strong);
}

void CADExportDlg::updateOutput(QString const &strong)
{
    m_ppto->onAppendQText(strong);
}


void CADExportDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void CADExportDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void CADExportDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("CADExportDlg");
    {
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void CADExportDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("CADExportDlg");
    {
        settings.setValue("WindowGeom", s_Geometry);
    }
    settings.endGroup();
}


