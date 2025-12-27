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

#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QModelIndex>
#include <QComboBox>
#include <QStackedWidget>

#include <interfaces/graphs/graph/graph.h>
#include <interfaces/graphs/containers/splinedgraphwt.h>
#include <interfaces/widgets/customwts/floatedit.h>

class XPlane;
class FloatEdit;
class Curve;
class PlaneOpp;
class PlanePolar;

class CPTableView;
class ActionItemModel;

class StabTimeCtrls : public QFrame
{
    Q_OBJECT

    public:
        StabTimeCtrls(QWidget *pParent=nullptr);
        ~StabTimeCtrls()  override;

        QString selectedCurveName();

        void selectCurve(Curve const *pCurve);

        void setControls();
        void fillAVLcontrols(PlanePolar const*pWPolar);

        bool isStabLongitudinal() const {return m_prbLongitudinal->isChecked();}

        void fillTimeCurve(PlaneOpp const*pPOpp, Curve **pCurve);
        void setMode(int iMode=-1);

        double deltaT() const {return m_pdeDeltat->value();}
        double totalTime() const {return m_pdeTotalTime->value();}

        void loadSettings(QSettings &settings);
        void saveSettings(QSettings &settings);

        static void setXPlane(XPlane *pXPlane) {s_pXPlane = pXPlane;}

    private:
        QSize sizeHint() const  override {return QSize(50,50);}
        void showEvent(QShowEvent *pEvent)  override;
        void resizeEvent(QResizeEvent *pEvent)  override;
        void keyPressEvent(QKeyEvent *pEvent)  override;
        void makeTable();
        void setupLayout();
        void connectSignals();
        void appendRow(const Curve *pCurve);

        double getControlInput(const double &time) const;
        void fillCurvesForcedResponse(const PlaneOpp *pPOpp, Curve **pCurve);
        void fillCurvesPerturbation(const PlaneOpp *pPOpp, Curve **pCurve);
        void addCurve();
        void fillCurveList();
        Curve *selectedCurve();

    private slots:
        void onDataChanged(QModelIndex,QModelIndex);
        void onCurveTableClicked(QModelIndex index);
        void onModeSelection();
        void onPlotStabilityGraph();
        void onReadData();
        void onResponseType();
        void onAddCurve();
        void onDeleteCurve();
        void onRenameCurve();
        void onSelChangeCurve(int sel);
        void onStabilityDirection();
        void onResizeColumns();

    private:

        static XPlane *s_pXPlane;

        QRadioButton *m_prbTimeMode[4];

        QLabel *m_plabStab1, *m_plabStab2, *m_plabStab3;
        FloatEdit  *m_pdeStabVar1, *m_pdeStabVar2, *m_pdeStabVar3;
        FloatEdit *m_pdeTotalTime, *m_pdeDeltat;
        QPushButton *m_ppbAddCurve;

        CPTableView *m_pcpCurveTable;
        ActionItemModel *m_pCurveModel;

        QComboBox *m_pcbAVLControls;

        QLabel *m_plabUnit1, *m_plabUnit2, *m_plabUnit3;
        QStackedWidget *m_pswInitialConditions;

        QRadioButton *m_prbModalResponse, *m_prbInitCondResponse, *m_prbForcedResponse;

        QAction *m_pRenameAct, *m_pDeleteAct, *m_pMoveUp, *m_pMoveDown;


        QRadioButton *m_prbLongitudinal, *m_prbLateral;

        int m_iCurrentMode;

        Graph m_InputGraph;
        SplinedGraphWt *m_pSplGraphWt;

        // time curve data
        double m_TimeInput[4];
        double m_TotalTime;
        double m_Deltat;

        enum enumStabTimeResponse {MODALRESPONSE, INITIALCONDITIONS, FORCEDRESPONSE};
        enumStabTimeResponse m_ResponseType;

};

