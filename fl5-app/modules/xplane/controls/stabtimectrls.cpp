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


#define _MATH_DEFINES_DEFINED


#include "stabtimectrls.h"

#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTimer>
#include <complex>


#include <core/displayoptions.h>
#include <modules/xplane/glview/gl3dxplaneview.h>
#include <modules/xplane/xplane.h>
#include <api/units.h>
#include <core/xflcore.h>
#include <api/utils.h>
#include <api/geom_global.h>
#include <interfaces/graphs/controls/graphoptions.h>
#include <interfaces/graphs/graph/curve.h>
#include <api/constants.h>
#include <api/planeopp.h>
#include <api/planepolar.h>
#include <api/planexfl.h>
#include <interfaces/widgets/customdlg/newnamedlg.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>


XPlane *StabTimeCtrls::s_pXPlane=nullptr;


StabTimeCtrls::StabTimeCtrls(QWidget *pParent) : QFrame(pParent)
{
    setWindowTitle("Stability time controls");

    m_InputGraph.setName("StabTime graph");
    m_InputGraph.setCurveModel(new CurveModel);
    m_InputGraph.setXVariableList({"s"});
    m_InputGraph.setYVariableList({"amp."});

    m_ResponseType = MODALRESPONSE;

    m_iCurrentMode = 0;
    m_TimeInput[0] = m_TimeInput[1] = m_TimeInput[2] = m_TimeInput[3] = 0.0;
    m_TotalTime = 1;//s
    m_Deltat    = 0.001;//s

    m_pRenameAct = new QAction("Rename", this);
    m_pDeleteAct = new QAction("Delete", this);

    makeTable();
    setupLayout();

    m_prbLongitudinal->setChecked(true);

    connectSignals();


}


StabTimeCtrls::~StabTimeCtrls()
{
    m_InputGraph.deleteCurveModel();
    if(m_pCurveModel) delete m_pCurveModel;
}


void StabTimeCtrls::connectSignals()
{
    connect(m_prbLongitudinal, SIGNAL(clicked(bool)), SLOT(onStabilityDirection()));
    connect(m_prbLateral,      SIGNAL(clicked(bool)), SLOT(onStabilityDirection()));

    for(int imode=0; imode<4; imode++)
        connect(m_prbTimeMode[imode], SIGNAL(clicked()), SLOT(onModeSelection()));

    connect(m_pdeDeltat,    SIGNAL(floatChanged(float)), SLOT(onReadData()));

    connect(m_prbInitCondResponse, SIGNAL(clicked()), SLOT(onResponseType()));
    connect(m_prbForcedResponse,   SIGNAL(clicked()), SLOT(onResponseType()));
    connect(m_prbModalResponse,    SIGNAL(clicked()), SLOT(onResponseType()));

    connect(m_ppbAddCurve,   SIGNAL(clicked()),      SLOT(onAddCurve()));

    connect(m_pRenameAct, SIGNAL(triggered(bool)), SLOT(onRenameCurve()));
    connect(m_pDeleteAct, SIGNAL(triggered(bool)), SLOT(onDeleteCurve()));

    connect(m_pcpCurveTable, SIGNAL(clicked(QModelIndex)), SLOT(onCurveTableClicked(QModelIndex)));
    connect(m_pCurveModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onDataChanged(QModelIndex,QModelIndex)));
}


void StabTimeCtrls::showEvent(QShowEvent *pEvent)
{
    QFrame::showEvent(pEvent);

    GraphOptions::resetGraphSettings(*m_pSplGraphWt->graph());
    QFont fnt(DisplayOptions::tableFont());
    fnt.setPointSize(DisplayOptions::tableFont().pointSize());
    m_pcpCurveTable->setFont(fnt);
    m_pcpCurveTable->horizontalHeader()->setFont(fnt);
    m_pcpCurveTable->verticalHeader()->setFont(fnt);


    onResizeColumns();
    setControls();
}


void StabTimeCtrls::resizeEvent(QResizeEvent *pEvent)
{
    QFrame::resizeEvent(pEvent);
    onResizeColumns();
}


void StabTimeCtrls::onResizeColumns()
{
    double w = double(m_pcpCurveTable->width())/100.0;
    m_pcpCurveTable->setColumnWidth(0, int(35.0*w));
    m_pcpCurveTable->setColumnWidth(1, int(35.0*w));
    update();
}


void StabTimeCtrls::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_ppbAddCurve->hasFocus()) m_ppbAddCurve->setFocus();
            else onPlotStabilityGraph();

            break;
        }
        default:
        {
            s_pXPlane->keyPressEvent(pEvent);
        }
    }
}


void StabTimeCtrls::onStabilityDirection()
{
    fillCurveList();

    setMode();
    setControls();

    bool bLongitudinal = isStabLongitudinal();
    s_pXPlane->updateStabilityDirection(bLongitudinal);
}


void StabTimeCtrls::onDataChanged(QModelIndex topleft,QModelIndex )
{
    // using the standard model, so retrieve the data
    // only case to process: col 0, the name has been edited
    // the line style is edited from this class using a LineMenu

    if(!topleft.isValid()) return;

    int row = topleft.row();
    if(topleft.column()==0)
    {
        QString curvename = m_pCurveModel->index(row, topleft.column(), QModelIndex()).data().toString();
        for(int ig=0; ig<s_pXPlane->m_TimeGraph.size(); ig++)
        {
            Curve *pCurve= s_pXPlane->m_TimeGraph.at(ig)->curve(row);
            if(pCurve)
            {
                pCurve->setName(curvename);
            }
        }
        s_pXPlane->makeLegend();
        s_pXPlane->updateView();
    }
}


