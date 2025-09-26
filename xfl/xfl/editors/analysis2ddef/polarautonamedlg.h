/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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

