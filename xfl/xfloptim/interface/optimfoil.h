/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QCheckBox>
#include <QRadioButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabWidget>
#include <QSettings>
#include <QStandardItemModel>
#include <QComboBox>

#include <xfloptim/tasks/particle.h>
#include <xfloptim/tasks/psotask.h>
#include <xfloptim/tasks/psotaskfoil.h>
#include <xflgraph/graph/graph.h>
#include <xflwidgets/customdlg/xfldialog.h>


class Foil;
class FoilWt;
class Polar;
class GraphWt;
class FloatEdit;
class IntEdit;
class XFoilTask;
struct OptObjective;

class PlainTextOutput;
class CPTableView;
class XflDelegate;
class ActionItemModel;

#define NOPT 4             // max. number of optimization points
#define NOBJECTIVES 5      // number of optimization targets


struct OptimizationPoint
{
    QString m_Name{"optimization point"}; // debug only
    double m_Alpha  = 0.0;
    double m_Re     = 1.0e6;
    double m_Mach   = 0.0;
    double m_NCrit  = 9.0;
    double m_XtrTop = 1.0;
    double m_XtrBot = 1.0;
};


class OptimFoil : public XflDialog
{
    Q_OBJECT

    public:
        OptimFoil(QWidget *pParent);
        ~OptimFoil();

        void setFoil(Foil *pFoil);

        bool isModified() const {return m_bModified;}

        QSize sizeHint() const override {return QSize(1300, 900);}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;

        void setupLayout();
        void connectSignals();

        void cancelTask();
        void enableControls(bool bEnable);
        void evolution();
        void fillObjectives();
        void listParticle(Particle const &particle, QString &log, QString prefix=QString());
        void makePSOSwarm();
        void makeRandomParticle(Particle *pParticle) const;
        int nActiveObjectives() const;
        void outputText(QString const &msg);
        void readData();
        void readObjectives();
        void runXFoil(const Foil *pFoil, Polar *pPolar, double alpha, double &Cl, double &Cd, double &ClCd, double &minCp , double &Cm, QVector<double> &Cpv, bool &bConverged);
        void setPSOObjectives(PSOTaskFoil *pPSOTask);
        void swarm();
        void updateCpGraphLayout();
        void updateCpGraphs(Particle const &particle);
        void updatePolars();
        void updateTaskParameters();
        void updateVariables(PSOTaskFoil *pPSOTask2d);

        void fillOptPoints();
        void readOptPoints();

    private slots:
        Foil *onStoreBestFoil();
        void onCellChanged(QModelIndex,QModelIndex);
        void onClose();
        void onContinueBest();
        void onFillOptPoint();
        void onMakeSwarm(bool bShow=true);
        void onNOpt(int NOpp);
        void onObjTableClicked(QModelIndex index);
        void onPlotHH();
        void onResizeColumns();
        void onRunOptimizer();
        void onRunXFoil();
        void onVarType();
        void reject() override;

        void onIterEvent(OptimEvent *result);

    private:
        bool m_bSaved;
        bool m_bModified;

        Foil *m_pFoil; // not const, potential need to adjust the TE hinge location
        Foil *m_pBestFoil; // used to animate the display
        Polar* m_pPolar[NOPT];

        QVector<Foil*> m_TempFoils; /**< pointers to debug foils to delete on exit */

        int m_iLE;  /**< the index of the leading edge point for thee current aoa */


        bool m_bIsSwarmValid;

        PSOTaskFoil *m_pPSOTaskFoil;

        //XFoil

        static OptimizationPoint s_Opt[NOPT];


        //Flap
        static double s_FlapAngleMin, s_FlapAngleMax;
        static double s_XHinge,  s_YHinge;

        //objectives
        static int s_NOpt;
        static OptObjective s_Objective[NOPT][NOBJECTIVES];


        //Geometry modification
        static PSOTaskFoil::enumMod s_ModType;

        //Hicks-Henne
        static double s_HHt1;     /**< t1 parameter of the HH functions */
        static double s_HHt2;     /**< t2 parameter of the HH functions */
        static int    s_HHn;      /**< number of HH functions to use */
        static double s_HHmax;    /**< the max amplitude of the HH functions */

        //Scale
        static double s_Thick[2];
        static double s_Camb[2];
        static double s_XThick[2];
        static double s_XCamb[2];

        QLabel *m_plabFoilInfo;
        FloatEdit *m_pfeThick[2];
        FloatEdit *m_pfeCamb[2];
        FloatEdit *m_pfeXThick[2];
        FloatEdit *m_pfeXCamb[2];


        // output
        QFrame *m_pfrCpGraph;
        Graph m_CpGraph[NOPT];
        GraphWt *m_pCpGraphWt[NOPT];

        FoilWt *m_pFoilWt;


        // Variables
        QRadioButton *m_prbHH, *m_prbScale;
        QStackedWidget *m_pswVarWidget;

        //T.E. flap
        FloatEdit *m_pdeTEYHinge, *m_pdeTEXHinge;
        FloatEdit *m_pdeFlapAngleMin, *m_pdeFlapAngleMax;


        // Hicks-Henne
        IntEdit *m_pieNHH;
        FloatEdit *m_pdeHHt1, *m_pdeHHt2, *m_pdeHHmax;
        Graph m_HHGraph;
        GraphWt *m_pHHGraphWt;

        //Objectives
        QComboBox *m_pcbNOpt;

        CPTableView *m_pcptOptPoints;
        QStandardItemModel *m_pOptPointsModel;
        XflDelegate *m_pOptPointsDelegate;

        CPTableView *m_pcptObjective;
        QStandardItemModel *m_pObjModel;
        XflDelegate *m_pObjDelegate;

        //PSO
        FloatEdit *m_pdeInertiaWeight;
        FloatEdit *m_pdeCognitiveWeight;
        FloatEdit *m_pdeSocialWeight;
        FloatEdit *m_pdeProbaRegen;

        IntEdit *m_piePopSize;
        IntEdit *m_pieMaxIter;
        QCheckBox *m_pchMultithread;

        QPushButton *m_ppbRunXFoil;
        QPushButton *m_ppbMakeSwarm, *m_ppbSwarm, *m_ppbStoreBestFoil, *m_ppbContinueBest;

        QSplitter *m_pLeftSplitter;
        QSplitter *m_pHSplitter, *m_pVSplitter;
        PlainTextOutput *m_ppto;


        static QByteArray s_Geometry;
        static QByteArray s_LeftSplitterSizes, s_HSplitterSizes, s_VSplitterSizes;
};


