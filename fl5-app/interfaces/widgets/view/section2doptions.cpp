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

#include <QHBoxLayout>
#include <QColorDialog>

#include "section2doptions.h"
#include <core/xflcore.h>
#include <interfaces/widgets/customwts/intedit.h>
#include <interfaces/widgets/view/section2dwt.h>
#include <interfaces/widgets/customwts/gridcontrol.h>
#include <interfaces/widgets/line/linebtn.h>
#include <interfaces/widgets/line/linemenu.h>
#include <interfaces/widgets/color/colorbtn.h>
#include <interfaces/widgets/customwts/floatedit.h>

Grid Section2dOptions::s_RefGrid;
bool Section2dOptions::s_bModified = false;

Section2dOptions::Section2dOptions(QWidget *pParent) : QWidget(pParent)
{
    setupLayout();
    connectSignals();
}


void Section2dOptions::setModified(bool bModified)
{
    s_bModified=bModified;
}


void Section2dOptions::onSetModified(bool bModified)
{
    s_bModified=bModified;
}


void Section2dOptions::onResetDefaults()
{
     s_RefGrid.setDefaults();
     Section2dWt::setNPixelSelection(20);
     Section2dWt::setAnimateTransitions(true);
     Section2dWt::setAntiAliasing(true);
     initWidgets();
     setModified(true);
}


void Section2dOptions::setupLayout()
{
    m_pGroupBox.push_back(new QGroupBox(tr("General")));
    {
        QVBoxLayout *pGeneralLayout = new QVBoxLayout;
        {
            QHBoxLayout *pDynamicLayout = new QHBoxLayout;
            {
                m_pchSpinAnimation = new QCheckBox(tr("Enable mouse animations"));
                m_pfeSpinDamping = new FloatEdit;
                m_pfeSpinDamping->setToolTip(tr("Defines the damping of the animation at each frame update.<br>"
                                             "Set to 0 for perpetual movement."));
                QLabel *plabpcDamping = new QLabel(tr("% damping"));
                pDynamicLayout->addWidget(m_pchSpinAnimation);
                pDynamicLayout->addWidget(m_pfeSpinDamping);
                pDynamicLayout->addWidget(plabpcDamping);
                pDynamicLayout->addStretch();
            }

            QGridLayout*pSelectionLayout = new QGridLayout;
            {
                QLabel *pSelectionLabel = new QLabel(tr("Point selection precision:"));
                QLabel *pSelectionUnit= new QLabel(tr("pixels"));

                m_pieSelectionPixels = new IntEdit;
                QString tip(tr("<p>Defines the tolerance in pixels within which points are "
                               "selected or highlighted when moving the mouse in 2d views</p>"));
                m_pieSelectionPixels->setToolTip(tip);

                QLabel *plabSym = new QLabel(tr("Symbol size:"));
                m_pieSymbolSize = new IntEdit;
                m_pieSymbolSize->setToolTip(tr("This value defines the half-size of symbols in the graphs and other 2d views."));
                QLabel *plabPixels = new QLabel(tr("x2 pixels"));

                pSelectionLayout->addWidget(pSelectionLabel,      1, 1);
                pSelectionLayout->addWidget(m_pieSelectionPixels, 1, 2);
                pSelectionLayout->addWidget(pSelectionUnit,       1, 3);
                pSelectionLayout->addWidget(plabSym,              2, 1);
                pSelectionLayout->addWidget(m_pieSymbolSize,      2, 2);
                pSelectionLayout->addWidget(plabPixels,           2, 3);
                pSelectionLayout->setColumnStretch(4,1);
            }

            m_pchAntiAliasing = new QCheckBox(tr("Enable anti-aliasing"));
            m_pchAntiAliasing->setToolTip(tr("Indicates that the paint engine should antialias edges of primitives if possible."));
            QHBoxLayout *pColorLayout = new QHBoxLayout;
            {
                QLabel *plabHigh = new QLabel(tr("Highlight style"));
                m_plbHigh = new LineBtn;
                QLabel *plabSel = new QLabel(tr("Selection style"));
                m_plbSelect = new LineBtn;

                pColorLayout->addWidget(plabHigh);
                pColorLayout->addWidget(m_plbHigh);
                pColorLayout->addStretch();
                pColorLayout->addWidget(plabSel);
                pColorLayout->addWidget(m_plbSelect);
                pColorLayout->addStretch();
            }


            pGeneralLayout->addLayout(pDynamicLayout);
            pGeneralLayout->addWidget(m_pchAntiAliasing);
            pGeneralLayout->addLayout(pSelectionLayout);
            pGeneralLayout->addLayout(pColorLayout);
        }
        m_pGroupBox.back()->setLayout(pGeneralLayout);
    }

    m_pGroupBox.push_back(new QGroupBox(tr("Axes and grids")));
    {
        QVBoxLayout *pGridDataLayout = new QVBoxLayout;
        {
            m_pGridControl = new GridControl;
            pGridDataLayout->addWidget(m_pGridControl);
            pGridDataLayout->addStretch();
        }
        m_pGroupBox.back()->setLayout(pGridDataLayout);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        for(int i=0; i<m_pGroupBox.size(); i++)
        {
            pMainLayout->addWidget(m_pGroupBox.at(i));
        }
        pMainLayout->addStretch();
    }
    setLayout(pMainLayout);
}


