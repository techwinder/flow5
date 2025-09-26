/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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

#include <xfl/globals/fileio.h>
#include <xflcore/xflcore.h>
#include <xflfoil/globals/objects2d_globals.h>
#include <xflfoil/objects2d/foil.h>
#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/polar.h>
#include <xflobjects/objects_globals/objects_global.h>

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

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        setButtons(QDialogButtonBox::Close);
        m_pButtonBox->addButton(m_ppbImportFiles, QDialogButtonBox::ActionRole);
        m_pButtonBox->addButton(m_ppbDeleteFiles, QDialogButtonBox::ActionRole);
        pMainLayout->addLayout(pTreeLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void FoilPlrListDlg::connectSignals()
{
    connect(m_ppbChangeDir,   SIGNAL(clicked()), SLOT(onChangeDir()));
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
    scanSelectedDirectory();
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
        scanSelectedDirectory();
    }
    else {
        //cancelled directory selection
    }
}


void FoilPlrListDlg::onDeleteSelectedFiles()
{
    int resp = QMessageBox::question(this, "Delete", "Delete permanently the selected files?",
                                     QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                     QMessageBox::Yes);
    if(resp != QMessageBox::Yes)
    {
        return;
    }

    QModelIndexList selectedIndexes = m_pTreeView->selectionModel()->selectedRows(1);
    foreach(QModelIndex idx, selectedIndexes)
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
    FileIO fileio;

    QModelIndexList selectedIndexes = m_pTreeView->selectionModel()->selectedRows(1);
    for(int i=0; i<selectedIndexes.size(); i++)
    {
        QModelIndex const &idx = selectedIndexes.at(i);
        QStandardItem const*pItem = m_pModel->itemFromIndex(idx);
        if(pItem)
        {
            QString filename = pItem->text();
            QFile file(filename);
            if(file.exists())
            {
                if (file.open(QIODevice::ReadOnly))
                {
                    QVector<Foil*> foillist;
                    QVector<Polar*> polarlist;
                    fileio.readPolarFile(file, foillist, polarlist);
                    file.close();

                    for(int j=0; j<foillist.size(); j++)
                    {
                        Foil *pFoil = foillist.at(j);
                        Objects2d::insertThisFoil(pFoil);
                    }
                    for(int j=0; j<polarlist.size(); j++)
                    {
                        Polar *pPolar = polarlist.at(j);
                        Objects2d::insertPolar(pPolar);
                    }
                }
            }
        }
    }
    m_pTreeView->selectionModel()->clearSelection();
}


void FoilPlrListDlg::scanSelectedDirectory()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    fillModel();
    update();
    QApplication::restoreOverrideCursor();
}


void FoilPlrListDlg::fillModel()
{
    QDir plrdir(m_DirName);
    if(!plrdir.exists())
    {
        return;
    }

    FileIO fileio;

    bool bRecursive = m_pchRecursive->isChecked();
    QStringList files = xfl::findFiles(m_DirName, {"*.plr"}, bRecursive);

    m_pModel->removeRows(0, m_pModel->rowCount());

    QStandardItem *pRootItem = m_pModel->invisibleRootItem();
    pRootItem->setText("Foils");

    QModelIndex ind = m_pModel->index(0,0);
    m_pTreeView->expand(ind);

    m_FoilList.clear();

    QVector<Foil*> foils;
    QVector<Polar*> polars;
    for (int i=0; i<files.size(); i++)
    {
        QString filename = files.at(i);
        QFileInfo fi(filename);
        if(fi.suffix()!="plr") continue;

        QFile plrfile(filename);
        if (plrfile.open(QIODevice::ReadOnly))
        {
            fileio.readPolarFile(plrfile, foils, polars);
            plrfile.close();

            for (int j=0; j<foils.size(); j++)
            {
                Foil *pFoil = foils.at(j);
                QStringList polarlist;
                for(int k=0; k<polars.size(); k++)
                {
                    Polar *pPolar = polars.at(k);
                    if(pFoil->name().compare(pPolar->foilName())==0)
                    {
                        polarlist.append(pPolar->name());
                    }
                    delete pPolar;
                }
                m_FoilList.push_back({filename, pFoil->name(), polarlist});
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
}

