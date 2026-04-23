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

#include <QWidget>
#include <QStandardItemModel>

#include <interfaces/widgets/customwts/cptableview.h>
#include <api/analysisrange.h>
#include <api/enums_objects.h>

class CPTableView;
class XflDelegate;


class AnalysisRangeTable : public CPTableView
{
    Q_OBJECT
    public:
        AnalysisRangeTable(QWidget *pParent = nullptr);
        ~AnalysisRangeTable();
        std::vector<double> oppList();
        QVector<AnalysisRange> ranges();

        void setFoilPolar(bool b) {m_bFoilPolar=b;}
        void setRangeType(AnalysisRange::RangeType type) {m_eRangeType=type;}

        bool hasActiveAnalysis() const;
        void setControls(xfl::enumPolarType type);
        void fillTable();
        int readTable(QVector<AnalysisRange> &Range);

        void setPolarType(xfl::enumPolarType type) {m_PolarType=type;}
        xfl::enumPolarType polarType() const {return m_PolarType;}

        QStandardItemModel const* rangeModel() const {return m_pRangeModel;} //debug only

        static QVector<AnalysisRange> const &alphaRange() {return s_AlphaRange;}
        static QVector<AnalysisRange> const &ClRange()    {return s_ClRange;}
        static QVector<AnalysisRange> const &ReRange()    {return s_ReRange;}
        static QVector<AnalysisRange> const &ThetaRange() {return s_ReRange;}

        static QVector<AnalysisRange> const &t12Range()   {return s_T12Range;}
        static QVector<AnalysisRange> const &t3Range()    {return s_T3Range;}
        static QVector<AnalysisRange> const &t4Range()    {return s_T4Range;}
        static QVector<AnalysisRange> const &t5Range()    {return s_T5Range;}
        static QVector<AnalysisRange> const &t6Range()    {return s_T6Range;}
        static QVector<AnalysisRange> const &t7Range()    {return s_T7Range;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        void setName(QString const &name);

        void showEvent(QShowEvent *pEvent) override;

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;

        void setupLayout();
        void connectSignals();
        void setRowEnabled(int iRow, bool bEnabled);

        QVector<AnalysisRange> *activeRange() const;

    private slots:
        void onRangeModelChanged();
        void onRangeTableClicked(QModelIndex index);
        void onActivate();
        void onDeActivateAll();
        void onMoveUp();
        void onMoveDown();
        void onDeleteRow();
        void onDuplicateRow();
        void onInsertBefore();
        void onInsertAfter();


    public slots:
        void onResizeColumns();



    private:
        QStandardItemModel *m_pRangeModel;
        XflDelegate *m_pRangeDelegate;

        bool m_bFoilPolar;

        AnalysisRange::RangeType m_eRangeType;

        xfl::enumPolarType m_PolarType;

        QString m_Name; /// used only for debugging

        /* need to keep track of the ranges for each analysis when the users switches polars */
        static QVector<AnalysisRange> s_T12Range;
        static QVector<AnalysisRange> s_T3Range;
        static QVector<AnalysisRange> s_T4Range;
        static QVector<AnalysisRange> s_T5Range;
        static QVector<AnalysisRange> s_T6Range;
        static QVector<AnalysisRange> s_T7Range;
        static QVector<AnalysisRange> s_BtRange;

        // case of foil polars
        static QVector<AnalysisRange> s_AlphaRange;
        static QVector<AnalysisRange> s_ClRange;
        static QVector<AnalysisRange> s_ReRange;
        static QVector<AnalysisRange> s_ThetaRange;
};

