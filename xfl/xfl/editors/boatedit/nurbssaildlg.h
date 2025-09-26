/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xfl/editors/boatedit/saildlg.h>

class IntEdit;
class CPTableView;
class ActionItemModel;
class XflDelegate;

class NURBSSailDlg : public SailDlg
{
    Q_OBJECT

    public:
        NURBSSailDlg(QWidget *pParent=nullptr);

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
        FloatEdit *m_pdeEdgeWeightu, *m_pdeEdgeWeightv;

        ActionItemModel *m_pSectionModel;
        XflDelegate *m_pSectionDelegate;

};
