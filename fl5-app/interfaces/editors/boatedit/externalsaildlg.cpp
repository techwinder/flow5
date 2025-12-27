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


#include <QGridLayout>


#include "externalsaildlg.h"




#include <api/externalsail.h>
#include <api/units.h>
#include <interfaces/editors/rotatedlg.h>
#include <interfaces/editors/scaledlg.h>
#include <interfaces/editors/translatedlg.h>
#include <interfaces/opengl/controls/gl3dgeomcontrols.h>
#include <interfaces/opengl/fl5views/gl3dsailview.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/plaintextoutput.h>

ExternalSailDlg::ExternalSailDlg(QWidget *pParent) : SailDlg(pParent)
{
    makeCommonWt();
}


void ExternalSailDlg::makeCommonWt()
{
    m_pCornersBox = new QGroupBox("Corner points");
    {
        QGridLayout *pCornersLayout = new QGridLayout;
        {
            QLabel *plabHelp = new QLabel("Pick");
            m_ppbHead = new QPushButton("Head");
            m_ppbPeak = new QPushButton("Peak");
            m_ppbClew = new QPushButton("Clew");
            m_ppbTack = new QPushButton("Tack");
            m_ppbHead->setCheckable(true);
            m_ppbTack->setCheckable(true);
            m_ppbClew->setCheckable(true);
            m_ppbPeak->setCheckable(true);
            pCornersLayout->addWidget(plabHelp,  1, 1, 1, 4);
            pCornersLayout->addWidget(m_ppbPeak, 2, 2);
            pCornersLayout->addWidget(m_ppbHead, 2, 4);
            pCornersLayout->addWidget(m_ppbClew, 3, 2);
            pCornersLayout->addWidget(m_ppbTack, 3, 4);
            pCornersLayout->setColumnStretch(3,1);
        }
        m_pCornersBox->setLayout(pCornersLayout);
    }

    m_pFlow5Link = new QLabel;
    m_pFlow5Link->setText("<a href=https://flow5.tech/docs/flow5_doc/Sailboats/External_Sails.html#StepByStep>https://flow5.tech/docs/flow5_doc/Sailboats/External_Sails.html</a>");
    m_pFlow5Link->setOpenExternalLinks(true);
    m_pFlow5Link->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
}


void ExternalSailDlg::connectSignals()
{
    connectBaseSignals();

    connect(m_prbThin,              SIGNAL(clicked(bool)),        SLOT(onThinSurface()));
    connect(m_prbThick,             SIGNAL(clicked(bool)),        SLOT(onThinSurface()));
    connect(m_ppbClew,              SIGNAL(clicked(bool)),        SLOT(onCornerPoint(bool)));
    connect(m_ppbTack,              SIGNAL(clicked(bool)),        SLOT(onCornerPoint(bool)));
    connect(m_ppbHead,              SIGNAL(clicked(bool)),        SLOT(onCornerPoint(bool)));
    connect(m_ppbPeak,              SIGNAL(clicked(bool)),        SLOT(onCornerPoint(bool)));
    connect(m_pglSailView,          SIGNAL(pickedNode(Vector3d)), SLOT(onPickedNode(Vector3d)));

    connect(m_pRotate,              SIGNAL(triggered(bool)),      SLOT(onRotateSail()));

    connect(m_pglSailControls->m_ptbDistance, SIGNAL(clicked()), SLOT(onNodeDistance()));
}


void ExternalSailDlg::showEvent(QShowEvent *pEvent)
{
    SailDlg::showEvent(pEvent);
    m_pfeTEAngle->setValue(s_TEMaxAngle);
    m_pchGuessOpposite->setChecked(s_bGuessOpposite);
}


void ExternalSailDlg::hideEvent(QHideEvent *pEvent)
{
    SailDlg::hideEvent(pEvent);
    s_TEMaxAngle = m_pfeTEAngle->value();
    s_bGuessOpposite = m_pchGuessOpposite->isChecked();
}


void ExternalSailDlg::keyPressEvent(QKeyEvent *pEvent)
{
/*    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;*/
/*    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;*/
/*    bool bAlt = false;
    if(pEvent->modifiers() & Qt::AltModifier)   bAlt =true;*/

    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
/*            if(m_pgl3dControls->getDistance())
            {
                m_pgl3dControls->m_ptbDistance->setChecked(false);
                m_pgl3dControls->m_ptbDistance->defaultAction()->setChecked(false);
                m_pgl3dControls->clearMeasure();
            }
            else  if(!deselectButtons())                reject(); */
            if(!deselectButtons())                reject();
            break;
        }
        default:
            SailDlg::keyPressEvent(pEvent);
            break;
    }
}


