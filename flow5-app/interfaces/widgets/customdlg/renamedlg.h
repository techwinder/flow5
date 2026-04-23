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
        void initDialog(std::string const &startname, std::vector<std::string> const &existingnames, QString const &question);

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




