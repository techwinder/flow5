/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xfl/editors/wingedit/wingdlg.h>

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


