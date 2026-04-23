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

#include <interfaces/editors/wingedit/wingdlg.h>

class ObjectTreeView;
class EditObjectDelegate;

class WingObjectDlg : public WingDlg
{
    Q_OBJECT

    public:
        WingObjectDlg(QWidget *pParent);
        ~WingObjectDlg();

        void initDialog(WingXfl*pWing) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void updateData() override;
        void setCurrentSection(int iSection) override;
        void readParams() override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();
        void identifySelection(const QModelIndex &indexSel);

    private slots:
        void onEndEdit();
        void onItemClicked(const QModelIndex &index);
        void onResizeColumns();

    private:
        ObjectTreeView *m_pTreeView;
        QStandardItemModel *m_pModel;
        EditObjectDelegate *m_pDelegate;

        int m_iActivePointMass;

        QSplitter *m_pHSplitter;
        static QByteArray s_HSplitterSizes;
};


