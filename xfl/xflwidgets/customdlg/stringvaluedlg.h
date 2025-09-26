/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QDialogButtonBox>

class StringValueDlg : public QDialog
{
    Q_OBJECT

    public:
        StringValueDlg(QWidget *pParent, QStringList const &list);

        void setStringList(QStringList const &list);
        QString selectedString() const;

    private:
        QSize sizeHint() const override {return QSize(500,700);}
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void setupLayout();

    private slots:
        void onItemDoubleClicked(QListWidgetItem *pItem);

    private:
        QStringList m_StringList;
        QListWidget *m_plwStrings;
        QDialogButtonBox *m_pButtonBox;
        static QByteArray s_WindowGeometry;
};