void StabTimeCtrls::onCurveTableClicked(QModelIndex index)
{
    if(!index.isValid()) return

    m_pcpCurveTable->selectRow(index.row());
    for(int i=0; i<s_pXPlane->m_TimeGraph.size(); i++)
    {
        s_pXPlane->m_TimeGraph.at(i)->clearSelection();
        Curve *pCurve = s_pXPlane->m_TimeGraph.at(i)->curve(index.row());
        s_pXPlane-> m_TimeGraph[i]->selectCurve(pCurve);
    }

    switch(index.column())
    {
        case 1:
        {
            int row = index.row();

            Curve *pCurrentCurve = s_pXPlane->m_TimeGraph[0]->curve(row);
            if(!pCurrentCurve) return;

            LineMenu *lineMenu = new LineMenu(nullptr);
            lineMenu->initMenu(pCurrentCurve->theStyle());
            lineMenu->exec(QCursor::pos());

            // update the model
            QStandardItem *pItem = m_pCurveModel->itemFromIndex(index);
            if(pItem) pItem->setData(QVariant::fromValue(lineMenu->theStyle()), Qt::DisplayRole);

            // update the curve style
            for(int ig=0; ig<s_pXPlane->m_TimeGraph.size(); ig++)
            {
                Curve *pCurrentCurve = s_pXPlane->m_TimeGraph[ig]->curve(row);
                pCurrentCurve->setTheStyle(lineMenu->theStyle());
            }
            s_pXPlane->makeLegend();
            s_pXPlane->updateView();
            break;
        }
        case 2:
        {
            QRect itemrect = m_pcpCurveTable->visualRect(index);
            QPoint menupos = m_pcpCurveTable->mapToGlobal(itemrect.topLeft());
            QMenu *pCurveTableRowMenu = new QMenu("Section",this);
            pCurveTableRowMenu->addAction(m_pRenameAct);
            pCurveTableRowMenu->addAction(m_pDeleteAct);
            pCurveTableRowMenu->exec(menupos, m_pRenameAct);

            break;
        }
        default:
        {
            break;
        }
    }
}


void StabTimeCtrls::onPlotStabilityGraph()
{
    s_pXPlane->createStabilityCurves();
    s_pXPlane->updateView();
}


void StabTimeCtrls::onModeSelection()
{
    if(s_pXPlane->isStabTimeView())
    {
        for(int imode=0; imode<4; imode++)
        {
            if(m_prbTimeMode[imode]->isChecked())
            {
                m_iCurrentMode = imode;
                break;
            }
        }
    }
    if(!isStabLongitudinal())m_iCurrentMode +=4;

    setMode(m_iCurrentMode);
}


void StabTimeCtrls::onReadData()
{
    m_Deltat = m_pdeDeltat->value();
}



void StabTimeCtrls::onResponseType()
{
    enumStabTimeResponse type = INITIALCONDITIONS;

    if     (m_prbInitCondResponse->isChecked())    type=INITIALCONDITIONS;
    else if(m_prbForcedResponse->isChecked())      type=FORCEDRESPONSE;
    else if(m_prbModalResponse->isChecked())       type=MODALRESPONSE;

    if(type==m_ResponseType) return;

    m_ResponseType=type;
    setControls();
    s_pXPlane->updateView();
}


void StabTimeCtrls::setMode(int iMode)
{
    if(iMode>=0)
    {
        m_iCurrentMode = iMode%4;
        if(!isStabLongitudinal()) m_iCurrentMode += 4;
    }
    else if(m_iCurrentMode<0) m_iCurrentMode=0;
}


void StabTimeCtrls::makeTable()
{
    m_pcpCurveTable = new CPTableView(this);
    m_pcpCurveTable->setShowGrid(false);
    m_pcpCurveTable->setEditable(true);

    m_pCurveModel = new ActionItemModel(this);
    m_pCurveModel->setRowCount(0);//nothing to start with
    m_pCurveModel->setColumnCount(3);
    m_pCurveModel->setActionColumn(2);
    m_pCurveModel->setHeaderData(0, Qt::Horizontal, "Name");
    m_pCurveModel->setHeaderData(1, Qt::Horizontal, "Style");
    m_pCurveModel->setHeaderData(2, Qt::Horizontal, "Actions");
    m_pcpCurveTable->setModel(m_pCurveModel);

    QHeaderView *pHorizontalHeader = m_pcpCurveTable->horizontalHeader();
    pHorizontalHeader->setStretchLastSection(true);

    XflDelegate *pEditActionDelegate = new XflDelegate(this);
    pEditActionDelegate->setActionColumn(2);
    pEditActionDelegate->setDigits({-1,-1,-1});

    QVector<XflDelegate::enumItemType> ItemType = {XflDelegate::STRING, XflDelegate::LINE, XflDelegate::ACTION};
    pEditActionDelegate->setItemTypes(ItemType);

    m_pcpCurveTable->setItemDelegate(pEditActionDelegate);
}


