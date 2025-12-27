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

