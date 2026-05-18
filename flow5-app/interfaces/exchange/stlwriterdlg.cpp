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


#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialogButtonBox>

#include "stlwriterdlg.h"

#include <core/xflcore.h>
#include <core/saveoptions.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>
#include <modules/xobjects.h>

#include <api/flow5-io.h>
#include <api/fusexfl.h>
#include <api/objects_global.h>
#include <api/planexfl.h>
#include <api/sail.h>
#include <api/wingxfl.h>


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
/*    m_prbBinary->setEnabled(false);
    m_prbASCII->setEnabled(false);*/
}


void STLWriterDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGroupBox *pgbExportFormat = new QGroupBox(tr("File format"));
        {
            QHBoxLayout *pFormatLayout = new QHBoxLayout;
            {
                m_prbBinary = new QRadioButton(tr("Binary"));
                m_prbASCII  = new QRadioButton("ASCII");
                pFormatLayout->addWidget(m_prbBinary);
                pFormatLayout->addWidget(m_prbASCII);
            }
            pgbExportFormat->setLayout(pFormatLayout);
        }

        m_plwNameList = new QListWidget;
        m_plwNameList->setSelectionMode(QAbstractItemView::MultiSelection);
        connect(m_plwNameList, SIGNAL(currentRowChanged(int)), this, SLOT(onSetLabels()));

        QGroupBox *pResolutionBox = new QGroupBox(tr("Output resolution"));
        {
            QGridLayout *pResolutionLayout = new QGridLayout;
            {
                m_plabChord = new QLabel("---");
                m_pieChordPanels = new IntEdit(17);

                m_plabSpan = new QLabel("---");
                m_pieSpanPanels = new IntEdit(17);

                pResolutionLayout->addWidget(m_plabChord,      1, 1);
                pResolutionLayout->addWidget(m_pieChordPanels, 1, 2);
                pResolutionLayout->addWidget(m_plabSpan,       2, 1);
                pResolutionLayout->addWidget(m_pieSpanPanels,  2, 2);
                pResolutionLayout->setColumnStretch(3,1);
            }
            pResolutionBox->setLayout(pResolutionLayout);
        }

        QHBoxLayout *pUnitLayout = new QHBoxLayout;
        {
            QLabel *pLabUnit = new QLabel(tr("Unit to write the file:"));
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
        {
            QPushButton *pExportButton  = new QPushButton(tr("Export part"));
            pButtonBox->addButton(pExportButton, QDialogButtonBox::ActionRole);
            connect(pExportButton, SIGNAL(clicked(bool)), this, SLOT(onExporttoSTL()));
            connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }


        m_ppto = new PlainTextOutput;

        pMainLayout->addWidget(pgbExportFormat);
        pMainLayout->addWidget(m_plwNameList);
        pMainLayout->addWidget(pResolutionBox);
        pMainLayout->addLayout(pUnitLayout);
        pMainLayout->addWidget(m_ppto);

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
            m_plwNameList->addItem(QString::fromStdString(m_pPlane->wingAt(i)->name()));
        }
        if(m_pPlane->hasFuse()) m_plwNameList->addItem(QString::fromStdString(m_pPlane->fuseAt(0)->name()));
    }
    else if(m_pFuse)
    {
        m_plwNameList->addItem(QString::fromStdString(m_pFuse->name()));
    }
    else if(m_pWing)
    {
        m_plwNameList->addItem(QString::fromStdString(m_pWing->name()));
    }
    else if(m_pSail)
    {
        m_plwNameList->addItem(QString::fromStdString(m_pSail->name()));
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
        m_plabChord->setText(tr("Number of x-panels"));
        m_plabSpan->setText(tr("Number of hoop panels"));
    }
    else if(m_pWing)
    {
        m_plabChord->setText(tr("Number of chordwise panels"));
        m_plabSpan->setText(tr("Number of span panels per surface"));
    }
    else if(m_pSail)
    {
        m_plabChord->setText(tr("Number of chordwise panels"));
        m_plabSpan->setText(tr("Number of z-panels"));
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

    if      (m_pFuse)  FileName = QString::fromStdString(m_pFuse->name()).trimmed();
    else if (m_pWing)  FileName = QString::fromStdString(m_pWing->name()).trimmed();
    else if (m_pSail)  FileName = QString::fromStdString(m_pSail->name()).trimmed();
    else if (m_pPlane) FileName = QString::fromStdString(m_pPlane->name()).trimmed();
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

    int nTriangles = 0;
    if(m_pPlane)
    {
        std::vector<Triangle3d> triangles;
        for(int i=0; i<m_SelectedList.size(); i++)
        {
            WingXfl const *pWing = m_pPlane->wingFromName(m_SelectedList.at(i).toStdString());
            Fuse const *pFuse = m_pPlane->fuseFromName(m_SelectedList.at(i).toStdString());
            if(pWing)
            {
                makeSTLTriangulation(pWing, triangles, s_iChordPanels, s_iSpanPanels, 1.0);
            }
            else if(pFuse)
            {
                triangles.insert(triangles.end(), pFuse->triangles().begin(), pFuse->triangles().end());
            }
        }
        if(bBinary)
        {
            nTriangles = io::exportTriangulationToSTLBinary(FileName, 1.0, triangles);
        }
        else
        {
            nTriangles = io::exportTriangulationToSTLText(FileName, 1.0, triangles);
        }
    }
    else if(m_pWing)
    {
            std::vector<Triangle3d> triangles;
            makeSTLTriangulation(m_pWing, triangles, s_iChordPanels, s_iSpanPanels, 1.0);
        if(bBinary)
        {
            nTriangles = io::exportTriangulationToSTLBinary(FileName, 1.0, triangles);
        }
        else
        {
            nTriangles = io::exportTriangulationToSTLText(FileName, 1.0, triangles);
        }
    }
    else if(m_pFuse)
    {
        if(bBinary)
        {
            nTriangles = io::exportTriangulationToSTLBinary(FileName, m_UnitFactor, m_pFuse->triangles());
        }
        else
        {
//            QTextStream out(&XFile);
        }
    }
    else if(m_pSail)
    {
        Objects3d::makeSailTriangulation(m_pSail, s_iChordPanels, s_iSpanPanels);
        if(bBinary)
        {
            nTriangles = io::exportTriangulationToSTLBinary(FileName, m_UnitFactor, m_pSail->triangles());
        }
        else
        {
            nTriangles = io::exportTriangulationToSTLText(FileName, m_UnitFactor, m_pSail->triangles());

        }
    }

    m_ppto->insertPlainText("The part has been successfully exported to the STL file\n");
    QString strong;
    strong = QString::asprintf("Total triangles: %d\n\n", nTriangles);
    m_ppto->insertPlainText(strong);

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


/** For STL export --> fuse intersections not taken into acount */
void STLWriterDlg::makeSTLTriangulation(const WingXfl *pWingXfl, std::vector<Triangle3d> &triangles, int CHORDPANELS, int SPANPANELS, double scalefactor) const
{
//    int nTriangles = m_Surface.count() * CHORDPANELS * SPANPANELS * 2 *2  + 2* ((CHORDPANELS-2) * 2 + 2);
    Vector3d Pt[3];
    std::vector<Node> PtTopLeft(CHORDPANELS+1);
    std::vector<Node> PtTopRight(CHORDPANELS+1);
    std::vector<Node> PtBotLeft(CHORDPANELS+1);
    std::vector<Node> PtBotRight(CHORDPANELS+1);
    std::vector<Vector3d> NA(CHORDPANELS+1), NB(CHORDPANELS+1);
    std::vector<double> xdistrib;
    xfl::getPointDistribution(xdistrib, CHORDPANELS, xfl::COSINE); // ensures good resolution at LE and TE

    for (int j=0; j<pWingXfl->nSurfaces(); j++)
    {
        Surface const &surf = pWingXfl->surfaceAt(j);
        Foil const *pFoilA = surf.foilA();
        Foil const *pFoilB = surf.foilB();

        for(int is=0; is<SPANPANELS; is++)
        {
            surf.getSidePoints(xfl::TOPSURFACE, nullptr, PtTopLeft, PtTopRight, NA, NB, xdistrib, xdistrib);
            for(uint ip=0; ip<PtTopLeft.size(); ip++) PtTopLeft[ip] *=scalefactor;
            for(uint ip=0; ip<PtTopLeft.size(); ip++) PtTopRight[ip]*=scalefactor;

            surf.getSidePoints(xfl::BOTSURFACE, nullptr, PtBotLeft, PtBotRight, NA, NB, xdistrib, xdistrib);
            for(uint ip=0; ip<PtBotLeft.size(); ip++) PtBotLeft[ip] *=scalefactor;
            for(uint ip=0; ip<PtBotLeft.size(); ip++) PtBotRight[ip]*=scalefactor;

            double tauA = double(is)   /double(SPANPANELS);
            double tauB = double(is+1) /double(SPANPANELS);

            //top surface
            for(int ic=0; ic<CHORDPANELS; ic++)
            {
                //left side vertices
                //1st triangle
                Pt[0] = PtTopLeft[ic]   * (1.0-tauA) + PtTopRight[ic]   * tauA;
                Pt[1] = PtTopLeft[ic+1] * (1.0-tauA) + PtTopRight[ic+1] * tauA;
                Pt[2] = PtTopLeft[ic]   * (1.0-tauB) + PtTopRight[ic]   * tauB;
                Triangle3d t3d0(Pt);
                if(!t3d0.isNull()) triangles.push_back(t3d0);

                //2nd triangle
                Pt[0] = PtTopLeft[ic+1] * (1.0-tauA) + PtTopRight[ic+1] * tauA;
                Pt[1] = PtTopLeft[ic+1] * (1.0-tauB) + PtTopRight[ic+1] * tauB;
                Pt[2] = PtTopLeft[ic]   * (1.0-tauB) + PtTopRight[ic]   * tauB;
                Triangle3d t3d1(Pt);
                if(!t3d1.isNull()) triangles.push_back(t3d1);
            }

            // bot surface
            for(int ic=0; ic<CHORDPANELS; ic++)
            {
                //1st triangle
                Pt[0] = PtBotLeft[ic]   * (1.0-tauA) + PtBotRight[ic]   * tauA;
                Pt[1] = PtBotLeft[ic]   * (1.0-tauB) + PtBotRight[ic]   * tauB;
                Pt[2] = PtBotLeft[ic+1] * (1.0-tauA) + PtBotRight[ic+1] * tauA;
                Triangle3d t3d0(Pt);
                if(!t3d0.isNull()) triangles.push_back(t3d0);

                //2nd triangle
                Pt[0] = PtBotLeft[ic+1] * (1.0-tauA) + PtBotRight[ic+1] * tauA;
                Pt[1] = PtBotLeft[ic]   * (1.0-tauB) + PtBotRight[ic]   * tauB;
                Pt[2] = PtBotLeft[ic+1] * (1.0-tauB) + PtBotRight[ic+1] * tauB;
                Triangle3d t3d1(Pt);
                if(!t3d1.isNull()) triangles.push_back(t3d1);
            }

            // Trailing edge panels if gap on either side
            if(pFoilA->TEGap()>0 || pFoilB->TEGap()>0)
            {
                //1st triangle
                Pt[0] = PtBotLeft.back() * (1.0-tauA) + PtBotRight.back() * tauA;
                Pt[1] = PtTopLeft.back() * (1.0-tauB) + PtTopRight.back() * tauB;
                Pt[2] = PtTopLeft.back() * (1.0-tauA) + PtTopRight.back() * tauA;
                Triangle3d t3d0(Pt);
                if(!t3d0.isNull()) triangles.push_back(t3d0);

                //2nd triangle
                Pt[0] = PtBotLeft.back() * (1.0-tauA) + PtBotRight.back() * tauA;
                Pt[1] = PtBotLeft.back() * (1.0-tauB) + PtBotRight.back() * tauB;
                Pt[2] = PtTopLeft.back() * (1.0-tauB) + PtTopRight.back() * tauB;
                Triangle3d t3d1(Pt);
                if(!t3d1.isNull()) triangles.push_back(t3d1);
            }
        }
    }

    //TIP PATCHES
    for (int j=0; j<pWingXfl->nSurfaces(); j++)
    {
        Surface const &surf = pWingXfl->surfaceAt(j);
        if(surf.isTipLeft())
        {
            surf.getSidePoints(xfl::TOPSURFACE, nullptr, PtTopLeft, PtTopRight, NA, NB, xdistrib, xdistrib);
            for(uint ip=0; ip<PtTopLeft.size(); ip++) PtTopLeft[ip] *=scalefactor;
            surf.getSidePoints(xfl::BOTSURFACE, nullptr, PtBotLeft, PtBotRight, NA, NB, xdistrib, xdistrib);
            for(uint ip=0; ip<PtTopLeft.size(); ip++) PtBotLeft[ip] *=scalefactor;

            for(int ic=0; ic<CHORDPANELS; ic++)
            {
                //1st triangle
                Triangle3d t3d0({PtBotLeft[ic], PtTopLeft[ic+1], PtTopLeft[ic]});
                if(!t3d0.isNull()) triangles.push_back(t3d0);

                //2nd triangle
                Triangle3d t3d1({PtBotLeft[ic], PtBotLeft[ic+1], PtTopLeft[ic+1]});
                if(!t3d1.isNull()) triangles.push_back(t3d1);
            }
        }

        if(surf.isTipRight())
        {
            surf.getSidePoints(xfl::TOPSURFACE, nullptr, PtTopLeft, PtTopRight, NA, NB, xdistrib, xdistrib);
            for(uint ip=0; ip<PtTopRight.size(); ip++) PtTopRight[ip]*=scalefactor;
            surf.getSidePoints(xfl::BOTSURFACE, nullptr, PtBotLeft, PtBotRight, NA, NB, xdistrib, xdistrib);
            for(uint ip=0; ip<PtBotRight.size(); ip++) PtBotRight[ip]*=scalefactor;

            for(int ic=0; ic<CHORDPANELS; ic++)
            {
                //1st triangle
                Triangle3d t3d0({PtBotRight[ic], PtTopRight[ic], PtTopRight[ic+1]});
                if(!t3d0.isNull()) triangles.push_back(t3d0);

                //2nd triangle
                Triangle3d t3d1({PtBotRight[ic], PtTopRight[ic+1], PtBotRight[ic+1]});
                if(!t3d1.isNull()) triangles.push_back(t3d1);
            }
        }
    }
}
