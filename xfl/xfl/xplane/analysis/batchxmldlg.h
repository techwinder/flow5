/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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

#include <xflcore/analysisrange.h>

class Plane;
class WPolar;

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
        WPolar * readXmlWPolarFile(QString path);
        void makeTables();
        void fillAnalysisModel();
        void updateAnalysisProperties(const WPolar *pWPolar);

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

