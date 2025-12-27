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

#include <QVBoxLayout>

#include "foilcamberdlg.h"


#include <interfaces/editors/foiledit/foilwt.h>
#include <interfaces/controls/splinectrl/splinectrl.h>
#include <interfaces/controls/splinectrl/splineapproxdlg.h>
#include <api/foil.h>
#include <api/objects2d.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>

QByteArray FoilCamberDlg::s_HSplitterSizes;
BSpline FoilCamberDlg::s_CSpline;
BSpline FoilCamberDlg::s_TSpline;


FoilCamberDlg::FoilCamberDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle("Foil from camber and thickness");
    setupLayout();
    connectSignals();

    FoilWt::showCamberLines(true);

    if(s_CSpline.outputSize()<=0) s_CSpline.setOutputSize(79);
    if(s_TSpline.outputSize()<=0) s_TSpline.setOutputSize(79);

    s_CSpline.setBunchType(Spline::SIGMOID);
    if(!s_CSpline.ctrlPointCount())
        makeDefaultSplines();
}


void FoilCamberDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);

    s_CSpline.setVisible(true);
    s_TSpline.setVisible(true);

    m_pCCtrls->initSplineCtrls(&s_CSpline);
    m_pTCtrls->initSplineCtrls(&s_TSpline);

    m_CStack.clear();
    m_CStack.push(s_CSpline);
    m_StackPos = 0;

    m_TStack.clear();
    m_TStack.push(s_TSpline);
    m_StackPos = 0;


    m_pBufferFoil->show();
    m_pFoilWt->addFoil(m_pBufferFoil);

    m_pFoilWt->addSpline(&s_CSpline);
    m_pFoilWt->addSpline(&s_TSpline);
    m_pFoilWt->setIn01(true);

    onApply();
}


void FoilCamberDlg::onNewSpline()
{
    makeDefaultSplines();

    onTakePicture();
}


void FoilCamberDlg::makeDefaultSplines()
{
    s_CSpline.setColor(fl5Color(255,0,0));
    s_CSpline.setStipple(Line::DASH);
    s_CSpline.setWidth(2);
    s_CSpline.clearControlPoints();
    s_CSpline.appendControlPoint({0,0});
    s_CSpline.appendControlPoint({0.1024,0.0088});
    s_CSpline.appendControlPoint({0.328,0.02903});
    s_CSpline.appendControlPoint({0.7618,0.0135});
    s_CSpline.appendControlPoint({1.00,0});
    s_CSpline.setBunchAmplitude(0.5);
    s_CSpline.updateSpline();
    s_CSpline.makeCurve();

    s_TSpline.setColor(fl5Color(209,71,209));
    s_TSpline.setStipple(Line::SOLID);
    s_TSpline.setWidth(2);
    s_TSpline.clearControlPoints();
    s_TSpline.appendControlPoint(0,0);
    s_TSpline.appendControlPoint(0.0, 0.05);
    s_TSpline.appendControlPoint(0.2697,0.1367);
    s_TSpline.appendControlPoint(0.80319,0.0484);
    s_TSpline.appendControlPoint({1.00,0});
    s_TSpline.setBunchAmplitude(0.5);
    s_TSpline.updateSpline();
    s_TSpline.makeCurve();
}


void FoilCamberDlg::onApply()
{
    makeFoilFromSplines();

    m_pBufferFoil->setName(std::string("Camb.-Th. foil"));
    m_pBufferFoil->initGeometry();

    m_pFoilWt->update();
}


void FoilCamberDlg::onReset()
{
    onNewSpline();
    onApply();
}


