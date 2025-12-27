/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
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

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCompleter>
#include <QDir>
#include <QDirIterator>
#include <QFileSystemModel>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QMessageBox>

#include "saveoptionswt.h"

#include <core/saveoptions.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/view2d/foilsvgwriter.h>

#include <api/fileio.h>

SaveOptionsWt::SaveOptionsWt(QWidget *parent) : QWidget{parent}
{
    setupLayout();
}

void SaveOptionsWt::setupLayout()
{
//    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    m_pGroupBox.push_back(new QGroupBox("General"));
    {
        QVBoxLayout *pGeneralLayout = new QVBoxLayout;
        {
            m_pchAutoLoadLast = new QCheckBox("Load last project on startup");

            QHBoxLayout *pSaveTimerLayout = new QHBoxLayout;
            {
                m_pchAutoSave = new QCheckBox("Autosave every");
                m_pieSaveInterval = new IntEdit(SaveOptions::s_SaveInterval);
                QLabel *plabMinutes = new QLabel("mn");
                pSaveTimerLayout->addWidget(m_pchAutoSave);
                pSaveTimerLayout->addWidget(m_pieSaveInterval);
                pSaveTimerLayout->addWidget(plabMinutes);
                pSaveTimerLayout->addStretch();

                connect(m_pchAutoSave, SIGNAL(clicked(bool)), m_pieSaveInterval, SLOT(setEnabled(bool)));

            }
            pGeneralLayout->addWidget(m_pchAutoLoadLast);
            pGeneralLayout->addLayout(pSaveTimerLayout);
        }
        m_pGroupBox.back()->setLayout(pGeneralLayout);
    }

    m_pGroupBox.push_back(new QGroupBox("Directories"));
    {
        QGridLayout *pDirLayout = new QGridLayout;
        {
            QHBoxLayout *pLastDirLayout = new QHBoxLayout;
            {
                QLabel *plabLast           = new QLabel("Project files:");
                QButtonGroup *pGroup = new QButtonGroup(this);
                {
                    m_prbUseLastDir = new QRadioButton("Use last used directory");
                    m_prbUseFixedDir = new QRadioButton("Use fixed directory");
                    pGroup->addButton(m_prbUseLastDir);
                    pGroup->addButton(m_prbUseFixedDir);
                    connect(m_prbUseLastDir,  SIGNAL(clicked()), SLOT(onLastUsedDir()));
                    connect(m_prbUseFixedDir, SIGNAL(clicked()), SLOT(onLastUsedDir()));
                }
                pLastDirLayout->addWidget(plabLast);
                pLastDirLayout->addWidget(m_prbUseLastDir);
                pLastDirLayout->addWidget(m_prbUseFixedDir);
                pLastDirLayout->addStretch();
            }

            m_pleLastDir     = new QLineEdit;
            {
                QCompleter *pComp = new QCompleter;
                QFileSystemModel *pDirModel = new QFileSystemModel;
                pDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
                pComp->setModel(pDirModel);
                m_pleLastDir->setCompleter(pComp);
            }
            m_pleDatFoilDir     = new QLineEdit;
            {
                QCompleter *pComp = new QCompleter;
                QFileSystemModel *pDirModel = new QFileSystemModel;
                pDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
                pComp->setModel(pDirModel);
                m_pleDatFoilDir->setCompleter(pComp);
            }

            m_plePlrPolarDir    = new QLineEdit;
            {
                QCompleter *pComp = new QCompleter;
                QFileSystemModel *pDirModel = new QFileSystemModel;
                pDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
                pComp->setModel(pDirModel);
                m_plePlrPolarDir->setCompleter(pComp);
            }

            m_pleXmlPolarDir    = new QLineEdit;
            {
                QCompleter *pComp = new QCompleter;
                QFileSystemModel *pDirModel = new QFileSystemModel;
                pDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
                pComp->setModel(pDirModel);
                m_pleXmlPolarDir->setCompleter(pComp);
            }
            m_pleXmlPlaneDir    = new QLineEdit;
            {
                QCompleter *pComp = new QCompleter;
                QFileSystemModel *pDirModel = new QFileSystemModel;
                pDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
                pComp->setModel(pDirModel);
                m_pleXmlPlaneDir->setCompleter(pComp);
            }
            m_pleXmlWPolarDir   = new QLineEdit;
            {
                QCompleter *pComp = new QCompleter;
                QFileSystemModel *pDirModel = new QFileSystemModel;
                pDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
                pComp->setModel(pDirModel);
                m_pleXmlWPolarDir->setCompleter(pComp);
            }
            m_pleXmlScriptDir   = new QLineEdit;
            {
                QCompleter *pComp = new QCompleter;
                QFileSystemModel *pDirModel = new QFileSystemModel;
                pDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
                pComp->setModel(pDirModel);
                m_pleXmlScriptDir->setCompleter(pComp);
            }
            m_pleCADDir         = new QLineEdit;
            {
                QCompleter *pComp = new QCompleter;
                QFileSystemModel *pDirModel = new QFileSystemModel;
                pDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
                pComp->setModel(pDirModel);
                m_pleCADDir->setCompleter(pComp);
            }

            m_pleSTLDir         = new QLineEdit;
            {
                QCompleter *pComp = new QCompleter;
                QFileSystemModel *pDirModel = new QFileSystemModel;
                pDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
                pComp->setModel(pDirModel);
                m_pleSTLDir->setCompleter(pComp);
            }
            m_pleTempDir        = new QLineEdit;
            {
                QCompleter *pComp = new QCompleter;
                QFileSystemModel *pDirModel = new QFileSystemModel;
                pDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
                pComp->setModel(pDirModel);
                m_pleTempDir->setCompleter(pComp);
            }
            connect(m_pleTempDir, SIGNAL(editingFinished()), SLOT(onCheckTempDir()));
            m_pleApplicationDir = new QLineEdit;
            QLabel *plabDatDir         = new QLabel("Foil files:");
            QLabel *plabPlrDir         = new QLabel("Foil polar files:");
            QLabel *plabXmlPolarDir    = new QLabel("XML foil analysis files:");
            QLabel *plabXmlPlaneDir    = new QLabel("XML plane and boat files:");
            QLabel *plabXmlWPolarDir   = new QLabel("XML plane and boat analysis files:");
            QLabel *plabXMLScript      = new QLabel("XML script files:");
            QLabel *plabCAD            = new QLabel("CAD files:");
            QLabel *plabSTL            = new QLabel("STL files:");
            QLabel *plabTemp           = new QLabel("Temporary files:");
            QLabel *plapAppDir         = new QLabel("Application directory:");


            QToolButton *pActiveDirBtn = new QToolButton();
            QAction *pActiveDirAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), "Set directory", this);
            pActiveDirBtn->setDefaultAction(pActiveDirAction);
            connect(pActiveDirAction, SIGNAL(triggered(bool)), this, SLOT(onActiveDir()));

            QToolButton *pDatFoilDirBtn = new QToolButton();
            QAction *pDatFoilDirAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), "Set directory", this);
            pDatFoilDirBtn->setDefaultAction(pDatFoilDirAction);
            connect(pDatFoilDirAction, SIGNAL(triggered(bool)), this, SLOT(onDatFoilDir()));

            QToolButton *pPlrPolarDirBtn = new QToolButton();
            QAction *pPlrPolarDirAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), "Set directory", this);
            pPlrPolarDirBtn->setDefaultAction(pPlrPolarDirAction);
            connect(pPlrPolarDirAction, SIGNAL(triggered(bool)), this, SLOT(onPlrPolarDir()));

            QToolButton *pXmlPolarDirBtn = new QToolButton();
            QAction *pXmlPolarDirAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), "Set directory", this);
            pXmlPolarDirBtn->setDefaultAction(pXmlPolarDirAction);
            connect(pXmlPolarDirAction, SIGNAL(triggered(bool)), this, SLOT(onXmlPolarDir()));

            QToolButton *pXmlPlaneDirBtn = new QToolButton();
            QAction *pXmlPlaneDirAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), "Set directory", this);
            pXmlPlaneDirBtn->setDefaultAction(pXmlPlaneDirAction);
            connect(pXmlPlaneDirAction, SIGNAL(triggered(bool)), this, SLOT(onXmlPlaneDir()));

            QToolButton *pXmlWPolarDirBtn = new QToolButton();
            QAction *pXmlWPolarDirAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), "Set directory", this);
            pXmlWPolarDirBtn->setDefaultAction(pXmlWPolarDirAction);
            connect(pXmlWPolarDirAction, SIGNAL(triggered(bool)), this, SLOT(onXmlWPolarDir()));

            QToolButton *pXMLScriptDirBtn = new QToolButton();
            QAction *pXMLScriptDirAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), "Set directory", this);
            pXMLScriptDirBtn->setDefaultAction(pXMLScriptDirAction);
            connect(pXMLScriptDirAction, SIGNAL(triggered(bool)), this, SLOT(onXmlScriptDir()));

            QToolButton *pCADDirBtn = new QToolButton();
            QAction *pCADDirAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), "Set directory", this);
            pCADDirBtn->setDefaultAction(pCADDirAction);
            connect(pCADDirAction, SIGNAL(triggered(bool)), this, SLOT(onCADDir()));

            QToolButton *pSTLDirBtn = new QToolButton();
            QAction *pSTLDirAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), "Set directory", this);
            pSTLDirBtn->setDefaultAction(pSTLDirAction);
            connect(pSTLDirAction, SIGNAL(triggered(bool)), this, SLOT(onSTLDir()));

            QToolButton *pTmpDirBtn = new QToolButton();
            QAction *pTmpDirAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), "Set directory", this);
            pTmpDirBtn->setDefaultAction(pTmpDirAction);
            connect(pTmpDirAction, SIGNAL(triggered(bool)), this, SLOT(onTempDir()));

            m_pchCleanOnExit = new QCheckBox("Clean log files on exit");

            pDirLayout->addLayout(pLastDirLayout,      1, 1, 1, 3);
            pDirLayout->addWidget(plabDatDir,          3, 1);
            pDirLayout->addWidget(plabPlrDir,          4, 1);
            pDirLayout->addWidget(plabXmlPolarDir,     7, 1);
            pDirLayout->addWidget(plabXmlPlaneDir,     8, 1);
            pDirLayout->addWidget(plabXmlWPolarDir,    9, 1);
            pDirLayout->addWidget(plabXMLScript,       10,1);
            pDirLayout->addWidget(plabCAD,             11,1);
            pDirLayout->addWidget(plabSTL,             12,1);
            pDirLayout->addWidget(plabTemp,            13,1);
            pDirLayout->addWidget(plapAppDir,          14,1);

            pDirLayout->addWidget(m_pleLastDir,        2, 2);
            pDirLayout->addWidget(m_pleDatFoilDir,     3, 2);
            pDirLayout->addWidget(m_plePlrPolarDir,    4, 2);
            pDirLayout->addWidget(m_pleXmlPolarDir,    7, 2);
            pDirLayout->addWidget(m_pleXmlPlaneDir,    8, 2);
            pDirLayout->addWidget(m_pleXmlWPolarDir,   9, 2);
            pDirLayout->addWidget(m_pleXmlScriptDir,   10,2);
            pDirLayout->addWidget(m_pleCADDir,         11,2);
            pDirLayout->addWidget(m_pleSTLDir,         12,2);
            pDirLayout->addWidget(m_pleTempDir,        13,2);
            pDirLayout->addWidget(m_pleApplicationDir, 14,2);

            pDirLayout->addWidget(pActiveDirBtn,       2, 3);
            pDirLayout->addWidget(pDatFoilDirBtn,      3, 3);
            pDirLayout->addWidget(pPlrPolarDirBtn,     4, 3);
            pDirLayout->addWidget(pXmlPolarDirBtn,     7, 3);
            pDirLayout->addWidget(pXmlPlaneDirBtn,     8, 3);
            pDirLayout->addWidget(pXmlWPolarDirBtn,    9, 3);
            pDirLayout->addWidget(pXMLScriptDirBtn,    10,3);
            pDirLayout->addWidget(pCADDirBtn,          11,3);
            pDirLayout->addWidget(pSTLDirBtn,          12,3);
            pDirLayout->addWidget(pTmpDirBtn,          13,3);
            pDirLayout->addWidget(m_pchCleanOnExit,    15,2);
        }
        m_pGroupBox.back()->setLayout(pDirLayout);
    }

    m_pGroupBox.push_back(new QGroupBox("Export options"));
    {
        QVBoxLayout *pExportOptionsLayout = new QVBoxLayout;
        {
            QGroupBox *pgbTextExport = new QGroupBox("To text files");
            {
                QHBoxLayout *pTextExportLayout = new QHBoxLayout;
                {
                    m_pTXT = new QRadioButton("TXT");
                    m_pCSV = new QRadioButton("CSV");
                    QLabel *pLabCsv = new QLabel("CSV field separator");
                    pLabCsv->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
                    m_pleCsvSeparator = new QLineEdit;

                    pTextExportLayout->addWidget(m_pTXT);
                    pTextExportLayout->addWidget(m_pCSV);
                    pTextExportLayout->addStretch();
                    pTextExportLayout->addWidget(pLabCsv);
                    pTextExportLayout->addWidget(m_pleCsvSeparator);

                    connect(m_pTXT, SIGNAL(clicked(bool)), SLOT(onExportFormat()));
                    connect(m_pCSV, SIGNAL(clicked(bool)), SLOT(onExportFormat()));
                }
                pgbTextExport->setLayout(pTextExportLayout);
            }

            QGroupBox *pgbXmlExport = new QGroupBox("To Xml files");
            {
                QHBoxLayout *pXmlExportLayout = new QHBoxLayout;
                {
                    m_pchXmlWingFoils = new QCheckBox("Include airfoil .dat files when exporting wings");
                    pXmlExportLayout->addWidget(m_pchXmlWingFoils);
                }
                pgbXmlExport->setLayout(pXmlExportLayout);
            }
            pExportOptionsLayout->addWidget(pgbTextExport);
            pExportOptionsLayout->addWidget(pgbXmlExport);
        }

        m_pGroupBox.back()->setLayout(pExportOptionsLayout);
    }

    m_pGroupBox.push_back(new QGroupBox("SVG foil export"));
    {
        QVBoxLayout *pExportLayout = new QVBoxLayout;
        {
            m_pchSVGCloseTE = new QCheckBox("Close foil TE");
            m_pchSVGFillFoil = new QCheckBox("Fill foil");
            m_pdeSVGScaleFactor = new FloatEdit;
            m_pdeSVGScaleFactor->setToolTip("Recommendation: Scale factor>=10000 for a smooth foil shape");
            m_pchSVGExportStyle = new QCheckBox("Export style");
            m_pdeSVGMargin = new FloatEdit;
            pExportLayout->addWidget(m_pchSVGCloseTE);
            pExportLayout->addWidget(m_pchSVGFillFoil);
            pExportLayout->addWidget(m_pchSVGExportStyle);
            QGridLayout *pScaleLayout = new QGridLayout;
            {
                pScaleLayout->addWidget(new QLabel("Margin"),       1,1);
                pScaleLayout->addWidget(m_pdeSVGMargin,             1,2);
                pScaleLayout->addWidget(new QLabel("(Foil units)"), 1,3);
                pScaleLayout->addWidget(new QLabel("Scale factor"), 2,1);
                pScaleLayout->addWidget(m_pdeSVGScaleFactor,        2,2);
                pScaleLayout->setColumnStretch(4,1);
            }
            pExportLayout->addLayout(pScaleLayout);
            pExportLayout->addStretch();
        }
        m_pGroupBox.back()->setLayout(pExportLayout);
    }

    m_pGroupBox.push_back(new QGroupBox("Operating points"));
    {
        QHBoxLayout *pSaveOppLayout = new QHBoxLayout;
        {
            QLabel *pSaveLabel = new QLabel("Save:");
            pSaveLabel->setAlignment(Qt::AlignRight);
            m_pchOpps  = new QCheckBox("Foil operating points");
            m_pchPOpps = new QCheckBox("Plane operating points");
            m_pchBtOpps = new QCheckBox("Boat operating points");
            pSaveOppLayout->addWidget(pSaveLabel);
            pSaveOppLayout->addStretch();
            pSaveOppLayout->addWidget(m_pchOpps);
            pSaveOppLayout->addWidget(m_pchPOpps);
            pSaveOppLayout->addWidget(m_pchBtOpps);
            pSaveOppLayout->addStretch();
        }
        m_pGroupBox.back()->setLayout(pSaveOppLayout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        for(int i=0; i<m_pGroupBox.size(); i++)
        {
            pMainLayout->addWidget(m_pGroupBox.at(i));
        }
        pMainLayout->addStretch();
    }
    setLayout(pMainLayout);
}


