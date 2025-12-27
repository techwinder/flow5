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

