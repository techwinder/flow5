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


#include <QKeyEvent>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include "foildlg.h"
#include <core/xflcore.h>
#include <core/displayoptions.h>
#include <api/objects2d.h>
#include <api/foil.h>
#include <interfaces/editors/foiledit/foilwt.h>
#include <interfaces/widgets/line/linemenu.h>



QByteArray FoilDlg::s_Geometry;

Grid FoilDlg::s_Grid;


FoilDlg::FoilDlg(QWidget *pParent) : XflDialog(pParent)
{
    setCursor(Qt::CrossCursor);
    setWindowFlag(Qt::WindowMinMaxButtonsHint);
    restoreGeometry(s_Geometry);

    s_Grid.showXMinGrid(true);
    s_Grid.showYMinGrid(0,true);

    m_pRefFoil = nullptr;
    m_pBufferFoil = new Foil; // geometry will be set and modified by derived classes
    m_pBufferFoil->setName(tr("Buffer foil").toStdString());

    m_bModified = false;

    m_Palette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    m_Palette.setColor(QPalette::Window,     DisplayOptions::backgroundColor());
    m_SliderPalette.setColor(QPalette::WindowText, DisplayOptions::textColor());
    m_SliderPalette.setColor(QPalette::Window, QColor(155,155,155,65)); // least worst solution for both light and dark modes

    makeCommonWidgets();
}


FoilDlg::~FoilDlg()
{
    if(m_pBufferFoil) delete m_pBufferFoil;
}


QSize FoilDlg::sizeHint() const
{
    return QSize(DisplayOptions::tableFontStruct().averageCharWidth()*205, DisplayOptions::tableFontStruct().height()*65);
}


void FoilDlg::initDialog(Foil *pFoil)
{
    m_pRefFoil = pFoil;
    m_pBufferFoil->setTheStyle(FoilWt::bufferFoilStyle());
    m_pBufferFoil->show();
    m_pBufferFoil->setFilled(FoilWt::isFilledBufferFoil());

    resetFoil();
    m_pBufferFoil->initGeometry();

    m_pFoilWt->setBufferFoil(m_pBufferFoil);
    m_pFoilWt->showCamberLines(false);
    m_pFoilWt->m_pBufferLineMenu->initMenu(m_pBufferFoil->theStyle());
}


void FoilDlg::resetFoil()
{
    FoilWt::setBufferFoilStyle(m_pBufferFoil->theStyle());
    if(m_pRefFoil) m_pBufferFoil->copy(m_pRefFoil,false);
    m_pBufferFoil->setTheStyle(FoilWt::bufferFoilStyle());
}


void FoilDlg::makeCommonWidgets()
{
    m_pFoilWt = new FoilWt(this);
    m_pFoilWt->setGrid(s_Grid);

    m_ppbMenuBtn = new QPushButton(tr("Display"));
    {
        QMenu *pMenuAction = new QMenu(tr("Menu"));
        {
            pMenuAction->addAction(m_pFoilWt->m_pShowCamberLines);
            pMenuAction->addAction(m_pFoilWt->m_pOverlayFoil);
        }
        m_ppbMenuBtn->setMenu(pMenuAction);
    }

    m_pButtonBox->setParent(m_pFoilWt);
    m_pButtonBox->setStandardButtons(QDialogButtonBox::Save  | QDialogButtonBox::Discard |
                                     QDialogButtonBox::Apply | QDialogButtonBox::Reset);
    {
        m_pButtonBox->setPalette(m_Palette);
        m_pButtonBox->setAutoFillBackground(false);

        QList<QAbstractButton *> buttons = m_pButtonBox->buttons();
        for(int i=0; i<buttons.size(); i++)
        {
            buttons[i]->setPalette(m_Palette);
            buttons[i]->setAutoFillBackground(false);
        }
        m_pButtonBox->setCursor(Qt::ArrowCursor);

        QPushButton *ppbSaveBtn = m_pButtonBox->button(QDialogButtonBox::Save);
        if(ppbSaveBtn)
        {
            ppbSaveBtn->setDefault(false);
            ppbSaveBtn->setAutoDefault(false);
        }
        QPushButton *ppbResetBtn = m_pButtonBox->button(QDialogButtonBox::Reset);
        if(ppbResetBtn)
        {
            ppbResetBtn->setDefault(false);
            ppbResetBtn->setAutoDefault(false);
        }
        QPushButton *ppbDiscardBtn = m_pButtonBox->button(QDialogButtonBox::Discard);
        if(ppbDiscardBtn)
        {
            ppbDiscardBtn->setDefault(false);
            ppbDiscardBtn->setAutoDefault(false);
        }
        QPushButton *ppbApplyBtn = m_pButtonBox->button(QDialogButtonBox::Apply);
        if(ppbApplyBtn)
        {
            ppbApplyBtn->setDefault(false);
            ppbApplyBtn->setAutoDefault(false);
        }

        m_pButtonBox->addButton(m_ppbMenuBtn, QDialogButtonBox::ActionRole);
    }
}

void FoilDlg::onButton(QAbstractButton*pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save)    == pButton) onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton) reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Apply)   == pButton) onApply();
    else if (m_pButtonBox->button(QDialogButtonBox::Reset)   == pButton) onReset();
}


void FoilDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
            }
            break;
        }
        default:
            pEvent->ignore();
            break;
    }
}


void FoilDlg::reject()
{
    if(m_bModified && xfl::bConfirmDiscard())
    {
        QString strong = tr("Discard the changes?");
        int Ans = QMessageBox::question(this, tr("Question"), strong,
                                        QMessageBox::Yes | QMessageBox::No);
        if (QMessageBox::Yes == Ans)
        {
            done(QDialog::Rejected);
            return;
        }
        else return;
    }

    done(QDialog::Rejected);
}


void FoilDlg::hideEvent(QHideEvent *pEvent)
{
    s_Geometry = saveGeometry();
    s_Grid = m_pFoilWt->grid();
    m_pFoilWt->setBufferFoilStyle(m_pBufferFoil->theStyle());

    pEvent->accept();
}


void FoilDlg::resizeEvent(QResizeEvent *)
{
    int h = m_pFoilWt->height();
    int w = m_pFoilWt->width();

    QPoint pos2(w-m_pButtonBox->width()-5, h-m_pButtonBox->height()-5);
    m_pButtonBox->move(pos2);
}


void FoilDlg::onReset()
{
    resetFoil();


    m_bModified = false;
    update();
}


void FoilDlg::onChanged()
{
    onApply();
}


void FoilDlg::onOK()
{
    readParams();

    onApply();

    m_pBufferFoil->initGeometry();

    accept();
}


void FoilDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("FoilDlg");
    {
        s_Geometry = settings.value("WindowGeom", QByteArray()).toByteArray();

        xfl::loadLineSettings(settings, FoilWt::bufferFoilStyle(), "BufferFoil");
        FoilWt::setFilledBufferFoil(settings.value("FillBufferFoil", false).toBool());

        s_Grid.loadSettings(settings);
    }
    settings.endGroup();
}


void FoilDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("FoilDlg");
    {
        xfl::saveLineSettings(settings, FoilWt::bufferFoilStyle(), "BufferFoil");

        settings.setValue("FillBufferFoil", FoilWt::isFilledBufferFoil());
        settings.setValue("WindowGeom", s_Geometry);

        s_Grid.saveSettings(settings);
    }
    settings.endGroup();
}

