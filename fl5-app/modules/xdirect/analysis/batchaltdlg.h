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


#include <api/enums_objects.h>
#include <modules/xdirect/analysis/batchdlg.h>


class ExpandableTreeView;
class ObjectTreeDelegate;
class ObjectTreeModel;

class BatchAltDlg : public BatchDlg
{
    Q_OBJECT

    public:
        BatchAltDlg(QWidget *pParent=nullptr);
        ~BatchAltDlg();

        void initDialog() override;

        static void initReList();
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();


    private slots:
        void onAnalyze() override;
        void onCurrentRowChanged(QModelIndex currentidx, QModelIndex);

    private:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void connectSignals();

        void setObjectProperties(QModelIndex index);


    private:

        ExpandableTreeView *m_pStruct;
        ObjectTreeModel *m_pModel;
        ObjectTreeDelegate *m_pDelegate;

        QSplitter *m_psplVLeft;

        PlainTextOutput *m_pptoObjectProps;

        static QByteArray s_VLeftSplitterSizes;
};

