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

#include <QComboBox>
#include <QRadioButton>

#include <interfaces/graphs/containers/graphwt.h>
#include <api/linestyle.h>

class IntEdit;
class FloatEdit;
class PlainTextOutput;

class Boat;
class BoatOpp;
class LineBtn;
class Opp3d;
class P3Analysis;
class P4Analysis;
class Plane;
class PlaneOpp;
class Polar3d;
class Vector3d;
class gl3dXflView;

class PanelAnalysisTest : public GraphWt
{
    Q_OBJECT

    public:
        PanelAnalysisTest();
        ~PanelAnalysisTest();

        void setAnalysis(const Plane *pPlane, Polar3d const *pWPolar,  PlaneOpp const*pPOpp);
        void setAnalysis(const Boat *pPlane,  Polar3d const *pBtPolar, BoatOpp  const*pBtOpp);
        void set3dView(gl3dXflView *p3dView);

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    public slots:
        void onPickedIndex(int idx);
        void onMakeGraph();
        void onCurveStyle(LineStyle ls);

    private:
        QFrame* makeControls();
        void readData();
        void getVelocityVector(Vector3d const &C, double const *Mu, double const *Sigma, double coreradius, Vector3d &velocity) const;
        double getPotential(Vector3d const &C, double const *Mu, double const *Sigma) const;

    private:
        IntEdit *m_piePanelId;
        FloatEdit *m_pdeZMax;
        FloatEdit *m_pdeZInc;
        PlainTextOutput *m_pptoOutput;

        gl3dXflView *m_pgl3dXflView;

        QRadioButton *m_prbVelocity, *m_prbPotential;

        QComboBox *m_pcbVcomp;

        LineBtn *m_pLineBtn;

        P4Analysis *m_pP4Analysis;
        P3Analysis *m_pP3Analysis;

        Polar3d const *m_pPolar3d;
        Opp3d const *m_pOpp3d;

        static LineStyle s_LS;
        static bool s_bPotential;
        static int s_PanelId;
        static int s_VComp;
        static double s_ZInc;
        static double s_ZMax;
        static QByteArray s_Geometry;
};

