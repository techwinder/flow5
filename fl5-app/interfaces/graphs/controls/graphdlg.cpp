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


#include <QFontDialog>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHeaderView>

#include <interfaces/graphs/controls/graphdlg.h>

#include <interfaces/graphs/graph/graph.h>
#include <interfaces/widgets/customwts/floatedit.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/color/colorbtn.h>
#include <interfaces/widgets/color/textclrbtn.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/customwts/cptableview.h>
#include <interfaces/widgets/customwts/actionitemmodel.h>
#include <interfaces/widgets/customwts/xfldelegate.h>
#include <api/linestyle.h>

int GraphDlg::s_iActivePage = 0;
QByteArray GraphDlg::s_Geometry;


GraphDlg::GraphDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle("Graph settings");

    m_pParent = pParent;

    m_bCurveStylePage = false;

    m_pGraph   = nullptr;
    m_bApplied = true;
    m_bVariableChanged = false;

    m_XSel = 0;
    m_YSel[0] = 1;
    m_YSel[1] = 2;

    m_pTitleFont = m_pLabelFont = nullptr;

    setupLayout();
    connectSignals();
}


GraphDlg::~GraphDlg()
{
}


void GraphDlg::connectSignals()
{
    connect(m_ptcbTitleClr,  SIGNAL(clickedTB()),  SLOT(onTitleColor()));
    connect(m_ptcbLabelClr,  SIGNAL(clickedTB()),  SLOT(onLabelColor()));
    connect(m_ptcbLegendClr, SIGNAL(clickedTB()),  SLOT(onLegendColor()));

    connect(m_ppbTitleButton,  SIGNAL(clicked()),  SLOT(onTitleFont()));
    connect(m_ppbLabelButton,  SIGNAL(clicked()),  SLOT(onLabelFont()));
    connect(m_ppbLegendButton, SIGNAL(clicked()),  SLOT(onLegendFont()));

    connect(m_pchXAuto, SIGNAL(clicked()), SLOT(onAutoX()));
    connect(m_plwXSel, SIGNAL(itemSelectionChanged()), SLOT(onVariableChanged()));
    connect(m_plwXSel, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(onOK()));
    for(int iy=0; iy<2; iy++)
    {
        connect(m_pchYAuto[iy],     SIGNAL(clicked()), SLOT(onAutoY()));
        connect(m_pchYInverted[iy], SIGNAL(clicked()), SLOT(onYInverted()));
        connect(m_plwYSel[iy],      SIGNAL(itemSelectionChanged()), SLOT(onVariableChanged()));
        connect(m_plwYSel[iy],      SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(onOK()));
    }

    connect(m_plbXAxisStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onXAxisStyle()));

    connect(m_pchXMajGridShow, SIGNAL(clicked(bool)), SLOT(onXMajGridShow()));
    connect(m_pchXMinGridShow, SIGNAL(clicked(bool)), SLOT(onXMinGridShow()));
    connect(m_plbXMajGridStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onXMajGridStyle()));
    connect(m_plbXMinGridStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onXMinGridStyle()));

    for(int iy=0; iy<2; iy++)
    {
        connect(m_plbYAxisStyle[iy], SIGNAL(clickedLB(LineStyle)), SLOT(onYAxisStyle()));

        connect(m_plabYMajGridShow[iy], SIGNAL(clicked(bool)), SLOT(onYMajGridShow()));
        connect(m_pchMinGridShow[iy], SIGNAL(stateChanged(int)), SLOT(onYMinGridShow()));
        connect(m_plbYMajGridStyle[iy], SIGNAL(clickedLB(LineStyle)), SLOT(onYMajGridStyle()));
        connect(m_plbYMinGridStyle[iy], SIGNAL(clickedLB(LineStyle)), SLOT(onYMinGridStyle()));
    }

    connect(m_pchGraphBorder, SIGNAL(stateChanged(int)), SLOT(onGraphBorder(int)));
    connect(m_pcobGraphBack, SIGNAL(clicked()), SLOT(onGraphBackColor()));

    connect(m_plbBorderStyle, SIGNAL(clickedLB(LineStyle)), SLOT(onBorderStyle(LineStyle)));

    connect(m_pchRightAxis, SIGNAL(clicked(bool)), SLOT(onRightAxis(bool)));

    connect(m_pchShowLegend, SIGNAL(toggled(bool)), SLOT(onShowLegend(bool)));

    connect(m_pcpCurveTable, SIGNAL(clicked(QModelIndex)), SLOT(onCurveTableClicked(QModelIndex)));
}


void GraphDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::RestoreDefaults) == pButton) onRestoreParams();
    else if (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)              onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)          reject();
    //    else if (m_pButtonBox->button(QDialogButtonBox::Apply) == pButton)           onApply();
}


void GraphDlg::fillVariableList()
{
    m_plwXSel->clear();

    m_plwYSel[0]->clear();
    m_plwYSel[1]->clear();
    //foil polar graph variables
    for(int iVar=0; iVar<m_pGraph->XVariableList().size(); iVar++)
    {
        m_plwXSel->addItem(m_pGraph->XVariableList().at(iVar));
    }
    for(int iVar=0; iVar<m_pGraph->YVariableList().size(); iVar++)
    {
        m_plwYSel[0]->addItem(m_pGraph->YVariableList().at(iVar));
        m_plwYSel[1]->addItem(m_pGraph->YVariableList().at(iVar));
    }
    m_plwXSel->adjustSize();
    m_plwYSel[0]->adjustSize();
    m_plwYSel[1]->adjustSize();
}


void GraphDlg::reject()
{
    m_pGraph->copySettings(m_SaveGraph);
    done(QDialog::Rejected);
}


void GraphDlg::resizeTableColumns()
{
    double w = double(m_pcpCurveTable->width());
    int w15 = int(double(w)*0.95/double(4));

    m_pcpCurveTable->setColumnWidth(0,3*w15);
    m_pcpCurveTable->setColumnWidth(1,w15);

    m_pcpCurveTable->resizeRowsToContents();

}


void GraphDlg::showEvent(QShowEvent *pEvent)
{
    QDialog::showEvent(pEvent);
    m_pTabWidget->setCurrentIndex(s_iActivePage);
    restoreGeometry(s_Geometry);
    resizeTableColumns();
}


void GraphDlg::hideEvent(QHideEvent *pEvent)
{
    QDialog::hideEvent(pEvent);
    s_Geometry = saveGeometry();
}


void GraphDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            m_pButtonBox->button(QDialogButtonBox::Ok)->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            pEvent->ignore();
    }
}


void GraphDlg::onActivePage(int index)
{
    s_iActivePage = index;
    //    m_pButtonBox->button(QDialogButtonBox::Apply)->setEnabled(m_pTabWidget->currentIndex()!=0);
    if(index==5)
    {
        resizeTableColumns();
    }
}


void GraphDlg::onRightAxis(bool bChecked)
{
    m_pGraph->showRightAxis(bChecked);

    m_plwYSel[1]->setEnabled(bChecked);
    m_pchYAuto[1]->setEnabled(bChecked);
    m_pchYInverted[1]->setEnabled(bChecked);
    m_pdeYMin[1]->setEnabled(bChecked);
    m_pdeYMax[1]->setEnabled(bChecked);
    m_pdeYUnit[1]->setEnabled(bChecked);
    m_pchYLog[1]->setEnabled(bChecked);

    m_plbYAxisStyle[1]->setEnabled(bChecked);
    m_plabYMajGridShow[1]->setEnabled(bChecked);
    m_pchMinGridShow[1]->setEnabled(bChecked);
    m_plbYMajGridStyle[1]->setEnabled(bChecked);
    m_plbYMinGridStyle[1]->setEnabled(bChecked);

}


void GraphDlg::onAutoX()
{
    bool bAuto = m_pchXAuto->checkState() == Qt::Checked;
    m_pdeXMin->setEnabled(!bAuto);
    m_pdeXMax->setEnabled(!bAuto);
    m_pdeXUnit->setEnabled(!bAuto);
    setApplied(false);
}


void GraphDlg::onAutoY()
{
    QCheckBox *pAutoY = dynamic_cast<QCheckBox*>(sender());
    int iy=0;
    if(pAutoY==m_pchYAuto[1]) iy=1;

    bool bAuto = m_pchYAuto[iy]->checkState() == Qt::Checked;
    m_pdeYMin[iy]->setEnabled(!bAuto);
    m_pdeYMax[iy]->setEnabled(!bAuto);
    m_pdeYUnit[iy]->setEnabled(!bAuto);
    setApplied(false);
}

void GraphDlg::onXAxisStyle()
{
    LineMenu lineMenu(this, false);
    lineMenu.initMenu(m_pGraph->axisStyle(AXIS::XAXIS));
    lineMenu.exec(QCursor::pos());
    LineStyle ls = lineMenu.theStyle();
    m_pGraph->setAxisStyle(AXIS::XAXIS, ls);
    m_plbXAxisStyle->setTheStyle(ls);
}


void GraphDlg::onYAxisStyle()
{
    LineBtn *pMajY = dynamic_cast<LineBtn*>(sender());
    int iy=0;
    AXIS::enumAxis yaxis = AXIS::LEFTYAXIS;
    if     (pMajY==m_plbYAxisStyle[0])
    {
        iy=0;
        yaxis = AXIS::LEFTYAXIS;
    }
    else if(pMajY==m_plbYAxisStyle[1])
    {
        iy=1;
        yaxis = AXIS::RIGHTYAXIS;
    }
    else return;

    LineMenu lineMenu(this, false);
    lineMenu.initMenu(m_pGraph->axisStyle(yaxis));
    lineMenu.exec(QCursor::pos());
    LineStyle ls = lineMenu.theStyle();
    m_pGraph->setAxisStyle(yaxis, ls);
    m_plbYAxisStyle[iy]->setTheStyle(ls);
}


void GraphDlg::onBorderStyle(LineStyle ls)
{
    LineMenu lineMenu(nullptr);
    lineMenu.showPointStyle(false);
    lineMenu.initMenu(m_pGraph->borderStyle(), m_pGraph->borderWidth(), m_pGraph->borderColor(), Line::NOSYMBOL);
    lineMenu.exec(QCursor::pos());
    ls = lineMenu.theStyle();
    m_pGraph->setBorderStyle(ls.m_Stipple);
    m_pGraph->setBorderWidth(ls.m_Width);
    m_pGraph->setBorderColor(ls.m_Color);
    m_plbBorderStyle->setStipple(ls.m_Stipple);
    m_plbBorderStyle->setWidth(ls.m_Width);
    m_plbBorderStyle->setBtnColor(ls.m_Color);
    setApplied(false);
}


void GraphDlg::onGraphBorder(int state)
{
    bool bShow = (state==Qt::Checked);
    m_pGraph->setBorder(bShow);
    setApplied(false);
}


void GraphDlg::onGraphBackColor()
{
    QColor BkColor = m_pGraph->backgroundColor();
    BkColor = QColorDialog::getColor(BkColor, this, "Background", QColorDialog::ShowAlphaChannel);
    if(BkColor.isValid())
        m_pGraph->setBkColor(BkColor);

    m_pcobGraphBack->setColor(m_pGraph->backgroundColor());
    setButtonColors();
    setApplied(false);
}


void GraphDlg::onLabelColor()
{
    QColor color = m_pGraph->labelColor();
    color = QColorDialog::getColor(color, this, "Labels", QColorDialog::ShowAlphaChannel);
    if(color.isValid())
    {
        m_pGraph->setLabelColor(color);
        m_ptcbLabelClr->setTextColor(color);

        setApplied(false);
    }
    update();
}


void GraphDlg::onLabelFont()
{
    QFontDialog::FontDialogOptions dialogoptions;

    bool bOk = false;
    QFont font = QFontDialog::getFont(&bOk, m_pGraph->labelFont(), this, "Labels", dialogoptions);
    font.setStyleHint(QFont::SansSerif);
    if (bOk)
    {
        m_ppbLabelButton->setFont(font);
        m_ppbLabelButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
        m_ptcbLabelClr->setFont(font);
        m_pGraph->setLabelFont(font);
        setApplied(false);
    }
}


void GraphDlg::onLegendColor()
{
    QColor color = m_pGraph->legendColor();
    color = QColorDialog::getColor(color, this, "Legends", QColorDialog::ShowAlphaChannel);
    if(color.isValid())
    {
        m_pGraph->setLegendColor(color);
        m_ptcbLegendClr->setTextColor(color);

        setApplied(false);
    }
    update();
}


