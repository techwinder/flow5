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
#include <QLineEdit>
#include <QSettings>

class Boat;
class BoatPolar;

class BtPolarAutoNameDlg : public QDialog
{
    Q_OBJECT

    public:
        BtPolarAutoNameDlg();
        ~BtPolarAutoNameDlg();

        void initDialog(const Boat &boat, BoatPolar &wPolar);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();
        void readData();

    private slots:
        void onOptionChanged();


    private:
        Boat *m_pBoat;
        BoatPolar *m_pBtPolar;

        QCheckBox *m_pchMethod;
        QCheckBox *m_pchViscosity;

        QCheckBox *m_pchControls;
        QCheckBox *m_pchExtraDrag;
        QCheckBox *m_pchGround;

        QLineEdit *m_plePolarName;

};


