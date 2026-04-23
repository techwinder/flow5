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

#include <QStandardItemModel>


#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>

class XflDelegate;
class Inertia;
class PointMassTable : public CPTableView
{
    Q_OBJECT

    public:
        PointMassTable(QWidget *pParent);

        void setInertia(Inertia *pInertia) {m_pInertia=pInertia;}

        void fillMassModel();
        void readPointMassData();

        void resizeColumns();

    private slots:
        void onInsertMassBefore();
        void onInsertMassAfter();
        void onDeleteMassRow();
        void onDuplicateMassRow();
        void onMassTableClicked(QModelIndex);
        void onPointMassCellChanged(QWidget *);
        void onMoveUp();
        void onMoveDown();

    signals:
        void pointMassChanged();

    private:
        ActionItemModel *m_pMassModel;
        XflDelegate *m_pMassDelegate;

        Inertia *m_pInertia;

        QMenu *m_pContextMenu;

};