void StabTimeCtrls::setupLayout()
{

    QGroupBox *pgbDirection = new QGroupBox("Direction");
    {
        QHBoxLayout *pDirLayout = new QHBoxLayout;
        {
            QButtonGroup *pGroup = new QButtonGroup;
            {
                m_prbLongitudinal = new QRadioButton("Longitudinal");
                m_prbLateral = new QRadioButton("Lateral");
                pGroup->addButton(m_prbLongitudinal);
                pGroup->addButton(m_prbLateral);
            }

            pDirLayout->addStretch();
            pDirLayout->addWidget(m_prbLongitudinal);
            pDirLayout->addWidget(m_prbLateral);
            pDirLayout->addStretch();
        }
        pgbDirection->setLayout(pDirLayout);
    }

    QGroupBox *pgbCurveType = new QGroupBox("Response type");
    {
        QHBoxLayout *pResponseTypeLayout = new QHBoxLayout;
        {
            m_prbModalResponse = new QRadioButton("Modal");
            m_prbModalResponse->setToolTip("<p>Display the time response on a specific mode with normalized amplitude and random initial phase</p>");
            m_prbInitCondResponse = new QRadioButton("Initial conditions");
            m_prbInitCondResponse->setToolTip("<p>Display the time response for specified initial conditions</p>");
            m_prbForcedResponse = new QRadioButton("Forced");
            m_prbForcedResponse->setToolTip("<p>Display the time response for a given control actuation in the form "
                                            "of a user-specified function of time</p>");
            pResponseTypeLayout->addStretch();
            pResponseTypeLayout->addWidget(m_prbModalResponse);
            pResponseTypeLayout->addWidget(m_prbInitCondResponse);
            pResponseTypeLayout->addWidget(m_prbForcedResponse);
            pResponseTypeLayout->addStretch();
        }
        pgbCurveType->setLayout(pResponseTypeLayout);
    }

    m_pswInitialConditions = new QStackedWidget;
    {
        QGroupBox *pgbInitCondResponse = new QGroupBox("Initial conditions response");
        {
            QGridLayout *pVarParamsLayout = new QGridLayout;
            {
                m_plabStab1 = new QLabel("u0__");
                m_plabStab2 = new QLabel("w0__");
                m_plabStab3 = new QLabel("q0__");
                m_plabUnit1 = new QLabel("m/s");
                m_plabUnit2 = new QLabel("m/s");
                m_plabUnit3 = new QLabel("rad/s");
                m_plabStab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_plabStab2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_plabStab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_plabUnit1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                m_plabUnit2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                m_plabUnit3->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                m_pdeStabVar1 = new FloatEdit;
                m_pdeStabVar2 = new FloatEdit;
                m_pdeStabVar3 = new FloatEdit;
                pVarParamsLayout->addWidget(m_plabStab1,   1,1);
                pVarParamsLayout->addWidget(m_plabStab2,   2,1);
                pVarParamsLayout->addWidget(m_plabStab3,   3,1);
                pVarParamsLayout->addWidget(m_pdeStabVar1, 1,2);
                pVarParamsLayout->addWidget(m_pdeStabVar2, 2,2);
                pVarParamsLayout->addWidget(m_pdeStabVar3, 3,2);
                pVarParamsLayout->addWidget(m_plabUnit1,   1,3);
                pVarParamsLayout->addWidget(m_plabUnit2,   2,3);
                pVarParamsLayout->addWidget(m_plabUnit3,   3,3);
                pVarParamsLayout->setRowStretch(4,1);
                pVarParamsLayout->setColumnStretch(4,1);
            }
            pgbInitCondResponse->setLayout(pVarParamsLayout);
        }

        QGroupBox *pgbForcedResponse = new QGroupBox("Forced response");
        {
            QVBoxLayout *pForcedResponseLayout = new QVBoxLayout;
            {
                m_pcbAVLControls = new QComboBox(this);
                QLabel *pForcedText = new QLabel("Control function");

                m_pSplGraphWt = new SplinedGraphWt;
                {
                    m_pSplGraphWt->setGraph(&m_InputGraph);
                    m_pSplGraphWt->setEndPointConstrain(true);
                    m_pSplGraphWt->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                    GraphOptions::resetGraphSettings(m_InputGraph);
                }

                pForcedResponseLayout->addWidget(m_pcbAVLControls);
                pForcedResponseLayout->addWidget(pForcedText);
                pForcedResponseLayout->addWidget(m_pSplGraphWt);
            }
            pgbForcedResponse->setLayout(pForcedResponseLayout);
        }

        QGroupBox *pgbModalTime = new QGroupBox("Modal response");
        {
            pgbModalTime->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            QVBoxLayout *pModalTimeLayout = new QVBoxLayout;
            {
                for(int imode=0; imode<4; imode++)
                {
                    m_prbTimeMode[imode] = new QRadioButton(QString::asprintf("Mode %d", imode+1));
                    pModalTimeLayout->addWidget(m_prbTimeMode[imode]);
                }
                pModalTimeLayout->addStretch(1);
            }
            pgbModalTime->setLayout(pModalTimeLayout);
        }

        m_pswInitialConditions->addWidget(pgbModalTime);
        m_pswInitialConditions->addWidget(pgbInitCondResponse);
        m_pswInitialConditions->addWidget(pgbForcedResponse);
    }


    QGridLayout *pDtLayout  = new QGridLayout;
    {
        QLabel *pDtLabel        = new QLabel("dt=");
        QLabel *pTotalTimeLabel = new QLabel("Total Time =");
        QLabel *pTimeLab1       = new QLabel("s");
        QLabel *pTimeLab2       = new QLabel("s");
        m_pdeTotalTime = new FloatEdit(5.0f);
        m_pdeTotalTime->setToolTip("<p>Define the total time range for the graphs</p>");
        m_pdeDeltat    = new FloatEdit(.01f);
        m_pdeDeltat->setToolTip("<p>Define the time step for the resolution of the differential equations</p>");
        pDtLayout->addWidget(pDtLabel,         1,1, Qt::AlignRight);
        pDtLayout->addWidget(m_pdeDeltat,      1,2);
        pDtLayout->addWidget(pTimeLab1,        1,3);
        pDtLayout->addWidget(pTotalTimeLabel,  2,1, Qt::AlignRight);
        pDtLayout->addWidget(m_pdeTotalTime,   2,2);
        pDtLayout->addWidget(pTimeLab2,        2,3);
    }

    QGroupBox *pgbCurveSettings = new QGroupBox("Curve settings");
    {
        QVBoxLayout *pTimeLayout = new QVBoxLayout;
        {
            m_ppbAddCurve  = new QPushButton("Add");
            m_ppbAddCurve->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            m_ppbAddCurve->setToolTip("<p>Add a new curve to the graphs, using the current user-specified input</p>");

            pTimeLayout->addLayout(pDtLayout);
            pTimeLayout->addWidget(m_ppbAddCurve);
            pTimeLayout->addWidget(m_pcpCurveTable);
        }
        pgbCurveSettings->setLayout(pTimeLayout);
    }

    QVBoxLayout *pTimeParamsLayout = new QVBoxLayout;
    {
        //    TimeParamsLayout->addLayout(InitialConditionsLayout);
        pTimeParamsLayout->addWidget(pgbDirection);
        pTimeParamsLayout->addWidget(pgbCurveType);
        pTimeParamsLayout->addWidget(m_pswInitialConditions);
        pTimeParamsLayout->addWidget(pgbCurveSettings);
    }

    setLayout(pTimeParamsLayout);
}