void FoilCamberDlg::setupLayout()
{
    QFrame *pLeftFrame = new QFrame;
    {
        pLeftFrame->setCursor(Qt::ArrowCursor);
        QVBoxLayout *pLeftLayout = new QVBoxLayout;
        {
            QTabWidget *pSplineCtrlTab = new QTabWidget;
            {
                QFrame *pCFrame = new QFrame;
                {
                    QVBoxLayout *pCLayout = new QVBoxLayout;
                    {
                        m_pCCtrls = new SplineCtrl(this);
                        m_pCCtrls->showPointTable(false);
                        m_pCCtrls->setEnabledClosedTE(false);
                        m_pCCtrls->setEnabledForceSym(false);
                        pCLayout->addWidget(m_pCCtrls);
                        pCLayout->addStretch();
                    }
                    pCFrame->setLayout(pCLayout);
                }
                QFrame *pTFrame = new QFrame;
                {
                    QVBoxLayout *pTLayout = new QVBoxLayout;
                    {
                        m_pTCtrls = new SplineCtrl(this);
                        m_pTCtrls->showPointTable(false);
                        m_pTCtrls->showBunchBox(false);
                        m_pTCtrls->setEnabledClosedTE(false);
                        m_pTCtrls->setEnabledForceSym(false);

                        pTLayout->addWidget(m_pTCtrls);
                        pTLayout->addStretch();
                    }
                    pTFrame->setLayout(pTLayout);
                }
                pSplineCtrlTab->addTab(pCFrame, "Camber spline");
                pSplineCtrlTab->addTab(pTFrame, "Thickness spline");
            }

            QHBoxLayout *pUndoRedoLayout = new QHBoxLayout;
            {
                m_ppbUndo = new QPushButton(QIcon(":/icons/OnUndo.png"), "Undo");
                m_ppbUndo->setToolTip("Ctrl+Z");
                m_ppbRedo = new QPushButton(QIcon(":/icons/OnRedo.png"), "Redo");
                m_ppbRedo->setToolTip("Ctrl+Y, Ctrl+Shift+Z");
                pUndoRedoLayout->addWidget(m_ppbUndo);
                pUndoRedoLayout->addWidget(m_ppbRedo);
            }

            m_ppbActionsMenuBtn = new QPushButton("Actions");
            {
                QMenu *pActionsMenu = new QMenu("Actions");
                {
                    QAction *pActReset      = new QAction("Reset spline",     this);
                    QAction *pActApproxFoil = new QAction("Approximate foil", this);
                    QAction *pActOverlay    = new QAction("Overlay foil",     this);

                    connect(pActReset,      SIGNAL(triggered(bool)), SLOT(onReset()));
                    connect(pActApproxFoil, SIGNAL(triggered(bool)), SLOT(onApproxFoil()));
                    connect(pActOverlay,    SIGNAL(triggered(bool)), m_pFoilWt, SLOT(onOverlayFoil()));

                    pActionsMenu->addAction(pActReset);
                    pActionsMenu->addAction(pActApproxFoil);
                    pActionsMenu->addAction(pActOverlay);
                }
                m_ppbActionsMenuBtn->setMenu(pActionsMenu);
            }

            pLeftLayout->addWidget(pSplineCtrlTab);
            pLeftLayout->addWidget(m_ppbActionsMenuBtn);
            pLeftLayout->addLayout(pUndoRedoLayout);
        }
        pLeftFrame->setLayout(pLeftLayout);
    }

    m_pHSplitter = new QSplitter;
    {
        m_pHSplitter->setChildrenCollapsible(false);
        m_pHSplitter->addWidget(pLeftFrame);
        m_pHSplitter->addWidget(m_pFoilWt);
        m_pHSplitter->setStretchFactor(0, 1);
        m_pHSplitter->setStretchFactor(1, 5);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(m_pHSplitter);
    setLayout(pMainLayout);
}


void FoilCamberDlg::connectSignals()
{
    connect(m_pHSplitter,  SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));

    connect(m_ppbUndo,   SIGNAL(clicked(bool)),          SLOT(onUndo()));
    connect(m_ppbRedo,   SIGNAL(clicked(bool)),          SLOT(onRedo()));
    connect(m_pFoilWt,   SIGNAL(mouseDragReleased()),    SLOT(onTakePicture()));
    connect(m_pFoilWt,   SIGNAL(mouseDragReleased()),    SLOT(onApply()));

    connect(m_pCCtrls,   SIGNAL(pointSelChanged()),      m_pFoilWt,  SLOT(update()));
    connect(m_pTCtrls,   SIGNAL(pointSelChanged()),      m_pFoilWt,  SLOT(update()));
    connect(m_pCCtrls,   SIGNAL(splineChanged()),        SLOT(onApply()));
    connect(m_pTCtrls,   SIGNAL(splineChanged()),        SLOT(onApply()));
}


void FoilCamberDlg::showEvent(QShowEvent *pEvent)
{
    FoilDlg::showEvent(pEvent);
    resizeEvent(nullptr);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    m_pButtonBox->setFocus();
}


void FoilCamberDlg::hideEvent(QHideEvent *pEvent)
{
    FoilDlg::hideEvent(pEvent);
    s_HSplitterSizes  = m_pHSplitter->saveState();
}


void FoilCamberDlg::resizeEvent(QResizeEvent *)
{
    int h = m_pFoilWt->height();
    int w = m_pFoilWt->width();


    QPoint pos2(w-m_pButtonBox->width()-5, h-m_pButtonBox->height()-5);
    m_pButtonBox->move(pos2);
}


void FoilCamberDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;
    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    switch (pEvent->key())
    {
        case Qt::Key_Y:
            if(bCtrl) onRedo();
            break;
        case Qt::Key_Z:
            if(bCtrl && bShift) onRedo();
            else if(bCtrl) onUndo();
            break;
        default:
            FoilDlg::keyPressEvent(pEvent);
            return;
    }
}


void FoilCamberDlg::onSplitterMoved()
{
    resizeEvent(nullptr);
}


void FoilCamberDlg::onUndo()
{
    if(m_StackPos>0)
    {
        m_StackPos--;
        setPicture();
        m_ppbUndo->setEnabled(m_StackPos>0);
        m_ppbRedo->setEnabled(m_StackPos<m_CStack.size()-1);
    }
    else
    {
        //nothing to restore
    }
}


void FoilCamberDlg::onRedo()
{
    if(m_StackPos<m_CStack.size()-1)
    {
        m_StackPos++;
        setPicture();
        m_ppbUndo->setEnabled(m_StackPos>0);
        m_ppbRedo->setEnabled(m_StackPos<m_CStack.size()-1);
    }

}



/**
  * Clears the stack starting at a given position.
  * @param the first stack element to remove
  */
void FoilCamberDlg::clearStack(int pos)
{
    for(int il=m_CStack.size()-1; il>pos; il--)
    {
        m_CStack.pop();
    }
    m_StackPos = m_CStack.size()-1;

    for(int il=m_TStack.size()-1; il>pos; il--)
    {
        m_TStack.pop();
    }
    m_StackPos = m_TStack.size()-1;
}


void FoilCamberDlg::onTakePicture()
{
    m_bModified = true;

    clearStack(m_StackPos);

    m_CStack.push(s_CSpline);
    m_StackPos = m_CStack.size()-1;
    m_TStack.push(s_TSpline);
    m_StackPos = m_TStack.size()-1;

    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_CStack.size()-1);
}


void FoilCamberDlg::setPicture()
{
    if(m_StackPos<0 || m_StackPos>=m_CStack.size()) return;

    s_CSpline.duplicate(m_CStack.at(m_StackPos));
    s_CSpline.updateSpline();
    s_CSpline.makeCurve();

    s_TSpline.duplicate(m_TStack.at(m_StackPos));
    s_TSpline.updateSpline();
    s_TSpline.makeCurve();

    onApply();
}


void FoilCamberDlg::onApproxFoil()
{
    SplineApproxDlg dlg(this);
    int degree      = s_TSpline.degree();
    int nCtrlPoints = s_TSpline.ctrlPointCount();
    dlg.initDialog(false, degree, nCtrlPoints);

    if(dlg.exec()!=QDialog::Accepted) return;

    degree = dlg.degree();
    nCtrlPoints = dlg.nCtrlPoints();

    QString foilname = dlg.selectedFoilName();

    if(!foilname.length()) return; // nothing to approximate

    Foil const *pRefFoil = Objects2d::foil(foilname.toStdString());
    if(!pRefFoil) return;

    onTakePicture();

    Q_ASSERT(pRefFoil->baseCbLine().size()==pRefFoil->thickness().size());
    int size = int(pRefFoil->baseCbLine().size());
    s_CSpline.approximate(degree, nCtrlPoints, pRefFoil->baseCbLine());

    std::vector<Node2d> points(size);

    for(int i=0; i<size; i++)
    {
        points[i].x = pRefFoil->baseCbLine().at(i).x;
        points[i].y = pRefFoil->baseCbLine().at(i).y + pRefFoil->thickness().at(i)/2.0;
    }
    s_TSpline.approximate(degree, nCtrlPoints, points);

    s_CSpline.updateSpline();
    s_CSpline.makeCurve();
    s_TSpline.updateSpline();
    s_TSpline.makeCurve();

    onApply();
    onTakePicture();
}


void FoilCamberDlg::makeFoilFromSplines()
{
    // make the difference between the two splines to deduce the thickness
    std::vector<double> thick;
    for(int io=0; io<s_CSpline.outputSize(); io++)
    {
        Vector2d const &cb = s_CSpline.outputPt(io);
        double xc = cb.x;
        double extrados = s_TSpline.getY(xc, true);
        double th = (extrados-cb.y)*2;
        thick.push_back(th);
    }

    m_pBufferFoil->setBaseTop(s_CSpline.outputPts()); // to resize and to set the x-values
    m_pBufferFoil->setBaseBot(s_CSpline.outputPts());

    m_pBufferFoil->setBaseCamberLine(s_CSpline.outputPts());
    m_pBufferFoil->setThicknessLine(thick);

    m_pBufferFoil->makeBaseFromCamberAndThickness();
    m_pBufferFoil->rebuildPointSequenceFromBase();
}



