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
#include <QFrame>
#include <QDialogButtonBox>
#include <interfaces/editors/foiledit/foildlg.h>


class FloatEdit;

class FoilTEGapDlg : public FoilDlg
{
    Q_OBJECT

    public:
        FoilTEGapDlg(QWidget *pParent);

        void initDialog(Foil *pFoil) override;
        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent(QResizeEvent*) override;

        void setupLayout();

    private slots:
        void onApply() override;


    private:
        QFrame * m_pOverlayFrame;
        FloatEdit *m_pdeBlend, *m_pdeGap;

        static double s_BlendLength;
};

