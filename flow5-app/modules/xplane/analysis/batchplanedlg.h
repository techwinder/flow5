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
#include <QLabel>
#include <QDialog>
#include <QSplitter>
#include <QLineEdit>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QModelIndex>

#include <api/analysisrange.h>

class ExpandableTreeView;
class ObjectTreeDelegate;
class ObjectTreeModel;

class Plane;
class PlanePolar;

class IntEdit;
class FloatEdit;
class PlainTextOutput;

class CPTableView;
class ActionItemModel;

class AnalysisRangeTable;
class T8RangeTable;
class XflExecutor;
class PlaneTask;

class BatchPlaneDlg : public QDialog
{
    Q_OBJECT
    public:
        BatchPlaneDlg(QWidget *parent = nullptr);
        ~BatchPlaneDlg();

        void initDialog();
        bool bChanged() const {return m_bChanged;}

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(1200, 900);}
        void keyPressEvent(QKeyEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();
        void setObjectProperties(QModelIndex index);
        void calculate();

    private slots:
        void onAnalysisFinished();
        void onButton(QAbstractButton *pButton);
        void onCurrentRowChanged(QModelIndex currentidx, QModelIndex );
        void onEditAnalysis();
        void onMessage(QString const &msg);
        void onPlaneTaskStarted(int iTask);
        void onResizeColumns();
        void onOption();
        void reject() override;

    private:
        bool m_bChanged;
        bool m_bHasErrors;
        ExpandableTreeView *m_pStruct;
        ObjectTreeModel *m_pModel;
        ObjectTreeDelegate *m_pDelegate;


        AnalysisRangeTable *m_pT12RangeTable;
        AnalysisRangeTable *m_pT3RangeTable;
        AnalysisRangeTable *m_pT5RangeTable;
        AnalysisRangeTable *m_pT6RangeTable;
        AnalysisRangeTable *m_pT7RangeTable;
        T8RangeTable *m_pT8Table;

        QTabWidget *m_pTabWidget;
        QCheckBox *m_pchStorePOpps;
        QCheckBox *m_pchStabDerivatives;

        QPushButton *m_ppbAnalyze;

        QSplitter *m_psplHMain;
        QSplitter *m_psplVLeft;

        PlainTextOutput *m_pptoObjectProps;

        PlainTextOutput *m_ppto;

        QDialogButtonBox *m_pButtonBox;

        QLabel *m_plabStatus;

        XflExecutor *m_pExecutor;

        static QMap<QString, bool> s_Analyses;
        static bool s_bStorePOpps;
        static QByteArray s_Geometry;


        static QByteArray s_HMainSplitterSizes;
        static QByteArray s_VLeftSplitterSizes;
};


