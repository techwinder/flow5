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

#include <api/t8opp.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <api/enums_objects.h>


class CPTableView;
class XflDelegate;


class T8RangeTable : public CPTableView
{
    Q_OBJECT
    public:
        T8RangeTable(QWidget *pParent = nullptr);
        ~T8RangeTable();

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;

        bool hasActiveAnalysis() const;
        void setControls();
        void fillRangeTable();
        int readRangeTable(std::vector<T8Opp> &Range, bool bActiveOnly);

        static std::vector<T8Opp> const &t8range() {return s_T8Range;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        void setName(QString const &name);

    private:
        void setupLayout();
        void connectSignals();
        void setRowEnabled(int iRow, bool bEnabled);


    private slots:
        void onRangeModelChanged();
        void onRangeTableClicked(QModelIndex index);
        void onActivate();
        void onMoveUp();
        void onMoveDown();
        void onDeleteRow();
        void onDuplicateRow();
        void onInsertBefore();
        void onInsertAfter();


    public slots:
        void onResizeColumns();

    signals:
        void xRangeChanged();

    private:
        QStandardItemModel *m_pRangeModel;
        XflDelegate *m_pRangeDelegate;

        QString m_Name; /// used only for debugging

        /* need to keep track of the ranges for each analysis when the users switches polars */
        static std::vector<T8Opp> s_T8Range;

};

