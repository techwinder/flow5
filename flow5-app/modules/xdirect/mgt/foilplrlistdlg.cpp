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

#include <QApplication>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QDir>
#include <QVector>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>

#include "foilplrlistdlg.h"


#include <core/xflcore.h>
#include <api/utils.h>
#include <api/objects2d_globals.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <api/polar.h>
#include <api/objects_global.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>


QByteArray FoilPlrListDlg::s_WindowGeometry;


FoilPlrListDlg::FoilPlrListDlg(QWidget *pParent) : XflDialog (pParent)
{
    setupLayout();
    connectSignals();
}


void FoilPlrListDlg::setupLayout()
{
    QHBoxLayout *pDirLayout = new QHBoxLayout;
    {
        m_plabDirName = new QLabel("Directory");
        m_plabDirName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_ppbChangeDir = new QPushButton("Change directory");
        m_pchRecursive = new QCheckBox("Recursive");
        m_pchRecursive->setToolTip("<p>Also scan sub-directories</p>");
        pDirLayout->addWidget(m_plabDirName);
        pDirLayout->addWidget(m_ppbChangeDir);
        pDirLayout->addWidget(m_pchRecursive);
        pDirLayout->setStretchFactor(m_plabDirName,1);
    }

    QHBoxLayout *pTreeLayout = new QHBoxLayout;
    {
        m_pTreeView = new QTreeView(this);
        m_pModel = new QStandardItemModel(this);
        m_pModel->setColumnCount(2);
        m_pModel->clear();
        QStringList labels;
        labels << "Foils" << "File name";
        m_pModel->setHorizontalHeaderLabels(labels);
        m_pModel->setHeaderData(2, Qt::Horizontal, Qt::AlignRight, Qt::TextAlignmentRole);
        QHeaderView *pHorizontalHeader = m_pTreeView->header();
        pHorizontalHeader->setStretchLastSection(true);
        m_pTreeView->setModel(m_pModel);

        QVBoxLayout *pFileActionLayout = new QVBoxLayout;
        {
            pFileActionLayout->addStretch();
            m_ppbDeleteFiles = new QPushButton("Delete files");
            m_ppbImportFiles = new QPushButton("Import files");

        }
        pTreeLayout->addWidget(m_pTreeView);
    }

    m_pTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_ppto = new PlainTextOutput;
    m_ppto->setCharDimensions(50,5);

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        setButtons(QDialogButtonBox::Close);
        m_pButtonBox->addButton(m_ppbImportFiles, QDialogButtonBox::ActionRole);
        m_pButtonBox->addButton(m_ppbDeleteFiles, QDialogButtonBox::ActionRole);

        pMainLayout->addLayout(pDirLayout);
        pMainLayout->addLayout(pTreeLayout);
        pMainLayout->addWidget(m_ppto);
        pMainLayout->addWidget(m_pButtonBox);
        pMainLayout->setStretchFactor(pTreeLayout,3);
        pMainLayout->setStretchFactor(m_ppto,1);
    }
    setLayout(pMainLayout);
}


void FoilPlrListDlg::connectSignals()
{
    connect(m_ppbChangeDir,   SIGNAL(clicked()), SLOT(onChangeDir()));
    connect(m_pchRecursive,   SIGNAL(clicked()), SLOT(onScanDirectory()));
}


void FoilPlrListDlg::onButton(QAbstractButton *pButton)
{
    if     (m_ppbImportFiles==pButton) onImportSelectedFiles();
    else if(m_ppbDeleteFiles==pButton) onDeleteSelectedFiles();
    else XflDialog::onButton(pButton);
}


void FoilPlrListDlg::initDlg(const QString &pathname)
{
    m_DirName = pathname;
    m_plabDirName->setText(m_DirName);
    onScanDirectory();
}


void FoilPlrListDlg::showEvent(QShowEvent *pEvent)
{
    XflDialog::showEvent(pEvent);

    restoreGeometry(s_WindowGeometry);
    int w = m_pTreeView->width();
    m_pTreeView->setColumnWidth(0, w/3);
}


void FoilPlrListDlg::hideEvent(QHideEvent*pEvent)
{
    XflDialog::hideEvent(pEvent);
    s_WindowGeometry = saveGeometry();
}


void FoilPlrListDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("FoilPlrListDlg");
    {
        s_WindowGeometry = settings.value("WindowGeometry").toByteArray();
    }
    settings.endGroup();
}


void FoilPlrListDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("FoilPlrListDlg");
    {
        settings.setValue("WindowGeometry", s_WindowGeometry);
    }
    settings.endGroup();
}