void StabTimeCtrls::fillAVLcontrols(PlanePolar const*pWPolar)
{
    m_pcbAVLControls->clear();
    if(pWPolar)
    {
        for(int ic=0; ic<pWPolar->nAVLCtrls(); ic++)
        {
            m_pcbAVLControls->addItem(QString::fromStdString(pWPolar->AVLCtrl(ic).name()));
        }
    }
}


void StabTimeCtrls::setControls()
{
    QString str, strong;
    str = Units::speedUnitQLabel();

    m_pswInitialConditions->setCurrentIndex(m_ResponseType);

    m_prbInitCondResponse->setChecked(m_ResponseType==INITIALCONDITIONS);
    m_prbForcedResponse->setChecked(  m_ResponseType==FORCEDRESPONSE);
    m_prbModalResponse->setChecked(   m_ResponseType==MODALRESPONSE);

    setMode(m_iCurrentMode);

    strong = DEGch + "/s";
    if(isStabLongitudinal())
    {
        m_plabStab1->setText("<p>u<sub>0</sub>=</p>");
        m_plabStab2->setText("<p>w<sub>0</sub>=</p>");
        m_plabStab3->setText("<p>q<sub>0</sub>=</p>");
        m_plabUnit1->setText(str);
        m_plabUnit2->setText(str);
        m_plabUnit3->setText(strong);
    }
    else
    {
        m_plabStab1->setText("<p>v<sub>0</sub>=</p>");
        m_plabStab2->setText("<p>p<sub>0</sub>=</p>");
        m_plabStab3->setText("<p>r<sub>0</sub>=</p>");
        m_plabUnit1->setText(str);
        m_plabUnit2->setText(strong);
        m_plabUnit3->setText(strong);
    }

    m_pdeStabVar1->setValue(m_TimeInput[0]);
    m_pdeStabVar2->setValue(m_TimeInput[1]);
    m_pdeStabVar3->setValue(m_TimeInput[2]);
    m_pdeTotalTime->setValue(m_TotalTime);
    m_pdeDeltat->setValue(m_Deltat);

    bool bEnableTimeCtrl = s_pXPlane->m_pCurPOpp && s_pXPlane->m_pCurPOpp->isType7() && s_pXPlane->isStabTimeView();
    m_ppbAddCurve->setEnabled(bEnableTimeCtrl);
    m_pcpCurveTable->setEnabled(m_pCurveModel->rowCount());

    for(int imode=0; imode<4; imode++)
    {
        m_prbTimeMode[imode]->setChecked(m_iCurrentMode%4==imode);
        m_prbTimeMode[imode]->setEnabled(bEnableTimeCtrl);
    }

    m_pdeStabVar1->setEnabled(bEnableTimeCtrl);
    m_pdeStabVar2->setEnabled(bEnableTimeCtrl);
    m_pdeStabVar3->setEnabled(bEnableTimeCtrl);
    m_pdeDeltat->setEnabled(bEnableTimeCtrl);
    m_pdeTotalTime->setEnabled(bEnableTimeCtrl);

    m_pcbAVLControls->setEnabled(bEnableTimeCtrl && m_prbForcedResponse->isChecked());
    m_pSplGraphWt->initialize();
}


QString StabTimeCtrls::selectedCurveName()
{
    QModelIndex index = m_pcpCurveTable->currentIndex();
    if(!index.isValid()) return QString();

    QModelIndex sib = index.sibling(index.row(), 0);
    if(sib.isValid()) return m_pCurveModel->data(sib).toString();
    else              return QString();
}


Curve *StabTimeCtrls::selectedCurve()
{
    // cannot proceed by index because the sorting order of the graph's CurveModel
    // may not match the order of the table's model

    QModelIndex index = m_pcpCurveTable->currentIndex();
    if(!index.isValid()) return nullptr;

    QModelIndex sib = index.sibling(index.row(), 0);
    if(!sib.isValid()) return nullptr;

    QString strange = m_pCurveModel->data(sib).toString();
    return s_pXPlane->m_TimeGraph.at(0)->curve(strange);
}


void StabTimeCtrls::selectCurve(Curve const *pCurve)
{
    if(!pCurve) return;

    for(int ir=0; ir<m_pCurveModel->rowCount(); ir++)
    {
        QModelIndex index = m_pCurveModel->index(ir, 0);
        QStandardItem *pItem = m_pCurveModel->itemFromIndex(index);
        if(pItem && pItem->text().compare(pCurve->name())==0)
        {
            m_pcpCurveTable->setCurrentIndex(index);
        }
    }
}

