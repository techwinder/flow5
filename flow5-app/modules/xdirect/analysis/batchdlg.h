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

        bool hasChanges() const {return m_bChanged;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static void setXDirect(XDirect*pXDirect){s_pXDirect=pXDirect;}

    protected:

        virtual void hideEvent(QHideEvent *pEvent) override;
        virtual void showEvent(QShowEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;
        void keyPressEvent(QKeyEvent  *pEvent) override;
        void reject() override;

        virtual void readParams();

        void batchLaunch();
        void cleanUp();
        void connectBaseSignals();
        void makeCommonWts();
        void runQTask(XFoilTask *pTask);
        void setFileHeader();
        void startOneTask();
        void writeString(const QString &strong);

    protected slots:
        virtual void onCalculate() = 0;

        void onAcl();
        void onButton(QAbstractButton *pButton);
        void onClose();
        void onSpecChanged();


    protected:
        QCheckBox *m_pchStoreOpp;

        QRadioButton *m_prbAlpha, *m_prbCl;

        QFrame *m_pfrOpp;

        QDialogButtonBox *m_pButtonBox;
        QPushButton *m_ppbAnalyze;
        PlainTextOutput *m_ppto;

        QSplitter *m_pHSplitter;
        QTabWidget *m_pLeftTabWt;

        AnalysisRangeTable *m_pT12RangeTable;
        AnalysisRangeTable *m_pT4RangeTable;
        AnalysisRangeTable *m_pT6RangeTable;

        bool m_bCancel;
        bool m_bIsRunning;

        bool m_bChanged;

        QFile *m_pXFile;

        Foil *m_pFoil;

    protected:
        int m_nTaskStarted;         /**< the number of started tasks */
        int m_nTaskDone;            /**< the number of finished tasks */
        int m_nAnalysis;            /**< the number of analysis pairs to run */

        QVector<FoilAnalysis> m_AnalysisPair;

        static bool s_bAlpha;
        static bool s_bStoreOpp;

        static QByteArray s_Geometry;
        static XDirect* s_pXDirect;

        static QByteArray s_HSplitterSizes;

};

