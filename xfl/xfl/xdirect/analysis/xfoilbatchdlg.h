/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QAbstractButton>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QModelIndex>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QSplitter>
#include <QTimer>

#include <xflcore/enums_objects.h>

class ActionItemModel;
class AnalysisRangeTable;
class CPTableView;
class FloatEdit;
class Foil;
class IntEdit;
class PlainTextOutput;
class Polar;
class XDirect;
class XFoilTask;
class XflDelegate;
struct FoilAnalysis;

class XFoilBatchDlg : public QDialog
{
    Q_OBJECT

    public:
        XFoilBatchDlg(QWidget *pParent=nullptr);
        ~XFoilBatchDlg();

        void initDialog();

        QSize sizeHint() const override {return QSize(1100,900);}

        void setFoil(Foil *pFoil) {m_pFoil=pFoil;}

        static void initReList();
        static void setXDirect(XDirect*pXDirect){s_pXDirect=pXDirect;}
        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void cleanUp();
        void customEvent(QEvent *pEvent) override;
        void setupLayout();

        void updateOutput(const QString &str);

    private slots:
        void onAnalyze();
        void onTimerEvent();

        void onAcl();
        void onButton(QAbstractButton *pButton);
        void onClose();
        void onInitBL(int);
        void onSpecChanged();
        void onUpdatePolarView();

        void onDelete();
        void onInsertBefore();
        void onInsertAfter();
        void onCellChanged(QModelIndex topLeft, QModelIndex botRight);
        void onReTableClicked(QModelIndex index);

        void onResizeColumns();


    protected:
        void keyPressEvent(QKeyEvent  *pEvent) override;
        virtual void showEvent(QShowEvent *pEvent) override;
        virtual void hideEvent(QHideEvent *pEvent) override;
        void reject() override;
        void resizeEvent(QResizeEvent *pEvent) override;

        void connectBaseSignals();
        void readParams();

        void outputReList();
        void setFileHeader();
        void writeString(const QString &strong);
        void fillReModel();
        void sortRe();
        void setRowEnabled(int row, bool bEnabled);

        void readFoils(QVector<Foil *> &foils);

    private:
        QListWidget *m_plwNameList;

        QFrame *m_pfrPolars;
        QRadioButton *m_prbT1, *m_prbT2, *m_prbT3;

        QRadioButton *m_prbAlpha, *m_prbCl;

        FloatEdit *m_pdeXTopTr, *m_pdeXBotTr;

        QLabel *m_plabMaType, *m_plabReType;
        QCheckBox *m_pchInitBL, *m_pchUpdatePolarView;
        QCheckBox *m_pchStoreOpp;

        QFrame *m_pfrRangeVars;
        QFrame *m_pfrOptions;

        QDialogButtonBox *m_pButtonBox;
        QPushButton *m_ppbAnalyze;
        PlainTextOutput *m_ppto;

        QSplitter *m_pHSplitter;

        CPTableView *m_pcptReTable;
        ActionItemModel *m_pReModel;
        XflDelegate *m_pFloatDelegate;

        QAction *m_pInsertBeforeAct, *m_pInsertAfterAct, *m_pDeleteAct;

        AnalysisRangeTable *m_pAnalysisRangeTable;

        bool m_bCancel;             /**< true if the user has clicked the cancel button */
        bool m_bIsRunning;          /**< true until all the pairs of (foil, polar) have been calculated */

        QFile *m_pXFile;                   /**< a pointer to the output log file */

        Foil *m_pFoil;                  /**< a pointer to the current Foil */

    private:
        int m_nTaskStarted;         /**< the number of started tasks */
        int m_nTaskDone;            /**< the number of finished tasks */
        int m_nAnalysis;            /**< the number of analysis pairs to run */

        QTimer m_Timer;

        QVector<FoilAnalysis*> m_AnalysisPair;  /**< the list of all analysis to be performed. Once performed, an analysis is removed from the list. */

    private:
        static bool s_bAlpha;              /**< true if the analysis should be performed for a range of aoa rather than lift coefficient */

        static QVector<bool> s_ActiveList;    /**< the vector list of active Re numbers */
        static QVector<double> s_ReList;        /**< the user-defined list of Re numbers, used for batch analysis */
        static QVector<double> s_MachList;      /**< the user-defined list of Mach numbers, used for batch analysis */
        static QVector<double> s_NCritList;     /**< the user-defined list of NCrit numbers, used for batch analysis */

        static double s_XTop;            /**< the point of forced transition on the upper surface */
        static double s_XBot;            /**< the point of forced transition on the lower surface */

        static xfl::enumPolarType s_PolarType;  /**< the type of analysis to perform */

        static bool s_bInitBL;             /**< true if the boundary layer should be restored to the default value before each polar analysis */

        static QByteArray s_Geometry;
        static XDirect* s_pXDirect;           /**< a void pointer to the unique instance of the QXDirect class */
        static bool s_bUpdatePolarView;    /**< true if the polar graphs should be updated during the analysis */

        static QByteArray s_HSplitterSizes;

};

