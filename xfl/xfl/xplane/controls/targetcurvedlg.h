/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QRadioButton>
#include <QCheckBox>

#include <xflcore/linestyle.h>

class FloatEdit;
class LineBtn;

class TargetCurveDlg : public QDialog
{
    Q_OBJECT
    public:
        TargetCurveDlg(QWidget *pParent=nullptr);
        void initDialog(bool bShowElliptic, bool bShowBell, bool bMaxCl, double curveExp, const LineStyle &ls);
        double bellCurveExp() const;
        bool bShowBellCurve() const;
        bool bShowEllipticCurve() const;
        bool bMaxCl() const;
        LineStyle lineStyle() const;

    private:
        void setupLayout();

    private slots:
        void onCurveStyle();

    private:
        FloatEdit *m_pdeExptEdit;
        QRadioButton *m_prbRadio1, *m_prbRadio2;
        QCheckBox *m_pchShowBellCurve, *m_pchShowEllipticCurve;

        LineBtn *m_plbCurveStyle;
};


