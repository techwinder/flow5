/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QSplitter>
#include <QSettings>
#include <QModelIndex>

#include <xflgeom/geom3d/quaternion.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects3d/inertia/pointmass.h>

class PartInertiaModel;
class PartInertiaDelegate;
class gl3dXflView;
class gl3dGeomControls;
class FloatEdit;
class Plane;
class Fuse;
class FuseXfl;
class WingXfl;

class CPTableView;
class PointMassTable;

class PlaneInertiaDlg : public QDialog
{
    Q_OBJECT

    public:
        PlaneInertiaDlg(QWidget *pParent);
        ~PlaneInertiaDlg();

        virtual void initDialog(Plane *pPlane);

        bool hasChanged() const {return m_bChanged;}

        void hideSaveAsNew() {m_ppbSaveAsNew->hide();}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    protected slots:
        virtual void onResizeColumns();
        virtual void onOK(int iExitCode=QDialog::Accepted) = 0;
        virtual void onCopyInertiaToClipboard() = 0;
        virtual void onExportToAVL() = 0;
        virtual void onImportExisting() = 0;

        void onManInertiaCellChanged(QModelIndex index);
        void onPointMassCellChanged();
        void onPointMassDataPasted();
        void onMassRowChanged(QModelIndex index);

        void onRedraw();
        void onButton(QAbstractButton *pButton);

    protected:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void reject() override;

        void makeCommonWts();
        void updateTotalInertia();
        void connectBaseSignals();


    protected:
        //layout widget variables
        QSplitter *m_pHSplitter, *m_pVSplitter;

        QFrame *m_pfrPointMass, *m_pfrTotalMass;

        QDialogButtonBox *m_pButtonBox;
        QAction *m_pImportInertia;
        QAction *m_pExportToAVL, *m_pExportToClipboard;
        QPushButton *m_ppbSaveAsNew;

        QLabel *m_plabPlaneName;

        QLabel *m_plabMassUnit2;

        QLabel *m_plabLengthUnit20, *m_plabLengthUnit21, *m_plabLengthUnit22;
        QLabel *m_plabTotalMass;
        QLabel *m_plabInertiaUnit10, *m_plabInertiaUnit20, *m_plabInertiaUnit30, *m_plabInertiaUnit40;

        // Custom input table
        CPTableView *m_pInertiaManTable;
        PartInertiaModel *m_pStructInertiaModel;
        PartInertiaDelegate *m_pPartInertiaDelegate;

        PointMassTable *m_ppmtMasses;

        FloatEdit *m_pfeTotalIxx, *m_pfeTotalIyy, *m_pfeTotalIzz, *m_pfeTotalIxz;
        FloatEdit *m_pdeXTotalCoG, *m_pdeYTotalCoG, *m_pdeZTotalCoG;
        FloatEdit *m_pdeTotalMass;


        gl3dXflView *m_pgl3dPlaneView;
        gl3dGeomControls *m_pgl3dControls;

        Plane *m_pPlane;


        bool m_bChanged;

        static QByteArray s_Geometry;

        static QByteArray s_HSplitterSizes, s_VSplitterSizes;

        static Quaternion s_ab_quat;
};