void ExternalSailDlg::onCornerPoint(bool bChecked)
{
    m_ppbTEBotMid->setChecked(false);
    m_pglSailControls->stopDistance();
    m_pglSailView->selectPanels(false);
    m_pglSailView->clearMeasure();

    QPushButton *pCornerBtn = qobject_cast<QPushButton*>(sender());
/*    if(m_ppbClew==pCornerBtn)
    {
        m_ppbHead->setChecked(false);
        m_ppbTack->setChecked(false);
        m_ppbPeak->setChecked(false);
    }
    else if(m_ppbHead==pCornerBtn)
    {
        m_ppbClew->setChecked(false);
        m_ppbTack->setChecked(false);
        m_ppbPeak->setChecked(false);
    }
    else if(m_ppbTack==pCornerBtn)
    {
        m_ppbClew->setChecked(false);
        m_ppbHead->setChecked(false);
        m_ppbPeak->setChecked(false);
    }
    else if(m_ppbPeak==pCornerBtn)
    {
        m_ppbClew->setChecked(false);
        m_ppbHead->setChecked(false);
        m_ppbTack->setChecked(false);
    }*/

    m_ppbClew->setChecked(pCornerBtn==m_ppbClew);
    m_ppbHead->setChecked(pCornerBtn==m_ppbHead);
    m_ppbTack->setChecked(pCornerBtn==m_ppbTack);
    m_ppbPeak->setChecked(pCornerBtn==m_ppbPeak);

    m_pglSailView->showCornerPoints(bChecked);
    m_pglSailView->stopPicking();
    if(m_pSail->isStlSail())
        m_pglSailView->setPicking(xfl::TRIANGLENODE);
    else if(m_pSail->isOccSail())
        m_pglSailView->setPicking(xfl::VERTEX);
    m_pglSailView->update();
}


bool ExternalSailDlg::deselectButtons()
{
    bool bChecked = false;
    bChecked |= m_ppbTEBotMid->isChecked();
    bChecked |= m_ppbClew->isChecked();
    bChecked |= m_ppbHead->isChecked();
    bChecked |= m_ppbTack->isChecked();
    bChecked |= m_ppbPeak->isChecked();
    bChecked |= m_pglSailControls->getDistance();

    m_ppbClew->setChecked(false);
    m_ppbTack->setChecked(false);
    m_ppbHead->setChecked(false);
    m_ppbPeak->setChecked(false);
    m_pglSailView->showCornerPoints(m_pglSailControls->bShowCornerPts());

    m_pglSailControls->stopDistance();
    m_pglSailView->selectPanels(false);
    m_pglSailView->stopPicking();
    m_pglSailView->clearMeasure();

    return bChecked;
}


void ExternalSailDlg::setControls()
{
    SailDlg::setControls();

    if(!m_pSail->isThinSurface())
    {
        m_ppbTETop->setEnabled(true);
        m_ppbTEBotMid->setText("Bottom panels");
        m_pchGuessOpposite->setEnabled(true);
        m_pfeTEAngle->setEnabled(true);
    }
    else
    {
        m_ppbTETop->setEnabled(false);
        m_ppbTEBotMid->setText("Mid. panels");
        m_pchGuessOpposite->setEnabled(false);
        m_pfeTEAngle->setEnabled(false);
    }
}


void ExternalSailDlg::onPickedNode(Vector3d I)
{
    if(m_ppbClew->isChecked())
    {
        m_pSail->m_Clew = I;
        m_pglSailView->resetPickedNodes(); // no need for a second node
    }
    else if(m_ppbHead->isChecked())
    {
        m_pSail->m_Head = I;
        m_pglSailView->resetPickedNodes();
    }
    else if(m_ppbTack->isChecked())
    {
        m_pSail->m_Tack = I;
        m_pglSailView->resetPickedNodes();
    }
    else if(m_ppbPeak->isChecked())
    {
        m_pSail->m_Peak = I;
        m_pglSailView->resetPickedNodes();
    }
    m_pglSailView->resetglSail();
    m_pglSailView->update();
    m_bChanged = true;
}


void ExternalSailDlg::onTranslateSail()
{
    TranslateDlg dlg(this);
    if(dlg.exec()!=QDialog::Accepted) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
   // ExternalSail *pExtSail = dynamic_cast<ExternalSail*>(m_pSail);
    m_pSail->translate(dlg.translationVector());

    m_pglSailView->resetglSail();
    updateView();

    m_bChanged = true;
    QApplication::restoreOverrideCursor();
}


void ExternalSailDlg::onRotateSail()
{
    RotateDlg dlg(this);
    if(dlg.exec()!=QDialog::Accepted)
    {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    Vector3d O, axis;
    if      (dlg.axis()==0) axis.set(1.0,0.0,0.0);
    else if (dlg.axis()==1) axis.set(0.0,1.0,0.0);
    else if (dlg.axis()==2) axis.set(0.0,0.0,1.0);;

    ExternalSail *pExtSail = dynamic_cast<ExternalSail*>(m_pSail);
    pExtSail->rotate(O, axis, dlg.angle());

    m_pglSailView->resetglSail();
    updateView();

    m_bChanged = true;

    QApplication::restoreOverrideCursor();
}


