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

#include <interfaces/editors/fuseedit/xflfuseedit/fusexfldlg.h>


class FuseXfl;
class Frame;


class FloatEdit;
class CPTableView;
class ActionItemModel;
class XflDelegate;
class PlainTextOutput;

class FuseXflDefDlg : public FuseXflDlg
{
    Q_OBJECT

    public:
        FuseXflDefDlg(QWidget *pParent=nullptr);
        ~FuseXflDefDlg() override;

        void setBody(FuseXfl *pFuseXfl=nullptr) override;
        void initDialog(Fuse *pFuse) override;

        static bool loadSettings(QSettings &settings);
        static bool saveSettings(QSettings &settings);

        static void setFrameGrid(Grid const & grid) {s_FrameGrid=grid;}
        static void setBodyLineGrid(Grid const & grid) {s_BodyLineGrid=grid;}

    private:
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void blockSignalling(bool bBlock);

        void fillFrameDataTable();
        void readFrameSectionData(int sel);

        void fillPointDataTable();
        void readPointSectionData(int sel);

        void setFrame(int iFrame);

        void connectSignals();
        void setupLayout();
        void setTableUnits();
        void updateFuseDlg() override;

        void enableStackBtns() override;
        void setControls();

        void setPicture() override;

    private:
        void makeTables();

    private slots:       
        void onControlPoints();
        void onConvertToFlatFace() override;
        void onEdgeWeight();
        void onFitPrecision();
        void onFrameCellChanged();
        void onFrameClickedIn2dView() override;
        void onFrameItemClicked(QModelIndex const &index);
        void onInsertFrameAfter();
        void onInsertFrameBefore();
        void onInsertPointAfter();
        void onInsertPointBefore();
        void onNURBSPanels();
        void onPointCellChanged();
        void onPointClickedIn2dView() override;
        void onPointItemClicked(const QModelIndex &index);
        void onRemoveSelectedFrame();
        void onRemoveSelectedPoint();
        void onResizeTables();
        void onSelChangeHoopDegree(int sel);
        void onSelChangeXDegree(int sel);
        void onSelectFrame(QModelIndex const &index);
        void onSelectFrame(int iFrame);
        void onUpdateFuseDlg();

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

};

