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

#include <xfoil.h>
#include <interfaces/editors/foiledit/foildlg.h>


class IntEdit;
class FloatEdit;
class Foil;
class XFoil;

class XFoilPanelDlg : public FoilDlg
{
    Q_OBJECT

    public:
        XFoilPanelDlg(QWidget *pParent);

        XFoil m_XFoil;

        void initDialog(Foil* pFoil) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onApply();
        void onReset();

    private:
        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent(QResizeEvent *) override;

        void setupLayout();
        void readParams();

    private:
        QFrame *m_pfrXFoilPanel;

        IntEdit  *m_pieNPanels;
        FloatEdit *m_pfeCVpar,  *m_pfeCTErat, *m_pfeCTRrat;
        FloatEdit *m_pfeXsRef1, *m_pfeXsRef2, *m_pfeXpRef1, *m_pfeXpRef2;

        static int s_npan;
        static double s_cvpar;
        static double s_cterat;
        static double s_ctrrat;
        static double s_xsref1;
        static double s_xsref2;
        static double s_xpref1;
        static double s_xpref2;
};


