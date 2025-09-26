/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <xfl/editors/boatedit/saildlg.h>
#include <xflgeom/geom3d/segment3d.h>


class PlainTextOutput;

class ExternalSailDlg : public SailDlg
{
    Q_OBJECT

    public:
        ExternalSailDlg(QWidget *pParent);

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

    protected:
        void fillSectionModel() override {}
        void readSectionData() override {}
        void connectSignals();
        void setControls() override;

        void resizePointTableColumns() override {}
        void resizeSectionTableColumns() override {}

        bool deselectButtons() override;
        void makeCommonWt();

    protected slots:
        void onSelectSection(int ) override {}
        void onAlignLuffPoints() override {}
        void onCurrentSectionChanged(const QModelIndex &) override {}
        void onCurrentPointChanged(const QModelIndex &) override {}
        void onPickedNode(Vector3d I);
        void onCornerPoint(bool bChecked);

        void onSectionItemClicked(const QModelIndex &) override {}
        void onPointItemClicked(const QModelIndex &) override {}
        void onInsertSectionBefore() override {}
        void onInsertSectionAfter() override {}
        void onDeleteSection() override {}
        void onTranslateSection() override {}
        void onRotateSail();

        void onTranslateSail() override;


    protected:
        QPushButton *m_ppbPeak, *m_ppbHead, *m_ppbClew, *m_ppbTack;

        QGroupBox *m_pCornersBox;




        QLabel *m_pFlow5Link;


};


