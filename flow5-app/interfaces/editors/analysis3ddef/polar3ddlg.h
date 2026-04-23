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
#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QStandardItemModel>
#include <QLineEdit>

class Plane;
class Polar3d;
class PlanePolar;
class IntEdit;
class FloatEdit;
class ExtraDragWt;

class Polar3dDlg : public QDialog
{
    Q_OBJECT

    public:
        Polar3dDlg(QWidget *pParent);
        virtual ~Polar3dDlg();


    protected:
        void connectBaseSignals();
        virtual void enableControls() = 0;

        virtual void checkMethods() = 0;
        virtual void setPolar3dName()=0;

        virtual void keyPressEvent(QKeyEvent *pEvent) override;
        virtual void showEvent(QShowEvent *) override;
        virtual void hideEvent(QHideEvent *) override;
        virtual void resizeEvent(QResizeEvent *pEvent) override;


    protected slots:
        virtual void onOK() = 0;
        virtual void onReset() = 0;
        virtual void onEditingFinished() = 0;
        virtual void onNameOptions() = 0;
        virtual void onMethod();
        virtual void onTabChanged(int iTab);
        virtual void onVortonWake() = 0;
        virtual void onAeroData() = 0;
        void onPolar3dName();
        void onAutoName();

        void onButton(QAbstractButton *pButton);

    protected:

        virtual void resizeColumns() {}
        virtual void readData() = 0;

        void disableVortonWake(Polar3d &polar3d);
        void makeBaseCommonControls();

        virtual void readMethodData() = 0;
        virtual void readFluidProperties() = 0;
        virtual void readReferenceDimensions() = 0;

        virtual void readWakeData(Polar3d &polar3d) const;
        virtual void setVPWUnits(Polar3d &polar3d);

    protected:

        QLabel *m_plabParentObjectName;

        bool m_bAutoName;

        QRadioButton *m_prbThinSurfaces, *m_prbThickSurfaces;

        QCheckBox *m_pchIncludeFuseMi;
        QCheckBox *m_pchIncludeWingTipMi;

        FloatEdit *m_pfeDensity;
        FloatEdit *m_pfeViscosity;

        QRadioButton *m_prbLLTMethod;
        QRadioButton *m_prbQuadMethod;
        QRadioButton *m_prbTriUniMethod, *m_prbTriLinMethod;
        QRadioButton *m_prbVLM1Method, *m_prbVLM2Method;

        QFrame *m_pfrMethod;

        QPushButton *m_ppbNameOptions;
        QCheckBox *m_pchAutoName;

        QRadioButton *m_prbArea1, *m_prbArea2, *m_prbArea3;
        FloatEdit *m_pfeRefChord, *m_pfeRefArea, *m_pfeRefSpan;

        QCheckBox *m_pchOtherWings;


        QLineEdit *m_plePolarName;

        QPushButton	*m_ppbFromData;

        QDialogButtonBox *m_pButtonBox;

        ExtraDragWt *m_pExtraDragWt;

        QGroupBox *m_pgbHullBox;
        QCheckBox *m_pchIncludeHull;

        QGroupBox *m_pgbWingSurf, *m_pgbFuseMi;
        QGroupBox *m_pgbVortonWake;

        QRadioButton*m_prbPanelWake;
        QRadioButton*m_prbVortonWake;
        FloatEdit *m_pfeVPWBufferWake;
        FloatEdit *m_pfeVortonL0;
        FloatEdit *m_pfeVortonCoreSize, *m_pfeVPWLength;
        IntEdit *m_pieVPWIterations;
        QLabel *m_plabVPWBufferWakeUnit, *m_plabVPWStepUnit,*m_plabVtnCoreUnit, *m_plabVPWDiscard, *m_plabVPWMaxLength;

        QGroupBox *m_pgbFlatWakePanels;
        FloatEdit *m_pfeWakeLength, *m_pfeWakePanelFactor;
        IntEdit *m_pieNXWakePanels;
        QLabel *m_plabWakeLengthLabUnit;

        QFrame *m_pfrRefDims;

        QFrame *m_pfrFluid;

        QFrame *m_pfrWake;
        QFrame *m_pfrPolarName;

        static QByteArray s_Geometry;
};

