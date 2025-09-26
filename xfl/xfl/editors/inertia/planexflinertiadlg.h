/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QStackedWidget>

#include <xfl/editors/inertia/planeinertiadlg.h>

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