void GraphDlg::onLegendFont()
{
    QFontDialog::FontDialogOptions dialogoptions;

    bool bOk = false;
    QFont font = QFontDialog::getFont(&bOk, m_pGraph->legendFont(), this, "Legend", dialogoptions);
    font.setStyleHint(QFont::SansSerif);
    if (bOk)
    {
        m_ppbLegendButton->setFont(font);
        m_ppbLegendButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
        m_ptcbLegendClr->setFont(font);
        m_pGraph->setLegendFont(font);
        setApplied(false);
    }
}


void GraphDlg::onOK()
{
    applyChanges();

    m_XSel = m_plwXSel->currentRow();
    m_YSel[0] = m_plwYSel[0]->currentRow();
    m_YSel[1] = m_plwYSel[1]->currentRow();

    m_pGraph->setVariables(m_XSel, m_YSel[0], m_YSel[1]);

    accept();
}


void GraphDlg::applyChanges()
{
    m_pGraph->setAutoX(m_pchXAuto->isChecked());
    m_pGraph->setXMin(m_pdeXMin->value());
    m_pGraph->setXMax(m_pdeXMax->value());
    m_pGraph->setXLnScale(m_pchXLog->isChecked());

    if     (m_prbResetting->isChecked()) m_pGraph->setScaleType(GRAPH::RESETTING);
    else if(m_prbExpanding->isChecked()) m_pGraph->setScaleType(GRAPH::EXPANDING);

    m_pGraph->setAutoY(0, m_pchYAuto[0]->isChecked());
    for(int iy=0; iy<2; iy++)
    {
        m_pGraph->setYMin(iy, m_pdeYMin[iy]->value());
        m_pGraph->setYMax(iy, m_pdeYMax[iy]->value());
        m_pGraph->setYUnit(iy, m_pdeYUnit[iy]->value());
        m_pGraph->setYLnScale(iy, m_pchYLog[iy]->isChecked());
    }

    m_pGraph->setAutoXMinUnit(true);
    m_pGraph->setAutoYMinUnit(true);

    m_pGraph->setLeftMargin(m_pieLMargin->value());
    m_pGraph->setRightMargin(m_pieRMargin->value());
    m_pGraph->setTopMargin(m_pieTMargin->value());
    m_pGraph->setBotMargin(m_pieBMargin->value());

    m_pGraph->setLegendVisible(m_pchShowLegend->isChecked());
    if     (m_prbLeft->isChecked())        m_pGraph->setLegendPosition(Qt::AlignVCenter | Qt::AlignLeft);
    else if(m_prbRight->isChecked())       m_pGraph->setLegendPosition(Qt::AlignVCenter | Qt::AlignRight);
    else if(m_prbTop->isChecked())         m_pGraph->setLegendPosition(Qt::AlignHCenter | Qt::AlignTop);
    else if(m_prbTopLeft->isChecked())     m_pGraph->setLegendPosition(Qt::AlignLeft    | Qt::AlignTop);
    else if(m_prbTopRight->isChecked())    m_pGraph->setLegendPosition(Qt::AlignRight   | Qt::AlignTop);
    else if(m_prbBottom->isChecked())      m_pGraph->setLegendPosition(Qt::AlignHCenter | Qt::AlignBottom);
    else if(m_prbBottomLeft->isChecked())  m_pGraph->setLegendPosition(Qt::AlignLeft    | Qt::AlignBottom);
    else if(m_prbBottomRight->isChecked()) m_pGraph->setLegendPosition(Qt::AlignRight   | Qt::AlignBottom);

    if(m_bCurveStylePage)
    {
        for(int i=0; i<m_pCurveModel->rowCount(); i++)
        {
            Curve *pCurve = m_pGraph->curve(i);
            if(!pCurve) continue;
            QModelIndex ind = m_pCurveModel->index(i, 0);
//            QString name = m_pCurveModel->data(ind, Qt::DisplayRole).toString();
            QString name2 = m_pCurveModel->data(ind, Qt::EditRole).toString(); //same
            pCurve->setName(name2);
        }
    }
}


void GraphDlg::onApply()
{
    applyChanges();

    m_pGraph->setYInverted(0, m_pchYInverted[0]->isChecked());
    m_pGraph->setYInverted(1, m_pchYInverted[0]->isChecked());

    if(m_pParent) m_pParent->update();
    setApplied(true);
}



void GraphDlg::onRestoreParams()
{
    m_pGraph->copySettings(m_SaveGraph);

    setControls();
    setApplied(true);

    if(m_pParent) m_pParent->update();
}


void GraphDlg::onTitleColor()
{
    QColor color = m_pGraph->titleColor();
    color = QColorDialog::getColor(color, this, "Title", QColorDialog::ShowAlphaChannel);
    if(color.isValid())
    {
        m_pGraph->setTitleColor(color);
        m_ptcbTitleClr->setTextColor(color);
        setApplied(false);
    }
    update();
}


void GraphDlg::onTitleFont()
{
    QFontDialog::FontDialogOptions dialogoptions;


    bool bOk=false;
    QFont TitleFont("Arial");
    TitleFont = m_pGraph->titleFont();

    QFont font = QFontDialog::getFont(&bOk, TitleFont, this, "Axis titles", dialogoptions);

    if (bOk)
    {
        m_ppbTitleButton->setFont(font);
        m_ppbTitleButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
        m_ptcbTitleClr->setFont(font);
        m_pGraph->setTitleFont(font);
        setApplied(false);
    }
}


void GraphDlg::onVariableChanged()
{
    m_bVariableChanged = true;
}


void GraphDlg::onMargin()
{
    m_pGraph->setLeftMargin(m_pieLMargin->value());
    m_pGraph->setRightMargin(m_pieRMargin->value());
    m_pGraph->setTopMargin(m_pieTMargin->value());
    m_pGraph->setBotMargin(m_pieBMargin->value());
}


void GraphDlg::onXMajGridStyle()
{
    bool bShow=false;
    LineStyle ls;
    m_pGraph->xMajGrid(bShow,ls);
    LineMenu lineMenu(this);
    lineMenu.showPointStyle(false);
    lineMenu.initMenu(ls);
    lineMenu.exec(QCursor::pos());
    ls = lineMenu.theStyle();
    m_pGraph->setXMajGrid(bShow, ls);
    m_plbXMajGridStyle->setStipple(ls.m_Stipple);
    m_plbXMajGridStyle->setWidth(ls.m_Width);
    m_plbXMajGridStyle->setBtnColor(ls.m_Color);
    setApplied(false);
}


