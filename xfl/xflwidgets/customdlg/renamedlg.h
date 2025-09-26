/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QListWidget>
#include <QSettings>

class RenameDlg : public QDialog
{
    Q_OBJECT

    public:
        RenameDlg(QWidget *pParent=nullptr);

        void initDialog(QString const &startname, QStringList const &existingnames, QString const &question);

        QString newName() const;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private slots:
        void onOverwrite();
        void onOK();
        void onSelChangeList(int);
        void onDoubleClickList(QListWidgetItem * pItem);
        void onButton(QAbstractButton *pButton);
        void onEnableOverwrite(QString name);

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(700,800);}

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void setupLayout();

        QStringList m_strNames;

        QLabel      *m_plabMessage;
        QLineEdit   *m_pleName;
        QListWidget *m_plwNameList;
        QPushButton *m_pOverwriteButton;
        QDialogButtonBox *m_pButtonBox;

        static QByteArray s_WindowGeometry;
};