void SaveOptionsWt::initWidget()
{
    QFileSystemModel *pModel = nullptr;
    m_pchOpps->setChecked(FileIO::bOpps());
    m_pchPOpps->setChecked(FileIO::bPOpps());
    m_pchBtOpps->setChecked(FileIO::bBtOpps());

    m_pchAutoLoadLast->setChecked(SaveOptions::bAutoLoadLast());
    m_pchAutoSave->setChecked(SaveOptions::bAutoSave());
    m_pieSaveInterval->setValue(SaveOptions::s_SaveInterval);
    m_pieSaveInterval->setEnabled(SaveOptions::bAutoSave());

    m_prbUseLastDir->setChecked(SaveOptions::s_bUseLastDir);
    m_prbUseFixedDir->setChecked(!SaveOptions::s_bUseLastDir);
    m_pleLastDir->setEnabled(!SaveOptions::s_bUseLastDir);

    m_pleLastDir->setText(QFileInfo(SaveOptions::lastDirName()).canonicalFilePath());
    pModel  = dynamic_cast<QFileSystemModel*>(m_pleLastDir->completer()->model());
    pModel->setRootPath(SaveOptions::lastDirName());

    m_pleDatFoilDir->setText(QFileInfo(SaveOptions::datFoilDirName()).canonicalFilePath());
    pModel  = dynamic_cast<QFileSystemModel*>(m_pleDatFoilDir->completer()->model());
    pModel->setRootPath(SaveOptions::datFoilDirName());

    m_plePlrPolarDir->setText(QFileInfo(SaveOptions::plrPolarDirName()).canonicalFilePath());
    pModel  = dynamic_cast<QFileSystemModel*>(m_plePlrPolarDir->completer()->model());
    pModel->setRootPath(SaveOptions::plrPolarDirName());

    m_pleXmlPolarDir->setText(QFileInfo(SaveOptions::xmlPolarDirName()).canonicalFilePath());
    pModel  = dynamic_cast<QFileSystemModel*>(m_pleXmlPolarDir->completer()->model());
    pModel->setRootPath(SaveOptions::xmlPolarDirName());

    m_pleXmlPlaneDir->setText(QFileInfo(SaveOptions::xmlPlaneDirName()).canonicalFilePath());
    pModel  = dynamic_cast<QFileSystemModel*>(m_pleXmlPlaneDir->completer()->model());
    pModel->setRootPath(SaveOptions::xmlPlaneDirName());

    m_pleXmlWPolarDir->setText(QFileInfo(SaveOptions::xmlWPolarDirName()).canonicalFilePath());
    pModel  = dynamic_cast<QFileSystemModel*>(m_pleXmlWPolarDir->completer()->model());
    pModel->setRootPath(SaveOptions::xmlWPolarDirName());

    m_pleXmlScriptDir->setText(QFileInfo(SaveOptions::xmlScriptDirName()).canonicalFilePath());
    pModel  = dynamic_cast<QFileSystemModel*>(m_pleXmlScriptDir->completer()->model());
    pModel->setRootPath(SaveOptions::xmlScriptDirName());

    m_pleCADDir->setText(QFileInfo(SaveOptions::CADDirName()).canonicalFilePath());
    pModel  = dynamic_cast<QFileSystemModel*>(m_pleCADDir->completer()->model());
    pModel->setRootPath(SaveOptions::CADDirName());

    m_pleSTLDir->setText(QFileInfo(SaveOptions::STLDirName()).canonicalFilePath());
    pModel  = dynamic_cast<QFileSystemModel*>(m_pleSTLDir->completer()->model());
    pModel->setRootPath(SaveOptions::STLDirName());

    m_pleTempDir->setText(QFileInfo(SaveOptions::tempDirName()).canonicalFilePath());
    pModel  = dynamic_cast<QFileSystemModel*>(m_pleTempDir->completer()->model());
    pModel->setRootPath(SaveOptions::tempDirName());


    m_pleApplicationDir->setText(qApp->applicationDirPath());
    m_pleApplicationDir->setReadOnly(true);
    m_pleApplicationDir->setEnabled(false);

    m_pCSV->setChecked(SaveOptions::s_ExportFileType==xfl::CSV);
    m_pTXT->setChecked(SaveOptions::s_ExportFileType==xfl::TXT);

    m_pleCsvSeparator->setText(SaveOptions::s_CsvSeparator);
    m_pleCsvSeparator->setEnabled(SaveOptions::s_ExportFileType==xfl::CSV);

    m_pchXmlWingFoils->setChecked(SaveOptions::s_bXmlWingFoils);

    m_pchSVGCloseTE->setChecked(FoilSVGWriter::bSVGClosedTE());
    m_pchSVGFillFoil->setChecked(FoilSVGWriter::bSVGFillFoil());
    m_pchSVGExportStyle->setChecked(FoilSVGWriter::bSVGExportStyle());
    m_pdeSVGScaleFactor->setValue(FoilSVGWriter::SVGScaleFactor());
    m_pdeSVGMargin->setValue(FoilSVGWriter::SVGMargin());
    m_pchCleanOnExit->setChecked(SaveOptions::s_bCleanOnExit);
}


