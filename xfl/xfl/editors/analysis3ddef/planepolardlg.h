/****************************************************************************

    flow5
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

#include <QSettings>
#include <QTreeView>

#include <xflcore/enums_objects.h>
#include <xfl/editors/analysis3ddef/polar3ddlg.h>


class CtrlTableDelegate;

class PlanePolarDlg : public Polar3dDlg
{
    Q_OBJECT
    public:
        PlanePolarDlg(QWidget *pParent);

        static WPolar & staticWPolar() {return s_WPolar;}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    protected:
        virtual bool checkWPolarData();
        void enableControls() override;
        virtual void initPolar3dDlg(const Plane *pPlane, const WPolar *pWPolar=nullptr);
        void readData() override;
        void readFuseDragData();
        void readFluidProperties() override;
        void readReferenceDimensions() override;
        void readMethodData() override;
        void readViscousData();
        void checkMethods() override;
        void makeCommonControls();
        void connectSignals();
        virtual void fillInertiaPage();
        void readInertiaData();
        void fillFlapControls();
        void setPolar3dName() override;


    protected slots:
        void onReset() override;
        void onArea();
        void onAeroData() override;
        virtual void onPlaneInertia();
        void onWingSurfaces();
        void onNameOptions() override;
        void onViscous();
        void onGroundEffect();
        void onFuseDrag();
        void onVortonWake() override;
        void onFlapControls();


    protected:

        Plane const *m_pPlane;

        QFrame *m_pfrFlaps;
        QTreeView *m_pFlapTreeView;
        QStandardItemModel *m_pFlapModel;
        CtrlTableDelegate *m_pFlapDelegate;
        QLineEdit *m_pleFlapSetName;

        QFrame *m_pfrGround;
        QCheckBox *m_pchGround, *m_pchFreeSurf;
        FloatEdit *m_pfeHeight;

        QFrame *m_pfrInertia;
        QCheckBox *m_pchAutoInertia;
        FloatEdit *m_pfePlaneMass;
        FloatEdit *m_pdeXCoG, *m_pdeZCoG;
        FloatEdit *m_pdeIxx, *m_pdeIyy, *m_pdeIzz, *m_pdeIxz;


        QFrame *m_pfrFuseDrag;
        QCheckBox *m_pchFuseDrag;
        QRadioButton *m_prbPSDrag, *m_prbKSDrag, *m_prbCustomFuseDrag;
        QLabel *m_plabFuseFormFactor, *m_plabFuseWettedArea, *m_plabFuseDragFormula;
        FloatEdit *m_pdeCustomFF;

        QFrame *m_pfrViscosity;
        QCheckBox *m_pchViscAnalysis;
        QRadioButton *m_prbViscInterpolated, * m_prbViscOnTheFly;
        QRadioButton *m_prbViscFromCl, *m_prbViscFromAlpha;
        FloatEdit *m_pfeNCrit;
        FloatEdit *m_pfeXTopTr, *m_pfeXBotTr;
        QFrame *m_pfrInterpolated, *m_pfrOntheFly;
        QCheckBox *m_pchViscousLoop;

        static WPolar s_WPolar;
};