void GraphDlg::onXMinGridStyle()
{
    LineStyle ls;
    bool bShow=false;
    bool bAuto=true;
    double unit=1.0;

    m_pGraph->bXMinGrid(bShow,bAuto, ls, unit);
    LineMenu lineMenu(this);
    lineMenu.showPointStyle(false);
    lineMenu.initMenu(ls);
    lineMenu.exec(QCursor::pos());
    ls = lineMenu.theStyle();
    m_pGraph->setXMinGrid(bShow, bAuto, ls, unit);
    m_plbXMinGridStyle->setStipple(ls.m_Stipple);
    m_plbXMinGridStyle->setWidth(ls.m_Width);
    m_plbXMinGridStyle->setBtnColor(ls.m_Color);
    setApplied(false);
}


void GraphDlg::onXMajGridShow()
{
    bool bShow = m_pchXMajGridShow->isChecked();
    m_pGraph->showXMajGrid(bShow);
    m_plbXMajGridStyle->setEnabled(bShow);
    setApplied(false);
}


void GraphDlg::onXMinGridShow()
{
    bool bShow = m_pchXMinGridShow->isChecked();
    m_pGraph->showXMinGrid(bShow);
    m_plbXMinGridStyle->setEnabled(bShow);

    setApplied(false);
}


void GraphDlg::onYInverted()
{
    m_pGraph->setYInverted(0, m_pchYInverted[0]->checkState() == Qt::Checked);
    m_pGraph->setYInverted(1, m_pchYInverted[1]->checkState() == Qt::Checked);
    setApplied(false);
}


void GraphDlg::onYMajGridShow()
{
    QCheckBox *pMajY = dynamic_cast<QCheckBox*>(sender());
    int iy=0;
    if     (pMajY==m_plabYMajGridShow[0]) iy=0;
    else if(pMajY==m_plabYMajGridShow[1]) iy=1;
    else return;

    bool bShow = m_plabYMajGridShow[iy]->isChecked();
    m_pGraph->showYMajGrid(iy, bShow);
    m_plbYMajGridStyle[iy]->setEnabled(bShow);
    setApplied(false);
}

void GraphDlg::onYMajGridStyle()
{
    LineBtn *pMajY = dynamic_cast<LineBtn*>(sender());
    int iy=0;
    if     (pMajY==m_plbYMajGridStyle[0]) iy=0;
    else if(pMajY==m_plbYMajGridStyle[1]) iy=1;
    else return;

    bool bShow=false;
    LineStyle ls;
    m_pGraph->yMajGrid(iy, bShow, ls);
    LineMenu lineMenu(this);
    lineMenu.showPointStyle(false);
    lineMenu.initMenu(ls);
    lineMenu.exec(QCursor::pos());
    ls = lineMenu.theStyle();
    m_pGraph->setYMajGrid(iy, bShow, ls);
    m_plbYMajGridStyle[iy]->setTheStyle(ls);
    setApplied(false);
}


void GraphDlg::onYMinGridShow()
{
    QCheckBox *pMajY = dynamic_cast<QCheckBox*>(sender());
    int iy=0;
    if     (pMajY==m_pchMinGridShow[0]) iy=0;
    else if(pMajY==m_pchMinGridShow[1]) iy=1;
    else return;

    bool bShow = m_pchMinGridShow[iy]->isChecked();
    m_pGraph->showYMinGrid(iy, bShow);
    m_plbYMinGridStyle[iy]->setEnabled(bShow);

    setApplied(false);
}


void GraphDlg::onYMinGridStyle()
{
    LineBtn *pMajY = dynamic_cast<LineBtn*>(sender());
    int iy=0;
    if     (pMajY==m_plbYMinGridStyle[0]) iy=0;
    else if(pMajY==m_plbYMinGridStyle[1]) iy=1;
    else return;

    LineStyle ls;
    bool bShow=false;
    bool bAuto=true;
    double unit=1.0;

    m_pGraph->bYMinGrid(iy, bShow,bAuto, ls, unit);
    LineMenu lineMenu(this);
    lineMenu.showPointStyle(false);
    lineMenu.initMenu(ls);
    lineMenu.exec(QCursor::pos());
    ls = lineMenu.theStyle();
    m_pGraph->setYMinGrid(iy, bShow, bAuto, ls, unit);
    m_plbYMinGridStyle[iy]->setTheStyle(ls);
    setApplied(false);
}


void GraphDlg::setApplied(bool bApplied)
{
    m_bApplied = bApplied;
}


void GraphDlg::setButtonColors()
{
    m_ptcbTitleClr->setTextColor(m_pGraph->titleColor());
    m_ptcbTitleClr->setBackgroundColor(m_pGraph->backgroundColor());

    m_ptcbLabelClr->setTextColor(m_pGraph->labelColor());
    m_ptcbLabelClr->setBackgroundColor(m_pGraph->backgroundColor());

    m_ptcbLegendClr->setTextColor(m_pGraph->legendColor());
    m_ptcbLegendClr->setBackgroundColor(m_pGraph->backgroundColor());
}


