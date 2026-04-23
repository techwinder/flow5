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




#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include "panelcheckdlg.h"
#include <api/units.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>

QByteArray PanelCheckDlg::s_Geometry;

bool PanelCheckDlg::s_bCheckSkinny(false);
bool PanelCheckDlg::s_bCheckMinAngle(false);
bool PanelCheckDlg::s_bCheckMinArea(false);
bool PanelCheckDlg::s_bCheckMinSize(false);
bool PanelCheckDlg::s_bCheckQuadWarp(false);
double PanelCheckDlg::s_Quality(1.414);
double PanelCheckDlg::s_MinAngle(10.0);
double PanelCheckDlg::s_MinArea(0.00001);
double PanelCheckDlg::s_MinSize(0.001);
double PanelCheckDlg::s_MaxQuadWarp(10.0); //degrees


PanelCheckDlg::PanelCheckDlg(bool bQuads)
{
    m_bQuads = bQuads;

    setupLayout();

    m_pdeQuality->setValue(s_Quality);
    m_pdeMinAngle->setValue(s_MinAngle);
    m_pdeMinArea->setValue(s_MinArea*Units::m2toUnit());
    m_pdeMinSize->setValue(s_MinSize*Units::mtoUnit());
    m_pdeMaxQuadWarp->setValue(s_MaxQuadWarp);

    m_pchCheckSkinny->setEnabled(!m_bQuads);
    m_pchCheckQuadWarp->setEnabled(m_bQuads);

    m_pchCheckSkinny->setChecked(s_bCheckSkinny);
    m_pchCheckAngle->setChecked(s_bCheckMinAngle);
    m_pchCheckArea->setChecked(s_bCheckMinArea);
    m_pchCheckSize->setChecked(s_bCheckMinSize);
    m_pchCheckQuadWarp->setChecked(s_bCheckQuadWarp);

    m_pdeQuality->setEnabled(s_bCheckSkinny);
    m_pdeMinAngle->setEnabled(s_bCheckMinAngle);
    m_pdeMaxQuadWarp->setEnabled(s_bCheckQuadWarp);
    m_pdeMinSize->setEnabled(s_bCheckMinSize);
    m_pdeMinArea->setEnabled(s_bCheckMinArea);
}


void PanelCheckDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QGridLayout *pCheckLayout = new QGridLayout;
        {
            QLabel *pLabTriangles = new QLabel("Triangles only:");
            QLabel *pLabQuads     = new QLabel("Quads only:");
            QLabel *pLabAll       = new QLabel("All panels:");
            QLabel *pLabAngle0    = new QLabel("<p>&deg;</p>");
            QLabel *pLabAngle1    = new QLabel("<p>&deg;</p>");
            QLabel *pLabSize      = new QLabel(Units::lengthUnitQLabel());
            QLabel *pLabArea      = new QLabel(Units::areaUnitQLabel());

            m_pchCheckSkinny   = new QCheckBox("Skinny triangles");
            m_pchCheckAngle    = new QCheckBox("Check angles");
            m_pchCheckArea     = new QCheckBox("Check area");
            m_pchCheckSize     = new QCheckBox("Check small panels");
            m_pchCheckQuadWarp = new QCheckBox("Quad warp angle");

            m_pdeQuality  = new FloatEdit(s_Quality,3);
            m_pdeMinAngle = new FloatEdit(s_MinAngle,3);
            m_pdeMinArea  = new FloatEdit(s_MinArea*Units::m2toUnit());
            m_pdeMinSize  = new FloatEdit(s_MinSize*Units::mtoUnit());
            m_pdeMaxQuadWarp = new FloatEdit(s_MaxQuadWarp,3);

            QString qualitytip("<p>A triangle whose circumradius-to-shortest edge ratio is greater "
                               "than a minimum value B is said to be skinny.\n"
                               "Ruppert’s algorithm employs a bound of sqrt(2.0), and Chew’s second Delaunay "
                               "refinement algorithm employs a bound of 1.0. "
                               "All angles are bounded between 20.7° and 138.6° for Ruppert’s algorithm "
                               "and between 30° and 130° for Chew’s.<br>"
                               "J.R. Shewchuk in \"Delaunay Refinement Algorithms for Triangular Mesh Generation\"</p>");
            m_pchCheckSkinny->setToolTip(qualitytip);
            m_pdeQuality->setToolTip(qualitytip);

            QString warptip = QString("<p>The minimum dihedral angle formed by the planes intersecting in the diagonals</p>");
            m_pchCheckQuadWarp->setToolTip(warptip);
            m_pdeMaxQuadWarp->setToolTip(warptip);

            QString angletip = QString("<p>The panel's minimum internal angle</p>");
            m_pchCheckAngle->setToolTip(angletip);
            m_pdeMinAngle->setToolTip(angletip);

            QString areatip = QString("The panel's area");
            m_pchCheckArea->setToolTip(areatip);
            m_pdeMinArea->setToolTip(areatip);

            QString sizetip = QString("The panel's minimum edge length");
            m_pchCheckSize->setToolTip(sizetip);
            m_pdeMinSize->setToolTip(sizetip);

            pCheckLayout->addWidget(pLabTriangles,      1,1,1,4);
            pCheckLayout->addWidget(m_pchCheckSkinny,   2,2);
            pCheckLayout->addWidget(m_pdeQuality,       2,3);
            pCheckLayout->addWidget(pLabQuads,          3,1,1,4);
            pCheckLayout->addWidget(m_pchCheckQuadWarp, 4,2);
            pCheckLayout->addWidget(m_pdeMaxQuadWarp,   4,3);
            pCheckLayout->addWidget(pLabAngle1,         4,4);
            pCheckLayout->addWidget(pLabAll,            5,1,1,4);
            pCheckLayout->addWidget(m_pchCheckAngle,    6,2);
            pCheckLayout->addWidget(m_pdeMinAngle,      6,3);
            pCheckLayout->addWidget(pLabAngle0,         6,4);
            pCheckLayout->addWidget(m_pchCheckSize,     7,2);
            pCheckLayout->addWidget(m_pdeMinSize,       7,3);
            pCheckLayout->addWidget(pLabSize,           7,4);
            pCheckLayout->addWidget(m_pchCheckArea,     8,2);
            pCheckLayout->addWidget(m_pdeMinArea,       8,3);
            pCheckLayout->addWidget(pLabArea,           8,4);
            pCheckLayout->setColumnStretch(4,1);
        }

        QLabel *pLabAddPanelList   = new QLabel("Additional panel indexes to highlight");

        m_pptePanelIndexes = new PlainTextOutput;
        m_pptePanelIndexes->setReadOnly(false);
        m_pptePanelIndexes->setToolTip("<p>Enter the indexes of additional panels to highlight. "
                                       "Use a space separator.</p>");

        QLabel *pLabAddNodeList   = new QLabel("Additional node indexes to highlight:");

        m_ppteNodeIndexes = new PlainTextOutput;
        m_ppteNodeIndexes->setReadOnly(false);
        m_ppteNodeIndexes->setToolTip("<p>Enter the indexes of additional panels to highlight. "
                                      "Use a space separator.</p>");

        QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        {
            pButtonBox->button(QDialogButtonBox::Ok)->setText("Check");
            connect(pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }

        pMainLayout->addLayout(pCheckLayout);
        pMainLayout->addWidget(pLabAddPanelList);
        pMainLayout->addWidget(m_pptePanelIndexes);
        pMainLayout->addWidget(pLabAddNodeList);
        pMainLayout->addWidget(m_ppteNodeIndexes);
        pMainLayout->addWidget(pButtonBox);

        pMainLayout->setStretchFactor(m_pptePanelIndexes, 1);
        pMainLayout->setStretchFactor(m_ppteNodeIndexes,  1);
    }

    setLayout(pMainLayout);

    connect(m_pchCheckSkinny,   SIGNAL(clicked(bool)), SLOT(onCheckSkinny()));
    connect(m_pchCheckAngle,    SIGNAL(clicked(bool)), SLOT(onCheckMinAngle()));
    connect(m_pchCheckArea,     SIGNAL(clicked(bool)), SLOT(onCheckMinArea()));
    connect(m_pchCheckSize,     SIGNAL(clicked(bool)), SLOT(onCheckMinSize()));
    connect(m_pchCheckQuadWarp, SIGNAL(clicked(bool)), SLOT(onCheckQuadWarp()));
}


