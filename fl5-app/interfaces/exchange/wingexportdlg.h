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

#include <interfaces/exchange/cadexportdlg.h>

class WingXfl;
class IntEdit;
class FloatEdit;

class WingExportDlg : public CADExportDlg
{
    Q_OBJECT
    public:
        WingExportDlg(QWidget*pParent);
        void init(const WingXfl *pWing);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();
        void hideEvent(QHideEvent*pEvent) override;

        void exportShapes() override;

        void readParams();

    private slots:
        void onExportType();

    private:
        QRadioButton *m_prbFacets, *m_prbNURBS, *m_prbSwept;


        IntEdit *m_pieChordRes;

        IntEdit *m_pieSplineDegre;
        IntEdit *m_pieSplineCtrlPts;
        FloatEdit *m_pdeStitchPrecision;

        WingXfl const* m_pWing;

        static int s_iChordRes;

        static double s_StitchPrecision;
        static int s_SplineDegree;
        static int s_nSplineCtrlPts;

        static int s_SurfaceType;
};


