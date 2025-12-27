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
#include <QSettings>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QTextEdit>
#include <QLabel>
#include <QRadioButton>
#include <QAction>
#include <QSplitter>

#include <api/quaternion.h>
#include <api/segment3d.h>
#include <api/triangle3d.h>
#include <api/vector3d.h>
#include <interfaces/widgets/color/colorbtn.h>
#include <interfaces/widgets/view/grid.h>
#include <interfaces/widgets/customdlg/xfldialog.h>

class ActionItemModel;
class CPTableView;
class ColorBtn;
class FloatEdit;
class IntEdit;
class LineBtn;
class MesherWt;
class GMesherWt;
class PlainTextOutput;
class Sail;
class SailSectionView;
class Segment3d;
class Triangle3d;
class XflDelegate;
class gl3dGeomControls;
class gl3dSailView;


class SailDlg : public XflDialog
{
    Q_OBJECT

    public:
        SailDlg(QWidget *pParent=nullptr);

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;

        virtual void keyPressEvent(QKeyEvent *pEvent) override;
        virtual void initDialog(Sail *pSail);

        static void loadSettings(QSettings &pSettings);
        static void saveSettings(QSettings &settings);
        static void setSectionGrid(Grid const & grid) {s_SectionGrid=grid;}

    protected:
        void connectBaseSignals();

        virtual void updateSailGeometry();
        virtual void updateSailDataOutput();
        virtual void updateSailSectionOutput();
        void updateTriMesh();
        virtual void setSailData();
        virtual void updateView();
        virtual void setControls();

        virtual void fillSectionModel() = 0;
        virtual void fillPointModel() {}
        virtual void readData();
        virtual void readSectionData() = 0;
        virtual void readPointData() {}
        void makeBaseTables();
        virtual void resizePointTableColumns();
        virtual void resizeSectionTableColumns() = 0;

        void readMeshData();
        void makeCommonWts();


        bool guessThickTE();
        bool guessThinTE();
        bool makeFreeTELine(Vector3d const& clew, Vector3d const& peak, QVector<Segment3d> const &freeedges, QVector<Segment3d> &TELine);

        virtual void makeEdgeNodes(std::vector<Node> &nodes);

    protected slots:
        void onSailColor();
        void onExportToXml();
        void onExportMeshToStl();
        void onExportTrianglesToStl();
        void onExportToStep();
        void onLineStyle(const LineStyle &);
        virtual void onUpdateMesh();
        void onNodeDistance();
        void onPickedNodePair(QPair<int, int> nodepair);
        void onDefinitions();
        void onSetChanged();

        virtual void accept() override;


        void onBotTEPanels(bool bChecked);
        void onCheckFreeEdges();
        void onCheckTEPanels();
        void onClearHighlighted();
        void onClearTEPanels();
        void onConnectPanels();
        void onGuessTE();
        void onPanelSelected(int i3);
        void onThinSurface();
        void onTopTEPanels(bool bChecked);


        virtual void onSelectSection(int iSection) = 0;

        virtual void onAlignLuffPoints() = 0;
        virtual void onFlipXZ();
        virtual void onScaleSize();
        virtual void onScaleShape();
        virtual void onTranslateSail();
        virtual void onScaleSection();
        virtual void onSelectCtrlPoint(int);
        virtual void onUpdate();
        virtual void onSectionDataChanged();
        virtual void onPointDataChanged();

        virtual void onCurrentSectionChanged(const QModelIndex &index)=0;
        virtual void onCurrentPointChanged(const QModelIndex &) {} /** @todo remove virtual? */

        virtual void onSectionItemClicked(const QModelIndex &index) = 0;
        virtual void onPointItemClicked(const QModelIndex &) {} /** @todo remove virtual? */

        virtual void onInsertSectionBefore() = 0;
        virtual void onInsertSectionAfter() = 0;
        virtual void onDeleteSection() = 0;
        virtual void onTranslateSection() = 0;

        virtual void onInsertPointBefore(){} /** @todo remove virtual? */
        virtual void onInsertPointAfter(){} /** @todo remove virtual? */
        virtual void onDeletePoint(){} /** @todo remove virtual? */

