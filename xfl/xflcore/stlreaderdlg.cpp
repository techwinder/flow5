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

#include <QFileDialog>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

#include "stlreaderdlg.h"

#include <xflcore/xflcore.h>
#include <xflcore/saveoptions.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>


int StlReaderDlg::s_LengthUnitIndex = 0;
QByteArray StlReaderDlg::s_Geometry;

StlReaderDlg::StlReaderDlg(QWidget *pParent)  : QDialog(pParent)
{
    setWindowTitle("STL Reader");
    m_bCancel = false;
    m_bIsRunning = false;
    setupLayout();
}


void StlReaderDlg::setupLayout()
{
    QHBoxLayout *pUnitLayout = new QHBoxLayout;
    {
        QLabel *pLabUnit = new QLabel("Length unit with which to read the file");
        pLabUnit->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        m_pcbLengthUnitSel = new QComboBox;
        QStringList list;
        list <<"mm"<<"cm"<<"dm"<<"m"<<"in"<<"ft";
        m_pcbLengthUnitSel->clear();
        m_pcbLengthUnitSel->addItems(list);
        m_pcbLengthUnitSel->setCurrentIndex(s_LengthUnitIndex);
        m_pcbLengthUnitSel->setToolTip("Select the length unit to read the STL file");

        QLabel *pLabTol = new QLabel("Tolerance on node position:");
        pLabTol->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

        pUnitLayout->addWidget(pLabUnit);
        pUnitLayout->addWidget(m_pcbLengthUnitSel);
    }

    m_ppbImport = new QPushButton("Import file");
    connect(m_ppbImport, SIGNAL(clicked(bool)), SLOT(onImportFromStlFile()));

    m_pptoTextOutput = new PlainTextOutput;
    m_pptoTextOutput->setCharDimensions(25,17);

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pUnitLayout);
        pMainLayout->addWidget(m_ppbImport);
        pMainLayout->addWidget(m_pptoTextOutput);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
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


QString StlReaderDlg::logMsg() const {return m_pptoTextOutput->toPlainText();}


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
        case 3: unitfactor=1.0/1.0;           break;
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

    m_ppbImport->setText("Cancel");
    m_bIsRunning = true;

    QVector<Triangle3d> triangles;
    QString logmsg;
    m_bCancel = false;

    m_pptoTextOutput->onAppendThisPlainText("Starting import process\n");
    

    QApplication::setOverrideCursor(Qt::BusyCursor);
    bool bSuccess = importTrianglesFromStlFile(FileName, unitfactor, triangles);
    QApplication::restoreOverrideCursor();

    m_pptoTextOutput->onAppendThisPlainText(logmsg);
    

    if(!bSuccess || m_bCancel)
    {
        if(m_bCancel) m_pptoTextOutput->insertPlainText("   Operation cancelled... cleaning up\n\n");
        else          m_pptoTextOutput->insertPlainText("   Error importing... cleaning up\n\n");
        m_Triangle.clear();
        m_bIsRunning = false;
        m_ppbImport->setText("Import file");
        return;
    }

    m_Triangle = triangles;

    m_ppbImport->setText("Import file");
    m_bIsRunning = false;
    m_bCancel = false;
}


bool StlReaderDlg::importTrianglesFromStlFile(QString const &FileName, double unitfactor, QVector<Triangle3d> &trianglelist) const
{
    QFile stlfile(FileName);
    if (!stlfile.open(QIODevice::ReadOnly))
    {
        m_pptoTextOutput->onAppendThisPlainText("Unable to open the file:" + FileName + "\n");
        return false;
    }

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
        bSuccess = importStlTextFile(textstream, unitfactor, trianglelist, solidname);
    }
    else
    {
        m_pptoTextOutput->onAppendThisPlainText("Not recognized as a Text file... Switching to binary\n");
        //Try the binary format
        QDataStream inStream(&stlfile);
        inStream.setByteOrder(QDataStream::LittleEndian);
        //    uint u = inStream.byteOrder();

        bSuccess = importStlBinaryFile(inStream, unitfactor, trianglelist, solidname);
    }

    stlfile.close();

    return bSuccess;
}


