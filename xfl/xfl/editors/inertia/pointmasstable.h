/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QStandardItemModel>


#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/actionitemmodel.h>

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

