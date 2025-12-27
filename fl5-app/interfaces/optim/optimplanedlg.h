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

#include <QLabel>
#include <QCheckBox>
#include <QElapsedTimer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QSplitter>
#include <QTabWidget>
#include <QStandardItemModel>
#include <QListWidget>
#include <QTreeView>
#include <QPlainTextEdit>

#include <api/optstructures.h>
#include <interfaces/optim/particle.h>
#include <interfaces/graphs/graph/graph.h>
#include <api/planexfl.h>

class PSOTaskPlane;

class PlainTextOutput;
class GraphWt;
class FloatEdit;
class IntEdit;
class CPTableView;
class XflDelegate;
class ActionItemModel;
class EditObjectDelegate;
class XflTreeView;
class gl3dParetoView;

#define NOBJECTIVES 7

class OptimEvent;

class OptimPlaneDlg : public QDialog
{
    Q_OBJECT
    public:
        OptimPlaneDlg(QWidget *pParent);
        ~OptimPlaneDlg();

        void initDialog(const PlaneXfl *pPlaneXfl);

        void setObjectives(QVector<OptObjective> const &objectives);
        QVector<OptObjective> const &objectives() const {return m_Objective;}

        bool bChanged() const {return m_bChanged;}
        PlaneXfl *bestPlane() {return m_pBestPlane;}

        QSize sizeHint() const override {return QSize(1300, 900);}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:

        void customEvent(QEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

        QString variableName(int ivar) const;
        bool readObjectives();
        bool runPlaneAnalysis(PlaneXfl *pPlaneXfl, PlanePolar *pWPolar, const std::vector<double> &opplist);
        int nActiveObjectives() const;
        void clearResults();
        void connectSignals();
        void enableControls(bool bEnable);
        void fillObjectives();
        void fillPlaneList();
        void fillResults();
        void fillVariables();
        void getUnit(int ivar, double &factor, QString &labunit) const;
        void listParticle(Particle const &particle, QString &log, const QString &prefix) const;
        void listObjectives(QString &list) const;
        void makeCommonWt();
        void makeOptVariables();
        void readData();
        void readVariables(int &nActive, QString &log, const QString &prefix);
        void resetOutput();
        void setTaskObjectives(PSOTaskPlane *pPSOTask);
        void setupLayout();
        void showObjectiveGraphs();
        void updateParetoViews(int iSelect);

    protected slots:

        PlaneXfl* onStoreBest();
        bool onMakeSwarm();
        void invalidatePareto() {m_bResetPareto=true;}
        void invalidateSwarm()  {m_bResetSwarm=m_bResetPareto=true;}
        void on2dDemo();
        void onActionResultClicked(QModelIndex index);
        void onAnalysisDef();
        void onButton(QAbstractButton *pButton);
        void onClose();
        void onContinueBest();
        void onIterEvent(OptimEvent*pEvent);
        void onObjTableClicked(QModelIndex index);
        void onObjectiveChanged();
        void onOutputMessage(QString const &msg);
        void onPlaneSelected(QListWidgetItem *pItem);
        void onResetOptVariables();
        void onResetParetoFrontier();
        void onResizeColumns();
        void onRestorePSODefaults();
        void onRunAnalysis();
        void onSortColumn(int col, Qt::SortOrder order);
        void onSwarm();
        void onVariableChanged(QModelIndex,QModelIndex);
        void reject() override;

    protected:
        bool m_bChanged;
        bool m_bSaved;
        bool m_bResetSwarm;    /**< true if the swarm needs to be recreated due to a variable or objective change for instance */
        bool m_bResetPareto;   /**< true if the value of the objectives has changed and the Pareto frontier needs to be recreated */

        int m_IterTotal;       /**< the cumulated number of iterations excluding those of the current run */
        int m_ParticleCounter; /**< A counter of the number of particles created so far */


        QVector<OptObjective> m_Objective;
        std::vector<OptVariable>  m_OptVariable;

        PSOTaskPlane *m_pPSOTask;

        PlaneXfl const *m_pPlaneXfl;
        PlaneXfl *m_pBestPlane;

        Particle m_BestParticle;


        QTabWidget *m_ptwControls;
        QPushButton *m_ppbMenuBtn;

        // Pareto
        Graph m_ParetoGraph;
        GraphWt *m_pParetoGraphWt;
        gl3dParetoView *m_pgl3dPareto;

        // Controls
        QPushButton * m_ppbSwarm, *m_ppbMakeSwarm, *m_ppbStoreBest, *m_ppbContinueBest;
        PlainTextOutput *m_ppto;
        QDialogButtonBox *m_pButtonBox;
        QLabel *m_pFlow5Link;

        //PSO
        QFrame *m_pPSOFrame;
        IntEdit *m_pieSwarmSize, *m_pieArchiveSize;
        IntEdit *m_pieMaxIter;
        FloatEdit *m_pdeInertiaWeight;
        FloatEdit *m_pdeCognitiveWeight;
        FloatEdit *m_pdeSocialWeight;
        FloatEdit *m_pdePropRegenerate;
        QPushButton *m_ppbRestoreDefault;
        QCheckBox *m_pchMultiThread;


        //Results
        CPTableView *m_pcptResults;
        ActionItemModel *m_pResultsModel;
        XflDelegate *m_pResultsDelegate;


        QElapsedTimer m_Clock;


        QListWidget *m_plwPlanes;

        QTabWidget *m_ptwPareto;

        //Objectives
        CPTableView *m_pcptObjective;
        QStandardItemModel *m_pObjModel;
        XflDelegate *m_pObjDelegate;


        //Variables
        QFrame *m_pVarFrame;
        XflTreeView *m_ptvPlane;
        QStandardItemModel *m_pVariableModel;
        EditObjectDelegate *m_pVariableDelegate;
        QPushButton *m_ppbResetVariables;

        //Analysis
        QFrame *m_pAnalysisFrame;
        QPlainTextEdit *m_pptePolar;
        QPushButton *m_ppbAnalysisDef, *m_ppbRunAnalysis;

        //Variables graph
        Graph m_VariableGraph;
        GraphWt *m_pVariableGraphWt;


        // objectives graphs
        Graph m_ObjGraph[NOBJECTIVES];
        GraphWt *m_pObjGraphWt[NOBJECTIVES];


        QSplitter *m_pHSplitter, *m_pLeftVSplitter, *m_pRightVSplitter;

        static QByteArray s_HSplitterSizes;
        static QByteArray s_LeftVSplitterSizes;
        static QByteArray s_RightVSplitterSizes;

        static double s_AlphaMin;
        static double s_AlphaMax;

        static int s_iActiveTab;
        static QByteArray s_Geometry;

};