bool StlReaderDlg::importStlBinaryFile(QDataStream &binstream, double unitfactor, QVector<Triangle3d> &trianglelist,
                                       QString &solidname) const
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
    float xmin=1.e10, xmax=-1.0e10;
    float ymin=1.e10, ymax=-1.0e10;
    float zmin=1.e10, zmax=-1.0e10;
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
        Triangle3d &t3d = trianglelist.last();
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

        //        trianglelist.last().displayNodes("BNodes==");
        binstream.readRawData(buffer, 2);

        if(j%1000==0)
        {
            QString strange;
            strange = QString::asprintf("   imported %d triangles\n", int(trianglelist.size()-1));
            m_pptoTextOutput->onAppendThisPlainText(strange);
            
            if(m_bCancel)
                return false;
        }
    }

    QString logmsg;
    strong = QString::asprintf("Read %d STL triangles, made %d panels\n", nTriangles, int(trianglelist.size()));
    logmsg+=strong;
    strong = QString::asprintf("Reordered vertices of %d inverted triangles\n", nNegTriangles);
    logmsg+=strong;


    if(nTriangles>0)
    {
        strong = QString::asprintf("   xmin=%13g  xmax=%13g\n", double(xmin), double(xmax));
        logmsg+=strong;
        strong = QString::asprintf("   ymin=%13g  ymax=%13g\n", double(ymin), double(ymax));
        logmsg+=strong;
        strong = QString::asprintf("   zmin=%13g  zmax=%13g\n", double(zmin), double(zmax));
        logmsg+=strong;
    }

    double dimension = fabs(double(xmin));
    dimension = std::max(dimension, fabs(double(xmax)));
    dimension = std::max(dimension, fabs(double(ymin)));
    dimension = std::max(dimension, fabs(double(ymax)));
    dimension = std::max(dimension, fabs(double(zmin)));
    dimension = std::max(dimension, fabs(double(zmax)));
    //    dimension = sqrt((xmax-xmin)*(xmax-xmin) + (ymax-ymin)*(ymax-ymin) + (zmax-zmin)*(zmax-zmin));
    strong = QString::asprintf("   Max. model size = %9.3g m\n", dimension);
    logmsg+=strong;

    m_pptoTextOutput->onAppendThisPlainText(logmsg+"\n");
    return true;
}


bool StlReaderDlg::importStlTextFile(QTextStream &textstream, double unitfactor, QVector<Triangle3d> &trianglelist,
                                     QString &solidname) const
{
    QString strong;
    int iLine=-1;
    //header
    do
    {
        strong = textstream.readLine();
        m_pptoTextOutput->onAppendThisPlainText(strong+"\n");
        iLine++;
    }
    while(!strong.length());

    if(!strong.contains("solid", Qt::CaseInsensitive))
    {
        m_pptoTextOutput->onAppendThisPlainText("Error reading header: keyword \'solid\' not found");
        return false;
    }
    
    solidname = strong.replace("solid", "").trimmed();

    double xmin=1.e100, xmax=-1.0e100;
    double ymin=1.e100, ymax=-1.0e100;
    double zmin=1.e100, zmax=-1.0e100;
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
            m_pptoTextOutput->onAppendThisPlainText(log);
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
        Triangle3d &t3d = trianglelist.last();
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
/*        if(trianglelist.last().isNull())
        {
            nullsize++;
            trianglelist.pop_back();
        }*/
        iLine++;
        if(iLine%1000==0)
        {
            QString strange;
            strange = QString::asprintf("   imported %d triangles\n", int(trianglelist.size())-1);
            m_pptoTextOutput->onAppendThisPlainText(strange);
            
            if(m_bCancel) return false;
        }
    }

    log = QString::asprintf("Read %d STL triangles successfully\n", nTriangles);
    strong = QString::asprintf("Reordered vertices of %d inverted triangles\n", nNegTriangles);
    log+=strong;

    strong = QString::asprintf("Final triangle count is %d\n", int(trianglelist.size()));
    log += strong;
    strong = QString::asprintf("   xmin=%13g  xmax=%13g\n", xmin, xmax);
    log += strong;
    strong = QString::asprintf("   ymin=%13g  ymax=%13g\n", ymin, ymax);
    log += strong;
    strong = QString::asprintf("   zmin=%13g  zmax=%13g\n", zmin, zmax);
    log += strong;


    double dimension = fabs(xmin);
    dimension = std::max(dimension, double(fabs(xmax)));
    dimension = std::max(dimension, double(fabs(ymin)));
    dimension = std::max(dimension, double(fabs(ymax)));
    dimension = std::max(dimension, double(fabs(zmin)));
    dimension = std::max(dimension, double(fabs(zmax)));

    //    dimension = sqrt((xmax-xmin)*(xmax-xmin) + (ymax-ymin)*(ymax-ymin) + (zmax-zmin)*(zmax-zmin));
    strong = QString::asprintf("   Max. model size = %9.3g m\n", dimension);
    log+=strong;

    m_pptoTextOutput->onAppendThisPlainText(log+"\n");
    return true;
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

