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


#include <QFileDialog>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

#include "stlreaderdlg.h"

#include <core/flow5events.h>
#include <core/qunits.h>
#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>

#include <api/units.h>
#include <api/flow5-io.h>
#include <flow5-io.h>

int StlReaderDlg::s_LengthUnitIndex = 0;
QByteArray StlReaderDlg::s_Geometry;

StlReaderDlg::StlReaderDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("External mesh reader"));
    setupLayout();
}


void StlReaderDlg::setupLayout()
{
    QHBoxLayout *pUnitLayout = new QHBoxLayout;
    {
        QLabel *plabUnit = new QLabel(tr("Length unit with which to read the file"));
        plabUnit->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_pcbLengthUnitSel = new QComboBox;
        QStringList list;
        list <<"mm"<<"cm"<<"dm"<<"m"<<"in"<<"ft";
        m_pcbLengthUnitSel->clear();
        m_pcbLengthUnitSel->addItems(list);
        m_pcbLengthUnitSel->setCurrentIndex(s_LengthUnitIndex);
        m_pcbLengthUnitSel->setToolTip(tr("Select the length unit to read the STL file"));

        QLabel *plabTol = new QLabel(tr("Tolerance on node position:"));
        plabTol->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

        pUnitLayout->addWidget(plabUnit);
        pUnitLayout->addWidget(m_pcbLengthUnitSel);
    }



    m_ppto = new PlainTextOutput;
    m_ppto->setCharDimensions(25,17);
//    connect(this, SIGNAL(outputMsg(QString)), m_ppto, SLOT(onAppendQText(QString)));

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard);
    {
        m_ppbImport = new QPushButton(tr("Import mesh file"));
        m_pButtonBox->addButton(m_ppbImport, QDialogButtonBox::ActionRole);
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pUnitLayout);
        pMainLayout->addWidget(m_ppto);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void StlReaderDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("StlReaderDlg");
    {
        s_LengthUnitIndex =  settings.value("LengthUnitIndex", s_LengthUnitIndex).toInt();
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void StlReaderDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("StlReaderDlg");
    {
        settings.setValue("LengthUnitIndex", s_LengthUnitIndex);

        settings.setValue("WindowGeom", s_Geometry);
    }
    settings.endGroup();
}


void StlReaderDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void StlReaderDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
    s_LengthUnitIndex = m_pcbLengthUnitSel->currentIndex();
}


void StlReaderDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)
    {
        accept();
    }
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)
    {
        reject();
    }
    else if (m_ppbImport == pButton)
    {
        onImportMeshFile();
    }
}


QString StlReaderDlg::logMsg() const {return m_ppto->toPlainText();}


void StlReaderDlg::onImportMeshFile()
{
    s_LengthUnitIndex = m_pcbLengthUnitSel->currentIndex();
    double unitfactor=1.0;
    switch(s_LengthUnitIndex)
    {
        case 0: unitfactor=1.0/1000.0;     break;
        case 1: unitfactor=1.0/100.0;      break;
        case 2: unitfactor=1.0/10.0;       break;
        default:
        case 3: unitfactor=1.0/1.0;        break;
        case 4: unitfactor=0.0254;         break;
        case 5: unitfactor=0.0254*12.0;    break;
    }

    QString filter ="Mesh files (*.stl *.obj)";
    QString FileName;

    QFileDialog dlg(this);
    FileName = dlg.getOpenFileName(this, "Import mesh file",
                                   SaveOptions::STLDirName(),
                                   filter);


    if(!FileName.length()) return;

    std::vector<Triangle3d> triangles;

    m_ppbImport->setEnabled(false);
    m_ppto->onAppendQText(tr("Starting import process\n"));
    
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if(!importTrianglesFromMeshFile(FileName, unitfactor))
    {
        m_ppto->onAppendQText("\n***** Error reading the file *****\n\n");
    }

    QApplication::restoreOverrideCursor();

    m_ppbImport->setEnabled(true);
    m_ppto->onAppendQText("Done.\n\n");
}


bool StlReaderDlg::importTrianglesFromMeshFile(QString const &FileName, double unitfactor)
{
    QFile meshfile(FileName);
    if (!meshfile.open(QIODevice::ReadOnly))
    {
        m_ppto->onAppendQText("Unable to open the file:" + FileName + "\n");
        return false;
    }

    QFileInfo fi(meshfile);
    bool bSTL(true);
    QString extension = fi.suffix();
    if(extension.compare("stl", Qt::CaseInsensitive)==0)
    {
        m_ppto->onAppendQText("Importing from an STL file\n");
        bSTL = true;
    }
    else if(extension.compare("obj", Qt::CaseInsensitive)==0)
    {
        m_ppto->onAppendQText("Importing from an OBJ file\n");
        bSTL = false;
    }
    else
    {
        m_ppto->onAppendQText("Unknown file extension: cannot determine file type\n"
                              "Aborting.");
        return false;
    }


    std::vector<Triangle3d> triangles;

    bool bSuccess(false);
    Vector3d botleft, topright;
    if(bSTL)
    {
        double MergeAngle = 0.0; // ?
        bSuccess = io::readSTLFile(FileName.toStdString(), unitfactor, MergeAngle, triangles, botleft, topright);
    }
    else
    {
        bSuccess = io::readOBJFile(FileName.toStdString(), unitfactor, triangles, botleft, topright);
    }

    if(bSuccess)
    {
        botleft  *= Units::mtoUnit();
        topright *= Units::mtoUnit();
        QString logmsg;
        logmsg += QString::asprintf("Imported %d triangles\n\n", int(triangles.size()));
        logmsg +=                   "Bounding box limits:                     x           y           z\n";
        logmsg += QString::asprintf("                     botleft = %11g %11g %11g ", botleft.x,  botleft.y,  botleft.z)  + Units::lengthUnitQLabel() + EOLch;
        logmsg += QString::asprintf("                     topright= %11g %11g %11g ", topright.x, topright.y, topright.z) + Units::lengthUnitQLabel() + EOLch;
        logmsg += EOLstr;
        m_ppto->onAppendQText(logmsg);
        m_Triangle = triangles;
    }

    return bSuccess;
}

