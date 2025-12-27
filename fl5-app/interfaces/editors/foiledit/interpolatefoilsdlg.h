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
#include <QDialogButtonBox>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QSlider>

#include <interfaces/editors/foiledit/foildlg.h>


#include <api/linestyle.h>

class Foil;
class FloatEdit;
class LineBtn;
class FoilWt;

class InterpolateFoilsDlg : public FoilDlg
{
    Q_OBJECT

    public:
        InterpolateFoilsDlg(QWidget *pParent);
        ~InterpolateFoilsDlg();

        void initDialogFoils(); // new name to avoid compiler warning for hidden method

        Foil const *interpolatedFoil() const {return m_pBufferFoil;}

    private:
        void showEvent(QShowEvent *pEvent) override;
        void showSelectedFoils();
        void setupLayout();
        void setFoil1();
        void setFoil2();


    private slots:
        void onSelChangeFoil1(int);
        void onSelChangeFoil2(int);
        void onFrac();

        void onSlider(int val);
        void onApply() override;


    private:
        QComboBox *m_pcbFoil1, *m_pcbFoil2;
        QLabel *m_plabProps1, *m_plabProps2, *m_plabProps3;
        QSlider *m_pslMix;
        FloatEdit *m_pdeFrac;

    private:
        Foil *m_pFoil1, *m_pFoil2;
        double m_Frac;
};










