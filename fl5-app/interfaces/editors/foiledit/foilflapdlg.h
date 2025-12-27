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


#include <QCheckBox>
#include <QFrame>

#include <interfaces/editors/foiledit/foildlg.h>


class Foil;
class FoilWt;
class FloatEdit;


class FoilFlapDlg : public FoilDlg
{
    Q_OBJECT

    public:
        FoilFlapDlg(QWidget *pParent);

    public:
        void initDialog(Foil *pFoil) override;


    private:
        void enableTEFlap(bool bEnable);
        void enableLEFlap(bool bEnable);
        void readParams() override;
        void setupLayout();

    protected:
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

    private slots:
        void onLEFlapCheck();
        void onTEFlapCheck();

        void onOK() override;
        void onApply() override;

    private:
        bool m_bTEFlap;
        bool m_bLEFlap;


        double m_LEXHinge, m_LEYHinge, m_LEFlapAngle;
        double m_TEXHinge, m_TEYHinge, m_TEFlapAngle;


        QFrame *m_pOverlayFrame;
        FloatEdit	*m_pfeLEYHinge;
        FloatEdit	*m_pfeLEXHinge;
        FloatEdit	*m_pfeLEFlapAngle;
        FloatEdit	*m_pfeTEYHinge;
        FloatEdit	*m_pfeTEXHinge;
        FloatEdit	*m_pfeTEFlapAngle;

        QCheckBox *m_pchLEFlapCheck;
        QCheckBox *m_pchTEFlapCheck;

        QCheckBox *m_pchMakePermanent;
};