void StabTimeCtrls::onRenameCurve()
{
    Curve *pSelCurve = selectedCurve();
    if(!pSelCurve) return;

    QString NewName = "Test Name";
    NewNameDlg dlg(pSelCurve->name(), this);

    if(dlg.exec() != QDialog::Accepted) return;
    NewName = dlg.newName();

    for (int i=0; i<s_pXPlane->m_TimeGraph.at(0)->curveCount(); i++)
    {
        Curve *pCurve = s_pXPlane->m_TimeGraph.at(0)->curve(i);
        if(pCurve && (pCurve == pSelCurve))
        {
            for(int ig=0; ig<s_pXPlane->m_TimeGraph.size(); ig++)
            {
                pCurve = s_pXPlane->m_TimeGraph.at(ig)->curve(i);
                pCurve->setName(NewName);
            }

            fillCurveList();
            s_pXPlane->makeLegend();
            s_pXPlane->updateView();
            return;
        }
    }
}


void StabTimeCtrls::onSelChangeCurve(int )
{
}


void StabTimeCtrls::addCurve()
{
    QString strong;
    bool bLongitudinal = isStabLongitudinal();

    double val1 = m_pdeStabVar1->value();
    double val2 = m_pdeStabVar2->value();
    double val3 = m_pdeStabVar3->value();
    switch(m_ResponseType)
    {
        case INITIALCONDITIONS:
        {
            if(bLongitudinal) strong = QString::asprintf("u0=%5.2f w0=%5.2f q0=%5.2f", val1, val2, val3);
            else              strong = QString::asprintf("v0=%5.2f p0=%5.2f r0=%5.2f", val1, val2, val3);
            break;
        }
        case FORCEDRESPONSE:
        {
            strong = "Forced response";
            break;
        }
        case MODALRESPONSE:
        {
            strong = QString::asprintf("Mode %1d", m_iCurrentMode+1);
            break;
        }
    }

    // check if the name exists
    for(int row=0; row<m_pCurveModel->rowCount(); row++)
    {
        QStandardItem *pItem = m_pCurveModel->item(row);
        if(pItem && pItem->text().compare(strong, Qt::CaseInsensitive)==0)
        {
            strong += " (2)";
        }
    }

    Curve *pCurve=nullptr;
    fl5Color clr = xfl::randomfl5Color(!DisplayOptions::isLightTheme());
    for(int ig=0; ig<s_pXPlane->m_TimeGraph.size(); ig++)
    {
        pCurve = s_pXPlane->m_TimeGraph.at(ig)->addCurve();
        pCurve->setColor(clr);
        pCurve->setDefaultLineWidth(Curve::defaultLineWidth());
        pCurve->setName(strong);
    }

    if(pCurve)
    {
        appendRow(pCurve);
    }
//    m_pCurveModel->sort(0, Qt::AscendingOrder);

    int row=0;
    if(pCurve)
    {
        for(row=0; row<m_pCurveModel->rowCount(); row++)
        {
            QStandardItem *pItem = m_pCurveModel->item(row);
            if(pItem && pItem->text().compare(pCurve->name())==0) break;
        }
    }

    if(row<m_pCurveModel->rowCount())
    {
        m_pcpCurveTable->selectRow(row);
    }

    m_pcpCurveTable->setEnabled(   s_pXPlane->m_pCurPOpp && m_pCurveModel->rowCount());
}


void StabTimeCtrls::appendRow(Curve const *pCurve)
{
    int rowcount = m_pCurveModel->rowCount();
    rowcount++;
    m_pCurveModel->setRowCount(rowcount);

    m_pCurveModel->blockSignals(true);

    QModelIndex Xindex = m_pCurveModel->index(rowcount-1, 0, QModelIndex());
    m_pCurveModel->setData(Xindex, pCurve->name());

    QModelIndex lineindex = m_pCurveModel->index(rowcount-1, 1, QModelIndex());
    QStandardItem *pItem = m_pCurveModel->itemFromIndex(lineindex);
    if(pItem) pItem->setData(QVariant::fromValue(pCurve->theStyle()), Qt::DisplayRole);

    QModelIndex actionindex = m_pCurveModel->index(rowcount-1, 2, QModelIndex());
    m_pCurveModel->setData(actionindex, QString("..."));

    m_pCurveModel->blockSignals(false);
    m_pcpCurveTable->resizeRowsToContents();
}


void StabTimeCtrls::onAddCurve()
{
    addCurve();

    onPlotStabilityGraph();
    s_pXPlane->makeLegend();
}


void StabTimeCtrls::onDeleteCurve()
{
    Curve *pSelCurve = selectedCurve();
    if(!pSelCurve) return;
    QString curvename = pSelCurve->name();
    for(int ig=0; ig<s_pXPlane->m_TimeGraph.size(); ig++) s_pXPlane->m_TimeGraph[ig]->deleteCurve(curvename);

    int row=0;
    for(row=0; row<m_pCurveModel->rowCount(); row++)
    {
        QStandardItem *pItem = m_pCurveModel->item(row);
        if(pItem->text().compare(curvename)==0)
        {
            m_pCurveModel->removeRow(row);
            break;
        }
    }

    m_pcpCurveTable->setEnabled(    m_pCurveModel->rowCount());

    QModelIndex index = m_pcpCurveTable->currentIndex();
    if(index.isValid())
    {
        int newrow = index.row();
        QStandardItem *pItem = m_pCurveModel->item(newrow);
        if(pItem) curvename = pItem->text();
        else      curvename.clear();
    }

    s_pXPlane->makeLegend();
    s_pXPlane->updateView();
}


void StabTimeCtrls::fillCurveList()
{
     m_pCurveModel->setRowCount(0);
    for(int i=0; i<s_pXPlane->m_TimeGraph.at(0)->curveCount(); i++)
    {
        appendRow(s_pXPlane->m_TimeGraph.at(0)->curve(i));
    }
}


