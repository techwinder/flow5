/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QLabel>

class Sail;
class FloatEdit;

class SailScaleDlg : public QDialog
{
    Q_OBJECT
    public:
        SailScaleDlg(QWidget *pParent);

        void initDialog(Sail *pSail);
        void keyPressEvent(QKeyEvent *pEvent) override;

    private:
        void setupLayout();
        void readData();
        void setResults();
        void enableControls();

    private slots:
        void onOK();
        void onClickedCheckBox();
        void onEditingFinished();
        void onButton(QAbstractButton*pButton);

    public:
        QDialogButtonBox *m_pButtonBox;

        QCheckBox *m_pchScaleArea, *m_pchScaleAR, *m_pchTwist;
        FloatEdit *m_pdeNewTwist, *m_pdeNewArea, *m_pdeNewAR;

        QLabel *m_plabRefArea, *m_plabRefAR, *m_plabRefTwist;
        QLabel *m_plabAreaRatio,*m_plabARRatio, *m_plabTwistRatio;

        bool m_bArea, m_bAR, m_bTwist;

        double m_NewArea, m_NewAR, m_NewTwist;
        double m_RefArea, m_RefAR, m_RefTwist;
};




