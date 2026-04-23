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
#include <QThread>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QSettings>

class IntEdit;
class FloatEdit;
class Graph;
class GraphWt;

class Analysis3dSettings : public QDialog
{
    Q_OBJECT

    public:
        Analysis3dSettings(QWidget *pParent);
        ~Analysis3dSettings();

        void initDialog(int iPage=s_iPage);

        QSize sizeHint() const override {return QSize(900,550);}

        void keyPressEvent(QKeyEvent *pEvent) override;
        void reject() override;

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        static void setStabDerivatives(bool b) {s_bStabDerivatives=b;}
        static bool bStabDerivatives() {return s_bStabDerivatives;}

        static void setKeepOpenOnErrors(bool bKeepOpen) {s_bKeepOpenOnErrors=bKeepOpen;}
        static bool keepOpenOnErrors() {return s_bKeepOpenOnErrors;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private slots:
        void onResetDefaults();
        void onButton(QAbstractButton *pButton);
        void onMakeVortexGraph();

    private:
        void setupLayout();
        void readData();
        void setData();

    private:
        GraphWt *m_pVortexGraphWt;

        QTabWidget *m_pTabWt;

        QDialogButtonBox *m_pButtonBox;

        FloatEdit *m_pfeCoreRadius;
        QComboBox *m_pcbVortexModel;

        FloatEdit *m_pfeLLTRelax;
        FloatEdit *m_pfeLLTAlphaPrec;
        IntEdit *m_pieLLTNStation;
        IntEdit *m_pieLLTIterMax;

        QCheckBox *m_pchViscInitVTwist;
        FloatEdit *m_pfeViscPanelRelax;
        FloatEdit *m_pfeViscPanelTwistPrec;
        IntEdit *m_pieViscPanelIterMax;

        IntEdit *m_pieMaxRHS;

        QCheckBox *m_pchKeepOpenOnErrors;

        IntEdit *m_pieQuadPoints;

        FloatEdit *m_pfeMinPanelSize;
        FloatEdit *m_pfeRFF;
        FloatEdit *m_pfeVortexPos;
        FloatEdit *m_pfeControlPos;

        QRadioButton *m_prbSinglePrecision, *m_prbDoublePrecision;

        //Vortex particle wake
        QCheckBox *m_pchVortonRedist, *m_pchVortonStrengthEx;

        static bool s_bKeepOpenOnErrors;
        static bool s_bStabDerivatives;

        static int s_iPage;
        static QByteArray s_WindowGeometry;
};