void SaveOptionsWt::readData()
{
    SaveOptions::s_bUseLastDir = m_prbUseLastDir->isChecked();

    SaveOptions::s_bAutoLoadLast = m_pchAutoLoadLast->isChecked();
    FileIO::saveOpps(  m_pchOpps->isChecked());
    FileIO::savePOpps( m_pchPOpps->isChecked());
    FileIO::saveBtOpps(m_pchBtOpps->isChecked());
    SaveOptions::s_bAutoSave     = m_pchAutoSave->isChecked();
    SaveOptions::s_bXmlWingFoils = m_pchXmlWingFoils->isChecked();

    SaveOptions::s_SaveInterval = m_pieSaveInterval->value();

    SaveOptions::s_LastDirName      = m_pleLastDir->text();
    SaveOptions::s_datFoilDirName   = m_pleDatFoilDir->text();
    SaveOptions::s_plrPolarDirName  = m_plePlrPolarDir->text();
    SaveOptions::s_xmlPolarDirName  = m_pleXmlPolarDir->text();
    SaveOptions::s_xmlPlaneDirName  = m_pleXmlPlaneDir->text();
    SaveOptions::s_xmlWPolarDirName = m_pleXmlWPolarDir->text();
    SaveOptions::s_xmlScriptDirName = m_pleXmlScriptDir->text();
    SaveOptions::s_CADDirName       = m_pleCADDir->text();
    SaveOptions::s_STLDirName       = m_pleSTLDir->text();
    SaveOptions::s_TempDirName      = m_pleTempDir->text();

    if(m_pCSV->isChecked()) SaveOptions::s_ExportFileType = xfl::CSV;
    else                    SaveOptions::s_ExportFileType = xfl::TXT;
    SaveOptions::s_CsvSeparator = m_pleCsvSeparator->text();

    FoilSVGWriter::setSVGClosedTE(m_pchSVGCloseTE->isChecked());
    FoilSVGWriter::setSVGFillFoil(m_pchSVGFillFoil->isChecked());
    FoilSVGWriter::setSVGExportStyle(m_pchSVGExportStyle->isChecked());
    FoilSVGWriter::setSVGScaleFactor(m_pdeSVGScaleFactor->value());
    FoilSVGWriter::setSVGMargin(m_pdeSVGMargin->value());
    SaveOptions::s_bCleanOnExit = m_pchCleanOnExit->isChecked();
}


