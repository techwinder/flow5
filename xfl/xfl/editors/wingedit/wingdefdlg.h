/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xfl/editors/wingedit/wingdlg.h>
class CPTableView;
class WingSectionDelegate;

class WingDefDlg : public WingDlg
{
    Q_OBJECT

    public:
        WingDefDlg(QWidget *pParent=nullptr);
        ~WingDefDlg();

        void makeWingTable();
        void initDialog(WingXfl*pWing) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    protected:
        void connectSignals();
        void setupLayout();

        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void updateData() override;
        void setCurrentSection(int iSection) override;
        void readParams() override;
        void setControls() override;

    protected slots:
        void onCellChanged();
        void onCopy();
        void onPaste() override;
        void onRowChanged(const QModelIndex &currentindex, const QModelIndex &);
        void onSide();
        void onTipStrips();
        void onWingSides();
        void onWingTableClicked(QModelIndex index);
        void onWingTableContextMenu(QPoint);

    private:
        QCheckBox *m_pchTwoSided;
        QCheckBox *m_pchsymmetric;
        QCheckBox *m_pchCloseInnerSide;

        IntEdit *m_pieTipStrips;

        QRadioButton *m_prbLeftSide, *m_prbRightSide;

        CPTableView *m_pcptSections;
        WingSectionModel *m_pSectionModel;
        WingSectionDelegate *m_pSectionDelegate;

        QMenu *m_pTableContextMenu;
        QSplitter *m_pHSplitter, *m_pVSplitter;

        static QByteArray s_HSplitterSizes, s_VSplitterSizes;
};

