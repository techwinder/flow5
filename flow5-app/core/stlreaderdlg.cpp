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

#include <core/saveoptions.h>
#include <core/xflcore.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>

#include <api/units.h>
#include <api/flow5events.h>

int StlReaderDlg::s_LengthUnitIndex = 0;
QByteArray StlReaderDlg::s_Geometry;

StlReaderDlg::StlReaderDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("STL Reader"));
    m_bCancel = false;
    m_bIsRunning = false;
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

        QLabel *pLabTol = new QLabel(tr("Tolerance on node position:"));
        pLabTol->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

        pUnitLayout->addWidget(plabUnit);
        pUnitLayout->addWidget(m_pcbLengthUnitSel);
    }

    m_ppbImport = new QPushButton(tr("Import file"));
    connect(m_ppbImport, SIGNAL(clicked(bool)), SLOT(onImportFromStlFile()));

    m_ppto = new PlainTextOutput;
    m_ppto->setCharDimensions(25,17);
    connect(this, SIGNAL(outputMsg(QString)), m_ppto, SLOT(onAppendQText(QString)));

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pUnitLayout);
        pMainLayout->addWidget(m_ppbImport);
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
        m_bCancel = true;
        accept();
    }
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)
    {
        m_bCancel = true;
        reject();
    }
}


void StlReaderDlg::onMessage(QString const &msg)
{
    m_ppto->onAppendQText(msg);
}


QString StlReaderDlg::logMsg() const {return m_ppto->toPlainText();}


void StlReaderDlg::onImportFromStlFile()
{
    if(m_bIsRunning)
    {
        m_bCancel = true;
        return;
    }

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

    QString filter ="STL Binary File (*.stl)";
    QString FileName;

    QFileDialog dlg(this);
    FileName = dlg.getOpenFileName(this, "Import STL",
                                   SaveOptions::STLDirName(),
                                   "STL Binary File (*.stl)",
                                   &filter);

    if(!FileName.length()) return;

    m_ppbImport->setText(tr("Cancel"));
    m_bIsRunning = true;

    std::vector<Triangle3d> triangles;
    m_bCancel = false;

    m_ppto->onAppendQText(tr("Starting import process\n"));
    


    //running this function in a separate thread to keep the UI responsive
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        QFuture<bool> future = QtConcurrent::run(&StlReaderDlg::importTrianglesFromStlFile, this, FileName, unitfactor);
#else
        QFuture<bool> future = QtConcurrent::run(this, &StlReaderDlg::importTrianglesFromStlFile, FileName, unitfactor);
#endif
}


void StlReaderDlg::postMessageEvent(QString const &msg)
{
    // both comm methods work; signals seem to arrive in sequence
/*    MessageEvent *pMsgEvent = new MessageEvent(msg); // forward to the UI thread for user notification
    qApp->postEvent(this, pMsgEvent);*/

    emit outputMsg(msg);
}


bool StlReaderDlg::importTrianglesFromStlFile(QString const &FileName, double unitfactor)
{
    QFile stlfile(FileName);
    if (!stlfile.open(QIODevice::ReadOnly))
    {
        postMessageEvent("Unable to open the file:" + FileName + "\n");
        return false;
    }
    QApplication::setOverrideCursor(Qt::BusyCursor);

    std::vector<Triangle3d> triangles;

    QString solidname;

    //Try the text format first
    bool bText=true;
    QTextStream textstream(&stlfile);
    QString strong = textstream.readLine(80);
    if(textstream.status()!=QTextStream::Ok)
    {
        bText=false;
    }
    else
    {
        if(strong.contains("solid"))
        {
            // continue testing, some binary STL files use 'solid' keyword in header
            strong = textstream.readLine(80);
            int pos = strong.indexOf("facet", Qt::CaseInsensitive);
            if(pos<0)
            {
                bText = false;
            }
        }
        else bText=false;
    }
    textstream.seek(0);

    bool bSuccess = false;
    if(bText)
    {
        postMessageEvent(tr("\nImporting from a text file\n\n"));
        bSuccess = importStlTextFile(textstream, unitfactor, triangles, solidname);
    }
    else
    {
        postMessageEvent(tr("\nImporting from a binary file\n\n"));
        //Try the binary format
        QDataStream inStream(&stlfile);
        inStream.setByteOrder(QDataStream::LittleEndian);
        //    uint u = inStream.byteOrder();

        bSuccess = importStlBinaryFile(inStream, unitfactor, triangles, solidname);
    }

    stlfile.close();



    if(!bSuccess || m_bCancel)
    {
        if(m_bCancel) m_ppto->insertPlainText(tr("   Operation cancelled... cleaning up\n\n"));
        else          m_ppto->insertPlainText(tr("   Error importing... cleaning up\n\n"));
        m_Triangle.clear();
        m_bIsRunning = false;
        m_ppbImport->setText(tr("Import file"));
        return bSuccess;
    }

    m_Triangle = triangles;

    m_ppbImport->setText(tr("Import file"));
    m_bIsRunning = false;
    m_bCancel = false;

    QApplication::restoreOverrideCursor();
    return bSuccess;
}


