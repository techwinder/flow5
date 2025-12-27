/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois

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
#include <QCheckBox>
#include <QRadioButton>
#include <QFrame>
#include <QSplitter>
#include <QFile>
#include <QSettings>
#include <QListWidget>
#include <QLabel>

/*
#include <QAbstractButton>
#include <QGroupBox>
#include <QModelIndex>
#include <QPushButton>
#include <QTimer>
 */

#include <api/enums_objects.h>

class ActionItemModel;
class AnalysisRangeTable;
class CPTableView;
class FloatEdit;
class Foil;
class IntEdit;
class PlainTextOutput;
class Polar;
class XDirect;
class XflDelegate;
class XFoilTask;
struct FoilAnalysis;

class BatchDlg : public QDialog
{
    Q_OBJECT

    public:
        BatchDlg(QWidget *pParent=nullptr);
        ~BatchDlg();

        virtual void initDialog();

        QSize sizeHint() const override {return QSize(1100,900);}

        void setFoil(Foil *pFoil) {m_pFoil=pFoil;}


        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static void setXDirect(XDirect*pXDirect){s_pXDirect=pXDirect;}

    protected:
        void cleanUp();
        void customEvent(QEvent *pEvent) override;
        void makeCommonWts();

        void batchLaunch();

    protected slots:
        virtual void onAnalyze() = 0;

        void onAcl();
        void onButton(QAbstractButton *pButton);
        void onClose();
        void onSpecChanged();
        void onUpdatePolarView();

    protected:
        void keyPressEvent(QKeyEvent  *pEvent) override;
        virtual void showEvent(QShowEvent *pEvent) override;
        virtual void hideEvent(QHideEvent *pEvent) override;
        void reject() override;

        void connectBaseSignals();
        virtual void readParams();

        void setFileHeader();
        void writeString(const QString &strong);

    protected:
        QCheckBox *m_pchUpdatePolarView;
        QCheckBox *m_pchStoreOpp;

        QRadioButton *m_prbAlpha, *m_prbCl;

        QTabWidget *m_pfrRangeVars;
        QFrame *m_pfrOptions;

        QDialogButtonBox *m_pButtonBox;
        QPushButton *m_ppbAnalyze;
        PlainTextOutput *m_ppto;

        QSplitter *m_pHSplitter;
        QTabWidget *m_pLeftTabWt;

        AnalysisRangeTable *m_pT12RangeTable;
        AnalysisRangeTable *m_pT4RangeTable;
        AnalysisRangeTable *m_pT6RangeTable;

        bool m_bCancel;             /**< true if the user has clicked the cancel button */
        bool m_bIsRunning;          /**< true until all the pairs of (foil, polar) have been calculated */

        QFile *m_pXFile;                   /**< a pointer to the output log file */

        Foil *m_pFoil;                  /**< a pointer to the current Foil */

    protected:
        int m_nTaskStarted;         /**< the number of started tasks */
        int m_nTaskDone;            /**< the number of finished tasks */
        int m_nAnalysis;            /**< the number of analysis pairs to run */

        QVector<FoilAnalysis> m_AnalysisPair;  /**< the list of all analysis to be performed. Once performed, an analysis is removed from the list. */
        std::vector<XFoilTask*> m_Tasks;


    protected:
        static bool s_bAlpha;              /**< true if the analysis should be performed for a range of aoa rather than lift coefficient */

        static QByteArray s_Geometry;
        static XDirect* s_pXDirect;           /**< a void pointer to the unique instance of the QXDirect class */
        static bool s_bUpdatePolarView;    /**< true if the polar graphs should be updated during the analysis */

        static QByteArray s_HSplitterSizes;

};

