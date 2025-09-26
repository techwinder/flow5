/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QSettings>
#include <QLineEdit>
#include <QPlainTextEdit>

#include <xflgeom/geom3d/triangle3d.h>
#include <xflgeom/geom3d/quaternion.h>
#include <xflcore/linestyle.h>
#include <xflwidgets/customdlg/xfldialog.h>

class Plane;

class gl3dXflView;
class gl3dGeomControls;

class PlanePartModel;
class PlaneStl;
class FloatEdit;
class IntEdit;
class ColorMenuBtn;
class PlainTextOutput;

class PlaneDlg : public XflDialog
{
    Q_OBJECT

    public:
        PlaneDlg(QWidget *pParent);
        virtual void initDialog(Plane *pPlane, bool bAcceptName);
        QSize sizeHint() const override {return QSize(1100,900);}

        virtual void showEvent(QShowEvent *pEvent) override;
        virtual void hideEvent(QHideEvent *pEvent) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    protected:
        void connectBaseSignals();
        virtual void updateOutput(QString const &msg) = 0;
        virtual void setControls() = 0;
        void makeCommonControls();
        void outputPanelProperties(int panelindex);

    protected slots:
        virtual void onButton(QAbstractButton *pButton) override;
        virtual void onFlipNormals() = 0;
        virtual void onOK(int iExitCode=QDialog::Accepted) = 0;
        virtual void onPlaneInertia() = 0;
        void onCenterViewOnPanel();
        void onCheckFreeEdges();
        void onCheckMesh();
        void onClearHighlighted();
        void onConnectPanels();
        void onNodeDistance();
        void onMergeFuseToWingNodes();
        void onMetaDataChanged();
        virtual void onPickedNode(int iNode);

        virtual void onUpdatePlane() = 0;
        virtual void onUpdatePlaneProps();
        void reject() override;

    protected:
        gl3dGeomControls *m_pglControls;
        gl3dXflView *m_pglPlaneView;

        QLineEdit *m_pleName;
        QPlainTextEdit *m_pleDescription;

        QPushButton *m_ppbActions;

        QPushButton *m_ppbSaveAsNew;
        QAction *m_pClearOutput;
        QAction *m_pPlaneInertia, *m_pFlipNormals;
        QAction *m_pCheckMesh, *m_pClearHighlighted, *m_pCenterOnPanel;
        QAction *m_pCheckFreeEdges, *m_pMergeFuseToWingNodes, *m_pConnectPanels;


        Plane *m_pPlane;

        bool m_bAcceptName;

        static double s_TEMaxAngle; // to connect TE in STL meshes
        static bool s_bOutline, s_bSurfaces, s_bVLMPanels, s_bAxes, s_bShowMasses, s_bFoilNames;

        static Quaternion s_ab_quat;
};