bool StlReaderDlg::importStlBinaryFile(QDataStream &binstream, double unitfactor, std::vector<Triangle3d> &trianglelist,
                                       QString &solidname)
{
    QString strong;
    qint8  ch=0;
    strong.clear();
    solidname = "STL_binary_solid";
    //80 character header, avoid word "solid"
    //                       0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
    for(int j=0; j<80;j++)
    {
        strong += " ";
        binstream >> ch;
        strong[j] = char(ch);
    }

    int nTriangles=0;
    binstream >> nTriangles;

    trianglelist.reserve(nTriangles);

    Vector3d N, v[3];
    float xmin=LARGEVALUE, xmax=-LARGEVALUE;
    float ymin=LARGEVALUE, ymax=-LARGEVALUE;
    float zmin=LARGEVALUE, zmax=-LARGEVALUE;
    float x=0, y=0, z=0;
    float nx=0, ny=0, nz=0;
    char buffer[12];
    int nNegTriangles=0;

    for (int j=0; j<nTriangles; j++)
    {
        // the normal
        xfl::readFloat(binstream, nx);
        xfl::readFloat(binstream, ny);
        xfl::readFloat(binstream, nz);
        N.set(double(nx), double(ny), double(nz));

        for(int iv=0; iv<3; iv++)
        {
            //vertex 0
            xfl::readFloat(binstream, x);
            xfl::readFloat(binstream, y);
            xfl::readFloat(binstream, z);
            x *= float(unitfactor);
            y *= float(unitfactor);
            z *= float(unitfactor);
            xmin = std::min(x,xmin);   xmax = std::max(x,xmax);
            ymin = std::min(y,ymin);   ymax = std::max(y,ymax);
            zmin = std::min(z,zmin);   zmax = std::max(z,zmax);
            v[iv].set(double(x), double(y), double(z));

        }

        trianglelist.push_back({v[0], v[1], v[2]});
        Triangle3d &t3d = trianglelist.back();
        if(t3d.normal().dot(N)<.0)
        {
            // re-order vertices to have a positive oriented triangle
            Vector3d tmp = t3d.vertexAt(1);
            t3d.setVertex(1, t3d.vertexAt(2));
            t3d.setVertex(2, tmp);
            t3d.setTriangle();

            nNegTriangles++;
        }

        t3d.setNormal(N);

        //add, then remove if nullptr; this avoids double construction
/*        if(t3d.isNull())
        {
            trianglelist.pop_back();
            nNullTriangles++;
        }*/

        //        trianglelist.back().displayNodes("BNodes==");
        binstream.readRawData(buffer, 2);

        if(j%1000==0)
        {
            QString strange;
            strange = QString::asprintf("   imported %d triangles\n", int(trianglelist.size()-1));
            postMessageEvent(strange);
            
            if(m_bCancel)
                return false;
        }
    }

    QString logmsg;
    strong = QString::asprintf("Read %d STL triangles, made %d panels\n", nTriangles, int(trianglelist.size()));
    logmsg+=strong;
    strong = QString::asprintf("Reordered vertices of %d inverted triangles\n", nNegTriangles);
    logmsg+=strong;


    logmsg += "\nBounding box:\n";

    strong  = QString::asprintf("   xmin=%13g ", xmin*Units::mtoUnit() ) + Units::lengthUnitQLabel();
    strong += QString::asprintf("   xmax=%13g ", xmax*Units::mtoUnit() ) + Units::lengthUnitQLabel() + EOLch;
    logmsg += strong;

    strong  = QString::asprintf("   ymin=%13g ", ymin*Units::mtoUnit() ) + Units::lengthUnitQLabel();
    strong += QString::asprintf("   ymax=%13g ", ymax*Units::mtoUnit() ) + Units::lengthUnitQLabel() + EOLch;
    logmsg += strong;

    strong  = QString::asprintf("   zmin=%13g ", zmin*Units::mtoUnit() ) + Units::lengthUnitQLabel();
    strong += QString::asprintf("   zmax=%13g ", zmax*Units::mtoUnit() ) + Units::lengthUnitQLabel() + EOLch;
    logmsg += strong + EOLch;


    postMessageEvent(logmsg+"\n");
    return true;
}


