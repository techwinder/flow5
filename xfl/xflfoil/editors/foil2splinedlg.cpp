/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#include <QVBoxLayout>
#include <QMessageBox>

#include "foil2splinedlg.h"
#include <xflfoil/splinectrl/splinefoilctrls.h>
#include <xflfoil/editors/foilwt.h>

#include <xflfoil/objects2d/objects2d.h>
#include <xflfoil/objects2d/foil.h>
#include <xflwidgets/customwts/floatedit.h>
#include <xflwidgets/customwts/intedit.h>



QByteArray Foil2SplineDlg::s_HSplitterSizes;
SplineFoil Foil2SplineDlg::s_SF;


Foil2SplineDlg::Foil2SplineDlg(QWidget *pParent) : FoilDlg(pParent)
{
    setWindowTitle("Foil from 2 splines");
    setupLayout();
    connectSignals();

}


void Foil2SplineDlg::initDialog(Foil *pFoil)
{
    FoilDlg::initDialog(pFoil);
    m_pFoilWt->setBufferFoil(nullptr);
    m_pBufferFoil->setVisible(false);
    m_pBufferFoil->setName("Splined foil");

    s_SF.setVisible(true);

    m_pSFCtrls->initDialog(&s_SF);
    m_pFoilWt->setSplineFoil(&s_SF);

    if(s_SF.extrados().ctrlPointCount()==0 || s_SF.intrados().ctrlPointCount()==0)
        onNewSpline();

    m_SFStack.clear();
    m_SFStack.push(s_SF);
    m_StackPos = 0;

    m_CStackPos = 0;

    if(!m_pRefFoil)  onApply();
}


void Foil2SplineDlg::onNewSpline()
{
    bool bQuestion = false;

    if(m_SFStack.size()>1) bQuestion = true;

    if(bQuestion)
    {
        int resp = QMessageBox::question(this, "Question", "Discard the changes?",  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(resp != QMessageBox::Yes) return;
    }

    s_SF.initSplineFoil();

    onTakePicture();
    update();
}


void Foil2SplineDlg::onApply()
{
    s_SF.exportToFoil(m_pBufferFoil);
    m_pBufferFoil->initGeometry();

    m_pFoilWt->update();
}


void Foil2SplineDlg::onReset()
{
    onNewSpline();
}


void Foil2SplineDlg::setupLayout()
{
    QFrame *pLeftFrame = new QFrame;
    {
        pLeftFrame->setCursor(Qt::ArrowCursor);
        QVBoxLayout *pLeftLayout = new QVBoxLayout;
        {
            m_pSFCtrls = new SplineFoilCtrls(this);

            QHBoxLayout *pUndoRedoLayout = new QHBoxLayout;
            {
                m_ppbUndo = new QPushButton(QIcon(":/icons/OnUndo.png"), "Undo");
                m_ppbUndo->setToolTip("Ctrl+Z");
                m_ppbRedo = new QPushButton(QIcon(":/icons/OnRedo.png"), "Redo");
                m_ppbRedo->setToolTip("Ctrl+Y, Ctrl+Shift+Z");
                pUndoRedoLayout->addWidget(m_ppbUndo);
                pUndoRedoLayout->addWidget(m_ppbRedo);
            }

            pLeftLayout->addWidget(m_pSFCtrls);
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


void Foil2SplineDlg::connectSignals()
{
    connect(m_pHSplitter,      SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));

    connect(m_ppbUndo,         SIGNAL(clicked(bool)),          SLOT(onUndo()));
    connect(m_ppbRedo,         SIGNAL(clicked(bool)),          SLOT(onRedo()));

    connect(m_pFoilWt,         SIGNAL(mouseDragReleased()),    SLOT(onTakePicture()));
    connect(m_pFoilWt,         SIGNAL(objectModified()),       SLOT(onTakePicture()));

    connect(m_pSFCtrls,        SIGNAL(splineFoilChanged()),    SLOT(update()));
}


void Foil2SplineDlg::showEvent(QShowEvent *pEvent)
{
    FoilDlg::showEvent(pEvent);
    resizeEvent(nullptr);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    m_pButtonBox->setFocus();
}


void Foil2SplineDlg::hideEvent(QHideEvent *pEvent)
{
    FoilDlg::hideEvent(pEvent);
    s_HSplitterSizes  = m_pHSplitter->saveState();
}


void Foil2SplineDlg::resizeEvent(QResizeEvent *)
{
    int h = m_pFoilWt->height();
    int w = m_pFoilWt->width();

    QPoint pos2(w-m_pButtonBox->width()-5, h-m_pButtonBox->height()-5);
    m_pButtonBox->move(pos2);
}


void Foil2SplineDlg::keyPressEvent(QKeyEvent *pEvent)
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


void Foil2SplineDlg::onSplitterMoved()
{
    resizeEvent(nullptr);
}


void Foil2SplineDlg::onUndo()
{
    if(m_StackPos>0)
    {
        m_StackPos--;
        setPicture();
        m_ppbUndo->setEnabled(m_StackPos>0);
        m_ppbRedo->setEnabled(m_StackPos<m_SFStack.size()-1);
    }
    else
    {
        //nothing to restore
    }
//    m_pSplineCtrl->fillPointModel();
}


void Foil2SplineDlg::onRedo()
{
    if(m_StackPos<m_SFStack.size()-1)
    {
        m_StackPos++;
        setPicture();
        m_ppbUndo->setEnabled(m_StackPos>0);
        m_ppbRedo->setEnabled(m_StackPos<m_SFStack.size()-1);
    }
//    m_pSplineCtrl->fillPointModel();
}



/**
  * Clears the stack starting at a given position.
  * @param the first stack element to remove
  */
void Foil2SplineDlg::clearStack(int pos)
{
    for(int il=m_SFStack.size()-1; il>pos; il--)
    {
        m_SFStack.pop();
    }
    m_StackPos = m_SFStack.size()-1;
}


/**
 * Copies the current SplineFoil object to a new SplineFoil object and pushes it on the stack.
 */
void Foil2SplineDlg::onTakePicture()
{
    m_bModified = true;

    clearStack(m_StackPos);
    m_SFStack.push(s_SF);
    m_StackPos = m_SFStack.size()-1;

    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_SFStack.size()-1);

    m_pSFCtrls->fillPointLists();
}


/**
 * Restores the Spline definition from the current position in the stack.
 */
void Foil2SplineDlg::setPicture()
{
    if(m_StackPos<0 || m_StackPos>=m_SFStack.size()) return;
    s_SF.copy(&m_SFStack.at(m_StackPos));
    s_SF.makeSplineFoil();

    update();
}



