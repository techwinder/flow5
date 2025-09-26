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

class WPolar;
class PlaneXfl;

class WPolarAutoNameDlg : public QDialog
{
        Q_OBJECT

    public:
        WPolarAutoNameDlg();
        ~WPolarAutoNameDlg();

        void initDialog(const WPolar &wPolar);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void connectSignals();
        void readData();

    private slots:
        void onOptionChanged();


    private:
        PlaneXfl *m_pPlane;
        WPolar *m_pWPolar;

        QCheckBox *m_pchType;
        QCheckBox *m_pchMethod;
        QCheckBox *m_pchSurfaces;
        QCheckBox *m_pchViscosity;
        QCheckBox *m_pchInertia;

        QCheckBox *m_pchControls;
        QCheckBox *m_pchExtraDrag;
        QCheckBox *m_pchFuseDrag;
        QCheckBox *m_pchGround;

        QLineEdit *m_plePolarName;

        QRadioButton *m_prbType1,*m_prbType2,*m_prbType6, *m_prbType7;

};

