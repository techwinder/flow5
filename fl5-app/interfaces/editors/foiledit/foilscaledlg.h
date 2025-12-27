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
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QDialogButtonBox>

#include <interfaces/editors/foiledit/foildlg.h>


class FloatEdit;
class Foil;

class FoilWt;

class FoilScaleDlg : public FoilDlg
{
    Q_OBJECT

    public:
        FoilScaleDlg(QWidget *pParent);

        void initDialog(Foil *pFoil) override;


    private slots:
        void onReset() override;
        void onApply() override;
        void onCamberSlide(int pos);
        void onXCamberSlide(int pos);
        void onThickSlide(int pos);
        void onXThickSlide(int pos);
        void onCamber();
        void onXCamber();
        void onThickness();
        void onXThickness();

    private:
        void resizeEvent(QResizeEvent *) override;
        void showEvent(QShowEvent *pEvent) override;
        void setupLayout();


    private:
        QSlider	*m_pslCamberSlide, *m_pslThickSlide, *m_pslXThickSlide, *m_pslXCamberSlide;
        FloatEdit *m_pfeXCamber, *m_pfeXThickness, *m_pfeThickness, *m_pfeCamber;


    private:

        QFrame *m_pOverlayFrame;
//        double m_fCamber;
//        double m_fThickness;
//        double m_fXCamber;
//        double m_fXThickness;
};



