/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QWidget>
#include <QStandardItemModel>

#include <xflcore/t8opp.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflcore/enums_objects.h>

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
        int readRangeTable(QVector<T8Opp> &Range, bool bActiveOnly);

        static QVector<T8Opp> const &tXrange() {return s_T8Range;}

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
        static QVector<T8Opp> s_T8Range;

};

