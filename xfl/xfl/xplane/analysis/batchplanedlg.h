/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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

#include <xflcore/analysisrange.h>

class ExpandableTreeView;
class ObjectTreeDelegate;
class ObjectTreeModel;

class Plane;
class WPolar;

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

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();
        void setObjectProperties(QModelIndex index);

    private slots:
        void onAnalysisFinished();
        void onAnalyze();
        void onButton(QAbstractButton *pButton);
        void onCurrentRowChanged(QModelIndex currentidx, QModelIndex );
        void onEditAnalysis();
        void onMessage(QString const &msg);
        void onPlaneTaskStarted(int iTask);
        void onResizeColumns();
        void onStorePOpps();
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


