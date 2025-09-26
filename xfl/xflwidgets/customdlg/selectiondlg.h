/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QListWidgetItem>
#include <QSettings>

#include <xflwidgets/customdlg/xfldialog.h>

class SelectionDlg : public XflDialog
{
    Q_OBJECT
    public:
        SelectionDlg(QWidget *pParent=nullptr);
        void initDialog(const QString &question, QStringList const &namelist, QStringList const &selectionlist, bool bSingleSel);
        QStringList const&selectedList() const {return m_SelectedList;}
        QString selection() const;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void accept() override;
        void onButton(QAbstractButton*pButton) override;

        void onSelectAll();
        void onDoubleClickList(QListWidgetItem *);

    private:
        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(700,900);}

    private:
        QStringList m_SelectedList, m_strNameList;

        QLabel *m_plabQuestion;
        QListWidget *m_plwNameList;
        QPushButton *m_ppbSelectAll;

        static QByteArray s_Geometry;
};
