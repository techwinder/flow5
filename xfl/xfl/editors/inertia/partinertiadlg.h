/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QLabel>
#include <QStackedWidget>
#include <QRadioButton>
#include <QPushButton>
#include <QSplitter>
#include <QModelIndex>
#include <QSettings>

#include <xflgeom/geom3d/quaternion.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects3d/inertia/pointmass.h>

class QTableView;
class CPTableView;
class PartInertiaModel;
class PartInertiaDelegate;
class gl3dWingView;
class gl3dFuseView;
class gl3dGeomControls;
class FloatEdit;
class PlaneXfl;
class WingXfl;
class Fuse;
class Part;

class PointMassTable;

class PartInertiaDlg : public QDialog
{
    Q_OBJECT

    public:
        PartInertiaDlg(WingXfl *pWing, Fuse *pFuse, QWidget *pParent);

        void initDialog();
        void setPart(Part *pPart) {m_pPart = pPart;}
        void setPlaneName(QString const &name) {if(m_plabPlaneName) m_plabPlaneName->setText(name);}

        bool hasChanged() const {return m_bChanged;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private slots:
        void onOK();
        void onStructInertiaCellChanged();
        void onPointMassCellChanged();
        void onPointMassDataPasted();
        void onExportToAVL();
        void onCopyInertiaToClipboard();
        void onRedraw();
        void onResizeColumns();
        void onButton(QAbstractButton *pButton);
        void onMassRowChanged(QModelIndex index);
        void onAutoInertia();
        void reject() override;

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void setupLayout();
        void connectSignals();
        void readData();
        void updateTotalInertia();

        //layout widget variables
        QSplitter *m_pHSplitter, *m_pVSplitter;

        QDialogButtonBox *m_pButtonBox;

        QLabel *m_plabPlaneName;

        QCheckBox *m_pchAutoInertia;

        QLabel *m_plabMassUnit1, *m_plabMassUnit2;
        QLabel *m_plabLengthUnit20, *m_plabLengthUnit21, *m_plabLengthUnit22;
        QLabel *m_plabTotalMassLabel;
        QLabel *m_plabInertiaUnit10, *m_plabInertiaUnit20, *m_plabInertiaUnit30, *m_plabInertiaUnit40;

        QPushButton *m_ppbExportToAVL, *m_ppbExportToClipboard;

        CPTableView *m_pStructInertiaTable;
        PartInertiaModel *m_pStructInertiaModel;
        PartInertiaDelegate *m_pPartInertiaDelegate;

        FloatEdit *m_pdeStructMass;

        PointMassTable *m_ppmtMasses;

        FloatEdit *m_pdeTotalIxx, *m_pdeTotalIyy, *m_pdeTotalIzz, *m_pdeTotalIxz;
        FloatEdit *m_pdeXTotalCoG,*m_pdeYTotalCoG,*m_pdeZTotalCoG;
        FloatEdit *m_pdeTotalMass;

        QStackedWidget *m_p3dViewStack;
        gl3dWingView * m_pgl3dWingView;
        gl3dFuseView * m_pgl3dFuseView;
        gl3dGeomControls *m_pgl3dControls;

        Part *m_pPart;

        bool m_bChanged;

        static QByteArray s_Geometry;

        static QByteArray s_HSplitterSizes;
        static QByteArray s_VSplitterSizes;

        static Quaternion s_ab_quat;
};

