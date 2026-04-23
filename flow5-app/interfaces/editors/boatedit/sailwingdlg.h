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

#include <interfaces/editors/boatedit/saildlg.h>

class IntEdit;
class WingSailSectionModel;
class WingSailSectionDelegate;

class SailWingDlg : public SailDlg
{
    Q_OBJECT

    public:
        SailWingDlg(QWidget *pParent=nullptr);
        void initDialog(Sail *pSail) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

    protected:
        void setupLayout();
        void connectSignals();
        void updateSailSectionOutput() override;
        void setSailData() override;
        void setControls() override;

        void fillSectionModel() override;
        void readSectionData() override;

        void makeTables();
        void resizeSectionTableColumns() override;

    protected slots:
        void onAlignLuffPoints() override;
        void onCurrentSectionChanged(const QModelIndex &index) override;
        void onDeleteSection() override;
        void onInsertSectionAfter() override;
        void onInsertSectionBefore() override;
        void accept() override;
        void onSectionDataChanged() override;
        void onSectionItemClicked(const QModelIndex &index) override;
        void onSelectCtrlPoint(int) override{}
        void onSelectSection(int iSection) override;
        void onTranslateSection() override;
        void onUpdate() override;


        // ----------------------- Variables -----------------------------

    private:
        WingSailSectionModel *m_pWSSectionModel;
        WingSailSectionDelegate *m_pWSSectionDelegate;
};