double StabTimeCtrls::getControlInput(const double &time) const
{
    BSpline const &spline = m_pSplGraphWt->spline();
    if(time<0.0) return 0.0;
    if(time>=spline.lastCtrlPoint().x) return 0.0;
    for(int io=1; io<spline.outputSize(); io++)
    {
        if(spline.outputPt(io-1).x<=time && time<spline.outputPt(io).x)
        {
            double tau = (time - spline.outputPt(io-1).x)/(spline.outputPt(io).x-spline.outputPt(io-1).x);
            return spline.outputPt(io-1).y + tau * (spline.outputPt(io).y-spline.outputPt(io-1).y);
        }
    }
    return 0.0;
}


void StabTimeCtrls::fillTimeCurve(PlaneOpp const*pPOpp, Curve **pCurve)
{
    if(m_ResponseType==FORCEDRESPONSE)
        fillCurvesForcedResponse(pPOpp, pCurve);
    else
        fillCurvesPerturbation(pPOpp, pCurve);
}


void StabTimeCtrls::fillCurvesForcedResponse(PlaneOpp const*pPOpp, Curve **pCurve)
{
    // Builds the forced response from the state matrix and the forced input matrix
    // using a RK4 integration scheme.
    // The forced input is interpolated in the control history defined in the input table.

    if(pPOpp->m_BLong.size()==0) return;

    bool bLongitudinal = isStabLongitudinal();

    double A[4][4];    memset(A, 0, 16*sizeof(double));
    double m[5][4];    memset(m, 0, 20*sizeof(double));
    double B[]{0,0,0,0};
    double y[]{0,0,0,0};
    double yp[]{0,0,0,0};


    if(!pPOpp || !pPOpp->isType7()) return;//nothing to plot

    int iAVLCtrl = m_pcbAVLControls->currentIndex();
    if(iAVLCtrl<0||iAVLCtrl>=m_pcbAVLControls->count()) return;

    if(bLongitudinal)
    {
        memcpy(A, pPOpp->m_ALong, 4*4*sizeof(double));
        if(pPOpp->m_BLong.size())
        {
            B[0] = pPOpp->m_BLong.at(iAVLCtrl).at(0);
            B[1] = pPOpp->m_BLong.at(iAVLCtrl).at(1);
            B[2] = pPOpp->m_BLong.at(iAVLCtrl).at(2);
            B[3] = pPOpp->m_BLong.at(iAVLCtrl).at(3);
        }
    }
    else
    {
        memcpy(A, pPOpp->m_ALat, 4*4*sizeof(double));
        if(pPOpp->m_BLat.size())
        {
            B[0] = pPOpp->m_BLat.at(iAVLCtrl).at(0);
            B[1] = pPOpp->m_BLat.at(iAVLCtrl).at(1);
            B[2] = pPOpp->m_BLat.at(iAVLCtrl).at(2);
            B[3] = pPOpp->m_BLat.at(iAVLCtrl).at(3);
        }
    }

    m_Deltat    = deltaT();
    m_TotalTime = totalTime();

    double  dt = m_TotalTime/1000.;
    if(dt<m_Deltat) dt = m_Deltat;

    int TotalPoints  = std::min(1000, int(m_TotalTime/dt));
    int PlotInterval = std::max(1,    int(TotalPoints/200));

    // we are considering forced response from initial steady state, so set
    // initial conditions to 0
    double t=0.0, ctrl_t=0.0;
    y[0] = y[1] = y[2] = y[3] = 0.0;
    pCurve[0]->appendPoint(0.0, y[0]);
    pCurve[1]->appendPoint(0.0, y[1]);
    pCurve[2]->appendPoint(0.0, y[2]);
    pCurve[3]->appendPoint(0.0, y[3]);

    // RK4
    for(int i=0; i<TotalPoints; i++)
    {
        //initial slope m1
        m[0][0] = A[0][0]*y[0] + A[0][1]*y[1] + A[0][2]*y[2] + A[0][3]*y[3];
        m[0][1] = A[1][0]*y[0] + A[1][1]*y[1] + A[1][2]*y[2] + A[1][3]*y[3];
        m[0][2] = A[2][0]*y[0] + A[2][1]*y[1] + A[2][2]*y[2] + A[2][3]*y[3];
        m[0][3] = A[3][0]*y[0] + A[3][1]*y[1] + A[3][2]*y[2] + A[3][3]*y[3];

        ctrl_t = getControlInput(t);
        m[0][0] += B[0] * ctrl_t;
        m[0][1] += B[1] * ctrl_t;
        m[0][2] += B[2] * ctrl_t;
        m[0][3] += B[3] * ctrl_t;

        //middle point m2
        yp[0] = y[0] + dt/2.0 * m[0][0];
        yp[1] = y[1] + dt/2.0 * m[0][1];
        yp[2] = y[2] + dt/2.0 * m[0][2];
        yp[3] = y[3] + dt/2.0 * m[0][3];

        m[1][0] = A[0][0]*yp[0] + A[0][1]*yp[1] + A[0][2]*yp[2] + A[0][3]*yp[3];
        m[1][1] = A[1][0]*yp[0] + A[1][1]*yp[1] + A[1][2]*yp[2] + A[1][3]*yp[3];
        m[1][2] = A[2][0]*yp[0] + A[2][1]*yp[1] + A[2][2]*yp[2] + A[2][3]*yp[3];
        m[1][3] = A[3][0]*yp[0] + A[3][1]*yp[1] + A[3][2]*yp[2] + A[3][3]*yp[3];

        ctrl_t = getControlInput(t+dt/2.0);
        m[1][0] += B[0] * ctrl_t;
        m[1][1] += B[1] * ctrl_t;
        m[1][2] += B[2] * ctrl_t;
        m[1][3] += B[3] * ctrl_t;

        //second point m3
        yp[0] = y[0] + dt/2.0 * m[1][0];
        yp[1] = y[1] + dt/2.0 * m[1][1];
        yp[2] = y[2] + dt/2.0 * m[1][2];
        yp[3] = y[3] + dt/2.0 * m[1][3];

        m[2][0] = A[0][0]*yp[0] + A[0][1]*yp[1] + A[0][2]*yp[2] + A[0][3]*yp[3];
        m[2][1] = A[1][0]*yp[0] + A[1][1]*yp[1] + A[1][2]*yp[2] + A[1][3]*yp[3];
        m[2][2] = A[2][0]*yp[0] + A[2][1]*yp[1] + A[2][2]*yp[2] + A[2][3]*yp[3];
        m[2][3] = A[3][0]*yp[0] + A[3][1]*yp[1] + A[3][2]*yp[2] + A[3][3]*yp[3];

        ctrl_t = getControlInput(t+dt/2.0);

        m[2][0] += B[0] * ctrl_t;
        m[2][1] += B[1] * ctrl_t;
        m[2][2] += B[2] * ctrl_t;
        m[2][3] += B[3] * ctrl_t;

        //third point m4
        yp[0] = y[0] + dt * m[2][0];
        yp[1] = y[1] + dt * m[2][1];
        yp[2] = y[2] + dt * m[2][2];
        yp[3] = y[3] + dt * m[2][3];

        m[3][0] = A[0][0]*yp[0] + A[0][1]*yp[1] + A[0][2]*yp[2] + A[0][3]*yp[3];
        m[3][1] = A[1][0]*yp[0] + A[1][1]*yp[1] + A[1][2]*yp[2] + A[1][3]*yp[3];
        m[3][2] = A[2][0]*yp[0] + A[2][1]*yp[1] + A[2][2]*yp[2] + A[2][3]*yp[3];
        m[3][3] = A[3][0]*yp[0] + A[3][1]*yp[1] + A[3][2]*yp[2] + A[3][3]*yp[3];

        ctrl_t = getControlInput(t+dt);

        m[3][0] += B[0] * ctrl_t;
        m[3][1] += B[1] * ctrl_t;
        m[3][2] += B[2] * ctrl_t;
        m[3][3] += B[3] * ctrl_t;

        //final slope m5
        m[4][0] = 1./6. * (m[0][0] + 2.0*m[1][0] + 2.0*m[2][0] + m[3][0]);
        m[4][1] = 1./6. * (m[0][1] + 2.0*m[1][1] + 2.0*m[2][1] + m[3][1]);
        m[4][2] = 1./6. * (m[0][2] + 2.0*m[1][2] + 2.0*m[2][2] + m[3][2]);
        m[4][3] = 1./6. * (m[0][3] + 2.0*m[1][3] + 2.0*m[2][3] + m[3][3]);

        y[0] += m[4][0] * dt;
        y[1] += m[4][1] * dt;
        y[2] += m[4][2] * dt;
        y[3] += m[4][3] * dt;
        t +=dt;
        if(fabs(y[0])>1.e10 || fabs(y[1])>1.e10 || fabs(y[2])>1.e10  || fabs(y[3])>1.e10 ) break;

        if(i%PlotInterval==0)
        {
            if(bLongitudinal)
            {
                pCurve[0]->appendPoint(t, y[0]*Units::mstoUnit());
                pCurve[1]->appendPoint(t, y[1]*Units::mstoUnit());
                pCurve[2]->appendPoint(t, y[2]*180.0/PI);//deg/s
                pCurve[3]->appendPoint(t, y[3]*180.0/PI);//deg
            }
            else
            {
                pCurve[0]->appendPoint(t, y[0]*Units::mstoUnit());
                pCurve[1]->appendPoint(t, y[1]*180.0/PI);//deg/s
                pCurve[2]->appendPoint(t, y[2]*180.0/PI);//deg/s
                pCurve[3]->appendPoint(t, y[3]*180.0/PI);//deg
            }
        }
    }
}


