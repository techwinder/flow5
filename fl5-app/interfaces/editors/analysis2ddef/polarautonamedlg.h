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
#include <QCheckBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QSettings>

class Polar;

class PolarAutoNameDlg : public QDialog
{
	Q_OBJECT

    public:
        PolarAutoNameDlg();
        ~PolarAutoNameDlg();

        void initDialog(Polar* pPolar);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();
        void readData();

    private slots:
        void onOptionChanged();


    private:
        Polar *m_pPolar;

        QCheckBox *m_pchType;
        QCheckBox *m_pchReynolds;
        QCheckBox *m_pchMach;
        QCheckBox *m_pchNCrit;
        QCheckBox *m_pchXTrTop;
        QCheckBox *m_pchXTrBot;

        QCheckBox *m_pchBLMethod;

        QLineEdit *m_plePolarName;

        QRadioButton *m_prbType1,*m_prbType2,*m_prbType3,*m_prbType4;

};