void SaveOptionsWt::showBox(int iBox)
{
    if(iBox<0)
    {
        for(int i=0; i<m_pGroupBox.size(); i++)
            m_pGroupBox[i]->setVisible(true);
    }
    else
    {
        for(int i=0; i<m_pGroupBox.size(); i++)
            m_pGroupBox[i]->setVisible(i==iBox);
    }
}


void SaveOptionsWt::onLastUsedDir()
{
    SaveOptions::s_bUseLastDir = m_prbUseLastDir->isChecked();
    m_pleLastDir->setEnabled(!SaveOptions::s_bUseLastDir);
}



void SaveOptionsWt::onActiveDir()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Select directory",
                                                 m_pleLastDir->text(),
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dirName.length())
    {
        SaveOptions::s_LastDirName = dirName;
        m_pleLastDir->setText(SaveOptions::s_LastDirName);
    }
}


void SaveOptionsWt::onDatFoilDir()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Select directory",
                                                        m_pleDatFoilDir->text(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(dirName.length())
    {
        SaveOptions::s_datFoilDirName = dirName;
        m_pleDatFoilDir->setText(SaveOptions::s_datFoilDirName);
    }
}


void SaveOptionsWt::onPlrPolarDir()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Select directory",
                                                        m_plePlrPolarDir->text(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(dirName.length())
    {
        SaveOptions::s_plrPolarDirName = dirName;
        m_plePlrPolarDir->setText(SaveOptions::s_plrPolarDirName);
    }
}


