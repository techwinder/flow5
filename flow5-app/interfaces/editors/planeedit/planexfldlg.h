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

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QRadioButton>
#include <QSettings>
#include <QSplitter>
#include <QStack>
#include <QStackedWidget>
#include <QTableView>
#include <QTextEdit>

#include <TopTools_ListOfShape.hxx>
#include <TopoDS_Wire.hxx>

#include <interfaces/editors/planeedit/planedlg.h>
#include <api/triangle3d.h>
#include <api/linestyle.h>
#include <api/trimesh.h>

class gl3dPlaneXflView;
class gl3dGeomControls;

class CPTableView;
class PlanePartDelegate;
class PlanePartModel;

class PlaneXfl;
class Fuse;

class WingXfl;
class Part;
class WingSection;
class Surface;
class FloatEdit;
class IntEdit;

class MesherWt;
class GMesherWt;
class OccTessCtrlsWt;

class PlaneXflDlg : public PlaneDlg
{
    Q_OBJECT
    public:
        PlaneXflDlg(QWidget *pParent);
        virtual ~PlaneXflDlg() override;

        void initDialog(Plane *pPlane, bool bAcceptName) override;

        void customEvent(QEvent *pEvent) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    protected:
        void setupLayout();
        void makeActions();
        void connectSignals();

        void cutFuseShapes(Fuse *pFuse, const Vector3d &fusepos, TopoDS_ListOfShape &tools);
        void cutFuseXflRightShapes(Fuse *pFuse, Vector3d const &fusepos, TopoDS_ListOfShape &tools);

        void editWing(WingXfl *pWing, bool bAdvanced=false);
        void editFuse(int iFuse, bool bAdvanced=false);
        void readParams();

        void readVSPSection(QTextStream &stream, QString &wingname, int &index, WingSection &ws);

        void setControls() override;

        void updateData();

        void scaleWing(WingXfl *pWing);
        void scaleFuse(Fuse*pFuse);

        bool cancelPicks();

        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;


    private:
        Fuse *activeFuse(); // depends on the active tab
        int selectedPart();
        void makePartTable();


        void takePicture();
        void setPicture();
        void enableStackBtns();
        void clearStack(int pos);

        bool endPanelMods();

        bool makeFragments();

        QVector<WingXfl> thinWingList() const;

    protected slots:
        void onButton(QAbstractButton *pButton) override;
        void onClearOutput();
        void onCutFuse();
        void onPartDataChanged();
        void onDuplicatePart();
        void onEditPart();
        void onExportMeshToSTLFile();
        void onFlipNormals() override;
        void onFuseMeshDlg();
        void onHighlightSel(bool bSel);
        void onInsertElevator();
        void onInsertFin();
        void onImportOtherWing();
        void onImportOtherFuse();
        void onInsertFuseOcc();
        void onInsertFuseStl();
        void onInsertEllipticWing();
        void onInsertCADShape();
        void onInsertSTLCylinderFuse();
        void onInsertSTLSphereFuse();
        void onInsertFuseXfl();
        void onInsertFuseXml();
        void onInsertWing();
        void onInsertWingFromXml();
        void onInsertWingFromVSP();
        void onThinListClick();
        void onMovePartDown();
        void onMovePartUp();
        void onNamesChanged();
        void onOK(int iExitCode=QDialog::Accepted) override;
        void onPartInertia();
        void onPartItemClicked(QModelIndex index);
        void onPlaneInertia() override;
        void onUpdatePlane() override;
        void onRemovePart();
        void onResetFuse();
        void onResetFuseMesh();
        void onResizeColumns();
        void onScalePart();
        void onScalePlane();
        void onSelectPart(Part *pPart);
        void onSplitterMoved(int pos, int index);
        void onTabChanged(int iNewTab);
        void onTessellation();
        void onThinThick();
        void onUpdateHighlightedPanels();
        void onUpdatePlaneProps() override;
        void onUpdateMesh();
        void onPickedNodePair(QPair<int, int> nodepair);

        void onMergeNodes(bool bIsMerging);

        void onSelectPanels(bool bSelect);
        void onDeleteP3Selection();

        void onMakeP3(bool bCheck);
        void onMakeP3Strip(bool bCheck);
        void onPickedNode(int iNode) override;

        void onUndo();
        void onRedo();

    protected:

        PlaneXfl *m_pPlaneXfl;


        QCheckBox *m_pchHighlightSel;

        QPushButton *m_ppbExportBody;

        QAction *m_pResetFuse, *m_pTessellation;

        GMesherWt *m_pGMesherWt;
        QAction *m_pRestoreFuseMesh, *m_pFuseMesher;

        QFrame *m_pMeshCorrectionsFrame;
        QPushButton *m_ppbCheckMenuBtn, *m_ppbMoveNode;
        QPushButton *m_ppbSelectPanels, *m_ppbDeletePanel;
        QPushButton *m_ppbMakeP3, *m_ppbMakeP3Strip;
        QCheckBox *m_pchMakeP3Opposite;

        QLabel *m_plabStackInfo;
        QPushButton *m_ppbUndo, *m_ppbRedo;
        int m_StackPos;                  /**< the current position on the Undo stack */
        QList<TriMesh> m_UndoStack;      /**< the stack of incremental modifications to the SplineFoil;
                                              we can't use the QStack though, because we need to access
                                              any point in the case of multiple undo operations */

        QComboBox *m_pcbStepFormat;

        QMenu *m_pPartMenu;
        QAction *m_pScalePlane;
        QAction *m_pMoveUp, *m_pMoveDown;
        QAction *m_pRemovePart, *m_pDuplicatePart, *m_pEditPartDef;
//        QAction *m_pEditPartObject;
        QAction *m_pInsertWing, *m_pInsertWingXml, *m_pInsertWingVSP, *m_pInsertWingOther;
        QAction *m_pInsertElev, *m_pInsertFin;
        QAction *m_pInsertFuseXflSpline, *m_pInsertFuseXflFlat, *m_pInsertFuseXflSections;
        QAction *m_pInsertFuseXml, *m_pInsertFuseOther, *m_pInsertFuseOcc, *m_pInsertFuseStl;
        QAction *m_pInsertEllWing;
        QAction *m_pInsertSTLSphere, *m_pInsertSTLCylinder;
        QAction *m_pInsertCADSphere, *m_pInsertCADCylinder, *m_pInsertCADBox;
        QAction *m_pPartInertia, *m_pPartScale;
        QAction *m_pExportMeshSTL;


        QSplitter *m_pHSplitter;

        QRadioButton *m_prbThin, *m_prbThick;
        QListWidget *m_plwWings;

        QSplitter *m_pPartSplitter;

        QTabWidget *m_pLeftTabWidget;
        QFrame *m_pglPlaneViewFrame;

        CPTableView *m_pcptParts;
        PlanePartModel *m_pPartModel;
        PlanePartDelegate *m_pPartDelegate;

        QStack<TriMesh> m_MeshStack;


        static double s_StitchPrecision;
        static double s_NodeMergeDistance;

        static QByteArray s_HSplitterSizes;
        static QByteArray s_PartSplitterSizes;
        static QByteArray s_XflGeometry;

        static int s_iActivePage;

};