void StabTimeCtrls::fillCurvesPerturbation(PlaneOpp const*pPOpp, Curve **pCurve)
{
    // The time response is calculated analytically based on the eigenvalues and eigenvectors
    std::complex<double> M[16];// the modal matrix
    std::complex<double> InvM[16];// the inverse of the modal matrix
    std::complex<double> q[4],q0[4],y[4];//the part of each mode in the solution

    std::complex<double> in[4];

    if(!pPOpp || !pPOpp->isType7()) return;

    bool bLongitudinal = isStabLongitudinal();

    m_Deltat    = deltaT();
    m_TotalTime = totalTime();
    double dt = m_TotalTime/1000.;
    if(dt<m_Deltat) dt = m_Deltat;

    int TotalPoints = std::min(1000, int(m_TotalTime/dt));
    //read the initial state condition
    m_TimeInput[0] = m_pdeStabVar1->value();
    m_TimeInput[1] = m_pdeStabVar2->value();
    m_TimeInput[2] = m_pdeStabVar3->value();
    m_TimeInput[3] = 0.0;//we start with an initial 0.0 value for pitch or bank angles

    if(m_ResponseType==INITIALCONDITIONS)
    {
        //start with the user input initial conditions
        in[0] = std::complex<double>(m_TimeInput[0], 0.0);
        in[1] = std::complex<double>(m_TimeInput[1], 0.0);
        in[2] = std::complex<double>(m_TimeInput[2], 0.0);
        in[3] = std::complex<double>(m_TimeInput[3], 0.0);
    }
    else if(m_ResponseType==MODALRESPONSE)
    {
        //start with the initial conditions which will excite only the requested mode
        in[0] = pPOpp->m_EigenVector[m_iCurrentMode][0];
        in[1] = pPOpp->m_EigenVector[m_iCurrentMode][1];
        in[2] = pPOpp->m_EigenVector[m_iCurrentMode][2];
        in[3] = pPOpp->m_EigenVector[m_iCurrentMode][3];
    }
    else return;

    //fill the modal matrix
    int k=0;
    if(bLongitudinal) k=0; else k=1;
    for (int i=0; i<4; i++)
    {
        for(int j=0;j<4;j++)
        {
            *(M+4*j+i) = pPOpp->m_EigenVector[k*4+i][j];
        }
    }

    //Invert the matrix
    if(!matrix::invert44(M, InvM))
    {
    }
    else
    {
        //calculate the modal coefficients at t=0
        q0[0] = InvM[0] * in[0] + InvM[1] * in[1] + InvM[2] * in[2] + InvM[3] * in[3];
        q0[1] = InvM[4] * in[0] + InvM[5] * in[1] + InvM[6] * in[2] + InvM[7] * in[3];
        q0[2] = InvM[8] * in[0] + InvM[9] * in[1] + InvM[10]* in[2] + InvM[11]* in[3];
        q0[3] = InvM[12]* in[0] + InvM[13]* in[1] + InvM[14]* in[2] + InvM[15]* in[3];

        for(int i=0; i<TotalPoints; i++)
        {
            double t = i * dt;
            q[0] = q0[0] * exp(pPOpp->m_EigenValue[0+k*4]*t);
            q[1] = q0[1] * exp(pPOpp->m_EigenValue[1+k*4]*t);
            q[2] = q0[2] * exp(pPOpp->m_EigenValue[2+k*4]*t);
            q[3] = q0[3] * exp(pPOpp->m_EigenValue[3+k*4]*t);
            y[0] = *(M+4*0+0) * q[0] +*(M+4*0+1) * q[1] +*(M+4*0+2) * q[2] +*(M+4*0+3) * q[3];
            y[1] = *(M+4*1+0) * q[0] +*(M+4*1+1) * q[1] +*(M+4*1+2) * q[2] +*(M+4*1+3) * q[3];
            y[2] = *(M+4*2+0) * q[0] +*(M+4*2+1) * q[1] +*(M+4*2+2) * q[2] +*(M+4*2+3) * q[3];
            y[3] = *(M+4*3+0) * q[0] +*(M+4*3+1) * q[1] +*(M+4*3+2) * q[2] +*(M+4*3+3) * q[3];
            if(std::abs(q[0])>1.e10 || std::abs(q[1])>1.e10 || std::abs(q[2])>1.e10  || std::abs(q[3])>1.e10 ) break;

            pCurve[0]->appendPoint(t, y[0].real());
            if(bLongitudinal) pCurve[1]->appendPoint(t, y[1].real());
            else                           pCurve[1]->appendPoint(t, y[1].real()*180.0/PI);
            pCurve[2]->appendPoint(t, y[2].real()*180.0/PI);
            pCurve[3]->appendPoint(t, y[3].real()*180.0/PI);
        }
    }
}


