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

#pragma once

#include <QStandardItemModel>
#include <QCheckBox>
#include <QSettings>
#include <QPushButton>
#include <QTreeView>
#include <QLabel>

#include <interfaces/widgets/customdlg/xfldialog.h>

class PlainTextOutput;

struct FoilPolars
{
    QString m_FileName;
    QString m_FoilName;
    QStringList m_FoilPolars;
};


class FoilPlrListDlg : public XflDialog
{
    Q_OBJECT
    public:
        FoilPlrListDlg(QWidget*pParent);

        void initDlg(QString const &pathname);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        QSize minimumSizeHint() const  override {return QSize(500,700);}


    private:
        void setupLayout();
        void connectSignals();

    private slots:
        void onDeleteSelectedFiles();
        void onImportSelectedFiles();
        void onButton(QAbstractButton *pButton) override;

        void onChangeDir();
        void onScanDirectory();

    private:
        QVector<FoilPolars> m_FoilList;

        QString m_DirName;

        QLabel *m_plabDirName;
        QTreeView *m_pTreeView;
        QStandardItemModel *m_pModel;

        QCheckBox*m_pchRecursive;
        QPushButton *m_ppbChangeDir;
        QPushButton *m_ppbDeleteFiles, *m_ppbImportFiles;

        PlainTextOutput *m_ppto;

        static QByteArray s_WindowGeometry;
};

