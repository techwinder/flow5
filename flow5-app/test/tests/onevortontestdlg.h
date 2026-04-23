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
#include <QSplitter>
#include <QComboBox>
#include <QCheckBox>

#include <interfaces/graphs/containers/graphwt.h>
#include <api/vorton.h>
#include <api/quaternion.h>


class IntEdit;
class FloatEdit;
class gl3dVortonField;

class OneVortonTestDlg : public QDialog
{
    Q_OBJECT
    public:
        OneVortonTestDlg();
        ~OneVortonTestDlg();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(900,700);}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    public slots:
        void onRecalc();

    private:
        void setupLayout();
        void readData();

    private:

        QSplitter *m_pHSplitter;
        QComboBox *m_pcbVPlotDir;
        QComboBox *m_pcbGradDir;
        QCheckBox *m_pchbGradV;
        QCheckBox *m_pchbDiff;

        std::vector<Vorton> m_Vorton;

        FloatEdit *m_pdeVtnCoreSize;

        FloatEdit *m_pdeX0, *m_pdeY0, *m_pdeZ0;
        FloatEdit *m_pdeX1, *m_pdeY1, *m_pdeZ1;

        IntEdit *m_pieCurvePts;

        GraphWt *m_pGraphWt;
        gl3dVortonField *m_pglVortonField;

        static int s_nCurvePts;
        static bool s_bDiff;
        static bool s_bGradV;
        static int s_iVPlotDir;
        static int s_iGradDir;
        static double s_X0, s_Y0, s_Z0;
        static double s_X1, s_Y1, s_Z1;
        static QByteArray s_HSplitterSizes;
        static QByteArray s_Geometry;
        static Quaternion s_ab_quat;

};