void PanelCheckDlg::accept()
{
    s_Quality     = m_pdeQuality->value();
    s_MinAngle    = m_pdeMinAngle->value();
    s_MinArea     = m_pdeMinArea->value()/Units::m2toUnit();
    s_MinSize     = m_pdeMinSize->value()/Units::mtoUnit();
    s_MaxQuadWarp = m_pdeMaxQuadWarp->value();
    QDialog::accept();
}


void PanelCheckDlg::onCheckSkinny()
{
    s_bCheckSkinny = m_pchCheckSkinny->isChecked();
    m_pdeQuality->setEnabled(s_bCheckSkinny);
}


void PanelCheckDlg::onCheckMinAngle()
{
    s_bCheckMinAngle = m_pchCheckAngle->isChecked();
    m_pdeMinAngle->setEnabled(s_bCheckMinAngle);
}


void PanelCheckDlg::onCheckMinArea()
{
    s_bCheckMinArea  = m_pchCheckArea->isChecked();
    m_pdeMinArea->setEnabled(s_bCheckMinArea);
}


void PanelCheckDlg::onCheckMinSize()
{
    s_bCheckMinSize  = m_pchCheckSize->isChecked();
    m_pdeMinSize->setEnabled(s_bCheckMinSize);
}


void PanelCheckDlg::onCheckQuadWarp()
{
    s_bCheckQuadWarp = m_pchCheckQuadWarp->isChecked();
    m_pdeMaxQuadWarp->setEnabled(m_bQuads && s_bCheckQuadWarp);
}


void PanelCheckDlg::setPanelIndexes(QVector<int> intvalues)
{
    QString strange;
    for(int i=0; i<intvalues.size(); i++)
    {
        strange.append(QString("%1\n").arg(intvalues.at(i)));
    }
    m_pptePanelIndexes->setPlainText(strange);
}


QVector<int> PanelCheckDlg::panelIndexes() const
{
    QString strange = m_pptePanelIndexes->toPlainText();
    QStringList list = strange.split(QRegularExpression("\\s+"));
    QVector<int> intlist;
    for(int i=0; i<list.size(); i++)
    {
        bool bOk = false;
        int val = list.at(i).toInt(&bOk);
        if(bOk) intlist.push_back(val);
    }
    std::sort(intlist.begin(), intlist.end());
    return intlist;
}


QVector<int> PanelCheckDlg::nodeIndexes() const
{
    QString strange = m_ppteNodeIndexes->toPlainText();
    QStringList list = strange.split(QRegularExpression("\\s+"));
    QVector<int> intlist;
    for(int i=0; i<list.size(); i++)
    {
        bool bOk = false;
        int val = list.at(i).toInt(&bOk);
        if(bOk) intlist.push_back(val);
    }
    std::sort(intlist.begin(), intlist.end());
    return intlist;
}


void PanelCheckDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    restoreGeometry(s_Geometry);
}


void PanelCheckDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void PanelCheckDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("PanelCheckDlg");
    {
        s_Geometry = settings.value("WindowGeometry").toByteArray();

        s_Quality     = settings.value("Quality",          s_Quality).toDouble();
        s_MinAngle    = settings.value("MinInternalAngle", s_MinAngle).toDouble();
        s_MinArea     = settings.value("MinArea",          s_MinArea).toDouble();
        s_MinSize     = settings.value("MinSize",          s_MinSize).toDouble();
        s_MaxQuadWarp = settings.value("MaxQuadWarp",      s_MaxQuadWarp).toDouble();
    }
    settings.endGroup();
}


void PanelCheckDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("PanelCheckDlg");
    {
        settings.setValue("WindowGeometry",   s_Geometry);
        settings.setValue("Quality",          s_Quality);
        settings.setValue("MinInternalAngle", s_MinAngle);
        settings.setValue("MinArea",          s_MinArea);
        settings.setValue("MinSize",          s_MinSize);
        settings.setValue("MaxQuadWarp",      s_MaxQuadWarp);
    }
    settings.endGroup();

}