void FoilPlrListDlg::onChangeDir()
{
    QString DirName = QFileDialog::getExistingDirectory(this, ("Change directory"),
                                                        m_DirName,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(DirName.length())
    {
        m_DirName = DirName;
        onScanDirectory();
    }
    else {
        //cancelled directory selection
    }
}


void FoilPlrListDlg::onDeleteSelectedFiles()
{
    int resp = QMessageBox::question(this, tr("Delete"), tr("Delete permanently the selected files?"),
                                     QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                     QMessageBox::Yes);
    if(resp != QMessageBox::Yes)
    {
        return;
    }

    QModelIndexList selectedIndexes = m_pTreeView->selectionModel()->selectedRows(1);
    for(QModelIndex const &idx : selectedIndexes)
    {
        QStandardItem *pItem = m_pModel->itemFromIndex(idx);
        QString filename = pItem->text();
        QFile file(filename);
        if(file.exists()) file.remove();

    }
    for (QModelIndexList::const_iterator idx=selectedIndexes.constEnd()-1; idx>=selectedIndexes.constBegin(); --idx)
    {
        int row = idx->row();
        m_pModel->removeRow(idx->row());
        m_FoilList.removeAt(row);
    }

/*    int row = m_pTreeView->selectionModel()->currentIndex().row();
    FoilPolars const &fp = m_FoilList.at(row);
    QString filename = fp.m_FileName;

    QFile file(filename);
    if(file.exists()) file.remove();
    m_pModel->removeRow(row);
    m_FoilList.removeAt(row);
*/
    update();
}


void FoilPlrListDlg::onImportSelectedFiles()
{
    QModelIndexList selectedIndexes = m_pTreeView->selectionModel()->selectedRows(1);

    m_ppto->onAppendQText(QString::asprintf("Importing %d files", int(selectedIndexes.size()))+EOLch);

    for(int i=0; i<selectedIndexes.size(); i++)
    {
        QModelIndex const &idx = selectedIndexes.at(i);
        QStandardItem const*pItem = m_pModel->itemFromIndex(idx);
        if(pItem)
        {
            QString filename = pItem->text();
            QFile file(filename);

            if (!file.exists() || !file.open(QIODevice::ReadOnly))
            {
                m_ppto->onAppendQText("   Error reading file " +filename + EOLch);
                continue;
            }

            std::vector<Foil*> foillist;
            std::vector<Polar*> polarlist;
            objects::readPolarFile(file, foillist, polarlist);
            file.close();

            m_ppto->onAppendQText(QString::asprintf("   file contains %d foil(s) and %d polar(s)", int(foillist.size()), int(polarlist.size()))+EOLch);

            for(uint j=0; j<foillist.size(); j++)
            {
                Foil *pFoil = foillist.at(j);
                Objects2d::insertThisFoil(pFoil);
            }
            for(uint j=0; j<polarlist.size(); j++)
            {
                Polar *pPolar = polarlist.at(j);
                Objects2d::insertPolar(pPolar);
            }
        }
    }
    m_pTreeView->selectionModel()->clearSelection();

    m_ppto->onAppendQText("Done importing file(s)" + EOLch+EOLch);
}


void FoilPlrListDlg::onScanDirectory()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QDir plrdir(m_DirName);
    if(!plrdir.exists())
    {
        m_ppto->onAppendQText("Directory "+ m_DirName+" does not exist" + EOLch);
        return;
    }
    m_ppto->onAppendQText("Scanning directory "+ m_DirName+EOLch);

    bool bRecursive = m_pchRecursive->isChecked();
    QStringList files = xfl::findFiles(m_DirName, {"*.plr"}, bRecursive);

    m_ppto->onAppendQText(QString::asprintf("Reading %d files\n", int(files.size())));

    m_pModel->removeRows(0, m_pModel->rowCount());

    QStandardItem *pRootItem = m_pModel->invisibleRootItem();
    pRootItem->setText(tr("Foils"));

    QModelIndex ind = m_pModel->index(0,0);
    m_pTreeView->expand(ind);

    m_FoilList.clear();

    std::vector<Foil*> foils;
    std::vector<Polar*> polars;
    for (int i=0; i<files.size(); i++)
    {
        QString filename = files.at(i);
        QFileInfo fi(filename);
        if(fi.suffix()!="plr")
        {
            m_ppto->onAppendQText("Skipping file " + filename + EOLch);
            continue;
        }

        QFile plrfile(filename);
        if (!plrfile.open(QIODevice::ReadOnly))
        {
            m_ppto->onAppendQText("Could not open file " + filename + EOLch);
        }
        else
        {
            m_ppto->onAppendQText("Reading file " + filename + EOLch);

            objects::readPolarFile(plrfile, foils, polars);
            plrfile.close();

            for(uint j=0; j<foils.size(); j++)
            {
                Foil *pFoil = foils.at(j);
                QStringList polarlist;
                for(uint k=0; k<polars.size(); k++)
                {
                    Polar *pPolar = polars.at(k);
                    if(pFoil->name().compare(pPolar->foilName())==0)
                    {
                        polarlist.append(QString::fromStdString(pPolar->name()));
                    }
                    delete pPolar;
                }
                m_FoilList.push_back({filename, QString::fromStdString(pFoil->name()), polarlist});
                delete pFoil;
            }

            polars.clear();
            foils.clear();
        }
    }

    //sort the foils
    std::sort(m_FoilList.begin(), m_FoilList.end(), [](const FoilPolars& a, const FoilPolars & b) {
        return a.m_FoilName.compare(b.m_FoilName, Qt::CaseInsensitive)<0;
    });

    // fill the view
    for (int j=0; j<m_FoilList.size(); j++)
    {
        FoilPolars const &fp = m_FoilList.at(j);
        QList<QStandardItem *> pFoilRowItems;
        pFoilRowItems << new QStandardItem(fp.m_FoilName) << new QStandardItem(fp.m_FileName) ;
        pFoilRowItems.at(0)->setData(fp.m_FoilName, Qt::DisplayRole);
        pFoilRowItems.at(1)->setData(fp.m_FileName, Qt::DisplayRole);
        pRootItem->appendRow(pFoilRowItems);
        for(int k=0; k<fp.m_FoilPolars.size(); k++)
        {
            QList<QStandardItem *>  pPolarrowitems;
            pPolarrowitems << new QStandardItem(fp.m_FoilPolars.at(k));
            pPolarrowitems.at(0)->setData(fp.m_FoilPolars.at(k), Qt::DisplayRole);
            pFoilRowItems[0]->appendRow( pPolarrowitems);
        }
    }

    m_ppto->onAppendQText(EOLch);
    m_ppto->ensureCursorVisible();

    update();
    QApplication::restoreOverrideCursor();
}


