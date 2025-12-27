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

#include <QFrame>

#include <interfaces/editors/foiledit/foildlg.h>


class FloatEdit;

class FoilLEDlg : public FoilDlg
{
    Q_OBJECT

    public:
        FoilLEDlg(QWidget *pParent);

        void initDialog(Foil *pFoil) override;

    private slots:
        void onApply() override;
        void onLERadiusDisplay();

    private:

        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent(QResizeEvent *) override;

    private:
        QFrame *m_pOverlayFrame;

        FloatEdit *m_pfeDisplayRadius;
        FloatEdit *m_pfeBlend, *m_pfeLEfactor;

        static double s_LErfac;
        static double s_BlendLength;
        static double s_LERadius;
};

