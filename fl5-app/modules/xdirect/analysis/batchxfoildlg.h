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


#include <api/enums_objects.h>
#include <modules/xdirect/analysis/batchdlg.h>

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

class BatchXFoilDlg : public BatchDlg
{
    Q_OBJECT

    public:
        BatchXFoilDlg(QWidget *pParent=nullptr);
        ~BatchXFoilDlg();

        void initDialog() override;

        static void initReList();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();


    private slots:
        void onAnalyze() override;

        void onDelete();
        void onInsertBefore();
        void onInsertAfter();
        void onCellChanged(QModelIndex topLeft, QModelIndex botRight);
        void onReTableClicked(QModelIndex index);

        void onResizeColumns();
        void onSpecChanged();

    protected:
        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;

        void connectSignals();
        void readParams() override;

        void outputReList();
        void fillReModel();
        void sortRe();
        void setRowEnabled(int row, bool bEnabled);

        void readFoils(QVector<Foil *> &foils);

    private:
        QListWidget *m_plwNameList;

        QFrame *m_pfrPolars;
        QRadioButton *m_prbT1, *m_prbT2, *m_prbT3;

        FloatEdit *m_pfeXTopTr, *m_pfeXBotTr;
        QCheckBox *m_pchTransAtHinge;

        QLabel *m_plabMaType, *m_plabReType;


        CPTableView *m_pcptReTable;
        ActionItemModel *m_pReModel;
        XflDelegate *m_pFloatDelegate;

        QAction *m_pInsertBeforeAct, *m_pInsertAfterAct, *m_pDeleteAct;


    private:

        static QVector<bool> s_ActiveList;      /**< the vector list of active Re numbers */
        static QVector<double> s_ReList;        /**< the user-defined list of Re numbers, used for batch analysis */
        static QVector<double> s_MachList;      /**< the user-defined list of Mach numbers, used for batch analysis */
        static QVector<double> s_NCritList;     /**< the user-defined list of NCrit numbers, used for batch analysis */

        static double s_XTop;            /**< the point of forced transition on the upper surface */
        static double s_XBot;            /**< the point of forced transition on the lower surface */

        static xfl::enumPolarType s_PolarType;  /**< the type of analysis to perform */
        static bool s_bTransAtHinge;
};

