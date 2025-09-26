/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <xflobjects/exchange/cadexportdlg.h>

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


