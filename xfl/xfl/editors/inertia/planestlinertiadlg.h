/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QCheckBox>


#include <xfl/editors/inertia/planeinertiadlg.h>
#include <xfl/editors/inertia/planeinertiamodel.h>

class PlaneSTL;

class PlaneStlInertiaDlg : public PlaneInertiaDlg
{
    Q_OBJECT

    public:
        PlaneStlInertiaDlg(QWidget *pParent);
        void initDialog(Plane *pPlane) override;

    private:
        void setupLayout();
        void connectSignals();

    private slots:
        void onImportExisting() override;
        void onCopyInertiaToClipboard() override;
        void onExportToAVL() override;
        void onOK(int iExitCode) override;
        void onAutoInertia();

    private:
        PlaneSTL *m_pPlaneSTL;

        QCheckBox *m_pchAutoEstimation;
        FloatEdit *m_pdeStructMass;
};


