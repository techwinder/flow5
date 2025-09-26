/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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


