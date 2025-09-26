/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QSettings>

class PlainTextOutput;


class SeparatorsDlg : public QDialog
{
    Q_OBJECT
    public:
        SeparatorsDlg(QWidget *pParent=nullptr);

        void hideEvent(QHideEvent *pEvent) override;

        static bool bWhiteSpace() {return s_bWhiteSpace;}
        static bool bTab()        {return s_bTab;}
        static bool bComma()      {return s_bComma;}
        static bool bSemiColon()  {return s_bSemiColon;}

        static void setWhiteSpace(bool b) {s_bWhiteSpace=b;}
        static void setTab(bool b)        {s_bTab=b;}
        static void setComma(bool b)      {s_bComma=b;}
        static void setSemiColon(bool b)  {s_bSemiColon=b;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();

    private:
        QDialogButtonBox *m_pButtonBox;

        QCheckBox *m_pchWhiteSpace;
        QCheckBox *m_pchTab, *m_pchComma, *m_pchSemiColon;

        static bool s_bWhiteSpace;
        static bool s_bTab;
        static bool s_bComma;
        static bool s_bSemiColon;
};



