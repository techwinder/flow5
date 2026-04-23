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

#include <QListWidgetItem>
#include <QSettings>

#include <interfaces/widgets/customdlg/xfldialog.h>

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
