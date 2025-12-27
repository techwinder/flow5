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

#include <QPushButton>
#include <QColorDialog>
#include <QPainter>
#include <QPaintEvent>
#include <QVBoxLayout>


#include "colorgraddlg.h"
#include <interfaces/widgets/color/colorbtn.h>
#include <interfaces/opengl/controls/colourlegend.h>
#include <api/utils.h>
#include <core/xflcore.h>

ColorGradDlg::ColorGradDlg(QVector<QColor>const &clrs, QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Colour gradients");

    m_Clr=clrs;

    setupLayout();

    if(m_Clr.size()==2)
    {
        m_prb2Colors->setChecked(true);
    }
    else
    {
         m_prb3Colors->setChecked(true);
    }
    updateColouredFrame();
}


void ColorGradDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard, this);
        {
             m_prb2Colors = new QRadioButton("2 colors");
             m_prb3Colors = new QRadioButton("3 colors");
             connect(m_prb2Colors, SIGNAL(clicked(bool)), SLOT(onNColors()));
             connect(m_prb3Colors, SIGNAL(clicked(bool)), SLOT(onNColors()));

             connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
             m_pButtonBox->addButton(m_prb2Colors, QDialogButtonBox::ActionRole);
             m_pButtonBox->addButton(m_prb3Colors, QDialogButtonBox::ActionRole);

             m_pButtonBox->setAutoFillBackground(true);
        }


        QHBoxLayout *pClrLayout = new QHBoxLayout;
        {
            m_pDiscreteClrFrame = new QFrame(this);
            makeCtrlFrameLayout();
            m_pTestClrFrame = new QWidget;
            pClrLayout->addWidget(m_pDiscreteClrFrame);
            pClrLayout->addWidget(m_pTestClrFrame);
            pClrLayout->setStretchFactor(m_pTestClrFrame, 1);
        }
        pMainLayout->addLayout(pClrLayout);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void ColorGradDlg::makeCtrlFrameLayout()
{
    qDeleteAll(m_pDiscreteClrFrame->children()); // removes the layout  and destroys the color btns

    m_pColorBtn.resize(m_Clr.size());
    QVBoxLayout*pClrBtnLayout = new QVBoxLayout;
    {
        for(int i=m_pColorBtn.size()-1; i>=0; i--)
        {
            m_pColorBtn[i] = new ColorBtn;
            m_pColorBtn[i]->setColor(m_Clr.at(i));
            connect(m_pColorBtn[i], SIGNAL(clickedCB(QColor)), SLOT(onColorBtn()));
            pClrBtnLayout->addWidget(m_pColorBtn[i]);
            if(i>0) pClrBtnLayout->addStretch();
        }
    }
    m_pDiscreteClrFrame->setLayout(pClrBtnLayout);
}


void ColorGradDlg::onNColors()
{
     QRadioButton *pSenderBtn = dynamic_cast<QRadioButton*>(sender());
     if(pSenderBtn==m_prb2Colors)
     {
        if(m_Clr.size()!=2)
        {
            m_Clr.resize(2);
        }

        m_Clr[0].setRgb(255,255,255);
        m_Clr[1].setRgb( 0, 0, 0);

     }
     else if(pSenderBtn==m_prb3Colors)
     {
        if(m_Clr.size()!=3)
        {
            m_Clr.resize(3);
        }
        m_Clr[2] = Qt::red;
        m_Clr[1] = Qt::green;
        m_Clr[0] = Qt::blue;
     }
     makeCtrlFrameLayout();
     updateColouredFrame();
     update();
}


void ColorGradDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)       accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}


void ColorGradDlg::onColorBtn()
{
    ColorBtn *pClrbtn = qobject_cast<ColorBtn *>(sender());

    QColor BtnColor = pClrbtn->color();
    BtnColor = QColorDialog::getColor(BtnColor);
    if(BtnColor.isValid())
    {
        pClrbtn->setColor(BtnColor);
        for(int i=0; i<m_pColorBtn.size(); i++)
        {
            if(pClrbtn==m_pColorBtn[i])
            {
                m_Clr[i] = BtnColor;
            }
        }
    }
    updateColouredFrame();
    update();
}

#define NDISCRETE 11

void ColorGradDlg::updateColouredFrame()
{
    QString style("background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,");
    QString strange;
    for(int i=0; i<NDISCRETE; i++)
    {
        double fi = double(i)/double(NDISCRETE-1);
        strange = QString::asprintf(" stop:%11.3f", fi) + xfl::colour(m_Clr, 1.0-fi).name();
        if(fi<NDISCRETE-1) strange += ",";
        style += strange;
    }
    style += ")";


    m_pTestClrFrame->setStyleSheet(style);
    m_pTestClrFrame->update();
}


QColor ColorGradDlg::colour(float tau) const
{
    if(tau<=0.0f) return m_Clr.front();

    double df = double(m_Clr.size()-1);
    for(int i=1; i<m_Clr.size(); i++)
    {
        double fi  = double(i)/df;
        if(tau<fi)
        {
            double hue0 = qMax(0.0, m_Clr.at(i-1).hueF()); // hue returns -1 if grey
            double sat0 = m_Clr.at(i-1).saturationF();
            double val0 = m_Clr.at(i-1).valueF();
            double hue1 = qMax(0.0, m_Clr.at(i).hueF());
            double sat1 = m_Clr.at(i).saturationF();
            double val1 = m_Clr.at(i).valueF();

            double t = (fi-tau)/(1/df);

            // hue is undefined for pure grey colors, so use the other color's hue
            if(sat0<0.005 && sat1>0.005) hue0 = hue1;
            if(sat0>0.005 && sat1<0.005) hue1 = hue0;

            double hue = t*hue0+(1-t)*hue1;
            double sat = t*sat0+(1-t)*sat1;
            double val = t*val0+(1-t)*val1;

            return QColor::fromHsvF(hue, sat, val); // does not accept negative hue


/*            double r0 = m_Clr.at(i-1).redF();
            double g0 = m_Clr.at(i-1).greenF();
            double b0 = m_Clr.at(i-1).blueF();
            double r1 = m_Clr.at(i).redF();
            double g1 = m_Clr.at(i).greenF();
            double b1 = m_Clr.at(i).blueF();

            double t = (fi-tau)/(1/df);

            double red   = t*r0+(1-t)*r1;
            double green = t*g0+(1-t)*g1;
            double blue  = t*b0+(1-t)*b1;
            return QColor::fromRgbF(red, green, blue);*/
        }
    }
    return m_Clr.back();
}