void Section2dOptions::connectSignals()
{
    connect(m_pGridControl,     SIGNAL(gridModified(bool)),   SLOT(onSetModified(bool)));
    connect(m_plbHigh,          SIGNAL(clickedLB(LineStyle)), SLOT(onHighStyle(LineStyle)));
    connect(m_plbSelect,        SIGNAL(clickedLB(LineStyle)), SLOT(onSelStyle(LineStyle)));

    connect(m_pchSpinAnimation, SIGNAL(clicked(bool)),        SLOT(onSpinAnimation(bool)));
}


void Section2dOptions::initWidgets()
{
    m_pchSpinAnimation->setChecked(Section2dWt::bAnimateTransitions());
    m_pfeSpinDamping->setValue(Section2dWt::spinDamping()*100.0);
    m_pfeSpinDamping->setEnabled(Section2dWt::bAnimateTransitions());

    m_pieSelectionPixels->setValue(Section2dWt::nPixelSelection());
    m_pieSymbolSize->setValue(xfl::symbolSize());
    m_pchAntiAliasing->setChecked(Section2dWt::bAntiAliasing());
    m_pGridControl->initControls(&s_RefGrid);

    m_plbHigh->setTheStyle(Section2dWt::highStyle());
    m_plbSelect->setTheStyle(Section2dWt::selectStyle());
}


void Section2dOptions::readData()
{
    xfl::setSymbolSize(m_pieSymbolSize->value());
    Section2dWt::setNPixelSelection(m_pieSelectionPixels->value());
    Section2dWt::setAnimateTransitions(m_pchSpinAnimation->isChecked());
    Section2dWt::setSpinDamping(m_pfeSpinDamping->value()/100.0);
    Section2dWt::setAntiAliasing(m_pchAntiAliasing->isChecked());
}


void Section2dOptions::onSpinAnimation(bool bSpin)
{
    m_pfeSpinDamping->setEnabled(bSpin);
}


void Section2dOptions::showBox(int iBox)
{
    if(iBox<0)
    {
        for(int i=0; i<m_pGroupBox.size(); i++)
            m_pGroupBox[i]->setVisible(true);
    }
    else
    {
        for(int i=0; i<m_pGroupBox.size(); i++)
            m_pGroupBox[i]->setVisible(i==iBox);
    }
}


void Section2dOptions::onHighStyle(LineStyle ls)
{
    LineMenu lm(nullptr, false);
    lm.initMenu(ls);
    lm.exec(QCursor::pos());

    Section2dWt::setHighStyle(lm.theStyle());
    m_plbHigh->setTheStyle(lm.theStyle());
}


void Section2dOptions::onSelStyle(LineStyle ls)
{
    LineMenu lm(nullptr, false);
    lm.initMenu(ls);
    lm.exec(QCursor::pos());

    Section2dWt::setSelectStyle(lm.theStyle());
    m_plbSelect->setTheStyle(lm.theStyle());
}


void Section2dOptions::loadSettings(QSettings &settings)
{
    settings.beginGroup("Section2dOptions");
    {
        Section2dWt::setAnimateTransitions(settings.value("AnimateTransitions", Section2dWt::bAnimateTransitions()).toBool());
        Section2dWt::setNPixelSelection(   settings.value("NPixelSel",          Section2dWt::nPixelSelection()).toInt());
        Section2dWt::setAntiAliasing(      settings.value("bAntiAliasing",      Section2dWt::bAntiAliasing()).toBool());

        if(settings.contains("HighStyle_color"))
        {
            LineStyle ls;
            xfl::loadLineSettings(settings, ls, "HighStyle");
            Section2dWt::setHighStyle(ls);
            xfl::loadLineSettings(settings, ls, "SelStyle");
            Section2dWt::setSelectStyle(ls);
         }

        s_RefGrid.loadSettings(settings);
    }
    settings.endGroup();
}


void Section2dOptions::saveSettings(QSettings &settings)
{
    settings.beginGroup("Section2dOptions");
    {
        settings.setValue("AnimateTransitions", Section2dWt::bAnimateTransitions());
        settings.setValue("NPixelSel",          Section2dWt::nPixelSelection());
        settings.setValue("bAntiAliasing",      Section2dWt::bAntiAliasing());

        xfl::saveLineSettings(settings, Section2dWt::highStyle(), "HighStyle");
        xfl::saveLineSettings(settings, Section2dWt::selectStyle(), "SelStyle");

        s_RefGrid.saveSettings(settings);
    }
    settings.endGroup();
}


