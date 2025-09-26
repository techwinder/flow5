/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once
#include <QDialog>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QListWidget>
#include <QComboBox>

class IntEdit;

class SplineApproxDlg : public QDialog
{
    Q_OBJECT

    public:
        SplineApproxDlg(QWidget *pParent);

        void initDialog(bool bCubic, int degree, int nCtrlPts);

        QString selectedFoilName() const {if(m_plwNames->currentItem()) return m_plwNames->currentItem()->text(); else return QString();}
        int degree() const;
        int nCtrlPoints() const;

    private:
        void setupLayout();
        QSize sizeHint() const override {return QSize(700,900);}

    private slots:
        void onButton(QAbstractButton *pButton);

    private:

        QListWidget *m_plwNames;

        QComboBox *m_pcbSplineDegree;
        IntEdit	*m_pieCtrlPoints;

        QDialogButtonBox *m_pButtonBox;

};

