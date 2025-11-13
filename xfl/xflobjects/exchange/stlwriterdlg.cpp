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


#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialogButtonBox>

#include "stlwriterdlg.h"

#include <xflcore/xflcore.h>
#include <xflcore/saveoptions.h>
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/plaintextoutput.h>
#include <xflobjects/objects_globals/objects_global.h>
#include <xflobjects/objects3d/plane/planexfl.h>
#include <xflobjects/objects3d/fuse/fusexfl.h>
#include <xflobjects/objects3d/wing/wingxfl.h>
#include <xflobjects/sailobjects/sails/sail.h>



QByteArray STLWriterDlg::s_Geometry;


bool STLWriterDlg::s_bBinary = true;
int STLWriterDlg::s_iChordPanels = 13;
int STLWriterDlg::s_iSpanPanels = 17;

int STLWriterDlg::s_LengthUnitIndex = 3;


STLWriterDlg::STLWriterDlg(QWidget *pParent) : QDialog(pParent)
{
    m_pPlane = nullptr;
    m_pFuse = nullptr;
    m_pWing = nullptr;
    m_pSail = nullptr;

    setupLayout();

    m_UnitFactor=1.0;
    m_prbBinary->setEnabled(false);
    m_prbASCII->setEnabled(false);
}


void STLWriterDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGroupBox *pExportFormat = new QGroupBox("File format");
        {
            QHBoxLayout *pFormatLayout = new QHBoxLayout;
            {
                m_prbBinary = new QRadioButton("Binary");
                m_prbASCII  = new QRadioButton("ASCII");
                pFormatLayout->addWidget(m_prbBinary);
                pFormatLayout->addWidget(m_prbASCII);
            }
            pExportFormat->setLayout(pFormatLayout);
        }

        m_plwNameList = new QListWidget;
        m_plwNameList->setSelectionMode(QAbstractItemView::MultiSelection);
        connect(m_plwNameList, SIGNAL(currentRowChanged(int)), this, SLOT(onSetLabels()));

        QGroupBox *pResolutionBox = new QGroupBox("Output Resolution");
        {
            QVBoxLayout *pResolutionLayout = new QVBoxLayout;
            {
                QHBoxLayout *pChordLayout = new QHBoxLayout;
                {
                    m_plabChord = new QLabel("Chordwise panels");
                    m_plabChord->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_pieChordPanels = new IntEdit(17);
                    m_pieChordPanels->setAlignment(Qt::AlignRight);
                    pChordLayout->addStretch();
                    pChordLayout->addWidget(m_plabChord);
                    pChordLayout->addWidget(m_pieChordPanels);
                }

                QHBoxLayout *pSpanLayout = new QHBoxLayout;
                {
                    m_plabSpan = new QLabel("Spanwise panels");
                    m_plabSpan->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_pieSpanPanels = new IntEdit(17);
                    m_pieSpanPanels->setAlignment(Qt::AlignRight);
                    pSpanLayout->addStretch();
                    pSpanLayout->addWidget(m_plabSpan);
                    pSpanLayout->addWidget(m_pieSpanPanels);
                }
                pResolutionLayout->addLayout(pChordLayout);
                pResolutionLayout->addLayout(pSpanLayout);
            }
            pResolutionBox->setLayout(pResolutionLayout);
        }

        QHBoxLayout *pUnitLayout = new QHBoxLayout;
        {
            QLabel *pLabUnit = new QLabel("Unit to write the file:");
            pLabUnit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            m_pcbLengthUnitSel = new QComboBox;
            QStringList list;
            list <<"mm" << "cm"<<"dm"<<"m"<<"in"<<"ft";
            m_pcbLengthUnitSel->clear();
            m_pcbLengthUnitSel->addItems(list);
            m_pcbLengthUnitSel->setCurrentIndex(s_LengthUnitIndex);
            m_pcbLengthUnitSel->setToolTip("Select the length unit to read the STL file");

            pUnitLayout->addWidget(pLabUnit);
            pUnitLayout->addWidget(m_pcbLengthUnitSel);
        }

        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        QPushButton *pExportButton  = new QPushButton("Export Part");
        connect(pExportButton, SIGNAL(clicked(bool)), this, SLOT(onExporttoSTL()));
        pExportButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        m_pptoOutputLog = new PlainTextOutput;

        pMainLayout->addWidget(pExportFormat);
        pMainLayout->addWidget(m_plwNameList);
        pMainLayout->addWidget(pResolutionBox);
        pMainLayout->addLayout(pUnitLayout);
        pMainLayout->addWidget(pExportButton);
        pMainLayout->addWidget(m_pptoOutputLog);

        pMainLayout->addWidget(pButtonBox);
    }
    setLayout(pMainLayout);
}


