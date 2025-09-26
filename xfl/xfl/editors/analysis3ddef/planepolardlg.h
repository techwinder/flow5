/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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

