/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QDialogButtonBox>

#include <xflwidgets/view/grid.h>
#include <xflwidgets/customwts/gridcontrol.h>

class LineBtn;
class FloatEdit;

class GridSettingsDlg : public QDialog
{
    Q_OBJECT

    public:
        GridSettingsDlg(QWidget *pParent=nullptr);
        void initDialog(const Grid &grid, bool bShowUnit);

        Grid const &grid() {return m_Grid;}

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *event) override;

    private slots:
        void onButton(QAbstractButton *pButton);


    private:
        void setupLayout();

        GridControl *m_pGridControl;

        Grid m_Grid;

        QDialogButtonBox *m_pButtonBox;

        bool m_bWithUnit;

        static QByteArray s_WindowGeometry;

};