void STLWriterDlg::accept()
{
    readParams();
    done(QDialog::Accepted);
}


void STLWriterDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void STLWriterDlg::hideEvent(QHideEvent*pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void STLWriterDlg::initDialog(PlaneXfl *pPlane, WingXfl *pWing, Fuse *pFuse, Sail *pSail)
{
    m_pPlane = pPlane;
    m_pFuse = pFuse;
    m_pWing = pWing;
    m_pSail = pSail;
    m_prbBinary->setChecked(s_bBinary);
    m_prbASCII->setChecked(!s_bBinary);
    m_pieChordPanels->setValue(s_iChordPanels);
    m_pieSpanPanels->setValue(s_iSpanPanels);

    if(m_pPlane)
    {
        for(int i=0; i<m_pPlane->nWings(); i++)
        {
            m_plwNameList->addItem(m_pPlane->wingAt(i)->name());
        }
        if(m_pPlane->hasFuse()) m_plwNameList->addItem(m_pPlane->fuseAt(0)->name());
    }
    else if(m_pFuse)
    {
        m_plwNameList->addItem(m_pFuse->name());
    }
    else if(m_pWing)
    {
        m_plwNameList->addItem(m_pWing->name());
    }
    else if(m_pSail)
    {
        m_plwNameList->addItem(m_pSail->name());
    }

    if(m_plwNameList->count()) m_plwNameList->setCurrentRow(0);
    onSetLabels();
}


void STLWriterDlg::readParams()
{
    s_bBinary = m_prbBinary->isChecked();
    s_iChordPanels = m_pieChordPanels->value();
    s_iSpanPanels  = m_pieSpanPanels->value();

    s_LengthUnitIndex = m_pcbLengthUnitSel->currentIndex();

    switch(s_LengthUnitIndex)
    {
        case 0: m_UnitFactor=1.0/1000.0;     break;
        case 1: m_UnitFactor=1.0/100.0;      break;
        case 2: m_UnitFactor=1.0/10.0;       break;
        default:
        case 3: m_UnitFactor=1.0/1.0;           break;
        case 4: m_UnitFactor=0.0254;         break;
        case 5: m_UnitFactor=0.0254*12.0;    break;
    }

    QListWidgetItem *pItem = m_plwNameList->currentItem();
    if(pItem && m_pPlane)
    {
        QListWidgetItem *pItem =  nullptr;

        m_SelectedList.clear();
        for(int i=0; i<m_plwNameList->count();i++)
        {
            pItem = m_plwNameList->item(i);
            if(pItem->isSelected())
            {
                m_SelectedList.append(pItem->text());
            }
        }
    }
}


void STLWriterDlg::onSetLabels()
{
    readParams();
    if(m_pFuse)
    {
        m_plabChord->setText("Number of x-panels");
        m_plabSpan->setText("Number of hoop panels");
    }
    else if(m_pWing)
    {
        m_plabChord->setText("Number of chordwise panels");
        m_plabSpan->setText("Number of span panels per surface");
    }
    else if(m_pSail)
    {
        m_plabChord->setText("Number of chordwise panels");
        m_plabSpan->setText("Number of z-panels");
    }
}


/**
 * Exports the geometrical data of the acitve wing or plane to a text file readable by AVL
 *@todo AVL expects consistency of the units, need to check all lines and cases
 */
void STLWriterDlg::onExporttoSTL()
{
    readParams();

    if (!m_pPlane && !m_pWing && !m_pFuse && !m_pSail) return;

    QString filter ="STL File (*.stl)";
    QString FileName;

    if      (m_pFuse)  FileName = m_pFuse->name().trimmed();
    else if (m_pWing)  FileName = m_pWing->name().trimmed();
    else if (m_pSail)  FileName = m_pSail->name().trimmed();
    else if (m_pPlane) FileName = m_pPlane->name().trimmed();
    FileName.replace('/', '_');
    FileName.replace(' ', '_');

    QFileDialog Fdlg(this);
    FileName = Fdlg.getSaveFileName(this, "Export to STL file",
                                    SaveOptions::STLDirName() + "/"+FileName+".stl",
                                    "STL File (*.stl)",
                                    &filter);

    if(!FileName.length()) return;

    bool bBinary = STLWriterDlg::s_bBinary;

    int pos = FileName.indexOf(".stl", Qt::CaseInsensitive);
    if(pos<0) FileName += ".stl";

    QFile XFile(FileName);


    if (!XFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(window(), "Warning", "Could not open the file for writing");
        return;
    }

    int nTriangles = 0;
    if(m_pPlane)
    {
        if(bBinary)
        {
            QDataStream out(&XFile);
            // stl format uses Little-Endian byte order
            out.setByteOrder(QDataStream::LittleEndian);
//            nTriangles = exportWingToSTLBinary(m_pWing, out, STLWriterDlg::s_iChordPanels, STLWriterDlg::s_iSpanPanels, m_UnitFactor);

            QVector<Triangle3d> triangles;
            for(int i=0; i<m_SelectedList.size(); i++)
            {
                WingXfl const *pWing = m_pPlane->wingFromName(m_SelectedList.at(i));
                Fuse const *pFuse = m_pPlane->fuseFromName(m_SelectedList.at(i));
                if(pWing)
                {
                    pWing->makeTriangulation(triangles, s_iChordPanels, s_iSpanPanels, 1.0);
                }
                else if(pFuse)
                {
                    triangles.append(pFuse->triangles());
                }
            }
            nTriangles = exportTriangulation(out, 1.0, triangles);
        }
        else
        {
            QTextStream out(&XFile);
            nTriangles = exportWingToSTLText(m_pWing, out, STLWriterDlg::s_iChordPanels, STLWriterDlg::s_iSpanPanels, 1.0);
        }
    }
    else if(m_pWing)
    {
        if(bBinary)
        {
            QDataStream out(&XFile);
            // stl format uses Little-Endian byte order
            out.setByteOrder(QDataStream::LittleEndian);
            QVector<Triangle3d> triangles;
            m_pWing->makeTriangulation(triangles, s_iChordPanels, s_iSpanPanels, 1.0);
            nTriangles = exportTriangulation(out, 1.0, triangles);
        }
        else
        {
            QTextStream out(&XFile);
            nTriangles = exportWingToSTLText(m_pWing, out, STLWriterDlg::s_iChordPanels, STLWriterDlg::s_iSpanPanels, 1.0);
        }
    }
    else if(m_pFuse)
    {
        if(bBinary)
        {
            QDataStream out(&XFile);
            out.setByteOrder(QDataStream::LittleEndian);
            nTriangles = exportTriangulation(out, m_UnitFactor, m_pFuse->triangles());
        }
        else
        {
//            QTextStream out(&XFile);
        }
    }
    else if(m_pSail)
    {
        if(bBinary)
        {
            QDataStream out(&XFile);
            // stl format uses Little-Endian byte order
            out.setByteOrder(QDataStream::LittleEndian);
            nTriangles = exportSailToSTLBinary(m_pSail, out, STLWriterDlg::s_iChordPanels, STLWriterDlg::s_iSpanPanels, m_UnitFactor);
        }
    }

    m_pptoOutputLog->insertPlainText("The part has been successfully exported to the STL file");
    m_pptoOutputLog->insertPlainText("\n");
    QString strong;
    strong = QString::asprintf("Total triangles: %d", nTriangles);
    m_pptoOutputLog->insertPlainText(strong);
    XFile.close();
}



int STLWriterDlg::exportWingToSTLText(WingXfl const *pWing, QTextStream &outstream, int CHORDPANELS, int SPANPANELS, double scalefactor) const
{
    Q_UNUSED(scalefactor)
    /***
     * solid name
     *
     *       facet normal ni nj nk
     *         outer loop
     *           vertex v1x v1y v1z
     *           vertex v2x v2y v2z
     *           vertex v3x v3y v3z
     *      endloop
     *   endfacet
     *
     * endsolid name
    */
    QString name = pWing->name();
    name.replace(" ","");
    QString strong = "solid " + name + "\n";
    outstream << strong;

    Vector3d N, Pt;

    QVector<Node> PtLeft(CHORDPANELS+1);
    QVector<Node> PtRight(CHORDPANELS+1);
    QVector<Node> PtBotLeft(CHORDPANELS+1);
    QVector<Node> PtBotRight(CHORDPANELS+1);

    QVector<Surface> const &surfaces = pWing->m_Surface;

    //Number of triangles
    // nSurfaces
    //   *CHORDPANELS*SPANPANELS   quads
    //   *2                        2 triangles/quad
    //   *2                        top and bottom surfaces
    // 2 Tip patches
    //   1 LE triangle
    //   1 TE triangle
    //   CHORDPANELS-1  quads
    //   *2 triangles/quad

    int nTriangles = surfaces.count() * CHORDPANELS * SPANPANELS * 2 *2
            + 2* ((CHORDPANELS-2) * 2 + 2);
    N.set(0.0, 0.0, 0.0);
    int iTriangles = 0;

    for (int j=0; j<surfaces.size(); j++)
    {
        //top surface
        for(int is=0; is<SPANPANELS; is++)
        {
            surfaces.at(j).getSidePoints_1(xfl::TOPSURFACE, nullptr,
                                           PtLeft, PtRight, CHORDPANELS+1, xfl::COSINE);

            double tauA = double(is)  /double(SPANPANELS);
            double tauB = double(is+1)/double(SPANPANELS);
            double tau = (tauA+tauB)/2.0;
            for(int ic=0; ic<CHORDPANELS; ic++)
            {
                N = (PtLeft[ic].normal()+PtLeft[ic+1].normal()) * (1.0-tau) + (PtRight[ic].normal()+PtRight[ic+1].normal()) * tau;
                N.normalize();

                //1st triangle
                outstream << QString::asprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outstream << "    outer loop\n";
                Pt = PtLeft[ic]   * (1.0-tauA) + PtRight[ic]   * tauA;
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic+1] * (1.0-tauA) + PtRight[ic+1] * tauA;
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic]   * (1.0-tauB) + PtRight[ic]   * tauB;
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                outstream << "    endloop\n  endfacet\n";

                //2nd triangle
                outstream << QString::asprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outstream << "    outer loop\n";
                Pt = PtLeft[ic]   * (1.0-tauB) + PtRight[ic]   * tauB;
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic+1] * (1.0-tauA) + PtRight[ic+1] * tauA;
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic+1] * (1.0-tauB) + PtRight[ic+1] * tauB;
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                outstream << "    endloop\n  endfacet\n";
                iTriangles +=2;
            }
        }

        //bottom surface
        for(int is=0; is<SPANPANELS; is++)
        {
            surfaces.at(j).getSidePoints_1(xfl::BOTSURFACE, nullptr, PtLeft, PtRight, CHORDPANELS+1, xfl::COSINE);

            double tauA = double(is)   / double(SPANPANELS);
            double tauB = double(is+1) / double(SPANPANELS);
            double tau = (tauA+tauB)/2.0;
            for(int ic=0; ic<CHORDPANELS; ic++)
            {
                //left side vertices
                N = (PtLeft[ic].normal()+PtLeft[ic+1].normal()) * (1.0-tau) + (PtRight[ic].normal()+PtRight[ic+1].normal()) * tau;
                N.normalize();

                //1st triangle
                outstream << QString::asprintf("facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outstream << "    outer loop\n";
                Pt = PtLeft[ic]   * (1.0-tauA) + PtRight[ic]   * tauA;
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic+1] * (1.0-tauA) + PtRight[ic+1] * tauA;
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic]   * (1.0-tauB) + PtRight[ic]   * tauB;
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                outstream << "    endloop\n  endfacet\n";

                //2nd triangle
                outstream << QString::asprintf("facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outstream << "    outer loop\n";
                Pt = PtLeft[ic+1] * (1.0-tauA) + PtRight[ic+1] * tauA;
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic+1] * (1.0-tauB) + PtRight[ic+1] * tauB;
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic]   * (1.0-tauB) + PtRight[ic]   * tauB;
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                outstream << "    endloop\n  endfacet\n";
                iTriangles +=2;
            }
        }
    }

    Q_ASSERT(iTriangles==surfaces.count() * CHORDPANELS * SPANPANELS * 2 *2);

    //TIP PATCHES

    for (int j=0; j<surfaces.size(); j++)
    {
        Surface  const &surf = surfaces.at(j);

        if(surf.isTipLeft())
        {
            surf.getSidePoints_1(xfl::TOPSURFACE, nullptr, PtLeft,    PtRight, CHORDPANELS+1, xfl::COSINE);
            surf.getSidePoints_1(xfl::BOTSURFACE, nullptr, PtBotLeft, PtBotRight, CHORDPANELS+1, xfl::COSINE);

            N = surfaces.at(j).normal();
            N.rotateX(90.0);

            //L.E. triangle
            outstream << QString::asprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
            outstream << "    outer loop\n";
            outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[0].x, PtBotLeft[0].y, PtBotLeft[0].z);
            outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtLeft[1].x, PtLeft[1].y, PtLeft[1].z);
            outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[1].x, PtBotLeft[1].y, PtBotLeft[1].z);
            outstream << "    endloop\n  endfacet\n";
            iTriangles +=1;

            for(int ic=1; ic<CHORDPANELS-1; ic++)
            {
                //1st triangle
                outstream << QString::asprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outstream << "    outer loop\n";
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[ic].x, PtBotLeft[ic].y, PtBotLeft[ic].z);
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtLeft[ic].x, PtLeft[ic].y, PtLeft[ic].z);
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[ic+1].x, PtBotLeft[ic+1].y, PtBotLeft[ic+1].z);
                outstream << "    endloop\n  endfacet\n";
                //2nd triangle
                outstream << QString::asprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outstream << "    outer loop\n";
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[ic+1].x, PtBotLeft[ic+1].y, PtBotLeft[ic+1].z);
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtLeft[ic].x, PtLeft[ic].y, PtLeft[ic].z);
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtLeft[ic+1].x, PtLeft[ic+1].y, PtLeft[ic+1].z);
                outstream << "    endloop\n  endfacet\n";
                iTriangles +=2;
            }
            //T.E. triangle
            int ic = CHORDPANELS-1;
            outstream << QString::asprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
            outstream << "    outer loop\n";
            outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[ic].x, PtBotLeft[ic].y, PtBotLeft[ic].z);
            outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtLeft[ic].x, PtLeft[ic].y, PtLeft[ic].z);
            outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[ic+1].x, PtBotLeft[ic+1].y, PtBotLeft[ic+1].z);
            outstream << "    endloop\n  endfacet\n";
            iTriangles +=1;
        }

        if(surf.isTipRight())
        {
            surf.getSidePoints_1(xfl::TOPSURFACE, nullptr, PtLeft,    PtRight,    CHORDPANELS+1, xfl::COSINE);
            surf.getSidePoints_1(xfl::BOTSURFACE, nullptr, PtBotLeft, PtBotRight, CHORDPANELS+1, xfl::COSINE);

            N = surfaces.at(j).normal();
            N.rotateX(-90.0);

            //L.E. triangle
            outstream << QString::asprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
            outstream << "    outer loop\n";
            outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[0].x, PtBotRight[0].y, PtBotRight[0].z);
            outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtRight[1].x, PtRight[1].y, PtRight[1].z);
            outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[1].x, PtBotRight[1].y, PtBotRight[1].z);
            outstream << "    endloop\n  endfacet\n";
            iTriangles +=1;

            for(int ic=1; ic<CHORDPANELS-1; ic++)
            {
                //1st triangle
                outstream << QString::asprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outstream << "    outer loop\n";
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[ic].x, PtBotRight[ic].y, PtBotRight[ic].z);
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtRight[ic].x, PtRight[ic].y, PtRight[ic].z);
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[ic+1].x, PtBotRight[ic+1].y, PtBotRight[ic+1].z);
                outstream << "    endloop\n  endfacet\n";
                //2nd triangle
                outstream << QString::asprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outstream << "    outer loop\n";
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[ic+1].x, PtBotRight[ic+1].y, PtBotRight[ic+1].z);
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtRight[ic].x, PtRight[ic].y, PtRight[ic].z);
                outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtRight[ic+1].x, PtRight[ic+1].y, PtRight[ic+1].z);
                outstream << "    endloop\n  endfacet\n";
                iTriangles +=2;
            }
            //T.E. triangle
            int ic = CHORDPANELS-1;
            outstream << QString::asprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
            outstream << "    outer loop\n";
            outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[ic].x, PtBotRight[ic].y, PtBotRight[ic].z);
            outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtRight[ic].x, PtRight[ic].y, PtRight[ic].z);
            outstream << QString::asprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[ic+1].x, PtBotRight[ic+1].y, PtBotRight[ic+1].z);
            outstream << "    endloop\n  endfacet\n";
            iTriangles +=1;
        }
    }

    Q_ASSERT(iTriangles==nTriangles);

    strong = "endsolid " + name + "\n";
    outstream << strong;


    return nTriangles;
}


int STLWriterDlg::exportSailToSTLBinary(Sail *pSail, QDataStream &outStream, int CHORDPANELS, int SPANPANELS, double scalefactor) const
{
    // make a triangulation for export
    pSail->makeTriangulation(CHORDPANELS, SPANPANELS);
    return exportTriangulation(outStream, scalefactor, pSail->triangles());
}


void STLWriterDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("StlWriterDlg");
    {
        s_bBinary         = settings.value("Binary", true).toBool();
        s_iChordPanels    = settings.value("ChordPanels", 13).toInt();
        s_iSpanPanels     = settings.value("SpanPanels", 17).toInt();
        s_LengthUnitIndex = settings.value("LengthUnitIndex", 0).toInt();
        s_Geometry        = settings.value("WindowGeom", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void STLWriterDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("StlWriterDlg");
    {
        settings.setValue("Binary", s_bBinary);
        settings.setValue("ChordPanels", s_iChordPanels);
        settings.setValue("SpanPanels", s_iSpanPanels);
        settings.setValue("LengthUnitIndex",s_LengthUnitIndex);
        settings.setValue("WindowGeom", s_Geometry);
    }
    settings.endGroup();
}

