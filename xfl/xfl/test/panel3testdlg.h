/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once
#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QSplitter>

#include <xflcore/linestyle.h>
#include <xflpanels/panels/testpanels.h>
#include <xflpanels/panels/panel3.h>
#include <xflpanels/panels/panel4.h>
#include <xflmath/mathelem.h>

class IntEdit;
class FloatEdit;
class PlainTextOutput;
class GraphWt;
class Graph;

class Vector3d;
class LineBtn;
class gl3dPanelField;


class Panel3TestDlg : public QDialog
{
    Q_OBJECT

    public:
        Panel3TestDlg(QWidget *pParent=nullptr);
        ~Panel3TestDlg();

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    public slots:
        void onMakeViews();
        void onCurveStyle(LineStyle ls);

    private:
        QFrame* makeControls();
        void connectSignals();
        void makeGraph();
        void makePanels();
        void readData();
        void setControls();
        void setupLayout();

//        Vector3d velocity(Vector3d const &pt);
//        double potential(Vector3d const &pt);

    private:

        QSplitter *m_pHSplitter;
        QCheckBox *m_pchSinglePanel;

        QRadioButton *m_prbP4, *m_prbP3;

        IntEdit *m_pieNXPanels, *m_pieNYPanels;
        QComboBox *m_pcbXDistrib, *m_pcbYDistrib;
        QRadioButton *m_prbSource, *m_prbDoublet;

        QComboBox *m_pcbVortexModel;
        FloatEdit *m_pdeCoreRadius;

        FloatEdit *m_pdeX0, *m_pdeY0, *m_pdeZ0;
        FloatEdit *m_pdeX1, *m_pdeY1, *m_pdeZ1;
        IntEdit *m_pieCurvePts;

        QRadioButton *m_prbVelocity, *m_prbPotential;

        QRadioButton *m_prbBasis, *m_prbVortex, *m_prbN4023;

        QComboBox *m_pcbVcomp;

        LineBtn *m_pLineBtn;

        PlainTextOutput *m_ppto;

        QVector<Panel3> m_Panel3;
        QVector<Panel4> m_Panel4;

        Graph *m_pGraph;
        GraphWt *m_pGraphWt;

        gl3dPanelField *m_pgl3dPanelView;

        static LineStyle s_LS;     /**< The curve's style */

        static bool s_bP3;
        static bool s_b1TriPanel;
        static int s_NXPanels;
        static int s_NYPanels;
        static xfl::enumDistribution s_iXDistrib, s_iYDistrib;
        static double s_Side;      /**< The panel's __side__ */
        static bool s_bSource;     /**< If true, the source influence will be plotted; if false, the doublet influence will be plotted */
        static bool s_bPotential;  /**< If true, the potential will be plotted; if false, the velocity will be plotted */
        static Vortex::enumVortex s_VortexModel;
        static double s_CoreRadius;
        static int s_VComp;

        static PANELMETHOD s_Method;

        static double s_X0, s_Y0, s_Z0;
        static double s_X1, s_Y1, s_Z1;
        static int s_nCurvePts;

        static QByteArray s_HSplitterSizes;
        static QByteArray s_Geometry;

};

