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
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QSlider>
#include <QSettings>
#include <QLabel>
#include <QRadioButton>
#include <QToolButton>
#include <QStandardItemModel>
#include <QPushButton>
#include <QSplitter>
#include <QOpenGLBuffer>
#include <QPlainTextEdit>

#include <api/vector3d.h>
#include <api/quaternion.h>
#include <interfaces/editors/wingedit/wingsectionmodel.h>
#include <interfaces/widgets/customdlg/xfldialog.h>

class WingDlg;
class gl3dWingView;
class gl3dGeomControls;
class WingXfl;
class Panel4;
class Foil;
class ColorBtn;
class FloatEdit;
class IntEdit;

class WingDlg: public XflDialog
{
    Q_OBJECT

    public:
        WingDlg(QWidget *pParent=nullptr);
        virtual ~WingDlg() override;

        QSize sizeHint() const override {return QSize(1100, 900);}

        virtual void initDialog(WingXfl *pWing);

        void setPlaneName(QString const &name) {if(m_plabPlaneName) m_plabPlaneName->setText(name);}

        int iSection() const {return m_iSection;}

        void hideSaveAsNew() {m_ppbSaveAsNew->hide();}

    protected slots:

        void onOK();
        void onMetaDataChanged();
        void onWingColor(QColor);
        void onSurfaceColor();

        void onInsertNBefore();
        void onInsertNAfter();
        void onDuplicateSection();
        void onDeleteSection();
        void onResetSection();
        void onResetMesh();
        void onScaleWing();
        void onTranslateWing();
        void onInertia();
        void onExportWingToXML();
        void onExportWingToCADFile();
        void onExportWingToStlFile();
        void onButton(QAbstractButton *pButton) override;
        void onSplitterMoved();
        void onNodeDistance();
        void onPickedNodePair(QPair<int, int> nodepair);

        virtual void onPaste() {}

    protected:

        virtual void readParams();
        virtual void updateData() = 0;
        virtual void setCurrentSection(int iSection) = 0;

        void connectWingSignals();
        virtual void accept() override;
        virtual void reject() override;
        virtual void contextMenuEvent(QContextMenuEvent *pEvent) override;
        virtual void keyPressEvent(QKeyEvent *pEvent) override;
        virtual void resizeEvent(QResizeEvent *pEvent) override;
        virtual void showEvent(QShowEvent *pEvent) override;
        virtual void hideEvent(QHideEvent *pEvent) override;

        void makeCommonWts();

        bool checkWing();
        void computeGeometry();

        void setWingProps();
        void setScale();
        int VLMGetPanelTotal();
        bool VLMSetAutoMesh(int total=0);

        void updateWingOutput();

        virtual void setControls();

        void insertNSectionsAfter(int n0, int nsec);

    protected:

        gl3dWingView *m_pglWingView;              /**< a pointer to the openGL 3.0 widget where 3d calculations and rendering are performed */
        gl3dGeomControls *m_pglControls;

        QLabel *m_plabPlaneName;
        QLineEdit *m_pleWingName;
        QPlainTextEdit *m_ppteDescription;
        ColorBtn *m_pcbColor;

        QPushButton *m_ppbActionMenuButton;
        QPushButton *m_ppbSaveAsNew;

        QAction *m_pResetMesh;
        QAction *m_pTranslateWing, *m_pScaleWing, *m_pInertia;
        QAction *m_pExportToXml;
        QAction *m_pExportToCADFile, *m_pExportToStl;

        QAction *m_pInsertBefore, *m_pInsertAfter;
        QAction *m_pInsertNBefore, *m_pInsertNAfter;
        QAction *m_pDeleteSection, *m_pResetSection, *m_pDuplicateSection;
        QAction *m_pCopyAction, *m_pPasteAction;

        QAction *m_pBackImageLoad, *m_pBackImageClear, *m_pBackImageSettings;

        WingXfl *m_pWing;

        bool m_bRightSide;

        int m_iSection;

        static QByteArray s_Geometry;

        static double s_MaxEdgeDeflection;
        static double s_MaxEdgeLength;
        static double s_QualityBound;
        static int s_MaxMeshIter;
        static bool s_bOutline, s_bSurfaces, s_bVLMPanels, s_bAxes, s_bShowMasses, s_bFoilNames;

        static Quaternion s_ab_quat;
};

