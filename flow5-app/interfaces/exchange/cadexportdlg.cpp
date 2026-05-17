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

#include "cadexportdlg.h"
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <core/saveoptions.h>

#include <api/flow5-io.h>

int CADExportDlg::s_ExportIndex=0;
QByteArray CADExportDlg::s_Geometry;


CADExportDlg::CADExportDlg(QWidget*pParent) : QDialog(pParent)
{
    makeCommonWts();

    m_PartName = QString("Part_Name");
}


void CADExportDlg::init(TopoDS_ListOfShape const & listofshape, const QString &partname)
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


void CADExportDlg::init(const TopoDS_Shape &shape, const QString &partname)
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
        QPushButton *ppbClear = new QPushButton(tr("Clear output"));
        ppbClear->setToolTip(tr("<p>Clears the text output</p>"));
        connect(ppbClear, SIGNAL(clicked(bool)), m_ppto, SLOT(clear()));
        m_pButtonBox->addButton(ppbClear, QDialogButtonBox::ActionRole);
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
//    Tell OCC that all dimensions are in meters
//    UnitsAPI::SetLocalSystem(UnitsAPI_SI);

    QString logmsg;
    if     (m_prbBRep->isChecked())
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

        io::exportBRep(filename, m_ShapesToExport, logmsg);
        updateOutput(logmsg);
    }
    else if(m_prbSTEP->isChecked())
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

        QModelIndex index = m_plwListFormat->currentIndex();
        io::exportSTEP(filename, m_ShapesToExport, index.row(), logmsg);
        updateOutput(logmsg);
    }
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


