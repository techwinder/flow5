/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xfl/editors/boatedit/saildlg.h>

class IntEdit;
class WingSailSectionModel;
class WingSailSectionDelegate;

class WingSailDlg : public SailDlg
{
    Q_OBJECT

    public:
        WingSailDlg(QWidget *pParent=nullptr);
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


