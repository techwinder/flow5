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

#include <QTableView>
#include <QSplitter>
#include <QComboBox>

#include <interfaces/editors/fuseedit/fusedlg.h>
#include <interfaces/widgets/view/grid.h>

class FuseXfl;
class Frame;
class FuseLineWt;
class FuseFrameWt;
class FloatEdit;
class CPTableView;
class ActionItemModel;
class XflDelegate;
class PlainTextOutput;

class FuseXflDlg : public FuseDlg
{
    Q_OBJECT

    public:
        FuseXflDlg(QWidget *pParent=nullptr);
        ~FuseXflDlg() override;

        void setBody(FuseXfl *pFuseXfl=nullptr);
        void initDialog(Fuse *pFuse);

        static bool loadSettings(QSettings &settings);
        static bool saveSettings(QSettings &settings);

        static void setFrameGrid(Grid const & grid) {s_FrameGrid=grid;}
        static void setBodyLineGrid(Grid const & grid) {s_BodyLineGrid=grid;}

    private:
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;

        void updateProperties(bool bFull=false) override;
        void blockSignalling(bool bBlock);

        void fillFrameDataTable();
        void readFrameSectionData(int sel);

        void fillPointDataTable();
        void readPointSectionData(int sel);

        void setFrame(int iFrame);

        void connectSignals();
        void setupLayout();
        void setTableUnits();
        void updateFuseDlg();

        void enableStackBtns();
        void setControls();

        void setPicture();
        void clearStack(int pos=0);
        void takePicture();

        void updateView();
        void makeCommonWts();

        void updateFuseXfl();

    private:
        void makeTables();
        void createActions();

    private slots:

        void accept() override;

        void onScaleFuse();
        void onScaleFuse(bool bFrameOnly);
        void onTranslateFuse();

        void onUndo();
        void onRedo();
        void onResetScales();
        void onExportFuseToXML();
        void onResetFuse();

        void onControlPoints();
        void onEdgeWeight();
        void onFitPrecision();
        void onFrameCellChanged();
        void onFrameClickedIn2dView();
        void onFrameItemClicked(QModelIndex const &index);
        void onInsertFrameAfter();
        void onInsertFrameBefore();
        void onInsertPointAfter();
        void onInsertPointBefore();
        void onNURBSPanels();
        void onPointCellChanged();
        void onPointClickedIn2dView();
        void onPointItemClicked(const QModelIndex &index);
        void onRemoveSelectedFrame();
        void onRemoveSelectedPoint();
        void onResizeTables();
        void onSelChangeHoopDegree(int sel);
        void onSelChangeXDegree(int sel);
        void onSelectFrame(QModelIndex const &index);
        void onSelectFrame(int iFrame);
        void onUpdateFuseDlg();

        void onRemoveFrame(int iFrame);
        void onInsertFrame(Vector3d const &pos);
        void onRemovePoint(int iPt);
        void onInsertPoint(const Vector3d &pos);

    private:
        QTabWidget *m_ptwDefinition;

        QFrame *m_pNURBSParams;
        QFrame *m_pFrameTables;
        QGroupBox *m_pgbUVParams, *m_pgbFit;
        QPushButton *m_ppbUndo, *m_ppbRedo;

        QSlider *m_pslEdgeWeight;
        QSlider *m_pslBunchAmp;

        IntEdit *m_pieNXPanels, *m_pieNHoopPanels;
        QComboBox *m_pcbXDegree, *m_pcbHoopDegree;

        FloatEdit *m_pfeFitPrecision;

        CPTableView *m_pcptFrameTable;
        ActionItemModel *m_pFrameModel;
        XflDelegate *m_pFrameDelegate;
        QItemSelectionModel *m_pSelectionModelFrame;

        CPTableView *m_pcptPointTable;
        ActionItemModel *m_pPointModel;
        XflDelegate *m_pPointDelegate;
        QItemSelectionModel *m_pSelectionModelPoint;

        QSplitter *m_pTableSplitter;
        QSplitter *m_pMainHSplitter;

        static Grid s_FrameGrid, s_BodyLineGrid;
        static QByteArray s_MainSplitterSizes, s_TableSplitterSizes;
        static int s_PageIndex;

    private:
        FuseXfl *m_pFuseXfl;
        int m_StackPos;                /**< the current position on the Undo stack */
        QList<Fuse const*> m_UndoStack;      /**< the stack of incremental modifications to the SplineFoil;
                                     we can't use the QStack though, because we need to access
                                     any point in the case of multiple undo operations */
        FuseLineWt *m_pFuseLineView;
        FuseFrameWt *m_pFrameView;

        QAction *m_pResetFuse;
        QAction *m_pExportBodyXML;
        QAction *m_pScaleBody, *m_pTranslateBody;

        QPushButton *m_ppblRedraw, *m_ppbMenuButton;

        QSplitter *m_pViewHSplitter, *m_pViewVSplitter;

        static bool s_bShowCtrlPoints;
        static QByteArray s_VViewSplitterSizes, s_HViewSplitterSizes;
        static QByteArray s_Geometry;
};

