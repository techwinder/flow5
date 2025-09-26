/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QStandardItemModel>
#include <QCheckBox>
#include <QSettings>
#include <QPushButton>
#include <QTreeView>
#include <QLabel>

#include <xflwidgets/customdlg/xfldialog.h>

struct FoilPolars
{
    QString m_FileName;
    QString m_FoilName;
    QStringList m_FoilPolars;

/*    bool operator<(const FoilPolars &other) const
    {
      return m_FileName.compare(other.m_FileName, Qt::CaseInsensitive)<0;
    }*/

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

        QSize sizeHint() const  override {return QSize(500,700);}


    private:
        void fillModel();
        void setupLayout();
        void connectSignals();
        void scanSelectedDirectory();

    private slots:
        void onDeleteSelectedFiles();
        void onImportSelectedFiles();
        void onButton(QAbstractButton *pButton) override;

        void onChangeDir();

    private:
        QVector<FoilPolars> m_FoilList;

        QString m_DirName;

        QLabel *m_plabDirName;
        QTreeView *m_pTreeView;
        QStandardItemModel *m_pModel;

        QCheckBox*m_pchRecursive;
        QPushButton *m_ppbChangeDir;
        QPushButton *m_ppbDeleteFiles, *m_ppbImportFiles;

        static QByteArray s_WindowGeometry;
};