        virtual void onResizeTableColumns();

        virtual bool deselectButtons() {return true;}

        virtual void onPickEdge(bool bPick);
        virtual void onPickedEdge(int iFace, int iEdge);
        virtual void onMakeEdgeSplits();
        virtual void onReadEdgeSplits() {}

        virtual void onRuledMesh() {}

    protected:

        bool m_bIsMeshing;


        QTabWidget *m_pTabWidget;

        QFrame *m_pfr2dView, *m_pfr3dView;

        QFrame *m_pfrMeta;
        QGroupBox *m_pSurfaceBox;
        QFrame *m_pfrThickness;

        QFrame *m_pfrMesh;
        QGroupBox *m_pgbMeshType;
        QFrame *m_pfrRuledMesh;

        QFrame *m_pfrTE;
        QPushButton *m_ppbConnectPanels;
        QPushButton *m_ppbTEBotMid, *m_ppbTETop;
        QPushButton *m_ppbGuessTE, *m_ppbCheckTE, *m_ppbClearTE;
        FloatEdit *m_pfeTEAngle;
        QCheckBox *m_pchGuessOpposite;

        QRadioButton *m_prbThin, *m_prbThick;

        QLineEdit *m_pleSailName;
        QTextEdit *m_pteSailDescription;
        QLabel *m_plabSailDef;

        QRadioButton *m_prbRuledMesh, *m_prbFreeMesh;
        QComboBox *m_pcbXDistType, *m_pcbZDistType;
        IntEdit *m_pieNXPanels;
        IntEdit *m_pieNZPanels;

        GMesherWt *m_pGMesherWt;

        CPTableView *m_pcptSections;

        CPTableView *m_pcptPoints;
        ActionItemModel *m_pPointModel;
        XflDelegate *m_pPointDelegate;

        QLineEdit *m_pleWingName;
        ColorBtn *m_pcbColor;

        FloatEdit *m_pfeRefArea, *m_pfeRefChord;

        QPushButton *m_ppbSailOps;

        gl3dSailView *m_pglSailView;
        gl3dGeomControls *m_pglSailControls;

        SailSectionView *m_p2dSectionView;

        QAction *m_pUndo, *m_pRedo;
        QAction *m_pFlipXZ;
        QAction *m_pScaleSize, *m_pScaleShape, *m_pTranslate, *m_pRotate;
//        QAction *m_pScaleArea;

        QAction *m_p3dLightAct;
        QAction *m_pAlignLuff;
        QAction *m_pExportXML, *m_pExportStep, *m_pExportMeshToSTL, *m_pExportTrianglesToSTL;
        QAction *m_pDefinitions;

        QAction *m_pBackImageLoad, *m_pBackImageClear, *m_pBackImageSettings;

        PlainTextOutput *m_ppto;

        QPushButton *m_ppbMeshOps;
        QAction *m_pConnectPanels, *m_pClearHighlighted, *m_pCheckFreeEdges, *m_pClearTE;      

        QCheckBox *m_pchFillFoil;
        LineBtn *m_plbSectionStyle;

        Sail *m_pSail;

        int m_iActiveSection;

        QSplitter *m_pViewHSplitter, *m_pViewVSplitter;
        QSplitter *m_pExternalSplitter;
        QSplitter *m_pInternalSplitter;
        QSplitter *m_pSectionTableSplitter;



        static bool s_bAxes, s_bOutline, s_bSurfaces, s_bPanels, s_bCtrlPoints, s_bCornerPts;


        static Quaternion s_ab_quat;


        static bool s_bGuessOpposite;
        static double s_TEMaxAngle;

        static bool s_bRuledMesh;

        static Grid s_SectionGrid;

        static QByteArray s_WindowGeometry;
        static QByteArray s_HSplitterSizes, s_VSplitterSizes;
        static QByteArray s_IntSplitterSizes;
        static QByteArray s_ExtSplitterSizes;
        static QByteArray s_TableSplitterSizes;

};

