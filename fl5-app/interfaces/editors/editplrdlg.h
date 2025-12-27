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

#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QSettings>
#include <QModelIndex>
#include <QSplitter>
#include <QVBoxLayout>
#include <QScrollArea>

class QLabel;
class Polar;
class PlanePolar;
class BoatPolar;
class XflDelegate;
class ActionItemModel;
class CPTableView;


class EditPlrDlg : public QDialog
{
    Q_OBJECT

    public:
        EditPlrDlg(QWidget *pParent=nullptr);

        void initDialog(Polar *pPolar, PlanePolar *pWPolar, BoatPolar *pBtPolar=nullptr);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private slots:
        void accept() override;
        void onDataChanged();
        void onInsertRowBefore();
        void onInsertRowAfter();
        void onDeleteRow();
        void onDeleteAllPoints();
        void onTableClicked(QModelIndex index);
        void onButton(QAbstractButton*pButton);
        void onSplitterMoved();
        void onPolarVariable();
        void onWPolarVariable();
        void onBtPolarVariable();
        void onResizeData();

    private:
        void setupLayout();
        void connectSignals();
        void createCheckBoxes();
        void fillPolarData();
        void fillWPolarData();
        void fillBtPolarData();
        void readWPolarExtData();
        void resizeColumns();

        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent*pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(900,700);}

    signals:
        void dataChanged();

    private:
        Polar *m_pPolar;
        PlanePolar *m_pWPolar;
        BoatPolar *m_pBtPolar;

        QPushButton *m_ppbResizeDataBtn;
        QDialogButtonBox *m_pButtonBox;
        QLabel *m_plabPlaneName, *m_plabPolarName;
        QScrollArea *m_pScrollArea;

        CPTableView *m_pcptPoint;
        ActionItemModel *m_pPointModel;
        XflDelegate *m_pActionDelegate;

        QVector<QCheckBox*> m_pchVariables;

        QSplitter *m_pHSplitter;

        bool m_bDataChanged;


        static QVector<bool> s_PolarVariables;
        static QVector<bool> s_WPolarVariables;
        static QVector<bool> s_BtPolarVariables;

        static QByteArray s_HSplitterSizes;
        static QByteArray s_WindowGeometry;

};





