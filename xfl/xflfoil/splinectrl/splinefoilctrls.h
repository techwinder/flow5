/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QCheckBox>

#include <xflcore/linestyle.h>

class FloatEdit;
class IntEdit;
class XflDelegate;
class SplineFoil;
class CPTableView;
class LineBtn;

class SplineFoilCtrls: public QWidget
{
        Q_OBJECT
        friend class AFoil;

    public:
        SplineFoilCtrls(QWidget *pParent);

        void initDialog(SplineFoil *pSF);

        void fillPointLists();

    private slots:
        void onUpdate();
        void onSplineStyle(LineStyle);

    signals:
        void splineFoilChanged();

    private:
        void showEvent(QShowEvent *pEvent) override;

        void readData();
        void setControls();
        void setupLayout();
        void connectSignals();
        void updateSplines();

    private:
        IntEdit   *m_pieOutExtrados;
        IntEdit   *m_pieOutIntrados;
        QComboBox *m_pcbDegExtrados;
        QComboBox *m_pcbDegIntrados;
        QCheckBox *m_pchSymmetric, *m_pchCloseLE, *m_pchCloseTE;

        CPTableView *m_pcptUpperList, *m_pcptLowerList;
        QStandardItemModel *m_pUpperListModel,*m_pLowerListModel;
        XflDelegate *m_pUpperFloatDelegate, *m_pLowerFloatDelegate;

        LineBtn *m_plbSplineStyle;


    protected:
        SplineFoil *m_pSF;
};

