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
#include <QSettings>
#include <QDialog>
#include <QSplitter>
#include <QLineEdit>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QLabel>

#include <api/analysisrange.h>

class Plane;
class PlanePolar;

class IntEdit;
class FloatEdit;
class PlainTextOutput;

class CPTableView;
class XflDelegate;
class ActionItemModel;

class AnalysisRangeTable;
class T8RangeTable;
class XPlane;
class XflExecutor;
class PlaneTask;

class BatchXmlDlg : public QDialog
{
    Q_OBJECT
    public:
        BatchXmlDlg(QWidget *parent = nullptr);

        void initDialog();
        bool bChanged() const {return m_bChanged;}

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(1200, 900);}

        static void setXPlane(XPlane *pXPlane) {s_pXPlane=pXPlane;}
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();
        void fillPlaneModel();
        PlanePolar * readXmlWPolarFile(QString path);
        void makeTables();
        void fillAnalysisModel();
        void updateAnalysisProperties(const PlanePolar *pWPolar);

    private slots:
        void onAnalysisClicked(QModelIndex const &index);
        void onAnalysisDoubleClicked(QModelIndex const &index);
        void onAnalysisFinished();
        void onAnalyze();
        void onButton(QAbstractButton *pButton);
        void onClearPlaneName();
        void onDefineWPolar();
        void onDeleteFile();
        void onDuplicateFile();
        void onEditAnalysis();
        void onListDirAnalyses();
        void onMessage(QString const &msg);
        void onPlaneClicked(QModelIndex const &index);
        void onPlaneTaskStarted(int iTask);
        void onRenameFile();
        void onResizeColumns();
        void onStorePOpps();
        void onToggleAllAnalysisSel();
        void onToggleAllPlaneSel();
        void onWPolarXmlDir();
        void reject() override;

    private:
        bool m_bChanged;
        bool m_bHasErrors;

        AnalysisRangeTable *m_pT12RangeTable;
        AnalysisRangeTable *m_pT3RangeTable;
        AnalysisRangeTable *m_pT6RangeTable;
        AnalysisRangeTable *m_pT7RangeTable;
        T8RangeTable *m_pTXRangeTable;

        CPTableView *m_pcpPlaneTable;
        XflDelegate *m_pPlaneDelegate;
        ActionItemModel *m_pPlaneModel;

        CPTableView *m_pcpAnalysisTable;
        XflDelegate *m_pAnalysisDelegate;
        ActionItemModel *m_pAnalysisModel;

        QLineEdit *m_pleWPolarDir;

        QTabWidget *m_pTabWidget;
        QCheckBox *m_pchStorePOpps;


        QPushButton *m_ppbAnalyze;

        QSplitter *m_pHSplitter, *m_pPlaneSplitter, *m_pWPolarSplitter;

        PlainTextOutput *m_pptoPlaneProps;
        PlainTextOutput *m_pptoAnalysisProps;
        PlainTextOutput *m_pptoAnalysisOutput;

        QDialogButtonBox *m_pButtonBox;

        QLabel *m_plabStatus;

        XflExecutor *m_pExecutor;

        static QMap<QString, bool> s_Analyses;
        static bool s_bStorePOpps;
        static QByteArray s_Geometry;

        static int s_LastTabIndex;

        static QByteArray s_HSplitterSizes, s_SplitterWPolarSizes, s_SplitterPlaneSizes;
        static XPlane *s_pXPlane;
};

