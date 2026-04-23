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
#include <QSettings>

#include <api/enums_objects.h>

class WingSelDlg : public QDialog
{
    Q_OBJECT

    public:
        WingSelDlg(QWidget *pParent=nullptr);

        static void showMainWingCurve(bool bShow)  {s_bMainWing=bShow;}
        static void showElevatorCurve(bool bShow)  {s_bElevator=bShow;}
        static void showFinCurve(bool bShow)       {s_bFin=bShow;}
        static void showOtherWingCurve(bool bShow) {s_bOtherWing=bShow;}

        static bool mainWingCurve()  {return s_bMainWing;}
        static bool elevatorCurve()  {return s_bElevator;}
        static bool finCurve()       {return s_bFin;}
        static bool otherWingCurve() {return s_bOtherWing;}


        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    protected:
        void accept() override;

    private:
        void setupLayout();

    private slots:

    private:
        QVector<QCheckBox*> m_pchWing;

        static bool s_bMainWing;
        static bool s_bOtherWing;
        static bool s_bElevator;
        static bool s_bFin;
};