void GraphDlg::setControls()
{
    m_pchXAuto->setChecked(m_pGraph->bAutoX());
    m_pchXLog->setChecked(m_pGraph->bXLogScale());
    m_pdeXMin->setValue(m_pGraph->xMin());
    m_pdeXMax->setValue(m_pGraph->xMax());

    for(int iy=0; iy<2; iy++)
    {
        m_pchYAuto[iy]->setChecked(m_pGraph->bAutoY(iy));
        m_pchYLog[iy]->setChecked(m_pGraph->bYLogScale(iy));
        m_pdeYMin[iy]->setValue(m_pGraph->yMin(iy));
        m_pdeYMax[iy]->setValue(m_pGraph->yMax(iy));
    }

    m_prbExpanding->setChecked(m_pGraph->scaleType()==GRAPH::EXPANDING);
    m_prbResetting->setChecked(m_pGraph->scaleType()==GRAPH::RESETTING);

    m_pchRightAxis->setEnabled(m_pGraph->isRightAxisEnabled());
    m_pchRightAxis->setChecked(m_pGraph->hasRightAxis());
    onRightAxis(m_pGraph->hasRightAxis());

    onAutoX();

    m_pdeYMin[0]->setEnabled(!m_pGraph->bAutoY(0));
    m_pdeYMax[0]->setEnabled(!m_pGraph->bAutoY(0));
    m_pdeYUnit[0]->setEnabled(!m_pGraph->bAutoY(0));

    m_pdeYMin[1]->setEnabled(m_pGraph->hasRightAxis() && !m_pGraph->bAutoY(1));
    m_pdeYMax[1]->setEnabled(m_pGraph->hasRightAxis() && !m_pGraph->bAutoY(1));
    m_pdeYUnit[1]->setEnabled(m_pGraph->hasRightAxis() && !m_pGraph->bAutoY(1));

    setButtonColors();

    QFont font;
    font = m_pGraph->titleFont();
    m_ppbTitleButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
    m_ppbTitleButton->setFont(font);

    font = m_pGraph->labelFont();
    m_ppbLabelButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
    m_ppbLabelButton->setFont(font);

    font = m_pGraph->legendFont();
    m_ppbLegendButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
    m_ppbLegendButton->setFont(font);

    bool bState=false, bAuto=false;
    double unit=1;
    LineStyle ls;
    m_pGraph->xMajGrid(bState, ls);
    m_pchXMajGridShow->setChecked(bState);
    m_plbXMajGridStyle->setTheStyle(ls);
    m_plbXMajGridStyle->setEnabled(bState);

    m_pGraph->bXMinGrid(bState, bAuto, ls, unit);
    m_pchXMinGridShow->setChecked(bState);
    m_plbXMinGridStyle->setTheStyle(ls);
    m_plbXMinGridStyle->setEnabled(bState);

    for(int iy=0; iy<2; iy++)
    {
        m_pGraph->yMajGrid(iy, bState, ls);
        m_plabYMajGridShow[iy]->setChecked(bState);
        m_plbYMajGridStyle[iy]->setTheStyle(ls);
        m_plbYMajGridStyle[iy]->setEnabled(bState);

        m_pGraph->bYMinGrid(iy,bState, bAuto, ls, unit);
        m_pchMinGridShow[iy]->setChecked(bState);
        m_plbYMinGridStyle[iy]->setTheStyle(ls);
        m_plbYMinGridStyle[iy]->setEnabled(bState);
    }

    m_plbXAxisStyle->setTheStyle(m_pGraph->axisStyle(AXIS::XAXIS));
    m_plbYAxisStyle[0]->setTheStyle(m_pGraph->axisStyle(AXIS::LEFTYAXIS));
    m_plbYAxisStyle[1]->setTheStyle(m_pGraph->axisStyle(AXIS::RIGHTYAXIS));

    m_pchGraphBorder->setChecked(m_pGraph->hasBorder());
    m_plbBorderStyle->setTheStyle(m_pGraph->theBorderStyle());

    m_pcobGraphBack->setColor(m_pGraph->backgroundColor());

    m_pieLMargin->setValue(m_pGraph->leftMargin());
    m_pieRMargin->setValue(m_pGraph->rightMargin());
    m_pieTMargin->setValue(m_pGraph->topMargin());
    m_pieBMargin->setValue(m_pGraph->bottomMargin());

    m_pchYInverted[0]->setChecked(m_pGraph->bYInverted(0));
    m_pchYInverted[1]->setChecked(m_pGraph->bYInverted(1));


    m_plwXSel->setCurrentRow(m_pGraph->xVariable());
    m_plwYSel[0]->setCurrentRow(m_pGraph->yVariable(0));
    m_plwYSel[1]->setCurrentRow(m_pGraph->yVariable(1));

    m_bVariableChanged = false;

    m_pchShowLegend->setChecked(m_pGraph->isLegendVisible());

    m_prbLeft->setChecked(       m_pGraph->legendPosition() == (Qt::AlignLeft   | Qt::AlignVCenter));
    m_prbRight->setChecked(      m_pGraph->legendPosition() == (Qt::AlignRight  | Qt::AlignVCenter));
    m_prbTop->setChecked(        m_pGraph->legendPosition() == (Qt::AlignTop    | Qt::AlignHCenter));
    m_prbTopLeft->setChecked(    m_pGraph->legendPosition() == (Qt::AlignTop    | Qt::AlignLeft));
    m_prbTopRight->setChecked(   m_pGraph->legendPosition() == (Qt::AlignTop    | Qt::AlignRight));
    m_prbBottom->setChecked(     m_pGraph->legendPosition() == (Qt::AlignBottom | Qt::AlignHCenter));
    m_prbBottomLeft->setChecked( m_pGraph->legendPosition() == (Qt::AlignBottom | Qt::AlignLeft));
    m_prbBottomRight->setChecked(m_pGraph->legendPosition() == (Qt::AlignBottom | Qt::AlignRight));
    onShowLegend(m_pchShowLegend->isChecked());

    setApplied(true);

    //    m_pButtonBox->button(QDialogButtonBox::Apply)->setEnabled(m_pTabWidget->currentIndex()!=0);
}


