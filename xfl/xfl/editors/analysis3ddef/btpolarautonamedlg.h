/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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


