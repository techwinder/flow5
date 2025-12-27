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

#include <QTreeView>
#include <QStandardItem>


class PlaneXfl;
class WingXfl;
class Vector3d;
class Inertia;
class PointMass;

class ObjectTreeView : public QTreeView
{
    public:
        ObjectTreeView(QWidget *pParent);
        void fillWingTreeView(WingXfl const *pWing);
        void fillInertia(Inertia const &inertia, QStandardItem * pWingInertiaFolder);
        void fillColorTree(QColor clr, QStandardItem *pColorItem);

        void readInertiaTree(Inertia &inertia, QModelIndex indexLevel);
        void readVectorTree(Vector3d &V, QModelIndex indexLevel);
        void readColorTree(QColor &color, QModelIndex indexLevel);
        void readPointMassTree(PointMass &pm, QModelIndex indexLevel);
        void readWingTree(WingXfl *pWing);

        void readWingSectionTree(WingXfl *pWing, QModelIndex indexLevel);


};