void SaveOptionsWt::onXmlPolarDir()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Select directory",
                                                        m_pleXmlPolarDir->text(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(dirName.length())
    {
        SaveOptions::s_xmlPolarDirName = dirName;
        m_pleXmlPolarDir->setText(SaveOptions::s_xmlPolarDirName);
    }
}


void SaveOptionsWt::onXmlPlaneDir()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Select directory",
                                                        m_pleXmlPlaneDir->text(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(dirName.length())
    {
        SaveOptions::s_xmlPlaneDirName = dirName;
        m_pleXmlPlaneDir->setText(SaveOptions::s_xmlPlaneDirName);
    }
}


void SaveOptionsWt::onXmlWPolarDir()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Select directory",
                                                        m_pleXmlWPolarDir->text(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(dirName.length())
    {
        SaveOptions::s_xmlWPolarDirName = dirName;
        m_pleXmlWPolarDir->setText(SaveOptions::s_xmlWPolarDirName);
    }
}


void SaveOptionsWt::onXmlScriptDir()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Select directory",
                                                        m_pleXmlScriptDir->text(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(dirName.length())
    {
        SaveOptions::s_xmlScriptDirName = dirName;
        m_pleXmlScriptDir->setText(SaveOptions::s_xmlScriptDirName);
    }
}


