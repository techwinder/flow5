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
#include <QRadioButton>
#include <QCheckBox>

#include <api/linestyle.h>

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