bool StlReaderDlg::importStlTextFile(QTextStream &textstream, double unitfactor, std::vector<Triangle3d> &trianglelist,
                                     QString &solidname)
{
    QString strong;
    int iLine=-1;
    //header
    do
    {
        strong = textstream.readLine();
        postMessageEvent(strong+"\n");
        iLine++;
    }
    while(!strong.length());

    if(!strong.contains("solid", Qt::CaseInsensitive))
    {
        postMessageEvent("Error reading header: keyword \'solid\' not found");
        return false;
    }
    
    solidname = strong.replace("solid", "").trimmed();

    double xmin=LARGEVALUE, xmax=-LARGEVALUE;
    double ymin=LARGEVALUE, ymax=-LARGEVALUE;
    double zmin=LARGEVALUE, zmax=-LARGEVALUE;
    Vector3d v[3], N;
    double x=0,y=0,z=0;
    double nx=0, ny=0, nz=0;
    int nTriangles=0;
    int nNegTriangles = 0;

    QString log;
    while(!textstream.atEnd())
    {
        strong = textstream.readLine();
        iLine++;
        if(textstream.status() != QTextStream::Ok)
        {
            log = QString::asprintf("Unknown error reading line %d", iLine);
            postMessageEvent(log);
            return false;
        }

        if(strong.contains("endsolid", Qt::CaseInsensitive)) break;

        if(strong.contains("facet", Qt::CaseInsensitive) && strong.contains("normal", Qt::CaseInsensitive))
        {
            //read a triangle
            strong.replace("facet","");
            strong.replace("normal","");
            strong.replace("FACET","");
            strong.replace("NORMAL","");
            strong = strong.trimmed();
            if(xfl::readValues(strong,nx,ny,nz)==3)
            {
                strong = textstream.readLine();
                iLine++;
                if(strong.contains("outer", Qt::CaseInsensitive) && strong.contains("loop", Qt::CaseInsensitive))
                {
                    for(int iv=0; iv<3; iv++)
                    {
                        strong = textstream.readLine();
                        iLine++;
                        if(strong.contains("vertex", Qt::CaseInsensitive))
                        {
                            strong = strong.replace("vertex","").trimmed();
                            if(xfl::readValues(strong,x,y,z)==3)
                            {
                                x*=unitfactor;
                                y*=unitfactor;
                                z*=unitfactor;
                                xmin = std::min(x,xmin);   xmax = std::max(x,xmax);
                                ymin = std::min(y,ymin);   ymax = std::max(y,ymax);
                                zmin = std::min(z,zmin);   zmax = std::max(z,zmax);
                                v[iv].set(x,y,z);
                            }
                            else
                            {
                                log = QString::asprintf("Error reading triangles: could not read 3 values on line %d", iLine);
                                return false;
                            }
                        }
                        else
                        {
                            log = QString::asprintf("Error reading triangles: keyword \'vertex\' not found on line %d", iLine);
                            return false;
                        }
                    }
                    strong = textstream.readLine();
                    iLine++;
                    if(!strong.contains("endloop", Qt::CaseInsensitive))
                    {
                        log = QString::asprintf("Error reading triangles: keyword \'endloop\' not found on line %d", iLine);
                        return false;
                    }
                }
                else
                {
                    log = QString::asprintf("Error reading triangles: keyword \'outer loop\' not found on line %d", iLine);
                    return false;
                }
            }
        }
        else
        {
            log = QString::asprintf("Error reading triangles: keyword \'facet\' not found on line %d", iLine);
            return false;
        }
        strong = textstream.readLine();
        if(!strong.contains("endfacet", Qt::CaseInsensitive))
        {
            log = QString::asprintf("Error reading triangles: keyword \'endfacet\' not found on line %d", iLine);
            return false;
        }

        trianglelist.push_back({v[0], v[1], v[2]});
        Triangle3d &t3d = trianglelist.back();
        N.set(nx,ny,nz);
        if(t3d.normal().dot(N)<.0)
        {
            // re-order vertices to have a positive oriented triangle
            Vector3d tmp = t3d.vertexAt(1);
            t3d.setVertex(1, t3d.vertexAt(2));
            t3d.setVertex(2, tmp);
            t3d.setTriangle();

            nNegTriangles++;
        }
        t3d.setNormal(N);
        nTriangles++;
/*        if(trianglelist.back().isNull())
        {
            nullsize++;
            trianglelist.pop_back();
        }*/
        iLine++;
        if(iLine%1000==0)
        {
            QString strange;
            strange = QString::asprintf("   imported %d triangles\n", int(trianglelist.size())-1);
            postMessageEvent(strange);
            
            if(m_bCancel) return false;
        }
    }

    log = QString::asprintf("Read %d STL triangles successfully\n", nTriangles);
    strong = QString::asprintf("Reordered vertices of %d inverted triangles\n", nNegTriangles);
    log+=strong;

    strong = QString::asprintf("Final triangle count is %d\n\n", int(trianglelist.size()));
    log += strong;

    log += "Bounding box:\n";

    strong  = QString::asprintf("   xmin=%13g ", xmin*Units::mtoUnit() ) + Units::lengthUnitQLabel();
    strong += QString::asprintf("   xmax=%13g ", xmax*Units::mtoUnit() ) + Units::lengthUnitQLabel() + EOLch;
    log += strong;

    strong  = QString::asprintf("   ymin=%13g ", ymin*Units::mtoUnit() ) + Units::lengthUnitQLabel();
    strong += QString::asprintf("   ymax=%13g ", ymax*Units::mtoUnit() ) + Units::lengthUnitQLabel() + EOLch;
    log += strong;

    strong  = QString::asprintf("   zmin=%13g ", zmin*Units::mtoUnit() ) + Units::lengthUnitQLabel();
    strong += QString::asprintf("   zmax=%13g ", zmax*Units::mtoUnit() ) + Units::lengthUnitQLabel() + EOLch;
    log += strong + EOLch;

    postMessageEvent(log+"\n");
    return true;
}

