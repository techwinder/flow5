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

#include <QSettings>
#include <QRadioButton>
#include <QCheckBox>
#include <QSplitter>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QModelIndex>


#include <api/quaternion.h>
#include <api/vector3d.h>
#include <interfaces/widgets/customdlg/xfldialog.h>

class CPTableView;
class ColorBtn;
class FloatEdit;
class Foil;
class IntEdit;
class Panel4;
class WingDlg;
class WingSectionDelegate;
class WingSectionModel;
class WingXfl;
class gl3dGeomControls;
class gl3dWingView;

class WingXflDlg : public XflDialog
{
    Q_OBJECT

    public:
        WingXflDlg(QWidget *pParent=nullptr);
        ~WingXflDlg();

        void makeWingTable();
        void initDialog(WingXfl*pWing);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        QSize sizeHint() const override {return QSize(1100, 900);}

        void setPlaneName(QString const &name) {if(m_plabPlaneName) m_plabPlaneName->setText(name);}

        int iSection() const {return m_iSection;}

        void hideSaveAsNew() {m_ppbSaveAsNew->hide();}

    protected slots:


        void onButton(QAbstractButton *pButton) override;
        void onCellChanged();
        void onCopy();
        void onDeleteSection();
        void onDuplicateSection();
        void onExportWingToCADFile();
        void onExportWingToStlFile();
        void onExportWingToXML();
        void onInertia();
        void onInsertNAfter();
        void onInsertNBefore();
        void onMetaDataChanged();
        void onNodeDistance();
        void onOK();
        void onPaste();
        void onPickedNodePair(QPair<int, int> nodepair);
        void onResetMesh();
        void onResetSection();
        void onRowChanged(const QModelIndex &currentindex, const QModelIndex &);
        void onScaleWing();
        void onSide();
        void onSplitterMoved();
        void onSurfaceColor();
        void onTipStrips();
        void onTranslateWing();
        void onWingColor(QColor);
        void onWingSides();
        void onWingTableClicked(QModelIndex index);
        void onWingTableContextMenu(QPoint);


    protected:

        bool VLMSetAutoMesh(int total=0);
        bool checkWing();
        int VLMGetPanelTotal();
        void accept() override;
        void computeGeometry();
        void connectSignals();
        void connectWingSignals();
        void contextMenuEvent(QContextMenuEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void insertNSectionsAfter(int n0, int nsec);
        void keyPressEvent(QKeyEvent *pEvent) override;
        void makeCommonWts();
        void readParams();
        void reject() override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void setControls();
        void setCurrentSection(int iSection);
        void setScale();
        void setWingProps();
        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;
        void updateData();
        void updateWingOutput();

    private:
        QCheckBox *m_pchTwoSided;
        QCheckBox *m_pchsymmetric;
        QCheckBox *m_pchCloseInnerSide;

        IntEdit *m_pieTipStrips;

        QRadioButton *m_prbLeftSide, *m_prbRightSide;

        CPTableView *m_pcptSections;
        WingSectionModel *m_pSectionModel;
        WingSectionDelegate *m_pSectionDelegate;

        QMenu *m_pTableContextMenu;
        QSplitter *m_pHSplitter, *m_pVSplitter;

        static QByteArray s_HSplitterSizes, s_VSplitterSizes;

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

