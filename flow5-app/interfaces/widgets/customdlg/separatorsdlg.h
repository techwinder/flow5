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



