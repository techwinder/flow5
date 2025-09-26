/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

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
        void initDialog(int iPage=s_iPage);

        QSize sizeHint() const override {return QSize(900,550);}

        void keyPressEvent(QKeyEvent *pEvent) override;
        void reject() override;

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

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

        FloatEdit *m_pdeCoreRadius;
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

        FloatEdit *m_pdeMinPanelSize;
        FloatEdit *m_pdeRFF;
        FloatEdit *m_pdeVortexPos;
        FloatEdit *m_pdeControlPos;

        QRadioButton *m_prbSinglePrecision, *m_prbDoublePrecision;

        //Vortex particle wake
        QCheckBox *m_pchVortonRedist, *m_pchVortonStrengthEx;

        static bool s_bKeepOpenOnErrors;

        static int s_iPage;
        static QByteArray s_WindowGeometry;
};

