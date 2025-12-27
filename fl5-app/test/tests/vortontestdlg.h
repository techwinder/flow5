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
#include <QSettings>
#include <QSplitter>
#include <QComboBox>
#include <QRadioButton>

#include <api/panel3.h>
#include <api/vorton.h>

class PlaneOpp;

class gl3dVortonField;
class Graph;
class GraphWt;

class FloatEdit;
class IntEdit;

class VortonTestDlg : public QDialog
{
    Q_OBJECT

    public:
        VortonTestDlg(const PlaneOpp *pPOpp);
        ~VortonTestDlg();

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        QFrame *makeControls();
        void connectSignals();
        void readData();

        void makeGraph(const Vector3d &P0, const Vector3d &P1, int nPts);
        void makePanels(const Node *nd);
        void setupLayout();

    private slots:
        void onMakePanels();
        void onRecalc();

    private:

        QSplitter *m_pHSplitter;

        std::vector<Panel3> m_Panels;
        std::vector<Vorton> m_Vortons;

        GraphWt *m_pGraphWt;
        gl3dVortonField *m_pglVortonField;

        FloatEdit *m_pdeWidth, *m_pdeLength;
        IntEdit *m_pieNXPanels, *m_pieNYPanels;
        IntEdit *m_pieVtnSide;
        FloatEdit *m_pdeVtnCoreSize;

        IntEdit *m_pieCurvePts;
        QComboBox *m_pcbVPlotDir;
        FloatEdit *m_pdeX0, *m_pdeY0, *m_pdeZ0;
        FloatEdit *m_pdeX1, *m_pdeY1, *m_pdeZ1;

        QRadioButton *m_prbV, *m_prbGradV;

        QRadioButton *m_prbVelSrc[2];
        QComboBox *m_pcbGradDir;

        static int s_nXPanels, s_nYPanels;
        static double s_Width;
        static double s_Length;


        static bool s_bGradV;
        static int s_iGradDir;
        static double s_X0, s_Y0, s_Z0;
        static double s_X1, s_Y1, s_Z1;
        static int s_nCurvePts;
        static int s_iVPlotDir;
        static int s_nVtnSide; // number of vortons/side
        static int s_iVelSrc; // from vortices, panels or vortons
        static QByteArray s_HSplitterSizes;
        static QByteArray s_Geometry;
        static Quaternion s_ab_quat;
};

