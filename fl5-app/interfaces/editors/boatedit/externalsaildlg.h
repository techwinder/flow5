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
#include <api/segment3d.h>


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


