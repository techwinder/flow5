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

#include <interfaces/editors/boatedit/thinsaildlg.h>

class IntEdit;
class CPTableView;
class ActionItemModel;
class XflDelegate;

class SailNurbsDlg : public ThinSailDlg
{
    Q_OBJECT

    public:
        SailNurbsDlg(QWidget *pParent=nullptr);

        void initDialog(Sail *pSail) override;

    protected:
        void connectSignals();
        void setSailData() override;
        void setControls() override;
        void updateSailSectionOutput() override;
        void readData() override;
        void setupLayout();
        void keyPressEvent(QKeyEvent *pEvent) override;

    protected slots:
        void onAlignLuffPoints() override;
        void onSelectCtrlPoint(int iPoint) override;
        void onNurbsMetaChanged();

        void onSectionItemClicked(const QModelIndex &index) override;
        void onPointItemClicked(const QModelIndex &index) override;

        void onCurrentSectionChanged(const QModelIndex &index) override;
        void onCurrentPointChanged(const QModelIndex &index) override;

        void onInsertSectionBefore() override;
        void onInsertSectionAfter() override;
        void onDeleteSection() override;
        void onTranslateSection() override;

        void onInsertPointBefore() override;
        void onInsertPointAfter() override;
        void onDeletePoint() override;

        void onSelectSection(int iSection) override;

        void onSwapUV();
        void onReverseHPoints();
        void onReverseVSections();


    private:
        void makeTables();
        void resizeSectionTableColumns() override;

        void fillSectionModel() override;
        void fillPointModel() override;

        void readSectionData() override;
        void readPointData() override;

    private:
        IntEdit *m_pieNXDegree, *m_pieNZDegree;
        FloatEdit *m_pfeEdgeWeightu, *m_pfeEdgeWeightv;

        ActionItemModel *m_pSectionModel;
        XflDelegate *m_pSectionDelegate;

};