void SaveOptionsWt::onCADDir()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Select directory",
                                                        m_pleCADDir->text(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(dirName.length())
    {
        SaveOptions::s_CADDirName = dirName;
        m_pleCADDir->setText(SaveOptions::s_CADDirName);
    }
}


void SaveOptionsWt::onSTLDir()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Select directory",
                                                        m_pleSTLDir->text(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(dirName.length())
    {
        SaveOptions::s_STLDirName = dirName;
        m_pleSTLDir->setText(SaveOptions::s_STLDirName);
    }
}


void SaveOptionsWt::onTempDir()
{
    QString oldname = SaveOptions::s_TempDirName;
    QString dirName = QFileDialog::getExistingDirectory(this, "Select directory",
                                                        m_pleTempDir->text(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    SaveOptions::s_TempDirName = dirName;
    m_pleTempDir->setText(SaveOptions::s_TempDirName);

    if(!onCheckTempDir())
    {
        SaveOptions::s_TempDirName = oldname;
        m_pleTempDir->setText(SaveOptions::s_TempDirName);
    }
}


bool SaveOptionsWt::onCheckTempDir()
{
    QString strange = m_pleTempDir->text();
    QFileInfo tmpdirinfo(strange);

    if(!tmpdirinfo.isDir() || !tmpdirinfo.isWritable())
    {
        QMessageBox::warning(this, "Warning", "Please select an existing directory with write permission for the temp files.");
        m_pleTempDir->setText(strange);
        m_pleTempDir->selectAll();
        m_pleTempDir->setFocus();
        return false;
    }
    return true;
}


void SaveOptionsWt::onExportFormat()
{
    if(m_pCSV->isChecked()) SaveOptions::s_ExportFileType = xfl::CSV;
    else                    SaveOptions::s_ExportFileType = xfl::TXT;
    m_pleCsvSeparator->setEnabled(SaveOptions::s_ExportFileType==xfl::CSV);
}
