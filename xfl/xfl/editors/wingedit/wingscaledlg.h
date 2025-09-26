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

class WingXfl;
class FloatEdit;

class WingScaleDlg : public QDialog
{
    Q_OBJECT

    public:
        WingScaleDlg(QWidget *pParent);
        void initDialog(double const &RefSpan, double const &RefChord, double const &RefSweep, double const &RefTwist, const double &RefArea, const double &RefAR, const double &RefTR);
        void initDialog(WingXfl *pWing);
        void keyPressEvent(QKeyEvent *event) override;

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
        QCheckBox *m_pchSpan, *m_pchChord, *m_pchSweep, *m_pchTwist;
        QCheckBox *m_pchScaleArea, *m_pchScaleAR, *m_pchScaleTR;
        FloatEdit *m_pdeNewSpan, *m_pdeNewChord, *m_pdeNewSweep, *m_pdeNewTwist;
        FloatEdit *m_pdeNewArea, *m_pdeNewAR, *m_pdeNewTR;
        QLabel *m_plabRefSpan, *m_plabRefChord, *m_plabRefSweep, *m_plabRefTwist;
        QLabel *m_plabRefArea,*m_plabRefAR, *m_plabRefTR;
        QLabel *m_plabSpanRatio, *m_plabChordRatio, *m_plabSweepRatio, *m_plabTwistRatio;
        QLabel *m_plabAreaRatio,*m_plabARRatio, *m_plabTRRatio;

        bool m_bSweep, m_bSpan, m_bChord, m_bTwist;
        bool m_bArea, m_bAR, m_bTR;

        double m_NewSweep, m_NewChord, m_NewTwist, m_NewSpan, m_NewArea, m_NewAR, m_NewTR;
        double m_RefSweep, m_RefChord, m_RefTwist, m_RefSpan, m_RefArea, m_RefAR, m_RefTR;
};




