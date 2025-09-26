/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QSettings>

#include <xflcore/enums_objects.h>

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



