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

#include <api/quaternion.h>
#include <api/vector3d.h>
#include <api/pointmass.h>

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
class Inertia;


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
        QString toString(const Inertia &in) const;

        //layout widget variables
        QSplitter *m_pHSplitter, *m_pVSplitter;

        QDialogButtonBox *m_pButtonBox;

        QLabel *m_plabPlaneName;

        QCheckBox *m_pchAutoInertia;

        QPushButton *m_ppbExportToAVL, *m_ppbExportToClipboard;

        CPTableView *m_pStructInertiaTable;
        PartInertiaModel *m_pStructInertiaModel;
        PartInertiaDelegate *m_pPartInertiaDelegate;

        FloatEdit *m_pfeStructMass;

        PointMassTable *m_ppmtMasses;

        FloatEdit *m_pfeTotalIxx, *m_pfeTotalIyy, *m_pfeTotalIzz, *m_pfeTotalIxz;
        FloatEdit *m_pfeXTotalCoG,*m_pfeYTotalCoG,*m_pfeZTotalCoG;
        FloatEdit *m_pfeTotalMass;

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

