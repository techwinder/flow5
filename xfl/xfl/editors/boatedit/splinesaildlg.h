/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xfl/editors/boatedit/saildlg.h>

class IntEdit;


class SplineSailDlg : public SailDlg
{
    Q_OBJECT

    public:
        SplineSailDlg(QWidget *pParent=nullptr);
        void initDialog(Sail *pSail) override;

    protected:
        void connectSignals();
        void setSailData() override;

        void updateSailSectionOutput() override;
        void setupLayout();
        void updateSailGeometry() override;
        void keyPressEvent(QKeyEvent *pEvent) override;

    protected slots:

        void onAlignLuffPoints() override;
        void onSelectCtrlPoint(int iPoint) override;

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

        void onUpdate() override;

        void onConvertSplines(int index);
        void onBSplineDegreeChanged();

    private:
        void fillSectionModel() override;
        void fillPointModel() override;
        void readSectionData() override;
        void readPointData() override;

        void makeTables();
        void resizeSectionTableColumns() override;

    private:
        IntEdit *m_pieBSplineDeg;
        QComboBox *m_pcbSailType;


        ActionItemModel *m_pSectionModel;
        XflDelegate *m_pSectionDelegate;
};


