/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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
class WPolar;
class BoatPolar;
class XflDelegate;
class ActionItemModel;
class CPTableView;


class EditPlrDlg : public QDialog
{
    Q_OBJECT

    public:
        EditPlrDlg(QWidget *pParent=nullptr);

        void initDialog(Polar *pPolar, WPolar *pWPolar, BoatPolar *pBtPolar=nullptr);

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
        WPolar *m_pWPolar;
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