void StabTimeCtrls::loadSettings(QSettings &settings)
{
    settings.beginGroup("StabTimeControls");
    {
        QString strong;
        int functionsize = settings.value("timefunctsize", 5).toInt();
        std::vector<Node2d> pts(functionsize);
        for(int i=0; i<functionsize; i++)
        {
            strong = QString("ForcedTime%1").arg(i);
            pts[i].x =settings.value(strong, double(i)).toDouble();
        }
        for(int i=0; i<functionsize; i++)
        {
            strong = QString("ForcedAmplitude%1").arg(i);
            pts[i].y =settings.value(strong, double(i)).toDouble();
        }
        m_pSplGraphWt->spline().setCtrlPoints(pts);

        m_TotalTime         = settings.value("TotalTime",1.0).toDouble();
        m_Deltat            = settings.value("Delta_t",0.001).toDouble();

        m_TimeInput[0]      = settings.value("TimeIn0",0.0).toDouble();
        m_TimeInput[1]      = settings.value("TimeIn1",0.0).toDouble();
        m_TimeInput[2]      = settings.value("TimeIn2",0.0).toDouble();
        m_TimeInput[3]      = settings.value("TimeIn3",0.0).toDouble();

    }
    settings.endGroup();
}


void StabTimeCtrls::saveSettings(QSettings &settings)
{
    settings.beginGroup("StabTimeControls");
    {
        QString strong;
        BSpline const &bSpline = m_pSplGraphWt->spline();
        settings.setValue("timefunctsize", bSpline.ctrlPointCount());

        for(int i=0; i<int(bSpline.ctrlPointCount()); i++)
        {
            strong = QString("ForcedTime%1").arg(i);
            settings.setValue(strong, bSpline.controlPoint(i).x);
        }
        for(int i=0; i<int(bSpline.ctrlPointCount()); i++)
        {
            strong = QString("ForcedAmplitude%1").arg(i);
            settings.setValue(strong, bSpline.controlPoint(i).y);
        }
        settings.setValue("TotalTime", m_TotalTime);
        settings.setValue("Delta_t", m_Deltat);

        settings.setValue("TimeIn0", m_TimeInput[0]);
        settings.setValue("TimeIn1", m_TimeInput[1]);
        settings.setValue("TimeIn2", m_TimeInput[2]);
        settings.setValue("TimeIn3", m_TimeInput[3]);
    }
    settings.endGroup();
}