void GraphDlg::setupLayout()
{
    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                                        QDialogButtonBox::RestoreDefaults);
    {
        QPushButton *pOKBtn = m_pButtonBox->button(QDialogButtonBox::Ok);
        if(pOKBtn)
        {
            pOKBtn->setDefault(true);
            pOKBtn->setAutoDefault(true);
        }
        m_pButtonBox->button(QDialogButtonBox::RestoreDefaults)->setDefault(false);
        m_pButtonBox->button(QDialogButtonBox::RestoreDefaults)->setAutoDefault(false);
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }


    QWidget *pScalePage    = new QWidget(this);
    QWidget *pGridPage     = new QWidget(this);
    QWidget *pFontPage     = new QWidget(this);
    QWidget *pVariablePage = new QWidget(this);
    QWidget *pLegendPage   = new QWidget(this);
    QWidget *pCurvePage    = new QWidget(this);

    //________Variable Page_______________________
    QGridLayout *pVariablePageLayout = new QGridLayout(this);
    {

        QLabel *pXAxis = new QLabel("XAxis");
        QLabel *pY0Axis = new QLabel("Left y-axis");
        m_pchRightAxis = new QCheckBox("Right y-axis");

        QLabel *pLabVs = new QLabel("vs.");

        m_plwXSel = new QListWidget;
        m_plwYSel[0] = new QListWidget;
        m_plwYSel[1] = new QListWidget;
        pVariablePageLayout->addWidget(pY0Axis,        1, 1, Qt::AlignHCenter);
        pVariablePageLayout->addWidget(m_pchRightAxis, 1, 2, Qt::AlignHCenter);
        pVariablePageLayout->addWidget(pXAxis,         1, 4, Qt::AlignHCenter);
        pVariablePageLayout->addWidget(m_plwYSel[0],   2, 1);
        pVariablePageLayout->addWidget(m_plwYSel[1],   2, 2);
        pVariablePageLayout->addWidget(pLabVs,         2, 3);
        pVariablePageLayout->addWidget(m_plwXSel,      2, 4);
    }
    pVariablePage->setLayout(pVariablePageLayout);
    //________End Variable Page___________________

    //________Font Page___________________________
    QVBoxLayout *pFontPageLayout = new QVBoxLayout;
    {
        QGroupBox *pFontBox = new QGroupBox("Fonts");
        {
            QGridLayout *pFontButtonsLayout = new QGridLayout;
            {
                QLabel *pLab1  = new QLabel("Axis titles:");
                QLabel *pLab2  = new QLabel("Axis labels:");
                QLabel *pLab3  = new QLabel("In-graph legend:");
                QLabel *pLab402  = new QLabel("Font");
                QLabel *pLab403  = new QLabel("Colour");
                pLab1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pLab2->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pLab402->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
                pLab403->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
                pFontButtonsLayout->addWidget(pLab402, 1,2);
                pFontButtonsLayout->addWidget(pLab403, 1,3);
                pFontButtonsLayout->addWidget(pLab1,   2,1);
                pFontButtonsLayout->addWidget(pLab2,   3,1);
                pFontButtonsLayout->addWidget(pLab3,   4,1);

                m_ppbTitleButton  = new QPushButton();
                m_ppbLabelButton  = new QPushButton();
                m_ppbLegendButton = new QPushButton();

                pFontButtonsLayout->addWidget(m_ppbTitleButton,  2,2);
                pFontButtonsLayout->addWidget(m_ppbLabelButton,  3,2);
                pFontButtonsLayout->addWidget(m_ppbLegendButton, 4,2);

                m_ptcbTitleClr  = new TextClrBtn(this);
                m_ptcbTitleClr->setText("Title colour");
                m_ptcbLabelClr  = new TextClrBtn(this);
                m_ptcbLabelClr->setText("Label colour");
                m_ptcbLegendClr  = new TextClrBtn(this);
                m_ptcbLegendClr->setText("Legend colour");

                pFontButtonsLayout->addWidget(m_ptcbTitleClr,  2,3);
                pFontButtonsLayout->addWidget(m_ptcbLabelClr,  3,3);
                pFontButtonsLayout->addWidget(m_ptcbLegendClr, 4,3);
            }
            pFontBox->setLayout(pFontButtonsLayout);
        }

        QGroupBox *pBackBox = new QGroupBox("BackGround");
        {
            QGridLayout *pBackDataLayout = new QGridLayout;
            {
                QLabel *GraphBackLabel = new QLabel("Graph background");
                GraphBackLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_pchGraphBorder = new QCheckBox("Graph border");

                m_pcobGraphBack = new ColorBtn;
                m_plbBorderStyle = new LineBtn(this);

                pBackDataLayout->addWidget(GraphBackLabel,   1,1);
                pBackDataLayout->addWidget(m_pchGraphBorder, 2,1,Qt::AlignRight | Qt::AlignVCenter);

                pBackDataLayout->addWidget(m_pcobGraphBack,  1,2);
                pBackDataLayout->addWidget(m_plbBorderStyle, 2,2);

                pBackDataLayout->setColumnStretch(1,7);
                pBackDataLayout->setColumnStretch(2,3);
            }
            pBackBox->setLayout(pBackDataLayout);
        }
        QGroupBox *pPaddingBox = new QGroupBox("Margins");
        {
            QGridLayout *pPaddingLayout = new QGridLayout;
            {
                QLabel *pMarginUnit = new QLabel("pixels");
                m_pieLMargin = new IntEdit(31, this);
                m_pieRMargin = new IntEdit(31, this);
                m_pieTMargin = new IntEdit(31, this);
                m_pieBMargin = new IntEdit(31, this);

                //                pPaddingLayout->addWidget(pMarginLabel);
                pPaddingLayout->addWidget(m_pieLMargin, 2, 2);
                pPaddingLayout->addWidget(m_pieRMargin, 2, 4);
                pPaddingLayout->addWidget(m_pieTMargin, 1, 3);
                pPaddingLayout->addWidget(m_pieBMargin, 3, 3);
                pPaddingLayout->addWidget(pMarginUnit,2,5);
            }
            pPaddingBox->setLayout(pPaddingLayout);
        }

        pFontPageLayout->addWidget(pFontBox);
        pFontPageLayout->addStretch(1);
        pFontPageLayout->addWidget(pBackBox);
        pFontPageLayout->addStretch(1);
        pFontPageLayout->addWidget(pPaddingBox);
        pFontPageLayout->addStretch(1);
    }
    pFontPage->setLayout(pFontPageLayout);
    //________End Font Page_______________________

    //________Scale Page______________________
    QVBoxLayout *pScalePageLayout = new QVBoxLayout;
    {
        QFrame *pScaleFrame = new QFrame;
        {
            QGridLayout *pScaleLayout = new QGridLayout;
            {
                QLabel *pXAxis  = new QLabel("X Axis");
                QLabel *pYAxis0 = new QLabel("Y-Axis-left");
                QLabel *pYAxis1 = new QLabel("Y-Axis-right");
                pXAxis->setAlignment(Qt::AlignCenter);
                pYAxis0->setAlignment(Qt::AlignCenter);
                pYAxis1->setAlignment(Qt::AlignCenter);

                QLabel *pMinLabel = new QLabel("Min");
                QLabel *pMaxLabel = new QLabel("Max");
                QLabel *pUnitLabel = new QLabel("Unit");
                pMinLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                pMaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                pUnitLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                //    ScaleData->addStretch(1);
                pScaleLayout->addWidget(pMinLabel,   4,1);
                pScaleLayout->addWidget(pMaxLabel,   5,1);
                //        pScalePageLayout->addWidget(pUnitLabel,  6,1);

                m_pchXAuto    = new QCheckBox("Auto scale");
                m_pdeXMin     = new FloatEdit;
                m_pdeXMax     = new FloatEdit;

                m_pdeXUnit    = new FloatEdit;
                m_pchXLog     = new QCheckBox("Log scale");

                pScaleLayout->addWidget(pXAxis,       1,2, Qt::AlignHCenter | Qt::AlignBottom);
                pScaleLayout->addWidget(m_pchXAuto,   3,2);
                pScaleLayout->addWidget(m_pdeXMin,    4,2);
                pScaleLayout->addWidget(m_pdeXMax,    5,2);
                pScaleLayout->addWidget(m_pchXLog,    7,2);

                for(int iy=0; iy<2; iy++)
                {
                    m_pchYInverted[iy] = new QCheckBox("Inverted axis");
                    m_pchYAuto[iy]     = new QCheckBox("Auto scale");
                    m_pdeYMin[iy]      = new FloatEdit;
                    m_pdeYMax[iy]      = new FloatEdit;
                    m_pdeYUnit[iy]     = new FloatEdit;
                    m_pchYLog[iy]      = new QCheckBox("Log scale");
                }

                pScaleLayout->addWidget(pYAxis0,           1,3, Qt::AlignHCenter | Qt::AlignBottom);
                pScaleLayout->addWidget(m_pchYInverted[0], 2,3);
                pScaleLayout->addWidget(m_pchYAuto[0],     3,3);
                pScaleLayout->addWidget(m_pdeYMin[0],      4,3);
                pScaleLayout->addWidget(m_pdeYMax[0],      5,3);
                pScaleLayout->addWidget(m_pchYLog[0],      7,3);

                pScaleLayout->addWidget(pYAxis1,             1,4, Qt::AlignHCenter | Qt::AlignBottom);
                pScaleLayout->addWidget(m_pchYInverted[1], 2,4);
                pScaleLayout->addWidget(m_pchYAuto[1],     3,4);
                pScaleLayout->addWidget(m_pdeYMin[1],      4,4);
                pScaleLayout->addWidget(m_pdeYMax[1],      5,4);
                pScaleLayout->addWidget(m_pchYLog[1],      7,4);

                pScaleLayout->setRowStretch(8,1);
                pScaleLayout->setColumnStretch(5,1);
            }
            pScaleFrame->setLayout(pScaleLayout);
        }
        QFrame *pScaleTypeFrame = new QFrame;
        {
            QHBoxLayout *pScaleTypeLayout = new QHBoxLayout;
            {
                QLabel *pScaleTypeLab = new QLabel("Scale type");
                m_prbResetting = new QRadioButton("Resetting");
                m_prbExpanding = new QRadioButton("Expanding");
                pScaleTypeLayout->addWidget(pScaleTypeLab);
                pScaleTypeLayout->addWidget(m_prbExpanding);
                pScaleTypeLayout->addWidget(m_prbResetting);
                pScaleTypeLayout->addStretch();
            }
            pScaleTypeFrame->setLayout(pScaleTypeLayout);
        }

        pScalePageLayout->addWidget(pScaleFrame);
        pScalePageLayout->addStretch();
        pScalePageLayout->addWidget(pScaleTypeFrame);
    }
    pScalePage->setLayout(pScalePageLayout);
    //________End Scale Page______________________

    //________Axis Page___________________________
    QHBoxLayout *pStylePageLayout = new QHBoxLayout;
    {
        QGridLayout *pXGridStyleLayout = new QGridLayout;
        {
            QLabel *pXGridLabel = new QLabel("X");
            QLabel *pAxisStyleLabel = new QLabel("Axis style");

            m_plbXAxisStyle = new LineBtn(this);

            m_pchXMajGridShow = new QCheckBox("Major grid");
            m_pchXMinGridShow = new QCheckBox("Minor grid");

            m_plbXMajGridStyle = new LineBtn(this);
            m_plbXMinGridStyle = new LineBtn(this);


            pXGridStyleLayout->addWidget(pAxisStyleLabel,      1, 0, Qt::AlignHCenter | Qt::AlignRight);
            pXGridStyleLayout->addWidget(m_pchXMajGridShow,  2, 0, Qt::AlignHCenter | Qt::AlignRight);
            pXGridStyleLayout->addWidget(m_pchXMinGridShow,  3, 0, Qt::AlignHCenter | Qt::AlignRight);

            pXGridStyleLayout->addWidget(pXGridLabel,  0, 1, Qt::AlignHCenter);


            pXGridStyleLayout->addWidget(m_plbXAxisStyle,     1, 1);
            pXGridStyleLayout->addWidget(m_plbXMajGridStyle,  2, 1);
            pXGridStyleLayout->addWidget(m_plbXMinGridStyle,  3, 1);
            pXGridStyleLayout->setRowStretch(4,1);
        }
        pStylePageLayout->addLayout(pXGridStyleLayout);

        for(int iy=0; iy<2; iy++)
        {
            QGridLayout *pYGridStyleLayout = new QGridLayout;
            {
                QString name = iy==0 ? "Y left" : "Y right";

                QLabel *pXGridLabel = new QLabel(name);
                QLabel *pAxisStyleLabel = new QLabel("Axis style");

                m_plbYAxisStyle[iy] = new LineBtn(this);

                m_plabYMajGridShow[iy] = new QCheckBox("Major grid");
                m_pchMinGridShow[iy] = new QCheckBox("Minor grid");

                m_plbYMajGridStyle[iy] = new LineBtn(this);
                m_plbYMinGridStyle[iy] = new LineBtn(this);


                pYGridStyleLayout->addWidget(pAxisStyleLabel,        1, 0, Qt::AlignHCenter | Qt::AlignRight);
                pYGridStyleLayout->addWidget(m_plabYMajGridShow[iy], 2, 0, Qt::AlignHCenter | Qt::AlignRight);
                pYGridStyleLayout->addWidget(m_pchMinGridShow[iy],   3, 0, Qt::AlignHCenter | Qt::AlignRight);

                pYGridStyleLayout->addWidget(pXGridLabel,  0, 1, Qt::AlignHCenter);

                pYGridStyleLayout->addWidget(m_plbYAxisStyle[iy],     1, 1);
                pYGridStyleLayout->addWidget(m_plbYMajGridStyle[iy],  2, 1);
                pYGridStyleLayout->addWidget(m_plbYMinGridStyle[iy],  3, 1);
                pYGridStyleLayout->setRowStretch(4,1);

            }
            pStylePageLayout->addLayout(pYGridStyleLayout);
        }
    }
    pGridPage->setLayout(pStylePageLayout);
    //________End Axis Page_______________________

    //________Legend Page_________________________
    QVBoxLayout *pLegendLayout = new QVBoxLayout;
    {
        m_pchShowLegend  = new QCheckBox("Show in-graph legend");
        m_prbLeft        = new QRadioButton("Left");
        m_prbRight       = new QRadioButton("Right");
        m_prbTop         = new QRadioButton("Top");
        m_prbTopLeft     = new QRadioButton("TL");
        m_prbTopRight    = new QRadioButton("TR");
        m_prbBottom      = new QRadioButton("Bottom");
        m_prbBottomLeft  = new QRadioButton("BL");
        m_prbBottomRight = new QRadioButton("BR");
        QGridLayout *pLegendPlaceLayout = new QGridLayout;
        {
            pLegendPlaceLayout->addWidget(m_prbTopLeft,     1, 1);
            pLegendPlaceLayout->addWidget(m_prbTop,         1, 2);
            pLegendPlaceLayout->addWidget(m_prbTopRight,    1, 3);
            pLegendPlaceLayout->addWidget(m_prbLeft,        2, 1);
            pLegendPlaceLayout->addWidget(m_prbRight,       2, 3);
            pLegendPlaceLayout->addWidget(m_prbBottomLeft,  3, 1);
            pLegendPlaceLayout->addWidget(m_prbBottom,      3, 2);
            pLegendPlaceLayout->addWidget(m_prbBottomRight, 3, 3);
            pLegendPlaceLayout->setColumnStretch(3,1);
        }
        pLegendLayout->addWidget(m_pchShowLegend);
        pLegendLayout->addLayout(pLegendPlaceLayout);
        pLegendLayout->addStretch();
    }
    pLegendPage->setLayout(pLegendLayout);
    //________End Legend Page_____________________

    //________Curve Page__________________________

    QVBoxLayout *pCurveLayout = new QVBoxLayout;
    {
        m_pcpCurveTable = new CPTableView(this);
        m_pcpCurveTable->setShowGrid(false);
        m_pcpCurveTable->setEditable(true);
        m_pcpCurveTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        m_pCurveModel = new ActionItemModel(this);
        m_pCurveModel->setRowCount(0);//nothing to start with
        m_pCurveModel->setColumnCount(2);
        m_pCurveModel->setActionColumn(-1);
        m_pCurveModel->setHeaderData(0, Qt::Horizontal, "Name");
        m_pCurveModel->setHeaderData(1, Qt::Horizontal, "Style");
        m_pCurveModel->setHeaderData(2, Qt::Horizontal, "Actions");

        m_pcpCurveTable->setModel(m_pCurveModel);

        XflDelegate *m_pEditActionDelegate = new XflDelegate(this);
        m_pEditActionDelegate->setActionColumn(2);
        m_pEditActionDelegate->setDigits({-1,-1,-1});

        QVector<XflDelegate::enumItemType> ItemType = {XflDelegate::STRING, XflDelegate::LINE, XflDelegate::ACTION};
        m_pEditActionDelegate->setItemTypes(ItemType);

        m_pcpCurveTable->setItemDelegate(m_pEditActionDelegate);
        pCurveLayout->addWidget(m_pcpCurveTable);
    }
    pCurvePage->setLayout(pCurveLayout);
    //________End Curve Page______________________

    m_pTabWidget = new QTabWidget(this);
    m_pTabWidget->addTab(pVariablePage, "Variables");
    m_pTabWidget->addTab(pScalePage,    "Scales");
    m_pTabWidget->addTab(pGridPage,     "Axes and grids");
    m_pTabWidget->addTab(pFontPage,     "Fonts and background");
    m_pTabWidget->addTab(pLegendPage,   "Legend");
    m_pTabWidget->addTab(pCurvePage,    "Curves");

    m_pTabWidget->setCurrentIndex(s_iActivePage);
    connect(m_pTabWidget, SIGNAL(currentChanged(int)), SLOT(onActivePage(int)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    {
        mainLayout->addWidget(m_pTabWidget);
        mainLayout->addWidget(m_pButtonBox);
    }
    setLayout(mainLayout);
}


void GraphDlg::setGraph(Graph *pGraph)
{
    m_pGraph = pGraph;
    m_SaveGraph.copySettings(*m_pGraph);
    fillVariableList();
    if(m_bCurveStylePage) fillCurvePage();
    setControls();
}



void GraphDlg::setActivePage(int iPage)
{
    s_iActivePage =iPage;
}


void GraphDlg::onShowLegend(bool bShow)
{
    m_prbLeft->setEnabled(bShow);
    m_prbRight->setEnabled(bShow);
    m_prbTop->setEnabled(bShow);
    m_prbTopLeft->setEnabled(bShow);
    m_prbTopRight->setEnabled(bShow);
    m_prbBottom->setEnabled(bShow);
    m_prbBottomLeft->setEnabled(bShow);
    m_prbBottomRight->setEnabled(bShow);
}


void GraphDlg::fillCurvePage()
{
//    m_pCurveModel->blockSignals(true);
    m_pCurveModel->setRowCount(m_pGraph->curveCount());
    int ncurves = m_pCurveModel->rowCount();

    for(int ic=0; ic<ncurves; ic++)
    {
        Curve const *pCurve = m_pGraph->curve(ic);
        if(!pCurve) continue;

        QModelIndex Xindex = m_pCurveModel->index(ic, 0, QModelIndex());
        m_pCurveModel->setData(Xindex, pCurve->name());

        QModelIndex lineindex = m_pCurveModel->index(ic, 1, QModelIndex());
        QStandardItem *pItem = m_pCurveModel->itemFromIndex(lineindex);
        if(pItem) pItem->setData(QVariant::fromValue(pCurve->theStyle()), Qt::DisplayRole);
    }

//    m_pCurveModel->blockSignals(false);
    m_pcpCurveTable->resizeRowsToContents();
}


void GraphDlg::onCurveTableClicked(QModelIndex index)
{
    if(!index.isValid()) return

    m_pcpCurveTable->selectRow(index.row());
    Curve *pCurrentCurve = m_pGraph->curve(index.row());
    if(!pCurrentCurve) return;

    switch(index.column())
    {
        case 1:
        {
            LineMenu *lineMenu = new LineMenu(nullptr);
            lineMenu->initMenu(pCurrentCurve->theStyle());
            lineMenu->exec(QCursor::pos());

            // update the model
            QStandardItem *pItem = m_pCurveModel->itemFromIndex(index);
            if(pItem) pItem->setData(QVariant::fromValue(lineMenu->theStyle()), Qt::DisplayRole);
             pCurrentCurve->setTheStyle(lineMenu->theStyle());

            break;
        }
        default:
        {
            break;
        }
    }
}


void GraphDlg::showCurvePage(bool bShow)
{
    m_bCurveStylePage=bShow;
    m_pTabWidget->setTabEnabled(5, bShow);
}







