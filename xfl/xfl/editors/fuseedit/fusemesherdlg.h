/****************************************************************************

    flow5
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

#include <QLabel>
#include <QAbstractButton>
#include <QPushButton>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QSplitter>
#include <QSettings>
#include <QStack>

#include <TopoDS_Face.hxx>


#include <xflpanels/mesh/trimesh.h>


class gl3dGeomControls;

class Fuse;
class MesherWt;
class gl3dFuseView;
class gl3dShapeView;
class IntEdit;
class PlainTextOutput;

class FuseMesherDlg : public QDialog
{
    Q_OBJECT
    public:
        FuseMesherDlg(QWidget *parent = nullptr);
        ~FuseMesherDlg()  override = default;

        QSize sizeHint() const override {return QSize(1300, 700);}
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *) override;
        void customEvent(QEvent *pEvent) override;

        void initDialog(Fuse *pFuse);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    public slots:
        void onButton(QAbstractButton *pButton);
        void onCheckMesh();
        void onClearHighlighted();

        void onUpdate3dView();
        void onUpdateHighlightedPanels();

        void onResetFuseMesh();

        void onDoubleNodes();
        void onMergeNodes(bool bMerge);
        void onPickedNodePair(QPair<int, int> nodepair);
        void onUndoLastMerge();

        void onConnectTriangles();
        void onCheckFreeEdges();

        void onNodeDistance();

    private:
        void setupLayout();
        void connectSignals();
        void outputPanelProperties(int panelindex);

    private:

        MesherWt *m_pMesherWt;

        QSplitter *m_pHSplitter;
        QDialogButtonBox *m_pButtonBox;

        QTabWidget *m_ptabViewWt;

        gl3dFuseView *m_pglFuseView;
        gl3dGeomControls *m_pgl3dFuseControls;

        gl3dShapeView *m_pglShapeView;
        gl3dGeomControls *m_pgl3dShapeControls;

        QAction *m_pCheckMesh, *m_pClearHighlighted, *m_pCleanDoubleNode, *m_pRestoreFuseMesh;
        QAction *m_pConnectPanels, *m_pCheckFreeEdges;
        QPushButton *m_ppbMoveNode, *m_ppbUndoLastMerge;

        PlainTextOutput *m_pptoOutput;

        Fuse *m_pFuse;

        QStack<TriMesh> m_MeshStack;

        static int s_ViewIndex;
        static bool s_bOutline, s_bSurfaces, s_bVLMPanels, s_bAxes, s_bShowMasses;


        static QByteArray s_HSplitterSizes;
        static QByteArray s_Geometry;

        static Quaternion s_ab_quat_fuse;
        static Quaternion s_ab_quat_shape;
};


