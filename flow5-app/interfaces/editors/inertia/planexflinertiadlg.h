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

#include <QStackedWidget>

#include <interfaces/editors/inertia/planeinertiadlg.h>

class PlaneXfl;
class PlaneInertiaModel;
class XflDelegate;

class PlaneXflInertiaDlg : public PlaneInertiaDlg
{
        Q_OBJECT
    public:
        PlaneXflInertiaDlg(QWidget *pParent);
        void initDialog(Plane *pPlane) override;

    private:
        void connectSignals();
        void setupLayout();
        void editFuseInertia();
        void editWingInertia(int iWing);

    private slots:
        void onOK(int iExitCode) override;
        void onResizeColumns() override;
        void onItemClicked(QModelIndex index);
        void onItemDoubleClicked(QModelIndex index);
        void onImportExisting() override;
        void onCopyInertiaToClipboard() override;
        void onExportToAVL() override;
        void onDataChanged(QModelIndex topleft, QModelIndex);
        void onInertiaSource();

    private:
        PlaneXfl *m_pPlaneXfl;

        QStackedWidget *m_pswInputSource;

        QRadioButton *m_prbInertiaFromParts, *m_prbInertiaManual;

        CPTableView *m_pcptPart;
        PlaneInertiaModel *m_pPartInertiaModel;
        XflDelegate  *m_pPartTableDelegate;
};

